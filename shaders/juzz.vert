#version 150 core

#define HEIGHT 10.0

uniform sampler2D heightmap;
uniform sampler2D normalmap;
uniform vec3 camera_pos;
uniform mat4 rotprojMatrix;

in vec3 in_Color;
in vec3 in_Position;
in vec2 in_TexCoord;

out vec4 vert_Color;
out vec3 vert_Normal;
out vec2 vert_TexCoord;

void main(){
	float height, camera_height;
	vec2 texCoord;

	texCoord.s = in_TexCoord.s + camera_pos.x;
	texCoord.t = in_TexCoord.t + camera_pos.z;

	camera_height = 1.0;
	height = texture2D(heightmap, texCoord).r - camera_height;

	gl_Position = vec4(in_Position, 1.0);
	gl_Position.y += height * HEIGHT;
	gl_Position = rotprojMatrix * gl_Position;

	vert_Color = vec4(in_Color, 1.0);	
	
	//we swizzle the rgb to rbg so that blue represents vertical vector
	vert_Normal = normalize(texture2D(normalmap, texCoord).rbg*2.0 - 1.0);

	vert_TexCoord = texCoord;
}

