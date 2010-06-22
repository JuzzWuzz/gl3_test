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
	glDeleteBuffers(VBOCOUNT, cubeVBO);
	glDeleteVertexArrays(1, &cubeVAO);
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
	cameraProjection = perspective_proj(PI * 0.5f, aspect, 0.1f, 100.0f);

	//Init Camera
	cameraRotation.x = 0;
	cameraRotation.y = 0;
	cameraRotation.z = 0;

	//Init Shaders
	//Get the Shaders to Compile
	m_shMain = new ShaderProg("shaders/juzz.vert","","shaders/juzz.frag");

	//Bind attributes to shader variables. NB = must be done before linking shader
	//allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_Color");
	glBindAttribLocation(m_shMain->m_programID, 2, "in_Normal");

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
	//glUniform1i(glGetUniformLocation(m_shMain->m_programID, "heightmap"), 0);
	//glUniform1i(glGetUniformLocation(m_shMain->m_programID, "normalmap"), 1);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "projection"), 1, GL_FALSE, cameraProjection.m);
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
	angle = 0.0f;

	//The eight verticies for a cube
	vector3 verts[8];
	//Front
	//Top Left
	verts[0][0] = -10.0f;
	verts[0][1] = 1.0f;
	verts[0][2] = 10.0f;
	//Top Right
	verts[1][0] = 10.0f;
	verts[1][1] = 1.0f;
	verts[1][2] = 10.0f;
	//Bottom Right
	verts[2][0] = 1.0f;
	verts[2][1] = -1.0f;
	verts[2][2] = 1.0f;
	//Bottom Left
	verts[3][0] = -1.0f;
	verts[3][1] = -1.0f;
	verts[3][2] = 1.0f;
	//Back
	//Top Left
	verts[4][0] = -10.0f;
	verts[4][1] = 1.0f;
	verts[4][2] = -10.0f;
	//Top Right
	verts[5][0] = 10.0f;
	verts[5][1] = 1.0f;
	verts[5][2] = -10.0f;
	//Bottom Right
	verts[6][0] = 1.0f;
	verts[6][1] = -1.0f;
	verts[6][2] = -1.0f;
	//Bottom Left
	verts[7][0] = -1.0f;
	verts[7][1] = -1.0f;
	verts[7][2] = -1.0f;

	vector3 cols[6];
	//Red
	cols[0][0] = 1.0f;
	cols[0][1] = 0.0f;
	cols[0][2] = 0.0f;
	//Green
	cols[1][0] = 0.0f;
	cols[1][1] = 1.0f;
	cols[1][2] = 0.0f;
	//Blue
	cols[2][0] = 0.0f;
	cols[2][1] = 0.0f;
	cols[2][2] = 1.0f;
	//Yellow
	cols[3][0] = 1.0f;
	cols[3][1] = 1.0f;
	cols[3][2] = 0.0f;
	//Turquoise
	cols[4][0] = 0.0f;
	cols[4][1] = 1.0f;
	cols[4][2] = 1.0f;
	//Pink
	cols[5][0] = 1.0f;
	cols[5][1] = 0.0f;
	cols[5][2] = 1.0f;

	//Constructs the cube based on the 8 verticies, 6 quads with 2 tris / quad
	/*verticies = new vector3[36];
	//Front face
	verticies[0] = verts[2];
	verticies[1] = verts[1];
	verticies[2] = verts[0];
	verticies[3] = verts[0];
	verticies[4] = verts[3];
	verticies[5] = verts[2];

	//Right face
	verticies[6] = verts[6];
	verticies[7] = verts[5];
	verticies[8] = verts[1];
	verticies[9] = verts[1];
	verticies[10] = verts[2];
	verticies[11] = verts[6];

	//Back face
	verticies[12] = verts[7];
	verticies[13] = verts[4];
	verticies[14] = verts[5];
	verticies[15] = verts[5];
	verticies[16] = verts[6];
	verticies[17] = verts[7];

	//Left face
	verticies[18] = verts[3];
	verticies[19] = verts[0];
	verticies[20] = verts[4];
	verticies[21] = verts[4];
	verticies[22] = verts[7];
	verticies[23] = verts[3];

	//Top face
	verticies[24] = verts[1];
	verticies[25] = verts[5];
	verticies[26] = verts[4];
	verticies[27] = verts[4];
	verticies[28] = verts[0];
	verticies[29] = verts[1];

	//Bottom face
	verticies[30] = verts[6];
	verticies[31] = verts[2];
	verticies[32] = verts[3];
	verticies[33] = verts[3];
	verticies[34] = verts[7];
	verticies[35] = verts[6];*/

	vector3 verticies[36] =
	{
		//Front face
		verts[2],
		verts[1],
		verts[0],
		verts[0],
		verts[3],
		verts[2],

		//Right face
		verts[6],
		verts[5],
		verts[1],
		verts[1],
		verts[2],
		verts[6],

		//Back face
		verts[7],
		verts[4],
		verts[5],
		verts[5],
		verts[6],
		verts[7],

		//Left face
		verts[3],
		verts[0],
		verts[4],
		verts[4],
		verts[7],
		verts[3],

		//Top face
		verts[1],
		verts[5],
		verts[4],
		verts[4],
		verts[0],
		verts[1],

		//Bottom face
		verts[6],
		verts[2],
		verts[3],
		verts[3],
		verts[7],
		verts[6]
	};

	//The colours
	vector3 colors[36] =
	{
		//Red
		cols[0],
		cols[0],
		cols[0],
		cols[0],
		cols[0],
		cols[0],

		//Green
		cols[1],
		cols[1],
		cols[1],
		cols[1],
		cols[1],
		cols[1],

		//Blue
		cols[2],
		cols[2],
		cols[2],
		cols[2],
		cols[2],
		cols[2],

		//Yellow
		cols[3],
		cols[3],
		cols[3],
		cols[3],
		cols[3],
		cols[3],

		//Turquoise
		cols[4],
		cols[4],
		cols[4],
		cols[4],
		cols[4],
		cols[4],

		//Pink
		cols[5],
		cols[5],
		cols[5],
		cols[5],
		cols[5],
		cols[5],
	};

	//Calculate the normals for the cube
	//normals = new vector3[36];
	//CalculateNormals(verticies, 36, normals);
	vector3 normals[36];
	for (int i = 0; i < 36 / 3; i++)
	{
		int a = i * 3;
		int b = a + 1;
		int c = b + 1;
		normals[a] = (verticies[b] - verticies[a]).Cross(verticies[c] - verticies[a]);
		normals[b] = (verticies[c] - verticies[b]).Cross(verticies[a] - verticies[b]);
		normals[c] = (verticies[a] - verticies[c]).Cross(verticies[b] - verticies[c]);
		normals[a].Normalize();
		normals[b].Normalize();
		normals[c].Normalize();
	}

	//Indicies
	numOfIndices = 36;
	GLubyte *inds = new GLubyte[numOfIndices];
	for (int i = 0; i < numOfIndices; i++)
	{
		inds[i] = i;
	}
	
	//Create the vertex array
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	//Generate three VBOs for vertices, indices, colors
	glGenBuffers(VBOCOUNT, cubeVBO);

	//Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfIndices, verticies, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	//Setup the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfIndices, colors, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	//Setup the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfIndices, normals, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	//Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVBO[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * numOfIndices, inds, GL_STATIC_DRAW);

	cubeWorld = translate_tr(0.0f, 0.0f, 0.0f);
	
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
		cameraRotation.x += dt * move.y * PI * 0.1f;
		//Yaw
		cameraRotation.y += dt * move.x * PI * 0.1f;
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

	if (m_input.IsKeyPressed(SDLK_r))
	{
		runn = !runn;
	}

	float speed = 10.0f;
	if (m_input.IsKeyPressed(SDLK_w))
	{
		matrix4 rot = rotate_tr(-cameraRotation.y, 0.0f, 1.0f, 0.0f);
		cameraTranslation += rot * vector3(0.0f, 0.0f, speed) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_s))
	{
		matrix4 rot = rotate_tr(-cameraRotation.y, 0.0f, 1.0f, 0.0f);
		cameraTranslation += rot * vector3(0.0f, 0.0f, -speed) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_a))
	{
		matrix4 rot = rotate_tr(-cameraRotation.y, 0.0f, 1.0f, 0.0f);
		cameraTranslation += rot * vector3(speed, 0.0f, 0.0f) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_d))
	{
		matrix4 rot = rotate_tr(-cameraRotation.y, 0.0f, 1.0f, 0.0f);
		cameraTranslation += rot * vector3(-speed, 0.0f, 0.0f) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_u))
	{
		matrix4 rot = rotate_tr(-cameraRotation.y, 0.0f, 1.0f, 0.0f);
		cameraTranslation += rot * vector3(0.0f, -speed, 0.0f) * dt;
	}
	if (m_input.IsKeyPressed(SDLK_j))
	{
		matrix4 rot = rotate_tr(-cameraRotation.y, 0.0f, 1.0f, 0.0f);
		cameraTranslation += rot * vector3(0.0f, speed, 0.0f) * dt;
	}

	reGL3App::ProcessInput(dt);
}

