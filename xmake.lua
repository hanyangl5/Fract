
-- -- add_requires("vcpkg::directx-dxc")
-- add_requires("vcpkg::d3d12-memory-allocator")
-- -- add_requires("vcpkg::directxmath")
-- -- add_requires("vcpkg::directxtk12")
-- -- add_requires("vcpkg::assimp")
-- add_requires("vcpkg::glfw3")

target("fract_lib") 
    set_kind("static") 
    add_files("fract/camera/*.cpp")
    add_files("fract/geometry/*.cpp")
    add_files("fract/light/*.cpp")
    add_files("fract/materials/*.cpp")
    add_files("fract/ray/*.cpp")
    add_files("fract/rhi/*.cpp")
    add_files("fract/sampling/*.cpp")

target("fract_render")
    set_kind("binary") 
    add_files("fract_render/fract_render.cpp")
    add_deps("fract_lib")