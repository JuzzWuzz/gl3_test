#version 150 core

#define HEIGHT 10.0

uniform sampler2D heightmap;
uniform float texIncr;
uniform float quadSize;

in vec3 in_Color;
in vec3 in_Position;
in vec2 in_TexCoord;
out vec4 geom_Color;
out vec4 geom_Normal;

void main(){
	float height, camera_height;
	float left, right, top, bottom;
	vec3 binormal, tangent;

	camera_height = texture2D(heightmap, vec2(.5,.5)).r + 1.0;
	height = texture2D(heightmap, in_TexCoord).r - camera_height;

	gl_Position = vec4(in_Position, 1.0);
	gl_Position.y += height *HEIGHT;
	geom_Color=vec4(in_Color, 1.0);	
	//geom_Normal = normalize(vec4(in_Normal, .0));
	
	left = texture2D(heightmap, 	vec2(in_TexCoord.s-texIncr,in_TexCoord.t)).r;
	right = texture2D(heightmap, 	vec2(in_TexCoord.s+texIncr,in_TexCoord.t)).r;
	top = texture2D(heightmap, 	vec2(in_TexCoord.s, in_TexCoord.t+texIncr)).r;
	bottom = texture2D(heightmap, vec2(in_TexCoord.s, in_TexCoord.t-texIncr)).r;

	tangent.x = .0;
	tangent.y = HEIGHT* (top-bottom);
	tangent.z = 2*quadSize;
	binormal.x = 2*quadSize;
	binormal.y = HEIGHT* (right-left);
	binormal.z = .0;

	geom_Normal = normalize(vec4(cross(tangent, binormal), .0));
}

