#version 150 core

uniform sampler2D heightmap;
uniform float texIncr;

in vec3 in_Color;
in vec3 in_Position;
in vec2 in_TexCoord;
out vec4 geom_Color;
out vec4 geom_Normal;

void main(){
	float height;
	float left, right, top, bottom;
	height = texture2D(heightmap, in_TexCoord).r;

	gl_Position = vec4(in_Position, 1.0);
	gl_Position.y += height *10.0;
	geom_Color=vec4(in_Color, 1.0);	
	//geom_Normal = normalize(vec4(in_Normal, .0));
	left = texture(heightmap, vec2(in_TexCoord.x-texIncr,in_TexCoord.y)).r;
	right = texture(heightmap, vec2(in_TexCoord.x+texIncr,in_TexCoord.y)).r;
	top = texture(heightmap, vec2(in_TexCoord.x, in_TexCoord.y+texIncr)).r;
	bottom = texture(heightmap, vec2(in_TexCoord.x, in_TexCoord.y-texIncr)).r;
	geom_Normal = normalize(vec4(.0, 1.0, .0, .0));
}

