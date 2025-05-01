#version 460

layout(location = 0) in vec3 fragColor; 
layout(location = 0) out vec4 outColor; 

layout(set = 0, binding = 1) uniform sampler2D texSampler;

void main() {
  
   vec3 _color = texture(texSampler, fragColor.xy).rgb ; //* fragColor;

   //encode gamma
    outColor =  pow(vec4(_color, 1.0), vec4(1.0/2.2));

   //outColor = vec4(_color, 1.0);  
   //outColor = vec4(fragColor, 1.0);
}