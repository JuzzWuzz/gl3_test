#version 150 core
#extension GL_ARB_gpu_shader5 : enable

// Declare the incoming primitive type
layout(triangles) in;

// Declare the resulting primitive type
layout(triangles, max_vertices=108)  out;

// Uniforms (from host code)
uniform sampler2D heightmap;
uniform mat4 rotprojMatrix;
uniform int degree;
uniform int technique;
uniform float rise;

// Incoming from vertex shader
in vec4 geom_Color[3];
in vec4 geom_Normal[3];
in vec2 geom_TexCoord[3];

// Outgoing per-vertex information
out vec3 interpColor;
out vec3 interpNormal;

// Triangle with vertices specified using barycentric coordinates
struct Triangle{
	vec3 v0, v1, v2;
};


void equi_1(int levels, Triangle t);
void equi_2(int levels, Triangle t);
void equi_cent(int levels, Triangle t);
void equi_bisect_1(int levels, Triangle t);
void equi_bisect_2(int levels, Triangle t);
void centroid_1(int levels, Triangle t);
void centroid_2(int levels, Triangle t);
void centroid_3(int levels, Triangle t);
void bisect_1(int levels, Triangle t);
void bisect_2(int levels, Triangle t);
void bisect_3(int levels, Triangle t);
void bisect_4(int levels, Triangle t);
void bisect_5(int levels, Triangle t);
void make_tri(Triangle t);
vec4 vert_interp(vec3 v);


// Globals
vec4 vertex[3];
vec4 normal[3];

//--------------------------------------------------------
void main(){
	int i;
	Triangle t;

	// calculate transformed vertices and store globally
	vertex[0] = gl_in[0].gl_Position;
	vertex[1] = gl_in[1].gl_Position;
	vertex[2] = gl_in[2].gl_Position;
	normal[0] = normalize(geom_Normal[0]);
	normal[1] = normalize(geom_Normal[1]);
	normal[2] = normalize(geom_Normal[2]);

	// "Recursively" tessellate triangle
	if (degree == 0){
		for (i = 0; i < gl_in.length(); i++){
			gl_Position = rotprojMatrix * vertex[i];
			interpColor = geom_Color[i].xyz;
			interpNormal= normal[i].xyz;
			EmitVertex();
		}
		EndPrimitive();
	}
	else{

		t.v0 = vec3(1.0, .0, .0);
		t.v1 = vec3(.0, 1.0, .0);
		t.v2 = vec3(.0, .0, 1.0);
	
		if (technique==0){
			bisect_1(min(5, degree) - 1 ,t);
		}
		else if (technique==1){
			equi_1(min(2,degree) - 1 ,t);
		}
		else if (technique==2){
			centroid_1(min(3,degree) - 1 ,t);
		}
		else if (technique==3){
			equi_bisect_1(min(3, degree) -1,t);
		}
	}
}

//--------------------------------------------------------
vec4 vert_interp(vec3 v){
	vec4 newvert;
	vec2 interpTexCoord;
	float a,b,c;

	// barycentric parameters
	a = v.x;
	b = v.y;
	c = v.z;

	newvert = a * vertex[0] + b * vertex[1] + c * vertex[2];
	// compute texture coordinates
	interpTexCoord = a * geom_TexCoord[0] + b * geom_TexCoord[1] + c * geom_TexCoord[2];
	newvert.y = 10.0*texture2D(heightmap, interpTexCoord).r-10.0;

	return newvert;
}

//--------------------------------------------------------
void make_tri (Triangle t){
	vec4 newvert, p0, p1, p2;
	
	// Vertex 1
	gl_Position = rotprojMatrix * vert_interp(t.v0);
	interpColor = (t.v0.x * geom_Color[0] + t.v0.y * geom_Color[1] + t.v0.z * geom_Color[2]).xyz;
	interpNormal= (t.v0.x * normal[0] + t.v0.y * normal[1] + t.v0.z * normal[2]).xyz;
	EmitVertex();

	// Vertex 2
	gl_Position = rotprojMatrix * vert_interp(t.v1);
	interpColor = (t.v1.x * geom_Color[0] + t.v1.y * geom_Color[1] + t.v1.z * geom_Color[2]).xyz;
	interpNormal= (t.v1.x * normal[0] + t.v1.y * normal[1] + t.v1.z * normal[2]).xyz;
	EmitVertex();

	// Vertex 3
	gl_Position = rotprojMatrix * vert_interp(t.v2);
	interpColor = (t.v2.x * geom_Color[0] + t.v2.y * geom_Color[1] + t.v2.z * geom_Color[2]).xyz;
	interpNormal= (t.v2.x * normal[0] + t.v2.y * normal[1] + t.v2.z * normal[2]).xyz;
	EmitVertex();
	EndPrimitive();
}

