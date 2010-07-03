#version 150 core

uniform mat4 world;
uniform vec3 worldTrans;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform vec3 light_Pos;
uniform int useCameraLight;

in vec3 in_Position;
in vec2 in_Texcoords;
in vec3 in_Normal;

out vec3 vert_Normal;
out vec3 vert_View;
out vec3 vert_LightDir;
out vec2 vert_Texcoords;

void main()
{
	//Perform model view projection matrix calculation
	mat4 mv = view * world;
	mat4 mvp = projection * view * world;

	//Output the final position of the vertex
	vec4 pos = vec4(in_Position, 1.0);
	gl_Position = mvp * pos;

	//Choice between camera light or world light
	if (useCameraLight == 1)
	{
		//Calculate the vertex pos and light in view space
		pos = mv * pos;
		vert_Normal = normalMatrix * in_Normal;
		vert_View = pos.xyz / pos.w;
		vert_LightDir = vec3(view * vec4(light_Pos - worldTrans, 0.0f));

	}
	else
	{
		//Calculate the vertex pos, light and normal in world space
		pos = world * pos;
		vert_Normal = vec3(world * vec4(in_Normal, 0.0));
		vert_View = pos.xyz / pos.w;
		vert_LightDir = light_Pos;
	}

	//Pass through the texture coordinates
	vert_Texcoords = in_Texcoords;
}
