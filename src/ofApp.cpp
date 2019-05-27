#include "ofApp.h"

#ifdef OPEN_MP_TEST
#include <omp.h>
#endif

#ifdef NUITRACK
using namespace tdv::nuitrack;
#endif

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup() {
	ofEnableDepthTest();
	ofSetVerticalSync(true);

	cam.setGlobalPosition(glm::vec3(0, 0, 0));
	cam.tilt(70);

	player = new Player();
	floor = new Floor();

	player->gravity = -10.0;

#ifdef NUITRACK
	// REALSENSE
	bool bPlayFile = true;
	pc.setMode(OF_PRIMITIVE_POINTS);

	tracker = ofxnui::Tracker::create();
	tracker->init();
	tracker->setConfigValue("Realsense2Module.Depth.FPS", "30");
	//tracker->setConfigValue("Realsense2Module.Depth.RawWidth", "1280");
	//tracker->setConfigValue("Realsense2Module.Depth.RawHeight", "720");
	//tracker->setConfigValue("Realsense2Module.Depth.ProcessWidth", "1280");
	//tracker->setConfigValue("Realsense2Module.Depth.ProcessHeight", "720");
	tracker->setConfigValue("Realsense2Module.Depth.ProcessMaxDepth", "7000");
	tracker->setConfigValue("DepthProvider.RotateAngle", "90");

	tracker->setConfigValue("Realsense2Module.RGB.FPS", "30");
	//tracker->setConfigValue("Realsense2Module.RGB.RawWidth", "1280");
	//tracker->setConfigValue("Realsense2Module.RGB.RawHeight", "720");
	//tracker->setConfigValue("Realsense2Module.RGB.ProcessWidth", "1280");
	//tracker->setConfigValue("Realsense2Module.RGB.ProcessHeight", "720");

	// post processing setting
	tracker->setConfigValue("Realsense2Module.Depth.PostProcessing.SpatialFilter.spatial_alpha", "0.1");
	tracker->setConfigValue("Realsense2Module.Depth.PostProcessing.SpatialFilter.spatial_delta", "50");

	if (bPlayFile) {
		string path = ofToDataPath("video.bag", true); // must be absolute path
		tracker->setConfigValue("Realsense2Module.FileRecord", path);
	}

	tracker->setConfigValue("Segmantation.MAX_DISTANCE", "10000");
	tracker->setConfigValue("Skeletonization.MaxDistance", "10000");

	tracker->setIssuesCallback([this](IssuesData::Ptr data) {
		auto issue = data->getIssue<Issue>();
		if (issue) {
			ofLogNotice() << "Issue detected! " << issue->getName() << " [" << issue->getId() << "]";
		}
	});

	tracker->setRGBCallback([this](RGBFrame::Ptr data) {
		rgbFrameSize.x = data->getCols();
		rgbFrameSize.y = data->getRows();
		const Color3 *colorData = data->getData();
		uint8_t *colorDataPtr = (uint8_t *)colorData;
		ofPixels pix;

		string fileName = tracker->getConfigValue("Realsense2Module.FileRecord");
		if (fileName == "") {
			//Live feed = BGR
			pix.setFromPixels(colorDataPtr, rgbFrameSize.x, rgbFrameSize.y, OF_PIXELS_BGR);
		}
		else {
			// BAG file = RGB
			pix.setFromPixels(colorDataPtr, rgbFrameSize.x, rgbFrameSize.y, OF_PIXELS_RGB);
		}
		rgbTex.loadData(pix);
	});

	tracker->setDepthCallback([this](DepthFrame::Ptr data) {
		depthFrameSize.x = data->getCols();
		depthFrameSize.y = data->getRows();
		const unsigned short *depthData = data->getData();
		ofShortPixels pix;
		pix.setFromPixels(depthData, depthFrameSize.x, depthFrameSize.y, OF_PIXELS_GRAY);
		depthTex.loadData(pix);
		bNeedPointcloudUpdate = true;
	});


	tracker->setSkeletonCallback([this](SkeletonData::Ptr data) {
		skeletons = data->getSkeletons();
	});

	tracker->setUserCallback([this](UserFrame::Ptr data) {
		floorPoint = ofxnui::Tracker::fromVector3(data->getFloor()) / vec3(1000);
		floorNormal = ofxnui::Tracker::fromVector3(data->getFloorNormal());
		users = data->getUsers();
	});

	tracker->run();
#endif
}

