#include "ofApp.h"

//#define OPEN_MP_TEST
// comment out to enable OpenMP
// also need turnning on openMP support from visual studio project properties

#ifdef OPEN_MP_TEST
#include <omp.h>
#endif

#ifdef NUITRACK
using namespace tdv::nuitrack;
#endif

pantallesJoc stage = START;

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup() {

	// APP
	//ofSetFullscreen(true);
	//ofHideCursor();

	// GENRAL
	ofSetVerticalSync(true);
	ofSetCircleResolution(300);
	ofTrueTypeFont::setGlobalDpi(72);
	singleton = Singleton::getInstance();

	// TEMPS DE JOC
	jocMinutsTimer.setup(3 * 60 * 1000, false); // 3 minuts = 3*60*1000 ms
	jocMinutsTimer.stopTimer();

	duradaTheEndTimer.setup(5 * 1000, false); // 5 segons
	duradaTheEndTimer.stopTimer();

	// WARPER
	warper.setup(0, 0, APP_WIDTH, APP_HEIGT);
	warper.load("TrampoLima_CornerPin.xml");
	warper.enableKeys(true);
	warper.deactivate();

	// PUNTER
	punter.load("punter.png");
	punterWidthMig = punter.getWidth()*0.5;
	punterHeightMig = punter.getHeight()*0.5;

	// COLOR JOC
	saltingBlue = ofColor(0, 192, 255);

	// TYPO
	ofTrueTypeFont::setGlobalDpi(72);
	saltingTypo.load("Oswald-Bold.otf", 27, true, true); // temps nivell i normal, kids, pro
	saltingTypo.setLetterSpacing(1.2);

	// PECES
	comptadorPeces = 0;
	peca1.setup(0, 0, myGrid.returnPosicioOfPeca(0), 50);
	ofAddListener(pecaEmpty::actualitzaPunts, this, &ofApp::actualitzaPuntsEmpty);

	// BOTONS
	guib = new ofxUICanvas(20, 20, APP_WIDTH*0.3, APP_HEIGT*0.9);

	guib->addLabel(":: BOTONS ::", OFX_UI_FONT_MEDIUM);
	guib->addSpacer();
	guib->addIntSlider("botoStartX", 0, APP_WIDTH, &botoStart.botoX)->setIncrement(1);
	guib->addIntSlider("botoStartY", 0, APP_HEIGT, &botoStart.botoY)->setIncrement(1);

	botoStart.setup(botoStart.botoX, botoStart.botoY, 80, saltingBlue);

	guib->autoSizeToFitWidgets();
	ofAddListener(guib->newGUIEvent, this, &ofApp::guiEvent);
	guib->loadSettings("TrampoLima_Botons.xml");

	guib->setVisible(false);

	// GRID
	myGrid.setup();
	myGrid.gridActivaExcepteMargesSupInfDretaEsq();

	// GUI APP
	guia = new ofxUICanvas(20, 20, APP_WIDTH*0.4, APP_HEIGT*0.9);

	guia->addLabel(":: TRAMPOLIMA SETTINGS ::", OFX_UI_FONT_MEDIUM);
	guia->addSpacer();

	guia->addSpacer();
	guia->addLabelButton("load factory defaults", false);

	guia->addSpacer();
	guia->addLabel("GAME IMAGE CORNERS", OFX_UI_FONT_MEDIUM);
	guia->addLabelButton("reset corners", false);

	guia->addSpacer();
	guia->addLabel("4 GAME CONTROLLERS POSITION", OFX_UI_FONT_MEDIUM);
	guia->addSlider("adjustment up-down", -1.5*APP_WIDTH, 1.5*APP_WIDTH, &baixaHoTotAvall);
	guia->addSlider("adjustment left-right", -1.5*APP_WIDTH, 1.5*APP_WIDTH, &mouHoTotDretaEsq);

	guia->autoSizeToFitWidgets();
	ofAddListener(guia->newGUIEvent, this, &ofApp::guiEvent);
	guia->loadSettings("TrampoLima_Deteccio.xml");

	guia->setVisible(false);

	// HELP
	guih = new ofxUICanvas(20, 20, APP_WIDTH*0.7, APP_HEIGT*0.9);

	guih->addLabel(":: HELP ::", OFX_UI_FONT_MEDIUM);
	guih->addSpacer();
	guih->addFPS();
	guih->addSpacer();
	guih->addTextArea("helpText1", "step 1) WARP: press W | w and mouse click and drag to adjust game's image corners");
	guih->addTextArea("helpText2", "step 2) CAM: press C | c to show and hide camera image");
	guih->addTextArea("helpText3", "step 3) GAME: press J | j | G | g to adjust game");
	guih->addTextArea("helpText5", "step 4) BUTTONS & GRID: press B | b to show and hide grid and buttons adjustments");

	guih->autoSizeToFitWidgets();
	ofAddListener(guih->newGUIEvent, this, &ofApp::guiEvent);

	guih->setVisible(false);

	// PARTIDA
	setVariablesIniciPartida();

#ifdef NUITRACK
	// REALSENSE
	bool bPlayFile = false;
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

	menu_image.loadImage("menu.png");

	cam.tilt(80);
	cam.setGlobalPosition(glm::vec3(0, 0, 0));

	player = new Player();
	/*
	Floor* fInit = new Floor();
	fInit->position = glm::vec3(0, 600, 0);
    fInit->size_ = 30000;
    floors.push_back(fInit);

	for (int i = 3; i < 15; i++)
	{
		Floor* f = new Floor();
		f->position = glm::vec3(0, i * 600, 0);
		f->size_ = 500;
		floors.push_back(f);
    }

	for (int i = 0; i < 1; i++)
	{
		Box* b = new Box();
		b->position = glm::vec3(0, 500 + i * 400, 200);
		b->size_ = 100;
		boxes.push_back(b);
	}
	*/
	generateEasyLevel();
	generateHardLevel();

	player->gravity = -3.0;
}

