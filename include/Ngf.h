/*
 * =====================================================================================
 *
 *       Filename:  Ngf.h
 *
 *    Description:  The NGF framework header.
 *
 *        Version:  1.0
 *        Created:  10/22/2008 11:32:56 AM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

//----------------------------------------------------------------------------------
//Define one of the following, for the respective physics features.
//#define NGF_USE_OGREODE
//#define NGF_USE_OGREBULLET
#define NGF_USE_BULLET
//----------------------------------------------------------------------------------

#ifndef _NGF_H_
#define _NGF_H_

#include <map>
#include <vector>

#include "OgreSingleton.h"
#include "OgreException.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreQuaternion.h"
#include "OgreFrameListener.h"
#include "OgreResourceGroupManager.h"
#include "OgreUserDefinedObject.h"
#include "OgreColourValue.h"

#ifdef NGF_USE_OGREBULLET
#define _PRECOMP
#include "OgreBulletCollisions.h"
#include "OgreBulletDynamics.h"
#endif

#ifdef NGF_USE_OGREODE
#include "OgreOde_Core.h"
#endif

#ifdef NGF_USE_BULLET
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionDispatch/btManifoldResult.h"
#include "BulletCollision/NarrowPhaseCollision/btManifoldPoint.h"
#endif

#include "boost/any.hpp"

#include "ConfigScript.h"
#include "FastDelegate.h"

//Send a reply. Used inside a GameObject::receiveMessage() function.
#define NGF_SEND_REPLY(value) {NGF::GameObjectManager::getSingletonPtr()->setReply(value);}

//Register a GameObject type. For easy registration if you're too lazy to type both the
//typename and a string. ;-)
#define NGF_REGISTER_OBJECT_TYPE(type) NGF::GameObjectFactory::getSingleton().registerObjectType< type >( #type )

namespace NGF {

/*
 * =====================================================================================
 *        Class: PropertyList
 *  Description: A map of Strings and StringVectors.
 * =====================================================================================
 */

typedef std::pair<Ogre::String, std::vector<Ogre::String> > PropertyPair;

class PropertyList : public std::map<Ogre::String, std::vector<Ogre::String> >
{
public:
	PropertyList() { }

	//Get a value. It returns defaultVal if the key isn't found or the index is out of bounds.
	Ogre::String getValue(Ogre::String key, unsigned int index, Ogre::String defaultVal);
    
	//Add a property. Specify the key, and the values (seperated by delemiters specified in delims).
	//You can chain this method like so: props.addProperty(x,y).addProperty(a,b).addProperty(m,n).
	PropertyList & addProperty(Ogre::String key, Ogre::String values, Ogre::String delims = " ");

	//Allows you to quickly create a new PropertyList. Same parameters as 'addProperty',
	//but just creates a new PropertyList instead of adding to an existing one.
	static PropertyList create(Ogre::String key, Ogre::String values, Ogre::String delims = " ");
};

/*
 * =====================================================================================
 *       Struct: Message
 *  Description: This struct represents the Messages that are sent between the
 *               GameObjects. It uses the MessageParams typedef, which allows you to
 *               send any number of copy-constructible (Vector3, Quaternion etc) objects
 *		 in a message. This requres boost.
 * =====================================================================================
 */

typedef std::vector<boost::any> MessageParams;

struct Message
{
protected:
	Ogre::String mName;
	MessageParams mParams;

public:
	//Create a Message with the given subject, body and parameters.
	Message(Ogre::String name, MessageParams parameters = MessageParams())
	    : mName(name),
	      mParams(parameters)
	{
	}

	//These methods allow you to get the subject, body and parameters respectively.
	
	Ogre::String getName() { return mName; }
	template<typename T> T getParam(int index) { return boost::any_cast<T>(mParams[index]); }

	//So you can do:
	// gom->sendMessage(obj, (NGF::Message("printStuff"), Vector3(10,20,30), Quaternion(1,2,3,4)));
	template<typename T> Message& operator,(T thing) { mParams.push_back(boost::any(thing)); return *this; }
};

