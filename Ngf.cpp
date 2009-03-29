/*
 * =====================================================================================
 *
 *       Filename:  Ngf.cpp
 *
 *    Description:  NGF implementation
 *
 *        Version:  1.0
 *        Created:  10/22/2008 11:29:15 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#include "Ngf.h"

template<> NGF::GameObjectFactory* Ogre::Singleton<NGF::GameObjectFactory>::ms_Singleton = 0;
template<> NGF::GameObjectManager* Ogre::Singleton<NGF::GameObjectManager>::ms_Singleton = 0;
template<> NGF::WorldManager* Ogre::Singleton<NGF::WorldManager>::ms_Singleton = 0;

namespace NGF {

/*
 * =====================================================================================
 * NGF::PropertyList
 * =====================================================================================
 */

    Ogre::String PropertyList::getValue(Ogre::String key, unsigned int index, Ogre::String defaultVal)
    {
	    PropertyList::iterator itr = find(key);
	    if (itr != end())
	    {
                    std::vector<Ogre::String> values = (*itr).second;

		    if (index < values.size())
		    {
			    return values[index];
		    }
	    }

	    return defaultVal;
    }
    //----------------------------------------------------------------------------------   
    PropertyList & PropertyList::addProperty(Ogre::String key, Ogre::String values, 
		    Ogre::String delims)
    {
            std::vector<Ogre::String> vals;
            vals.reserve(10);

            unsigned int numSplits = 0;
            size_t start = 0, pos;
            do 
            {
                    pos = values.find_first_of(delims, start);
                    if (pos == start)
                    {
                            //Do nothing
                            start = pos + 1;
                    }
                    else if (pos == Ogre::String::npos)
                    {
                            //Copy the rest of the string
                            vals.push_back(values.substr(start));
                            break;
                    }
                    else
                    {
                            //Copy up to delimiter
                            vals.push_back(values.substr(start, pos - start));
                            start = pos + 1;
                    }
                    //Parse up to next real data
                    start = values.find_first_not_of(delims, start);
                    ++numSplits;

            } while (pos != Ogre::String::npos);

            insert(PropertyPair(key, vals));
            return *this;
    }
    //----------------------------------------------------------------------------------
    PropertyList PropertyList::create(Ogre::String key, Ogre::String values, Ogre::String delims)
    {
	    NGF::PropertyList props;
	    props.addProperty(key, values, delims);
	    return props;
    }

/*
 * =====================================================================================
 * NGF::GameObject
 * =====================================================================================
 */

    GameObject* GameObject::addFlag(Ogre::String flag)
    {
	    if (mFlags.empty())
	    {
		    mFlags = "|";
	    }
	    mFlags += (flag + "|");

	    return this;
    }
    //----------------------------------------------------------------------------------
    bool GameObject::removeFlag(Ogre::String flag)
    {
	    std::string::size_type pos1 = mFlags.find("|" + flag + "|");

	    if (pos1 == Ogre::String::npos)
	    {
		    return false;
	    }

	    ++pos1;
	    std::string::size_type pos2 = flag.length() + 1;

	    mFlags.erase(pos1, pos2);

	    return true;
    }
    //----------------------------------------------------------------------------------
    bool GameObject::hasFlag(Ogre::String flag) const
    {
	    return !(mFlags.find("|" + flag + "|") == Ogre::String::npos);
    }

/*
 * =====================================================================================
 * NGF::GameObjectFactory
 * =====================================================================================
 */

    GameObjectFactory& GameObjectFactory::getSingleton(void)
    {
	    assert(ms_Singleton); return *ms_Singleton;
    }
    GameObjectFactory* GameObjectFactory::getSingletonPtr(void)
    {
	    return ms_Singleton;
    }
    //----------------------------------------------------------------------------------
    GameObject* GameObjectFactory::createObject(Ogre::String type, Ogre::Vector3 pos, Ogre::Quaternion rot, PropertyList props, Ogre::String name)
    {
	    CreateFunctionMap::iterator iter = mCreateFunctions.find(type);

	    if (iter != mCreateFunctions.end())
		    return ((*iter).second)(pos, rot, props, name); //Found.
	    return 0; //Not found.
    }

