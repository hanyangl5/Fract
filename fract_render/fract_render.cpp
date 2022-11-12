#include <rhi/device.h>
#include <utils/window/Window.h>

using namespace Fract;
int main() {

    Memory::initialize();

    Fract::Device device;
    device.Initialize();
    Fract::Window *window = new Fract::Window("fract", 1280, 800);
    SwapChain *swap_chain =
        device.CreateSwapChain(SwapChainCreateInfo{2}, window);
    
    auto buffer = device.CreateBuffer(BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER, ResourceState::RESOURCE_STATE_SHADER_RESOURCE,32},MemoryFlag::DEDICATE_GPU_MEMORY);

                    TextureCreateInfo create_info{};

    create_info.width = 64;
                    create_info.height = 64;

    create_info.depth = 1;
    create_info.texture_format =
        TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM; // TOOD: optimize format
    create_info.texture_type = TextureType::TEXTURE_TYPE_2D; // TODO: cubemap?
    create_info.descriptor_types = DescriptorType::DESCRIPTOR_TYPE_TEXTURE;
    create_info.initial_state = ResourceState::RESOURCE_STATE_SHADER_RESOURCE;
    create_info.enanble_mipmap = false;

    auto texture = device.CreateTexture(create_info);

    //device.CreateComputePipeline();

    CommandList *cmd = device.GetCommandList(CommandQueueType::GRAPHICS);
    
    cmd->BeginRecording();
    cmd->Dispatch(1, 1, 1);
    cmd->EndRecording();
    

    QueueSubmitInfo submit_info{};
    submit_info.command_lists.push_back(cmd);

    device.SubmitCommandLists(submit_info);
    while (!window->ShouldClose()) {
        glfwPollEvents();

    }

}
