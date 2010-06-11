/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	Programed in collaboration with Andrew Flower <andrew.flower@gmail.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "juzz_proto.h"

//Constructor to create the new instance
juzz_proto::juzz_proto(AppConfig& conf) : reGL3App(conf)
{
	m_shMain = NULL;
}

//Destructor
juzz_proto::~juzz_proto()
{
	glUseProgram(0);
	RE_DELETE(m_shMain);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDeleteBuffers(4, m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteTextures(1, &m_heightmap_tex);
	glDeleteTextures(1, &m_normalmap_tex);
}

//OpenGL Initialisation method to setup the OpenGL context and settings
bool juzz_proto::InitGL()
{
	int res;
	char* shVersion;
	int nVertTexUnits, nGeomTexUnits, nTexUnits, nTexUnitsCombined, nColorAttachments, nTexSize, nTexLayers, nVertAttribs, nGeomVerts, nGeomComponents;
	float aspect;

	if (!reGL3App::InitGL())
		return false;

	//If the system is windows then load GLEW to handle calls
#ifdef _WIN32
	if (glewInit()!=GLEW_OK){
		printf("Could not init GLEW\n");
		return false;
	}
#endif

	//Query some Hardware specs
	shVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &nTexUnits);
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &nVertTexUnits);
	glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &nGeomTexUnits);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nTexUnitsCombined);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nTexSize);
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &nGeomVerts);
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &nGeomComponents);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &nTexLayers);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &nColorAttachments);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nVertAttribs);
	printf("Supports GLSL version:\t\t%s\n", shVersion);
	printf("# of texture units:\t\t%d (VS), %d (GS), %d (FS), %d (comb.)\n",nVertTexUnits, nGeomTexUnits, nTexUnits, nTexUnitsCombined);
	printf("Max texture size:\t\t%d\n", nTexSize);
	printf("Max output vertices (GS):\t%d\n", nGeomVerts);
	printf("Max output components:\t\t%d\n", nGeomVerts);
	printf("Max FBO color attachments:\t%d\n", nColorAttachments);
	printf("Max array texture layers:\t%d\n", nTexLayers);
	printf("Max vertex attributes:\t\t%d\n", nVertAttribs);
	printf("-----------------------------------------\n");

	glEnable(GL_CULL_FACE);

	//Check to see that OpenGL enabled properly
	if (!CheckError("Enabling GL settings"))
		return false;

	//Setup camera projection matrix and aspect ratio
	aspect = float(m_config.winWidth) / (float)m_config.winHeight;
	m_proj_mat = perspective_proj(PI * 0.5f, aspect, 0.5f, 50.0f);

	//Init Camera
	m_cam_rotate.x = PI*.1f;

	//Init Shaders
	//Get the Shaders to Compile
	m_shMain = new ShaderProg("shaders/juzz.vert","","shaders/juzz.frag");

	//Bind attributes to shader variables. NB = must be done before linking shader
	//allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_Color");
	glBindAttribLocation(m_shMain->m_programID, 2, "in_TexCoord");

	//NB. must be done after binding attributes
	//Compiles the shaders and makes sure they are valid
	printf("Compiling shaders...\n");
	res = m_shMain->CompileAndLink();
	if (!res)
	{
		printf("Shaders compilation failed\n");
		printf("Will not continue without working shaders\n");
		return false;
	}
	printf("Shaders compiled successfully\n");

	//Assign samplers to texture units
	printf("Setting initial uniform variables\n");
	glUseProgram(m_shMain->m_programID);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "heightmap"),0);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "normalmap"),1);
	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("Done setting initial uniforms\n");

	//Start the generation and setup of geometry
	printf("Creating geometry...\n");
	if (!Init())
		return false;
	printf("Geometry created, starting program...\n");

	//Return that everything succeeded
	return true;
}

