cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 padding[15];
};
[numthreads(8, 8, 1)]
void Main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{

}