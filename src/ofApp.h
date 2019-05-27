#pragma once

#include "ofMain.h"

class Player;
class Floor;

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

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

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

	ofEasyCam cam;

	Player* player;
	Floor* floor;
};

class Player
{
public:
	Player();
	ofNode node;
	glm::vec3 position = glm::vec3(0, 0, 0);
	float yVelocity = 1;
	float zVelocity = 0;
	float gravity;
	float hitbox = 30;
	void render();
	void update();
};

class Floor
{
public:
	Floor();
	glm::vec3 position;
	float size;
	void render();
};