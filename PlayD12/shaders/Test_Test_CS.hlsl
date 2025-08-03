

RWStructuredBuffer<uint> OutputBuffer : register(u0);

[numthreads(64, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    OutputBuffer[DTid.x] = DTid.x * 2;
}