
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float TangentSign : TANGENT_SIGN;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR;
    float3 Offset : INSTANCE_OFFSET;
};


struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 WorldNormal : TEXCOORD1;
    float3 WorldTangent : TEXCOORD2;
    float TangentSign : TEXCOORD3;
    float2 UV : TEXCOORD4;
};



struct GBufferOutput
{
    float4 rt0_albedo_ao : SV_Target0; //R8G8B8A8_UNORM
    float4 rt1_normal_rough : SV_Target1; //R16G16B16A16_FLOAT
    float4 rt2_metallic_misc : SV_Target2; //R16G16B16A16_FLOAT
};
