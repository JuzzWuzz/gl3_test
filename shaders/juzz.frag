#version 150 core

uniform sampler2D colorMap;
uniform sampler2D normalMap;

in vec3 vert_View;
in vec3 vert_LightDir;
in vec4 vert_Color;
in vec2 vert_Texcoords;

vec4 light_Ambient = vec4(0.1, 0.1, 0.1, 1.0);
vec4 light_Diffuse = vec4(0.7, 0.7, 0.7, 1.0);
vec4 light_Specular = vec4(1.0, 1.0, 1.0, 1.0);

out vec4 frag_Color;

void main()
{
	//Initial variables and settings
	vec4 colorTex = texture2D(colorMap, vert_Texcoords);
	vec4 ambient = light_Ambient * colorTex;
	vec4 diffuse = light_Diffuse * colorTex;
	vec4 specular = light_Specular;

	frag_Color = ambient;

	//Read in the light direction and view vectors from vertex shader and normalize
	//Read in the normal stored in the normal map
	vec3 lightDir = normalize(vert_LightDir);
	vec3 normal = normalize(texture2D(normalMap, vert_Texcoords).rgb * 2.0 - 1.0);
	vec3 view = normalize(vert_View);

	//Check the dot product of the origional normal and light direction
	//to see if the light actually strikes the origional surface.
	//if (dot(normal, normalize(vec3(view * vec4(-light_Pos, 0.0f)))) < 0.0)
	//	return;
	
	//Calculate the reflec vector
	vec3 reflec = normalize(-reflect(lightDir, normal));

	//Calculate the diffuse intensity
	float diffuseIntensity = max(0.0, dot(normal, lightDir));

	//Calculate the specular intensity
	float specularIntensity = pow(max(0.0, dot(reflec, view)), 96.0);

	//Factor in the intensities to the diffuse and specular amounts
	diffuse *= diffuseIntensity;
	specular *= specularIntensity;
	
	//Add in the diffuse and specular
	frag_Color += diffuse + specular;
}
