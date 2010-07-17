/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	Programed in collaboration with Andrew Flower <andrew.flower@gmail.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "juzz_proto.h"

//Constructor to create the new instance
juzz_proto::juzz_proto(AppConfig& conf) : reGL3App(conf)
{
	shaders = NULL;
}

//Destructor
juzz_proto::~juzz_proto(void)
{
	glUseProgram(0);
	RE_DELETE(shaders);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDeleteBuffers(VBOCOUNT, cubeVBO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteTextures(1, &colorMap);
	glDeleteTextures(1, &normalMap);
	glDeleteTextures(1, &heightMap);
}

//OpenGL Initialisation method to setup the OpenGL context and settings
bool juzz_proto::InitGL(void)
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
	cameraProjection = perspective_proj(PI * 0.5f, aspect, 0.1f, 500.0f);

	//Create a new shader manager and send it the various shaders
	shaders = new ShaderManager();
	shaders->AddShader("Shaders/JuzzPhong.vert","","Shaders/JuzzPhong.frag");
	shaders->AddShader("Shaders/JuzzNormalParallax.vert","","Shaders/JuzzNormalParallax.frag");
	shaders->AddShader("Shaders/JuzzDisp.vert","","Shaders/JuzzDisp.frag");

	//Bind attributes to shader variables. NB = must be done before linking shader
	//allows the attributes to be declared in any order in the shader.
	shaders->BindAttrib("in_Position", 0);
	shaders->BindAttrib("in_Texcoord", 1);
	shaders->BindAttrib("in_Normal", 2);
	shaders->BindAttrib("in_Tangent", 3);

	//NB. must be done after binding attributes
	//Compiles the shaders and makes sure they are valid
	printf("Compiling shaders...\n");
	res = shaders->CompileAndLink();
	if (!res)
	{
		printf("Shaders compilation failed\n");
		printf("Will not continue without working shaders\n");
		return false;
	}
	printf("Shaders compiled successfully\n");

	//Assign samplers to texture units and set some initial uniforms
	printf("Setting initial uniform variables\n");
	shaders->UpdateUni1i("colorMap", 0);
	shaders->UpdateUni1i("normalMap", 1);
	shaders->UpdateUni1i("heightMap", 2);
	shaders->UpdateUni1i("useCameraLight", 1);
	shaders->UpdateUniMat4fv("projection", cameraProjection.m);
	if (!CheckError("Creating shaders and setting initial uniforms"))
		return false;
	printf("Done setting initial uniforms\n");

	//Start the generation and setup of geometry
	printf("Setting up scene...\n");
	if (!Init())
		return false;
	printf("Scene set up, starting program...\n");

	//Return that everything succeeded
	return true;
}

