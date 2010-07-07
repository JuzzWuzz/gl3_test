#version 150 core

uniform mat4 world;
uniform vec3 worldTrans;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform float texScale;
uniform vec3 light_Pos;
uniform int useCameraLight;

in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_Tangent;
in vec2 in_Texcoords;

out vec3 vert_NormalOrig;
out vec3 vert_LightDirOrig;
out vec3 vert_View;
out vec3 vert_LightDir;
out vec2 vert_Texcoords;

void main()
{
	//Perform model view projection matrix calculation
	mat4 mv = view * world;
	mat4 mvp = projection * view * world;

	//Read in the normal and tangent vectors then calculate the binormal
	//The normal and tangent need to be multiplied with the normal matrix
	vec3 normal = normalize(normalMatrix * in_Normal);
	vec3 tangent = normalize(normalMatrix * in_Tangent);
	vec3 binormal = normalize(cross(normal, tangent));

	//Calculate the TBN matrix
	mat3 tbn = mat3(tangent.x, binormal.x, normal.x,
					tangent.y, binormal.y, normal.y,
					tangent.z, binormal.z, normal.z);

	//Output the final position of the vertex
	vec4 pos = vec4(in_Position, 1.0);
	gl_Position = mvp * pos;

	//Choice between camera light or world light
	if (useCameraLight == 1)
	{
		//Calculate the vertex pos and light in view space
		pos = mv * pos;
		vert_LightDir = vec3(view * vec4(light_Pos - worldTrans, 0.0f));
	}
	else
	{
		//Calculate the vertex pos, light and normal in world space
		pos = world * pos;
		vert_LightDir = light_Pos;
		normal = vec3(world * vec4(in_Normal, 0.0));
	}

	//Save orig vectors before tbn multiplication
	vert_NormalOrig = normal;
	vert_LightDirOrig = vert_LightDir;

	//Multiply the view and light vectors by the tbn matrix
	vert_View = tbn * -(pos.xyz / pos.w);
	vert_LightDir = tbn * vert_LightDir;

	//Pass through the texture coordinates
	vert_Texcoords = in_Texcoords * texScale;
}
