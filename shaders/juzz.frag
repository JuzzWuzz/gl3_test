#version 150 core

in vec4 vert_Color;

out vec4 frag_Color;

void main(){

	frag_Color = vert_Color;
}

/*
uniform vec3 camera_pos;

in vec3 vert_Tangent;
in vec3 vert_Binormal;
in vec3 vert_Normal;
in vec4 vert_Pos;
in mat3 tbn;

	vec3 normal = normalize(vert_Normal);
	vec3 tangent = normalize(vert_Tangent);
	vec3 binormal = normalize(vert_Binormal);

	vec3 lightDir = mul(tbn, normalize(vec3(1.0, 5.0f, 8.0f)));
	//vec3 lightDir = normalize(vec3(1.0, 5.0f, 8.0f));
	vec3 view = mul(tbn, normalize(camera_pos - vert_Pos.xyz));
	//vec3 view = normalize(camera_pos - vert_Pos.xyz);

	//Calculate the reflection vector
	vec3 reflec = normalize(reflect(-lightDir, normal));
	
	//Calculate the diffuse part
	float diffuseIntensity = max(dot(normal, lightDir), 0.0f);

	//Calculate the specular part
	float specularPower = 4096.0;
	float specularIntensity = pow(max(dot(reflec, view), 0.0f), specularPower);

	vec4 materialDiffuseColor = vec4(0.8, 0.8, 0.8, 1.0);
	vec4 materialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

	vec4 lightAmbient = vec4(0.2, 0.2, 0.2, 1.0);

	materialDiffuseColor *= diffuseIntensity * vert_Color;
	materialSpecularColor *= specularIntensity;

	frag_Color = (materialDiffuseColor + materialSpecularColor);
	*/