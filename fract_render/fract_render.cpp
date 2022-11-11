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
