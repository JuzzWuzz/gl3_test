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
	glDeleteTextures(1, &colorMap);
	glDeleteTextures(1, &parallaxHeightMap);
	glDeleteTextures(1, &parallaxNormalMap);
	glDeleteTextures(1, &heightMap);
	glDeleteTextures(1, &heightNormalMap);

	RE_DELETE(cube);
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
	shaders->AddShader("Shaders/JuzzSimple.vert","","Shaders/JuzzSimple.frag");
	shaders->AddShader("Shaders/JuzzPhong.vert","","Shaders/JuzzPhong.frag");
	shaders->AddShader("Shaders/JuzzNormal.vert","","Shaders/JuzzNormal.frag");
	shaders->AddShader("Shaders/JuzzParallax.vert","","Shaders/JuzzParallax.frag");
	shaders->AddShader("Shaders/JuzzDisp.vert","","Shaders/JuzzDisp.frag");
	shaders->AddShader("Shaders/JuzzDispParallax.vert","","Shaders/JuzzDispParallax.frag");
	
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
	shaders->UpdateUni1i("parallaxHeightMap", 1);
	shaders->UpdateUni1i("parallaxNormalMap", 2);
	shaders->UpdateUni1i("heightMap", 3);
	shaders->UpdateUni1i("heightNormalMap", 4);
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
	cameraRotation = vector3(0.0f, 0.0f, 0.0f);
	cameraTarget = vector3(0.0f, 0.0f, 0.0f);

	//Update the view matrix
	UpdateViewMatrix();

	//Setup the fps variables
	frames = 0;
	timerCount = 0.0f;

	//Other variable initialisation
	wireframe = false;
	shaders->UpdateUni1i("wireframe", 0);

	//Set the world position of the cube
	cubeWorld = translate_tr(0.0f, 10.0f, 0.0f);

	//Load the textures
	int w, h, wn, hn, wh, hh, whn, hhn;
	LoadTextureJPG(&colorMap, &w, &h, "Images/brick.jpg");
	LoadTextureJPG(&parallaxHeightMap, &wn, &hn, "Images/brickParallaxHeight.jpg");
	LoadTextureJPG(&parallaxNormalMap, &wn, &hn, "Images/brickParallaxNormal.jpg");
	LoadTextureJPG(&heightMap, &wh, &hh, "Images/brickHeight.jpg");
	LoadTextureJPG(&heightNormalMap, &whn, &hhn, "Images/brickHeightNormal.jpg");

	//Create a cube and set it to use the standard phong shader
	printf("Cube Object\n");
	cube = new VBOData(shaders, 50.0f, 10.0f, true);
	cube->SetShader(1);

	//Return true that everything succeeded
	return(true);
}

