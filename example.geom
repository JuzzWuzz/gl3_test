#version 150 core
#extension GL_ARB_gpu_shader5 : enable

// Declare the incoming primitive type
layout(triangles) in;

// Declare the resulting primitive type
layout(triangles, max_vertices=6)  out;

// Uniforms (from host code)
uniform mat4 mvpMatrix;

// Incoming from vertex shader
in vec4 geom_Color[3];

// Outgoing per-vertex information
out vec4 interpColor;


void main(){
	int i;
	for (i = 0; i < gl_in.length(); i++){
		gl_Position = mvpMatrix * vec4(gl_in[i].gl_Position.xyzw);
		interpColor = geom_Color[i];
		EmitVertex();
	}
	EndPrimitive();
	// Output a mirror of the above tetrahedron
	for (i = 0; i < gl_in.length(); i++){
		gl_Position = gl_in[i].gl_Position;
		gl_Position = mvpMatrix * vec4(-gl_Position.xyz, gl_Position.w);
		interpColor = geom_Color[i];
		EmitVertex();
	}
	EndPrimitive();
}
