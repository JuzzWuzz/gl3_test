#version 150 core
#extension GL_ARB_gpu_shader5 : enable

// Declare the incoming primitive type
layout(triangles) in;

// Declare the resulting primitive type
layout(triangles, max_vertices=192)  out;

// Uniforms (from host code)
uniform mat4 mvpMatrix;

// Incoming from vertex shader
in vec4 geom_Color[3];
in vec4 geom_Normal[3];

// Outgoing per-vertex information
out vec4 interpColor;

struct Triangle{
	vec4 v0, v1, v2;
	vec4 n0, n1, n2;
};


void equi_1(int levels, Triangle t);
void equi_2(int levels, Triangle t);
void equi_cent(int levels, Triangle t);
void centroid_1(int levels, Triangle t);
void centroid_2(int levels, Triangle t);
void centroid_3(int levels, Triangle t);
vec4 lerpVert(vec3 bary, Triangle parent);



//--------------------------------------------------------
void main(){
	int levels =3;
	int i;
	Triangle t;

	if (levels == 0){
		for (i = 0; i < gl_in.length(); i++){
			gl_Position = mvpMatrix * gl_in[i].gl_Position;
			interpColor = geom_Color[i];
			EmitVertex();
		}
		EndPrimitive();
	}
	else{
		t.v0 = mvpMatrix * gl_in[0].gl_Position;
		t.v1 = mvpMatrix * gl_in[1].gl_Position;
		t.v2 = mvpMatrix * gl_in[2].gl_Position;
		
		equi_cent(levels - 1, t);
	}
}


//--------------------------------------------------------
void centroid_1(int levels, Triangle t){
	int i, j;
	Triangle t_new[3];
	vec4 v_cent;

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
			gl_Position = t_new[j].v0;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v1;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v2;
			interpColor = geom_Color[0];
			EmitVertex();
			EndPrimitive();
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
	vec4 v_cent;

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
			gl_Position = t_new[j].v0;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v1;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v2;
			interpColor = geom_Color[0];
			EmitVertex();
			EndPrimitive();
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
	vec4 v_cent;

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
			gl_Position = t_new[j].v0;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v1;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v2;
			interpColor = geom_Color[0];
			EmitVertex();
			EndPrimitive();
		}
	}
}

//--------------------------------------------------------
void equi_1(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec4 v01, v12, v20;

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
			gl_Position = t_new[j].v0;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v1;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v2;
			interpColor = geom_Color[0];
			EmitVertex();
			EndPrimitive();
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
	vec4 v01, v12, v20;

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
			gl_Position = t_new[j].v0;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v1;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v2;
			interpColor = geom_Color[0];
			EmitVertex();
			EndPrimitive();
		}
	}
}

//--------------------------------------------------------
void equi_cent(int levels, Triangle t){
	int i, j;
	Triangle t_new[4];
	vec4 v01, v12, v20;

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
			gl_Position = t_new[j].v0;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v1;
			interpColor = geom_Color[0];
			EmitVertex();
			gl_Position = t_new[j].v2;
			interpColor = geom_Color[0];
			EmitVertex();
			EndPrimitive();
		}
	}
	else{
		centroid_1(levels - 1, t_new[0]);
		centroid_1(levels - 1, t_new[1]);
		centroid_1(levels - 1, t_new[2]);
		centroid_1(levels - 1, t_new[3]);
	}
}
