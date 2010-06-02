#version 150 core
#extension GL_ARB_gpu_shader5 : enable

// Declare the incoming primitive type
layout(triangles) in;

// Declare the resulting primitive type
layout(line_strip, max_vertices=6)  out;

// Uniforms (from host code)
uniform mat4 mvpMatrix;

// Incoming from vertex shader
in vec4 geom_Color[3];
in vec4 geom_Normal[3];

// Outgoing per-vertex information
out vec4 interpColor;


void main(){
	int i;
	for (i = 0; i < gl_in.length(); i++){
		gl_Position = mvpMatrix * vec4(gl_in[i].gl_Position.xyzw);
		interpColor = vec4(.0, 1.0, .0, 1.0);
		EmitVertex();
		gl_Position = mvpMatrix * vec4(gl_in[i].gl_Position.xyzw+geom_Normal[i]*.1);
		EmitVertex();
	EndPrimitive();
	}
}
