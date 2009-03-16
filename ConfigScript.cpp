#include "ConfigScript.h"

#include <OgreException.h>
#include <OgreScriptLoader.h>
#include <OgreScriptLoader.h>
#include <OgreResourceGroupManager.h>
using namespace Ogre;

#include <vector>
#include <map>
using namespace std;


ConfigScriptLoader *ConfigScriptLoader::singletonPtr = NULL;

ConfigScriptLoader::ConfigScriptLoader(Ogre::String pattern = "*.object")
{
    //Init singleton
    if (singletonPtr)
	OGRE_EXCEPT(1, "Multiple ConfigScriptManager objects are not allowed", "ConfigScriptManager::ConfigScriptManager()");
    singletonPtr = this;

    //Register as a ScriptLoader
    mLoadOrder = 100.0f;
    mScriptPatterns.push_back(pattern);
    ResourceGroupManager::getSingleton()._registerScriptLoader(this);
}

ConfigScriptLoader::~ConfigScriptLoader()
{
    singletonPtr = NULL;

    //Delete all scripts
    std::map<String, ConfigNode*>::iterator i;
    for (i = scriptList.begin(); i != scriptList.end(); i++){
	delete i->second;
    }
    scriptList.clear();

    //Unregister with resource group manager
    if (ResourceGroupManager::getSingletonPtr())
	ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
}

Real ConfigScriptLoader::getLoadingOrder() const
{
    return mLoadOrder;
}

const StringVector &ConfigScriptLoader::getScriptPatterns() const
{
    return mScriptPatterns;
}

ConfigNode *ConfigScriptLoader::getConfigScript(const String &type, const String &name)
{
    std::map<String, ConfigNode*>::iterator i;

    String key = type + ' ' + name;
    i = scriptList.find(key);

    //If found..
    if (i != scriptList.end())
	return i->second;
    else
	return NULL;
}

std::vector<std::string> ConfigScriptLoader::getScriptsOfType(const Ogre::String &type)
{
    ScriptMap::iterator scripts = scriptListMap.find(type);

    if (scripts != scriptListMap.end())
    {
	return (*scripts).second;
    }

    return std::vector<std::string>();
}

void ConfigScriptLoader::parseScript(DataStreamPtr &stream, const String &groupName)
{
    //Copy the entire file into a buffer for fast access
    parseBuffLen = stream->size();
    parseBuff = new char[parseBuffLen];
    buffPtr = parseBuff;
    stream->read(parseBuff, parseBuffLen);
    parseBuffEnd = parseBuff + parseBuffLen;

    //Close the stream (it's no longer needed since everything is in parseBuff)
    stream->close();

    //Get first token
    _nextToken();
    if (tok == TOKEN_EOF)
	return;

    //Parse the script
    _parseNodes(0);

    if (tok == TOKEN_CloseBrace)
	OGRE_EXCEPT(1, "Parse Error: Closing brace out of place", "ConfigScript::load()");

    //Delete the buffer
    delete[] parseBuff;
}


