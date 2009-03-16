//------------------------------------------------------------------------------
// NGFEXAMPLEAPPLICATION.H
//------------------------------------------------------------------------------

void printFunc(String str)
{
    MyGUI::EditPtr out = MyGUI::Gui::getInstancePtr()->findWidget<MyGUI::Edit>("edt_console_out");
    out->setCaption(out->getCaption() + str);
}

class NGFExampleApplication : public ExampleApplication
{
    protected:
	NGFExampleFrameListener *mFrameListener;

	btAxisSweep3 *mBroadphase;
	btDefaultCollisionConfiguration *mCollisionConfig;
	btCollisionDispatcher *mDispatcher;
	btSequentialImpulseConstraintSolver *mSolver;

	MyGUI::Gui *mGUI;

    public:
	NGFExampleApplication()
	{
	    //Bullet initialisation.
	    mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
	    mCollisionConfig = new btDefaultCollisionConfiguration();
	    mDispatcher = new btCollisionDispatcher(mCollisionConfig);
	    mSolver = new btSequentialImpulseConstraintSolver();

	    Globals::phyWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
	    Globals::phyWorld->setGravity(btVector3(0,-9.8,0));
	}

	~NGFExampleApplication()
	{
	    delete mFrameListener;

	    delete NGF::PythonManager::getSingletonPtr();
	    //delete NGF::GameObjectFactory::getSingletonPtr();

	    //Free Bullet stuff.
	    delete Globals::phyWorld;
	    delete mSolver;
	    delete mDispatcher;
	    delete mCollisionConfig;
	    delete mBroadphase;
	}

	void consoleRun(MyGUI::WidgetPtr)
	{
	    MyGUI::EditPtr in = mGUI->findWidget<MyGUI::Edit>("edt_console_in");
	    String inStr = in->getOnlyText();
	    PyRun_SimpleString(inStr.c_str());
	}

    protected:

	void createScene(void)
	{
	    //GUI.
	    mGUI = new MyGUI::Gui();
	    mGUI->initialise(mWindow);
	    MyGUI::LayoutManager::getInstance().load("Console.layout");

	    MyGUI::ButtonPtr runButton = mGUI->findWidget<MyGUI::Button>("but_console_run");
	    runButton->eventMouseButtonClick = MyGUI::newDelegate(this, &NGFExampleApplication::consoleRun);
	    MyGUI::EditPtr edt = MyGUI::Gui::getInstancePtr()->findWidget<MyGUI::Edit>("edt_console_out");
	    edt->setTextAlign(MyGUI::Align::Top | MyGUI::Align::Left);
	    edt = MyGUI::Gui::getInstancePtr()->findWidget<MyGUI::Edit>("edt_console_in");
	    edt->setTextAlign(MyGUI::Align::Top | MyGUI::Align::Left);

	    //Globals
	    Globals::smgr = mSceneMgr;
	    Globals::window = mWindow;

	    //Set shadows and ambient and directional light.
	    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.75,0.75,0.75));
	    mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
	    mSceneMgr->setShadowColour(ColourValue(0.7,0.7,0.7));

	    Light *light = mSceneMgr->createLight("MainLight");
	    light->setType(Light::LT_DIRECTIONAL);
	    light->setDiffuseColour(ColourValue(0.15,0.15,0.15));
	    light->setSpecularColour(ColourValue(0,0,0));
	    light->setDirection(Vector3(1,-2.5,1)); 

	    //Camera.
	    mCamera->setPosition(Vector3(3,3,3));
	    mCamera->lookAt(Vector3::ZERO);
	    mCamera->setNearClipDistance(0.1);

	    //Add the worlds.
	    Globals::wom->addWorld(new Level("TestLevel"));
	    Globals::wom->addWorld(new Level("Level1"));

	    //Start running the Worlds.
	    Globals::wom->start(0);
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
	    Globals::load = new NGF::Loading::Loader(NULL);
	    new NGF::PythonManager(printFunc);

	    //Register GameObject types.
	    NGF_REGISTER_OBJECT_TYPE(Player);
	    NGF_REGISTER_OBJECT_TYPE(LevelGeometry);

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
