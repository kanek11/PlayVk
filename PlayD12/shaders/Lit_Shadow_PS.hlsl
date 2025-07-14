
struct VSOutput
{
    float4 position : SV_POSITION;
};


float4 PSMain(VSOutput input) : SV_TARGET
{ 
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}