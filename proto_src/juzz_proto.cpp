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
	glDeleteTextures(1, &colorMap);
	glDeleteTextures(1, &normalMap);
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

	//Init Shaders
	//Get the Shaders to Compile
	m_shMain = new ShaderProg("shaders/juzz.vert","","shaders/juzz.frag");

	//Bind attributes to shader variables. NB = must be done before linking shader
	//allows the attributes to be declared in any order in the shader.
	glBindAttribLocation(m_shMain->m_programID, 0, "in_Position");
	glBindAttribLocation(m_shMain->m_programID, 1, "in_Color");
	glBindAttribLocation(m_shMain->m_programID, 2, "in_Texcoord");
	glBindAttribLocation(m_shMain->m_programID, 3, "in_Normal");
	glBindAttribLocation(m_shMain->m_programID, 4, "in_Tangent");

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
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "colorMap"), 0);
	glUniform1i(glGetUniformLocation(m_shMain->m_programID, "normalMap"), 1);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "projection"), 1, GL_FALSE, cameraProjection.m);
	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("Done setting initial uniforms\n");

	//Setup Camera
	cameraRotation.x = -PI / 2.0f;
	cameraRotation.y = 0;
	cameraRotation.z = 20.0f;
	cameraTarget = vector3(0.0f, 0.0f, 0.0f);
	//Update the view matrix
	UpdateViewMatrix();

	//Start the generation and setup of geometry
	printf("Creating geometry...\n");
	if (!Init())
		return false;
	printf("Geometry created, starting program...\n");

	//Light initial crap
	vector3 lightPos(cos(angle) * 50.0f, 50.0f, sin(angle) * 50.0f);
	glUniform3f(glGetUniformLocation(m_shMain->m_programID, "light_Pos"), lightPos.x, lightPos.y, lightPos.z);

	//Return that everything succeeded
	return true;
}

//Initialisation method to setup the program
bool juzz_proto::Init()
{
	//Initialise the angle value
	angle = PI / 2.0f;

	//The eight verticies for a cube
	vector3 verts[8];
	//Front
	//Top Left
	verts[0][0] = -10.0f;
	verts[0][1] = 10.0f;
	verts[0][2] = 10.0f;
	//Top Right
	verts[1][0] = 10.0f;
	verts[1][1] = 10.0f;
	verts[1][2] = 10.0f;
	//Bottom Right
	verts[2][0] = 10.0f;
	verts[2][1] = -10.0f;
	verts[2][2] = 10.0f;
	//Bottom Left
	verts[3][0] = -10.0f;
	verts[3][1] = -10.0f;
	verts[3][2] = 10.0f;
	//Back
	//Top Left
	verts[4][0] = -10.0f;
	verts[4][1] = 10.0f;
	verts[4][2] = -10.0f;
	//Top Right
	verts[5][0] = 10.0f;
	verts[5][1] = 10.0f;
	verts[5][2] = -10.0f;
	//Bottom Right
	verts[6][0] = 10.0f;
	verts[6][1] = -10.0f;
	verts[6][2] = -10.0f;
	//Bottom Left
	verts[7][0] = -10.0f;
	verts[7][1] = -10.0f;
	verts[7][2] = -10.0f;

	//Setup an array to store various color vectors
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
	numOfIndices = 36;
	vector3 *verticies = new vector3[numOfIndices];
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
	verticies[35] = verts[6];

	//The colours
	vector3 *colors = new vector3[numOfIndices];
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			colors[(i * 6) + j] = cols[i];
		}
	}

	//Calculate the normals for the cube
	vector3 *normals = new vector3[numOfIndices];
	CalculateNormals(verticies, numOfIndices, normals);

	vector2 *texcoords = new vector2[numOfIndices];
	CalculateTexcoords(texcoords, numOfIndices);

	vector3 *tangents = new vector3[numOfIndices];
	CalculateTangents(verticies, texcoords, numOfIndices, tangents);

	//Indicies
	GLubyte *inds = new GLubyte[numOfIndices];
	for (int i = 0; i < numOfIndices; i++)
	{
		inds[i] = i;
	}
	
	//Set the world position of the cube
	cubeWorld = translate_tr(0.0f, 0.0f, 0.0f);

	//Load the textures
	//LoadTexture(&colorMap, "images/heightmaps/hmap01.pgm");
	int w, h, wn, hn;
	glActiveTexture(GL_TEXTURE0);
	LoadTextureJPG(&colorMap, &w, &h, "images/juzz/brick.jpg");
	LoadTextureJPG(&normalMap, &wn, &hn, "images/juzz/brickNormal.jpg");
	
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
	//Setup the texcoords buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * numOfIndices, texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	//Setup the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfIndices, normals, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
	//Setup the tangent buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vector3) * numOfIndices, tangents, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);
	//Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVBO[5]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * numOfIndices, inds, GL_STATIC_DRAW);

	delete[] verticies;
	delete[] colors;
	delete[] texcoords;
	delete[] normals;
	delete[] inds;

	//Return true that everything succeeded
	return(true);
}

