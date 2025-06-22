#version 460

layout(location=0)in vec2 inPos;
layout(location=1)in vec3 inColor;  
layout(location=2)in vec2 inTexCoords;
 
layout(location=3)in vec2 inInstancePos;

layout(location=0)out vec3 fragColor;


layout(set = 0,binding=0)uniform UniformBufferObject{
    mat4 model;
}ubo;


layout(push_constant) uniform PushConstants {
    float time;
} pushConstants;


void main(){


    vec4 _position = ubo.model *  vec4 ( (inPos + inInstancePos) , 0.0, 1.0);
    _position.x += sin(pushConstants.time) * 0.5;

    gl_Position = _position;
    //fragColor=inColor;
    fragColor= vec3(inTexCoords, 0.0);
}
