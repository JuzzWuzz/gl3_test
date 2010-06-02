#version 150 core

in vec3 in_Color;
in vec3 in_Position;
in vec3 in_Normal;
out vec4 geom_Color;
out vec4 geom_Normal;

void main(){
	gl_Position = vec4(in_Position, 1.0);
	geom_Color=vec4(in_Color, 1.0);	
	geom_Normal = normalize(vec4(in_Normal, .0));
}