/*
 * =====================================================================================
 * NGF::GameObjectManager
 * =====================================================================================
 */

    GameObjectManager* GameObjectManager::getSingletonPtr(void)
    {
	    return ms_Singleton;
    }
    GameObjectManager& GameObjectManager::getSingleton(void)
    {
	    assert(ms_Singleton); return *ms_Singleton;
    }
    //----------------------------------------------------------------------------------
    GameObjectManager::GameObjectManager()
	    : mCurrentIDNo(0),
	    mObjectFactory(new GameObjectFactory())
    {
    }
    //----------------------------------------------------------------------------------
    void GameObjectManager::tick(bool paused, const Ogre::FrameEvent & evt)
    {
	    std::map<ID,GameObject*>::iterator objIter;

	    for (objIter = mGameObjectMap.begin();
			    objIter != mGameObjectMap.end(); ++objIter)
	    {
		    GameObject *obj = objIter->second;

		    if (paused)
		    {
			    obj->pausedTick(evt);
		    }
		    else
		    {
			    obj->unpausedTick(evt);
		    }
	    }

	    std::vector<ID>::iterator iter;

	    for (iter = mObjectsToDestroy.begin();
			    iter != mObjectsToDestroy.end(); ++iter)
	    {
		    destroyObject(*iter);
	    }
	    mObjectsToDestroy.clear();
    }
    //----------------------------------------------------------------------------------
    bool GameObjectManager::destroyObject(ID objID)
    {
	    std::map<ID,GameObject*>::iterator objIter = mGameObjectMap.find(objID);

	    if (objIter == mGameObjectMap.end())
	    {
		    return false;
	    }
	    else
	    {
		    mUnusedIDs.push_back(objIter->first);
		    GameObject *obj = objIter->second;

		    mGameObjectMap.erase(objIter);
		    obj->destroy(); //For scripting, as scripting languages are GCed.
		    delete obj;

		    return true;
	    }
    }
    //----------------------------------------------------------------------------------
    void GameObjectManager::destroyAll(void)
    {
	    std::map<ID,GameObject*>::iterator objIter;

	    for (objIter = mGameObjectMap.begin();
			    objIter != mGameObjectMap.end(); ++objIter)
	    {
		    delete objIter->second;
	    }

	    mGameObjectMap.clear();
	    mCurrentIDNo = 0;
	    mUnusedIDs.clear();
    }
    //----------------------------------------------------------------------------------
    GameObject* GameObjectManager::getByID(ID objID) const
    {
	    std::map<ID,GameObject*>::const_iterator objIter = mGameObjectMap.find(objID);

	    return (objIter == mGameObjectMap.end()) ? NULL : objIter->second;
    }
    //----------------------------------------------------------------------------------
    void GameObjectManager::forEachGameObject(ForEachFunction func)
    {
	    std::map<ID,GameObject*>::iterator objIter;

	    for (objIter = mGameObjectMap.begin();
			    objIter != mGameObjectMap.end(); ++objIter)
	    {
		    func(objIter->second);
	    }
    }
    //----------------------------------------------------------------------------------
    void GameObjectManager::sendMessage(GameObject *obj, Message msg) const
    {
	    if (obj)
	    {
		    obj->receiveMessage(msg);
	    }
    }
    //----------------------------------------------------------------------------------
    GameObject* GameObjectManager::getByName(Ogre::String name)
    {
	    GameObject* findObj = NULL;
	    std::map<ID,GameObject*>::iterator objIter;

	    for (objIter = mGameObjectMap.begin();
			    objIter != mGameObjectMap.end(); ++objIter)
	    {
		    GameObject *obj = objIter->second;
		    if (obj->getName() == name)
		    {
			    findObj = obj;
			    break;
		    }
	    }

	    return findObj;
    }

/*
 * =====================================================================================
 * NGF::WorldManager
 * =====================================================================================
 */

    WorldManager& WorldManager::getSingleton(void)
    {
	    assert(ms_Singleton); return *ms_Singleton;
    }
    WorldManager* WorldManager::getSingletonPtr(void)
    {
	    return ms_Singleton;
    }
    //----------------------------------------------------------------------------------
    WorldManager::WorldManager()
    {
	    shuttingdown = false;
	    stoppedLast = false;
    }
    //----------------------------------------------------------------------------------
    WorldManager::~WorldManager()
    {
	    std::vector<World*>::iterator iter;
	    for (iter = worlds.begin(); iter!= worlds.end(); ++iter)
	    {
		    delete *iter;
	    }
	    worlds.clear();
    }
    //----------------------------------------------------------------------------------
    void WorldManager::shutdown(void)
    {
	    shuttingdown = true;
    }
    //----------------------------------------------------------------------------------
    bool WorldManager::tick(const Ogre::FrameEvent &evt)
    {
	    if (!shuttingdown)
	    {
		    worlds[currentWorld]->tick(evt);
		    return true;
	    }
	    else
	    {
		    if (!stoppedLast)
		    {
			    worlds[currentWorld]->stop();
			    stoppedLast = true;
		    }

		    return false;
	    }
    }
    //----------------------------------------------------------------------------------
    void WorldManager::addWorld(World *newWorld)
    {
	    worlds.push_back(newWorld);
    }
    //----------------------------------------------------------------------------------
    void WorldManager::start(unsigned int firstWorld)
    {
	    if (firstWorld <= worlds.size() && firstWorld != -1)
	    {
		    currentWorld = firstWorld;
		    worlds[currentWorld]->init();
	    }
	    else
	    {
		    OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Bad world index given", "NGF::WorldManager::start()");
	    }
    }
    //----------------------------------------------------------------------------------
    void WorldManager::nextWorld(void)
    {
	    if ((currentWorld + 1) < worlds.size())
	    {
		    worlds[currentWorld]->stop();
		    worlds[++currentWorld]->init();
	    }
	    else
	    {
		    shutdown();
	    }
    }
    //----------------------------------------------------------------------------------
    bool WorldManager::previousWorld(void)
    {
	    if(currentWorld != 0)
	    {
		    worlds[currentWorld]->stop();
		    worlds[--currentWorld]->init();

		    return true;
	    }
	    return false;
    }
    //----------------------------------------------------------------------------------
    void WorldManager::gotoWorld(unsigned int worldNumber)
    {
	    if (worldNumber < worlds.size() && worldNumber != -1)
	    {
		    worlds[currentWorld]->stop();
		    worlds[(currentWorld = worldNumber)]->init();
	    }
	    else
	    {
		    OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Bad world index given", "NGF::WorldManager::gotoWorld()");
	    }
    }
    //----------------------------------------------------------------------------------
    unsigned int WorldManager::getCurrentWorldIndex()
    {
	    return currentWorld;
    }

