#version 150 core

in vec4 vert_Color;
in vec3 vert_Normal;

out vec4 frag_Color;

void main(){
	vec3 light = normalize(vec3(.0, -.8, -1.0));

	vec3 normal = normalize(vert_Normal);
	frag_Color = vert_Color * dot(-light, normal);
}

