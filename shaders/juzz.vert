#version 150 core

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;
uniform vec3 light_Pos;
uniform vec3 camera_Pos;
uniform float useCameraLight;

in vec3 in_Position;
in vec3 in_Color;
in vec2 in_Texcoords;
in vec3 in_Normal;
in vec3 in_Tangent;

out vec3 vert_View;
out vec3 vert_LightDir;
out vec4 vert_Color;
out vec2 vert_Texcoords;

void main()
{
	//Perform world view projection matrix calculation
	mat4 mv = view * world;
	mat4 mvp = projection * mv;

	//Read in the normal and tangent vectors then calculate the binormal
	//The normal and tangent need to be multiplied with the normal matrix
	vec3 normal = normalize(vec3(mv * vec4(in_Normal, 0.0f)));
	vec3 tangent = normalize(vec3(mv * vec4(in_Tangent, 0.0f)));
	vec3 binormal = normalize(cross(normal, tangent));

	//Calculate the TBN matrix
	mat3 tbn = mat3(tangent.x, binormal.x, normal.x,
					tangent.y, binormal.y, normal.y,
					tangent.z, binormal.z, normal.z);

	//Output the final position of the vertex
	gl_Position = vec4(in_Position, 1.0);
	gl_Position = mvp * gl_Position;

	//Use the vertex's position to locate the position of the view vector
	//in tangent space.This is the '-' of the vertex position since calculations 
	//are done in eye space.
	vec4 pos = mv * vec4(in_Position, 1.0);
	vert_View = -(pos.xyz / pos.w);
	vert_View = tbn * vert_View;

	//Calculate the position of the light in tangent space.
	vec3 lightDir = light_Pos;
	if (useCameraLight == 1.0)
		lightDir = camera_Pos;
	vert_LightDir = vec3(view * vec4(lightDir, 0.0f));
	vert_LightDir = tbn * vert_LightDir;

	//Pass through the color of the vertex
	vert_Color = vec4(in_Color, 1.0);

	//Pass through the texture coordinates
	vert_Texcoords = in_Texcoords;
}
