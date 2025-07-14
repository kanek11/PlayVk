struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
    //return float4(float3(1.0f, 0.0f, 0.0f), 1.0);
}