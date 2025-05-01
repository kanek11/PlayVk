
#version 460

layout(location=0)in vec3 inPos;
layout(location=1)in vec2 inUV;
layout(location=2)in vec3 inNormal;
layout(location=3)in vec3 inTangent;

layout(location=0)out vec2 outUV0;  
 

void main()
{  
	gl_Position = vec4(inPos , 1.0) ;

	outUV0 = inUV;
}

