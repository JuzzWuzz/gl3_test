#version 150 core

//Include common vertex shader parameters
#IncludeFile:CommonVert.txt

//Additional shader specific parameters
in vec3 in_Tangent;
out vec3 vert_LightDirOrig;

void main()
{
	//Call the common method for vertex shader
	CommonVertexProcessing();

	//Read in the tangent and calculate the binormal
	vec3 tangent = normalize(normalMatrix * in_Tangent);
	vec3 binormal = normalize(cross(vert_Normal, tangent));

	//Calculate the TBN matrix
	mat3 tbn = mat3(tangent.x, binormal.x, vert_Normal.x,
					tangent.y, binormal.y, vert_Normal.y,
					tangent.z, binormal.z, vert_Normal.z);


	//Save orig lightDir before tbn multiplication
	vert_LightDirOrig = vert_LightDir;

	//Multiply the view and light vectors by the tbn matrix
	vert_View = tbn * vert_View;
	vert_LightDir = tbn * vert_LightDir;
}
