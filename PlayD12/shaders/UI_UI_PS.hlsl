 
 
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

float4 PSMain(PSInput input) : SV_TARGET
{ 
    return float4(input.color);  
}