//Initialisation method to setup the program
bool juzz_proto::Init(void)
{
	//Setup Camera
	useCameraLight = true;
	cameraRotation.set(-PI / 2.0f, 0.0f, 20.0f);
	cameraTarget = vector3(0.0f, 0.0f, 0.0f);

	//Update the view matrix
	UpdateViewMatrix();

	//Setup the lights
	lightRotation.set(0.0f, HPI, 20.0f);
	UpdateLight();

	//Setup the fps variables
	frames = 0;
	timerCount = 0.0f;

	//Set the world position of the cube
	cubeWorld = translate_tr(0.0f, 0.0f, 0.0f);

	//Load the textures
	int w, h, wn, hn, wh, hh;
	LoadTextureJPG(&colorMap, &w, &h, "Images/brick.jpg");
	LoadTextureJPG(&normalMap, &wn, &hn, "Images/brickNormal.jpg");
	LoadTextureJPG(&heightMap, &wh, &hh, "Images/brickHeight.jpg");
	texScale = 1.0f;
	shaders->UpdateUni1f("texScale", texScale);
	
	//Create the vertex array
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	//Generate three VBOs for vertices, indices, colors
	glGenBuffers(VBOCOUNT, cubeVBO);

	VBOData cube(50.0f, 10.0f, true);
	numOfIndices = cube.GetNumOfIndicies();

	//Setup the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, cube.GetVerticiesSize(), cube.GetVerticies(), GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Setup the texcoords buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, cube.GetTexcoordsSize(), cube.GetTexcoords(), GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	//Setup the normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, cube.GetNormalsSize(), cube.GetNormals(), GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	//Setup the tangent buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[3]);
	glBufferData(GL_ARRAY_BUFFER, cube.GetTangentsSize(), cube.GetTangents(), GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	//Setup the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVBO[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.GetIndiciesSize(), cube.GetIndicies(), GL_STATIC_DRAW);

	//Return true that everything succeeded
	return(true);
}

//Method to handle user input to program
void juzz_proto::ProcessInput(float dt)
{
	//Change in mouse position
	MouseDelta move = m_input.GetMouseDelta();

	//Variables to determine if viewing from light
	viewFromLightPrev = viewFromLight;
	viewFromLight = false;
	
	//Movement of the light about its orbit
	if (m_input.IsKeyPressed(SDLK_q) && !useCameraLight)
	{
		viewFromLight = true;

		//Mouse left button down
		if (m_input.IsButtonPressed(1))
		{
			//Horizontal movement around a circular orbit
			lightRotation.x += dt * move.x * PI * 0.1f;
			//Wrap the values to prevent them getting to large
			if (lightRotation.x < -TPI)
				lightRotation.x = TPI;
			else if (lightRotation.x > TPI)
				lightRotation.x = -TPI;

			//Vertical movement along an arc
			lightRotation.y += dt * move.y * PI * 0.1f;
			//Clamp the vertical movement to prevent inverting the light
			if (lightRotation.y < -HPI)
				lightRotation.y = -HPI;
			else if (lightRotation.y > HPI)
				lightRotation.y = HPI;
		}
		//Mouse right button down
		else if (m_input.IsButtonPressed(3))
		{
			//Ability to track inwards and outwards
			lightRotation.z += dt * move.y * 10.0f;
			if (lightRotation.z < 5.0f)
				lightRotation.z = 5.0f;
		}

		//Update the lights position
		UpdateLight();
	}
	//If not affecting the light then allow camera movement
	else
	{
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
			cameraRotation.y += dt * move.y * PI * 0.1f;
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
			//Ability to track inwards and outwards
			cameraRotation.z += dt * move.y * 10.0f;
			if (cameraRotation.z < 0.1f)
				cameraRotation.z = 0.1f;

			//Update the view matrix
			UpdateViewMatrix();
		}
	}

	//If the light was being updated and now is not then return to camera view
	if (viewFromLightPrev == true && viewFromLight == false)
		UpdateViewMatrix();

	//Toggle wireframe
	if (m_input.WasKeyPressed(SDLK_l))
	{
		wireframe = !wireframe;
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shaders->UpdateUni1i("wireframe", 1);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			shaders->UpdateUni1i("wireframe", 0);
		}
	}

	//Input to change the shaders
	if (m_input.WasKeyPressed(SDLK_1))
		shaders->SetActiveShader(0);
	else if (m_input.WasKeyPressed(SDLK_2))
		shaders->SetActiveShader(1);
	else if (m_input.WasKeyPressed(SDLK_3))
		shaders->SetActiveShader(2);

	//Switch between static scene light or a light based off the camera position
	if (m_input.WasKeyPressed(SDLK_c))
	{
		useCameraLight = !useCameraLight;
		if (useCameraLight)
		{
			UpdateViewMatrix();
			shaders->UpdateUni1i("useCameraLight", 1);
		}
		else
		{
			UpdateLight();
			shaders->UpdateUni1i("useCameraLight", 0);
		}
	}

	//Controls to adjust the scale of the texture coordinates
	if (m_input.WasKeyPressed(SDLK_PAGEUP))
	{
		texScale += 0.5f;
		texScale = min(texScale, 10.0f);
		shaders->UpdateUni1f("texScale", texScale);
	}
	if (m_input.WasKeyPressed(SDLK_PAGEDOWN))
	{
		texScale -= 0.5f;
		texScale = max(texScale, 0.5f);
		shaders->UpdateUni1f("texScale", texScale);
	}

	reGL3App::ProcessInput(dt);
}

