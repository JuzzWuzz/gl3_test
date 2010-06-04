#ifndef _PROTO_MAIN_H
#define _PROTO_MAIN_H

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

public:
	ShaderProg*		m_shMain;	// use the provided shader program class
	ShaderProg*		m_shDrawNormals;

	GLuint			m_vao;
	GLuint			m_vbo[4];	// VBOs for vertices, normals, colors, indices
	matrix4			m_proj_mat;
	matrix4			m_camera_mat;
	vector3			m_cam_rotate;
	vector3			m_cam_translate;
	int				m_levels;	// levels of tessellation
	float			m_rise;		// percentage of new teseellated vertex to use
};


#endif
