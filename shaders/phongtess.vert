#version 150 core

#define HEIGHT 10.0

uniform sampler2D heightmap;
uniform sampler2D normalmap;
uniform float texIncr;
uniform float quadSize;
uniform vec3 camera_pos;

in vec3 in_Color;
in vec3 in_Position;
in vec2 in_TexCoord;
out vec4 geom_Color;
out vec4 geom_Normal;
out vec2 geom_TexCoord;

void main(){
	float height, camera_height;
	float left, right, top, bottom;
	vec3 binormal, tangent;
	vec2 texCoord;

	texCoord.s = in_TexCoord.s + camera_pos.x;
	texCoord.t = in_TexCoord.t + camera_pos.z;

	//camera_height = texture2D(heightmap, vec2(.5,.5)).r + 1.0;
	camera_height = 1.0;
	height = texture2D(heightmap, texCoord).r - camera_height;

	gl_Position = vec4(in_Position, 1.0);
	gl_Position.y += height *HEIGHT;
	geom_Color=vec4(in_Color, 1.0);	
	
	// we swizzle the rgb to rbg so that blue represents vertical vector
	geom_Normal = vec4(normalize(texture2D(normalmap, texCoord).rbg*2.0 - 1.0),.0);

	geom_TexCoord = texCoord;
}

