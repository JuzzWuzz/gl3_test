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
//#	pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#	pragma comment(linker, "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
#	define GLEW_STATIC 1
#	pragma comment(lib, "opengl32.lib")
#	pragma comment(lib, "sdl.lib")
#	pragma comment(lib, "sdlmain.lib")
#endif

#include "regl3.h"
#include "re_math.h"
using namespace reMath;
#include "re_shader.h"
#include "main.h"


#define PI					(3.14159265358f)
#define TECH_BISECT			(0)
#define TECH_EQUI			(1)
#define TECH_CENTROID		(2)
#define TECH_EQUI_BISECT	(3)

void CheckError(string);


/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[]){
	AppConfig conf;
	conf.VSync = true;
	conf.gl_major = 3;
	conf.gl_minor = 2;
	conf.fsaa=4;
	ProtoApp test(conf);
	
	if (!test.Start())
		printf("Application failed to start\n");

	return 0;
}


//--------------------------------------------------------
ProtoApp::ProtoApp(AppConfig& conf) : reGL3App(conf){
	m_shMain = NULL;
	m_shDrawNormals = NULL;
	m_levels = 0;
	m_technique = TECH_EQUI;
	m_rise = .75f;
	m_geom = 0;
}

//--------------------------------------------------------
ProtoApp::~ProtoApp(){
	glUseProgram(0);
	RE_DELETE(m_shMain);
	RE_DELETE(m_shDrawNormals);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(8, m_vbo);
	glDeleteVertexArrays(2, m_vao);
}

//--------------------------------------------------------
bool
ProtoApp::InitGL(){
	if (!reGL3App::InitGL())
		return false;
#ifdef _WIN32
	if (glewInit()!=GLEW_OK){
		printf("Could not init GLEW\n");
		return false;
	}
#endif

	glEnable(GL_CULL_FACE);

	// init projection matrix
	float aspect = float(m_config.winWidth)/m_config.winHeight;
	m_proj_mat = perspective_proj(PI*.5f, aspect, .1f, 20.0f);

	// Init Camera
	m_cam_translate.z = -5.0f;
	m_cam_rotate.x = PI*.1f;
	m_camera_mat = translate_tr(m_cam_translate.x, m_cam_translate.y, m_cam_translate.z)
				*	rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f)
				*	rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);

	// Init Shaders
	// Get the Shaders to Compile
	m_shMain = new ShaderProg("shaders/phongtess.vert","shaders/phongtess.geom","shaders/phongtess.frag");
	m_shDrawNormals = new ShaderProg("shaders/phongtess.vert","shaders/normals.geom","shaders/phongtess.frag");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_Color");
	glBindAttribLocation(m_shMain->m_programID, 2, "in_Normal");
	glBindAttribLocation(m_shDrawNormals->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shDrawNormals->m_programID, 1, "in_Color");
	glBindAttribLocation(m_shDrawNormals->m_programID, 2, "in_Normal");

	// NB. must be done after binding attributes
	printf("compiling shaders...\n");
	int res = m_shMain->CompileAndLink() && m_shDrawNormals->CompileAndLink();
	if (!res){
		printf("Will not continue without working shaders\n");
		return false;
	}
	printf("done\n");

	printf("creating geometry...\n");
	if (!Init())
		return false;
	printf("done\n");
	return true;
}

//--------------------------------------------------------
bool
ProtoApp::Init(){
	const GLfloat verts[3][3]={
		{.5f, .0f, .433f},
		{.0f, .0f, -.433f},
		{-.5f,.0f, .433f}
	};
	const GLfloat colors[3][3]={
		{1.0f, .0f, .0f},
		{.0f, 1.0f, .0f},
		{.0f, .0f, 1.0f}
	};
	const GLfloat normals[3][3]={
		{1.0f, 1.0f, 1.0f},
		{.0f, 1.0f, -1.0f},
		{-1.0f, 1.0f, 1.0f}
	};
	const GLubyte inds[3] = {0,1,2};


	// Create the vertex array
	glGenVertexArrays(1, m_vao);
	glBindVertexArray(m_vao[0]);

	// Generate three VBOs for vertices, indices, colors
	glGenBuffers(4, m_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 3, verts, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 3, colors, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 3, normals, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 3, inds, GL_STATIC_DRAW);

	//
	// THE QUAD PATCH

	const GLfloat verts2[4][3]={
		{.5f, .0f, .5f},
		{.5f, .0f, -.5f},
		{-.5f,.0f, -.5f},
		{-.5f,.0f,  .5f}
	};
	const GLfloat colors2[4][3]={
		{1.0f, .0f, .0f},
		{.0f, 1.0f, .0f},
		{.0f, 1.0f, 1.0f},
		{.0f, .0f, 1.0f}
	};
	const GLfloat normals2[4][3]={
		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, -1.0f},
		{-1.0f, 1.0f, -1.0f},
		{-1.0f, 1.0f, 1.0f}
	};
	const GLubyte inds2[6] = {1,3,0,  3,1,2};


	// Create the vertex array
	glGenVertexArrays(1, m_vao+1);
	glBindVertexArray(m_vao[1]);

	// Generate three VBOs for vertices, indices, colors
	glGenBuffers(4, m_vbo+4);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, verts2, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, colors2, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, normals2, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[7]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, inds2, GL_STATIC_DRAW);

	return true;
}