//--------------------------------------------------------------
void ofApp::update() 
{
#ifdef NUITRACK
	// TRACKER --------------------------------------------
	tracker->poll();
	updatePointcloud();

	totalBlobsDetected = 0; // s'actualitza dins el updateJoint
	for (int i = 0; i < skeletons.size(); i++) {
		auto s = skeletons[i];
		auto joints = s.joints;
		for (int n = 0; n < joints.size(); n++) {
			auto j = joints[n];
			updateJoint(j);
		}
	}
#endif

	/*
	if (zVelocity <= 0)
		zVelocity = 0;
	if(cam.getGlobalPosition().z > 225)
		zVelocity = zVelocity + gravity;

	cam.setPosition(cam.getGlobalPosition().x, cam.getGlobalPosition().y + yVelocity, cam.getGlobalPosition().z + zVelocity);
	*/
}

#ifdef NUITRACK
//--------------------------------------------------------------
void ofApp::updatePointcloud() {
	if (bNeedPointcloudUpdate) {

		DepthSensor::Ptr dtracker = tracker->depthTracker;
		DepthFrame::Ptr dframe = dtracker->getDepthFrame();
		int row = dframe->getRows();
		int col = dframe->getCols();
		int skip = 4;	// downsampling level, 1 = max
		int size = ((float)row / skip) * ceil((float)col / skip);

		// allocate vertices and colors only once
		if (pc.getVertices().size() == 0) {
			pc.clear();

			vector<glm::vec3> p;
			p.assign(size, glm::vec3(0, 0, 0));
			pc.addVertices(p);

			vector<ofFloatColor> c;
			c.assign(size, ofFloatColor(0, 0, 0, 0.9));
			pc.addColors(c);
		}

		const unsigned short * data = dframe->getData();

#ifdef OPEN_MP_TEST
#pragma omp parallel for num_threads(4)
#endif
		for (int y = 0; y < row; y += skip) {
			for (int x = 0; x < col; x += skip) {
				int index = y * col + x;
				int skippedIndex = (y / skip) * (col / skip) + (x / skip);
				unsigned short d = data[index];
				Vector3 v = dtracker->convertProjToRealCoords(x, y, d);
				pc.setVertex(skippedIndex, glm::vec3(v.x, v.y, v.z)*0.001);
				if (v.y > -900) {
					float g = ofMap(d, 0, 5000, 0.1, 1.0, true);
					pc.setColor(skippedIndex, ofFloatColor(g, g, 0, 0.9)); // groc
				}
				else {
					pc.setColor(skippedIndex, ofFloatColor(0, 0, 0, 0));
				}
			}
		}

		bNeedPointcloudUpdate = false;
	}
}
#endif

//--------------------------------------------------------------
void ofApp::draw()
{
	ofDrawAxis(100);
	cam.begin();

	player->render();
	floor->render();

	cam.end();

#ifdef NUITRACK
	// CAMERA
	if (bshowCamera) {
		ofSetColor(255);
		if (rgbTex.isAllocated()) {
			rgbTex.draw(0, 0, APP_WIDTH, APP_HEIGT);
		}
	}
#endif

#ifdef NUITRACK
	// INFO ------------------------------------------------------------------------
	// CAMERES & STICKMAN & PRINTS
	if (bshowImagesAndSkeleton) {
		float camWidthPorc = rgbFrameSize.x*0.25;
		float camHeightPorc = rgbFrameSize.y*0.25;
		ofSetColor(255);
		if (rgbTex.isAllocated()) {
			rgbTex.draw(400, 20, camWidthPorc, camHeightPorc);
		}
		if (depthTex.isAllocated()) {
			depthTex.draw(400 + camWidthPorc + 20, 20, camWidthPorc, camHeightPorc);
		}

		ofSetHexColor(0xffffff);
		ofDrawBitmapString("camera image", 400, 10);
		ofDrawBitmapString("depth texture", 400 + camWidthPorc + 20, 10);

		//drawPointcloud();
		ofDrawBitmapString(ofToString(skeletons.size()) + " skeletons", 400, 20 + camHeightPorc + 20);
		for (int i = 0; i < skeletons.size(); i++) {
			auto s = skeletons[i];
			drawSkeleton(s);
		}
		drawUsers(); // // draw user's center of mass

		ofSetColor(255, 255, 0);
		ofDrawBitmapString("RGBFRAME SIZE " + ofToString(rgbFrameSize.x) + ",  " + ofToString(rgbFrameSize.y), 20, APP_HEIGT_MEITAT - 20);
	}
#endif
}

