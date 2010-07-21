#version 150 core

//Include common fragment shader parameters
#IncludeFile:CommonFrag.txt

//Additional shader specific parameters
in vec3 vert_View_TBN;
in vec3 vert_LightDir_TBN;
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
		//Normalize the tangent space vectors
		vec3 viewTBN = normalize(vert_View_TBN);
		vec3 lightDirTBN = normalize(vert_LightDir_TBN);

		//Set the normal to be the displacement mapped one
		normal = normalize(vert_NormalDisp);

		frag_Color = CommonFragmentProcessing(texcoords, normal, view, lightDir);

		//Iterative parallax mapping technique
		vec3 parallaxTexcoords = vec3(texcoords, 0.0);
		for (int i = 0; i < parallaxItrNum; i++)
		{
			//Get the height from the heightmap for the parallax effect
			vec4 hmap = texture2D(parallaxHeightMap, parallaxTexcoords.xy);
			float height = (hmap.r * parallaxScale) + parallaxBias;
	
			//Update the texcoords and extract the normal
			parallaxTexcoords += (height - parallaxTexcoords.z) * hmap.z * viewTBN;
		}

		//Read the normal offset by the parallax effect
		normal = normalize(texture2D(parallaxNormalMap, parallaxTexcoords.xy).xyz * 2.0 - 1.0);

		//Output the final color
		frag_Color = (frag_Color + CommonFragmentProcessing(texcoords, normal, viewTBN, lightDirTBN)) * 0.5;
	}
}
