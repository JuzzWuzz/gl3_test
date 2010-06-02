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
#include "re_shader.h"
#include "main.h"

using namespace reMath;


/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[]){
	AppConfig conf;
	conf.VSync = true;
	conf.gl_major = 3;
	conf.gl_minor = 2;
	conf.fsaa=0;
	TestApp test(conf);
	
	if (!test.Start())
		printf("Application failed to start\n");

	return 0;
}


//--------------------------------------------------------
ProtoApp::ProtoApp(AppConfig& conf) : reGL3App(conf){
	m_shMain = NULL;
}

//--------------------------------------------------------
ProtoApp::~ProtoApp(){
	glUseProgram(0);
	RE_DELETE(m_shMain);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(4, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
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
	if (!Init())
		return false;
}

//--------------------------------------------------------
bool
ProtoApp::Init(){
	GLuint m_vbo[4], ibo;

	const GLfloat verts[][3]={};
	const GLfloat colors[][3]={};
	const GLubyte inds[] = { };

	// init projection matrix
	float aspect = float(m_config.winWidth)/m_config.winHeight;
	m_proj_mat = frustum_proj(-1.0f, 1.0f, -1.f/aspect, 1.f/aspect, 1.0f, 100.0f);

	// Create the vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Generate three VBOs for vertices, indices, colors
	glGenBuffers(4, m_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, verts, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, colors, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 12, inds, GL_STATIC_DRAW);

	// Get the Shaders to Compile
	m_shMain = new ShaderProg("","","");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_Color");

	// NB. must be done after binding attributes
	int res = m_shMain->CompileAndLink();
	if (!res){
		printf("Will not continue without working shaders\n");
		Quit();
	}

	// Use the shader program
	glUseProgram(m_shMain->m_programID);
	return true;
}


//--------------------------------------------------------
void
ProtoApp::ProcessInput(float dt){

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

	static float rotate = .0f;
	rotate += dt;

	matrix4 modelview;

	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "mvpMatrix"), 1, GL_FALSE,
			(m_proj_mat*modelview).m);

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, 0);

	SDL_GL_SwapWindow(m_pWindow);
}


