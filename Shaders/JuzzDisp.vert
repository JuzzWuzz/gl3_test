#version 150 core

//Include common vertex shader parameters
#IncludeFile:CommonVert.txt

//Additional shader specific parameters
uniform sampler2D heightMap;
uniform sampler2D heightNormalMap;
out vec3 vert_NormalDisp;

void main()
{
	//Need to access texcoords (DUPLICATED - NO WORKAROUND FOUND)
	vec2 texcoords = in_Texcoords;

	//Calculate rotation matrix to reorientate the normal
	//Vec a -> Textures stored with z-up so set this statically
	//Vec b -> Normalized form of the vertex normal
	vec3 a = vec3(0.0, 0.0, 1.0);
	vec3 b = normalize(in_Normal);
	
	//Calculate the cosine and t value
	//Check for special case 180degrees and correct for it
	float c = dot(a, b);
	bool specialCase = false;
	if (c == -1.0)
	{
		specialCase = true;
		b = normalize(vec3(1.0, 0.0, 0.0));
		c = dot(a, b);
	}
	float t = 1.0 - c;
	
	//Use the corss profuct as the axis of rotation
	//Use the cross product for the value of sine
	vec3 axis = cross(a, b);
	float s = length(axis);
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	//Construct the rotation matrix
	mat3 normalRot = mat3(t*x*x + c  , t*x*y + z*s, t*x*z - y*s,
						  t*x*y - z*s, t*y*y + c  , t*y*z + x*s,
						  t*x*z + y*s, t*y*z - x*s, t*z*z + c);

	//Use two 90 degree shifts for the 180 degree rotations to prevent bug
	if (specialCase)
		normalRot *= normalRot;

	//Only displace non-border verticies
	if (!(texcoords.s == 0.0 || texcoords.s == 1.0 || texcoords.t == 0.0 || texcoords.t == 1.0))
	{  
		//Read in the height value and offset this in the normals direction
		heightOffset = vec4(in_Normal, 0.0) * texture2D(heightMap, texcoords).r;
		CommonVertexProcessing();
		
		//Read in the normal for the heightmap and multiply this with the normalMatrix and normalRot
		vert_NormalDisp = normalMatrix * normalRot * normalize(texture2D(heightNormalMap, vert_Texcoords).xyz * 2.0 - 1.0);
	}
	//On the border - do not displace
	else
	{
		CommonVertexProcessing();
		vert_NormalDisp = vert_Normal;
	}
}
