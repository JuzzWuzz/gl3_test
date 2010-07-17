#version 150 core

//Include common vertex shader parameters
#IncludeFile:CommonVert.txt

//Additional shader specific parameters
uniform sampler2D normalMap;
uniform sampler2D heightMap;

void main()
{
	//Need to access texcoords (DUPLICATED - NO WORKAROUND FOUND)
	vec2 texcoords = in_Texcoords * texScale;

	//Only displace non-border verticies
	if (!(texcoords.s == 0.0 || texcoords.s == texScale || texcoords.t == 0.0 || texcoords.t == texScale))
	{
		heightOffset = vec4(in_Normal, 0.0) * texture2D(heightMap, texcoords).r;
		CommonVertexProcessing();
		vert_Normal = normalize(texture2D(normalMap, vert_Texcoords).xyz * 2.0 - 1.0);
	}
	//On the border - do not displace
	else
	{
		CommonVertexProcessing();
	}

}
