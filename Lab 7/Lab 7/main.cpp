/*
=================
main.cpp
Main entry point for the Card application
=================
*/


#include "GameConstants.h"
#include "GameResources.h"
#include "cD3DManager.h"
#include "cD3DXSpriteMgr.h"
#include "cD3DXTexture.h"
#include "cBalloon.h"
#include "cSprite.h"
#include "cExplosion.h"
#include "cXAudio.h"
#include "cD3DXFont.h"
#include <time.h>

using namespace std;

HINSTANCE hInst; // global handle to hold the application instance
HWND wndHandle; // global variable to hold the window handle

// Get a reference to the DirectX Manager
static cD3DManager* d3dMgr = cD3DManager::getInstance();

// Get a reference to the DirectX Sprite renderer Manager 
static cD3DXSpriteMgr* d3dxSRMgr = cD3DXSpriteMgr::getInstance();

D3DXVECTOR2 balloonTrans = D3DXVECTOR2(300,300);

vector<cBalloon*> aBalloon;
vector<cBalloon*>::iterator iter;
vector<cBalloon*>::iterator index;

RECT clientBounds;
bool gameStarted = false;
bool gameFinished = false;

TCHAR szTempOutput[30];

bool gHit = false;
int gBalloonsBurst = 0;
int lives = 10;
char gBalloonsBurstStr[50];

D3DXVECTOR3 expPos;
list<cExplosion*> gExplode;

cXAudio gExplodeSound;
cXAudio gBackgroundNoise;
cXAudio gZombieNoise;

cD3DXTexture* balloonTextures[4];
char* balloonTxtres[] = {"Images\\Balloon.png","Images\\cyborg.png","Images\\BalloonRed.png","Images\\explosion.png"};

/*
==================================================================
* LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
* LPARAM lParam)
* The window procedure
==================================================================
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Check any available messages from the queue
	switch (message)
	{
		case WM_LBUTTONDOWN:
			{
				if(gameStarted == true)
				{
				POINT mouseXY;
				mouseXY.x = LOWORD(lParam);
				mouseXY.y = HIWORD(lParam);
				
				expPos = D3DXVECTOR3((float)mouseXY.x,(float)mouseXY.y, 0.0f);

				iter = aBalloon.begin();
				while (iter != aBalloon.end() && !gHit)
				{
					if ( (*iter)->insideRect((*iter)->getBoundingRect(),mouseXY))
					{
						OutputDebugString("Hit!\n");
						gHit = true;
						expPos = (*iter)->getSpritePos();
						gExplode.push_back(new cExplosion(expPos,balloonTextures[3]));

						gExplodeSound.playSound(L"Sounds\\explosion.wav",false);
						gExplodeSound.playSound(L"Sounds\\splatter.wav",false);
						iter = aBalloon.erase(iter);
						gBalloonsBurst++;
						sprintf_s( gBalloonsBurstStr, 50, "Score : %d", gBalloonsBurst);
					}
					else
					{
						++iter;
					}
				}

				gHit = false;
				return 0;
				}
				
				if (gameStarted == false)
				{
				gameStarted = true;
				return 0;
				}
				return 0;
			
}
		case WM_CLOSE:
			{
			// Exit the Game
				PostQuitMessage(0);
				 return 0;
			}

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
	}
	// Always return the message to the default window
	// procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
==================================================================
* bool initWindow( HINSTANCE hInstance )
* initWindow registers the window class for the application, creates the window
==================================================================
*/
bool initWindow( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;
	// Fill in the WNDCLASSEX structure. This describes how the window
	// will look to the system
	wcex.cbSize = sizeof(WNDCLASSEX); // the size of the structure
	wcex.style = CS_HREDRAW | CS_VREDRAW; // the class style
	wcex.lpfnWndProc = (WNDPROC)WndProc; // the window procedure callback
	wcex.cbClsExtra = 0; // extra bytes to allocate for this class
	wcex.cbWndExtra = 0; // extra bytes to allocate for this instance
	wcex.hInstance = hInstance; // handle to the application instance
	wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MyWindowIcon)); // icon to associate with the application
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_GUNSIGHT));// the default cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // the background color
	wcex.lpszMenuName = NULL; // the resource name for the menu
	wcex.lpszClassName = "Balloons"; // the class name being created
	wcex.hIconSm = LoadIcon(hInstance,"guns.ico"); // the handle to the small icon

	RegisterClassEx(&wcex);
	// Create the window
	wndHandle = CreateWindow("Balloons",			// the window class to use
							 "Augmented",			// the title bar text
							WS_OVERLAPPEDWINDOW,	// the window style
							CW_USEDEFAULT, // the starting x coordinate
							CW_USEDEFAULT, // the starting y coordinate
							800, // the pixel width of the window
							600, // the pixel height of the window
							NULL, // the parent window; NULL for desktop
							NULL, // the menu for the application; NULL for none
							hInstance, // the handle to the application instance
							NULL); // no values passed to the window
	// Make sure that the window handle that is created is valid
	if (!wndHandle)
		return false;
	// Display the window on the screen
	ShowWindow(wndHandle, SW_SHOW);
	UpdateWindow(wndHandle);
	return true;
}

