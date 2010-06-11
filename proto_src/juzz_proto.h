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

class juzz_proto : public reGL3App
{
//Methods
public:
	juzz_proto(AppConfig& conf);
	~juzz_proto();

	//Overridden methods
	void ProcessInput (float dt);
	void Logic (float dt);
	void Render (float dt);

private:
	bool Init (void);
	bool InitGL (void);

	bool CheckError(string);
	bool LoadHeightmap(GLuint *tex, GLuint *normal_tex, string filename, string normalmap_Filename); 

//Variables
public:
	ShaderProg* m_shMain; //Use the provided shader program class

	GLuint m_vao;
	GLuint m_vbo[4]; //VBOs for vertices, normals, colors, tex coords, indices
	GLuint m_heightmap_tex;
	GLuint m_normalmap_tex;
	int m_nIndices;
	float m_scale_metreToTex;

	matrix4 m_proj_mat;
	matrix4 m_camera_mat;
	vector3 m_cam_rotate;
	vector3 m_cam_translate;
};

#endif
