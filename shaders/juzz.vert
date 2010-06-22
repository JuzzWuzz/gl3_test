#version 150 core

#define HEIGHT 10.0

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 camera_pos;

in vec3 in_Position;
in vec3 in_Color;
in vec3 in_Normal;

out vec4 vert_Color;


void main()
{
	//Perform world view projection matrix calculation
	mat4 wvp = projection * view * world;

	gl_Position = vec4(in_Position, 1.0);
	gl_Position = wvp * gl_Position;


	vert_Color = vec4(in_Color, 1.0);
}

/*

uniform sampler2D heightmap;
uniform sampler2D normalmap;
uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 camera_pos;

in vec2 in_TexCoord;

out vec2 vert_TexCoord;
out vec3 vert_Tangent;
out vec3 vert_Binormal;
out vec3 vert_Normal;
out mat3 tbn;
out vec4 vert_Pos;


	float height, camera_height;
	vec2 texCoord;

	texCoord.s = in_TexCoord.s + camera_pos.x;
	texCoord.t = in_TexCoord.t + camera_pos.z;

	camera_height = 1.0;
	height = texture2D(heightmap, texCoord).r - camera_height;

	//Perform world view projection matrix calculation
	mat4 wvp = projection * view * world;

	gl_Position = vec4(in_Position, 1.0);
	gl_Position.y += height * HEIGHT;
	//vert_Pos = world * gl_Position;
	//vert_Pos /= vert_Pos.w;
	gl_Position = wvp * gl_Position;

	vert_Color = vec4(in_Color, 1.0);
	
	//we swizzle the rgb to rbg so that blue represents vertical vector
	vert_Normal = vec3(1.0, 0.0, 0.0);//normalize(texture2D(normalmap, texCoord).rbg*2.0 - 1.0);
	vert_Tangent = cross(vert_Normal, vec3(0.0, 1.0, 0.0));
	vert_Binormal = cross(vert_Normal, vert_Tangent);

	tbn[0] = vert_Tangent;
	tbn[1] = vert_Binormal;
	tbn[2] = vert_Normal;

	vert_TexCoord = texCoord;
*/