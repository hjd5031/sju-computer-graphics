#version 330

uniform float uTime;

uniform bool uWavePlay;

in vec4 vPosition;
in vec4 vColor;

out vec4 color;
out vec4 position;

void main()
{
	vec4 temp = vPosition;
	float r = sqrt(temp.x*temp.x + temp.y*temp.y);             //distance from origin
	float maxRadius = 1.8f;                 //max distance of wave
	float amplitude = (0.4 - r / maxRadius);//amplitude proportional to distance from origin
	if(amplitude<0)amplitude = 0.0f;

	
	float freq = 5.0;   //frequency
	float k = 20.0;     //

	if(uWavePlay)
		temp.z = (amplitude) * sin(uTime * freq -  r* k);
	
	float angle1 = uTime * 45.0f;
	float angle2 = 225.0f;
	float rad1 = angle1 / 180.0f * 3.141592f;
	float rad2 = angle2 / 180.0f * 3.141592f;
	
	mat4 rot1 = mat4(1);
	mat4 rot2 = mat4(1);
	/*
	rot[i][j] : i = col, j = row : col-major
	rot[0][0];		rot[1][0];		rot[2][0];		rot[3][0];
	rot[0][1];		rot[1][1];		rot[2][1];		rot[3][1];
	rot[0][2];		rot[1][2];		rot[2][2];		rot[3][2];
	rot[0][3];		rot[1][3];		rot[2][3];		rot[3][3];
	*/
	rot1[0][0] = cos(rad1);		rot1[1][0] = -sin(rad1);//matrix for z-axis rot
	rot1[0][1] = sin(rad1);		rot1[1][1] = cos(rad1);

	rot2[1][1] = cos(rad2);	rot2[2][1] = -sin(rad2);//matrix for x-axis rot
	rot2[1][2] = sin(rad2);	rot2[2][2] = cos(rad2);

	
	
	vec4 pos = rot2 * rot1 * temp;

	color = vColor;

	position = temp; //send the model's coordinate to fshader

	gl_Position = pos;
}
