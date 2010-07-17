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
	vec3 lightDir = normalize(vert_LightDir);
	vec3 view = normalize(vert_View);

	//Check to see if the vertex can be seen by the light
	//If not the function call will set the pixel to ambient and skip this
	if (CanDoLighting(texcoords, normal, lightDir))
	{
		//Get the height from the heightmap for the parallax effect
		float height = texture2D(heightMap, texcoords).r;
		height = (height * parallaxScale) + parallaxBias;
	
		//Update the texcoords and extract the normal
		vec2 parallaxTexcoords = vert_Texcoords + (height * view.xy);
		normal = normalize(texture2D(normalMap, parallaxTexcoords).xyz * 2.0 - 1.0);

		//Output the final color
		frag_Color = CommonFragmentProcessing(texcoords, normal, lightDir, view);
	}
}