//Method to handle user input to program
void juzz_proto::ProcessInput(float dt)
{
	//Change in mouse position
	MouseDelta move = m_input.GetMouseDelta();
	
	//Mouse left button down
	if (m_input.IsButtonPressed(1))
	{
		//Horizontal movement around a circular orbit
		cameraRotation.x += dt * move.x * PI * 0.1f;
		//Wrap the values to prevent them getting to large
		if (cameraRotation.x < -TPI)
			cameraRotation.x = TPI;
		else if (cameraRotation.x > TPI)
			cameraRotation.x = -TPI;

		//Vertical movement along an arc
		cameraRotation.y -= dt * move.y * PI * 0.1f;
		//Clamp the vertical movement to prevent inverting the camera
		if (cameraRotation.y < -HPI)
			cameraRotation.y = -HPI;
		else if (cameraRotation.y > HPI)
			cameraRotation.y = HPI;

		//Update the view matrix
		UpdateViewMatrix();
	}
	//Mouse right button down
	else if (m_input.IsButtonPressed(3))
	{
		cameraRotation.z += dt * move.y * 10.0f;
		if (cameraRotation.z < -1.0f)
			cameraRotation.z = 0.0f;

		//Update the view matrix
		UpdateViewMatrix();
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
		angle += (PI / 8.0f) * dt;
		if (angle > 2.0f * PI)
			angle -= 2.0f * PI;

		vector3 lightPos(cos(angle) * 50.0f, 50.0f, sin(angle) * 50.0f);
		glUniform3f(glGetUniformLocation(m_shMain->m_programID, "light_Pos"), lightPos.x, lightPos.y, lightPos.z);
	}

	static bool useCameraLight = false;
	if (m_input.WasKeyPressed(SDLK_c))
	{
		useCameraLight ^= true;
		if (useCameraLight)
		{
			glUniform1f(glGetUniformLocation(m_shMain->m_programID, "useCameraLight"), 1.0f);
		}
		else
		{
			glUniform1f(glGetUniformLocation(m_shMain->m_programID, "useCameraLight"), 0.0f);
		}
	}

	reGL3App::ProcessInput(dt);
}

//Logic method
void juzz_proto::Logic(float dt)
{
	//Update camera position in shaders
	/*matrix4 modelView = cubeWorld * cameraView;
	matrix3 normalMat = modelView.GetMatrix3();
	normalMat = normalMat.Inverse().Transpose();
	glUniformMatrix3fv(glGetUniformLocation(m_shMain->m_programID, "normalMat"), 1, GL_FALSE, normalMat.m);*/

	timerCount += dt;
	if (timerCount >= 1.0f)
	{
		timerCount = 0.0f;
		printf("FPS: %d\n", frames);
		frames = 0;
	}
}

//Rendering method
void juzz_proto::Render(float dt)
{
	frames++;
	//Clear the screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	//Draw the cube
	glBindVertexArray(cubeVAO);
	glUseProgram(m_shMain->m_programID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "world"), 1, GL_FALSE, cubeWorld.m);
	glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_BYTE, 0);

	vector3 lightPos(cos(angle) * 50.0f, 50.0f, sin(angle) * 50.0f);
	matrix4 lworld = translate_tr(lightPos.x, lightPos.y, lightPos.z);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "world"), 1, GL_FALSE, lworld.m);
	glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_BYTE, 0);
	
	//Check for any errors while drawing
	CheckError("Error drawing");

	SDL_GL_SwapWindow(m_pWindow);
}