//Logic method
void juzz_proto::Logic(float dt)
{
	//Update the cameras view matrix
	matrix4 camRot = rotate_tr(cameraRotation.x, 1.0f, 0.0f, 0.0f) * rotate_tr(cameraRotation.y, 0.0f, 1.0f, 0.0f);
	matrix4 camTrans = translate_tr(cameraTranslation.x, cameraTranslation.y, cameraTranslation.z);
	cameraView = camRot * camTrans;

	//Update camera position in shaders
	vector3 pos = cameraTranslation;
	glUseProgram(m_shMain->m_programID);
	glUniform3f(glGetUniformLocation(m_shMain->m_programID, "camera_pos"), pos.x, pos.y, pos.z);
	printf("Camera Pos: (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "view"), 1, GL_FALSE, cameraView.m);

	if (!runn)
	{
		angle += (PI / 8.0f) * dt;
		if (angle > 2.0f * PI)
			angle -= 2.0f * PI;
	}

	vector3 lightPos(cos(angle) * 10.0f, 10.0f, sin(angle) * 10.0f);
	glUniform3f(glGetUniformLocation(m_shMain->m_programID, "light_Pos"), lightPos.x, lightPos.y, lightPos.z);
}

//Rendering method
void juzz_proto::Render(float dt)
{
	//Clear the screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Draw the cube
	glBindVertexArray(cubeVAO);
	glUseProgram(m_shMain->m_programID);
	//glBindTexture(GL_TEXTURE_2D, m_heightmap_tex);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "world"), 1, GL_FALSE, cubeWorld.m);
	glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_BYTE, 0);

	vector3 lightPos(cos(angle) * 10.0f, 10.0f, sin(angle) * 10.0f);
	matrix4 lworld = translate_tr(lightPos.x, lightPos.y, lightPos.z);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "world"), 1, GL_FALSE, lworld.m);
	glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_BYTE, 0);
	
	//Check for any errors while drawing
	CheckError("Error drawing");

	SDL_GL_SwapWindow(m_pWindow);
}


//Function to calculate the normals for each vertex based on the triangles that it is part of
void juzz_proto::CalculateNormals(vector3 *verticies, int size, vector3 *normals)
{

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