//--------------------------------------------------------
void
ProtoApp::ProcessInput(float dt){

	MouseDelta move = m_input.GetMouseDelta();
	int zoomTicks = m_input.GetWheelTicks();
	if (m_input.IsButtonPressed(1)){
		//pitch
		m_cam_rotate.x += dt*move.y*PI*.1f;
		//yaw
		m_cam_rotate.y += dt*move.x*PI*.1f;

		m_camera_mat = translate_tr(m_cam_translate.x, m_cam_translate.y, m_cam_translate.z)
					*	rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f)
					*	rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);
	}
	
	if (zoomTicks){
		m_cam_translate.z += zoomTicks * dt * 20.0f;
		m_camera_mat = translate_tr(m_cam_translate.x, m_cam_translate.y, m_cam_translate.z)
					*	rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f)
					*	rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);
	}

	// Toggle mouse grabbinga
	if (m_input.WasKeyPressed(SDLK_m)){
		printf("Toggle\n");
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(0);
	}
	static bool wireframe = false;
	// Toggle wireframe
	if (m_input.WasKeyPressed(SDLK_l)){
		wireframe^=true;
		if (wireframe){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	// levels of tessellation
	if (m_input.WasKeyPressed(SDLK_0))
		m_levels = 0;
	if (m_input.WasKeyPressed(SDLK_1))
		m_levels = 1;
	if (m_input.WasKeyPressed(SDLK_2))
		m_levels = 2;
	if (m_input.WasKeyPressed(SDLK_3))
		m_levels = 3;
	if (m_input.WasKeyPressed(SDLK_4))
		m_levels = 4;
	if (m_input.WasKeyPressed(SDLK_5))
		m_levels = 5;
	// height of tessellation
	if (m_input.WasKeyPressed(SDLK_EQUALS))
		m_rise+=.05f;
	if (m_input.WasKeyPressed(SDLK_MINUS))
		m_rise-=.05f;
	// tessellation technique
	if (m_input.WasKeyPressed(SDLK_b))
		m_technique = TECH_BISECT;
	else if (m_input.WasKeyPressed(SDLK_e))
		m_technique = TECH_EQUI;
	else if (m_input.WasKeyPressed(SDLK_c))
		m_technique = TECH_CENTROID;
	else if (m_input.WasKeyPressed(SDLK_t))
		m_technique = TECH_EQUI_BISECT;

	// Switch to tri/quad patch
	if (m_input.WasKeyPressed(SDLK_SPACE))
		m_geom ^= 1;

	reGL3App::ProcessInput(dt);
}

//--------------------------------------------------------
void 
ProtoApp::Logic(float dt){
}

//--------------------------------------------------------
void
ProtoApp::Render(float dt){
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	matrix4 modelview(m_camera_mat);

	modelview *= scale_tr(2.0f, 2.0f, 2.0f);


	glBindVertexArray(m_vao[m_geom]);
	// Draw the normals
	glUseProgram(m_shDrawNormals->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shDrawNormals->m_programID, "mvpMatrix"), 1, GL_FALSE,
			(m_proj_mat*modelview).m);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, 0);
	// Use the shader program
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "mvpMatrix"), 1, GL_FALSE,
			(m_proj_mat*modelview).m);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "degree"), m_levels);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "technique"), m_technique);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "rise"), m_rise);
	if (m_geom)
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	else
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, 0);

	SDL_GL_SwapWindow(m_pWindow);
}

//--------------------------------------------------------
void CheckError(string text){
	GLuint err = glGetError();

	if (err!=GL_NO_ERROR){
		printf("OpenGL Error: ");
		switch(err){
			case GL_INVALID_ENUM:
				fprintf(stderr, "Invalid Enum  ");
				break;
			case GL_INVALID_VALUE:
				fprintf(stderr, "Invalid Value  ");
				break;
			case GL_INVALID_OPERATION:
				fprintf(stderr, "Invalid Operation  ");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				fprintf(stderr, "Invalid FBO operation  ");
				break;
			case GL_OUT_OF_MEMORY:
				fprintf(stderr, "Out of Memory  ");
				break;
		}
		printf ("[%s]\n", text.c_str());
	}
}
