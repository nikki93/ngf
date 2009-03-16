//------------------------------------------------------------------------------
// GLOBALS.H
//------------------------------------------------------------------------------

namespace Globals
{
    OIS::Keyboard *keyboard;
    RenderWindow *window;
    SceneManager *smgr;

    NGF::GameObjectManager *gom;
    NGF::WorldManager *wom;
    NGF::Loading::Loader *load;

    btDynamicsWorld *phyWorld;
}

enum QueryFlags
{
    QF_LEVELGEOMETRY = 1<<7
};
