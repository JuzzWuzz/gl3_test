/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	Programed in collaboration with Andrew Flower <andrew.flower@gmail.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "JuzzUtility.h"

using namespace reMath;

#ifndef JUZZ_PROTO
#define JUZZ_PROTO

#define FAR_PLANE			(1000.0f)
#define NEAR_PLANE			(.5f)

class juzz_proto : public reGL3App
{
//Methods
public:
	juzz_proto(AppConfig& conf);
	~juzz_proto(void);

	//Overridden methods
	void ProcessInput(float dt);
	void Logic(float dt);
	void Render(float dt);

private:
	bool Init(void);
	bool InitGL(void);

	void UpdateViewMatrix(void);
	void UpdateNormalMatrix(matrix4 world); 
	bool CheckError(string);
	bool LoadTexturePNG(GLuint* tex, int* width, int* height, string filename);
	bool LoadTextureJPG(GLuint* tex, int* width, int* height, string filename);

//Variables
public:
	ShaderManager *shaders;

	VBOData *cube;

	GLuint colorMap;
	GLuint parallaxHeightMap;
	GLuint parallaxNormalMap;
	GLuint heightMap;
	GLuint heightNormalMap;

	matrix4 cubeWorld;

	matrix4 cameraProjection;
	matrix4 cameraView;
	vector3 cameraRotation;
	vector3 cameraTranslation;
	vector3 cameraTarget;

	bool wireframe;

	int frames;
	float timerCount;
};

#endif