#ifdef NUITRACK
void ofApp::drawUsers() {
	for (auto & u : users) {
		int uid = u.id;

		// REAL -----
		const Vector3 & v = u.real; // real pos is in mm
		ofVec3f vmap = ofVec3f(0, 0, 0);
		vmap.x = ofMap(v.x, -rgbFrameSize.x*0.5, rgbFrameSize.x*0.5, 0, APP_WIDTH);
		vmap.y = ofMap(v.y, rgbFrameSize.y*0.5, -rgbFrameSize.y*0.5, 0, APP_HEIGT);
		vmap.z = 0;
		ofVec4f vwarp = warper.fromScreenToWarpCoord(vmap.x, vmap.y, vmap.z);
		float radius = ofMap(v.z, 50, 7000, 1, 20);

		ofSetColor(255, 255, 0);
		string info = "CENTER OF MASS " + ofToString(static_cast<int>(v.x)) + ", " + ofToString(static_cast<int>(v.y));
		info += ", " + ofToString(static_cast<int>(v.z));
		ofDrawBitmapString(info, 20, APP_HEIGT_MEITAT);

		/*
		// draw user's center of mass
		ofPushMatrix();
		ofTranslate(vmap.x, vmap.y, vmap.z);
		ofFill();
		ofSetColor(255, 255, 0); // yellow
		ofDrawSphere(0,0,0,radius);
		ofDrawBitmapString("USER " + ofToString(uid), radius+5, radius+5);
		ofPopMatrix();

		// draw user's center of mass warped
		ofPushMatrix();
		ofTranslate(vwarp.x, vwarp.y, vwarp.z);
		ofFill();
		ofSetColor(0, 255, 255); // cian
		ofDrawSphere(0,0,0,radius);
		ofDrawBitmapString("USER " + ofToString(uid), radius+5, radius+5);
		ofPopMatrix();
		*/

		// PROJ -----
		const Vector3 & p = u.proj;
		ofVec3f pmap = ofVec3f(0, 0, 0);
		pmap.x = ofMap(p.x, 0.0, 1.0, 0, APP_WIDTH);
		pmap.y = ofMap(p.y, 0.0, 1.0, 0, APP_HEIGT);
		pmap.z = 0;
		ofVec4f pwarp = warper.fromScreenToWarpCoord(pmap.x, pmap.y, pmap.z);
		BoundingBox bb = u.box;

		ofSetColor(255, 0, 255);
		info = "CENTER OF MASS " + ofToString(p.x) + ", " + ofToString(p.y);
		info += ", " + ofToString(p.z);
		ofDrawBitmapString(info, 20, APP_HEIGT_MEITAT + 20);

		/*
		// draw user's center of mass
		ofPushMatrix();
		ofTranslate(pmap.x, pmap.y, pmap.z);
		ofFill();
		ofSetColor(255, 0, 255); // magenta
		ofDrawSphere(0,0,0,radius);
		ofDrawBitmapString("USER " + ofToString(uid), radius+5, radius+5);
		ofNoFill();
		float bbample = (bb.right - bb.left)*APP_WIDTH;
		float bbalt = (bb.bottom- bb.top)*APP_HEIGT;
		ofDrawRectangle(bb.top, bb.left, bbample, bbalt);
		ofPopMatrix();
		*/

		// draw user's center of mass
		ofPushMatrix();
		ofTranslate(pwarp.x, pwarp.y, pwarp.z);
		ofFill();
		ofSetColor(255, 128, 0); // taronja
		ofDrawSphere(0, 0, 0, radius);
		ofDrawBitmapString("USER " + ofToString(uid), radius + 5, radius + 5);
		ofPopMatrix();
	}
}

void ofApp::drawSkeleton(Skeleton &s) {
	auto joints = s.joints;
	for (auto j : joints) {
		drawJoint(j);
	}
	drawBones(joints);
}

