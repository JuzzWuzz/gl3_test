#version 150 core

uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform int useParallaxMapping;

in vec3 vert_NormalOrig;
in vec3 vert_LightDirOrig;
in vec3 vert_View;
in vec3 vert_LightDir;
in vec2 vert_Texcoords;

vec4 light_Ambient = vec4(0.19225, 0.19225, 0.19225, 1.0);
vec4 light_Diffuse = vec4(0.50754, 0.50754, 0.50754, 1.0);
vec4 light_Specular = vec4(0.508273, 0.508273, 0.508273, 1.0);

float parallaxScale = 0.05;
float parallaxBias = -0.026;

out vec4 frag_Color;

void main()
{
	//Initial variables and settings
	vec2 texcoords = vert_Texcoords;
	vec4 colorTex = texture2D(colorMap, texcoords);
	vec4 ambient = light_Ambient;
	vec4 diffuse = light_Diffuse;
	vec4 specular = light_Specular;

	//Check the dot product of the origional normal and light direction
	//to see if the light actually strikes the origional surface.
	if (dot(normalize(vert_NormalOrig), normalize(vert_LightDirOrig)) < 0.0)
	{
		frag_Color = ambient * colorTex;
		return;
	}

	//Read in the light direction and view vectors from vertex shader and normalize
	vec3 lightDir = normalize(vert_LightDir);
	vec3 view = normalize(vert_View);

	//If using parallax mapping then alter the texcoords
	if (useParallaxMapping == 1)
	{
		//Get the height fromt eh heightmap for the parallax effect
		float height = texture2D(heightMap, texcoords).r;
		height = (height * parallaxScale) + parallaxBias;
	
		//Set the new texcoords
		texcoords = vert_Texcoords + (height * view.xy);
	}
	
	//Read in the normal stored in the normal map if normal mapping enabled
	vec3 normal = normalize(texture2D(normalMap, texcoords).xyz * 2.0 - 1.0);

	//Calculate the reflec vector
	vec3 reflec = normalize(-reflect(lightDir, normal));

	//Calculate the diffuse intensity
	float diffuseIntensity = max(0.0, dot(normal, lightDir));

	//Calculate the specular intensity
	float specularIntensity = pow(max(0.0, dot(reflec, view)), 60.0);

	//Factor in the intensities to the diffuse and specular amounts
	diffuse *= diffuseIntensity;
	specular *= specularIntensity;
	
	//Add in the diffuse and specular
	frag_Color = (ambient + diffuse + specular) * colorTex;
}
