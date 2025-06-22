
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


layout(set = 0, binding = 0 ,std140) uniform CameraUBO_t{
	mat4 view;
	mat4 proj;  
} CameraUBO;
 
 layout(push_constant) uniform PushConstants_t {
    mat4 model; 
} PushConstants;


void main()
{ 

    vec4 _Position = vec4(inPos.x, inPos.y, inPos.z, 1.0);
	gl_Position = CameraUBO.proj * CameraUBO.view * PushConstants.model * _Position;  
	//gl_Position = vec4(inPos + vec3(0.0f,-1.0f,0.0f) , 1.0);

	outUV0 = UV; 
}

