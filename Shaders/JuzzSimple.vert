#version 150 core

//Common uniform variables
uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

//Input variables
in vec3 in_Position;

void main()
{
	//Perform model view projection matrix calculation
	mat4 mvp = projection * view * world;

	//Output the final position of the vertex
	vec4 pos = vec4(in_Position, 1.0);
	gl_Position = mvp * pos;
}
