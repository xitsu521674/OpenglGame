/************************************************************************
     File:        TrainView.cpp

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include "Model.h"
#include "animator.h"
#include "particle.h"

#include <random>
struct TextC {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};



std::map<GLchar, TextC> Characters;
vector<unsigned int> news;
vector<glm::vec3> treePos;

Model *houseModel = nullptr;
Model* sceneModel = nullptr;
Model* radioModel = nullptr;
Model* wolfModel = nullptr;
Model* boxModel = nullptr;
Model* soldierModel = nullptr;
Model* rocketModel = nullptr;
Model* talkModel = nullptr;
Model* rockModel = nullptr;
Model* cubeModel = nullptr;
Model* subModel = nullptr;
Animation* soldierAnimation = nullptr;
Animation* soldierWaveAnimation = nullptr;
Animator* soldierAnimator = nullptr;
Animator* waveAnimator = nullptr;
Animator* soldierPlayAnimator = nullptr;
Animation* wolfAnimation = nullptr;
Animation* barkAnimation = nullptr;
Animator* wolfAnimator = nullptr;
Animator* barkAnimator = nullptr;
Animator* wolfPlayAnimator = nullptr;



Shader* basicShader = nullptr;
unsigned int basicVAO, basicVBO;
unsigned int LrailVAO, LrailVBO;
unsigned int RrailVAO, RrailVBO;

vector<glm::vec3> cableBasic = { glm::vec3(-1200,550,-545),glm::vec3(-900,520,-300),glm::vec3(-600,485,-545),glm::vec3(-600,-5885,-8439),glm::vec3(-900,-5825,-8584),glm::vec3(-1200,-5811,-8439) };
vector<glm::vec3> cablePoints;
vector<glm::vec3> cableLeft;
vector<glm::vec3> cableRight;


vector<glm::vec3> wolfBasic = { glm::vec3(-115, -6360, -7665), glm::vec3(115, -6360, -7865), glm::vec3(-115, -6360, -8065), glm::vec3(-315, -6360, -7865) };
vector<glm::vec3> wolfPoints;

glm::mat4 projection;
glm::mat4 view;
glm::vec3 rocketPosition = glm::vec3(-30, -6300, -5848);
float rocketScale = 10.0f;
glm::vec3 rockPosition = glm::vec3(-200, 20000, -25000);

int currentTrain = 0;
unsigned int currentWolf = 0;
unsigned int currentSoldier = 0;
float soldierWave = 0.0f;
float rockSize = 1;

std::random_device rd;
std::mt19937 gen = std::mt19937(rd());
std::uniform_real_distribution<float> dis(-10, 10);
std::uniform_real_distribution<float> fireDis(-400, 400);
float lastStirX = 100;
float lastStirY = 100;

ParticleSystem* ps = nullptr;
unsigned int TransloadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RGBA8;
		

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
glm::vec3 b_spine(glm::vec3 p[4], float t) {
	float G[] = { p[0].x,p[0].y, p[0].z, p[1].x,p[1].y, p[1].z, p[2].x,p[2].y, p[2].z, p[3].x,p[3].y, p[3].z };
	float TMG[3];
	//row major
	float tt[] = { 1,t,t * t,t * t * t };
	float m[] = { 1, 4, 1, 0,
				  -3, 0, 3, 0,
				   3,-6, 3, 0,
				  -1, 3,-3, 1 };
	// 1*4(tt) product 4*4(m) ->  1*4(TM)
	float TM[4];
	for (int i = 0; i < 4; ++i) {
		float sum = 0;
		for (int j = 0; j < 4; ++j) {
			sum += tt[j] * m[i + j * 4];
		}
		TM[i] = sum;
	}
	// 1*4(TM) product 4*3(G) ->  1*3(TMG)
	for (int i = 0; i < 3; ++i) {
		float sum = 0;
		for (int j = 0; j < 4; ++j) {
			sum += TM[j] * G[i + j * 3];
		}
		TMG[i] = sum / 6;
	}


	return glm::vec3(TMG[0], TMG[1], TMG[2]);
}
void calculateCable() {
	int seg = 100;
	float width = 30.0f;
	for (int i = 0; i < cableBasic.size(); ++i) {
		vector<int> numlist;
		glm::vec3 list[4];
		for (int j = 0; j < 4; ++j) {
			numlist.push_back((i + j) % cableBasic.size());
			list[j] = cableBasic[numlist[j]];
		}
		float t = 0.0f;
		for (int j = 0; j < seg; ++j) {
			cablePoints.push_back(b_spine(list, t));
			t += 1.0f/seg;
		}
	}
	cableRight.resize(cablePoints.size());
	cableLeft.resize(cablePoints.size());
	for (int i = 0; i < cablePoints.size(); ++i) {
		glm::vec3 forward = glm::normalize(cablePoints[(i + 1) % cablePoints.size()] - cablePoints[i]);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::cross(forward, up);
		cableRight[i] = cablePoints[i] + right * width;
		cableLeft[i] = cablePoints[i] - right * width;
	}
}
void calculateWolf() {
	int seg = 100;
	for (int i = 0; i < wolfBasic.size(); ++i) {
		vector<int> numlist;
		glm::vec3 list[4];
		for (int j = 0; j < 4; ++j) {
			numlist.push_back((i + j) % wolfBasic.size());
			list[j] = wolfBasic[numlist[j]];
		}
		float t = 0.0f;
		for (int j = 0; j < seg; ++j) {
			wolfPoints.push_back(b_spine(list, t));
			t += 1.0f / seg;
		}
	}
}
float Caldistance(glm::vec3 A, glm::vec3 B) {
	return sqrt((A.x - B.x) * (A.x - B.x) + (A.z - B.z) * (A.z - B.z));
}
void TrainView::stirScreen(float rate) {
	if (lastStirX != 100)
		currentCharacter->position.x -= lastStirX;
	if (lastStirY != 100)
		currentCharacter->position.y -= lastStirY;
	lastStirX = dis(gen)*rate;
	lastStirY = dis(gen)*rate;
	currentCharacter->position.x += lastStirX;
	currentCharacter->position.y += lastStirY;
}
//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l) 
	: Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );

	resetArcball();
	worldCharacter = new Character(false);
	mainCharacter = new Character(true);
	eventCharacter = new Character(false);
	currentCharacter = eventCharacter;
	wakeupTimer = new Timer(250);
	wakeupTimer->start(this->uTime);
	watchTvTimer = new Timer(700);
	outTimer = new Timer(100);
	goTrainTiemr = new Timer(420);
	backTrainTimer = new Timer(420);
	rocketTimer = new Timer(700);
	nightTimer = new Timer(500);

	calculateCable();
	calculateWolf();
	/*for (int i = 0; i < cablePoints.size(); ++i) {
		std::cout << cablePoints[i].x << " " << cablePoints[i].y << " " << cablePoints[i].z << "\n";
		std::cout << cableLeft[i].x << " " << cableLeft[i].y << " " << cableLeft[i].z << "\n";
		std::cout << cableRight[i].x << " " << cableRight[i].y << " " << cableRight[i].z << "\n";
		std::cout << "\n";
	}*/
	//std::cout << "\n";
	/*std::random_device rd;
	std::default_random_engine gen = std::default_random_engine(rd());
	std::uniform_int_distribution<int> zdis(-200,200);
	std::uniform_int_distribution<int> xdis(-200,200);*/
	
	/*TCHAR NPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, NPath);
	std::cout << NPath << "\n";*/
}