/*
 * =====================================================================================
 *        Class: GameObject
 *  Description: This class acts as a base class for an object in the game. This class 
 *  		 is one of the most important classes since the NGF system was chosen to
 *  		 be object-centric.
 * =====================================================================================
 */

//Each GameObject has a unique ID.
typedef unsigned int ID;

class GameObject : public Ogre::UserDefinedObject
{
	ID mID;
	Ogre::String mType;
	Ogre::String mFlags;
	Ogre::String mName;

	friend class GameObjectManager;

protected:
	PropertyList mProperties;

public:
	//------ Called by the Framework, to be overridden --------

	//Called on creation.
	GameObject(Ogre::Vector3, Ogre::Quaternion, ID id, PropertyList properties = PropertyList(), Ogre::String name = "")
	    : mID(id),
	      mName(name),
	      mProperties(properties),
	      mType("NGF::GameObject")
	{
	}

	//Called on destruction.
	virtual ~GameObject() { }

	//Called every unpaused frame.
	virtual void unpausedTick(const Ogre::FrameEvent& evt ) { }

	//Called every paused frame.
	virtual void pausedTick(const Ogre::FrameEvent& evt) { }

	//Called when a message is received.
	virtual void receiveMessage(Message msg) { }

#ifdef NGF_USE_OGREODE
	//Called on collision with a physics object. If you are using OgreOde, define
	//NGF_USE_OGREODE before including NGF, and register the GameObject to get 
	//this functionality.
	//
	//other: The GameObject collided with.
	//otherGeom: The OgreOde::Geometry collided with.
	//contact: The contect joint.
	virtual bool collide(GameObject *other, OgreOde::Geometry *otherGeom, OgreOde::Contact *contact) { return true; }
#endif
#ifdef NGF_USE_OGREBULLET
	//Called on collision with a physics object. If you are using OgreBullet, define
	//NGF_USE_OGREBULLET before including NGF, and register the GameObject to get 
	//this functionality.
	//
	//other: The GameObject collided with.
	//otherPhysicsObject: The OgreBulletCollisions::Object collided with.
	//contact: The mainfold contact point.
	virtual void collide(GameObject *other, OgreBulletCollisions::Object *otherPhysicsObject, btManifoldPoint &contact) { }
#endif
#ifdef NGF_USE_BULLET
	//Called on collision with a physics object. If you are using (just) Bullet, define
	//NGF_USE_BULLET before including NGF, and register the GameObject to get 
	//this functionality.
	//
	//other: The GameObject collided with.
	//otherPhysicsObject: The btCollisionObject collided with.
	//contact: The mainfold contact point.
	virtual void collide(GameObject *other, btCollisionObject *otherPhysicsObject, btManifoldPoint &contact) { }
#endif

	//Called on destruction (for scripted objects, as they are GCed).
	virtual void destroy(void) { }

	//------ Called by other objects, and not overridden ------

	//Returns the ID of the  GameObject.
	ID getID(void) const { return mID; }

	//Returns the name of the  GameObject.
	Ogre::String getName(void) const { return mName; }

	//Returns the properties of the GameObject.
	PropertyList getProperties(void) const { return mProperties; }

	//Adds a flag to the GameObject's flags.
	GameObject* addFlag(Ogre::String flag);

	//Removes a flag from the GameObject's flags. Returns whether it was found.
	bool removeFlag(Ogre::String flag);

	//Checks whether the GameObject has a flag.
	bool hasFlag(Ogre::String flag) const;

	//Returns the flags string.
	Ogre::String getFlags() const { return mFlags; }

	//------ Ogre UserDefinedObject stuff ------
	long getTypeID() const { return 1021993; }

	const Ogre::String& getTypeName(void) const { return mType; }
};

/*
 * =====================================================================================
 *        Class: GameObjectactory
 *  Description: Manages creation of GameObjects with the type as a string. This is
 *  		 useful for creating GameObjects by parsing a text file, or in a
 *  		 scripting language, for instance.
 * =====================================================================================
 */

