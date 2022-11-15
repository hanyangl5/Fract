#include <rhi/device.h>
#include <utils/window/Window.h>
#include <utils/renderdoc/RenderDoc.h>
using namespace Fract;

int main() {

    Memory::initialize();

    Fract::Device device;
    device.Initialize();
    Fract::Window *window = new Fract::Window("fract", 1280, 800);
    SwapChain *swap_chain =
        device.CreateSwapChain(SwapChainCreateInfo{2}, window);

    auto buffer = device.CreateBuffer(
        BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER,
                         ResourceState::RESOURCE_STATE_SHADER_RESOURCE, 32},
        MemoryFlag::DEDICATE_GPU_MEMORY);


    // device.CreateComputePipeline();

    Shader *cs = device.CreateShader(ShaderType::COMPUTE_SHADER, 0,
                                     "C:/FILES/Ori/data/test.comp.hlsl");
    Pipeline *compute_pass =
        device.CreateComputePipeline(ComputePipelineCreateInfo{});
    compute_pass->SetComputeShader(cs);
    // compute_pass->GetDescriptorSet();

    while (!window->ShouldClose()) {
        glfwPollEvents();
        device.AcquireNextFrame(swap_chain);
        for (u32 i = 0; i < 5; i++) {
            CommandList *cmd = device.GetCommandList(CommandQueueType::COMPUTE);
            cmd->BeginRecording();
            cmd->BindPipeline(compute_pass);
            cmd->Dispatch(1, 1, 1);
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


    //RDC::EndFrameCapture();



}
