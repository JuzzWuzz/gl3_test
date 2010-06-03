#version 150 core

in vec4 interpColor;
in vec4 interpNormal;

out vec4 frag_Color;

void main(){
	vec3 normal = normalize(interpNormal.xyz);
	frag_Color = interpColor;
}

