//------------------------------------------------------------------------------
// NGFEXAMPLEAPPLICATION.H
//------------------------------------------------------------------------------

class NGFExampleApplication : public ExampleApplication
{
protected:
    NGFExampleFrameListener *mFrameListener;

public:
    NGFExampleApplication()
    {
    }

    ~NGFExampleApplication()
    {
        delete mFrameListener;
        delete Globals::col;
    }

protected:

    void createScene(void)
    {
        //Globals
        Globals::smgr = mSceneMgr;
        Globals::window = mWindow;
		Globals::col = new MOC::CollisionTools(mSceneMgr);

        //Camera.
        mCamera->setPosition(Vector3(3,3,3));
        mCamera->lookAt(Vector3::ZERO);
        mCamera->setNearClipDistance(0.1);
    }

    void createFrameListener(void)
    {
	//Create and register the FrameListener.
        mFrameListener = new NGFExampleFrameListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }

    void setupResources(void)
    {
        //Create the NGF stuff. The level loader needs to be created before the resource parsing,
	//because it needs to know about the *.ngf files.
        Globals::gom = new NGF::GameObjectManager();
	Globals::wom = new NGF::WorldManager();
        Globals::load = new NGF::Loading::Loader(&loaderHelperFunction);

        //Load resource paths from config file.
        ConfigFile cf;
        cf.load("resources.cfg");

        //Go through all sections & settings in the file.
        ConfigFile::SectionIterator seci = cf.getSectionIterator();

        String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            ConfigFile::SettingsMultiMap *settings = seci.getNext();
            ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
            }
        }
    }
};
