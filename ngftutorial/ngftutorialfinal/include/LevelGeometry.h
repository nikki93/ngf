//---------------------------------------------------------------------------------------
// LEVELGEOMETRY.H
//---------------------------------------------------------------------------------------

class LevelGeometry : public NGF::GameObject
{
    protected:
	SceneNode *mNode;
	Entity *mEntity;

    public:
	LevelGeometry(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, NGF::PropertyList properties, String name) : NGF::GameObject(pos, rot, id , properties, name)
	{
	    String idStr = StringConverter::toString(id);
	    addFlag("LevelGeometry");

	    //Create Ogre stuff.
	    String brushMeshFile = properties.getValue("brushMeshFile", 0, "Player.mesh");
	    mEntity = Globals::smgr->createEntity(idStr + "-levelGeometryEntity", brushMeshFile);
	    mNode = Globals::smgr->getRootSceneNode()->createChildSceneNode(idStr + "levelGeometrySceneNode", pos, rot);
	    mNode->attachObject(mEntity);
	    mEntity->addQueryFlags(QF_LEVELGEOMETRY);
	}

	~LevelGeometry()
	{
	    //Destroy Ogre stuff.
	    mNode->detachAllObjects();
	    Globals::smgr->destroyEntity(mEntity);
	    Globals::smgr->destroySceneNode(mNode->getName());
	}

	void unpausedTick(const Ogre::FrameEvent &evt)
	{
	}

	void pausedTick(const Ogre::FrameEvent &evt)
	{
	}

        NGF::MessageReply receiveMessage(NGF::Message msg)
	{
	}
};