/*
==================================================================
// This is winmain, the main entry point for Windows applications
==================================================================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	// Initialize the window
	if ( !initWindow( hInstance ) )
		return false;
	// called after creating the window
	if ( !d3dMgr->initD3DManager(wndHandle) )
		return false;
	if ( !d3dxSRMgr->initD3DXSpriteMgr(d3dMgr->getTheD3DDevice()))
		return false;

	// Grab the frequency of the high def timer
	__int64 freq = 0;				// measured in counts per second;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	float sPC = 1.0f / (float)freq;			// number of seconds per count

	__int64 currentTime = 0;				// current time measured in counts per second;
	__int64 previousTime = 0;				// previous time measured in counts per second;

	float numFrames   = 0.0f;				// Used to hold the number of frames
	float timeElapsed = 0.0f;				// cumulative elapsed time

	GetClientRect(wndHandle,&clientBounds);

	float fpsRate = 1.0f/25.0f;

	D3DXVECTOR3 aballoonPos;

	sprintf_s( gBalloonsBurstStr, 50, "AUGMENTED : %d", gBalloonsBurst, "Lives : ", lives);

	//====================================================================
	// Load four textures for the balloons; yellow, green, red & explosion
	//====================================================================
	for (int txture = 0; txture < 4; txture++)
	{
		balloonTextures[txture] = new cD3DXTexture(d3dMgr->getTheD3DDevice(), balloonTxtres[txture]);
	}


	// Initial starting position for balloons
	D3DXVECTOR3 TLballoonPos;


	/* initialize random seed: */
	srand ( (unsigned int)time(NULL) );

	//Main Game Window
	LPDIRECT3DSURFACE9 aSurface;				// the Direct3D surface
	LPDIRECT3DSURFACE9 theBackbuffer = NULL;  // This will hold the back buffer
	
	//Menu Screen Window
	LPDIRECT3DSURFACE9 bSurface;			 // the Direct3D surface
	LPDIRECT3DSURFACE9 bBackBuffer = NULL;	 // This will hold the back buffer

	//End Screen Window
	LPDIRECT3DSURFACE9 cSurface;			// the Direct3D surface
	LPDIRECT3DSURFACE9 cBackBuffer = NULL;	 // This will hold the back buffer


	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );

	
	// Create the background surface
	aSurface = d3dMgr->getD3DSurfaceFromFile("Images\\dirt.jpg");
	bSurface = d3dMgr->getD3DSurfaceFromFile("Images\\mainmenu.png");
	cSurface = d3dMgr->getD3DSurfaceFromFile("Images\\Nighttime.png");
	
	
		// load custom font
	cD3DXFont* balloonFont = new cD3DXFont(d3dMgr->getTheD3DDevice(),hInstance, "Robotaur");

	//Start the background sound

	gBackgroundNoise.playSound(L"sounds\\backbeat.wav",true);


	RECT textPos;
	SetRect(&textPos, 50, 10, 550, 100);

	//define the enemies spawn position, its movement and its texture
		for(int loop = 0; loop < 8; loop ++)
		{
			TLballoonPos = D3DXVECTOR3((float)clientBounds.left + 5,(float)clientBounds.bottom-(75*loop),0);
			aBalloon.push_back(new cBalloon());
			aBalloon[loop]->setSpritePos(TLballoonPos);
			aBalloon[loop]->setTranslation(D3DXVECTOR2(150.0f,0.0f));
			aBalloon[loop]->setTexture(balloonTextures[(1)]);

		}


	QueryPerformanceCounter((LARGE_INTEGER*)&previousTime);

	while( msg.message!=WM_QUIT )
	{
		// Check the message queue
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			//make sure the game hasnt started already (and finished)
			if ((gameStarted == false) && (gameFinished == false))
				{
	
				//Main Menu Screen
				d3dMgr->beginRender();
				theBackbuffer = d3dMgr->getTheBackBuffer();					//THIS IS WHERE WE CAN CHANGE THE WINDOWS
				d3dMgr->updateTheSurface(bSurface, theBackbuffer);
				d3dMgr->releaseTheBackbuffer(theBackbuffer);
				d3dMgr->endRender();
				}

			//Main Game Screen

	
		// Game code goes here
			QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
			float dt = (currentTime - previousTime)*sPC;


			// Accumulate how much time has passed.
			timeElapsed += dt;
				

			/*
			==============================================================
			| Update the postion of the balloons and check for collisions
			==============================================================
			*/
			if(timeElapsed > fpsRate)
			{
				for(iter = aBalloon.begin(); iter != aBalloon.end(); ++iter)
				{
					(*iter)->update(timeElapsed);			// update balloon
					aballoonPos = (*iter)->getSpritePos();  // get the position of the current balloon
					/*
					==============================================================
					| Check for boundry collision and change balloon direction
					==============================================================
					*/
					if (aballoonPos.x>(clientBounds.right - 60) || aballoonPos.x < 2 )
					{
						(*iter)->setTranslation((*iter)->getTranslation()*(-1));
					}
					/*
					==============================================================
					| Check each balloon against each other for collisions
					==============================================================
					*/				
					for(index = aBalloon.begin(); index != aBalloon.end(); ++index)
					{
						if ((*iter)->collidedWith((*iter)->getBoundingRect(),(*index)->getBoundingRect()))
						{
							// if a collision occurs change the direction of each balloon that has collided
							OutputDebugString("Collision!   ");
							(*iter)->setTranslation((*iter)->getTranslation()*(-1));
							(*index)->setTranslation((*index)->getTranslation()*(-1));
						}
					}
				}
				if (gameStarted == true)
				{
				d3dMgr->beginRender();
				theBackbuffer = d3dMgr->getTheBackBuffer();					//THIS IS WHERE WE CAN CHANGE THE WINDOWS
				d3dMgr->updateTheSurface(aSurface, theBackbuffer);
				d3dMgr->releaseTheBackbuffer(theBackbuffer);
				}
				d3dxSRMgr->beginDraw();
				vector<cBalloon*>::iterator iterB = aBalloon.begin();
				for(iterB = aBalloon.begin(); iterB != aBalloon.end(); ++iterB)
				{
					d3dxSRMgr->setTheTransform((*iterB)->getSpriteTransformMatrix());  
					d3dxSRMgr->drawSprite((*iterB)->getTexture(),NULL,NULL,NULL,0xFFFFFFFF);
				
				}
				list<cExplosion*>::iterator iter = gExplode.begin();
				while(iter != gExplode.end())
				{
					if((*iter)->isActive() == false)
					{
						iter = gExplode.erase(iter);
					}
					else
					{
						(*iter)->update(timeElapsed);
						d3dxSRMgr->setTheTransform((*iter)->getSpriteTransformMatrix());  
						d3dxSRMgr->drawSprite((*iter)->getTexture(),&((*iter)->getSourceRect()),NULL,NULL,0xFFFFFFFF);
						++iter;
					}
				}
				d3dxSRMgr->endDraw();
				balloonFont->printText(gBalloonsBurstStr,textPos);
				d3dMgr->endRender();
				timeElapsed = 0.0f;
			
			}

			previousTime = currentTime;




	}

}
	d3dxSRMgr->cleanUp();
	d3dMgr->clean();
	return (int) msg.wParam;
}
