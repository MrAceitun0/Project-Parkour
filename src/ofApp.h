#pragma once

#include "ofMain.h"

class Player;
class Floor;
class Box;

//#define NUITRACK

#ifdef NUITRACK
#include <nuitrack/Nuitrack.h>
#include "ofxNuitrack.h"
#endif

#include "ofxGLWarper.h"
#include "ofxUI.h"
#include "ofxXmlSettings.h"
#include "ofxTimer.h"

#include "DEFINES.h"
#include "Singleton.h"

#include "grid/grid.h"
#include "botons/botoDonut.h"
#include "peces/pecaEmpty.h"

#ifdef NUITRACK
using namespace tdv::nuitrack;

class Bone {
public:
	Bone(JointType _from, JointType _to, glm::vec3 _direction) {
		from = _from;
		to = _to;
		direction = _direction;
	}

	JointType from;
	JointType to;
	glm::vec3 direction;
};
#endif

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	bool isFloor(glm::vec3 p_position);
	bool isBox(glm::vec3 p_position);

#ifdef NUITRACK		
	//--------------------------------------------------------------
	// REALSENSE
	void updatePointcloud();
	void updateJoint(Joint &j);
	void drawSkeleton(Skeleton &s);
	void drawJoint(Joint &j);
	void drawBones(vector<Joint> joints);
	void drawUsers();
	void drawPointcloud();

	bool bNeedPointcloudUpdate;
	ofxnui::TrackerRef tracker;
	ofTexture rgbTex;
	ofTexture depthTex;
	glm::vec2 rgbFrameSize;
	glm::vec2 depthFrameSize;

	vector<Skeleton> skeletons;
	vector<User> users;

	ofVboMesh pc;				// pointcloud

	float cameraHeight = 1.0;	// height of camera in realspace (m)
	glm::vec3 floorPoint;
	glm::vec3 floorNormal;
#endif

	//--------------------------------------------------------------
	// APP
	Singleton *singleton; // PUNTUACIÓ jugador

	pantallesJoc pantallaJoc;
	string pantallaToString();

	void setVariablesIniciPartida(); // endreçat
	void drawPuntuacio();
	//void drawEnd();

	// CORNER PIN
	ofxGLWarper warper;

	// BOTONS
	botoDonut botoStart;

	// GRID
	grid myGrid;

	// TEMPS DE JOC
	ofxTimer jocMinutsTimer;
	float jocMinutsTimerSegonsLeft;
	int jocMinutsTimerMinuts;
	int jocMinutsTimerSegons;
	void drawTemps();
	ofColor saltingBlue;
	ofTrueTypeFont	saltingTypo;

	// TEMPS DE PANTALLA FINAL
	ofxTimer duradaTheEndTimer;

	//--------------------------------------------------------------
	// PUNTER
	ofImage punter;
	float punterWidthMig, punterHeightMig;

	// PECES
	int comptadorPeces;
	pecaEmpty peca1;
	void setupPeca1();
	void comprobarEstatsPecesEmpty();
	void actualitzaPuntsEmpty(int & e);

	//--------------------------------------------------------------
	// DETECCIÓ
	float relAspectWidth; // detecció
	float relAspectHeight;
	float baixaHoTotAvall; // ajust fi amunt-avall
	float mouHoTotDretaEsq; // ajust fi esquerra-dreta

	bool bshowImagesAndSkeleton = false;
	bool bshowCamera = false;

	// contorns
	float tmpX, tmpY;
	ofVec4f tmpVecBlobPos;
	ofVec2f warpMousePos;

	//blobs
	ofVec2f posicionsBlobs[MAX_NUM_BLOBS];
	int totalBlobsDetected;

	// GUI APP i DETECCIO
	ofxUICanvas *guia;

	// GUI AJUST BOTONS
	ofxUICanvas *guib;

	// GUI HELP
	ofxUICanvas *guih;
	void guiEvent(ofxUIEventArgs &e); // per a tots els GUIs

	//--------------------------------------------------------------
	// HELP INFO
	void toogleDrawInfoGrid();
	bool bdrawMouseWarped = true;

	//--------------------------------------------------------------
	ofEasyCam cam;

	ofImage menu_image;

	Player* player = NULL;
	list<Floor*> floors;
	list<Box*> boxes;
	
	glm::vec3 actual_position;

	//bool is_slide = false;

	list<Floor> easy_floors;
	list<Box> easy_boxes;
	void generateEasyLevel();
	
	list<Floor> hard_floors;
	list<Box> hard_boxes;
	void generateHardLevel();

	void drawMenu();
	void drawLevel();
	void drawDeath();
	void drawEnd();

	ofImage menuImage;
	ofImage day;
	ofImage night;

	void restartGame();
};

class Player
{
public:
	Player();

	ofSoundPlayer normalSound;
	ofSoundPlayer highSound;
	ofSoundPlayer slideSound;
	ofSoundPlayer winSound;
	ofSoundPlayer deathSound;

	ofNode node;
	glm::vec3 position = glm::vec3(0, 0, 0);

	float yVelocity = 10;
	float zVelocity = 0;
	float gravity;
	float hitbox = 30;

	void render();
	void update();

	bool falling = false;
	bool jumping = false;
	bool collision = false;
	bool is_slide = false;
	bool sliding = false;
	bool slideDown = false;
	bool slideUp = false;

	int myJoints[24];

	int game_mode = 0;

	void normalJump();
	void highJump();
	void throughJump();
	void slide();
};

class Floor
{
public:
	Floor();
	glm::vec3 position;
	glm::vec2 size;
	glm::vec3 color;

	float size_;

	void render();

	void render2();
};

class Box
{
public:
	Box();
	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 color;

	float size_;

	void render();

	void render2();
};
