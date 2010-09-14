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

		//Iterative parallax mapping technique
		vec3 parallaxTexcoords = vec3(vert_Texcoords, 0.0);
		for (int i = 0; i < parallaxItrNum; i++)
		{
			//Get the height from the heightmap for the parallax effect
			vec4 hmap = texture2D(parallaxHeightMap, parallaxTexcoords.xy);
			float height = (hmap.r * parallaxScale) + parallaxBias;
	
			//Update the texcoords and extract the normal
			parallaxTexcoords += (height - parallaxTexcoords.z) * hmap.z * view;
		}

		normal = normalize(texture2D(parallaxNormalMap, parallaxTexcoords.xy).xyz * 2.0 - 1.0);
		
		/*/Non iterative method
		//Parallax mapping with offset limiting
		vec2 parallaxTexcoords = vert_Texcoords;
		float height = (texture2D(parallaxHeightMap, parallaxTexcoords).r * parallaxScale) + parallaxBias;
		parallaxTexcoords += height * view.xy;

		normal = normalize(texture2D(parallaxNormalMap, parallaxTexcoords).xyz * 2.0 - 1.0);
		/*/

		//Output the final color
		frag_Color = CommonFragmentProcessing(texcoords, normal, view, lightDir);
	}
}