void ofApp::drawJoint(Joint &j) {
	if (j.type == JOINT_NONE) {
		return;
	}
	if (j.confidence < 0.15) {
		return;
	}

	// real pos is in mm, lets convert to m
	if (j.type == JOINT_HEAD || j.type == JOINT_LEFT_HAND || j.type == JOINT_RIGHT_HAND ||
		j.type == JOINT_LEFT_ANKLE || j.type == JOINT_RIGHT_ANKLE) {
		// REAL  -----
		const Vector3 & v = j.real;
		ofVec3f vmap = ofVec3f(0, 0, 0);
		vmap.x = ofMap(v.x, -rgbFrameSize.x*0.5, rgbFrameSize.x*0.5, 0, APP_WIDTH);
		vmap.y = ofMap(v.y, rgbFrameSize.y*0.5, -rgbFrameSize.y*0.5, 0, APP_HEIGT);
		vmap.z = 0;
		ofVec4f vwarp = warper.fromScreenToWarpCoord(vmap.x, vmap.y, vmap.z);
		float radius = ofMap(v.z, 50, 7000, 1, 50);

		/*
		// draw joint's center of mass
		ofPushMatrix();
		ofTranslate(vmap.x, vmap.y, vmap.z);
		ofFill();
		ofSetColor(255, 255, 0); // yellow
		ofDrawBox(0,0,0,radius);
		ofPopMatrix();

		// draw joint's center of mass warped
		ofPushMatrix();
		ofTranslate(vwarp.x, vwarp.y, vwarp.z);
		ofFill();
		ofSetColor(0, 255, 255); // cian
		ofDrawBox(0,0,0,radius);
		ofPopMatrix();
		*/

		// PROJ -----
		const Vector3 & p = j.proj;
		ofVec3f pmap = ofVec3f(0, 0, 0);
		pmap.x = ofMap(p.x, 0.0, 1.0, 0, APP_WIDTH);
		pmap.y = ofMap(p.y, 0.0, 1.0, 0, APP_HEIGT);
		pmap.z = 0;
		pmap.x = APP_WIDTH - pmap.x; // TODO posr en un control aquest invertit per la pos de camera
		ofVec4f pwarp = warper.fromScreenToWarpCoord(pmap.x, pmap.y, pmap.z);

		/*
		// draw user's center of mass
		ofPushMatrix();
		ofTranslate(pmap.x, pmap.y, pmap.z);
		ofFill();
		ofSetColor(255, 0, 255); // magenta
		ofDrawBox(0,0,0,radius);
		ofDrawBitmapString(ofToString(pmap.x) + ", " + ofToString(pmap.y), 20,20);
		ofPopMatrix();
		*/

		// draw user's center of mass
		ofPushMatrix();
		ofTranslate(pwarp.x, pwarp.y, pwarp.z);
		ofFill();
		ofSetColor(255, 128, 0); // taronja
		ofDrawBox(0, 0, 0, radius);
		ofDrawBitmapString(ofToString(pwarp.x) + ", " + ofToString(pwarp.y), 20, 20);
		ofPopMatrix();
	}
}

void ofApp::updateJoint(Joint &j) {
	if (j.type == JOINT_NONE) {
		return;
	}
	if (j.confidence < 0.15) {
		return;
	}

	if (j.type == JOINT_HEAD || j.type == JOINT_LEFT_HAND || j.type == JOINT_RIGHT_HAND || j.type == JOINT_LEFT_ANKLE || j.type == JOINT_RIGHT_ANKLE) {
		// PROJ -----
		const Vector3 & p = j.proj;
		ofVec3f pmap = ofVec3f(0, 0, 0);
		pmap.x = ofMap(p.x, 0.0, 1.0, 0, APP_WIDTH);
		pmap.y = ofMap(p.y, 0.0, 1.0, 0, APP_HEIGT);
		pmap.z = 0;
		pmap.x = APP_WIDTH - pmap.x; // TODO posr en un control aquest invertit per la pos de camera
		posicionsBlobs[totalBlobsDetected].x = pmap.x;
		posicionsBlobs[totalBlobsDetected].y = pmap.y;
		totalBlobsDetected++;
	}
}

