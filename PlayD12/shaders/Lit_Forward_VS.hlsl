// Scene-level (b0)
cbuffer SceneCB : register(b0, space0)
{
    float4x4 PVMatrix;
    float4x4 LightPVMatrix;
    float3 LightDirection;  
    float3 LightColor;
    
    float padding[26]; 
};

// Object-level (b1)
cbuffer ObjectCB : register(b1, space0)
{
    float4x4 ModelMatrix;
};
 
//cbuffer LightCB : register(b2, space0)
//{ 
//    float4x4 LightPVMatrix;
//    float3 LightDirection;  
//    float3 LightColor;
//    //float ShadowBias; 
//    //float Padding;  
//};


struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 Offset : INSTANCE_OFFSET;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    
    
    float4 lightSpacePos : TEXCOORD1;
    
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(ModelMatrix, float4(input.Position + input.Offset, 1.0f));
    output.position = mul(PVMatrix, worldPos);
    output.texCoord = input.TexCoord;
    output.normal = normalize(mul((float3x3) ModelMatrix, input.Normal));
    output.color = input.Color;
     
    output.lightSpacePos = mul(LightPVMatrix, worldPos);
    
    return output;
}

 