//--------------------------------------------------------------
void ofApp::setVariablesIniciPartida() {
	// APP
	singleton->setPuntuacioJugador(0);

	// BOTONS
	botoStart.botoSeleccionat = false;

	// TEMPS DE JOC
	jocMinutsTimer.reset();
	jocMinutsTimer.stopTimer();
	duradaTheEndTimer.reset();
	duradaTheEndTimer.stopTimer();

}

//--------------------------------------------------------------
void ofApp::update() {
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

	for (int i = 0; i < totalBlobsDetected; i++) {
		posicionsBlobs[i].x = posicionsBlobs[i].x + mouHoTotDretaEsq;
		posicionsBlobs[i].y = posicionsBlobs[i].y + baixaHoTotAvall;
	}

	// PANTALLES ------------------------------------------
	if (pantallaJoc == START) {
		botoStart.update(totalBlobsDetected, posicionsBlobs);
		botoStart.updatem(warpMousePos);

		if (botoStart.botoSeleccionat == true) { // CANVI A pantallaJoc = PLAY;
			pantallaJoc = PLAY;
			jocMinutsTimer.startTimer();

			setupPeca1(); // la única peça d'aquest joc
		}
	}
	else if (pantallaJoc == PLAY) {
		peca1.updatem(warpMousePos);
		peca1.update(totalBlobsDetected, posicionsBlobs);
		comprobarEstatsPecesEmpty();
	}
	else if (pantallaJoc == END) {
		if (duradaTheEndTimer.isTimerFinished()) {
			setVariablesIniciPartida();
			pantallaJoc = START;
		}
	}

	// TEMPS ----------------------------------------------
	jocMinutsTimerSegonsLeft = jocMinutsTimer.getTimeLeftInSeconds();
	jocMinutsTimerMinuts = (int)jocMinutsTimerSegonsLeft / 60;
	jocMinutsTimerSegons = jocMinutsTimerSegonsLeft - jocMinutsTimerMinuts * 60;

	if (jocMinutsTimer.isTimerFinished()) {
		pantallaJoc = END;
		jocMinutsTimer.reset();
		jocMinutsTimer.stopTimer();
		duradaTheEndTimer.startTimer();
	}
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
void ofApp::draw() {
	// WARP begin
	warper.begin();
	warper.draw();

	// APP
	ofBackground(0);
	ofSetColor(255);
	ofSetWindowTitle("TrampoLima running at " + ofToString((int)ofGetFrameRate()) + " frames per second");

	// FONS
	/*
	ofSetColor(155);
	ofFill();
	ofDrawRectangle(0,0, APP_WIDTH, APP_HEIGT);
	*/

	// PANTALLES --------------------------------------------------------------------------
	/*
	if(pantallaJoc == START){
		drawTemps();
		drawPuntuacio();
		botoStart.draw();
	}
	else if(pantallaJoc == PLAY){
		drawTemps();
		drawPuntuacio();
		peca1.draw();
	}
	else if(pantallaJoc == END){
		drawEnd();
	}
	*/
	// HELP ----------------------------------------------------
	// GRID
	myGrid.draw();
	if (bdrawMouseWarped) {
		ofSetColor(255, 255, 0);
		ofDrawCircle(warpMousePos.x, warpMousePos.y, 5, 5);
	}
	//ofSetColor(255,255,0);
	//ofDrawBitmapString("ESTAT: " + pantallaToString(), 20,20);

	// PUNTERS ----------------------------------------------------
	ofSetColor(255);
	for (int i = 0; i < totalBlobsDetected; i++) {
		punter.draw(posicionsBlobs[i].x - punterWidthMig, posicionsBlobs[i].y - punterHeightMig);
	}

	if (stage == START)
	{
		drawMenu();
	}
	else if (stage == EASY_PLAY)
	{
		drawLevel();
	}
	else if (stage == HARD_PLAY)
	{
		drawLevel();
	}
	else if (stage == DEATH)
	{
		drawDeath();
	}
	else if (stage == END)
	{
		drawEnd();
	}

#ifdef NUITRACK
	// CAMERA
	if (bshowCamera) {
		ofSetColor(255);
		if (rgbTex.isAllocated()) {
			rgbTex.draw(0, 0, APP_WIDTH, APP_HEIGT);
		}
	}
#endif

	// WARP end
	warper.end();

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
		player->myJoints[j.type] = pmap.y;
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
void ofApp::exit() {
	warper.save("TrampoLima_CornerPin.xml");
	guia->saveSettings("TrampoLima_Deteccio.xml");
	delete guia;
	guib->saveSettings("TrampoLima_Botons.xml");
	delete guib;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	// H + F1 + a : help, aide, ayuda, ajuda
	// w : corner pin
	// J ajusta joc
	// G : grid -> per development (cal descomentar a baix)
	// D ajusta botons -> per development (cal descomentar a baix)

	if ((key == 'h') || (key == 'H') || (key == 'a') || (key == 'A') || (key == OF_KEY_F1)) {
		guia->setVisible(false);
		guib->setVisible(false);
		warper.deactivate();
		bshowImagesAndSkeleton = false;
		guih->toggleVisible();
	}
	else if ((key == 'c') || (key == 'C')) {
		guia->setVisible(false);
		guib->setVisible(false);
		guih->setVisible(false);
		bshowImagesAndSkeleton = false;
		warper.deactivate();
		bshowCamera = !bshowCamera;
	}
	else if ((key == 'y') || (key == 'Y')) {
		guia->setVisible(false);
		guib->setVisible(false);
		guih->setVisible(false);
		bshowImagesAndSkeleton = false;
		warper.toggleActive();
	}
	else if ((key == 'j') || (key == 'J') || (key == 'g') || (key == 'G')) {
		guib->setVisible(false);
		guih->setVisible(false);
		warper.deactivate();
		guia->toggleVisible();
		bshowImagesAndSkeleton = !bshowImagesAndSkeleton;
	}
	else if ((key == 'f') || (key == 'F')) {
		ofToggleFullscreen();
	}
	else if ((key == 'b') || (key == 'B')) {
		toogleDrawInfoGrid();
		guia->setVisible(false);
		guih->setVisible(false);
		warper.deactivate();
		bshowImagesAndSkeleton = false;
		guib->toggleVisible();
	}
	else if ((key == 'r') || (key == 'R')) {
		// reset corners
		warper.setCorner(ofxGLWarper::TOP_LEFT, 0, 0);
		warper.setCorner(ofxGLWarper::TOP_RIGHT, APP_WIDTH, 0);
		warper.setCorner(ofxGLWarper::BOTTOM_RIGHT, APP_WIDTH, APP_HEIGT);
		warper.setCorner(ofxGLWarper::BOTTOM_LEFT, 0, APP_HEIGT);
	}

	if (key == 's')
	{
		if (stage == EASY_PLAY || stage == HARD_PLAY)
		{
			player->normalJump();
		}
	}

	if (key == 'w')
	{
		if (stage == HARD_PLAY)
		{
			player->highJump();
		}
	}

	if (key == 'd')
	{
		if (stage == HARD_PLAY)
		{
			player->throughJump();
		}
	}

	if (key == 'x')
	{
		if (stage == HARD_PLAY)
		{
			player->slide();
		}
	}

	if (stage == HARD_PLAY)
	{
		if (!player->is_slide && player->sliding)
		{
			cam.rotateRad(-(PI / 4), glm::vec3(0, -1, 0));
			actual_position = player->position;
			player->sliding = false;
			player->is_slide = true;

		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) 
{
	if (key == '1')
	{
		if (stage == START)
		{
			stage = EASY_PLAY;
			player->game_mode = 1;
		}
	}

	if (key == '2')
	{
		if (stage == START)
		{
			stage = HARD_PLAY;
			player->game_mode = 2;
		}
	}
	
	if (key == 's')
	{
		if (stage == DEATH)
		{
			restartGame();
			stage = START;
			player->game_mode = 0;
		}
		else if (stage == END)
		{
			restartGame();
			stage = START;
			player->game_mode = 0;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	ofVec4f v = warper.fromScreenToWarpCoord(x, y, 0);
	warpMousePos.x = v.x;
	warpMousePos.y = v.y;
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
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e) {
	string name = e.widget->getName();
	int kind = e.widget->getKind();

	if (name == "reset corners") {
		// reset corners
		warper.setCorner(warper.TOP_LEFT, 0, 0);
		warper.setCorner(warper.TOP_RIGHT, APP_WIDTH, 0);
		warper.setCorner(warper.BOTTOM_RIGHT, APP_WIDTH, APP_HEIGT);
		warper.setCorner(warper.BOTTOM_LEFT, 0, APP_HEIGT);
	}
	else if (name == "load factory defaults") {
		// reset corners
		warper.setCorner(warper.TOP_LEFT, 0, 0);
		warper.setCorner(warper.TOP_RIGHT, APP_WIDTH, 0);
		warper.setCorner(warper.BOTTOM_RIGHT, APP_WIDTH, APP_HEIGT);
		warper.setCorner(warper.BOTTOM_LEFT, 0, APP_HEIGT);
		// parametres
		baixaHoTotAvall = 0; // adjustment fi up-down
		mouHoTotDretaEsq = 0; // adjustment fi left-right
	}
}

//--------------------------------------------------------------
//---PECES EMPTY -----------------------------------------------
//--------------------------------------------------------------
void ofApp::comprobarEstatsPecesEmpty() {
	if (peca1.estatPeca == THE_END) { // seguent peca: tornem-hi
		setupPeca1();
	}
}

//--------------------------------------------------------------
void ofApp::setupPeca1() {
	int g = 0;
	do {
		g = (int)ofRandom(0, NUM_COLS*NUM_ROWS);
	} while (myGrid.brectGridActiu[g] == false);
	peca1.init(comptadorPeces, g, myGrid.returnPosicioOfPeca(g));
	comptadorPeces++;

	peca1.estatPeca = CANVIA_ESTAT;
	peca1.estatPecaNext = APAREIX;
	peca1.bpecaActiva = true;
}

//--------------------------------------------------------------
//--- HELPERS --------------------------------------------------
//--------------------------------------------------------------
void ofApp::toogleDrawInfoGrid() {
	myGrid.toggleDrawInfoHelp();
	peca1.toggleDrawInfoHelp();
	botoStart.toggleDrawInfoHelp();
	bdrawMouseWarped = !bdrawMouseWarped;
}

//--------------------------------------------------------------
string ofApp::pantallaToString() {
	if (pantallaJoc == START) {
		return "START";
	}
	else if (pantallaJoc == EASY_PLAY) {
		return "EASY_PLAY";
	}
	else if (pantallaJoc == HARD_PLAY) {
		return "HARD_PLAY";
	}
	else if (pantallaJoc == PLAY) {
		return "PLAY";
	}
	else if (pantallaJoc == END) {
		return "END";
	}
	else {
		return "NO SE";
	}
}

//--------------------------------------------------------------
//--- PUNTUACIO ------------------------------------------------
//--------------------------------------------------------------
void ofApp::actualitzaPuntsEmpty(int & e) {
	singleton->setPuntuacioJugador(singleton->getPuntuacioJugador() + e);
}

//--------------------------------------------------------------
void ofApp::drawTemps() {
	string s = ofToString(jocMinutsTimerMinuts, 2, '0') + ":" + ofToString(jocMinutsTimerSegons, 2, '0');
	ofRectangle rs;
	rs = saltingTypo.getStringBoundingBox(s, 0, 0);
	ofPushMatrix();
	ofTranslate(APP_WIDTH_MEITAT - rs.width*0.5, 35);
	if (jocMinutsTimerSegonsLeft < 60) {
		ofSetColor(255, 0, 0, 255); // temps en vermell
	}
	else {
		ofSetColor(255); // temps en blanc
	}
	saltingTypo.drawString(s, 0, 0);
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::drawPuntuacio() {
	string s = ofToString(singleton->getPuntuacioJugador()) + " POINTS!";
	ofRectangle rs;
	rs = saltingTypo.getStringBoundingBox(s, 0, 0);
	ofPushMatrix();
	ofTranslate(APP_WIDTH_MEITAT - rs.width*0.5, 135);
	ofSetColor(saltingBlue);
	saltingTypo.drawString(s, 0, 0);
	ofPopMatrix();
}

//--------------------------------------------------------------
/*
void ofApp::drawEnd(){
	string s = "GREAT JOB!! " + ofToString(singleton->getPuntuacioJugador()) + " POINTS";
	ofRectangle rs;
		rs = saltingTypo.getStringBoundingBox(s,0,0);
		ofPushMatrix();
	ofTranslate(APP_WIDTH_MEITAT-rs.width*0.5,APP_HEIGT_MEITAT-rs.height*0.5);
	ofSetColor(255);
	saltingTypo.drawString(s,0,0);
	ofPopMatrix();
}
*/

Player::Player()
{
	position = glm::vec3(0, 0, 15);
}

void Player::render()
{
	update();

	ofSetColor(0, 255, 0);
	ofFill();
	position = glm::vec3(0, position.y + yVelocity, position.z + zVelocity);
	//ofDrawSphere(position, 15);
}

void Player::update()
{
	if (position.z <= 15 && falling == 0)
	{
		zVelocity += gravity;
	}
	else if (position.z > 16 && jumping == 1)
	{
		zVelocity += gravity;
	}

	if (position.z < 15 && jumping == 1)
	{
		zVelocity = 0;
		jumping = 0;
		position.z = 15;
	}

	if (collision || position.z < -200)
	{
		zVelocity = 0;
		stage = DEATH;
	}

	if (game_mode == 1)
	{
		if (position.y >= 9800)
		{
			zVelocity = 0;
			stage = END;
		}
	}
	else if (game_mode == 2)
	{
		if (position.y >= 19800)
		{
			zVelocity = 0;
			stage = END;
		}
	}
}

void Player::normalJump()
{
	if (stage == EASY_PLAY || stage == HARD_PLAY)
	{
		if (jumping)
			return;

		zVelocity = 30.0;
		jumping = true;
	}
}

void Player::highJump()
{
	if (stage == HARD_PLAY)
	{
		if (jumping)
			return;

		zVelocity = 50.0;
		jumping = true;
	}
}

void Player::throughJump()
{
	if (stage == HARD_PLAY)
	{
		if (jumping)
			return;

		zVelocity = 20.0;
		jumping = true;
	}
}

void Player::slide()
{
	if (stage == HARD_PLAY)
	{
		sliding = true;
		cout << "Slide\n";
	}
}

Floor::Floor()
{

}

void Floor::render()
{
	ofSetColor(color.x, color.y, color.z);
	ofFill();
	ofDrawPlane(position, size.x, size.y);
}

bool ofApp::isFloor(glm::vec3 p_position)
{
	if (stage == EASY_PLAY)
	{
		for (list<Floor>::iterator it = easy_floors.begin(); it != easy_floors.end(); ++it)
		{
			if (p_position.y >= (it->position.y - it->size.y / 2) && p_position.y <= (it->position.y + it->size.y / 2))
				return true;
		}
	}
	else if (stage == HARD_PLAY)
	{
		for (list<Floor>::iterator it = hard_floors.begin(); it != hard_floors.end(); ++it)
		{
			if (p_position.y >= (it->position.y - it->size.y / 2) && p_position.y <= (it->position.y + it->size.y / 2))
				return true;
		}
	}

	return false;
}

Box::Box()
{

}

void Box::render()
{
	ofSetColor(color.x, color.y, color.z);
	ofFill();
	ofDrawBox(position, size.x, size.y, size.z);
}

bool ofApp::isBox(glm::vec3 p_position)
{
	if (stage == EASY_PLAY)
	{
		for (list<Box>::iterator it = easy_boxes.begin(); it != easy_boxes.end(); ++it)
		{
			if (p_position.y >= (it->position.y - it->size.y / 2) && p_position.y <= (it->position.y + it->size.y / 2))
				if (p_position.z >= (it->position.z - it->size.z / 2) && p_position.z <= (it->position.z + it->size.z / 2))
					return true;
		}
	}
	else if (stage == HARD_PLAY)
	{
		for (list<Box>::iterator it = hard_boxes.begin(); it != hard_boxes.end(); ++it)
		{
			if (p_position.y >= (it->position.y - it->size.y / 2) && p_position.y <= (it->position.y + it->size.y / 2))
				if (p_position.z >= (it->position.z - it->size.z / 2) && p_position.z <= (it->position.z + it->size.z / 2))
					return true;
		}
	}

	return false;
}

void ofApp::drawMenu()
{
	menu_image.draw(0, 0, APP_WIDTH, APP_HEIGT);
}

void ofApp::drawLevel()
{
	player->falling = isFloor(player->position);
	player->collision = isBox(player->position);

	//CAMARA
	//cam.setGlobalPosition(cam.getGlobalPosition().x, cam.getGlobalPosition().y + player->yVelocity, cam.getGlobalPosition().z);
	cam.setGlobalPosition(player->position.x, player->position.y + player->yVelocity, player->position.z + player->zVelocity + 50);
	cam.begin();

	cout << cam.getGlobalPosition().z << endl;
	if (stage == EASY_PLAY)
	{
		for (list<Floor>::iterator it = easy_floors.begin(); it != easy_floors.end(); ++it)
		{
			it->render();
		}

		for (list<Box>::iterator it = easy_boxes.begin(); it != easy_boxes.end(); ++it)
		{
			it->render();
		}
	}
	else if (stage == HARD_PLAY)
	{
		if (player->is_slide)
		{
			if (player->position.y == (actual_position.y + 150))
			{
				player->is_slide = false;
				cam.rotateRad((PI / 4), glm::vec3(0, -1, 0));
			}
		}

		for (list<Floor>::iterator it = hard_floors.begin(); it != hard_floors.end(); ++it)
		{
			it->render();
		}

		for (list<Box>::iterator it = hard_boxes.begin(); it != hard_boxes.end(); ++it)
		{
			it->render();
		}
	}

	player->render();

	cam.end();
}

void ofApp::drawDeath()
{
	string s = "YOU DIED - JUMP TO GO TO MENU";
	ofRectangle rs;
	rs = saltingTypo.getStringBoundingBox(s, 0, 0);
	ofPushMatrix();
	ofTranslate(APP_WIDTH_MEITAT - rs.width*0.5, APP_HEIGT_MEITAT - rs.height*0.5);
	ofSetColor(255);
	saltingTypo.drawString(s, 0, 0);
	ofPopMatrix();
}

void ofApp::drawEnd()
{
	string s = "YOU WIN! - JUMP TO GO TO MENU";
	ofRectangle rs;
	rs = saltingTypo.getStringBoundingBox(s, 0, 0);
	ofPushMatrix();
	ofTranslate(APP_WIDTH_MEITAT - rs.width*0.5, APP_HEIGT_MEITAT - rs.height*0.5);
	ofSetColor(255);
	saltingTypo.drawString(s, 0, 0);
	ofPopMatrix();
}

void ofApp::restartGame()
{
	//cam.setGlobalPosition(glm::vec3(0, -100, 90));
	//cam.tilt(80);

	cam.setGlobalPosition(glm::vec3(0, -400, 90));

	player->position = glm::vec3(0, 0, 15);

	player->falling = false;
	player->jumping = false;
	player->collision = false;
}

void ofApp::generateEasyLevel()
{
	Floor f;
	Box b;

	f.position = glm::vec3(0, 0, 0);
	f.size = glm::vec2(1500, 1500);
	f.color = glm::vec3(255, 255, 0);
	f.position.y += (f.size.y / 2);
	easy_floors.push_back(f);

	f.position = glm::vec3(0, 1600, 0);
	f.size = glm::vec2(500, 2000);
	f.color = glm::vec3(255, 0, 0);
	f.position.y += (f.size.y / 2);
	easy_floors.push_back(f);

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 2650, 0);
		b.size = glm::vec3(100, 1, 40);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		easy_boxes.push_back(b);
	}

	f.position = glm::vec3(0, 3700, 0);
	f.size = glm::vec2(500, 2500);
	f.color = glm::vec3(255, 0, 0);
	f.position.y += (f.size.y / 2);
	easy_floors.push_back(f);

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 4500, 0);
		b.size = glm::vec3(100, 1, 40);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		easy_boxes.push_back(b);
	}

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 5400, 0);
		b.size = glm::vec3(100, 1, 40);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		easy_boxes.push_back(b);
	}

	f.position = glm::vec3(0, 6300, 0);
	f.size = glm::vec2(500, 3500);
	f.color = glm::vec3(255, 0, 0);
	f.position.y += (f.size.y / 2);
	easy_floors.push_back(f);

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 7300, 0);
		b.size = glm::vec3(100, 1, 40);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		easy_boxes.push_back(b);
	}

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 8300, 0);
		b.size = glm::vec3(100, 1, 40);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		easy_boxes.push_back(b);
	}

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 9300, 0);
		b.size = glm::vec3(100, 1, 40);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		easy_boxes.push_back(b);
	}

	f.position = glm::vec3(0, 9800, 0);
	f.size = glm::vec2(1500, 1500);
	f.color = glm::vec3(0, 2500, 0);
	f.position.y += (f.size.y / 2);
	easy_floors.push_back(f);

	/*
	f.position = glm::vec3(0, , 0);
	f.size = glm::vec2(500, );
	f.color = glm::vec3(255, 0, 0);
	f.position.y += (f.size.y / 2);
	easy_floors.push_back(f);
	*/

	/*
	b.position = glm::vec3(i * 200, , 0);
	b.size = glm::vec3(100, 10, 30);
	b.color = glm::vec3(0, 0, 255);
	b.position.y += (b.size.y / 2);
	easy_boxes.push_back(b);
	*/
}

