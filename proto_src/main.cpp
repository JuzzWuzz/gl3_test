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


#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
#define GLEW_STATIC 1
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "sdl.lib")
#pragma comment(lib, "sdlmain.lib")
#pragma comment(lib, "sdl_image.lib")
#endif

#include "juzz_proto.h"

/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[])
{
	AppConfig conf;
	conf.VSync = true;
	conf.gl_major = 3;
	conf.gl_minor = 2;
	conf.fsaa = 0;
	//conf.winWidth = 1920;
	//conf.winHeight = 1080;
	conf.sleepTime = 0.01f;
	if (false)
	{
		conf.VSync = false;
		conf.sleepTime = 0.0f;
	}
	juzz_proto juzz(conf);
	
	if (!juzz.Start())
	{
		printf("Application failed to start\n");
		Sleep(10000);
	}
	Sleep(1000);

	return 0;
}
