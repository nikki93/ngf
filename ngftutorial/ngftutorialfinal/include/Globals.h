//------------------------------------------------------------------------------
// GLOBALS.H
//------------------------------------------------------------------------------

namespace Globals
{
    OIS::Keyboard *keyboard;
    RenderWindow *window;
    SceneManager *smgr;
    MOC::CollisionTools *col;

    NGF::GameObjectManager *gom;
    NGF::WorldManager *wom;
    NGF::Loading::Loader *load;
}

enum QueryFlags
{
    QF_LEVELGEOMETRY = 1<<7
};