void ConfigScriptLoader::_nextToken()
{
    lastTok = tok;
    lastTokVal = tokVal;
    lastTokPos = buffPtr;

    //EOF token
    if (buffPtr >= parseBuffEnd){
	tok = TOKEN_EOF;
	return;
    }

    //(Get next character)
    int ch = *buffPtr++;
    while (ch == ' ' || ch == 9){	//Skip leading spaces / tabs
	ch = *buffPtr++;
    }

    //Newline token
    if (ch == '\r' || ch == '\n'){
	do {
	    ch = *buffPtr++;
	} while ((ch == '\r' || ch == '\n') && buffPtr < parseBuffEnd);
	buffPtr--;

	tok = TOKEN_NewLine;
	return;
    }

    //Open brace token
    else if (ch == '{'){
	tok = TOKEN_OpenBrace;
	return;
    }

    //Close brace token
    else if (ch == '}'){
	tok = TOKEN_CloseBrace;
	return;
    }

    //Text token
    if (ch < 32 || ch > 122)	//Verify valid char
	OGRE_EXCEPT(1, "Parse Error: Invalid character", "ConfigScript::load()");

    tokVal = "";
    tok = TOKEN_Text;

    if (ch == '"' || ch == '\'') {
	ch = *buffPtr++; //Skip the " or ' character.
	do {
	    //Skip comments
	    if (ch == '/'){
		int ch2 = *buffPtr;

		//C++ style comment (//)
		if (ch2 == '/'){
		    buffPtr++;
		    do {
			ch = *buffPtr++;
		    } while (ch != '\r' && ch != '\n' && buffPtr < parseBuffEnd);

		    tok = TOKEN_NewLine;
		    return;
		}
	    }

	    //Add valid char to tokVal
	    tokVal += ch;

	    //Next char
	    ch = *buffPtr++;
	} while (ch >= 32 && ch <= 122 && ch != '"' && ch != '\'' && buffPtr < parseBuffEnd);
    }
    else
    {
	do {
	    //Skip comments
	    if (ch == '/'){
		int ch2 = *buffPtr;

		//C++ style comment (//)
		if (ch2 == '/'){
		    buffPtr++;
		    do {
			ch = *buffPtr++;
		    } while (ch != '\r' && ch != '\n' && buffPtr < parseBuffEnd);

		    tok = TOKEN_NewLine;
		    return;
		}
	    }

	    //':' means 'string to end of line', and all next lines starting with ':'.
	    if (ch == ':')
	    {
		again:

		++buffPtr;
		do {
		    ch = *buffPtr++;
		    if (ch != '\r' && ch != '\n')
		    	tokVal += ch;
		} while (ch != '\r' && ch != '\n' && buffPtr < parseBuffEnd);
		
		//(Get next character)
		char *old = (buffPtr - 1);
		ch = *buffPtr++;
		while (ch == ' ' || ch == 9) {	//Skip leading spaces / tabs
		    ch = *buffPtr++;
		}

		if (ch == ':')
		{
		    tokVal += '\n';
		    goto again;
		}

		buffPtr = old;
		return;
	    }

	    //Add valid char to tokVal
	    tokVal += ch;

	    //Next char
	    ch = *buffPtr++;
	} while (ch > 32 && ch <= 122 && buffPtr < parseBuffEnd);
    }

    buffPtr--;

    return;
}

void ConfigScriptLoader::_prevToken()
{
    tok = lastTok;
    tokVal = lastTokVal;
    buffPtr = lastTokPos;
}

void ConfigScriptLoader::_parseNodes(ConfigNode *parent)
{
    typedef std::pair<String, ConfigNode*> ScriptItem;

    while (1) {
	switch (tok){
	    //Node
	    case TOKEN_Text:
		//Add the new node
		ConfigNode *newNode;
		if (parent)
		    newNode = parent->addChild(tokVal);
		else
		    newNode = new ConfigNode(0, tokVal);

		//Get values
		_nextToken();
		while (tok == TOKEN_Text){
		    newNode->addValue(tokVal);
		    _nextToken();
		}

		//Add root nodes to scriptList
		if (!parent){
		    String key;

		    if (newNode->getValues().empty())
		    {
			key = newNode->getName() + ' ';
		    }
		    else
		    {
			String name = newNode->getValues().front();
			String type = newNode->getName();

			key = type + ' ' + name;

			ScriptMap::iterator scripts = scriptListMap.find(type);

			if (scripts == scriptListMap.end())
			{
			    std::vector<std::string> newVector;
			    newVector.push_back(name);
			    scriptListMap.insert(std::pair<std::string, std::vector<std::string> >(type, newVector));
			}
			else
			{
			    ((*scripts).second).push_back(name);
			}
		    }

		    scriptList.insert(ScriptItem(key, newNode));
		}

		//Skip any blank spaces
		while (tok == TOKEN_NewLine)
		    _nextToken();

		//Add any sub-nodes
		if (tok == TOKEN_OpenBrace){
		    //Parse nodes
		    _nextToken();
		    _parseNodes(newNode);

		    //Skip blank spaces
		    while (tok == TOKEN_NewLine)
			_nextToken();

		    //Check for matching closing brace
		    if (tok != TOKEN_CloseBrace)
			OGRE_EXCEPT(1, "Parse Error: Expecting closing brace", "ConfigScript::load()");
		} else {
		    //If it's not a opening brace, back up so the system will parse it properly
		    _prevToken();
		}

		break;

		//Out of place brace
	    case TOKEN_OpenBrace:
		OGRE_EXCEPT(1, "Parse Error: Opening brace out of plane", "ConfigScript::load()");
		break;

		//Return if end of nodes have been reached
	    case TOKEN_CloseBrace:
		return;

		//Return if reached end of file
	    case TOKEN_EOF:
		return;
	}

	//Next token
	_nextToken();
    };
}