void ofApp::generateHardLevel()
{
	Floor f;
	Box b;

	f.position = glm::vec3(0, 0, 0);
	f.size = glm::vec2(1500, 1500);
	f.color = glm::vec3(255, 255, 0);
	f.position.y += (f.size.y / 2);
	hard_floors.push_back(f);

	f.position = glm::vec3(0, 1600, 0);
	f.size = glm::vec2(500, 4000);
	f.color = glm::vec3(255, 0, 0);
	f.position.y += (f.size.y / 2);
	hard_floors.push_back(f);

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 2600, 0);
		b.size = glm::vec3(100, 10, 60);
		b.color = glm::vec3(255, 0, 255);
		b.position.y += (b.size.y / 2);
		hard_boxes.push_back(b);
	}

	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, 3600, 200);
		b.size = glm::vec3(100, 100, 100);
		b.color = glm::vec3(0, 255, 255);
		b.position.y += (b.size.y / 2);
		hard_boxes.push_back(b);
	}

	/*
	f.position = glm::vec3(0, , 0);
	f.size = glm::vec2(500, );
	f.color = glm::vec3(255, 0, 0);
	f.position.y += (f.size.y / 2);
	hard_floors.push_back(f);
	*/

	/*	normal jump
	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, , 0);
		b.size = glm::vec3(100, 10, 30);
		b.color = glm::vec3(0, 0, 255);
		b.position.y += (b.size.y / 2);
		hard_boxes.push_back(b);
	}
	*/

	/*	high jump
	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, , 0);
		b.size = glm::vec3(100, 10, 60);
		b.color = glm::vec3(255, 0, 255);
		b.position.y += (b.size.y / 2);
		hard_boxes.push_back(b);
	}
	*/

	/*	slide
	for (int i = -1; i < 2; i++)
	{
		b.position = glm::vec3(i * 200, , 0);
		b.size = glm::vec3(100, 100, 100);
		b.color = glm::vec3(0, 255, 255);
		b.position.y += (b.size.y / 2);
		hard_boxes.push_back(b);
	}
	*/
}