class GameObjectFactory : public Ogre::Singleton<GameObjectFactory>
{
protected:
	typedef std::map<Ogre::String, fastdelegate::FastDelegate< 
		GameObject* (Ogre::Vector3 , Ogre::Quaternion , PropertyList, Ogre::String) > > 
		CreateFunctionMap;
	CreateFunctionMap mCreateFunctions;

public:
	//Register a GameObject type. Give the class as the template parameter, and the
	//string name of the type as the string parameter. You can then use
	//GameObjectManager::createObject to create an object of this type by passing a
	//string.
	template<typename T>
	void registerObjectType(Ogre::String type);

	//Create an object with the given type as a string. The type should be registered. 
	//Use GameObjectManager::createObject instead for consistency. This is similar to 
	//GameObjectManager::createObject's template version, just the way the type is 
	//specified is different.
	NGF::GameObject *createObject(Ogre::String type, Ogre::Vector3 pos, 
			Ogre::Quaternion rot, PropertyList props, Ogre::String name);
		
	//------ Singleton functions ------------------------------
	
	static GameObjectFactory& getSingleton(void);
	static GameObjectFactory* getSingletonPtr(void);
};


/*
 * =====================================================================================
 *        Class: GameObjectManager
 *  Description: Manages and handles all GameObjects, and notifies them of
 *  		 events, including collision events.
 * =====================================================================================
 */

class GameObjectManager
#ifdef NGF_USE_OGREODE
	: public OgreOde::CollisionListener , public Ogre::Singleton<NGF::GameObjectManager>
#else
	: public Ogre::Singleton<NGF::GameObjectManager>
