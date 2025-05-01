#version 460
layout(location = 0) out vec4 outFragColor; 

layout(location = 0) in vec2 UV0; 

layout(set = 0, binding = 0) uniform sampler2D texSampler;  

void main() {
    
   vec3 _color = texture(texSampler, UV0).rgb ;  
   outFragColor =  vec4(_color, 1.0);

}