//--------------------------------------------------------
void centroid_1(int levels, Triangle t){
	int i, j;
	Triangle t_new[3];
	vec3 v_cent;

	v_cent = (t.v0 + t.v1 + t.v2)*.3333333;

	t_new[0].v0 = t.v0;
	t_new[0].v1 = t.v1;
	t_new[0].v2 = v_cent;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v_cent;

	t_new[2].v0 = t.v2;
	t_new[2].v1 = t.v0;
	t_new[2].v2 = v_cent;

	if (levels==0){
		for (j = 0; j < 3; j++){
			make_tri(t_new[j]);
		}
	}
	else{
		centroid_2(levels - 1, t_new[0]);
		centroid_2(levels - 1, t_new[1]);
		centroid_2(levels - 1, t_new[2]);
	}

}

//--------------------------------------------------------
void centroid_2(int levels, Triangle t){
	int i, j;
	Triangle t_new[3];
	vec3 v_cent;

	v_cent = (t.v0 + t.v1 + t.v2)*.3333333;

	t_new[0].v0 = t.v0;
	t_new[0].v1 = t.v1;
	t_new[0].v2 = v_cent;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v_cent;

	t_new[2].v0 = t.v2;
	t_new[2].v1 = t.v0;
	t_new[2].v2 = v_cent;

	if (levels==0){
		for (j = 0; j < 3; j++){
			make_tri(t_new[j]);
		}
	}
	else{
		centroid_3(levels - 1, t_new[0]);
		centroid_3(levels - 1, t_new[1]);
		centroid_3(levels - 1, t_new[2]);
	}

}
//--------------------------------------------------------
void centroid_3(int levels, Triangle t){
	int i, j;
	Triangle t_new[3];
	vec3 v_cent;

	v_cent = (t.v0 + t.v1 + t.v2)*.3333333;

	t_new[0].v0 = t.v0;
	t_new[0].v1 = t.v1;
	t_new[0].v2 = v_cent;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v_cent;

	t_new[2].v0 = t.v2;
	t_new[2].v1 = t.v0;
	t_new[2].v2 = v_cent;

	if (levels==0){
		for (j = 0; j < 3; j++){
			make_tri(t_new[j]);
		}
	}
}

//--------------------------------------------------------
void equi_1(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec3 v01, v12, v20;

	v01 = mix(t.v0, t.v1, .5);
	v12 = mix(t.v1, t.v2, .5);
	v20 = mix(t.v2, t.v0, .5);

	t_new[0].v0 = v01;
	t_new[0].v1 = v20;
	t_new[0].v2 = t.v0;

	t_new[1].v0 = v12;
	t_new[1].v1 = v01;
	t_new[1].v2 = t.v1;

	t_new[2].v0 = v20;
	t_new[2].v1 = v12;
	t_new[2].v2 = t.v2;

	t_new[3].v0 = v12;
	t_new[3].v1 = v20;
	t_new[3].v2 = v01;

	if (levels==0){
		for (j = 0; j < 4; j++){
			make_tri(t_new[j]);
		}
	}
	else{
		equi_2(levels - 1, t_new[0]);
		equi_2(levels - 1, t_new[1]);
		equi_2(levels - 1, t_new[2]);
		equi_2(levels - 1, t_new[3]);
	}
}