//Logic method
void juzz_proto::Logic(float dt)
{
	timerCount += dt;
	if (timerCount >= 1.0f)
	{
		timerCount = 0.0f;
		//printf("FPS: %d\n", frames);
		frames = 0;
	}
}

//Rendering method
void juzz_proto::Render(float dt)
{
	//Increment the number of frames drawn
	frames++;

	//Clear the screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	//Bind the cube VAO
	glBindVertexArray(cubeVAO);

	//Bind the textures to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, heightMap);

	//Send through the world and world translation data to the shader
	//Calculate the normal matrix for the world
	//Draw the object
	shaders->UpdateUni3fv("worldTrans", cubeWorld.GetTranslation().v);
	shaders->UpdateUniMat4fv("world", cubeWorld.m);
	UpdateNormalMatrix(cubeWorld);
	glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_INT, 0);

	//Only draw the light if not updating its position
	if (!viewFromLight)
	{
		shaders->UpdateUni3fv("worldTrans", lightWorld.GetTranslation().v);
		shaders->UpdateUniMat4fv("world", lightWorld.m);
		UpdateNormalMatrix(lightWorld);
		glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_INT, 0);
	}

	//Check for any errors while drawing
	CheckError("Error drawing");

	SDL_GL_SwapWindow(m_pWindow);
}

//Update the lights details
void juzz_proto::UpdateLight(void)
{
	//Calculate the position of the light that follows a spherical orbit
	float radius = lightRotation.z * cos(lightRotation.y);
	float rotation = lightRotation.x;
	vector3 lightPos = vector3(cos(rotation) * radius, sin(lightRotation.y) * lightRotation.z, sin(rotation) * radius);

	//Update the lights world matrix
	lightWorld = translate_tr(lightPos) * scale_tr(0.25f);
	
	//Set the light position in the shader
	shaders->UpdateUni3fv("light_Pos", lightPos.v);

	//Set the view point to that of the light
	if (viewFromLight)
	{
		//Create the view matrix based on the caneras position, target and UP vector
		cameraView = create_look_at(lightPos, vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));
	
		//Send the view matrix to the shaders
		shaders->UpdateUniMat4fv("view", cameraView.m);

		//Set the cameras position in the shader
		shaders->UpdateUni3fv("camera_Pos", lightPos.v);
	}
}

//This function will update the view matrix
void juzz_proto::UpdateViewMatrix(void)
{
	//Calculate the position of the camera that follows a spherical orbit
	float radius = cameraRotation.z * cos(cameraRotation.y);
	float rotation = cameraRotation.x;
	vector3 cameraPos = vector3(cos(rotation) * radius, sin(cameraRotation.y) * cameraRotation.z, sin(rotation) * radius);

	//Create the view matrix based on the caneras position, target and UP vector
	cameraView = create_look_at(cameraPos, vector3(0.0f, 0.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f));
	
	//Send the view matrix to the shaders
	shaders->UpdateUniMat4fv("view", cameraView.m);

	//Set the cameras position in the shader
	shaders->UpdateUni3fv("camera_Pos", cameraPos.v);
}

//Updates the normal matrix
void juzz_proto::UpdateNormalMatrix(matrix4 world)
{
	//Calculate the modelview matrix
	matrix4 modelView;
	if (useCameraLight)
		modelView = cameraView * world;
	else
		modelView = world;

	//Retrieve the matrix3 then invert and transpose
	matrix3 normalMat = modelView.GetMatrix3().Inverse().Transpose();

	//Set the normal matrix in the shaders
	shaders->UpdateUniMat3fv("normalMatrix", normalMat.m);
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
