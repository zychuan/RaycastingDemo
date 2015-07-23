#version 330 core

layout(location = 0) out vec4 vFragColor;

smooth in vec3 vUV;

uniform sampler3D	volume;
uniform vec3		camPos;
uniform vec3		step_size;

const int MAX_SAMPLES = 300;
const vec3 texMin = vec3(0);
const vec3 texMax = vec3(1);
const float DELTA = 0.01;
const float isoValue = 1/255.0;

vec3 Bisection(vec3 left, vec3 right , float iso)
{ 
	for(int i=0;i<4;i++)
	{ 
		vec3 midpoint = (right + left) * 0.5;
		float cM = texture(volume, midpoint).x ;
		if(cM < iso)
			left = midpoint;
		else
			right = midpoint; 
	}
	return vec3(right + left) * 0.5;
}

vec3 GetGradient(vec3 uvw) 
{
	vec3 s1, s2;  

	s1.x = texture(volume, uvw-vec3(DELTA,0.0,0.0)).x ;
	s2.x = texture(volume, uvw+vec3(DELTA,0.0,0.0)).x ;

	s1.y = texture(volume, uvw-vec3(0.0,DELTA,0.0)).x ;
	s2.y = texture(volume, uvw+vec3(0.0,DELTA,0.0)).x ;

	s1.z = texture(volume, uvw-vec3(0.0,0.0,DELTA)).x ;
	s2.z = texture(volume, uvw+vec3(0.0,0.0,DELTA)).x ;
	 
	return normalize((s1-s2)/2.0); 
}

vec4 PhongLighting(vec3 L, vec3 N, vec3 V, float specPower, vec3 diffuseColor)
{
	float diffuse = max(dot(L,N),0.0);
	vec3 halfVec = normalize(L+V);
	float specular = pow(max(0.00001,dot(halfVec,N)),specPower);	
	return vec4((diffuse*diffuseColor + specular),1.0);
}

void main()
{ 
	vec3 dataPos = vUV;
	vec3 geomDir = normalize((vUV-vec3(0.5)) - camPos); 
	vec3 dirStep = geomDir * step_size; 
	bool stop = false; 

	for (int i = 0; i < MAX_SAMPLES; i++) {
		dataPos = dataPos + dirStep;
		stop = dot(sign(dataPos-texMin), sign(texMax-dataPos)) < 3.0;	
		if (stop) 
			break;
		float sample = texture(volume, dataPos).r;
		float sample2 = texture(volume, dataPos+dirStep).r;

		if(((sample - isoValue) <= 0.0 && (sample2 - isoValue) >= 0.0))  {
			vec3 xN = dataPos;
			vec3 xF = dataPos+dirStep;	
			vec3 tc = Bisection(xN, xF, isoValue);
			vec3 N = GetGradient(tc);
			vec3 V = -geomDir;
			vec3 L =  V;
			vFragColor =  PhongLighting(L, N, V, 250, vec3(0.5));	
			break;
		}
		if ( (sample - isoValue) >= 0.0 && (sample2 - isoValue) >= 0.0 ) {
			vec3 xN = dataPos-dirStep;
			vec3 xF = dataPos;	
			vec3 tc = Bisection(xN, xF, isoValue);
			vec3 N = GetGradient(tc);
			vec3 V = -geomDir;
			vec3 L =  V;
			vFragColor =  PhongLighting(L, N, V, 250, vec3(0.5));	
			break;
		}
	} 
}