//---------------------------------------------------------------------------------------
// PLAYER.H
//---------------------------------------------------------------------------------------

class Player : public NGF::PythonGameObject
{
protected:
	SceneNode *mNode;
	Entity *mEntity;

	btRigidBody *mBody;
	btCollisionShape *mShape;

	Real mTime;

	float mHealth;

public:
	Player(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, NGF::PropertyList properties, String name) : NGF::PythonGameObject(pos, rot, id , properties, name)
	{
		String idStr = StringConverter::toString(id);
		addFlag("Player");
		mTime = 0;
		
		//Set up script.
		setUpScript(properties.getValue("script", 0, ""));

		//Call init event in Python.
		mPyEvents["init"](mConnector);

		//Create Ogre stuff.
		mEntity = Globals::smgr->createEntity(idStr + "-playerEntity", "Player.mesh");
		mNode = Globals::smgr->getRootSceneNode()->createChildSceneNode(idStr + "playerSceneNode", pos, rot);
		mNode->attachObject(mEntity);

		//Create shape.
		BtOgre::StaticMeshToShapeConverter converter(mEntity);
		mShape = converter.createSphere();

		//Calculate inertia.
		btScalar mass = 5;
		btVector3 inertia;
		mShape->calculateLocalInertia(mass, inertia);

		//Create BtOgre MotionState (connects Ogre and Bullet).
		BtOgre::RigidBodyState *State = new BtOgre::RigidBodyState(mNode);

		//Create the Body.
		mBody = new btRigidBody(mass, State, mShape, inertia);
		Globals::phyWorld->addRigidBody(mBody);

		//Registar!
		Globals::gom->registerForCollision(this, mBody);

		//Print properties.
		for (NGF::PropertyList::iterator i = properties.begin(); i != properties.end(); ++i)
		{
			LogManager::getSingleton().logMessage("Property: " + (*i).first);
		}

		LogManager::getSingleton().logMessage(properties.getValue("ngf-evt-collide", 0, ""));
		LogManager::getSingleton().logMessage(properties.getValue("ngf-evt-create", 0, ""));

		//Test.
		mHealth = 777;

		//Call create event in Python.
		mPyEvents["create"](mConnector);

		LogManager::getSingleton().logMessage("Health is: " + StringConverter::toString(mHealth));
	}

	~Player()
	{
		//Script destroy.
		mPyEvents["destroy"](mConnector);

		//Destory physics stuff.
		delete mShape;
		delete mBody;

		//Destroy Ogre stuff.
		mNode->detachAllObjects();
		Globals::smgr->destroyEntity(mEntity);
		Globals::smgr->destroySceneNode(mNode->getName());
	}

	void unpausedTick(const Ogre::FrameEvent &evt)
	{
		mPyEvents["utick"](mConnector, evt.timeSinceLastFrame);
	}

	void pausedTick(const Ogre::FrameEvent &evt)
	{
		mPyEvents["ptick"](mConnector, evt.timeSinceLastFrame);
	}

	void receiveMessage(NGF::Message msg)
	{
		LogManager::getSingleton().logMessage("Player got message:\nSubject: " + msg.getSubject() + "\nBody: " + msg.getBody());
	}

	void collide(GameObject *other, btCollisionObject *otherPhysicsObject, btManifoldPoint &contact) 
	{ 
		NGF::PythonGameObject *oth = static_cast<NGF::PythonGameObject*>(other);
		mPyEvents["collide"](mConnector, oth->getConnector());
	}

	//--- Python declarations -------------------------------------------------------------------
	
	NGF_PY_BEGIN_DECL
	{
	    NGF_PY_METHOD_DECL(applyImpulse)
	    NGF_PY_METHOD_DECL(setPosition)
	    NGF_PY_METHOD_DECL(printStuff)

	    NGF_PY_PROPERTY_DECL(mHealth)
	}
	NGF_PY_END_DECL
};

//--- Python implementations ------------------------------------------------------------------------

#include "pydef/Player_py.h"

NGF_PY_BEGIN_IMPL(Player)
{
    NGF_PY_METHOD(applyImpulse)
    {
	Vector3 vec = py::extract<Vector3>(args[0]);

	mBody->applyImpulse(BtOgre::Convert::toBullet(vec), btVector3(0,0,0));
	return py::object();
    }

    NGF_PY_METHOD(setPosition)
    {
	Vector3 vec = py::extract<Vector3>(args[0]);

	btTransform oldTrans = mBody->getWorldTransform();
	mBody->setWorldTransform(btTransform(oldTrans.getRotation(), BtOgre::Convert::toBullet(vec)));

	return py::object();
    }

    NGF_PY_METHOD(printStuff)
    {
	std::cout << "Hihihih!\n";
    }

    NGF_PY_PROPERTY(mHealth, float)
}
NGF_PY_END_IMPL

