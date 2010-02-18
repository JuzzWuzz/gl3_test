/*
    Copyright (C) 2010 Andrew Flower <andrew.flower@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _REGL3_H
#define _REGL3_H


/******************************************************************************
 * TYPEDEFS
 ******************************************************************************/
typedef unsigned int u_int;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

/*
 * AppConfig
 * Contains settings to pass to AppMain for SDL, OpenGL setup.
 */
struct AppConfig{
	AppConfig(){
		fullscreen 	= false;
		VSync		= false;
		resizable	= false;
		lockLogicTimeStep = false;
		winWidth 	= 800;
		winHeight	= 600;
		fsaa		= 0; 		// Fullscreen Antialias (# of samples per pixel)
		sleepTime 	= .01f;		// Delay in seconds
		timeStep	= .02f;		// This applies if the logic time-step is locked.
		gl_major	= 2;
		gl_minor	= 1;
		title		= "SDL1.3 & OpenGL";
	}
	void Print(){
		printf("fullscreen: %d\n", fullscreen);
		printf("VSync: %d\n", VSync);
		printf("resizable: %d\n", resizable);
		printf("winWidth: %d\n", winWidth);
		printf("winHeight: %d\n", winHeight);
		printf("sleepTime: %.3f\n", sleepTime);
		printf("gl_version: %d.%d\n", gl_major, gl_minor);
		printf("title: %s\n", title.c_str());
	}
	bool 	fullscreen;
	bool 	VSync;
	bool	resizable;
	bool	lockLogicTimeStep;
	u_int 	winWidth;
	u_int 	winHeight;
	u_int	fsaa;
	u_int	gl_major;
	u_int	gl_minor;
	float	sleepTime;
	float	timeStep;
	string	title;
};


/******************************************************************************
 * Structs for the Input class
 ******************************************************************************/
struct MousePos{
	float x;
	float y;
};
struct MouseDelta{
	float x;
	float y;
};

/******************************************************************************
 * Input - an input-handler that the reGL3App class uses in order
 * to keep track of keyboard and mouse state.
 ******************************************************************************/
class Input
{
public:
	Input(void);
	~Input(void){}

	void PressKey(SDLKey key);
	void ReleaseKey(SDLKey key);
	void PressButton(Uint8 but);
	void ReleaseButton(Uint8 but);
	void WheelUp();
	void WheelDown();
	void MoveMouse(SDL_MouseMotionEvent evt);

	bool IsKeyPressed(SDLKey key);
	bool WasKeyPressed(SDLKey key);
	bool IsButtonPressed(Uint8 but);
	bool WasButtonPressed(Uint8 but);
	int	 GetWheelTicks();
	MousePos GetMousePos();
	MouseDelta GetMouseDelta();

protected:
	bool		m_keydown[384];
	bool		m_waskeydown[384];
	bool		m_buttondown[256];
	bool		m_wasbuttondown[256];
	MousePos	m_mouse_pos;
	MouseDelta	m_mouse_delta;
	int			m_wheelTicks;
};

/******************************************************************************
 * reGL3App - The Main app class, that can be instantiated
 * or derived to create an application using SDL and GL3
 ******************************************************************************/
class reGL3App{
public:
	reGL3App(AppConfig conf=AppConfig());
	virtual ~reGL3App();

	/* Start method calls InitGL, InitSDL and if they succeed, it then calls Run */
	bool		Start();
	void		Quit();

	virtual void	ProcessInput	(float dt);
	virtual	void	Render			(float dt);
	virtual	void	Logic			(float dt);

protected:
	/* Override InitGL with your GL setup and projection setups etc. */
	virtual bool		InitGL();
private:
	bool				InitSDL();
	void				Run();
	void				WinProc();



public:
	AppConfig		m_config;
	SDL_GLContext	m_context;
	SDL_Window*		m_pWindow;
	Input			m_input;

	bool 			m_isRunning;

	float			m_currentFPS;
};


#endif
