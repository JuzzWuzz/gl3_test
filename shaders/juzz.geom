#version 150 core
#extension GL_ARB_gpu_shader5 : enable
#pragma optionNV unroll all
#pragma optionNV inline all

//Declare the incoming primitive type
layout(triangles) in;

//Declare the resulting primitive type
layout(triangles, max_vertices=3)  out;

uniform mat4 world;
uniform mat4 view;

in vec4 vert_Pos[3];
in vec4 vert_Color[3];
in vec3 vert_Normal[3];
in vec2 vert_Texcoords[3];
//in mat3 vert_tbn[3];

out vec4 geom_Pos;
out vec4 geom_Color;
out vec3 geom_Normal;
out vec2 geom_Texcoords;
//out mat3 geom_tbn;

void main()
{
	// calculate transformed vertices and store globally
	mat4 viewMatrix = view * world;
	vec4 viewVerts[3];
	viewVerts[0] = gl_in[0].gl_Position;
	viewVerts[1] = gl_in[1].gl_Position;
	viewVerts[2] = gl_in[2].gl_Position;
	viewVerts[0] = viewMatrix * viewVerts[0];
	viewVerts[1] = viewMatrix * viewVerts[1];
	viewVerts[2] = viewMatrix * viewVerts[2];

	//Frustum Culling
	vec3 x = abs(vec3(viewVerts[0].x, viewVerts[1].x, viewVerts[2].x));
	vec3 y = abs(vec3(viewVerts[0].y, viewVerts[1].y, viewVerts[2].y));
	vec3 z = -vec3(viewVerts[0].z, viewVerts[1].z, viewVerts[2].z);

	if (!any(greaterThan(z, vec3(.5))))
		return;
	if (!any(lessThan(x,1.333333*z)))
		return;
	if (!any(lessThan(y,z)))
		return;

	for (int i = 0; i < 3; i++)
	{
		gl_Position = gl_in[i].gl_Position;
		geom_Pos = vert_Pos[i];
		geom_Color = vert_Color[i];
		geom_Normal = vert_Normal[i];
		geom_Texcoords = vert_Texcoords[i];
		//geom_tbn = vert_tbn[i];

		EmitVertex();
	}
	EndPrimitive();
}
