#version 460
layout(location = 0) out vec4 outFragColor; 

layout(location = 0) in vec2 UV0; 

layout(set = 1, binding = 0) uniform sampler2D Sampler0; 


void main() {
    
   vec3 _color = texture(Sampler0, UV0).rgb ; //* fragColor;
   outFragColor =  vec4(_color, 1.0); 

   //consider alpha
   //vec4 _color = texture(Sampler0, UV0).rgba ; 
   //outFragColor =  vec4(_color); 
}