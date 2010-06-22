/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	Programed in collaboration with Andrew Flower <andrew.flower@gmail.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "regl3.h"
#include "re_math.h"
#include "re_shader.h"

using namespace reMath;

#ifndef JUZZ_PROTO
#define JUZZ_PROTO

//A float 2 struct
typedef struct{
	float u, v;
} float2;

#define PI (3.14159265358f)
#define VBOCOUNT 4

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

	void CalculateNormals(vector3 *verticies, int size, vector3 *normals);
	bool CheckError(string);
	bool LoadHeightmap(GLuint *tex, GLuint *normal_tex, string filename, string normalmap_Filename); 

//Variables
public:
	ShaderProg* m_shMain; //Use the provided shader program class

	GLuint cubeVAO;
	GLuint cubeVBO[VBOCOUNT]; //VBOs for vertices, normals, colors, tex coords, indices
	//vector3 *verticies;
	//vector3 *normals;
	int numOfIndices;
	int terrainNumOfIndices;

	GLuint m_heightmap_tex;
	GLuint m_normalmap_tex;

	matrix4 cubeWorld;

	matrix4 cameraProjection;
	matrix4 cameraView;
	vector3 cameraRotation;
	vector3 cameraTranslation;

	bool runn;
	float angle;
};

#endif
