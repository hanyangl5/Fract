#include <rhi/device.h>
#include <utils/renderdoc/RenderDoc.h>
#include <utils/window/Window.h>

using namespace Fract;

u32 RoundUp(f32 x) { return static_cast<u32>(std::ceil(x)); }

int main() {
    u32 width = 800;
    u32 height = 600;
    u32 worp_group_size_x = RoundUp((float(width) / 8));
    u32 worp_group_size_y = RoundUp((float(600) / 8));
    Memory::initialize();

    Fract::Device device;
    device.Initialize();
    Fract::Window *window = new Fract::Window("fract", width, height);
    SwapChain *swap_chain =
        device.CreateSwapChain(SwapChainCreateInfo{2}, window);

    Shader *cs = device.CreateShader(ShaderType::COMPUTE_SHADER, 0,
                                     "C:/Fract/data/test.comp.hlsl");
    Pipeline *compute_pass =
        device.CreateComputePipeline(ComputePipelineCreateInfo{});
    compute_pass->SetComputeShader(cs);
    // compute_pass->GetDescriptorSet();

    Texture *render_target = device.CreateTexture(TextureCreateInfo{DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE,ResourceState::RESOURCE_STATE_RENDER_TARGET,TextureType::TEXTURE_TYPE_2D,TextureFormat::TEXTURE_FORMAT_RGBA8_UNORM,width, height});
    Buffer *cb = device.CreateBuffer(
        BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                         ResourceState::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 16},MemoryFlag::CPU_VISABLE_MEMORY);


    f32 cb_data[4] = {1,2,3,4};

    UINT8 *data{};

    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is
    // okay.
    CD3DX12_RANGE readRange(
        0, 0); // We do not intend to read from this resource on the CPU.
    CHECK_DX_RESULT(cb->GetResource()->Map(0, &readRange,
                                           reinterpret_cast<void **>(&data)));
    memcpy(data, &cb_data, sizeof(16));
    cb->GetResource()->Unmap(0, nullptr);

    while (!window->ShouldClose()) {
        glfwPollEvents();
        device.AcquireNextFrame(swap_chain);
        for (u32 i = 0; i < 5; i++) {
            CommandList *cmd = device.GetCommandList(CommandQueueType::COMPUTE);
            cmd->BeginRecording();
            cmd->BindPipeline(compute_pass);
            cmd->Dispatch(worp_group_size_x, worp_group_size_y, 1);
            cmd->EndRecording();
            QueueSubmitInfo submit_info{};
            submit_info.command_lists.push_back(cmd);
            submit_info.queue_type = CommandQueueType::COMPUTE;
            device.SubmitCommandLists(submit_info);
        }

        QueuePresentInfo present_info{};
        present_info.swap_chain = swap_chain;
        device.Present(present_info);
        device.WaitGpuExecution(CommandQueueType::COMPUTE);
    }

    // RDC::EndFrameCapture();
}