#endif
{
protected:
	std::map<ID,GameObject*> mGameObjectMap;
	GameObjectFactory *mObjectFactory;

	std::vector<ID> mUnusedIDs;
	std::vector<ID> mObjectsToDestroy;

	int mCurrentIDNo;

	boost::any mReply;

#ifdef NGF_USE_OGREBULLET
	OgreBulletCollisions::CollisionsWorld *mPhysicsWorld;
#endif

public:

	typedef fastdelegate::FastDelegate1<GameObject*> ForEachFunction;

	//------ Constructor/Destructor ---------------------------
	
#ifdef NGF_USE_OGREODE
	GameObjectManager(OgreOde::World *world);
#else
#ifdef NGF_USE_OGREBULLET
	GameObjectManager(OgreBulletCollisions::CollisionsWorld *world);
#else
	GameObjectManager();
#endif
#endif

	~GameObjectManager() { destroyAll(); delete mObjectFactory; }

	//------ Tick function ------------------------------------

	//Updates the GameObjects. Should be called per frame. Tell it whether
	//the game is paused, and pass it the Ogre::FrameEvent.
	void tick(bool paused, const Ogre::FrameEvent & evt);

	//------ Singleton functions ------------------------------

	static GameObjectManager* getSingletonPtr(void);
	static GameObjectManager& getSingleton(void);

	//------ Create/Destroy functions -------------------------

	//Creates a GameObject of the given type. Returns a pointer to the GameObject created.
	//Give name "noname" if you want the GameObject to not have a name.
	template<typename T>
	GameObject* createObject(Ogre::Vector3 pos, Ogre::Quaternion rot, PropertyList properties = PropertyList(), 
		Ogre::String name = "");

	//Creates a GameObject of the given type as a string. Returns a pointer to the 
	//GameObject created. Give name "noname" if you want the GameObject to not have a name.
	GameObject* createObject(Ogre::String type, Ogre::Vector3 pos, Ogre::Quaternion rot, 
		PropertyList properties = PropertyList(), Ogre::String name = "")
	{
		return mObjectFactory->createObject(type, pos, rot, properties, name);
	}

	//Destroys the GameObject with the given ID.
	//Returns false if it was not found and true if it was.
	bool destroyObject(ID objID);

	//Requests the GameObjectManager to destroy a GameObject soon. This is needed
	//if an object wants to destroy itself, or for other crazy situations.
	void requestDestroy(ID objID) { mObjectsToDestroy.push_back(objID); }

	//Destroys all the GameObjects that exist.
	void destroyAll(void);

	//------ Miscellaneous functions --------------------------

	//Returns a pointer to the GameObject with the given ID. If it was
	//not found, a NULL pointer will be returned.
	GameObject* getByID(ID objID) const;

	//Returns a pointer to the GameObject with the given name. If not found,
	//a NULL pointer is returned.
	GameObject* getByName(Ogre::String name);

	//Calls the function passed for each GameObject that exists. One argument
	//is passed to that function, which is the GameObject. Quite useful if
	//used with boost::lambda.
	void forEachGameObject(ForEachFunction func);

	//------ Messaging functions ------------------------------

	//This function sends a message to the GameObject.
	void sendMessage(GameObject *obj, Message msg) const;

	//This function sends the message, and gives a reply. GameObjects can reply using 
	//NGF_SEND_REPLY(reply) in a GameObject::receiveMessage() function.
	template<class ReturnType>
	ReturnType sendMessageWithReply(GameObject *obj, Message msg);

	//This function should be called by the GameObject while receiving
	//a message if the sender expects a reply. If no reply is sent,
	//the sender will get an empty boost::any.
	//
	//You can use NGF_SEND_REPLY(reply) instead.
	void setReply(boost::any arg) { mReply = arg; }

	//------ Collision event functions ------------------------

#ifdef NGF_USE_OGREODE
	//Tell the GameObjectManager about the relation between a GameObject
	//and an OgreOde::Geometry.
	void registerForCollision(GameObject *object, OgreOde::Geometry *geom);

	//Get the GameObject associated with the specified OgreOde::Geometry.
	GameObject* getObjectFromGeometry(OgreOde::Geometry *geom);

	//------ OgreOde collision callback (internal stuff) ------

	//OgreOde collision callback, don't call this function yourself.
	bool collision(OgreOde::Contact *contact);
#endif
#ifdef NGF_USE_OGREBULLET
	//Tell the GameObjectManager about the relation between a GameObject
	//and an OgreBulletCollisions::Object.
	void registerForCollision(GameObject *object, OgreBulletCollisions::Object *physic);

	//Get the GameObject associated with the specified OgreBulletCollisions::Object.
	GameObject* getObjectFromPhysicsObject(OgreBulletCollisions::Object *physic);

	//------ OgreBullet collision callbacks (internal stuff) --
	
	//The collision callbacks from Bullet.
	bool _contactAdded(btManifoldPoint& cp, const btCollisionObject* colObj0, int partId0, 
			int index0, const btCollisionObject* colObj1, int partId1, int index1);
	bool _contactDestroyed(btManifoldPoint& cp, void* body0, void* body1);

	//The actual collision callbacks. Bullet doesn't accept non-static member functions 
	//as callbacks, so we need to do this.
	static bool _contactAddedCallback(btManifoldPoint& cp,const btCollisionObject* colObj0, 
			int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1);
	static bool _contactDestroyedCallback(btManifoldPoint& cp,void* body0,void* body1);
#endif
#ifdef NGF_USE_BULLET
	//Tell the GameObjectManager about the relation between a GameObject
	//and an OgreBulletCollisions::Object.
	void registerForCollision(GameObject *object, btCollisionObject *physic);

	//Get the GameObject associated with the specified OgreBulletCollisions::Object.
	GameObject* getObjectFromPhysicsObject(btCollisionObject *physic);

	//------ OgreBullet collision callbacks (internal stuff) --
	
	//The collision callbacks from Bullet.
	bool _contactAdded(btManifoldPoint& cp, const btCollisionObject* colObj0, int partId0, 
			int index0, const btCollisionObject* colObj1, int partId1, int index1);
	bool _contactDestroyed(btManifoldPoint& cp, void* body0, void* body1);

	//The actual collision callbacks. Bullet doesn't accept non-static member functions 
	//as callbacks, so we need to do this.
	static bool _contactAddedCallback(btManifoldPoint& cp,const btCollisionObject* colObj0, 
			int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1);
	static bool _contactDestroyedCallback(btManifoldPoint& cp,void* body0,void* body1);
#endif

};

/*
 * =====================================================================================
 *        Class: World
 *  Description: This class acts as a base class for all Worlds (levels).
 * =====================================================================================
 */

