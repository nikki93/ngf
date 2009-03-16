//------------------------------------------------------------------------------
// MAIN.CPP
//------------------------------------------------------------------------------

//Library includes.
#include <Ogre.h>
#include <Ngf.h>
#include "CollisionTools.h"

#include <ExampleApplication.h>

//NGFExample includes.
#include "Globals.h"
#include "Player.h"
#include "LevelGeometry.h"
#include "Level.h"
#include "Helper.h"
#include "NGFExampleFrameListener.h"
#include "NGFExampleApplication.h"

int main()
{
    NGFExampleApplication app;

    try
    {
        app.go();
    }
    catch (Ogre::Exception &e)
    {
        std::cerr << "Exception:\n";
        std::cerr << e.getFullDescription().c_str() << "\n";
    }

    return 0;
}
