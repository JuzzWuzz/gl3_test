
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cmath>

#include "SDL/SDL.h"

//////////////////////
/*	 	TYPES		*/
//////////////////////
typedef unsigned int u_int;

//////////////////////
/* 		STRUCTS		*/
//////////////////////

/*
 * AppConfig
 * Contains settings to pass to AppMain for SDL, OpenGL setup.
 */
struct AppConfig{
	AppConfig(){
		fullscreen 	= false;
		VSync		= false;
		winWidth 	= 800;
		winHeight	= 600;
	}
	bool 	fullscreen;
	bool 	VSync;
	u_int 	winWidth;
	u_int 	winHeight;
	float	loopSleep;
};


#endif