class World
{
public:
	//The World constructor. This is called when the World is constructed, not when
	//it is run (look for World::init).
	World() { }

	//Called when the World is destroyed. Usually when the WorldManager is destroyed.
	//This is not called when the World ends (look for World::stop);
	virtual ~World() { }

	//Called when the World is run.
	virtual void init(void) { }

	//Called every frame the World is running.
	virtual void tick(const Ogre::FrameEvent &evt) { }

	//Called when the World stops running, that is, when we switch to a different World.
	virtual void stop(void) { }
};

/*
 * =====================================================================================
 *        Class: WorldManager
 *  Description: This class manages the Worlds, and plays them in order.
 * =====================================================================================
 */

class WorldManager : public Ogre::Singleton<NGF::WorldManager>
{
protected:
	unsigned int currentWorld;
	std::vector<World*> worlds;
	bool shuttingdown;
	bool stoppedLast;

public:
	WorldManager();
	~WorldManager();

	//Shutdown. Doesn't really shutdown, it just makes it return false
	//from the tick function, you gotta do the actual shutdown yourself.
	void shutdown();

	//Tick function. Call it every frame. Shutdown if it returns false.
	bool tick(const Ogre::FrameEvent &evt);

	//Add a world to the list. You can just call addWorld(new MyWorld()), and
	//not have to manually call delete worldPointer. All added Worlds are
	//automatically deleted by the WorldManager when it is destroyed.
	void addWorld(World *newWorld);
	
	//Start playing the worlds.
	void start(unsigned int firstWorld);

	//Go to the next world.
	void nextWorld();

	//Go to the previous world.
	bool previousWorld();

	//Jump to the specified world.
	void gotoWorld(unsigned int worldNumber);

	//Get the index number of the currently playing world.
	unsigned int getCurrentWorldIndex();

	//------ Singleton functions ------------------------------
	
	static WorldManager& getSingleton(void);
	static WorldManager* getSingletonPtr(void);
};

namespace Loading {

//The LoaderHelperFunctions. I know, I should have called it LoaderCallbackFunction.
//At that time, I didn't think of 'callback' for some reason (probably wasn't very familiar with programming jargon),
//and the name stuck. 
typedef fastdelegate::FastDelegate<void (Ogre::String,Ogre::String,Ogre::Vector3,Ogre::Quaternion,PropertyList) > LoaderHelperFunction;

/*
 * =====================================================================================
 *        Class: Loader
 *  Description: This class parses .ngf files and calls the callback function
 *               accordingly. If the type of object is 'Mesh', it makes a static mesh
 *		 with the given meshfile (this feature removed).
 * =====================================================================================
 */

class Loader
{
protected:
	LoaderHelperFunction mHelper;
	GameObjectManager *mGameMgr;
	ConfigScriptLoader *mScriptLoader;

	bool mUseFactory;

public:
	//Create the loader. Give it a pointer to the helper function, or NULL (0) if you want it to use the 
	//GameObjectFactory (through GameObjectManager::createObject(<string>, ...)).
	Loader(LoaderHelperFunction help);

	~Loader();

	//Whether to use factory or not. If no, you provide the callback (helper) function. Otherwise,
	//we use the GameObjectFactory (through GameObjectManager::createObject(<string>, ...)).
	void useFactory(bool use) { mUseFactory = use; }

	//Loads an NGF level. Give it the name of the level in the '.ngf' script. You can also give an additional
	//positional displacement and rotation. This can be useful for loading when a level is already loaded, to
	//'add on' to the existing level.
	void loadLevel(Ogre::String levelname, Ogre::Vector3 displace = Ogre::Vector3::ZERO, Ogre::Quaternion rotate = Ogre::Quaternion::IDENTITY);

	//Returns a vector containing the level names of all the levels parsed. Returns an empty vector if no levels
	//were found.
	Ogre::StringVector getLevels();
};

} //namespace Loading

//------ Some useful utility stuff ----------------------------

