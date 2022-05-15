/************************************************************************
     File:        TrainView.H

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

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

#include <AL/alut.h>
//#include <AL/al.h>
//#include <AL/alc.h>

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"
#include "Camera.H"
#include "Character.h"
#include "Timer.h"
#include <vector>
#include <ft2build.h>
#include "Shader2.h"
#include FT_FREETYPE_H 

class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();


		void update();
		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//set ubo
		void setUBO();

		void Testinteract();
		void interact();
		void RenderText(Shader* shader, std::string text, float x, float y, float scale, glm::vec3 color);
		void stirScreen(float);
	public:
		float			uTime = 0;

		bool			mouseIn = false;

		ArcBallCam		arcball;			// keep an ArcBall for the UI
		int				selectedCube = -1;  // simple - just remember which cube is selected

		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene

		Shader* shader		= nullptr;
		Texture2D* texture	= nullptr;
		VAO* plane			= nullptr;
		UBO* commom_matrices= nullptr;

		//OpenAL
		glm::vec3 source_pos;
		glm::vec3 listener_pos;
		ALCdevice* device	= nullptr;
		ALCcontext* context = nullptr;
		ALuint source;
		ALuint buffer;

		ALuint source2;
		ALuint buffer2;

		Character* currentCharacter = nullptr;
		Character* worldCharacter = nullptr;
		Character* mainCharacter = nullptr;
		Character* eventCharacter = nullptr;

		Shader* oShader = nullptr;
		Shader* skyboxShader = nullptr;
		unsigned int skyboxVAO, skyboxVBO, skyboxTexture, nightskyTexture;
		Shader* blackscreenShader = nullptr;
		unsigned int blackscreenVAO, blackscreenVBO;
		Shader* textShader = nullptr;
		unsigned int textVAO, textVBO;
		Shader* picShader = nullptr;
		unsigned int picVAO, picVBO;
		Shader*  aniShader = nullptr;
		Shader* particleShader = nullptr;
		unsigned int fireTexture;
		Shader* billShader = nullptr;
		unsigned int billVAO, billVBO;

		unsigned int treeTexture;
		int subTimes = 0;
		
		Timer* wakeupTimer = nullptr;
		Timer* watchTvTimer = nullptr;
		Timer* outTimer = nullptr;
		Timer* goTrainTiemr = nullptr;
		Timer* backTrainTimer = nullptr;
		Timer* rocketTimer = nullptr;
		Timer* nightTimer = nullptr;
		
		int newsToDraw = -1;
		int blackScreen = 4;
		int scene = 1;
		int soldierAni = 1;

		bool radioFlag = false;
		bool sitFlag = false;
		bool outFlag = false;
		bool restFlag = false;
		bool goTrainFlag = false;
		bool backTrainFlag = false;
		bool rocketFlag = false;
		bool talkFlag = false;
		bool fireFlag = false;
		bool completeFlag = false;
		bool thatFlag = false;

		bool rocketExist = true;
		bool wolfFlag = true;
		bool soldierFlag = true;
		

		FT_Library ft;
		FT_Face face;

};