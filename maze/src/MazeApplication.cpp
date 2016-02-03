
#include "stdafx.h"
#include "MazeApplication.h"

MazeApplication::MazeApplication(void) {
	mLogFile.open("Maze/logFile.txt");
	srand(mSeed);   mLogFile << "seed: " << mSeed << std::endl;
	mTranslation = 0;   mTurning = 0;   mTime = -1;
	// define initial and goal cells
	// (these must be on different rows and columns, and the Manhattan distance
	// between them must be at least mGridSize/2)
	mInitialRow = rand() % mGridSize;   mInitialCol = rand() % mGridSize;
	int dist, minDist = mGridSize/2, iter = 0;
	do {
		mGoalRow = rand() % mGridSize;   mGoalCol = rand() % mGridSize;
		dist = std::abs(mGoalRow-mInitialRow) + std::abs(mGoalCol-mInitialCol);   iter++;
	} while ( (iter < 50) && ((mGoalRow == mInitialRow) || (mGoalCol == mInitialCol) || (dist < minDist)) );
	mCurrentRow = mInitialRow;   mCurrentCol = mInitialCol;
	// read dialogs
	readDialogs("Maze/dialogsInterested.txt", mDialogsInterested, mIndDialogsI, mRemDialogsI);
	readDialogs("Maze/dialogsConfused.txt", mDialogsConfused, mIndDialogsC, mRemDialogsC);
	readDialogs("Maze/dialogsBored.txt", mDialogsBored, mIndDialogsB, mRemDialogsB);
	readDialogs("Maze/dialogsDesperate.txt", mDialogsDesperate, mIndDialogsD, mRemDialogsD);
	readDialogs("Maze/dialogsHelp.txt", mDialogsHelp, mIndDialogsH, mRemDialogsH);
}

MazeApplication::~MazeApplication(void) {
	mLogFile.close();
}

// overriden methods of BaseApplication ---------------------------------------

void MazeApplication::createCamera(void) {
	float x = (mInitialRow+0.5f)*CELL_SIZE, y = (FLOOR_Y+CEIL_Y)/2.0f, z = (mInitialCol+0.5f)*CELL_SIZE;
	mCamera = mSceneMgr->createCamera("PlayerCam");
	mCamera->setNearClipDistance(5);   mCamera->setFarClipDistance(mFarClipDist);
	mCamera->setPosition(x, y, z);     mCamera->lookAt(x+1, y, z);
	mCameraMan = new OgreBites::SdkCameraMan(mCamera);
	mCameraMan->setTopSpeed(mPlayerSpeed);
}

