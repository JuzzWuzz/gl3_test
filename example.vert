#version 150 core

in vec3 in_Color;
in vec3 in_Position;
out vec4 geom_Color;

void main(){
	gl_Position = vec4(in_Position, 1.0);
	geom_Color=vec4(in_Color, 1.0);	
}

