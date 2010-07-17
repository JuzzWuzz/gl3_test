/*
    Copyright (C) 2010 Justin Crause <juzz@crause.com>

	UCT Honours Project 2010 - Deformable Terrain
*/

#include "regl3.h"
#include "re_math.h"
#include "re_shader.h"
#include <vector>
#include <SDL/SDL_image.h>

using namespace reMath;

#define HPI (1.56905099753f)
#define  PI (3.14159265358f)
#define TPI (6.28318530716f)
#define VBOCOUNT 5
#define SHADERCOUNT 3
#define DISPSHADER true
#define DISABLESHADERS false

#ifndef JUZZUTILITY
#define JUZZUTILITY

//Class to handle the VBO data
class VBOData
{
public:
	VBOData(float n_divs, float n_width, bool cube);
	~VBOData(void);

	//Accessors
	int GetVerticiesSize(void);
	int GetTexcoordsSize(void);
	int GetNormalsSize(void);
	int GetTangentsSize(void);
	int GetIndiciesSize(void);
	int GetNumOfIndicies(void);
	vector3 *GetVerticies(void);
	vector2 *GetTexcoords(void);
	vector3 *GetNormals(void);
	vector3 *GetTangents(void);
	GLuint *GetIndicies(void);

private:
	//Hidden methods
	void CalculateCube(void);
	void CalculatePlane(void);
	void CalculateFace(vector3 tl, vector3 tr, vector3 bl);
	void CalculateNormalTangent(int vertCenter, int vertLeft, int vertRight);
	void AverageNormalsTangents(void);

	//Variables
	float divs;
	float width;
	int numOfVerticies;
	int numOfIndicies;
	int curVertOffset;
	int curIndOffset;

	//Initial texcoords
	vector2 textl;
	vector2 textr;
	vector2 texbl;

	//VBO data structures
	vector3 *verticies;
	vector2 *texcoords;
	vector3 *normals;
	vector3 *tangents;
	int *normalCount;
	GLuint *indicies;
};

//Class to manage multiple shaders
class ShaderManager
{
public:
	ShaderManager(void);
	~ShaderManager(void);
	bool AddShader(string vert, string geom, string frag);

	//Update methods
	void BindAttrib(char *name, int val);
	void UpdateUni1i(char *name, int val);
	void UpdateUni1f(char *name, float val);
	void UpdateUni3fv(char *name, float val[3]);
	void UpdateUniMat3fv(char *name, float val[9]);
	void UpdateUniMat4fv(char *name, float val[16]);

	int CompileAndLink(void);
	void SetActiveShader(int shader);

private:
	ShaderProg *shaders[SHADERCOUNT];
	int curIndex;
	int activeShader;
};

#endif
