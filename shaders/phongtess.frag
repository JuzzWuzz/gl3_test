#version 150 core

uniform mat4 mvpMatrix;

in vec4 interpColor;
in vec4 interpNormal;

out vec4 frag_Color;

void main(){
	vec4 light = vec4(.0, -.3, -1.0, 0.0);
	light = mvpMatrix * light;

	vec4 normal = normalize(interpNormal);
	//frag_Color = interpColor * dot(-light, normal);
	//frag_Color = vec4(dot(-light, normal));
	frag_Color = interpColor;
}

