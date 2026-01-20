#version 330

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal;

out vec4 color;
out vec4 pos;
out vec3 normal;


uniform mat4 uProjMat;
uniform mat4 uModelMat;

void main()
{
	gl_Position  = uProjMat * (uModelMat * vPosition);
	gl_Position *= vec4(1,1,-1,1);	// z축 방향이 반대임

	pos = uModelMat * vPosition;							// in camera coord
	normal = normalize((uModelMat * vec4(vNormal,0)).xyz);	// in camera coord
	
	color = vColor;

	
}
