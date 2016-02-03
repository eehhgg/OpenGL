
#include "stdafx.h"

#ifndef __MazeApplication_h_
#define __MazeApplication_h_

#include "BaseApplication.h"

struct cell {
	int col, row, remNeighbors, totalNeighbors;
	cell* neighbors[4];
	bool visited, openUp, openRight;
	cell* parent;   // used for graph traversal
	// image
	std::string imageCaption, imageAd;
};

class MazeApplication : public BaseApplication {
public:
    MazeApplication(void);
    virtual ~MazeApplication(void);

protected:
	// fields
	std::ofstream mLogFile;
	float mTranslation, mTurning;
	time_t mTime, mMessageTime, mPathTime1, mPathTime2;
	int mInitialCol, mInitialRow, mGoalCol, mGoalRow, mCurrentCol, mCurrentRow;
	std::vector< std::vector< cell > > mGrid;
	std::vector< cell* > mPath;
	std::vector< cell* > mCellsWithContent;
	int mRemCellsWithContent;
	Ogre::SceneNode *mMazeNode;
	Ogre::TextAreaOverlayElement *mTextArea, *mTextAreaTop;
	Ogre::MeshPtr mMarkerWhite, mMarkerBlue, mMarkerFlare, mMarkerBaseGreen, mMarkerBaseRed;
	// dialog fields
	std::vector< std::string > mDialogsInterested, mDialogsConfused;
	std::vector< std::string > mDialogsBored, mDialogsDesperate, mDialogsHelp;
	int mRemDialogsI, mRemDialogsC, mRemDialogsB, mRemDialogsD, mRemDialogsH;
	std::vector< int > mIndDialogsI, mIndDialogsC, mIndDialogsB, mIndDialogsD, mIndDialogsH;
	// overriden methods of BaseApplication
	void createCamera(void);
    void createScene(void);
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	bool mouseMoved(const OIS::MouseEvent &arg);
	// maze generation
	void generateMaze(void);
	void initializeGrid(void);
	int collision(Ogre::Vector3 &,Ogre::Vector3 &);
	bool hasTopRightCorner(int,int);
	// maze drawing
	void drawMaze(void);
	void drawMeshAt(float,float,Ogre::MeshPtr,Ogre::SceneNode*);
	Ogre::MeshPtr getWallMesh(float,float,float,float,Ogre::String);
	void addWallPlane(float,float,float,float,Ogre::Vector3,float,Ogre::ManualObject*);
	void createMarkers(void);
	Ogre::MeshPtr getBaseMesh(Ogre::ColourValue,Ogre::String);
	Ogre::MeshPtr getMarkerMesh(Ogre::ColourValue,Ogre::String);
	void addMarkerTriangle(Ogre::Vector3,Ogre::Vector3,Ogre::Vector3, Ogre::ColourValue,Ogre::ManualObject*);
	// interaction with the user
	void reactToMovement(void);
	void reactToInterested(void);
	void reactToConfused(void);
	void reactToBored(void);
	void reactToDesperate(void);
	// guidance
	void help(void);
	void attract(void);
	void setPath(int,int);
	bool connected(cell*,cell*);
	void drawPath(Ogre::MeshPtr &);
	void updatePath(void);
	void deletePath(void);
	void drawMarkerOnCell(int,int,Ogre::MeshPtr &);
	void eraseMarkerOnCell(int,int);
	// dialogs
	void createOverlays(void);
	void readDialogs(const char*,std::vector< std::string > &,std::vector< int > &,int &);
	void displayRandomDialog(std::vector< std::string >&,std::vector< int > &,int &);
	void setMessage(std::string);
	void setTopMessage(std::string);
	void formatMessage(std::string &,int &);
	void removeSpecialChars(std::string &);
	// images
	void loadImages(void);
	void addImageToRandomCell(std::string &,std::string &,std::string &,Ogre::NameGenerator*,std::vector<cell*> &);
	bool getRandomWall(std::vector<cell*> &,cell* &,int &,float &,float &,float &,float &,Ogre::Vector3 &);
};

#endif // #ifndef __MazeApplication_h_