//Initialisation method to setup the program
bool juzz_proto::Init()
{
	vector3* vertices;
	vector3* colors;
	float2* texcoords;
	GLushort* indices;
	int i, j, x, z;

	//Just some scaling settings
	//1 unit = 1 metre
	const float HMAP_SIZE = 100.0f; //heightmap is 100m x 100m
	const float GRID_SIZE = 50.0f; //visible grid is 50m x 50m
	const int GRID_DIM = 100; //100x100 quads in 50m x 50m grid area => quad = .5m x .5m

	float quadSize = GRID_SIZE / GRID_DIM;
	float gridCoverage = GRID_SIZE / HMAP_SIZE; //Percentage of full map => texture coord range
	float quadCoverage = gridCoverage / GRID_DIM;
	int nVerts = GRID_DIM + 1;
	m_nIndices = GRID_DIM * GRID_DIM * 2 * 3; //Quads * 2 tris * 3 verts

	//Initialize camera start position and scale
	m_scale_metreToTex = 1.0f / HMAP_SIZE;
	m_cam_translate.x = HMAP_SIZE * 0.5f;
	m_cam_translate.z = HMAP_SIZE * 0.5f;

	//Tell shader of the tex coord increments
	glUseProgram(m_shMain->m_programID);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "texIncr"), quadCoverage);
	glUniform1f(glGetUniformLocation(m_shMain->m_programID, "quadSize"), quadSize);

	//Create the Grid
	vertices = new vector3 [nVerts * nVerts];
	colors   = new vector3 [nVerts * nVerts];
	texcoords= new float2  [nVerts * nVerts];
	indices  = new GLushort[m_nIndices]; 
	i = 0;
	j = 0;
	for (z = 0; z < nVerts; z++)
	{
		for (x = 0; x < nVerts; x++)
		{
			vertices[j].x 	= - GRID_SIZE*.5f + x * quadSize;
			vertices[j].z 	= - GRID_SIZE*.5f + z * quadSize;
			colors[j] 		= vector3(.0f, .5f, .7f);
			texcoords[j].u	= quadCoverage * x;
			texcoords[j].v	= quadCoverage * z;
			if (x > 0 && z > 0)
			{
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

	//Load heightmap
	printf("\tLoading heightmap...\n");
	if (!LoadHeightmap(&m_heightmap_tex, &m_normalmap_tex, "images/heightmaps/hmap01.pgm", "images/heightmaps/hmap01_normal.ppm"))
		return false;
	printf("\tHeightmap loaded\n");

	//Create the vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//Generate four VBOs for vertices, colors, normals, texture coordinates and indices
	glGenBuffers(4, m_vbo);

	//Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVerts * nVerts, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	//Setup the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * nVerts * nVerts, colors, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	//Setup the texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * nVerts * nVerts, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	//Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * m_nIndices, indices, GL_STATIC_DRAW);

	//Free up the memory
	delete[] vertices;
	delete[] colors;
	delete[] texcoords;
	delete[] indices;

	//Return true that everything succeeded
	return true;
}

//Method to handle user input to program
void juzz_proto::ProcessInput(float dt)
{

	MouseDelta move = m_input.GetMouseDelta();
	if (m_input.IsButtonPressed(1))
	{
		//Pitch
		m_cam_rotate.x += dt * move.y * PI * 0.1f;
		//Yaw
		m_cam_rotate.y += dt * move.x * PI * 0.1f;
	}
	
	//Toggle mouse grabbing
	if (m_input.WasKeyPressed(SDLK_m))
	{
		printf("Toggle\n");
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(0);
	}

	//Toggle wireframe
	static bool wireframe = false;
	if (m_input.WasKeyPressed(SDLK_l))
	{
		wireframe ^= true;
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	float speed = 10.0f;
	if (m_input.IsKeyPressed(SDLK_w))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, 0.0f, 1.0f, 0.0f);
		m_cam_translate += rot * vector3(0.0f, 0.0f, -speed) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_s))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, 0.0f, 1.0f, 0.0f);
		m_cam_translate += rot * vector3(0.0f, 0.0f, speed) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_a))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, 0.0f, 1.0f, 0.0f);
		m_cam_translate += rot * vector3(-speed, 0.0f, 0.0f) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_d))
	{
		matrix4 rot = rotate_tr(-m_cam_rotate.y, 0.0f, 1.0f, 0.0f);
		m_cam_translate += rot * vector3(speed, 0.0f, 0.0f) * dt;
	}

	reGL3App::ProcessInput(dt);
}

//Logic method
void juzz_proto::Logic(float dt)
{
	// Update position
	vector3 pos = m_cam_translate * m_scale_metreToTex;
	glUseProgram(m_shMain->m_programID);
	glUniform3f(glGetUniformLocation(m_shMain->m_programID, "camera_pos"), pos.x, pos.y, pos.z);
}