//InlineVector allows this:
//
//addNumbers(InlineVector<int>(3)(5)(10)); //Returns 18
//
//Copied from ajs' wonderful 'Navi' library. Thanks a lot, ajs! ;-)
template <class T>
class InlineVector : public std::vector<T>
{
public:
	InlineVector() { }

	explicit InlineVector(const T &firstArg) : std::vector<T>(1, firstArg) { }

	InlineVector& operator()(const T &newArg)
	{
		this->push_back(newArg);
		return *this;
	}
};

class  Utility
{
public:
	//Set number (or anything that defines operator < and >) in the specified range.
	template<class Type>
	static Type Clamp(Type number, Type rangeMin, Type rangeMax)
	{
		Type big = (number < rangeMax) ? number : rangeMax;
		return (rangeMin > big) ? rangeMin : big;
	}

	//Create an Ogre::ColourValue from a 0 to 255 colour value.
	static inline Ogre::ColourValue Colour(Ogre::Real r, Ogre::Real g, Ogre::Real b, Ogre::Real a = 255)
	{
		return Ogre::ColourValue(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
	}

	//Calculate attenuation from distance. Kind of uses Blender's mechanism.
	static Ogre::Vector4 CalculateAttenuation(Ogre::Real distance, Ogre::Real energy, Ogre::Real quad1, Ogre::Real quad2)
	{
		return Ogre::Vector4(distance * energy * 20, 1/energy, quad1/(energy * distance), quad2/(energy * distance * distance));
	}
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+ End of the 'actual' header file, template function definitions follow +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

template<typename T>
void GameObjectFactory::registerObjectType(Ogre::String type)
{
	mCreateFunctions[type] = fastdelegate::MakeDelegate(GameObjectManager::getSingletonPtr(), &GameObjectManager::createObject<T>);
}
//--------------------------------------------------------------------------------------
template<typename T>
GameObject* GameObjectManager::createObject(Ogre::Vector3 pos, Ogre::Quaternion rot, 
	PropertyList properties, Ogre::String name)
{
	//Calculate ID to assign.
	ID objID;
	if (!(mUnusedIDs.empty()))
	{
		objID = mUnusedIDs[mUnusedIDs.size() - 1];
		mUnusedIDs.pop_back();
	}
	else
	{
		objID = mCurrentIDNo++;
	}

	//No name.
	name = ((name == "noname" ) ? "" : name);

	//Check if name is already used.
	if (name != "")
	{
		std::map<ID,GameObject*>::iterator objIter;

		for (objIter = mGameObjectMap.begin();
				   objIter != mGameObjectMap.end(); ++objIter)
		{
			GameObject *obj = objIter->second;
			if (obj->getName() == name)
			{
				OGRE_EXCEPT(Ogre::Exception::ERR_DUPLICATE_ITEM, "GameObject with name'" 
					+ name + "' already exists!", "NGF::GameObjectManager::createObject()");
			}
		}
	}

	//Create object.
	T *obj = new T(pos, rot, objID, properties, name);

	//Check if name and ID was correctly passed.
	if ((obj->getID() != objID) || (obj->getName() != name))
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, "Incorrect name or ID passed for GameObject with ID: " 
			+ Ogre::StringConverter::toString(obj->getID()) + ", and name: '" + obj->getName() 
			+ "'.", "NGF::GameObjectManager::createObject()");
	}

	//Put in map.
	mGameObjectMap.insert(std::pair<ID,GameObject*>(objID, obj));

	return obj;
}
//--------------------------------------------------------------------------------------
template<typename ReturnType>
ReturnType GameObjectManager::sendMessageWithReply(GameObject *obj, Message msg)
{
	if (obj)
	{
		ReturnType retVal;
		mReply = boost::any();
		obj->receiveMessage(msg);

		try
		{
			retVal =  boost::any_cast<ReturnType>(mReply);
		}
		catch (boost::bad_any_cast)
		{
			OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, "Bad ReturnType, or no reply!", "NGF::GameObjectManager::sendMessageWithReply()");
		}

		return retVal;
	}
	OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, "GameObject doesn't exist!", "NGF::GameObjectManager::sendMessageWithReply()");
}

} //namespace NGF

#endif //#define _NGF_H_
