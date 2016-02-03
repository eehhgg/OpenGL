/*
The file BaseApplication.cpp is based on the Ogre Tutorial Framework:
http://www.ogre3d.org/tikiwiki/
*/

#include "stdafx.h"
#include "BaseApplication.h"

//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mCameraMan(0),
    mDetailsPanel(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	// camera
	mGoingForward(false), mGoingRight(false), mGoingBack(false), mGoingLeft(false), mFastMove(false),
	mTurningUp(false), mTurningRight(false), mTurningDown(false), mTurningLeft(false),
	// parameters
	mSeed(time(NULL)), mCollisions(true), mFog(false), mPlayerSpeed(150), mGridSize(30),
	mGroundSize(mGridSize*CELL_SIZE), mFarClipDist(700), mInterval(30), mFogBrightness(0.9f), mAmbientBrightness(0.1f),
	mStrInitializing("Initializing"), mStrCreating("Creating maze"), mStrDrawing("Drawing maze"),
	mStrLoading("Loading textures"), mStrFinishing("Finishing"), mStrWelcome("Welcome to"),
	mStrGoal("Well done. Now you can go to the next maze.")
{
	// initialize seed
	srand(mSeed);   mSeed = rand();
	// read parameters
	std::ifstream configFile("Maze/configFile.txt");
	if (!configFile.is_open()) { return; }
	char param[100];   configFile >> param;
	while (configFile.good()) {
		if (!std::strcmp(param, "ambient_brightness")) {
			configFile >> mAmbientBrightness;
			if ( (mAmbientBrightness < 0) || (mAmbientBrightness > 1) ) { mAmbientBrightness = 0.1f; }
		}
		else if (!std::strcmp(param, "collisions")) {
			configFile >> mCollisions;
		}
		else if (!std::strcmp(param, "far_clip_dist")) {
			configFile >> mFarClipDist;
			if (mFarClipDist < CELL_SIZE) { mFarClipDist = 700; }
		}
		else if (!std::strcmp(param, "fog")) {
			configFile >> mFog;
		}
		else if (!std::strcmp(param, "fog_brightness")) {
			configFile >> mFogBrightness;
			if ( (mFogBrightness < 0) || (mFogBrightness > 1) ) { mFogBrightness = 0.9f; }
		}
		else if (!std::strcmp(param, "grid_size")) {
			configFile >> mGridSize;
			if ( (mGridSize < 10) || (mGridSize > 300) ) { mGridSize = 30; }
			mGroundSize = mGridSize * CELL_SIZE;
		}
		else if (!std::strcmp(param, "interval")) {
			configFile >> mInterval;
			if (mInterval < 1) { mInterval = 30; }
		}
		else if (!std::strcmp(param, "player_speed")) {
			configFile >> mPlayerSpeed;
			if ( (mPlayerSpeed < 10) || (mPlayerSpeed > 1000) ) { mPlayerSpeed = 150; }
		}
		else if (!std::strcmp(param, "seed")) {
			configFile >> mSeed;
		}
		else if(!std::strcmp(param, "str_initializing")) {
			getline(configFile, mStrInitializing);
		}
		else if(!std::strcmp(param, "str_creating")) {
			getline(configFile, mStrCreating);
		}
		else if(!std::strcmp(param, "str_drawing")) {
			getline(configFile, mStrDrawing);
		}
		else if(!std::strcmp(param, "str_loading")) {
			getline(configFile, mStrLoading);
		}
		else if(!std::strcmp(param, "str_finishing")) {
			getline(configFile, mStrFinishing);
		}
		else if(!std::strcmp(param, "str_welcome")) {
			getline(configFile, mStrWelcome);
		}
		else if(!std::strcmp(param, "str_goal")) {
			getline(configFile, mStrGoal);
		}
		configFile >> param;
	}
	configFile.close();
}

//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    if (mCameraMan) delete mCameraMan;
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

//-------------------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Maze");

        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void BaseApplication::createCamera(void) {
	// Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}
//-------------------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void BaseApplication::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
	if (mFog) { vp->setBackgroundColour(Ogre::ColourValue(mFogBrightness,mFogBrightness,mFogBrightness)); }
	else { vp->setBackgroundColour(Ogre::ColourValue(0,0,0)); }

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseApplication::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
};
//-------------------------------------------------------------------------------------
bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

	mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }
    return true;
}
//-------------------------------------------------------------------------------------
bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{
	if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

	// camera
	if (!mCollisions) { mCameraMan->injectKeyDown(arg);   return true; }
	if (arg.key == OIS::KC_W || arg.key == OIS::KC_UP) { mGoingForward = true; }
	else if (arg.key == OIS::KC_S || arg.key == OIS::KC_DOWN) { mGoingBack = true; }
	else if (arg.key == OIS::KC_LEFT) { mTurningLeft = true; }
	else if (arg.key == OIS::KC_RIGHT) { mTurningRight = true; }
	else if (arg.key == OIS::KC_A) { mGoingLeft = true; }
	else if (arg.key == OIS::KC_D) { mGoingRight = true; }
	else if (arg.key == OIS::KC_LSHIFT || arg.key == OIS::KC_RSHIFT) { mFastMove = true; }
	else if (arg.key == OIS::KC_PGUP) { mTurningUp = true; }
	else if (arg.key == OIS::KC_PGDOWN) { mTurningDown = true; }
    return true;
}

bool BaseApplication::keyReleased( const OIS::KeyEvent &arg )
{
	if (!mCollisions) { mCameraMan->injectKeyUp(arg);   return true; }
	if (arg.key == OIS::KC_W || arg.key == OIS::KC_UP) { mGoingForward = false; }
	else if (arg.key == OIS::KC_S || arg.key == OIS::KC_DOWN) { mGoingBack = false; }
	else if (arg.key == OIS::KC_LEFT) { mTurningLeft = false; }
	else if (arg.key == OIS::KC_RIGHT) { mTurningRight = false; }
	else if (arg.key == OIS::KC_A) { mGoingLeft = false; }
	else if (arg.key == OIS::KC_D) { mGoingRight = false; }
	else if (arg.key == OIS::KC_LSHIFT || arg.key == OIS::KC_RSHIFT) { mFastMove = false; }
	else if (arg.key == OIS::KC_PGUP) { mTurningUp = false; }
	else if (arg.key == OIS::KC_PGDOWN) { mTurningDown = false; }
	else if (arg.key == OIS::KC_END) {
		Ogre::Vector3 v = mCamera->getPosition();
		v.x += mCamera->getDirection().x;   v.z += mCamera->getDirection().z;
		mCamera->lookAt(v);
	}
    return true;
}

bool BaseApplication::mouseMoved( const OIS::MouseEvent &arg )
{
	//if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
    return true;
}

bool BaseApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool BaseApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}
