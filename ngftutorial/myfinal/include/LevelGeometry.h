//---------------------------------------------------------------------------------------
// LEVELGEOMETRY.H
//---------------------------------------------------------------------------------------

class LevelGeometry : public NGF::PythonGameObject
{
    protected:
	SceneNode *mNode;
	Entity *mEntity;

	btRigidBody *mBody;
	btCollisionShape *mShape;

	int mTestVar;

    public:
	LevelGeometry(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, NGF::PropertyList properties, String name) : NGF::PythonGameObject(pos, rot, id , properties, name)
	{
	    String idStr = StringConverter::toString(id);
	    addFlag("LevelGeometry");

	    //Create Ogre stuff.
	    String brushMeshFile = properties.getValue("brushMeshFile", 0, "Player.mesh");
	    mEntity = Globals::smgr->createEntity(idStr + "-levelGeometryEntity", brushMeshFile);
	    mNode = Globals::smgr->getRootSceneNode()->createChildSceneNode(idStr + "levelGeometrySceneNode", pos, rot);
	    mNode->attachObject(mEntity);

	    //Create the  shape.
	    BtOgre::StaticMeshToShapeConverter converter2(mEntity);
	    mShape = converter2.createTrimesh();

	    //Create MotionState (no need for BtOgre here, you can use it if you want to though).
	    btDefaultMotionState* State = new btDefaultMotionState(
		    btTransform(BtOgre::Convert::toBullet(rot),BtOgre::Convert::toBullet(pos)));

	    //Create the Body.
	    mBody = new btRigidBody(0, State, mShape, btVector3(0,0,0));
	    Globals::phyWorld->addRigidBody(mBody);

	    Globals::gom->registerForCollision(this, mBody);

	    mTestVar = 0;
	}

	~LevelGeometry()
	{
	    //Destroy Ogre stuff.
	    mNode->detachAllObjects();
	    Globals::smgr->destroyEntity(mEntity);
	    Globals::smgr->destroySceneNode(mNode->getName());

	    //Destory physics stuff.
	    delete mShape;
	    delete mBody;
	}

	void unpausedTick(const Ogre::FrameEvent &evt)
	{
	}

	void pausedTick(const Ogre::FrameEvent &evt)
	{
	}

	void receiveMessage(NGF::Message msg)
	{
	}

	//--- Python functions ------------------------------------------------------------------------------
	
	NGF_PY_BEGIN_DECL
	{
	    NGF_PY_METHOD_DECL(sayHi)

	    NGF_PY_PROPERTY_DECL(mTestVar)
	}
	NGF_PY_END_DECL

};

#include "pydef/LevelGeometry_py.h"

NGF_PY_BEGIN_IMPL(LevelGeometry)
{
    NGF_PY_METHOD(sayHi)
    {
	LogManager::getSingleton().logMessage("I said hi!");
	return py::object();
    }

    NGF_PY_PROPERTY(mTestVar, int)
}
NGF_PY_END_IMPL

