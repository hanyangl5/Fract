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

// cbuffer CameraConstant: register(b1)
// {
//     float4 eye_pos;
//     float4 at;
//     float4 fov_aspect_ratio;
// point3 origin;
// point3 lower_left_corner;
// vec3 horizontal;
// vec3 vertical;
// };

struct Ray{
    float3 origin;
    float pad0;
    float3 direction;
    float pad1;
};

bool hit_sphere(const float3 center, float radius, const Ray r) {
    float3 oc = r.origin - center;
    float a = dot(r.direction, r.direction);
    float b = 2.0 * dot(oc, r.direction);
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    return (discriminant > 0);
}

float3 ray_color(Ray r) {
    if (hit_sphere(float3(0,0,-1), 0.5, r))
        return float3(1, 0, 0);
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
    float aspect_ratio = float(width)/float(height);
    float viewport_height = 2.0;
    float viewport_width = viewport_height * aspect_ratio;

    float3 origin = float3(0, 0, 0);
    float3 horizontal = float3(viewport_width, 0, 0);
    float3 vertical = float3(0, viewport_height, 0);
    float3 lower_left_corner = origin - horizontal/2 - vertical/2 - float3(0, 0, focal_length);

            //     auto theta = degrees_to_radians(vfov);
            // auto h = tan(theta/2);
            // auto viewport_height = 2.0 * h;
            // auto viewport_width = aspect_ratio * viewport_height;

            // auto w = unit_vector(lookfrom - lookat);
            // auto u = unit_vector(cross(vup, w));
            // auto v = cross(w, u);

            // origin = lookfrom;
            // horizontal = viewport_width * u;
            // vertical = viewport_height * v;
            // lower_left_corner = origin - horizontal/2 - vertical/2 - w;

    float u = float(thread_id.x) / (width-1);
    float v = float(thread_id.y) / (height-1);

    Ray r;
    r.origin = origin;
    r.direction = lower_left_corner + u*horizontal + v*vertical - origin;
    r.direction = normalize(r.direction);
    float3 pixel_color = ray_color(r);
    color_texture[thread_id.xy].xyz = pixel_color;
}