void MazeApplication::createScene(void) {
	mMazeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	// markers and overlays
	createMarkers();   createOverlays();
	setMessage(mStrInitializing);   mRoot->renderOneFrame();
    // lights
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
	mSceneMgr->setAmbientLight(Ogre::ColourValue(mAmbientBrightness, mAmbientBrightness, mAmbientBrightness));
	Ogre::Vector3 lightdir(1.0f, -1.0f, -1.0f);   lightdir.normalise();
    Ogre::Light* light = mSceneMgr->createLight("dirLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(lightdir);
	const float LIGHT_COLOR = 0.4f;
	light->setDiffuseColour(Ogre::ColourValue(LIGHT_COLOR, LIGHT_COLOR, LIGHT_COLOR));
    light->setSpecularColour(Ogre::ColourValue(LIGHT_COLOR, LIGHT_COLOR, LIGHT_COLOR));
	// floor
	const float FLOOR_TEXTURE_SIZE = 2.0f*mGroundSize/1024;
	Ogre::Vector3 nFloor = Ogre::Vector3::UNIT_Y;
	Ogre::ManualObject* floor = mSceneMgr->createManualObject();
	floor->setCastShadows(false);   floor->setDynamic(false);
	floor->begin("Maze/GrassFloor", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	floor->position(0.0, FLOOR_Y, 0.0);						floor->normal(nFloor);   floor->textureCoord(0,0);
	floor->position(0.0, FLOOR_Y, mGroundSize);				floor->normal(nFloor);   floor->textureCoord(FLOOR_TEXTURE_SIZE,0);
	floor->position(mGroundSize, FLOOR_Y, mGroundSize);		floor->normal(nFloor);   floor->textureCoord(FLOOR_TEXTURE_SIZE,FLOOR_TEXTURE_SIZE);
	floor->position(mGroundSize, FLOOR_Y, 0.0);				floor->normal(nFloor);   floor->textureCoord(0,FLOOR_TEXTURE_SIZE);
	floor->quad(0,1,2,3);   floor->end();   floor->convertToMesh("floor");
	mMazeNode->attachObject( mSceneMgr->createEntity("floor") );
	// border walls
	float p1 = 0 - WALL_THICKNESS_2, p2 = mGroundSize + WALL_THICKNESS_2;
	getWallMesh(p2, p1, p1, p1, "leftWall");   // left
	mMazeNode->attachObject( mSceneMgr->createEntity("leftWall") );
	getWallMesh(p1, p1+WALL_THICKNESS, p1, p2-WALL_THICKNESS, "bottomWall");   // bottom
	mMazeNode->attachObject( mSceneMgr->createEntity("bottomWall") );
	getWallMesh(p1, p2, p2, p2, "rightWall");   // right
	mMazeNode->attachObject( mSceneMgr->createEntity("rightWall") );
	getWallMesh(p2, p2-WALL_THICKNESS, p2, p1+WALL_THICKNESS, "topWall");   // top
	mMazeNode->attachObject( mSceneMgr->createEntity("topWall") );
	// internal walls
	setMessage(mStrCreating);   mRoot->renderOneFrame();   generateMaze();
	setMessage(mStrDrawing);   mRoot->renderOneFrame();   drawMaze();
	setMessage(mStrLoading);   mRoot->renderOneFrame();   loadImages();
	setMessage(mStrFinishing);   mRoot->renderOneFrame();
	// initial and goal cells
	drawMeshAt(CELL_SIZE*(mInitialRow+0.5f), CELL_SIZE*(mInitialCol+0.5f), mMarkerBaseGreen, mMazeNode);
	drawMeshAt(CELL_SIZE*(mGoalRow+0.5f), CELL_SIZE*(mGoalCol+0.5f), mMarkerBaseRed, mMazeNode);
	mGrid[mGoalRow][mGoalCol].imageCaption = mStrGoal;
	// sky
	Ogre::Plane plane;   plane.d = 500;   plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;
	mSceneMgr->setSkyPlane(true, plane, "Maze/Sky", 15, 1, true, 1.5f, 100, 100);
	// fog
	if (mFog) {
		mSceneMgr->setFog(Ogre::FOG_LINEAR,
			Ogre::ColourValue(mFogBrightness,mFogBrightness,mFogBrightness),
			0.0, CELL_SIZE/2.0f, mFarClipDist);
	}
	// welcome message
	if (mCollisions) {
		std::stringstream caption;
		caption << mStrWelcome << " " << mSeed << "-" << mGridSize;
		setMessage(caption.str());
	}
}

bool MazeApplication::frameRenderingQueued(const Ogre::FrameEvent& evt) {
	if(mWindow->isClosed()) { return false; }
	if(mShutDown) { return false; }
    mKeyboard->capture();   mMouse->capture();
	// update the position and direction of the camera
	if (!mCollisions) { mCameraMan->frameRenderingQueued(evt);   return true; }
	if ( mGoingForward || mGoingRight || mGoingBack || mGoingLeft) {
		// position
		// get the current direction
		Ogre::Vector3 p1 = mCamera->getPosition(), p2 = p1, front = mCamera->getDirection();
		float distance = mPlayerSpeed * evt.timeSinceLastFrame;
		if (mFastMove) { distance *= 2; }
		front.y = 0;   front.normalise();   front *= distance;
		Ogre::Vector3 right;   right.x = -front.z;   right.y = 0;   right.z = front.x;
		// calculate the new position
		if (mGoingForward) { p2 += front; }
		else if (mGoingBack) { p2 -= front; }
		if (mGoingRight) { p2 += right; }
		else if (mGoingLeft) { p2 -= right; }
		// detect collisions
		int collisionValue = collision(p1,p2);
		if (collisionValue < 3) {
			if (collisionValue == 1) { p2.x = p1.x; }
			else if (collisionValue == 2) { p2.z = p1.z; }
			// move the camera
			mCamera->setPosition(p2);
			// record the movement
			if ( (p2.x != p1.x) || (p2.z != p1.z) ) { mTranslation += (distance/mPlayerSpeed); }
		}
	}
	if (mTurningUp || mTurningRight || mTurningDown || mTurningLeft) {
		// direction
		int rotSpeed = 1000, xRel = 0, yRel = 0;
		if (mTurningUp) {  yRel = -rotSpeed; }
		else if (mTurningDown) { yRel = rotSpeed; }
		if (mTurningRight) { xRel = rotSpeed; }
		else if (mTurningLeft) { xRel = -rotSpeed; }
		float pitch = -yRel * 0.15f * evt.timeSinceLastFrame;
		mCamera->yaw(Ogre::Degree(-xRel * 0.15f * evt.timeSinceLastFrame));
		mCamera->pitch(Ogre::Degree(pitch));
		// if the max vertical rotation is exceeded, undo the pitch
		if (mCamera->getUp().y <= 0) { mCamera->pitch(Ogre::Degree(-pitch)); }
		// record the movement
		mTurning += std::abs(xRel);
	}
	// update mCurrentRow, mCurrentCol and the top message
	Ogre::Vector3 p = mCamera->getPosition();
	int row = std::floor(p.x / CELL_SIZE), col = std::floor(p.z / CELL_SIZE);
	if ( (row != mCurrentRow) || (col != mCurrentCol) ) {
		mCurrentRow = row;   mCurrentCol = col;
		setTopMessage(mGrid[mCurrentRow][mCurrentCol].imageCaption);
	}
	// react to the user's movement
	reactToMovement();   return true;
}

bool MazeApplication::mouseMoved( const OIS::MouseEvent &arg ) {
	if (!mCollisions) { mCameraMan->injectMouseMove(arg);   return true; }
	// ignore the mouse movement when any of the turning keys is being pressed
	if (mTurningUp || mTurningDown || mTurningLeft || mTurningRight) { return true; }
	// FP camera
	float pitch = -arg.state.Y.rel * 0.15f;
	mCamera->yaw(Ogre::Degree(-arg.state.X.rel * 0.15f));
	mCamera->pitch(Ogre::Degree(pitch));
	// if the max vertical rotation is exceeded, undo the pitch
	if (mCamera->getUp().y <= 0) { mCamera->pitch(Ogre::Degree(-pitch)); }
	// record the movement
	mTurning += std::abs(arg.state.X.rel);   return true;
}

// maze generation ------------------------------------------------------------

void MazeApplication::generateMaze(void) {
	int i, j, k, rem;
	// initialize
	mGrid.resize(mGridSize);
	for (i = 0; i < mGridSize; i++) { mGrid[i].resize(mGridSize); }
	initializeGrid();
	// depth-first search
	cell *currentCell, *neighborCell;
	i = rand() % mGridSize;   j = rand() % mGridSize;   currentCell = &mGrid[i][j];
	currentCell->visited = true;
	std::stack<cell*> visitStack;   visitStack.push(currentCell);
	while (!visitStack.empty()) {
		currentCell = visitStack.top();   rem = currentCell->remNeighbors;
		if (rem == 0) { visitStack.pop(); }
		else {
			// select a neighbor, and "remove it" from the neighbors array
			k = rand() % rem;   neighborCell = currentCell->neighbors[k];
			currentCell->neighbors[k] = currentCell->neighbors[rem-1];
			currentCell->neighbors[rem-1] = neighborCell;   currentCell->remNeighbors--;
			if (!neighborCell->visited) {
				// remove the wall and visit the neighbor
				if (currentCell->row != neighborCell->row) {   // up neighbor
					i = std::min( currentCell->row, neighborCell->row );
					j = currentCell->col;   mGrid[i][j].openUp = true;
				} else {   // right neighbor
					j = std::min( currentCell->col, neighborCell->col );
					i = currentCell->row;   mGrid[i][j].openRight = true;
				}
				neighborCell->visited = true;   visitStack.push(neighborCell);
			}
		}
	}
	// remove some internal walls randomly
	rem = mGridSize/2;
	while (rem > 0) {
		i = rand() % (mGridSize-2) + 1;   j = rand() % (mGridSize-2) + 1;   k = rand() % 2;
		if (k == 0) {   // up wall
			if (!mGrid[i][j].openUp) { mGrid[i][j].openUp = true;   rem--; }
		} else {   // right wall
			if (!mGrid[i][j].openRight) { mGrid[i][j].openRight = true;   rem--; }
		}
	}
	// restore grid
	for (i = 0; i < mGridSize; i++) { for (j = 0; j < mGridSize; j++) {
		mGrid[i][j].visited = false;
		mGrid[i][j].remNeighbors = mGrid[i][j].totalNeighbors;
	} }
}

void MazeApplication::initializeGrid(void) {
	int i, j;
	// internal cells
	for (i = 1; i <= mGridSize-2; i++) { for (j = 1; j <= mGridSize-2; j++) {
		mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 4;
		mGrid[i][j].neighbors[0] = &mGrid[i+1][j];   mGrid[i][j].neighbors[1] = &mGrid[i][j+1];
		mGrid[i][j].neighbors[2] = &mGrid[i-1][j];   mGrid[i][j].neighbors[3] = &mGrid[i][j-1];
		mGrid[i][j].visited = false;   mGrid[i][j].openUp = false;   mGrid[i][j].openRight = false;
	} }
	// the border cells are asumed to be connected with the outside of the maze, so that
	// the drawMaze procedure does not draw the border walls, which are drawn separately
	// bottom row
	i = 0;   j = 0;
	mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 2;
	mGrid[i][j].neighbors[0] = &mGrid[i+1][j];    mGrid[i][j].neighbors[1] = &mGrid[i][j+1];
	mGrid[i][j].visited = false;   mGrid[i][j].openUp = false;   mGrid[i][j].openRight = false;
	for (j = 1; j <= mGridSize-2; j++) {
		mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 3;
		mGrid[i][j].neighbors[0] = &mGrid[i+1][j];    mGrid[i][j].neighbors[1] = &mGrid[i][j+1];
		mGrid[i][j].neighbors[2] = &mGrid[i][j-1];
		mGrid[i][j].visited = false;   mGrid[i][j].openUp = false;   mGrid[i][j].openRight = false;
	}
	j = mGridSize-1;
	mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 2;
	mGrid[i][j].neighbors[0] = &mGrid[i+1][j];    mGrid[i][j].neighbors[1] = &mGrid[i][j-1];
	mGrid[i][j].visited = false;   mGrid[i][j].openUp = false;   mGrid[i][j].openRight = true;   // right border
	// top row
	i = mGridSize-1;   j = 0;
	mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 2;
	mGrid[i][j].neighbors[0] = &mGrid[i][j+1];    mGrid[i][j].neighbors[1] = &mGrid[i-1][j];
	mGrid[i][j].visited = false;   mGrid[i][j].openUp = true;   mGrid[i][j].openRight = false;   // top border
	for (j = 1; j <= mGridSize-2; j++) {
		mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 3;
		mGrid[i][j].neighbors[0] = &mGrid[i][j+1];    mGrid[i][j].neighbors[1] = &mGrid[i-1][j];
		mGrid[i][j].neighbors[2] = &mGrid[i][j-1];
		mGrid[i][j].visited = false;   mGrid[i][j].openUp = true;   mGrid[i][j].openRight = false;   // top border
	}
	j = mGridSize-1;
	mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 2;
	mGrid[i][j].neighbors[0] = &mGrid[i-1][j];    mGrid[i][j].neighbors[1] = &mGrid[i][j-1];
	mGrid[i][j].visited = false;   mGrid[i][j].openUp = true;   mGrid[i][j].openRight = true;   // top-right border
	// left column
	j = 0;
	for (i = 1; i <= mGridSize-2; i++) {
		mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 3;
		mGrid[i][j].neighbors[0] = &mGrid[i+1][j];    mGrid[i][j].neighbors[1] = &mGrid[i][j+1];
		mGrid[i][j].neighbors[2] = &mGrid[i-1][j];
		mGrid[i][j].visited = false;   mGrid[i][j].openUp = false;   mGrid[i][j].openRight = false;
	}
	// right column
	j = mGridSize-1;
	for (i = 1; i <= mGridSize-2; i++) {
		mGrid[i][j].row = i;   mGrid[i][j].col = j;   mGrid[i][j].totalNeighbors = 3;
		mGrid[i][j].neighbors[0] = &mGrid[i+1][j];    mGrid[i][j].neighbors[1] = &mGrid[i-1][j];
		mGrid[i][j].neighbors[2] = &mGrid[i][j-1];
		mGrid[i][j].visited = false;   mGrid[i][j].openUp = false;   mGrid[i][j].openRight = true;   // right border
	}
	// remNeighbors = totalNeighbors
	for (i = 0; i < mGridSize; i++) { for (j = 0; j < mGridSize; j++) {
		mGrid[i][j].remNeighbors = mGrid[i][j].totalNeighbors;
	} }
}

int MazeApplication::collision(Ogre::Vector3 &position, Ogre::Vector3 &newPosition) {
	// returns 0 for no collision, 1 for collision on x, 2 for collision on z, and 3 for collision on x and z
	float x = position.x, z = position.z, nx = newPosition.x, nz = newPosition.z;
	int row = std::floor(x / CELL_SIZE), col = std::floor(z / CELL_SIZE);
	int nRow = std::floor(nx / CELL_SIZE), nCol = std::floor(nz / CELL_SIZE);
	if ( (std::abs(row-nRow) > 1) || (std::abs(col-nCol) > 1) ) { return 3; }   // too much distance
	if ( (std::abs(row-nRow) == 1) && (std::abs(col-nCol) == 1)   // diagonally adjacent cell
	&& hasTopRightCorner(std::min(row,nRow), std::min(col,nCol)) ) { return 3; }
	float top = (row+1) * CELL_SIZE, right = (col+1) * CELL_SIZE, bottom = top - CELL_SIZE, left = right - CELL_SIZE;
	float margin = WALL_THICKNESS_2 + 10;
	top -= margin;   right -= margin;   bottom += margin;   left += margin;
	bool borderX = (x > top) || (x < bottom), borderZ = (z > right) || (z < left);
	bool collisionX = false, collisionZ = false;
	// top
	if (nx > mGroundSize-margin) { collisionX = true; }
	else if (nx > top) {
		if (!mGrid[row][col].openUp) { collisionX = true; }
		else if ( (z > right) && hasTopRightCorner(row,col) ) { collisionX = true; }
		else if ( (z < left) && hasTopRightCorner(row,col-1) ) { collisionX = true; }
	}
	// right
	if (nz > mGroundSize-margin) { collisionZ = true; }
	else if (nz > right) {
		if (!mGrid[row][col].openRight) { collisionZ = true; }
		else if ( (x > top) && hasTopRightCorner(row,col) ) { collisionZ = true; }
		else if ( (x < bottom) && hasTopRightCorner(row-1,col) ) { collisionZ = true; }
	}
	// bottom
	if (nx < margin) { collisionX = true; }
	else if (nx < bottom) {
		if (!mGrid[row-1][col].openUp) { collisionX = true; }
		else if ( (z > right) && hasTopRightCorner(row-1,col) ) { collisionX = true; }
		else if ( (z < left) && hasTopRightCorner(row-1,col-1) ) { collisionX = true; }
	}
	// left
	if (nz < margin) { collisionZ = true; }
	else if (nz < left) {
		if (!mGrid[row][col-1].openRight) { collisionZ = true; }
		else if ( (x > top) && hasTopRightCorner(row,col-1) ) { collisionZ = true; }
		else if ( (x < bottom) && hasTopRightCorner(row-1,col-1) ) { collisionZ = true; }
	}
	if (!collisionX && !collisionZ) { return 0; }
	if (collisionX && !collisionZ) { return 1; }
	if (!collisionX && collisionZ) { return 2; }
	return 3;
}

bool MazeApplication::hasTopRightCorner(int row, int col) {
	if ( (row < 0) || (row >= mGridSize-1) || (col < 0) || (col >= mGridSize-1) ) { return false; }
	if (!mGrid[row][col].openUp || !mGrid[row][col].openRight
	|| !mGrid[row+1][col].openRight || !mGrid[row][col+1].openUp) { return true; }
	return false;
}

// maze drawing ---------------------------------------------------------------

void MazeApplication::drawMaze(void) {
	int i, j;
	float x, z;
	const float CORNER_OFFSET = -WALL_THICKNESS_2;
	Ogre::MeshPtr hWall = getWallMesh(0,0, 0,CELL_SIZE - WALL_THICKNESS, "hWall");
	Ogre::MeshPtr vWall = getWallMesh(0,0, CELL_SIZE - WALL_THICKNESS,0, "vWall");
	Ogre::MeshPtr corner = getWallMesh(0,0, 0,WALL_THICKNESS, "corner");
	for (i = 0; i < mGridSize; i++) { for (j = 0; j < mGridSize; j++) {
		if (!mGrid[i][j].openUp) {   // up wall
			x = CELL_SIZE * (i+1) - WALL_THICKNESS_2;   z = CELL_SIZE * j + WALL_THICKNESS_2;
			drawMeshAt(x,z, hWall, mMazeNode);
		}
		if (!mGrid[i][j].openRight) {   // right wall
			x = CELL_SIZE * i + WALL_THICKNESS_2;   z = CELL_SIZE * (j+1) + WALL_THICKNESS_2;
			drawMeshAt(x,z, vWall, mMazeNode);
		}
		if (hasTopRightCorner(i,j)) {   // top-right corner
			x = CELL_SIZE * (i+1) + CORNER_OFFSET;   z = CELL_SIZE * (j+1) + CORNER_OFFSET;
			drawMeshAt(x,z, corner, mMazeNode);
		}
	} }
}

void MazeApplication::drawMeshAt(float x, float z, Ogre::MeshPtr meshPtr, Ogre::SceneNode *node) {
	Ogre::SceneNode *node1 = node->createChildSceneNode();
	Ogre::Entity *ent1 = mSceneMgr->createEntity(meshPtr->getName());
	node1->attachObject(ent1);   node1->translate(x,0,z);
}

Ogre::MeshPtr MazeApplication::getWallMesh(float x1, float z1, float x2, float z2, Ogre::String name) {
	// define third and fourth points, 4 normals, and textureLength
	bool error = false;
	float x3, z3, x4, z4, textureLength;
	Ogre::Vector3 n1, n2, n3, n4;
	if (abs(x1 - x2) < 0.001) {
		if (z1 < z2) {   // -z to +z
			x3 = x1 + WALL_THICKNESS;
			n1 = Ogre::Vector3::NEGATIVE_UNIT_X;   n2 = Ogre::Vector3::UNIT_Z;
			n3 = Ogre::Vector3::UNIT_X;   n4 = Ogre::Vector3::NEGATIVE_UNIT_Z;
		} else {   // +z to -z
			x3 = x1 - WALL_THICKNESS;
			n1 = Ogre::Vector3::UNIT_X;   n2 = Ogre::Vector3::NEGATIVE_UNIT_Z;
			n3 = Ogre::Vector3::NEGATIVE_UNIT_X;   n4 = Ogre::Vector3::UNIT_Z;
		}
		z3 = z2;   z4 = z1;   x4 = x3;   textureLength = abs(z1-z2) * WALL_TEXTURE_SCALING;
	} else if (abs(z1 - z2) < 0.001) {
		if (x1 < x2) {   // -x to +x
			z3 = z1 - WALL_THICKNESS;
			n1 = Ogre::Vector3::UNIT_Z;   n2 = Ogre::Vector3::UNIT_X;
			n3 = Ogre::Vector3::NEGATIVE_UNIT_Z;   n4 = Ogre::Vector3::NEGATIVE_UNIT_X;
		} else {   // +x to -x
			z3 = z1 + WALL_THICKNESS;
			n1 = Ogre::Vector3::NEGATIVE_UNIT_Z;   n2 = Ogre::Vector3::NEGATIVE_UNIT_X;
			n3 = Ogre::Vector3::UNIT_Z;   n4 = Ogre::Vector3::UNIT_X;
		}
		x3 = x2;   x4 = x1;   z4 = z3;   textureLength = abs(x1-x2) * WALL_TEXTURE_SCALING;
	} else { error = true; }
	// draw
	Ogre::ManualObject *wall = mSceneMgr->createManualObject(name);
	wall->setCastShadows(true);   wall->setDynamic(false);
	if (!error) {   // wall
		// lateral planes
		addWallPlane(x1,z1,x2,z2,n1,textureLength,wall);   addWallPlane(x2,z2,x3,z3,n2,WALL_TEXTURE_THICKNESS,wall);
		addWallPlane(x3,z3,x4,z4,n3,textureLength,wall);   addWallPlane(x4,z4,x1,z1,n4,WALL_TEXTURE_THICKNESS,wall);
		// top and bottom planes
		Ogre::Vector3 nTop = Ogre::Vector3::UNIT_Y, nBottom = Ogre::Vector3::NEGATIVE_UNIT_Y;
		wall->begin("Maze/Rockwall", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		wall->position(x1, CEIL_Y, z1);    wall->normal(nTop);   wall->textureCoord(0, 0);
		wall->position(x2, CEIL_Y, z2);    wall->normal(nTop);   wall->textureCoord(textureLength, 0);
		wall->position(x3, CEIL_Y, z3);    wall->normal(nTop);   wall->textureCoord(textureLength, WALL_TEXTURE_THICKNESS);
		wall->position(x4, CEIL_Y, z4);    wall->normal(nTop);   wall->textureCoord(0, WALL_TEXTURE_THICKNESS);
		wall->position(x1, FLOOR_Y, z1);   wall->normal(nTop);   wall->textureCoord(0, 0);
		wall->position(x2, FLOOR_Y, z2);   wall->normal(nTop);   wall->textureCoord(textureLength, 0);
		wall->position(x3, FLOOR_Y, z3);   wall->normal(nTop);   wall->textureCoord(textureLength, WALL_TEXTURE_THICKNESS);
		wall->position(x4, FLOOR_Y, z4);   wall->normal(nTop);   wall->textureCoord(0, WALL_TEXTURE_THICKNESS);
		wall->quad(0,1,2,3);   wall->quad(7,6,5,4);   wall->end();
	} else {
		mLogFile << "error on getWallMesh(" << x1 << "," << z1 << ", "
			<< x2 << "," << z2 << ", " << name << std::endl;
		addWallPlane(x1,z1,x2,z2,Ogre::Vector3::UNIT_X,1,wall);
	}
	return wall->convertToMesh(name);
}

void MazeApplication::addWallPlane(float x1, float z1, float x2, float z2,
Ogre::Vector3 normal, float textureLength, Ogre::ManualObject *wall) {
	wall->begin("Maze/Rockwall", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	wall->position(x1, FLOOR_Y, z1);   wall->normal(normal);   wall->textureCoord(0, WALL_TEXTURE_HEIGHT);
	wall->position(x2, FLOOR_Y, z2);   wall->normal(normal);   wall->textureCoord(textureLength, WALL_TEXTURE_HEIGHT);
	wall->position(x2, CEIL_Y, z2);    wall->normal(normal);   wall->textureCoord(textureLength, 0);
	wall->position(x1, CEIL_Y, z1);    wall->normal(normal);   wall->textureCoord(0, 0);
	wall->quad(0,1,2,3);   wall->end();
}

void MazeApplication::createMarkers(void) {
	// mMarkerFlare
	Ogre::ColourValue color = Ogre::ColourValue::White;
	Ogre::ManualObject *marker = mSceneMgr->createManualObject();
	marker->setCastShadows(true);   marker->setDynamic(false);
	float side_2, y = FLOOR_Y + 0.1f;
	if (!mFog) {
		marker->begin("Maze/Flare", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		side_2 = CELL_SIZE/2.0f;
		marker->position(-side_2, y, -side_2);   marker->normal(Ogre::Vector3::UNIT_Y);   marker->textureCoord(0,0);   marker->colour(color);
		marker->position(-side_2, y, side_2);    marker->normal(Ogre::Vector3::UNIT_Y);   marker->textureCoord(1,0);   marker->colour(color);
		marker->position(side_2, y, side_2);     marker->normal(Ogre::Vector3::UNIT_Y);   marker->textureCoord(1,1);   marker->colour(color);
		marker->position(side_2, y, -side_2);    marker->normal(Ogre::Vector3::UNIT_Y);   marker->textureCoord(0,1);   marker->colour(color);
	} else {
		marker->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		side_2 = CELL_SIZE/4.0f;
		marker->position(-side_2, y, 0);   marker->normal(Ogre::Vector3::UNIT_Y);   marker->colour(color);
		marker->position(0, y, side_2);    marker->normal(Ogre::Vector3::UNIT_Y);   marker->colour(color);
		marker->position(side_2, y, 0);    marker->normal(Ogre::Vector3::UNIT_Y);   marker->colour(color);
		marker->position(0, y, -side_2);   marker->normal(Ogre::Vector3::UNIT_Y);   marker->colour(color);
	}
	marker->quad(0,1,2,3);   marker->end();
	mMarkerFlare= marker->convertToMesh("markerFlare");
	// floating markers
	mMarkerWhite = getMarkerMesh(Ogre::ColourValue::White,"markerWhite");
	mMarkerBlue = getMarkerMesh(Ogre::ColourValue::Blue,"markerBlue");
	// base markers
	mMarkerBaseGreen = getBaseMesh(Ogre::ColourValue::Green, "baseGreen");
	mMarkerBaseRed = getBaseMesh(Ogre::ColourValue::Red, "baseRed");
}

Ogre::MeshPtr MazeApplication::getBaseMesh(Ogre::ColourValue color, Ogre::String name) {
	// initialize
	const float SIZE_2 = (CELL_SIZE - WALL_THICKNESS) / 2.0f;
	const float x1 = -SIZE_2, z1 = -SIZE_2, x2 = SIZE_2, z2 = SIZE_2, xm = 0, zm = 0;
	const float y = FLOOR_Y + 0.1f;
	Ogre::ManualObject *base = mSceneMgr->createManualObject(name);
	base->setCastShadows(true);   base->setDynamic(false);
	base->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	// draw
	base->position(x1, y, z1);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(x1, y, zm);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(x1, y, z2);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(xm, y, z1);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(xm, y, zm);   base->colour(color);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(xm, y, z2);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(x2, y, z1);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(x2, y, zm);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->position(x2, y, z2);   base->colour(0,0,0);   base->normal(Ogre::Vector3::UNIT_Y);
	base->triangle(0,1,4);   base->triangle(0,4,3);   base->triangle(1,2,4);   base->triangle(2,5,4);
	base->triangle(3,4,6);   base->triangle(4,7,6);   base->triangle(4,5,8);   base->triangle(4,8,7);
	base->end();   return base->convertToMesh(name);
}

Ogre::MeshPtr MazeApplication::getMarkerMesh(Ogre::ColourValue color, Ogre::String name) {
	Ogre::ManualObject *marker = mSceneMgr->createManualObject();
	marker->setCastShadows(true);   marker->setDynamic(false);
	const float side_2 = CELL_SIZE/8.0f, y = 0;
	Ogre::Vector3 p1(-side_2,y,0), p2(0,y,side_2), p3(side_2,y,0), p4(0,y,-side_2), p5(0,y+side_2,0), p6(0,y-side_2,0);
	addMarkerTriangle(p1,p2,p5,color,marker);   addMarkerTriangle(p2,p3,p5,color,marker);   // upper half
	addMarkerTriangle(p3,p4,p5,color,marker);   addMarkerTriangle(p4,p1,p5,color,marker);
	addMarkerTriangle(p2,p1,p6,color,marker);   addMarkerTriangle(p3,p2,p6,color,marker);   // lower half
	addMarkerTriangle(p4,p3,p6,color,marker);   addMarkerTriangle(p1,p4,p6,color,marker);
	return marker->convertToMesh(name);
}

void MazeApplication::addMarkerTriangle(Ogre::Vector3 p1, Ogre::Vector3 p2, Ogre::Vector3 p3,
Ogre::ColourValue color, Ogre::ManualObject* obj) {
	Ogre::Vector3 dir1 = p2 - p1, dir2 = p3 - p1;
    Ogre::Vector3 normal = dir1.crossProduct(dir2).normalisedCopy();
	obj->begin("Maze/Marker", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	obj->position(p1);    obj->normal(normal);   obj->colour(color);
	obj->position(p2);    obj->normal(normal);   obj->colour(color);
	obj->position(p3);    obj->normal(normal);   obj->colour(color);
	obj->triangle(0,1,2);   obj->end();
}

// interaction with the user --------------------------------------------------

void MazeApplication::reactToMovement(void) {
	// the four recognized moods are separated by three parallel lines
	// in the plane of mTranslation (x) and mTurning (y)
	const float M = -62500;
	const float B_INTERESTED = 20000, B_CONFUSED = 40381, B_BORED = 61469;
	float tmp;
	time_t now = time(NULL);
	// initialize mTime
	if (mTime == -1) { mTime = now;   return; }
	// update message and return
	if (mTextArea->getCaption().length() > 0) {
		mTime = now;   mTurning = 0;   mTranslation = 0;
		if (now - mMessageTime >= 7) { setMessage(""); }
		return;
	}
	// update path and return
	if (mPath.size() > 0) {
		mTime = now;   mTurning = 0;   mTranslation = 0;
		updatePath();   return;
	}
	// perform new action
	if (now - mTime <= mInterval) { return; }
	mTranslation /= (float) mInterval;   mTurning /= (float) mInterval;
	tmp = M * mTranslation;
	if (mTurning < tmp + B_INTERESTED) {
		if (mDialogsInterested.size() > 0) { reactToInterested(); }
	}
	else if (mTurning < tmp + B_CONFUSED) {
		if (mDialogsConfused.size() > 0) { reactToConfused(); }
	}
	else if (mTurning < tmp + B_BORED) {
		if (mDialogsBored.size() > 0) { reactToBored(); }
	}
	else {
		if (mDialogsDesperate.size() > 0) { reactToDesperate(); }
	}
	mTime = now;   mTurning = 0;   mTranslation = 0;
}

void MazeApplication::reactToInterested(void) {
	int k;
	if (mDialogsInterested.size() > 0) { k = rand() % 2; } else { k = 0; }
	if (k == 0) {   // attract
		attract();
	} else {   // display message
		displayRandomDialog(mDialogsInterested, mIndDialogsI, mRemDialogsI);
	}
}

void MazeApplication::reactToConfused(void) {
	int k;
	if (mDialogsConfused.size() > 0) { k = rand() % 3; } else { k = rand() % 2; }
	if (k == 0) {   // attract
		attract();
	} else if (k == 1) {   // help
		help();
	} else {   // display message
		displayRandomDialog(mDialogsConfused, mIndDialogsC, mRemDialogsC);
	}
}

void MazeApplication::reactToBored(void) {
	int k;
	if (mDialogsBored.size() > 0) { k = rand() % 2; } else { k = 0; }
	if (k == 0) {   // attract
		attract();
	} else {   // display message
		displayRandomDialog(mDialogsBored, mIndDialogsB, mRemDialogsB);
	}
}

void MazeApplication::reactToDesperate(void) {
	int k;
	if (mDialogsDesperate.size() > 0) { k = rand() % 3; } else { k = rand() % 2; }
	if (k == 0) {   // attract
		attract();
	} else if (k == 1) {   // help
		help();
	} else {   // display message
		displayRandomDialog(mDialogsDesperate, mIndDialogsD, mRemDialogsD);
	}
}

// guidance -------------------------------------------------------------------

void MazeApplication::help(void) {
	setPath(mGoalRow, mGoalCol);
	if (mPath.size() >= 13) {
		displayRandomDialog(mDialogsHelp, mIndDialogsH, mRemDialogsH);
		mPath.resize( std::min((int)mPath.size()-10, 30) );
		drawPath(mMarkerWhite);
	} else { deletePath(); }   // too close to the exit
}

void MazeApplication::attract(void) {
	if (mRemCellsWithContent == 0) { return; }
	int k = rand() % mRemCellsWithContent;
	cell* selectedCell = mCellsWithContent[k];
	mCellsWithContent[k] = mCellsWithContent[mRemCellsWithContent-1];
	mCellsWithContent[mRemCellsWithContent-1] = selectedCell;
	mRemCellsWithContent--;
	if (mRemCellsWithContent == 0) {   // restore mRemCellsWithContent
		mRemCellsWithContent = mCellsWithContent.size();
	}
	setPath(selectedCell->row, selectedCell->col);
	if (mPath.size() == 0) { return; }
	setMessage(selectedCell->imageAd);   drawPath(mMarkerBlue);
}

void MazeApplication::setPath(int row2, int col2) {
	// initialize
	if (mPath.size() > 0) { deletePath(); }
	int row1 = mCurrentRow, col1 = mCurrentCol;
	cell *initialCell = &mGrid[row1][col1], *goalCell = &mGrid[row2][col2];
	if (initialCell == goalCell) { return; }
	// find a path (depth-first search)
	int index, rem, tmp1, tmp2;
	cell *currentCell, *neighborCell;
	initialCell->parent = NULL;   initialCell->visited = true;
	std::vector<cell*> visitList;   visitList.push_back(initialCell);   index = 0;
	while (index < (int) visitList.size()) {
		currentCell = visitList[index];   rem = currentCell->remNeighbors;
		if (rem == 0) { index++; }
		else {   // go to a neighbor cell
			neighborCell = currentCell->neighbors[rem-1];
			currentCell->remNeighbors--;
			if ( (!neighborCell->visited) && (connected(currentCell,neighborCell)) ) {
				neighborCell->parent = currentCell;   neighborCell->visited = true;
				visitList.push_back(neighborCell);
				if (neighborCell == goalCell) {
					// set path
					currentCell = goalCell;
					do {
						mPath.push_back(currentCell);   currentCell = currentCell->parent;
					} while (currentCell != NULL);
					tmp1 = 0;   tmp2 = mPath.size() - 1;
					while (tmp1 < tmp2) {
						currentCell = mPath[tmp1];   mPath[tmp1] = mPath[tmp2];   mPath[tmp2] = currentCell;
						tmp1++;   tmp2--;
					}
					// clean up
					for (tmp1 = 0; tmp1 <= index; tmp1++) {
						currentCell = visitList[tmp1];   currentCell->visited = false;
						currentCell->remNeighbors = currentCell->totalNeighbors;
					}
					tmp2 = visitList.size();
					for (tmp1 = index+1; tmp1 < tmp2; tmp1++) { visitList[tmp1]->visited = false; }
					return;
				}
			}
		}
	}
	// failed to find a path; clean up
	tmp2 = visitList.size();
	for (tmp1 = 0; tmp1 < tmp2; tmp1++) {
		currentCell = visitList[tmp1];   currentCell->visited = false;
		currentCell->remNeighbors = currentCell->totalNeighbors;
	}
}

bool MazeApplication::connected(cell *c1, cell *c2) {
	if (c1->row == c2->row) {
		if (c1->col == c2->col - 1) { return c1->openRight; }
		else if (c1->col == c2->col + 1) { return c2->openRight; }
		return false;
	} else if (c1->col == c2->col) {
		if (c1->row == c2->row - 1) { return c1->openUp; }
		else if (c1->row == c2->row + 1) { return c2->openUp; }
		return false;
	}
	return false;
}

void MazeApplication::drawPath(Ogre::MeshPtr &meshPtr) {
	int i, n = mPath.size();
	for (i = 0; i < n; i++) {
		drawMarkerOnCell(mPath[i]->row, mPath[i]->col, meshPtr);
	}
	mPathTime1 = time(NULL);   mPathTime2 = mPathTime1;
}

void MazeApplication::updatePath(void) {
	time_t now = time(NULL);
	if (now == mPathTime2) { return; }   // no update needed
	int i, j, n = mPath.size();
	cell* currentCell = &mGrid[mCurrentRow][mCurrentCol];
	for (i = 0; i < n; i++) { if (mPath[i] == currentCell) {   // standing on the path
		if (i > 0) {
			for (j = 0; j < i; j++) { eraseMarkerOnCell(mPath[j]->row, mPath[j]->col); }
			mPath.erase( mPath.begin(), mPath.begin()+i );
		}
		mPathTime1 = now;   mPathTime2 = now;   return;
	} }
	// delete path
	if (now-mPathTime1 >= 15) { deletePath();   return; }
	// shrink markers
	mPathTime2 = now;   return;   // disabled
	if (mPath.size() > 200) { mPathTime2 = now;   return; }
	Ogre::SceneNode *node;   n = mPath.size();
	for (i = 0; i < n; i++) {
		std::stringstream nameS;
		nameS << "marker_" << mPath[i]->row << "_" << mPath[i]->col;
		try {
			node = mSceneMgr->getSceneNode(nameS.str());
			node->scale(0.95f, 0.95f, 0.95f);
		} catch (...) {}
	}
	mPathTime2 = now;
}

void MazeApplication::deletePath(void) {
	int n = mPath.size();
	for (int i = 0; i < n; i++) {
		eraseMarkerOnCell(mPath[i]->row, mPath[i]->col);
	}
	mPath.clear();
}

void MazeApplication::drawMarkerOnCell(int row, int col, Ogre::MeshPtr &meshPtr) {
	// if the marker already exists, delete it
	eraseMarkerOnCell(row, col);
	// draw marker
	std::stringstream nameS;   nameS << "marker_" << row << "_" << col;
	Ogre::String name = nameS.str();
	Ogre::SceneNode *node = mMazeNode->createChildSceneNode(name);
	node->attachObject( mSceneMgr->createEntity(meshPtr->getName()) );
	float x = (row+0.5f)*CELL_SIZE, y = (CEIL_Y+FLOOR_Y)/2.0f, z = (col+0.5f)*CELL_SIZE;
	node->translate(x,y,z);
}

void MazeApplication::eraseMarkerOnCell(int row, int col) {
	std::stringstream nameS;   nameS << "marker_" << row << "_" << col;
	Ogre::SceneNode *node;
	Ogre::MovableObject *obj;
	try { node = mSceneMgr->getSceneNode(nameS.str());   obj = node->getAttachedObject(0); }
	catch (...) { return; }
	obj->detachFromParent();   mSceneMgr->destroyMovableObject(obj);
	mSceneMgr->destroySceneNode(node);
}

// dialogs --------------------------------------------------------------------

void MazeApplication::createOverlays(void) {
	Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
	// bottom text area
	// panel
	Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(
		overlayManager.createOverlayElement("Panel", "Panel1"));
	panel->setMetricsMode(Ogre::GMM_RELATIVE);
	panel->setVerticalAlignment(Ogre::GVA_BOTTOM);
	panel->setHorizontalAlignment(Ogre::GHA_LEFT);
	panel->setDimensions(0.9f, FONT_SIZE);   panel->setPosition(0,0);
	//panel->setMaterialName("BaseWhiteNoLighting");
	// text area
	mTextArea = static_cast<Ogre::TextAreaOverlayElement*>(
		overlayManager.createOverlayElement("TextArea", "TextArea1"));
	mTextArea->setMetricsMode(Ogre::GMM_RELATIVE);
	mTextArea->setPosition(0.0f, 0.0f);   mTextArea->setDimensions(1, 1);
	try { mTextArea->setFontName("BlueHighway12"); }
	catch (...) { mLogFile << "Couldn't load font for overlay" << std::endl; }
	mTextArea->setCharHeight(FONT_SIZE);   mTextArea->setSpaceWidth(FONT_SIZE/2);
	mTextArea->setColour(Ogre::ColourValue::White);
	//mTextArea->setColourBottom(Ogre::ColourValue(0.7f, 0.7f, 0.7f));
	//mTextArea->setColourTop(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	// overlay
	Ogre::Overlay* mOverlay = overlayManager.create("Overlay1");
	panel->addChild(mTextArea);   mOverlay->add2D(panel);   mOverlay->show();
	// top text area
	// panel
	panel = static_cast<Ogre::OverlayContainer*>(
		overlayManager.createOverlayElement("Panel", "Panel2"));
	panel->setMetricsMode(Ogre::GMM_RELATIVE);
	panel->setVerticalAlignment(Ogre::GVA_TOP);
	panel->setHorizontalAlignment(Ogre::GHA_LEFT);
	panel->setDimensions(0.9f, FONT_SIZE);   panel->setPosition(FONT_SIZE,FONT_SIZE);
	//panel->setMaterialName("BaseWhiteNoLighting");
	// text area
	mTextAreaTop = static_cast<Ogre::TextAreaOverlayElement*>(
		overlayManager.createOverlayElement("TextArea", "TextArea2"));
	mTextAreaTop->setMetricsMode(Ogre::GMM_RELATIVE);
	mTextAreaTop->setPosition(0.0f, 0.0f);   mTextAreaTop->setDimensions(1, 1);
	try { mTextAreaTop->setFontName("BlueHighway12"); }
	catch (...) { mLogFile << "Couldn't load font for overlay" << std::endl; }
	mTextAreaTop->setCharHeight(FONT_SIZE);   mTextAreaTop->setSpaceWidth(FONT_SIZE/2);
	mTextAreaTop->setColour(Ogre::ColourValue(1.0f,1.0f,0.0f));
	//mTextAreaTop->setColourBottom(Ogre::ColourValue(0.7f, 0.7f, 0.7f));
	//mTextAreaTop->setColourTop(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	// overlay
	mOverlay = overlayManager.create("Overlay2");
	panel->addChild(mTextAreaTop);   mOverlay->add2D(panel);   mOverlay->show();
	// fix a bug that prevents the rendering of text in the overlay, until the window is resized
	mRoot->renderOneFrame();
}

void MazeApplication::readDialogs(const char* fileName, std::vector< std::string > &dialogs,
std::vector< int > &ind, int &rem) {
	ind.clear();   rem = 0;
	std::ifstream dialogsFile(fileName);
	if (!dialogsFile.is_open()) { return; }
	std::string line;
	do {
		getline(dialogsFile, line);
		if (line.length() > 0) { /*removeSpecialChars(line);*/   dialogs.push_back(line); }
	} while (dialogsFile.good());
	dialogsFile.close();   rem = dialogs.size();   ind.reserve(rem);
	for (int i = 0; i < rem; i++) { ind.push_back(i); }
}

void MazeApplication::displayRandomDialog(std::vector< std::string > &dialogs,
std::vector< int > &ind, int &rem) {
	if (rem == 0) { return; }   // no available dialogs
	int i = rand() % rem, dialogI;
	dialogI = ind[i];   ind[i] = ind[rem-1];   ind[rem-1] = dialogI;
	rem--;   setMessage(dialogs[dialogI]);
	// if all dialogs have been displayed, restart rem
	if (rem == 0) { rem = ind.size(); }
}

void MazeApplication::setMessage(std::string str) {
	if (str.length() == 0) { mTextArea->setCaption("");   return; }
	int lines;   formatMessage(str, lines);
	if (str.length() == 0) { mTextArea->setCaption("");   return; }
	mTextArea->getParent()->setPosition(FONT_SIZE, -(lines+1)*FONT_SIZE);
	mTextArea->setCaption(str);   mMessageTime = time(NULL);
}

void MazeApplication::setTopMessage(std::string str) {
	if (str.length() == 0) { mTextAreaTop->setCaption("");   return; }
	int lines;   formatMessage(str, lines);
	if (str.length() == 0) { mTextAreaTop->setCaption("");   return; }
	mTextAreaTop->setCaption(str);
}

void MazeApplication::formatMessage(std::string &str, int &lines) {
	lines = 0;   if (str.length() == 0) { return; }
	int maxChars = std::floor(3.4f / FONT_SIZE);
	if (maxChars == 0) { str = "";   return; }
	// parse variables
	std::string search = "$pathSize";
	std::stringstream strS;   strS << mPath.size();
	std::string replace = strS.str();
	size_t pos = str.find(search);
	while (pos != std::string::npos) {
		str.replace(pos, search.length(), replace);
		pos = str.find(search, pos+1);
	}
	// line wrap
	std::stringstream caption;
	int chars = 0, index1 = 0, index2, n = str.length(), tmp;
	while ( ((str[index1] == ' ') || (str[index1] == '\t')) && (index1 < n) ) { index1++; }
	if (index1 == n) { str = "";   return; }
	index2 = index1;
	do {
		index2 = std::min(index1+maxChars, n);
		if (index2 < n) {
			tmp = index2;
			while ( (str[index2-1] != ' ') && (index2 > index1) ) { index2--; }
			if (index2 == index1) { index2 = tmp; }
		}
		caption << str.substr(index1, index2-index1) << std::endl;
		index1 = index2;   lines++;   chars = 0;
	} while (index1 < n);
	str = caption.str();
}

void MazeApplication::removeSpecialChars(std::string &str) {
	char c;
	int i, n = str.length();
	for (i = 0; i < n; i++) {
		c = str[i];   if ( (c < 32) || (c > 126) ) { str[i] = '_'; }
	}
}

// images ---------------------------------------------------------------------

void MazeApplication::loadImages(void) {
	mRemCellsWithContent = 0;
	std::ifstream imagesFile("Maze/images.txt");
	if (!imagesFile.is_open()) { return; }
	// initialize available cells
	std::vector<cell*> availableCells;   availableCells.reserve(mGridSize*mGridSize);
	int i, j;
	for (i = 0; i < mGridSize; i++) { for (j = 0; j < mGridSize; j++) {
		if ( ((i != mInitialRow) || (j != mInitialCol)) && ((i != mGoalRow) || (j != mGoalCol)) ) {
			availableCells.push_back(&mGrid[i][j]);
		}
	} }
	// load images
	Ogre::NameGenerator names ("image");
	std::string line, fileName, ad, caption;
	int section = 0;
	do {
		getline(imagesFile, line);
		if (line.length() > 0) {
			if (section == 0) { fileName = line;   section++; }   // file name
			else if (section == 1) { ad = line;   section++; }   // ad
			else {   // caption
				caption = line;
				addImageToRandomCell(fileName, ad, caption, &names, availableCells);
				section = 0;
			}
		}
	} while (imagesFile.good());
	mRemCellsWithContent = mCellsWithContent.size();   imagesFile.close();
}

void MazeApplication::addImageToRandomCell(std::string &fileName, std::string &ad,
std::string &caption, Ogre::NameGenerator *names, std::vector<cell*> &availableCells) {
	const float SIZE = CELL_SIZE - WALL_THICKNESS - 0.2f;   // also used in getRandomWall
	const float FLOOR = FLOOR_Y + 0.1f, CEIL = FLOOR + SIZE;
	// get a random wall
	float x1, z1, x2, z2;   cell* selectedCell;   int selectedInd;   Ogre::Vector3 normal;
	if ( !getRandomWall(availableCells,selectedCell,selectedInd,x1,z1,x2,z2,normal) ) { return; }
	std::string path = "images/";   path.append(fileName);
	try {
		Ogre::String name = names->generate();
		// create material
		Ogre::MaterialPtr mMat = Ogre::MaterialManager::getSingleton().create(name, "General", true);
		Ogre::TexturePtr mTex = Ogre::TextureManager::getSingleton().load(path, "General");
		mMat->getTechnique(0)->getPass(0)->createTextureUnitState()->setTextureName(mTex->getName());
		mMat->setLightingEnabled(false);
		// create square
		Ogre::ManualObject *obj = mSceneMgr->createManualObject();
		obj->setCastShadows(true);   obj->setDynamic(false);
		obj->begin(mMat->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);
		obj->position(x1, FLOOR, z1);   obj->normal(normal);   obj->textureCoord(0,1);
		obj->position(x2, FLOOR, z2);   obj->normal(normal);   obj->textureCoord(1,1);
		obj->position(x2, CEIL, z2);    obj->normal(normal);   obj->textureCoord(1,0);
		obj->position(x1, CEIL, z1);    obj->normal(normal);   obj->textureCoord(0,0);
		obj->quad(0,1,2,3);   obj->end();   obj->convertToMesh(name);
		mMazeNode->attachObject( mSceneMgr->createEntity(name) );
		availableCells[selectedInd] = availableCells[availableCells.size()-1];
		availableCells.pop_back();   mCellsWithContent.push_back(selectedCell);
		selectedCell->imageAd = ad;   selectedCell->imageCaption = caption;
		// draw flare
		int row = selectedCell->row, col = selectedCell->col;
		if ( ((row != mInitialRow) || (col != mInitialCol)) && ((row != mGoalRow) || (col != mGoalCol)) ) {
			drawMeshAt((row+0.5f)*CELL_SIZE, (col+0.5f)*CELL_SIZE, mMarkerFlare, mMazeNode);
		}
	} catch (...) { mLogFile << "unable to load " << path << std::endl; }
}

bool MazeApplication::getRandomWall(std::vector<cell*> &availableCells, cell* &selectedCell,
int &selectedInd, float &x1, float &z1, float &x2, float &z2, Ogre::Vector3 &normal) {
	const float OFFSET = WALL_THICKNESS_2+0.1f, SIZE = CELL_SIZE-2*OFFSET;   // also used in addImageToRandomCell
	float xBottom, zLeft;
	int nCells = availableCells.size(), cellInd, dirInd, remDirs, tmp;
	int dirs[] = {0, 1, 2, 3};
	cell *currentCell;
	bool hasWall;
	while (nCells > 0) {
		cellInd = rand() % nCells;   remDirs = 4;
		currentCell = availableCells[cellInd];
		xBottom = CELL_SIZE*(currentCell->row)+OFFSET;
		zLeft = CELL_SIZE*(currentCell->col)+OFFSET;
		do {
			dirInd = rand() % remDirs;
			if (dirs[dirInd] == 0) {   // up wall
				if (!currentCell->openUp) {
					selectedCell = currentCell;   selectedInd = cellInd;
					x1 = xBottom + SIZE;   z1 = zLeft;   x2 = x1;   z2 = z1+SIZE;
					normal = Ogre::Vector3::NEGATIVE_UNIT_X;   return true;
				}
			} else if (dirs[dirInd] == 1) {   // right wall
				if (!currentCell->openRight) {
					selectedCell = currentCell;   selectedInd = cellInd;
					x1 = xBottom + SIZE;   z1 = zLeft + SIZE;   x2 = xBottom;   z2 = z1;
					normal = Ogre::Vector3::NEGATIVE_UNIT_Z;   return true;
				}
			} else if (dirs[dirInd] == 2) {   // bottom wall
				if (currentCell->row == 0) { hasWall = true; }
				else { hasWall = !(mGrid[currentCell->row-1][currentCell->col].openUp); }
				if (hasWall) {
					selectedCell = currentCell;   selectedInd = cellInd;
					x1 = xBottom;   z1 = zLeft + SIZE;   x2 = x1;   z2 = zLeft;
					normal = Ogre::Vector3::UNIT_X;   return true;
				}
			} else {   // left wall
				if (currentCell->col == 0) { hasWall = true; }
				else { hasWall = !(mGrid[currentCell->row][currentCell->col-1].openRight); }
				if (hasWall) {
					selectedCell = currentCell;   selectedInd = cellInd;
					x1 = xBottom;   z1 = zLeft;   x2 = x1 + SIZE;   z2 = zLeft;
					normal = Ogre::Vector3::UNIT_Z;   return true;
				}
			}
			tmp = dirs[dirInd];   dirs[dirInd] = dirs[remDirs-1];
			dirs[remDirs-1] = tmp;   remDirs--;
		} while (remDirs > 0);
		availableCells[cellInd] = availableCells[nCells-1];
		availableCells.pop_back();   nCells--;
	}
	return false;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        MazeApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
