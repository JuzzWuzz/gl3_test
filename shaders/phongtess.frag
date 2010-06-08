#version 150 core


in vec3 interpColor;
in vec3 interpNormal;

out vec4 frag_Color;

void main(){
	vec3 light = vec3(.0, -.3, -1.0);

	vec3 normal = normalize(interpNormal);
	frag_Color = vec4(interpColor * dot(-light, normal),1.0);
	//frag_Color = vec4(dot(-light, normal));
	//frag_Color = interpColor;
}