ConfigNode::ConfigNode(ConfigNode *parent, const String &name)
{
    ConfigNode::name = name;
    ConfigNode::parent = parent;
    _removeSelf = true;	//For proper destruction
    lastChildFound = -1;

    //Add self to parent's child list (unless this is the root node being created)
    if (parent != NULL){
	parent->children.push_back(this);
	_iter = --(parent->children.end());
    }
}

ConfigNode::~ConfigNode()
{
    //Delete all children
    std::vector<ConfigNode*>::iterator i;
    for (i = children.begin(); i != children.end(); i++){
	ConfigNode *node = *i;
	node->_removeSelf = false;
	delete node;
    }
    children.clear();

    //Remove self from parent's child list
    if (_removeSelf && parent != NULL)
	parent->children.erase(_iter);
}

ConfigNode *ConfigNode::addChild(const String &name, bool replaceExisting)
{
    if (replaceExisting) {
	ConfigNode *node = findChild(name, false);
	if (node)
	    return node;
    }
    return new ConfigNode(this, name);
}

ConfigNode *ConfigNode::findChild(const String &name, bool recursive)
{
    int indx, prevC, nextC;
    int childCount = (int)children.size();

    if (lastChildFound != -1){
	//If possible, try checking the nodes neighboring the last successful search
	//(often nodes searched for in sequence, so this will boost search speeds).
	prevC = lastChildFound-1; if (prevC < 0) prevC = 0; else if (prevC >= childCount) prevC = childCount-1;
	nextC = lastChildFound+1; if (nextC < 0) nextC = 0; else if (nextC >= childCount) nextC = childCount-1;
	for (indx = prevC; indx <= nextC; ++indx){
	    ConfigNode *node = children[indx];
	    if (node->name == name) {
		lastChildFound = indx;
		return node;
	    }
	}

	//If not found that way, search for the node from start to finish, avoiding the
	//already searched area above.
	for (indx = nextC + 1; indx < childCount; ++indx){
	    ConfigNode *node = children[indx];
	    if (node->name == name) {
		lastChildFound = indx;
		return node;
	    }
	}
	for (indx = 0; indx < prevC; ++indx){
	    ConfigNode *node = children[indx];
	    if (node->name == name) {
		lastChildFound = indx;
		return node;
	    }
	}
    } else {
	//Search for the node from start to finish
	for (indx = 0; indx < childCount; ++indx){
	    ConfigNode *node = children[indx];
	    if (node->name == name) {
		lastChildFound = indx;
		return node;
	    }
	}
    }

    //If not found, search child nodes (if recursive == true)
    if (recursive){
	for (indx = 0; indx < childCount; ++indx){
	    children[indx]->findChild(name, recursive);
	}
    }

    //Not found anywhere
    return NULL;
}

void ConfigNode::setParent(ConfigNode *newParent)
{
    //Remove self from current parent
    parent->children.erase(_iter);

    //Set new parent
    parent = newParent;

    //Add self to new parent
    parent->children.push_back(this);
    _iter = --(parent->children.end());
}
