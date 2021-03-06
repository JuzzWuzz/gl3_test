#version 150 core

//Include common fragment shader parameters
#IncludeFile:CommonFrag.txt
in vec3 vert_NormalDisp;

void main()
{
	//Read in variables and store
	vec2 texcoords = vert_Texcoords;
	vec3 normal = normalize(vert_Normal);
	vec3 view = normalize(vert_View);
	vec3 lightDir = normalize(vert_LightDir);

	//Check to see if the vertex can be seen by the light
	//If not the function call will set the pixel to ambient and skip this
	if (CanDoLighting(texcoords, normal, lightDir))
	{
		//Set the normal to be the displacement mapped one
		normal = normalize(vert_NormalDisp);

		frag_Color = CommonFragmentProcessing(texcoords, normal, view, lightDir);
	}
}
