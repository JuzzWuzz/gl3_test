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

bool CheckError(string);


/******************************************************************************
 * Main 
 ******************************************************************************/
int main(int argc, char* argv[]){
	AppConfig conf;
	conf.VSync = true;
	conf.gl_major = 3;
	conf.gl_minor = 2;
	conf.fsaa=4;
	conf.sleepTime = .01f;
	ProtoApp test(conf);
	
	if (!test.Start())
		printf("Application failed to start\n");

	return 0;
}


//--------------------------------------------------------
ProtoApp::ProtoApp(AppConfig& conf) : reGL3App(conf){
	m_shMain = NULL;
	m_levels = 0;
	m_technique = TECH_EQUI;
	m_rise = .75f;
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
	glDeleteTextures(1, &m_heightmap_tex);
}

//--------------------------------------------------------
bool
ProtoApp::InitGL(){
	CheckError("Before all");
	if (!reGL3App::InitGL())
		return false;
#ifdef _WIN32
	if (glewInit()!=GLEW_OK){
		printf("Could not init GLEW\n");
		return false;
	}
#endif

	glEnable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_2D);

	if (!CheckError("Enabling GL settings"))
		return false;

	// init projection matrix
	float aspect = float(m_config.winWidth)/m_config.winHeight;
	m_proj_mat = perspective_proj(PI*.5f, aspect, .5f, 50.0f);

	// Init Camera
	m_cam_translate.z = -5.0f;
	m_cam_rotate.x = PI*.1f;
	m_camera_mat = translate_tr(m_cam_translate.x, m_cam_translate.y, m_cam_translate.z)
				*	rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f)
				*	rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);

	// Init Shaders
	// Get the Shaders to Compile
	m_shMain = new ShaderProg("shaders/phongtess.vert","shaders/phongtess.geom","shaders/phongtess.frag");

	// Bind attributes to shader variables. NB = must be done before linking shader
	// allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_Color");
	glBindAttribLocation(m_shMain->m_programID, 2, "in_TexCoord");

	// NB. must be done after binding attributes
	printf("compiling shaders...\n");
	int res = m_shMain->CompileAndLink();
	if (!res){
		printf("Will not continue without working shaders\n");
		return false;
	}
	glUseProgram(m_shMain->m_programID);
	
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "heightmap"),0);
	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
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
	vector3* vertices;
	vector3* colors;
	float2*	 texcoords;
	GLushort* indices;
	int i,j,x,z;

	// Just some scaling settings
	// 1 unit = 1 metre
	const float HMAP_SIZE = 100.0f;		// heightmap is 100m x 100m
	const float GRID_SIZE = 50.0f;		// visible grid is 50m x 50m
	const int	GRID_DIM  = 100;		// 100x100 quads in 50m x 50m grid area => quad = .5m x .5m

	float quadSize	   = GRID_SIZE/GRID_DIM;
	float gridCoverage = GRID_SIZE/HMAP_SIZE;	// percentage of full map => texture coord range
	float quadCoverage = gridCoverage/GRID_DIM;
	int nVerts = GRID_DIM + 1;
	m_nIndices = GRID_DIM*GRID_DIM * 2 * 3; 	// quads * 2 tris * 3 verts

	// Tell shader of the tex coord increments
	glUseProgram(m_shMain->m_programID);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "texIncr"), quadCoverage);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "quadSize"), quadSize);

	// Create the Grid
	vertices = new vector3 [nVerts * nVerts];
	colors   = new vector3 [nVerts * nVerts];
	texcoords= new float2  [nVerts * nVerts];
	indices  = new GLushort[m_nIndices]; 
	i = 0;
	j = 0;
	for (z = 0; z < nVerts; z++){
		for (x = 0; x < nVerts; x++){
			vertices[j].x 	= - GRID_SIZE*.5f + x * quadSize;
			vertices[j].z 	= - GRID_SIZE*.5f + z * quadSize;
			colors[j] 		= vector3(.0f, .5f, .7f);
			texcoords[j].u	= quadCoverage * x;
			texcoords[j].v	= quadCoverage * z;
			if (x > 0 && z > 0){
				indices[i++] = j-1;
				indices[i++] = j-nVerts;
				indices[i++] = j-nVerts-1;

				indices[i++] = j-nVerts;
				indices[i++] = j-1;
				indices[i++] = j;
			}
			j++;
		}
	}


	// Load heightmap
	printf("\tloading heightmap...\n");
	if (!LoadTexture(&m_heightmap_tex, "images/heightmaps/hmap02.pgm"))
		return false;
	printf("\tdone\n");

	// Create the vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Generate four VBOs for vertices, colors, normals, texture coordinates and indices
	glGenBuffers(4, m_vbo);

	// Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVerts*nVerts, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// Setup the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVerts*nVerts, colors, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * nVerts*nVerts, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	// Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * m_nIndices, indices, GL_STATIC_DRAW);

	delete[] vertices;
	delete[] colors;
	delete[] texcoords;
	delete[] indices;

	return true;
}

//--------------------------------------------------------
// LOADTEXTURE loads a heightmap from a pgm file into the given texture name.  It assumes there are
// no comment lines in the file.  The texture has a byte per texel.
bool
ProtoApp::LoadTexture(GLuint *tex, string filename){
	FILE* pFile;
	GLubyte *data;
	int w, h, max;
	char magicnumber[4]={0};
	int i;

	// Load heightmap data
	pFile  = fopen(filename.c_str(),"r");
	
	if (!pFile){
		fprintf(stderr, "File not found, or cannot open it: %s\n", filename.c_str());
		return false;
	}
	fread(magicnumber, 1, 3, pFile);
	if (strcmp(magicnumber, "P2\n")){
		fprintf(stderr, "Incorrect magic number %s should be P2\n", magicnumber);
		return false;
	}
	fscanf(pFile,"%d %d %d", &w, &h, &max);

	data = (GLubyte*)malloc(w*h);

	for (i = 0 ; i < w*h; i++){
		int tmp;
		fscanf(pFile,"%d", &tmp);
		data[i] = (GLubyte)tmp;
	}

	fclose(pFile);

	// Setup OpenGL texture
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Setting texture parameters"))
		return false;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	
	if (!CheckError("Copying data to heightmap texture"))
		return false;
	free(data);
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
	matrix4 rotate;


	rotate = rotate_tr(m_cam_rotate.x, 1.0f, .0f, .0f) * rotate_tr(m_cam_rotate.y, .0f, 1.0f, .0f);

	glBindVertexArray(m_vao);
	glBindTexture(GL_TEXTURE_2D, m_heightmap_tex);
	// Use the shader program and setup uniform variables.
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "rotprojMatrix"), 1, GL_FALSE,
			(m_proj_mat*rotate).m);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "degree"), m_levels);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "technique"), m_technique);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "rise"), m_rise);
	// Depending on state, draw triangle or quad patch
	glDrawElements(GL_TRIANGLES, m_nIndices, GL_UNSIGNED_SHORT, 0);

	SDL_GL_SwapWindow(m_pWindow);
}

//--------------------------------------------------------
bool CheckError(string text){
	GLuint err = glGetError();

	if (err!=GL_NO_ERROR){
		fprintf(stderr, "OpenGL Error: ");
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
		return false;
	}
	return true;
}