/*
 * =====================================================================================
 * NGF::Loading::Loader
 * =====================================================================================
 */

    Loading::Loader::Loader(LoaderHelperFunction help)
    {
	    mHelper = help;
	    mUseFactory = (help == NULL);
	    mGameMgr = GameObjectManager::getSingletonPtr();
	    mScriptLoader = new ConfigScriptLoader("*.ngf");
    }
    //----------------------------------------------------------------------------------
    Loading::Loader::~Loader()
    {
	    delete mScriptLoader;
    }
    //----------------------------------------------------------------------------------
    void Loading::Loader::loadLevel(Ogre::String levelname, Ogre::Vector3 displace, Ogre::Quaternion rotate)
    {
	    //Get the script and its children (the objects).
	    ConfigNode *lvl = mScriptLoader->getConfigScript("ngflevel", levelname);

	    if (!lvl)
	    {
		    OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "NGF level not found!", "NGF::Loading::Loader::loadNGF()");
		    return;
	    }

	    std::vector<ConfigNode*> objs = lvl->getChildren();

	    //Iterate through the children and do stuff.
	    for (std::vector<ConfigNode*>::iterator i = objs.begin(); i != objs.end(); ++i)
	    {
		    ConfigNode *obj = (*i);

		    //Get the type and name.
		    Ogre::String type = obj->findChild("type")->getValues()[0];
		    Ogre::String name = obj->findChild("name")->getValues()[0];

		    //Get the position.
		    std::vector<Ogre::String> posCoord = obj->findChild("position")->getValues();

		    Ogre::Vector3 pos(Ogre::StringConverter::parseReal(posCoord[0]),
				    Ogre::StringConverter::parseReal(posCoord[1]),
				    Ogre::StringConverter::parseReal(posCoord[2]));

		    //Displace it accordingly.
		    pos = rotate * pos;
		    pos += displace;

		    //Get the rotation.
		    std::vector<Ogre::String> rotCoord = obj->findChild("rotation")->getValues();

		    Ogre::Quaternion rot( Ogre::StringConverter::parseReal(rotCoord[0]),
				    Ogre::StringConverter::parseReal(rotCoord[1]),
				    Ogre::StringConverter::parseReal(rotCoord[2]),
				    Ogre::StringConverter::parseReal(rotCoord[3]));

		    //Displace it accordingly.
		    rot = rot * rotate;

		    //Since there are property keys and each key has more than one value, we have a lot to do.
		    PropertyList properties;
		    ConfigNode *propNode = obj->findChild("properties");

		    //Some objects might not store properties.
		    if (propNode)
		    {
			    //We get the keys and iterate through them.
			    std::vector<ConfigNode*> props = propNode->getChildren();

			    for (std::vector<ConfigNode*>::iterator j = props.begin(); j != props.end(); ++j)
			    {
				    //Put the key and its values in the map. Luckily, a ConfigNode can return an std::vector
				    //containing all its values, so we don't have to iterate through them.
				    ConfigNode *prop = (*j);
				    properties.insert(PropertyPair(prop->getName(), prop->getValues()));
			    }
		    }
		    else
		    {
			    //Use an empty property list in case the object doesn't have any properties.
			    properties = PropertyList();
		    }

		    //Call the callback function.
		    if (mUseFactory)
			    mGameMgr->createObject(type, pos, rot, properties, name);
		    else
			    mHelper(type, name, pos, rot, properties);
	    }
    }
    //----------------------------------------------------------------------------------
    std::vector<Ogre::String> Loading::Loader::getLevels()
    {
	    return mScriptLoader->getScriptsOfType("ngflevel");
    }

} //namespace NGF
