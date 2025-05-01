

layout(std140, set = 0, binding = 0) uniform GlobalUniforms_t {
    vec2 iResolution;
    float iTime;
    float iTimeDelta;  
	int iFrame;
    vec4 iMouse;
    vec2 iChannelResolution[4];
} GlobalUniforms;
 

layout(rgba32f, set=0, binding = 1) uniform image2D outputImage;

// Define macros to make variables "directly accessible"
#define iResolution GlobalUniforms.iResolution
#define iTime GlobalUniforms.iTime
#define iTimeDelta GlobalUniforms.iTimeDelta  
#define iMouse GlobalUniforms.iMouse
#define iFrame GlobalUniforms.iFrame

#define iChannelResolution GlobalUniforms.iChannelResolution


 
layout(set = 1, binding = 0) uniform sampler2D iChannel0;
layout(set = 1, binding = 1) uniform sampler2D iChannel1;
layout(set = 1, binding = 2) uniform sampler2D iChannel2;
layout(set = 1, binding = 3) uniform sampler2D iChannel3;  




// Redirect logic from mainImage()
void mainImage(out vec4 fragColor, in vec2 fragCoord); 

layout(local_size_x = 16, local_size_y = 16 , local_size_z = 1) in;
void main()
{ 
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy); 
    if (pixelCoords.x >= iResolution.x || pixelCoords.y >= iResolution.y)
        return;
         
    vec4 fragColor;
    mainImage(fragColor, vec2(pixelCoords.x, iResolution.y - pixelCoords.y));
    // mainImage(fragColor, vec2(pixelCoords.x, pixelCoords.y));

    imageStore(outputImage, pixelCoords,  fragColor); 
}

 

