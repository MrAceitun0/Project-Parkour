#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofEnableDepthTest();
	ofSetVerticalSync(true);

	cam.setGlobalPosition(glm::vec3(0, 0, 0));
	cam.tilt(70);

	player = new Player();
	floor = new Floor();

	player->gravity = -10.0;
}

//--------------------------------------------------------------
void ofApp::update(){
	/*
	if (zVelocity <= 0)
		zVelocity = 0;
	if(cam.getGlobalPosition().z > 225)
		zVelocity = zVelocity + gravity;

	cam.setPosition(cam.getGlobalPosition().x, cam.getGlobalPosition().y + yVelocity, cam.getGlobalPosition().z + zVelocity);
	*/
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofDrawAxis(100);
	cam.begin();

	player->render();
	floor->render();

	cam.end(); 
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	if (key == 's')
		player->zVelocity = 25.0;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

Player::Player()
{
	position = glm::vec3(0, 0, 15);
}

void Player::render()
{
	if (zVelocity < 0)
		zVelocity = 0;
	else if (position.z > 15)
		zVelocity += gravity;

	ofSetColor(0, 255, 0);
	ofFill();
	position = glm::vec3(0, position.y + yVelocity, position.z + zVelocity);
	ofDrawSphere(position, 15);
}

void Player::update()
{
}

Floor::Floor()
{
	
}

void Floor::render()
{
	ofSetColor(255, 0, 0);
	ofFill();
	ofDrawPlane(position, 100, 100);
}