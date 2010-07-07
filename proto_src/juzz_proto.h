/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	Programed in collaboration with Andrew Flower <andrew.flower@gmail.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "regl3.h"
#include "re_math.h"
#include "re_shader.h"
#include <SDL/SDL_image.h>

using namespace reMath;

#ifndef JUZZ_PROTO
#define JUZZ_PROTO

//A float 2 struct
typedef struct{
	float u, v;
} float2;

#define HPI (1.56905099753f)
#define  PI (3.14159265358f)
#define TPI (6.28318530716f)
#define VBOCOUNT 5

class juzz_proto : public reGL3App
{
//Methods
public:
	juzz_proto(AppConfig& conf);
	~juzz_proto();

	//Overridden methods
	void ProcessInput(float dt);
	void Logic(float dt);
	void Render(float dt);

private:
	bool Init(void);
	bool InitGL(void);

	void UpdateLight();
	void UpdateViewMatrix();
	void UpdateNormalMatrix(matrix4 world); 
	void CalculateNormals(vector3 *verticies, int size, vector3 *normals);
	void CalculateTexcoords(vector2 *texcoords, int size);
	void CalculateTangents(vector3 *verticies, vector2 *texcoords, int size, vector3 *tangents);
	bool CheckError(string);
	bool LoadTexturePNG(GLuint* tex, int* width, int* height, string filename);
	bool LoadTextureJPG(GLuint* tex, int* width, int* height, string filename);

//Variables
public:
	ShaderProg* shader; //Use the provided shader program class

	GLuint cubeVAO;
	GLuint cubeVBO[VBOCOUNT]; //VBOs for vertices, normals, tangents, tex coords, indices
	int numOfIndices;

	GLuint colorMap;
	GLuint normalMap;
	GLuint heightMap;

	matrix4 cubeWorld;

	matrix4 cameraProjection;
	matrix4 cameraView;
	vector3 cameraRotation;
	vector3 cameraTarget;

	float angle;
	bool useCameraLight;
	bool useParallaxMapping;
	float texScale;

	int frames;
	float timerCount;
};

#endif