//Rendering method
void juzz_proto::Render(float dt)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	matrix4 modelview(m_camera_mat);
	matrix4 rotate;


	rotate = rotate_tr(m_cam_rotate.x, 1.0f, 0.0f, 0.0f) * rotate_tr(m_cam_rotate.y, 0.0f, 1.0f, 0.0f);

	glBindVertexArray(m_vao);
	glBindTexture(GL_TEXTURE_2D, m_heightmap_tex);
	// Use the shader program and setup uniform variables.
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "rotprojMatrix"), 1, GL_FALSE, (m_proj_mat*rotate).m);
	// Depending on state, draw triangle or quad patch
	glDrawElements(GL_TRIANGLES, m_nIndices, GL_UNSIGNED_SHORT, 0);

	SDL_GL_SwapWindow(m_pWindow);
}


//--------------------------------------------------------
//LOADTEXTURE loads a heightmap from a pgm file into the given texture name.  It assumes there are
//no comment lines in the file.  The texture has a byte per texel.
bool juzz_proto::LoadHeightmap(GLuint *tex, GLuint* normal_tex, string filename, string normalmap_filename)
{
	FILE* pFile;
	GLubyte *data;
	int w, h, max;
	char magicnumber[4] = { 0 };
	int i, res;

	//Load texture data
	pFile  = fopen(filename.c_str(),"r");
	
	if (!pFile)
	{
		printf("File not found, or cannot open it: %s\n", filename.c_str());
		return false;
	}
	res = fread(magicnumber, 1, 3, pFile);

	if (strcmp(magicnumber, "P2\n"))
	{
		printf("Incorrect magic number %s should be P2\n", magicnumber);
		return false;
	}
	res = fscanf(pFile,"%d %d %d", &w, &h, &max);

	data = (GLubyte *)malloc(w * h);

	for (i = 0 ; i < w * h; i++)
	{
		int tmp;
		res = fscanf(pFile,"%d", &tmp);
		data[i] = (GLubyte)tmp;
	}

	fclose(pFile);

	//Setup OpenGL texture
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
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

	//Do the same for the normal map, except with three components per texel
	pFile  = fopen(normalmap_filename.c_str(),"r");
	
	if (!pFile)
	{
		printf("File not found, or cannot open it: %s\n", normalmap_filename.c_str());
		return false;
	}
	res = fread(magicnumber, 1, 3, pFile);
	if (strcmp(magicnumber, "P3\n"))
	{
		printf("Incorrect magic number %s should be P3\n", magicnumber);
		return false;
	}
	res = fscanf(pFile,"%d %d %d", &w, &h, &max);

	data = (GLubyte *)malloc(w * h * 3); //Three components per texel

	for (i = 0 ; i < w * h * 3; i += 3)
	{
		int r, g, b;
		res = fscanf(pFile, "%d %d %d", &r, &g, &b);
		data[i] = (GLubyte)r;
		data[i+1] = (GLubyte)g;
		data[i+2] = (GLubyte)b;
	}

	fclose(pFile);

	//Setup OpenGL texture
	glGenTextures(1, normal_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, *normal_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (!CheckError("Setting normal map texture parameters"))
		return false;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	if (!CheckError("Copying data to normalmap texture"))
		return false;

	free(data);

	glActiveTexture(GL_TEXTURE0);
	return true;
}

//Checks for any errors thrown by GL and uses this string as user friendly description
bool juzz_proto::CheckError(string text)
{
	GLuint err = glGetError();

	if (err!=GL_NO_ERROR)
	{
		printf("OpenGL Error: ");
		switch(err)
		{
			case GL_INVALID_ENUM:
				printf("Invalid Enum  ");
				break;
			case GL_INVALID_VALUE:
				printf("Invalid Value  ");
				break;
			case GL_INVALID_OPERATION:
				printf("Invalid Operation  ");
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				printf("Invalid FBO operation  ");
				break;
			case GL_OUT_OF_MEMORY:
				printf("Out of Memory  ");
				break;
		}
		printf ("[%s]\n", text.c_str());
		return false;
	}
	return true;
}
