

uniform mat4 mvpMatrix;
in vec3 in_Color;
in vec3 in_Position;

varying vec4 color;

void main(){
	gl_Position = gl_ModelViewProjectionMatrix * vec4(in_Position, 1.0);
	//gl_Position = vec4(in_Position, 1.0) * mvpMatrix;
	color=vec4(in_Color, 1.0);	
}

