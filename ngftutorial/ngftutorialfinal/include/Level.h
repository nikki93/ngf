//---------------------------------------------------------------------------------------
// LEVEL.H
//---------------------------------------------------------------------------------------

class Level : public NGF::World
{
    protected:
	String mLevel;
	bool mJustStartedN;
	bool mJustStartedP;

    public:
	Level(String levelName) 
	    : mLevel(levelName)
	{
	}

	~Level() 
	{
	}

	void init()
	{
	    Globals::load->loadLevel(mLevel);
	}

	void tick(const Ogre::FrameEvent &evt)
	{
	}

	void stop()
	{
	    Globals::gom->destroyAll();
	}
};

