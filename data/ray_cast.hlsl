struct T{
    float4 data[16];
};

cbuffer RenderConstant : register(b0)
{
    uint width;
    uint height;
    uint spp;
    uint pad0;
};

// cbuffer SceneConstant: register(b1)
// {

//     float4 ray_origin;
//     float4 ray_direction;
//     float4 sphere_center_radius;
// };

struct Ray{
    float3 origin;
    float pad0;
    float3 direction;
    float pad1;
};

float3 ray_color(Ray r) {
    float3 unit_direction = normalize(r.direction);
    float t = 0.5*(unit_direction.y + 1.0);
    return (1.0-t)*float3(1.0, 1.0, 1.0) + t*float3(0.5, 0.7, 1.0);
}


// SamplerState sampler : register(s0);

RWStructuredBuffer<T> abc : register(u0);

RWTexture2D<float4> color_texture : register(u1);

[numthreads(1, 1, 1)]
void Main(uint3 thread_group_id : SV_GroupID, uint3 thread_id : SV_DispatchThreadID, uint3 group_thread_id : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    float focal_length = 1.0;

    float3 origin = float3(0, 0, 0);
    float3 horizontal = float3(width, 0, 0);
    float3 vertical = float3(0, height, 0);
    float3 lower_left_corner = origin - horizontal/2 - vertical/2 - float3(0, 0, focal_length);

    float u = float(thread_id.x) / (width-1);
    float v = float(thread_id.y) / (height-1);

    Ray r;
    r.origin = origin;
    r.direction = lower_left_corner + u*horizontal + v*vertical - origin;

    float3 pixel_color = ray_color(r);
    color_texture[thread_id.xy].xyz = pixel_color;
}