#ifndef _PROTO_MAIN_H
#define _PROTO_MAIN_H

typedef struct{
	float u, v;
} float2;

class ProtoApp : public reGL3App{
public:
	ProtoApp(AppConfig& conf);
	~ProtoApp();

	void		ProcessInput	(float dt);	//override
	void		Logic			(float dt); //override
	void		Render			(float dt); //override

private:
	bool		Init			(void);
	bool		InitGL			(void);

	bool		LoadHeightmap	(GLuint *tex, GLuint *normal_tex, string filename, string
			normalmap_Filename); 

public:
	ShaderProg*		m_shMain;	// use the provided shader program class

	GLuint			m_vao;
	GLuint			m_vbo[5];	// VBOs for vertices, normals, colors, tex coords, indices
	GLuint			m_heightmap_tex;
	GLuint			m_normalmap_tex;
	int				m_nIndices;

	matrix4			m_proj_mat;
	matrix4			m_camera_mat;
	vector3			m_cam_rotate;
	vector3			m_cam_translate;
	int				m_levels;	// levels of tessellation
	int				m_technique;
	float			m_rise;		// percentage of new teseellated vertex to use
};


#endif