//--------------------------------------------------------
void equi_2(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec3 v01, v12, v20;

	v01 = mix(t.v0, t.v1, .5);
	v12 = mix(t.v1, t.v2, .5);
	v20 = mix(t.v2, t.v0, .5);

	t_new[0].v0 = v01;
	t_new[0].v1 = v20;
	t_new[0].v2 = t.v0;

	t_new[1].v0 = v12;
	t_new[1].v1 = v01;
	t_new[1].v2 = t.v1;

	t_new[2].v0 = v20;
	t_new[2].v1 = v12;
	t_new[2].v2 = t.v2;

	t_new[3].v0 = v12;
	t_new[3].v1 = v20;
	t_new[3].v2 = v01;

	if (levels==0){
		for (j = 0; j < 4; j++){
			make_tri(t_new[j]);
		}
	}
}
//--------------------------------------------------------
void equi_cent(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec3 v01, v12, v20;

	v01 = mix(t.v0, t.v1, .5);
	v12 = mix(t.v1, t.v2, .5);
	v20 = mix(t.v2, t.v0, .5);

	t_new[0].v0 = v01;
	t_new[0].v1 = v20;
	t_new[0].v2 = t.v0;

	t_new[1].v0 = v12;
	t_new[1].v1 = v01;
	t_new[1].v2 = t.v1;

	t_new[2].v0 = v20;
	t_new[2].v1 = v12;
	t_new[2].v2 = t.v2;

	t_new[3].v0 = v12;
	t_new[3].v1 = v20;
	t_new[3].v2 = v01;

	if (levels==0){
		for (j = 0; j < 4; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		centroid_1(levels - 1, t_new[0]);
		centroid_1(levels - 1, t_new[1]);
		centroid_1(levels - 1, t_new[2]);
		centroid_1(levels - 1, t_new[3]);
	}
}

//--------------------------------------------------------
void equi_bisect_1(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec3 v01, v12, v20;

	v01 = mix(t.v0, t.v1, .5);
	v12 = mix(t.v1, t.v2, .5);
	v20 = mix(t.v2, t.v0, .5);

	t_new[0].v0 = t.v0;
	t_new[0].v1 = v01;
	t_new[0].v2 = v20;

	t_new[1].v0 = v01;
	t_new[1].v1 = t.v1;
	t_new[1].v2 = v12;

	t_new[2].v0 = v20;
	t_new[2].v1 = v12;
	t_new[2].v2 = t.v2;

	t_new[3].v0 = v12;
	t_new[3].v1 = v20;
	t_new[3].v2 = v01;

	if (levels==0){
		for (j = 0; j < 4; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		equi_bisect_2(levels - 1, t_new[0]);
		equi_bisect_2(levels - 1, t_new[1]);
		equi_bisect_2(levels - 1, t_new[2]);
		equi_bisect_2(levels - 1, t_new[3]);
	}
}

//--------------------------------------------------------
void equi_bisect_2(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec3 v01, v12, v20;

	v01 = mix(t.v0, t.v1, .5);
	v12 = mix(t.v1, t.v2, .5);
	v20 = mix(t.v2, t.v0, .5);


	t_new[0].v0 = t.v0;
	t_new[0].v1 = v01;
	t_new[0].v2 = v20;

	t_new[1].v0 = v01;
	t_new[1].v1 = t.v1;
	t_new[1].v2 = v12;

	t_new[2].v0 = v20;
	t_new[2].v1 = v12;
	t_new[2].v2 = t.v2;

	t_new[3].v0 = v12;
	t_new[3].v1 = v20;
	t_new[3].v2 = v01;

	if (levels==0){
		for (j = 0; j < 4; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		bisect_5(levels - 1, t_new[0]);
		bisect_5(levels - 1, t_new[1]);
		bisect_5(levels - 1, t_new[2]);
		bisect_5(levels - 1, t_new[3]);
	}
}

//--------------------------------------------------------
void bisect_1(int levels, Triangle t){
	int i, j;
	Triangle t_new[2];
	vec3 v01;

	v01 = mix(t.v0, t.v1, .5);

	t_new[0].v0 = t.v2;
	t_new[0].v1 = t.v0;
	t_new[0].v2 = v01;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v01;

	if (levels==0){
		for (j = 0; j < 2; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		bisect_2(levels - 1, t_new[0]);
		bisect_2(levels - 1, t_new[1]);
	}
}

//--------------------------------------------------------
void bisect_2(int levels, Triangle t){
	int i, j;
	Triangle t_new[2];
	vec3 v01;

	v01 = mix(t.v0, t.v1, .5);

	t_new[0].v0 = t.v2;
	t_new[0].v1 = t.v0;
	t_new[0].v2 = v01;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v01;

	if (levels==0){
		for (j = 0; j < 2; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		bisect_3(levels - 1, t_new[0]);
		bisect_3(levels - 1, t_new[1]);
	}
}

//--------------------------------------------------------
void bisect_3(int levels, Triangle t){
	int i, j;
	Triangle t_new[2];
	vec3 v01;

	v01 = mix(t.v0, t.v1, .5);

	t_new[0].v0 = t.v2;
	t_new[0].v1 = t.v0;
	t_new[0].v2 = v01;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v01;

	if (levels==0){
		for (j = 0; j < 2; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		bisect_4(levels - 1, t_new[0]);
		bisect_4(levels - 1, t_new[1]);
	}
}

//--------------------------------------------------------
void bisect_4(int levels, Triangle t){
	int i, j;
	Triangle t_new[2];
	vec3 v01;

	v01 = mix(t.v0, t.v1, .5);

	t_new[0].v0 = t.v2;
	t_new[0].v1 = t.v0;
	t_new[0].v2 = v01;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v01;

	if (levels==0){
		for (j = 0; j < 2; j++){
			make_tri(t_new[j]);
		}
	}
	else{	
		bisect_5(levels - 1, t_new[0]);
		bisect_5(levels - 1, t_new[1]);
	}
}

//--------------------------------------------------------
void bisect_5(int levels, Triangle t){
	int i, j;
	Triangle t_new[2];
	vec3 v01;

	v01 = mix(t.v0, t.v1, .5);

	t_new[0].v0 = t.v2;
	t_new[0].v1 = t.v0;
	t_new[0].v2 = v01;

	t_new[1].v0 = t.v1;
	t_new[1].v1 = t.v2;
	t_new[1].v2 = v01;

	if (levels==0){
		for (j = 0; j < 2; j++){
			make_tri(t_new[j]);
		}
	}
}
