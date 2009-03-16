//------------------------------------------------------------------------------
// NGFEXAMPLEFRAMELISTENER.H
//------------------------------------------------------------------------------

class NGFExampleFrameListener : public ExampleFrameListener, public OIS::MouseListener, public OIS::KeyListener
{
protected:

public:
    NGFExampleFrameListener(RenderWindow* win, Camera* cam) : ExampleFrameListener(win, cam, true, true)
    {
        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

        Globals::keyboard = mKeyboard;
    }

    ~NGFExampleFrameListener()
    {
        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);
    }

    //Frame.
    bool frameStarted(const FrameEvent& evt)
    {
        //Move around when right mouse button is pressed.
        OIS::MouseState ms = mMouse->getMouseState();

        mCamera->setPolygonMode((Globals::keyboard->isKeyDown(OIS::KC_R)) ? PM_WIREFRAME : PM_SOLID);

        if (ms.buttonDown(OIS::MB_Right))
        {
            Vector3 translate = Vector3::ZERO;
            Real speed = 15;

            speed *= mKeyboard->isKeyDown(OIS::KC_LSHIFT) ? 6 : 1; //'Sprint' mode.

            if (mKeyboard->isKeyDown(OIS::KC_LEFT) || mKeyboard->isKeyDown(OIS::KC_A))
                translate.x = -speed;	//Move camera left.

            if (mKeyboard->isKeyDown(OIS::KC_RIGHT) || mKeyboard->isKeyDown(OIS::KC_D))
                translate.x = speed;	//Move camera right.

            if (mKeyboard->isKeyDown(OIS::KC_UP) || mKeyboard->isKeyDown(OIS::KC_W) )
                translate.z = -speed;	//Move camera forward.

            if (mKeyboard->isKeyDown(OIS::KC_DOWN) || mKeyboard->isKeyDown(OIS::KC_S) )
                translate.z = speed;	//Move camera backward.

            if (mKeyboard->isKeyDown(OIS::KC_PGUP))
                translate.y = speed;	//Move camera up.

            if (mKeyboard->isKeyDown(OIS::KC_PGDOWN))
                translate.y = -speed;	//Move camera down.

            Degree camYaw = Degree(-ms.X.rel * 0.3);
            Degree camPitch = Degree(-ms.Y.rel * 0.3);

            mCamera->yaw(camYaw);
            mCamera->pitch(camPitch);
            mCamera->moveRelative(translate * evt.timeSinceLastFrame);
        }

	//The 'default' frameStarted.
        ExampleFrameListener::frameStarted(evt);

        //Update NGF stuff. Exit (return false) if F12 key down.
        Globals::gom->tick(false, evt);
        return (!(mKeyboard->isKeyDown(OIS::KC_F12)));
    }

    //Key/Mouse events.
    bool keyPressed(const OIS::KeyEvent &arg)
    {
        return true;
    }

    bool keyReleased(const OIS::KeyEvent &arg)
    {
        return true;
    }

    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        return true;
    }

    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        return true;
    }

    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        return true;
    }
};
