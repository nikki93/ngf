//------------------------------------------------------------------------------
// MAIN.CPP
//------------------------------------------------------------------------------

//Library includes.
#include <Ogre.h>
#include <MyGUI.h>
#include <Ngf.h>
#include <ngfplugins/NgfPython.h>
#include <BtOgreGP.h>
#include <BtOgrePG.h>
#include <BtOgreExtras.h>

#include <ExampleApplication.h>

#include "Globals.h"
#include "Player.h"
#include "LevelGeometry.h"
#include "Level.h"
#include "NGFExampleFrameListener.h"
#include "NGFExampleApplication.h"

int main(int argc, char **argv)
{
	Py_Initialize();
	NGFExampleApplication app;

	try
	{
		app.go();
	}
	catch (py::error_already_set)
	{
		PyErr_Print();
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception:\n";
		std::cerr << e.what() << "\n";
	}

	Py_Finalize();
	return 0;
}
