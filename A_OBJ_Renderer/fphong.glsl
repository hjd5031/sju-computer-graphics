#version 330

in  vec4 color;
in  vec4 pos;
in  vec3 normal;

out vec4 fColor;

uniform float uShininess;
uniform vec4 uSpecularEffect;

void
main()
{
	vec4 LPos = vec4(2, 2, 2,1);			// in camera coord
	vec4 LColor = vec4(1,1,1,1);
	vec4 ka = color*0.1;
	vec4 kd = color;
	vec4 ks = uSpecularEffect;
	float sh = uShininess;
	   	 
	vec4 Amb = ka * LColor;
	
	vec3 L3 = normalize((LPos - pos).xyz);
	vec3 N3 = normalize(normal);
	float NL = max(dot(L3, N3),0);
	
	vec4 Diff = kd * LColor * NL;
	
	vec3 R3 = normalize(2* dot(N3, L3) * N3 - L3);
	vec3 V3 = normalize((vec4(0,0,0,1) - pos).xyz);	// camera is located at (0,0,0)
	float RV = pow(max(dot(R3, V3),0), sh);	// (R V)^n

	vec4 Spec = ks*LColor*RV;
	
	fColor = Amb + Diff + Spec;

}
