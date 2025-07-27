


struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};


VS_OUT VSMain(uint vertexID : SV_VertexID)
{
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    VS_OUT o;
    o.uv = uv;
    o.pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return o;
}