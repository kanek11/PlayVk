cbuffer MVPConstantBuffer : register(b0)
{
    float4x4 modelMatrix; // 64 bytes
    float4x4 pvMatrix; // 64 bytes
    float4 padding[32]; 
};
// Total: 256 bytes

struct VSInput
{
    float4 position : POSITION; // Per-vertex
    float3 normal : NORMAL;
    float4 color : COLOR; // Per-vertex
    float2 texCoord : TEXCOORD; // Per-vertex
    float3 instanceOffset : INSTANCE_OFFSET; // Per-instance
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};
  

PSInput VSMain(VSInput input)
{
    PSInput result;

    float4 worldPosition = float4(input.position.xyz + input.instanceOffset, 1.0f);
    result.position = mul(pvMatrix, mul(modelMatrix, worldPosition));
    result.color = input.color;

    //result.normal = normalize(mul((float3x3) vpMatrix, input.normal.xyz));
    //we don't have model rotation, so we can just use the normal as is
    result.normal = input.normal;
    result.texCoord = input.texCoord;

    return result;
}


