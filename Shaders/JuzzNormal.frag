#version 150 core

//Include common fragment shader parameters
#IncludeFile:CommonFrag.txt

//Additional shader specific parameters
in vec3 vert_LightDirOrig;

void main()
{
	//Read in variables and store
	vec2 texcoords = vert_Texcoords;
	vec3 normal = normalize(vert_Normal);
	vec3 view = normalize(vert_View);
	vec3 lightDir = normalize(vert_LightDir);

	//Check to see if the vertex can be seen by the light
	//If not the function call will set the pixel to ambient and skip this
	if (CanDoLighting(texcoords, normal, normalize(vert_LightDirOrig)))
	{
		//Read in the normal from the normal map for lighting
		normal = normalize(texture2D(parallaxNormalMap, texcoords).xyz * 2.0 - 1.0);

		//Output the final color
		frag_Color = CommonFragmentProcessing(texcoords, normal, view, lightDir);
	}
}
