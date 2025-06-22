
#version 460

layout(location=0)in vec3 inPos;
layout(location=1)in vec2 UV;
layout(location=2)in vec3 inNormal;
layout(location=3)in vec3 inTangent;

layout(location=0)out vec2 outUV0; 


struct Particle {
	vec3 position;
    float padding;
    vec3 velocity;
    float padding2;
};

layout(set = 0, binding = 0 ,std430) readonly buffer ParticleIn_t {
   Particle particlesIn[ ];
} ParticleIn;


void main()
{
    vec3 offset = ParticleIn.particlesIn[gl_InstanceIndex].position;

	gl_Position = vec4(inPos * 0.1 + offset * 2.0 ,1.0) ;

	outUV0 = UV; 
}

