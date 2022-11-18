cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
};

struct T{
    float4 data;
};

RWStructuredBuffer<T> abc : register(u0);

[numthreads(8, 8, 1)]
void Main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
abc[0].data = offset;
}