//This function will update the view matrix
void juzz_proto::UpdateViewMatrix()
{
	//Calculate the position of the camera that follows a spherical orbit
	float radius = cameraRotation.z * cos(cameraRotation.y);
	float rotation = cameraRotation.x;
	vector3 cameraPos = -(cameraTarget + vector3(cos(rotation) * radius, sin(cameraRotation.y) * cameraRotation.z, sin(rotation) * radius));

	//Create the view matrix based on the
	cameraView = create_look_at(cameraPos, cameraTarget, vector3(0.0f, 1.0f, 0.0f));
	
	glUseProgram(m_shMain->m_programID);
	glUniformMatrix4fv(glGetUniformLocation(m_shMain->m_programID, "view"), 1, GL_FALSE, cameraView.m);
	glUniform3f(glGetUniformLocation(m_shMain->m_programID, "camera_Pos"), cameraPos.x, cameraPos.y, cameraPos.z);
}

//Function to calculate the normals for each vertex based on the triangles that it is part of
void juzz_proto::CalculateNormals(vector3 *verticies, int size, vector3 *normals)
{
	for (int i = 0; i < size ; i+=6)
	{
		vector3 normal = (verticies[i + 1] - verticies[i]).Cross(verticies[i + 2] - verticies[i]);
		normal.Normalize();

		//printf("Normal: %.2f, %.2f, %.2f\n", normal.x, normal.y, normal.z);
		for (int j = 0; j < 6; j++)
		{
			normals[i + j] = normal;
		}
	}
}

//Function to generate the texcoords for the cube
void juzz_proto::CalculateTexcoords(vector2 *texcoords, int size)
{
	vector2 topL(0.0f, 0.0f);
	vector2 topR(1.0f, 0.0f);
	vector2 botL(0.0f, 1.0f);
	vector2 botR(1.0f, 1.0f);

	for (int i = 0; i < size / 6; i++)
	{
		int cur = i * 6;
		texcoords[cur++] = botR;
		texcoords[cur++] = topR;
		texcoords[cur++] = topL;
		texcoords[cur++] = topL;
		texcoords[cur++] = botL;
		texcoords[cur++] = botR;
	}
}

void juzz_proto::CalculateTangents(vector3 *verticies, vector2 *texcoords, int size, vector3 *tangents)
{
	for (int i = 0; i < size; i+=6)
	{
		vector3 edge1 = verticies[i + 1] - verticies[i];
		vector3 edge2 = verticies[i + 2] - verticies[i];

		edge1.Normalize();
		edge2.Normalize();

		vector2 texEdge1 = texcoords[i + 1] - texcoords[i];
		vector2 texEdge2 = texcoords[i + 2] - texcoords[i];

		texEdge1.Normalize();
		texEdge2.Normalize();

		float det = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x);

		vector3 tangent;
		if (det == 0.0f)
		{
			tangent.x = 1.0f;
			tangent.y = 0.0f;
			tangent.z = 0.0f;
		}
		else
		{
			det = 1.0f / det;

			float te2y = texEdge2.y;
			float te1y = texEdge1.y;
			tangent.x = (te2y * edge1.x - te1y * edge2.x) * det;
			tangent.y = (te2y * edge1.y - te1y * edge2.y) * det;
			tangent.z = (te2y * edge1.z - te1y * edge2.z) * det;

			tangent.Normalize();
		}

		//printf("Tangent: %.2f, %.2f, %.2f\n", tangent.x, tangent.y, tangent.z);
		for (int j = 0; j < 6; j++)
		{
			tangents[i + j] = tangent;
		}
	}
}

//Loads a RGB style png file (must be 3 channels - no alpha)
bool juzz_proto::LoadTexturePNG(GLuint* tex, int* width, int* height, string filename)
{
	SDL_Surface* surface;

	surface = IMG_Load(filename.c_str());
	if (surface == NULL)
	{
		fprintf(stderr, "Could not load PNG %s: %s\n", filename.c_str(), IMG_GetError());
		return false;
	}

	*width = surface->w;
	*height= surface->h;

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);//GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if (!CheckError("Loading PNG texture, setting parameters"))
		return false;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);
	return true;
}

//Loads a RGB style jpg file
bool juzz_proto::LoadTextureJPG(GLuint* tex, int* width, int* height, string filename)
{
	SDL_Surface* surface;

	surface = IMG_Load(filename.c_str());
	if (surface == NULL)
	{
		fprintf(stderr, "Could not load JPG %s: %s\n", filename.c_str(), IMG_GetError());
		return false;
	}

	*width = surface->w;
	*height= surface->h;

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);//GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if (!CheckError("Loading JPG texture, setting parameters"))
		return false;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(surface);
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
