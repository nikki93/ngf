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
			Globals::col->setHeightAdjust(0.5);

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
