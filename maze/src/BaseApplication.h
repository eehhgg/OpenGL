/*
The file BaseApplication.h is based on the Ogre Tutorial Framework:
http://www.ogre3d.org/tikiwiki/
*/

#include "stdafx.h"

#ifndef __BaseApplication_h_
#define __BaseApplication_h_

#include <stack>

#define WALL_HEIGHT  100.0f
#define WALL_THICKNESS  10.0f
#define WALL_THICKNESS_2  (WALL_THICKNESS / 2)
#define WALL_TEXTURE_SCALING  0.01f
#define WALL_TEXTURE_HEIGHT  (WALL_HEIGHT * WALL_TEXTURE_SCALING)
#define WALL_TEXTURE_THICKNESS  (WALL_THICKNESS * WALL_TEXTURE_SCALING)
#define FLOOR_Y  0.0f
#define CEIL_Y  (FLOOR_Y + WALL_HEIGHT)
#define CELL_SIZE  100
#define FONT_SIZE  0.05f

class BaseApplication : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    BaseApplication(void);
    virtual ~BaseApplication(void);
    virtual void go(void);

protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
    virtual void createScene(void) = 0; // Override me!
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    // Ogre::WindowEventListener
    //Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw);
    //Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw);

    Ogre::Root *mRoot;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    // OgreBites
    OgreBites::SdkTrayManager* mTrayMgr;
    OgreBites::SdkCameraMan* mCameraMan;       // basic camera controller
    OgreBites::ParamsPanel* mDetailsPanel;     // sample details panel
    bool mCursorWasVisible;                    // was cursor visible before dialog appeared
    bool mShutDown;

    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;

	// camera
	Ogre::Camera* mCamera;
	bool mGoingForward, mGoingRight, mGoingBack, mGoingLeft, mFastMove;
	bool mTurningUp, mTurningRight, mTurningDown, mTurningLeft;
	// parameters
	unsigned int mSeed;
	bool mCollisions, mFog;
	int mPlayerSpeed, mGridSize, mGroundSize, mFarClipDist, mInterval;
	float mFogBrightness, mAmbientBrightness;
	std::string mStrInitializing, mStrCreating, mStrDrawing, mStrLoading;
	std::string mStrFinishing, mStrWelcome, mStrGoal;
};

#endif // #ifndef __BaseApplication_h_
