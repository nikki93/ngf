//---------------------------------------------------------------------------------------
// PLAYER.H
//---------------------------------------------------------------------------------------

class Player : public NGF::GameObject
{
    protected:
	SceneNode *mNode;
	Entity *mEntity;
	Real mTime;

    public:
	Player(Ogre::Vector3 pos, Ogre::Quaternion rot, NGF::ID id, NGF::PropertyList properties, String name) : NGF::GameObject(pos, rot, id , properties, name)
	{
	    String idStr = StringConverter::toString(id);
	    addFlag("Player");
	    mTime = 0;

	    //Create Ogre stuff.
	    mEntity = Globals::smgr->createEntity(idStr + "-playerEntity", "Player.mesh");
	    mNode = Globals::smgr->getRootSceneNode()->createChildSceneNode(idStr + "playerSceneNode", pos, rot);
	    mNode->attachObject(mEntity);
	}

	~Player()
	{
	    //Destroy Ogre stuff.
	    mNode->detachAllObjects();
	    Globals::smgr->destroyEntity(mEntity);
	    Globals::smgr->destroySceneNode(mNode->getName());
	}

	void unpausedTick(const Ogre::FrameEvent &evt)
	{
	    Real move = Globals::keyboard->isKeyDown(OIS::KC_K) - Globals::keyboard->isKeyDown(OIS::KC_I);
	    Real rot = Globals::keyboard->isKeyDown(OIS::KC_J) - Globals::keyboard->isKeyDown(OIS::KC_L);

	    mNode->yaw(Degree(rot * 100 * evt.timeSinceLastFrame));

	    //Remember our old position.
	    Vector3 oldPos = mNode->getPosition();
	    mNode->translate(mNode->getOrientation() * Vector3(0,0,move * 5 * evt.timeSinceLastFrame));
	    Vector3 newPos = mNode->getPosition();

	    //Get back if the new position isn't very comfortable.
	    if (Globals::col->collidesWithEntity(oldPos, newPos, 0.2, 0, QF_LEVELGEOMETRY))
		mNode->setPosition(oldPos);

	    //We can't fly!
	    Globals::col->calculateY(mNode, false, false, 0, QF_LEVELGEOMETRY);

//	    mTime += evt.timeSinceLastFrame;
//
//	    Vector3 vec(Math::Sin(mTime), 0, Math::Cos(mTime));
//	    mNode->setPosition(vec);
	}

	void pausedTick(const Ogre::FrameEvent &evt)
	{
	}

	void receiveMessage(NGF::Message msg)
	{
	}
};