void ofApp::drawBones(vector<Joint> joints) {
	//if (joints.size() < 3) {
	//	return;
	//}

	// rotation based on T pose
	// sdk reports rotations relative to T pose..
	static vector<Bone> bones = {
		Bone(JOINT_WAIST, JOINT_TORSO, vec3(0, 1, 0)),
		Bone(JOINT_TORSO, JOINT_NECK, vec3(0, 1, 0)),
		Bone(JOINT_NECK, JOINT_HEAD, vec3(0, 1, 0)),

		Bone(JOINT_LEFT_COLLAR, JOINT_LEFT_SHOULDER, vec3(1, 0, 0)),
		Bone(JOINT_LEFT_SHOULDER, JOINT_LEFT_ELBOW, vec3(1, 0, 0)),
		Bone(JOINT_LEFT_ELBOW, JOINT_LEFT_WRIST, vec3(1, 0, 0)),
		Bone(JOINT_LEFT_WRIST, JOINT_LEFT_HAND, vec3(1, 0, 0)),

		Bone(JOINT_WAIST, JOINT_LEFT_HIP, vec3(1, 0, 0)),
		Bone(JOINT_LEFT_HIP, JOINT_LEFT_KNEE, vec3(0, -1, 0)),
		Bone(JOINT_LEFT_KNEE, JOINT_LEFT_ANKLE, vec3(0, -1, 0)),

		Bone(JOINT_RIGHT_COLLAR, JOINT_RIGHT_SHOULDER, vec3(-1, 0, 0)),
		Bone(JOINT_RIGHT_SHOULDER, JOINT_RIGHT_ELBOW, vec3(-1, 0, 0)),
		Bone(JOINT_RIGHT_ELBOW, JOINT_RIGHT_WRIST, vec3(-1, 0, 0)),
		Bone(JOINT_RIGHT_WRIST, JOINT_RIGHT_HAND, vec3(-1, 0, 0)),

		Bone(JOINT_WAIST, JOINT_RIGHT_HIP, vec3(-1, 0, 0)),
		Bone(JOINT_RIGHT_HIP, JOINT_RIGHT_KNEE, vec3(0, -1, 0)),
		Bone(JOINT_RIGHT_KNEE, JOINT_RIGHT_ANKLE, vec3(0, -1, 0)),
	};

	for (int i = 0; i < bones.size(); i++) {
		auto j1 = joints[bones[i].from];
		auto j2 = joints[bones[i].to];

		if (j1.confidence < 0.15 || j2.confidence < 0.15) {
			continue;
		}
		// REAL -----
		/*
		glm::vec3 v1 = ofxnui::Tracker::fromVector3(j1.real);
		glm::vec3 v2 = ofxnui::Tracker::fromVector3(j2.real);

		ofVec3f v1map = ofVec3f(0, 0, 0);
		ofVec3f v2map = ofVec3f(0, 0, 0);
		v1map.x = ofMap(v1.x, -rgbFrameSize.x*0.5,rgbFrameSize.x*0.5, 0,APP_WIDTH);
		v1map.y = ofMap(v1.y, rgbFrameSize.y*0.5,-rgbFrameSize.y*0.5, 0,APP_HEIGT);
		v1map.z = 0;
		v2map.x = ofMap(v2.x, -rgbFrameSize.x*0.5,rgbFrameSize.x*0.5, 0,APP_WIDTH);
		v2map.y = ofMap(v2.y, rgbFrameSize.y*0.5,-rgbFrameSize.y*0.5, 0,APP_HEIGT);
		v2map.z = 0;
		ofVec4f v1warp = warper.fromScreenToWarpCoord(v1map.x,v1map.y,v1map.z);
		ofVec4f v2warp = warper.fromScreenToWarpCoord(v2map.x,v2map.y,v2map.z);

		// draw joint's
		ofSetColor(255, 255, 0); // yellow
		ofDrawLine(v1map,v2map);

		// draw joint's
		ofSetColor(0, 255, 255); // cian
		ofDrawLine(v1warp.x,v1warp.y,v2warp.x,v2warp.y);
		*/

		// PROJ -----
		glm::vec3 p1 = ofxnui::Tracker::fromVector3(j1.proj);
		glm::vec3 p2 = ofxnui::Tracker::fromVector3(j2.proj);

		ofVec3f p1map = ofVec3f(0, 0, 0);
		ofVec3f p2map = ofVec3f(0, 0, 0);
		p1map.x = ofMap(p1.x, 0.0, 1.0, 0, APP_WIDTH);
		p1map.y = ofMap(p1.y, 0.0, 1.0, 0, APP_HEIGT);
		p1map.z = 0;
		p2map.x = ofMap(p2.x, 0.0, 1.0, 0, APP_WIDTH);
		p2map.y = ofMap(p2.y, 0.0, 1.0, 0, APP_HEIGT);
		p2map.z = 0;
		p1map.x = APP_WIDTH - p1map.x; // TODO posr en un control aquest invertit per la pos de camera
		p2map.x = APP_WIDTH - p2map.x; // TODO posr en un control aquest invertit per la pos de camera
		ofVec4f p1warp = warper.fromScreenToWarpCoord(p1map.x, p1map.y, p1map.z);
		ofVec4f p2warp = warper.fromScreenToWarpCoord(p2map.x, p2map.y, p2map.z);

		/*
		// draw joint's
		ofSetColor(255, 0, 255); // magenta
		ofDrawLine(p1map,p2map);
		//ofDrawBitmapString("bone: " + ofToString(i) + " at " + ofToString(p1map.x) + ", " + ofToString(p1map.y), 20,300+i*20);
		*/

		// draw joint's
		ofSetColor(255, 128, 0); // taronja
		ofDrawLine(p1warp.x, p1warp.y, p2warp.x, p2warp.y);

	}

}

void ofApp::drawPointcloud() {
	ofPushMatrix();
	glPointSize(1);
	pc.draw();
	ofPopMatrix();
}
#endif

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	if (key == 's')
		player->zVelocity = 25.0;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

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