//Method to handle user input to program
void juzz_proto::ProcessInput(float dt)
{
	//Change in mouse position
	MouseDelta move = m_input.GetMouseDelta();
	//Number of mouse wheel ticks since last update
	int wheel_ticks = m_input.GetWheelTicks();

	static vector2 clickPos;
	static bool clicked = false;

	//Controls to determine if the camera has been updated
	vector3 cameraRorationPrev = cameraRotation;
	vector3 cameraTranslationPrev = cameraTranslation;

	//Left click and hold to move mouse around
	if (m_input.IsButtonPressed(1))
	{
		//Pitch
		cameraRotation.x += move.y * 0.005f;
		//Yaw
		cameraRotation.y += move.x * 0.005f;

		//Clamp the camera to prevent the user flipping
		//upside down messing up everything
		if (cameraRotation.x < -HPI)
			cameraRotation.x = -HPI;
		if (cameraRotation.x > HPI)
			cameraRotation.x = HPI;
	}

	//Right mouse click for selecting point to deform
	if (m_input.IsButtonPressed(3))
	{
		MousePos pos = m_input.GetMousePos();
		float val;
		float w = m_config.winWidth;
		float h = m_config.winHeight;
		float aspect = float(m_config.winWidth) / m_config.winHeight;

		//Get value in z-buffer
		glReadPixels(pos.x, m_config.winHeight- pos.y, 1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &val);

		vector3 frag(pos.x,pos.y,val);
		//Derive inverse of view transform (could just use transpose of view matrix
		matrix4 inverse = rotate_tr(-cameraRotation.y, .0f, 1.0f, .0f) * rotate_tr(-cameraRotation.x, 1.0f, .0f, .0f);

		//Request unprojected coordinate
		vector3 p = perspective_unproj_world(frag, w, h, NEAR_PLANE, FAR_PLANE, 1.0f, inverse);
		p += cameraTranslation;
		//p *= m_scale_metreToTex;
		vector2 tc = vector2(p.x+.5f, p.z+.5f);

		clicked = true;
		clickPos = tc;

		//Pass to shader
		shaders->UpdateUni2fv("click_pos", tc.v);
	}

	//Speed in m/s for movement
	float speed = 5.0f;
	//If super speed activated then increase speed
	if (m_input.IsKeyPressed(SDLK_LSHIFT))
		speed *= 10.0f;

	//Controls for movement
	//Calculate rotation based on yaw for left/right movement
	matrix4 rot = rotate_tr(-cameraRotation.y, .0f, 1.0f, .0f);
	//Moving left
	if (m_input.IsKeyPressed(SDLK_a))
		cameraTranslation += rot * vector3(-speed, .0f, .0f) * dt;
	//Moving right
	if (m_input.IsKeyPressed(SDLK_d))
		cameraTranslation += rot * vector3(speed, .0f, .0f) * dt;

	//Factor in the pitch to the rotation for forwad/backward movement
	rot *= rotate_tr(-cameraRotation.x, 1.0f, .0f, .0f);
	//Moving forward 
	if (m_input.IsKeyPressed(SDLK_w))
		cameraTranslation += rot * vector3(.0f, .0f, -speed) * dt;
	//Moving backward
	if (m_input.IsKeyPressed(SDLK_s))
		cameraTranslation += rot * vector3(.0f, .0f, speed) * dt;

	//Moving upwards
	if (m_input.IsKeyPressed(SDLK_e))
		cameraTranslation += vector3(.0f, speed, .0f) * dt;
	//Moving downwards
	if (m_input.IsKeyPressed(SDLK_q))
		cameraTranslation += vector3(.0f, -speed, .0f) * dt;

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

	//Update the view matrix details only if the camera has changed
	if (cameraTranslation != cameraTranslationPrev ||
		cameraRotation != cameraRorationPrev)
		UpdateViewMatrix();

	//Input to change the shaders
	if (m_input.WasKeyPressed(SDLK_0))
		cube->SetShader(0);
	else if (m_input.WasKeyPressed(SDLK_1))
		cube->SetShader(1);
	else if (m_input.WasKeyPressed(SDLK_2))
		cube->SetShader(2);
	else if (m_input.WasKeyPressed(SDLK_3))
		cube->SetShader(3);
	else if (m_input.WasKeyPressed(SDLK_4))
		cube->SetShader(4);
	else if (m_input.WasKeyPressed(SDLK_5))
		cube->SetShader(5);

	reGL3App::ProcessInput(dt);
}

//Logic method
void juzz_proto::Logic(float dt)
{
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
	//Increment the number of frames drawn
	frames++;

	//Clear the screen
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	//Bind the textures to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, parallaxHeightMap);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, parallaxNormalMap);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, heightMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, heightNormalMap);

	//Send through the world and world translation data to the shader
	//Calculate the normal matrix for the world
	//Draw the object
	shaders->UpdateUni3fv("worldTrans", cubeWorld.GetTranslation().v);
	shaders->UpdateUniMat4fv("world", cubeWorld.m);
	UpdateNormalMatrix(cubeWorld);
	cube->DrawObject();

	//Check for any errors while drawing
	CheckError("Error drawing");

	SDL_GL_SwapWindow(m_pWindow);
}

//This function will update the view matrix
void juzz_proto::UpdateViewMatrix(void)
{
	//Create the view matrix based on the cameras rotation and translation
	cameraView = rotate_tr(cameraRotation.x, 1.0f, .0f, .0f) * rotate_tr(cameraRotation.y, .0f, 1.0f, .0f) * translate_tr(-cameraTranslation);

	//Set the cameras position in the shader
	shaders->UpdateUni3fv("camera_Pos", cameraTranslation.v);

	//Send the view matrix to the shaders
	shaders->UpdateUniMat4fv("view", cameraView.m);
}

//Updates the normal matrix
void juzz_proto::UpdateNormalMatrix(matrix4 world)
{
	//Calculate the modelview matrix
	matrix4 modelView;
	modelView = cameraView * world;

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
