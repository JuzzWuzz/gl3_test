#version 150 core

in vec4 interpColor;

out vec4 frag_Color;

void main(){
	frag_Color = interpColor;
}

