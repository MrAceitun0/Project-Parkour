#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"

class Player;
class Floor;

class ofApp : public ofBaseApp{
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

		ofEasyCam cam;

		Player* player;
		Floor* floor;
};

class Player
{
	public:
		Player();
		ofNode node;
		glm::vec3 position = glm::vec3(0,0,0);
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