//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	float xpos, ypos, xoffset, yoffset;
	float sX = x_root();
	float sY = y_root();
	float width = w();
	float height = h();


	int k, ks;
	// remember what button was used
	static int last_push;
	Camera& currentCamera = currentCharacter->camera;

	switch(event) {
		case FL_MOVE:
			if (!mouseIn)
				break;
			if (currentCharacter->FocusCamera)
				break;
			xpos = static_cast<float>(Fl::event_x());
			ypos = static_cast<float>(Fl::event_y());
			//std::cout << xpos << " " << ypos << "\n";
			if (currentCamera.firstMouse)
			{
				currentCamera.lastX = xpos;
				currentCamera.lastY = ypos;
				currentCamera.firstMouse = false;
			}

			xoffset = xpos - currentCamera.lastX;
			yoffset = currentCamera.lastY - ypos; // reversed since y-coordinates go from bottom to top

			currentCamera.lastX = xpos;
			currentCamera.lastY = ypos;

			currentCamera.ProcessMouseMovement(xoffset, yoffset);
			if ((xpos >= width-15) || (xpos <= 15) || (ypos >= height-15) || (ypos <= 15)) {
				currentCamera.firstMouse = true;
				SetCursorPos(sX+(width/2.0f), sY+(height/2.0f));
			}
			damage(1);
			break;
		// Mouse button being pushed event
		case FL_PUSH:
			last_push = Fl::event_button();
			// if the left button be pushed is left mouse button
			if (last_push == FL_LEFT_MOUSE  ) {
				std::cout << "left click!\n";
				//doPick();
				//damage(1);
				return 1;
			};
			break;

	   // Mouse button release event
		case FL_RELEASE: // button release
			damage(1);
			last_push = 0;
			return 1;

		// Mouse button drag event
		case FL_DRAG:

			break;

		// in order to get keyboard events, we need to accept focus
		case FL_FOCUS:
			return 1;

		// every time the mouse enters this window, aggressively take focus
		case FL_ENTER:	
			ShowCursor(false);
			mouseIn = true;
			focus(this);
			break;
		case FL_LEAVE:
			ShowCursor(true);
			mouseIn = false;
			break;
		

		case FL_KEYDOWN:
		 		k = Fl::event_key();
				ks = Fl::event_state();
				if (k == 'p') {
					// Print out the selected control point information
					if (selectedCube >= 0) 
						printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
								 selectedCube,
								 m_pTrack->points[selectedCube].pos.x,
								 m_pTrack->points[selectedCube].pos.y,
								 m_pTrack->points[selectedCube].pos.z,
								 m_pTrack->points[selectedCube].orient.x,
								 m_pTrack->points[selectedCube].orient.y,
								 m_pTrack->points[selectedCube].orient.z);
					else
						printf("Nothing Selected\n");

					return 1;
				}
				else if (k == 'w') {
					currentCamera.ProcessKeyboard(FORWARD,true);
				}
				else if (k == 's') {
					currentCamera.ProcessKeyboard(BACKWARD,true);
				}
				else if (k == 'a') {
					currentCamera.ProcessKeyboard(LEFT,true);
				}
				else if (k == 'd') {
					currentCamera.ProcessKeyboard(RIGHT,true);
				}
				else if (k == 'e') {
					interact();
				}
				else if (k == 'x') {
					wolfFlag = !wolfFlag;
				}
				else if (k == 'c') {
					if (tw->trainCam->value()==1) {
						tw->worldCam->setonly();
					}
					else if (tw->worldCam->value()==1) {
						tw->trainCam->setonly();
					}
				}
				else if (k == 'z') {
					if (currentCharacter->camera.MovementSpeed <= 5.0f)
						currentCharacter->camera.MovementSpeed = 100.0f;
					else
						currentCharacter->camera.MovementSpeed = 5.0f;
				}
				else if (k == 32) {
					if (tw->runButton->value() == 0)
						tw->runButton->value(1);
					else
						tw->runButton->value(0);
				}
				else if (k == 'v') {
					ShowCursor(false);
				}
				else if (k == 'j') {
					subTimes++;
					cubeModel = new Model("..\\src\\objects\\cube\\untitled.obj", subTimes);
				}
				else if (k == 'k') {
					subTimes--;
					if (subTimes < 0)
						subTimes = 0;
					cubeModel = new Model("..\\src\\objects\\cube\\untitled.obj", subTimes);
				}
				break;
		case FL_KEYUP:
			k = Fl::event_key();
			ks = Fl::event_state();

			if (k == 'w') {
				currentCamera.ProcessKeyboard(FORWARD, false);
			}
			else if (k == 's') {
				currentCamera.ProcessKeyboard(BACKWARD, false);
			}
			else if (k == 'a') {
				currentCamera.ProcessKeyboard(LEFT, false);
			}
			else if (k == 'd') {
				currentCamera.ProcessKeyboard(RIGHT, false);
			}
			break;
		
	}

	return Fl_Gl_Window::handle(event);
}
void TrainView::update() {

	if (!wakeupTimer->initiation&&wakeupTimer->started) {
		tw->topCam->setonly();
		eventCharacter->position = glm::vec3(-115, 14, 326);
		eventCharacter->camera.setCamera(-90, 89.9);
		eventCharacter->FocusPosition = true;
		eventCharacter->FocusCamera = true;
		wakeupTimer->initiation = true;
	}
	else if (!wakeupTimer->finitiation && wakeupTimer->finished) {
		tw->trainCam->setonly();
		currentCharacter = mainCharacter;
		mainCharacter->position = eventCharacter->position;
		mainCharacter->position.y = 0;
		mainCharacter->camera.Pitch = eventCharacter->camera.Pitch;
		mainCharacter->camera.Yaw = eventCharacter->camera.Yaw;
		wakeupTimer->finitiation = true;
	}
	if (wakeupTimer->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (wakeupTimer->getPassTime() < 100) {
			if (wakeupTimer->getPassTime() == 40) {
				blackScreen = 3;
			}
			else if (wakeupTimer->getPassTime() == 55) {
				blackScreen = 2;
			}
			else if (wakeupTimer->getPassTime() == 70) {
				blackScreen = 1;
			}
			else if (wakeupTimer->getPassTime() == 85) {
				blackScreen = 0;
			}
		}
		else if (wakeupTimer->getPassTime() < 150) {
			eventCharacter->position.z += 0.5;
			eventCharacter->camera.Pitch -= 90.0f / 50.0f;
		}
		else if(wakeupTimer->getPassTime()<200){
			eventCharacter->camera.Yaw += 90.0f / 50.0f;
		}
		else{
			eventCharacter->position.x += 0.5;
		}
		wakeupTimer->update();
	}

	if (!watchTvTimer->initiation&&watchTvTimer->started) {
		tw->topCam->setonly();
		eventCharacter->position = glm::vec3(120, 40, 290);
		eventCharacter->camera.setCamera(90, 0);
		eventCharacter->FocusPosition = true;
		eventCharacter->FocusCamera = true;
		watchTvTimer->initiation = true;
	}
	else if (!watchTvTimer->finitiation && watchTvTimer->finished) {
		newsToDraw = -1;
		tw->trainCam->setonly();
		currentCharacter = mainCharacter;
		mainCharacter->position = eventCharacter->position;
		mainCharacter->position.y = 0;
		mainCharacter->camera.Pitch = eventCharacter->camera.Pitch;
		mainCharacter->camera.Yaw = eventCharacter->camera.Yaw;
		watchTvTimer->finitiation = true;
	}
	if (watchTvTimer->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (watchTvTimer->getPassTime() < 75) {
			eventCharacter->camera.Yaw -= 180.0f / 75.0f;
		}
		else if (watchTvTimer->getPassTime() < 100) {
			eventCharacter->position.z += 0.6;
		}
		else if (watchTvTimer->getPassTime() < 150) {
			eventCharacter->position.y -= 0.5;
		}
		else if (watchTvTimer->getPassTime() < 300) {
		}
		else if (watchTvTimer->getPassTime() < 400) {
			newsToDraw = 0;
		}
		else if (watchTvTimer->getPassTime() < 500) {
			newsToDraw = 1;
		}
		else if (watchTvTimer->getPassTime() < 600) {
			newsToDraw = 2;
		}
		else if (watchTvTimer->getPassTime() < 700) {
			newsToDraw = 3;
		}
		watchTvTimer->update();
	}

	if (!outTimer->initiation && outTimer->started) {
		tw->trainCam->setonly();
		this->scene = 2;
		outTimer->initiation = true;
	}
	else if (!outTimer->finitiation && outTimer->finished) {
		tw->trainCam->setonly();
		currentCharacter = mainCharacter;
		outTimer->finitiation = true;
	}
	if (outTimer->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (outTimer->getPassTime() == 15) {
			blackScreen = 1;
		}
		else if (outTimer->getPassTime() == 20) {
			blackScreen = 2;
		}
		else if (outTimer->getPassTime() == 25) {
			blackScreen = 3;
		}
		else if (outTimer->getPassTime() == 30) {
			blackScreen = 4;
		}
		else if (outTimer->getPassTime() == 50) {
			mainCharacter->position = glm::vec3(-1, 0, 150);
			mainCharacter->camera.setCamera(90, 0);
		}
		else if (outTimer->getPassTime() == 80) {
			blackScreen = 3;
		}
		else if (outTimer->getPassTime() == 85) {
			blackScreen = 2;
		}
		else if (outTimer->getPassTime() == 90) {
			blackScreen = 1;
		}
		else if (outTimer->getPassTime() == 95) {
			blackScreen = 0;
		}
		outTimer->update();
	}


	if (!goTrainTiemr->initiation && goTrainTiemr->started) {
		alSourceStop(this->source);
		goTrainTiemr->initiation = true;
	}
	else if (!goTrainTiemr->finitiation && goTrainTiemr->finished) {
		tw->trainCam->setonly();
		blackScreen = 0;
		goTrainTiemr->finitiation = true;
	}
	if (goTrainTiemr->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (goTrainTiemr->getPassTime() == 5) {
			blackScreen = 1;
		}
		else if (goTrainTiemr->getPassTime() == 10) {
			blackScreen = 2;
		}
		else if (goTrainTiemr->getPassTime() == 15) {
			blackScreen = 3;
		}
		else if (goTrainTiemr->getPassTime() == 20) {
			blackScreen = 4;
		}
		else if (goTrainTiemr->getPassTime() == 25) {
			tw->topCam->setonly();
			eventCharacter->position = cablePoints[currentTrain];
			eventCharacter->position.y -= 400;
			eventCharacter->camera.setCamera(-90, 0);
			eventCharacter->FocusPosition = true;
			eventCharacter->FocusCamera = false;
		}
		else if (goTrainTiemr->getPassTime() == 45) {
			blackScreen = 3;
		}
		else if (goTrainTiemr->getPassTime() == 50) {
			blackScreen = 2;
		}
		else if (goTrainTiemr->getPassTime() == 55) {
			blackScreen = 1;
		}
		else if (goTrainTiemr->getPassTime() == 60) {
			blackScreen = 0;
		}
		else if (goTrainTiemr->getPassTime() > 60 && goTrainTiemr->getPassTime() < 360) {
			eventCharacter->position = cablePoints[++currentTrain];
			eventCharacter->position.y -= 400;

		}
		else if (goTrainTiemr->getPassTime() == 365) {
			blackScreen = 1;
		}
		else if (goTrainTiemr->getPassTime() == 370) {
			blackScreen = 2;
		}
		else if (goTrainTiemr->getPassTime() == 375) {
			blackScreen = 3;
		}
		else if (goTrainTiemr->getPassTime() == 380) {
			blackScreen = 4;
		}
		else if (goTrainTiemr->getPassTime() == 390) {
			tw->trainCam->setonly();
			currentCharacter = mainCharacter;
			mainCharacter->position = glm::vec3(-885, -6300, -8634);
			mainCharacter->camera.Pitch = eventCharacter->camera.Pitch;
			mainCharacter->camera.Yaw = eventCharacter->camera.Yaw;
		}
		else if (goTrainTiemr->getPassTime() == 400) {
			blackScreen = 3;
		}
		else if (goTrainTiemr->getPassTime() == 405) {
			blackScreen = 2;
		}
		else if (goTrainTiemr->getPassTime() == 415) {
			blackScreen = 1;
		}
		else if (goTrainTiemr->getPassTime() == 419) {
			blackScreen = 0;
		}
		goTrainTiemr->update();
	}
	
	if (!backTrainTimer->initiation && backTrainTimer->started) {
		
		backTrainTimer->initiation = true;
	}
	else if (!backTrainTimer->finitiation && backTrainTimer->finished) {
		tw->trainCam->setonly();
		currentCharacter = mainCharacter;
		mainCharacter->position = glm::vec3(-840, 0, -385);
		currentTrain = 0;
		backTrainTimer->finitiation = true;
	}
	if (backTrainTimer->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (backTrainTimer->getPassTime() == 5) {
			blackScreen = 1;
		}
		else if (backTrainTimer->getPassTime() == 10) {
			blackScreen = 2;
		}
		else if (backTrainTimer->getPassTime() == 15) {
			blackScreen = 3;
		}
		else if (backTrainTimer->getPassTime() == 20) {
			blackScreen = 4;
		}
		else if (backTrainTimer->getPassTime() == 25) {
			tw->topCam->setonly();
			eventCharacter->position = cablePoints[currentTrain];
			eventCharacter->position.y -= 400;
			eventCharacter->camera.setCamera(90, 0);
			eventCharacter->FocusPosition = true;
			eventCharacter->FocusCamera = false;
		}
		else if (backTrainTimer->getPassTime() == 45) {
			blackScreen = 3;
		}
		else if (backTrainTimer->getPassTime() == 50) {
			blackScreen = 2;
		}
		else if (backTrainTimer->getPassTime() == 55) {
			blackScreen = 1;
		}
		else if (backTrainTimer->getPassTime() == 60) {
			blackScreen = 0;
		}
		else if (backTrainTimer->getPassTime() > 60 && backTrainTimer->getPassTime() < 360) {
			eventCharacter->position = cablePoints[++currentTrain];
			eventCharacter->position.y -= 400;

		}
		else if (backTrainTimer->getPassTime() == 365) {
			blackScreen = 1;
		}
		else if (backTrainTimer->getPassTime() == 370) {
			blackScreen = 2;
		}
		else if (backTrainTimer->getPassTime() == 375) {
			blackScreen = 3;
		}
		else if (backTrainTimer->getPassTime() == 380) {
			blackScreen = 4;
		}
		else if (backTrainTimer->getPassTime() == 390) {
			tw->trainCam->setonly();
			currentCharacter = mainCharacter;
			mainCharacter->position = glm::vec3(-840, 0, -385);
			mainCharacter->camera.Pitch = eventCharacter->camera.Pitch;
			mainCharacter->camera.Yaw = eventCharacter->camera.Yaw;
		}
		else if (backTrainTimer->getPassTime() == 400) {
			blackScreen = 3;
		}
		else if (backTrainTimer->getPassTime() == 405) {
			blackScreen = 2;
		}
		else if (backTrainTimer->getPassTime() == 415) {
			blackScreen = 1;
		}
		else if (backTrainTimer->getPassTime() == 419) {
			blackScreen = 0;
		}
		backTrainTimer->update();
	}

	
	if (!rocketTimer->initiation && rocketTimer->started) {
		talkFlag = true;
		rocketTimer->initiation = true;
		alSourcePlay(this->source2);
	}
	else if (!rocketTimer->finitiation && rocketTimer->finished) {
		tw->trainCam->setonly();
		currentCharacter = mainCharacter;
		rocketTimer->finitiation = true;
		fireFlag = false;
		rocketExist = false;
		completeFlag = false;
	}
	if (rocketTimer->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (rocketTimer->getPassTime() < 150) {

		}
		else if (rocketTimer->getPassTime() == 150) {
			talkFlag = false;
			fireFlag = true;
		}
		else if(rocketTimer->getPassTime() > 150 && rocketTimer->getPassTime() < 300){
			rocketPosition.y += 5;
			stirScreen(1);
		}
		else if (rocketTimer->getPassTime() >= 300 && rocketTimer->getPassTime() < 625) {
			rocketPosition.y += 75;
			rocketScale -= 0.02;
		}
		else if (rocketTimer->getPassTime() == 625) {
			completeFlag = true;
		}
		
		rocketTimer->update();
	}

	
	if (!nightTimer->initiation && nightTimer->started) {
		skyboxTexture = nightskyTexture;
		nightTimer->initiation = true;
	}
	else if (!nightTimer->finitiation && nightTimer->finished) {
		blackScreen = 4;
		nightTimer->finitiation = true;
	}
	if (nightTimer->working) {
		//std::cout << wakeupTimer->getPassTime() << "\n";
		if (nightTimer->getPassTime() == 5) {
			blackScreen = 1;
		}
		else if (nightTimer->getPassTime() == 10) {
			blackScreen = 2;
		}
		else if (nightTimer->getPassTime() == 15) {
			blackScreen = 3;
		}
		else if (nightTimer->getPassTime() == 20) {
			blackScreen = 4;
		}
		else if (nightTimer->getPassTime() == 25) {
		}
		else if (nightTimer->getPassTime() == 50) {
			blackScreen = 0;
		}
		else if (nightTimer->getPassTime() == 60) {
			thatFlag = true;
		}
		else if (nightTimer->getPassTime() == 100) {
			thatFlag = false;
		}
		else if (nightTimer->getPassTime() > 100 && nightTimer->getPassTime() < 500) {
			rockPosition += glm::vec3(0, -50, 50);
			rockSize += 4.5;
			stirScreen((nightTimer->getPassTime() - 150) / 100);
		}
		nightTimer->update();
	}

	mainCharacter->scene = this->scene;
	currentCharacter->update();

	//std::cout << currentCharacter->position.x << " " << currentCharacter->position.y << " " << currentCharacter->position.z << " " << currentCharacter->camera.Yaw  << " " << currentCharacter->camera.Pitch << "\n";
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		
		if (!this->shader)
			this->shader = new
			Shader(
				PROJECT_DIR "/src/shaders/simple.vert",
				nullptr, nullptr, nullptr, 
				PROJECT_DIR "/src/shaders/simple.frag");

		if (!this->commom_matrices)
			this->commom_matrices = new UBO();
			this->commom_matrices->size = 2 * sizeof(glm::mat4);
			glGenBuffers(1, &this->commom_matrices->ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
			glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (!this->plane) {
			GLfloat  vertices[] = {
				-0.5f ,0.0f , -0.5f,
				-0.5f ,0.0f , 0.5f ,
				0.5f ,0.0f ,0.5f ,
				0.5f ,0.0f ,-0.5f };
			GLfloat  normal[] = {
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f };
			GLfloat  texture_coordinate[] = {
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f };
			GLuint element[] = {
				0, 1, 2,
				0, 2, 3, };

			this->plane = new VAO;
			this->plane->element_amount = sizeof(element) / sizeof(GLuint);
			glGenVertexArrays(1, &this->plane->vao);
			glGenBuffers(3, this->plane->vbo);
			glGenBuffers(1, &this->plane->ebo);

			glBindVertexArray(this->plane->vao);

			// Position attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			// Normal attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);

			// Texture Coordinate attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(2);

			//Element attribute
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->plane->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

			// Unbind VAO
			glBindVertexArray(0);
		}

		if (!this->texture)
			this->texture = new Texture2D(PROJECT_DIR "/Images/church.png");

		if (!this->device){
			//Tutorial: https://ffainelli.github.io/openal-example/
			this->device = alcOpenDevice(NULL);
			if (!this->device)
				puts("ERROR::NO_AUDIO_DEVICE");

			ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
			if (enumeration == AL_FALSE)
				puts("Enumeration not supported");
			else
				puts("Enumeration supported");

			this->context = alcCreateContext(this->device, NULL);
			if (!alcMakeContextCurrent(context))
				puts("Failed to make context current");

			this->source_pos = glm::vec3(-40, 0, 355);

			ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
			alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alListener3f(AL_VELOCITY, 0, 0, 0);
			alListenerfv(AL_ORIENTATION, listenerOri);

			alGenSources((ALuint)1, &this->source);
			alSourcef(this->source, AL_PITCH, 1);
			alSourcef(this->source, AL_GAIN, 1.0f);
			alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
			alSourcei(this->source, AL_LOOPING, AL_TRUE);

			alGenBuffers((ALuint)1, &this->buffer);

			ALsizei size, freq;
			ALenum format;
			ALvoid* data;
			ALboolean loop = AL_TRUE;

			//Material from: ThinMatrix
			alutLoadWAVFile((ALbyte*)"..\\src\\music\\JAZZ.wav", &format, &data, &size, &freq, &loop);
			alBufferData(this->buffer, format, data, size, freq);
			alSourcei(this->source, AL_BUFFER, this->buffer);

			///////////////////////////
			alGenSources((ALuint)1, &this->source2);
			alSourcef(this->source2, AL_PITCH, 1);
			alSourcef(this->source2, AL_GAIN, 1.0f);
			alSource3f(this->source2, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alSource3f(this->source2, AL_VELOCITY, 0, 0, 0);
			alSourcei(this->source2, AL_LOOPING, AL_FALSE);

			alGenBuffers((ALuint)1, &this->buffer2);

			ALsizei size2, freq2;
			ALenum format2;
			ALvoid* data2;
			ALboolean loop2 = AL_FALSE;

			//Material from: ThinMatrix
			alutLoadWAVFile((ALbyte*)"..\\src\\music\\chat.wav", &format2, &data2, &size2, &freq2, &loop2);
			alBufferData(this->buffer2, format2, data2, size2, freq2);
			alSourcei(this->source2, AL_BUFFER, this->buffer2);
			//////////////////////////


			if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
				puts("TYPE::STEREO");
			else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
				puts("TYPE::MONO");

			//alSourcePlay(this->source);

			// cleanup context
			//alDeleteSources(1, &source);
			//alDeleteBuffers(1, &buffer);
			//device = alcGetContextsDevice(context);
			//alcMakeContextCurrent(NULL);
			//alcDestroyContext(context);
			//alcCloseDevice(device);
		}
		if (!this->skyboxShader) {
			skyboxShader = new Shader("..\\src\\shaders\\skybox.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\skybox.frag");
			float skyboxVertices[] = {
				// positions          
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
			};
			glGenVertexArrays(1, &skyboxVAO);
			glGenBuffers(1, &skyboxVBO);
			glBindVertexArray(skyboxVAO);
			glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

			vector<std::string> faces
			{
				"..\\src\\images\\skybox\\right.jpg",
				"..\\src\\images\\skybox\\left.jpg",
				"..\\src\\images\\skybox\\top.jpg",
				"..\\src\\images\\skybox\\bottom.jpg",
				"..\\src\\images\\skybox\\front.jpg",
				"..\\src\\images\\skybox\\back.jpg",
			};
			skyboxTexture = loadCubemap(faces);
			vector<std::string> faces2
			{
				"..\\src\\images\\skybox\\right.png",
				"..\\src\\images\\skybox\\left.png",
				"..\\src\\images\\skybox\\top.png",
				"..\\src\\images\\skybox\\bottom.png",
				"..\\src\\images\\skybox\\front.png",
				"..\\src\\images\\skybox\\back.png",
			};
			nightskyTexture = loadCubemap(faces2);
		}
		if (!this->blackscreenShader) {
			blackscreenShader = new Shader("..\\src\\shaders\\blackscreen.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\blackscreen.frag");
			float blackscreenVertices[] = {
				// positions          
				-1.0f,  1.0f, 0.0f,
				-1.0f, -1.0f, 0.0f,
				-0.5f, -1.0f, 0.0f,
				-0.5f, -1.0f, 0.0f,
				-0.5f,  1.0f, 0.0f,
				-1.0f,  1.0f, 0.0f,

				-0.5f,  1.0f, 0.0f,
				-0.5f, -1.0f, 0.0f,
				 0.0f, -1.0f, 0.0f,
				 0.0f, -1.0f, 0.0f,
				 0.0f,  1.0f, 0.0f,
				-0.5f,  1.0f, 0.0f,

				 0.0f,  1.0f, 0.0f,
				 0.0f, -1.0f, 0.0f,
				 0.5f, -1.0f, 0.0f,
				 0.5f, -1.0f, 0.0f,
				 0.5f,  1.0f, 0.0f,
				 0.0f,  1.0f, 0.0f,

				 0.5f,  1.0f, 0.0f,
				 0.5f, -1.0f, 0.0f,
				 1.0f, -1.0f, 0.0f,
				 1.0f, -1.0f, 0.0f,
				 1.0f,  1.0f, 0.0f,
				 0.5f,  1.0f, 0.0f,
			};
			glGenVertexArrays(1, &blackscreenVAO);
			glGenBuffers(1, &blackscreenVBO);
			glBindVertexArray(blackscreenVAO);
			glBindBuffer(GL_ARRAY_BUFFER, blackscreenVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(blackscreenVertices), &blackscreenVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}
		if (!this->textShader) {
			textShader = new Shader("..\\src\\shaders\\text.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\text.frag");
			glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(w()), 0.0f, static_cast<float>(h()));
			textShader->Use();
			glUniformMatrix4fv(glGetUniformLocation(textShader->Program, "projection"), 1, GL_FALSE, &projection[0][0]);

			if (FT_Init_FreeType(&ft))
			{
				std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			}

			// find path to font
			std::string font_name = "..\\src\\fonts\\Antonio-VariableFont_wght.ttf";
			if (font_name.empty())
			{
				std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
			}

			// load font as face
			if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
				std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			}
			else {
				// set size to load glyphs as
				FT_Set_Pixel_Sizes(face, 0, 48);

				// disable byte-alignment restriction
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				// load first 128 characters of ASCII set
				for (unsigned char c = 0; c < 128; c++)
				{
					// Load character glyph 
					if (FT_Load_Char(face, c, FT_LOAD_RENDER))
					{
						std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
					}
					// generate texture
					unsigned int texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_RED,
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows,
						0,
						GL_RED,
						GL_UNSIGNED_BYTE,
						face->glyph->bitmap.buffer
					);
					// set texture options
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					// now store character for later use
					TextC character = {
						texture,
						glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
						glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
						static_cast<unsigned int>(face->glyph->advance.x)
					};
					Characters.insert(std::pair<char, TextC>(c, character));
				}
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			// destroy FreeType once we're finished
			FT_Done_Face(face);
			FT_Done_FreeType(ft);

			glGenVertexArrays(1, &textVAO);
			glGenBuffers(1, &textVBO);
			glBindVertexArray(textVAO);
			glBindBuffer(GL_ARRAY_BUFFER, textVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		if (!this->billShader) {
			billShader = new Shader("..\\src\\shaders\\billboard.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\billboard.frag");
			float picVertices[] = {
				// positions          
				-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
				-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,

				-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
				 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,


			};
			glGenVertexArrays(1, &billVAO);
			glGenBuffers(1, &billVBO);
			glBindVertexArray(billVAO);
			glBindBuffer(GL_ARRAY_BUFFER, billVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(picVertices), &picVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			treeTexture = loadTexture("..\\src\\images\\tree.png");
		}
		if (!this->picShader) {
			picShader = new Shader("..\\src\\shaders\\picture.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\picture.frag");
			float picVertices[] = {
				// positions          
				-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
				-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
				
				-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
				 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,

				
			};
			glGenVertexArrays(1, &picVAO);
			glGenBuffers(1, &picVBO);
			glBindVertexArray(picVAO);
			glBindBuffer(GL_ARRAY_BUFFER, picVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(picVertices), &picVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
			unsigned int news1 = loadTexture("..\\src\\images\\news\\news1.jpg");
			unsigned int news2 = loadTexture("..\\src\\images\\news\\news2.jpg");
			unsigned int news3 = loadTexture("..\\src\\images\\news\\news3.jpg");
			unsigned int news4 = loadTexture("..\\src\\images\\news\\news4.jpg");
			news.push_back(news1);
			news.push_back(news2);
			news.push_back(news3);
			news.push_back(news4);

		}
		if (!this->particleShader) {
			particleShader = new Shader("..\\src\\shaders\\particle.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\particle.frag");
			fireTexture = loadTexture("..\\src\\images\\fire.png");
			ps = new ParticleSystem(particleShader);
		}
		if (!basicShader) {
			basicShader = new Shader("..\\src\\shaders\\basic.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\basic.frag");
			glGenVertexArrays(1, &basicVAO);
			glGenBuffers(1, &basicVBO);
			glBindVertexArray(basicVAO);
			glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
			glBufferData(GL_ARRAY_BUFFER, 108*sizeof(float), nullptr, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}
		if (!this->oShader) {
			oShader = new Shader("..\\src\\shaders\\easy.vert", nullptr, nullptr, nullptr, "..\\src\\shaders\\easy.frag");
			houseModel = new Model("..\\src\\objects\\house\\house.obj",0);
			sceneModel = new Model("..\\src\\objects\\scene\\scene.obj",0);
			radioModel = new Model("..\\src\\objects\\radio\\radio.obj",0);
			boxModel = new Model("..\\src\\objects\\box\\Box.obj",0);
			rocketModel = new Model("..\\src\\objects\\rocket\\12215_rocket_v3_l1.obj",0);
			talkModel = new Model("..\\src\\objects\\talk\\Radio (Lost signal).obj",0);
			rockModel = new Model("..\\src\\objects\\rock\\Rock1.obj",0);
			cubeModel = new Model("..\\src\\objects\\cube\\untitled.obj",0);
		}
		if (!this->aniShader) {
			aniShader = new Shader("..\\src\\shaders\\animation.vert", nullptr, nullptr, nullptr,"..\\src\\shaders\\animation.frag");
			wolfModel = new Model("..\\src\\objects\\wolf\\Wolf_dae.dae",0);
			wolfAnimation = new Animation("..\\src\\objects\\wolf\\Wolf_dae.dae", wolfModel);
			barkAnimation = new Animation("..\\src\\objects\\wolf\\Wolf_One_dae.dae", wolfModel);
			wolfAnimator = new Animator(wolfAnimation);
			barkAnimator = new Animator(barkAnimation);
			soldierModel = new Model("..\\src\\objects\\soldier\\so.fbx",0);
			soldierAnimation = new Animation("..\\src\\objects\\soldier\\so.fbx", soldierModel);
			soldierWaveAnimation = new Animation("..\\src\\objects\\soldier\\sa.fbx", soldierModel);
			soldierAnimator = new Animator(soldierAnimation);
			waveAnimator = new Animator(soldierWaveAnimation);
		}
		
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");


	

	// Set up the view port
	glViewport(0,0,w(),h());

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0,0,.3f,0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);



	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here

	//######################################################################
	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection
	//######################################################################
	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	} else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[]	= {0,1,1,0}; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[]	= {1, 0, 0, 0};
	GLfloat lightPosition3[]	= {0, -1, 0, 0};
	GLfloat yellowLight[]		= {0.5f, 0.5f, .1f, 1.0};
	GLfloat whiteLight[]			= {1.0f, 1.0f, 1.0f, 1.0};
	GLfloat blueLight[]			= {.1f,.1f,.3f,1.0};
	GLfloat grayLight[]			= {.3f, .3f, .3f, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	// set linstener position 
	if(selectedCube >= 0)
		alListener3f(AL_POSITION, 
			m_pTrack->points[selectedCube].pos.x,
			m_pTrack->points[selectedCube].pos.y,
			m_pTrack->points[selectedCube].pos.z);
	else
		alListener3f(AL_POSITION, 
			this->source_pos.x, 
			this->source_pos.y,
			this->source_pos.z);


	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	setupFloor();
	glDisable(GL_LIGHTING);
	//drawFloor(200,10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}

	setUBO();
	glBindBufferRange(
		GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

	//bind shader
	this->shader->Use();

	glm::mat4 model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, this->source_pos);
	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));
	glUniformMatrix4fv(
		glGetUniformLocation(this->shader->Program, "u_model"), 1, GL_FALSE, &model_matrix[0][0]);
	glUniform3fv(
		glGetUniformLocation(this->shader->Program, "u_color"), 
		1, 
		&glm::vec3(0.0f, 1.0f, 0.0f)[0]);
	this->texture->bind(0);
	glUniform1i(glGetUniformLocation(this->shader->Program, "u_texture"), 0);
	
	//bind VAO
	glBindVertexArray(this->plane->vao);

	//glDrawElements(GL_TRIANGLES, this->plane->element_amount, GL_UNSIGNED_INT, 0);

	//unbind VAO
	glBindVertexArray(0);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);



	projection = currentCharacter->camera.projection;
	view = currentCharacter->camera.view;
	
	
	
	
	
	
	oShader->Use();
	glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "projection"), 1, false, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "view"), 1, false, &view[0][0]);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -50.0f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(30.0f, 30.0f, 30.0f));	// it's a bit too big for our scene, so scale it down
	glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &model[0][0]);
	houseModel->Draw(*oShader);
	sceneModel->Draw(*oShader);
	glm::mat4 radioPos = glm::mat4(1.0f);
	radioPos = glm::translate(radioPos, glm::vec3(-40, 0, 355));
	radioPos = glm::scale(radioPos, glm::vec3(5.0f,5.0f,5.0f));
	glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &radioPos[0][0]);
	radioModel->Draw(*oShader);
	glm::mat4 boxPos = glm::mat4(1.0f);
	boxPos = glm::translate(boxPos, glm::vec3(cablePoints[currentTrain].x, cablePoints[currentTrain].y-450, cablePoints[currentTrain].z));
	boxPos = glm::scale(boxPos, glm::vec3(0.3f, 0.3f, 0.3f));
	glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &boxPos[0][0]);
	boxModel->Draw(*oShader);

	if (rocketExist) {
		glm::mat4 rocketPos = glm::mat4(1.0f);

		rocketPos = glm::translate(rocketPos, rocketPosition);
		rocketPos = glm::rotate(rocketPos, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		rocketPos = glm::scale(rocketPos, glm::vec3(rocketScale, rocketScale, rocketScale));

		glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &rocketPos[0][0]);
		rocketModel->Draw(*oShader);
	}
	

	if (talkFlag) {
		glm::mat4 talkPos = glm::mat4(1.0f);
		glm::vec3 talkPosition = currentCharacter->camera.Position + glm::normalize(currentCharacter->camera.Front) * 10.0f;
		talkPos = glm::translate(talkPos, talkPosition);
		talkPos = glm::rotate(talkPos, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		talkPos = glm::rotate(talkPos, glm::radians(-currentCharacter->camera.Yaw-90), glm::vec3(0, 0, 1));
		talkPos = glm::scale(talkPos, glm::vec3(0.5f, 0.5f, 0.5f));

		glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &talkPos[0][0]);
		talkModel->Draw(*oShader);
	}

	if (nightTimer->started) {
		glm::mat4 rockPos = glm::mat4(1.0f);
		rockPos = glm::translate(rockPos, rockPosition);
		rockPos = glm::scale(rockPos, glm::vec3(rockSize, rockSize, rockSize));

		glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &rockPos[0][0]);
		rockModel->Draw(*oShader);
	}
	
	
	glm::mat4 cubePos = glm::mat4(1.0f);
	cubePos = glm::translate(cubePos, glm::vec3(300,0,-300));
	cubePos = glm::scale(cubePos, glm::vec3(100, 100, 100));
	glUniformMatrix4fv(glGetUniformLocation(oShader->Program, "model"), 1, false, &cubePos[0][0]);
	cubeModel->Draw(*oShader);
	
	
	
	
	
	
	
	
	
	
	
	if (!backTrainFlag) {
		aniShader->Use();
		glUniformMatrix4fv(glGetUniformLocation(aniShader->Program, "projection"), 1, false, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(aniShader->Program, "view"), 1, false, &view[0][0]);
		
		bool wolfStop = false;
		float disToWolf = Caldistance(wolfPoints[currentWolf], currentCharacter->position);
		if (disToWolf < 200) {
			wolfPlayAnimator = barkAnimator;
			wolfStop == true;
		}
		else {
			wolfStop == false;
			wolfPlayAnimator = wolfAnimator;
			currentWolf++;
			if (currentWolf >= wolfPoints.size())
				currentWolf = 0;
		}
		if (wolfFlag)
			wolfPlayAnimator->UpdateAnimation(0.1f);
		auto wolfTransforms = wolfPlayAnimator->GetFinalBoneMatrices();
		for (int i = 0; i < wolfTransforms.size(); ++i)
			glUniformMatrix4fv(glGetUniformLocation(aniShader->Program, ("finalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, false, &wolfTransforms[i][0][0]);

		// render the loaded model
		model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(-115.0f, -6420.0f, -7865.0f)); // translate it down so it's at the center of the scene
		model = glm::translate(model, wolfPoints[currentWolf]); // translate it down so it's at the center of the scene
		if(wolfStop)
			model = glm::rotate(model, glm::radians(-currentCharacter->camera.Yaw - 90), glm::vec3(0, 1, 0));
		else
			model = glm::rotate(model, glm::radians((360.0f / wolfPoints.size()) * currentWolf + 180), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));	// it's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(aniShader->Program, "model"), 1, false, &model[0][0]);

		

		wolfModel->Draw(*aniShader);



		glm::vec3 soldierPos = wolfPoints[currentSoldier];
		soldierPos.x -= 400;
		bool soldierStop = false;
		float disToSoldier = Caldistance(soldierPos, currentCharacter->position);
		if (disToSoldier < 200) {
			soldierPlayAnimator = waveAnimator;
			soldierStop = true;
			if (soldierFlag) {
				if (soldierWave < 3) {
					soldierPlayAnimator->UpdateAnimation(0.1f);
					soldierWave += 0.1f;
				}
			}
		}
		else {
			soldierStop = false;
			soldierPlayAnimator = soldierAnimator;
			soldierWave = 0;
			if (soldierFlag) {
				currentSoldier++;
				if (currentSoldier >= wolfPoints.size())
					currentSoldier = 0;
				soldierPlayAnimator->UpdateAnimation(0.04f);
			}
				
		}
		auto soldierTransforms = soldierPlayAnimator->GetFinalBoneMatrices();
		for (int i = 0; i < soldierTransforms.size(); ++i)
			glUniformMatrix4fv(glGetUniformLocation(aniShader->Program, ("finalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, false, &soldierTransforms[i][0][0]);

		// render the loaded model
		model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(-115.0f, -6420.0f, -7865.0f)); // translate it down so it's at the center of the scene
		
		model = glm::translate(model, soldierPos); // translate it down so it's at the center of the scene
		if(soldierStop)
			model = glm::rotate(model, glm::radians(-currentCharacter->camera.Yaw - 90), glm::vec3(0, 1, 0));
		else
			model = glm::rotate(model, glm::radians((360.0f / wolfPoints.size()) * currentSoldier + 180), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(aniShader->Program, "model"), 1, false, &model[0][0]);


		soldierModel->Draw(*aniShader);
	}

	
	
	
	if (newsToDraw != -1) {
		picShader->Use();
		//glm::mat4 newsModel(1.0f);
		//newsModel = glm::translate(newsModel, glm::vec3(121.0f, 40.0f, 323.0f)); // translate it down so it's at the center of the scene
		//newsModel = glm::scale(newsModel, glm::vec3(30.0f, 30.0f, 30.0f));	// it's a bit too big for our scene, so scale it down
		glUniform1i(glGetUniformLocation(picShader->Program, "texture1"), 0);
		glBindVertexArray(picVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, news[newsToDraw]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	
	
	
	
	
	
	
	
	blackscreenShader->Use();
	glBindVertexArray(blackscreenVAO);
	switch (blackScreen)
	{
	case 0:
		break;
	case 1:
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	case 2:
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawArrays(GL_TRIANGLES, 12, 6);
		break;
	case 3:
		glDrawArrays(GL_TRIANGLES, 0, 18);
		break;
	case 4:
		glDrawArrays(GL_TRIANGLES, 0, 24);
		break;
	}
	glBindVertexArray(0);

	

	



	
	
	
	
	//skybox has to be the last one
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxShader->Use();
	glm::mat4 skyboxModel(1.0f);
	skyboxModel = glm::translate(skyboxModel, currentCharacter->camera.Position);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader->Program, "view"), 1, false, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader->Program, "projection"), 1, false, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader->Program, "model"), 1, false, &skyboxModel[0][0]);




	// skybox cube
	glUniform1i(glGetUniformLocation(skyboxShader->Program, "skybox"), 0);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	billShader->Use();
	glUniformMatrix4fv(glGetUniformLocation(billShader->Program, "projection"), 1, false, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(billShader->Program, "view"), 1, false, &view[0][0]);
	glm::mat4 bModel(1.0f);
	bModel = glm::translate(bModel, glm::vec3(121.0f, 40.0f, -300.0f)); // translate it down so it's at the center of the scene
	bModel = glm::rotate(bModel, glm::radians(-currentCharacter->camera.Yaw - 90), glm::vec3(0, 1, 0));
	bModel = glm::scale(bModel, glm::vec3(120.0f, 120.0f, 120.0f));	// it's a bit too big for our scene, so scale it down
	glUniformMatrix4fv(glGetUniformLocation(billShader->Program, "model"), 1, false, &bModel[0][0]);

	glUniform1i(glGetUniformLocation(billShader->Program, "texture1"), 0);
	glBindVertexArray(billVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, treeTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (fireFlag) {
		glm::vec3 tmp = rocketPosition;
		tmp.x += 400;
		tmp.z -= 200;
		
		
		for (int i = 0; i < 100; ++i) {
			ps->spawn(glm::vec3(tmp.x+ fireDis(gen),tmp.y,tmp.z+ fireDis(gen)), glm::vec3(0, -1, 0), glm::vec2(0,0));
		}
		ps->update();
		ps->draw(projection, view, currentCharacter->camera.Yaw, currentCharacter->camera.Pitch,fireTexture);
	}
	glDisable(GL_BLEND);

	Testinteract();

}
void TrainView::RenderText(Shader* shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
	// activate corresponding render state	
	shader->Use();
	glUniform3f(glGetUniformLocation(shader->Program, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		TextC ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value()) {
		currentCharacter = worldCharacter;
		currentCharacter->camera.use(aspect);
		//std::cout << currentCamera->Position.x << " " << currentCamera->Position.y << " " << currentCamera->Position.z << "\n";
	}
	// Or we use the top cam
	else if (tw->topCam->value()) {
		currentCharacter = eventCharacter;
		currentCharacter->camera.use(aspect);
	} 
	// Or do the train view or other view here
	
	else {
		currentCharacter = mainCharacter;
		currentCharacter->camera.use(aspect);
	}

}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void drawCone(float r, float a, float b,glm::mat4 model, glm::vec3 color) {
	glm::vec3 vertices[2];
	vertices[0] = glm::vec3(0, 0, 0);
	for (float i = 0; i < 360; ++i) {
		vertices[1] = glm::vec3(r / a * cos(glm::radians(i)), -r, r / b * sin(glm::radians(i)));
		basicShader->Use();
		glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "view"), 1, false, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "projection"), 1, false, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "model"), 1, false, &model[0][0]);
		glUniform3fv(glGetUniformLocation(basicShader->Program, "color"), 1, &color[0]);
		glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
		glBindVertexArray(basicVAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
	}
	/*glBegin(GL_LINES);
	for (float i = 0; i < 360; ++i) {
		glVertex3f(0, 0, 0);
		glVertex3f(r / a * cos(glm::radians(i)), -r, r / b * sin(glm::radians(i)));
	}
	glEnd();*/
}
void drawCubicDownward(float r,float h, glm::mat4 model, glm::vec3 color) {
	glm::vec3 vertices[24];
	
	vertices[0] = glm::vec3(-r, 0, -r);
	vertices[1] = glm::vec3(r, 0, -r);
	vertices[2] = glm::vec3(r, 0, r);
	vertices[3] = glm::vec3(-r, 0, r);

	vertices[4] = glm::vec3(-r, -h, -r);
	vertices[5] = glm::vec3(r, -h, -r);
	vertices[6] = glm::vec3(r, -h, r);
	vertices[7] = glm::vec3(-r, -h, r);
	
	vertices[8] = glm::vec3(-r, -h, -r);
	vertices[9] = glm::vec3(r, -h, -r);
	vertices[10] = glm::vec3(r, 0, -r);
	vertices[11] = glm::vec3(-r, 0, -r);
	
	vertices[12] = glm::vec3(-r, -h, r);
	vertices[13] = glm::vec3(r, -h, r);
	vertices[14] = glm::vec3(r, 0, r);
	vertices[15] = glm::vec3(-r, 0, r);

	vertices[16] = glm::vec3(-r, -h, r);
	vertices[17] = glm::vec3(-r, -h, -r);
	vertices[18] = glm::vec3(-r, 0, -r);
	vertices[19] = glm::vec3(-r, 0, r);

	vertices[20] = glm::vec3(r, -h, r);
	vertices[21] = glm::vec3(r, -h, -r);
	vertices[22] = glm::vec3(r, 0, -r);
	vertices[23] = glm::vec3(r, 0, r);

	basicShader->Use();
	glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "view"), 1, false, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "projection"), 1, false, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "model"), 1, false, &model[0][0]);
	glUniform3fv(glGetUniformLocation(basicShader->Program, "color"), 1, &color[0]);
	glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
	glBindVertexArray(basicVAO);
	glDrawArrays(GL_QUADS, 0, 24);
	glBindVertexArray(0);
}
void drawTree(glm::mat4 model,float a,float b,float h) {
	glm::vec3 green = glm::vec3(3, 75, 0);
	green /= 255.0f;
	glm::vec3 brown = glm::vec3(150, 75, 0);
	brown /= 255.0f;
	model = glm::translate(model, glm::vec3(0.0f, 100.0f+h, 0.0f));
	drawCone(10, a, b,model,green);
	model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
	drawCone(20, a, b,model, green);
	model = glm::translate(model, glm::vec3(0.0f, -20.0f, 0.0f));
	drawCone(30, a, b,model, green);
	model = glm::translate(model, glm::vec3(0.0f, -30.0f, 0.0f));
	drawCubicDownward(5, 40+h,model,brown);
}
void drawRaill() {
	basicShader->Use();
	glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "view"), 1, false, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "projection"), 1, false, &projection[0][0]);
	glm::mat4 model = glm::mat4(1.0f);
	glm::vec3 color = glm::vec3(0.0, 0.0, 0.0);
	glUniformMatrix4fv(glGetUniformLocation(basicShader->Program, "model"), 1, false, &model[0][0]);
	glUniform3fv(glGetUniformLocation(basicShader->Program, "color"), 1, &color[0]);
	
	for (int i = 0; i < cablePoints.size()-1; ++i) {
		vector<glm::vec3> tmp = vector<glm::vec3>(cableLeft.begin() + i, cableLeft.begin()+i+2);
		glBindBuffer(GL_ARRAY_BUFFER,basicVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tmp), &tmp[0]);
		glBindVertexArray(basicVAO);
		glDrawArrays(GL_LINES, 0, 2);
		tmp = vector<glm::vec3>(cableRight.begin()+i, cableRight.begin() +i+2);
		glBindBuffer(GL_ARRAY_BUFFER,basicVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tmp), &tmp[0]);
		glBindVertexArray(basicVAO);
		glDrawArrays(GL_LINES, 0, 2);
		tmp = vector<glm::vec3>{ cableLeft[i],cableRight[i] };
		glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tmp), &tmp[0]);
		glBindVertexArray(basicVAO);
		glDrawArrays(GL_LINES, 0, 2);
	}
	glm::vec3 bot = cableLeft[currentTrain];
	bot.y -= 450;
	vector<glm::vec3> tmp = { cableLeft[currentTrain],bot };
	glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tmp), &tmp[0]);
	glBindVertexArray(basicVAO);
	glDrawArrays(GL_LINES, 0, 2);
	bot = cableRight[currentTrain];
	bot.y -= 450;
	tmp = { cableRight[currentTrain],bot };
	glBindBuffer(GL_ARRAY_BUFFER, basicVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tmp), &tmp[0]);
	glBindVertexArray(basicVAO);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
}
void TrainView::drawStuff(bool doingShadows)
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(240, 0, 200));
	drawTree(model,1,1,0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(240, 0, 300));
	drawTree(model, 0.5, 0.5, 20);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(240, 0, 360));
	drawTree(model, 3, 3, -10);
	drawRaill();
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	//// draw the cubes, loading the names as we go
	//for(size_t i=0; i<m_pTrack->points.size(); ++i) {
	//	glLoadName((GLuint) (i+1));
	//	m_pTrack->points[i].draw();
	//}

	//// go back to drawing mode, and see how picking did
	//int hits = glRenderMode(GL_RENDER);
	//if (hits) {
	//	// warning; this just grabs the first object hit - if there
	//	// are multiple objects, you really want to pick the closest
	//	// one - see the OpenGL manual 
	//	// remember: we load names that are one more than the index
	//	selectedCube = buf[3]-1;
	//} else // nothing hit, nothing selected
	//	selectedCube = -1;

	//printf("Selected Cube %d\n",selectedCube);
}

void TrainView::setUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TrainView::Testinteract() {
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float Yaw = currentCharacter->camera.Yaw;
	float Pitch = currentCharacter->camera.Pitch;

	//radio yaw 90 pitch -35
	glm::vec3 radioPos = glm::vec3(-40, 0, 355);
	float disToRadio = Caldistance(radioPos, currentCharacter->position);
	if (disToRadio <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch + 35) <= 30 && !radioFlag) {
		RenderText(textShader, "Interact with radio[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	glm::vec3 sofaPos = glm::vec3(120, 0, 313);
	float disToSofa = Caldistance(sofaPos, currentCharacter->position);
	if (disToSofa <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch + 35) <= 30 && !sitFlag) {
		RenderText(textShader, "Sit on sofa and watch TV[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	glm::vec3 doorPos = glm::vec3(-1, 0, 200);
	float disToDoor = Caldistance(doorPos, currentCharacter->position);
	if (disToDoor <= 30 && abs(Yaw + 90) <= 30 && abs(Pitch) <= 30 && !outFlag) {
		RenderText(textShader, "Go out[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	if (disToDoor <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch) <= 30 && !restFlag) {
		RenderText(textShader, "Rest[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	glm::vec3 goTrainPos = glm::vec3(-840, 0, -385);
	float disToGoTrain = Caldistance(goTrainPos, currentCharacter->position);
	if (disToGoTrain <= 30 && abs(Yaw + 180) <= 30 && abs(Pitch-18) <= 30 && !goTrainFlag) {
		RenderText(textShader, "Boarding[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	glm::vec3 backTrainPos = glm::vec3(-885, -6300, -8600);
	float disToBackTrain = Caldistance(backTrainPos, currentCharacter->position);
	if (disToBackTrain <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch - 18) <= 30 && !backTrainFlag && rocketTimer->finished) {
		RenderText(textShader, "Boarding[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	glm::vec3 sendRocketPos = glm::vec3(325, -6360, -6907);
	float disToRocket = Caldistance(sendRocketPos, currentCharacter->position);
	if (disToRocket <= 400 && !rocketFlag) {
		RenderText(textShader, "Launch rocket[e]", 245, 245, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	if (completeFlag) {
		RenderText(textShader, "I have completed my job. Let's go back.", 160, 245, 0.5f, glm::vec3(1.0, 1.0f, 1.0f));
	}
	if (thatFlag) {
		RenderText(textShader, "Wait. What is that?", 200, 245, 0.5f, glm::vec3(1.0, 1.0f, 1.0f));
	}

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
void TrainView::interact() {
	float Yaw = currentCharacter->camera.Yaw;
	float Pitch = currentCharacter->camera.Pitch;
	//radio yaw 90 pitch -35
	glm::vec3 radioPos = glm::vec3(-40, 0, 355);
	float disToRadio = Caldistance(radioPos, currentCharacter->position);
	if (disToRadio <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch + 35) <= 30&&!radioFlag) {
		alSourcePlay(this->source);
		radioFlag = true;
	}
	glm::vec3 sofaPos = glm::vec3(120, 0, 313);
	float disToSofa = Caldistance(sofaPos, currentCharacter->position);
	if (disToSofa <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch + 35) <= 30 && !sitFlag) {
		sitFlag = true;
		watchTvTimer->start(this->uTime);
	}
	glm::vec3 doorPos = glm::vec3(-1, 0, 200);
	float disToDoor = Caldistance(doorPos, currentCharacter->position);
	if (disToDoor <= 30 && abs(Yaw + 90) <= 30 && abs(Pitch) <= 30 && !outFlag) {
		outFlag = true;
		outTimer->start(this->uTime);
	}
	if (disToDoor <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch) <= 30 && !restFlag) {
		restFlag = true;
		nightTimer->start(this->uTime);
	}
	glm::vec3 goTrainPos = glm::vec3(-840, 0, -385);
	float disToGoTrain = Caldistance(goTrainPos, currentCharacter->position);
	if (disToGoTrain <= 30 && abs(Yaw + 180) <= 30 && abs(Pitch - 18) <= 30 && !goTrainFlag) {
		goTrainFlag = true;
		goTrainTiemr->start(this->uTime);
	}
	glm::vec3 backTrainPos = glm::vec3(-885, -6300, -8600);
	float disToBackTrain = Caldistance(backTrainPos, currentCharacter->position);
	if (disToBackTrain <= 30 && abs(Yaw - 90) <= 30 && abs(Pitch - 18) <= 30 && !backTrainFlag && rocketTimer->finished) {
		backTrainFlag = true;
		backTrainTimer->start(this->uTime);

	}
	glm::vec3 sendRocketPos = glm::vec3(325, -6360, -6907);
	float disToRocket = Caldistance(sendRocketPos, currentCharacter->position);
	if (disToRocket <= 400 && !rocketFlag) {
		rocketTimer->start(this->uTime);
		rocketFlag = true;
	}
}