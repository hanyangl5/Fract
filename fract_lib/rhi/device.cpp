/*****************************************************************//**
 * \file   device.cpp
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#include "device.h"

#include <string>

#include <utils/log/log.h>

namespace Fract {

thread_local CommandContext *thread_command_context;

Device::Device() noexcept {}

Device::~Device() noexcept {}

void Device::Initialize() {
    CreateFactory();
    CreateDevice();
    InitializeD3DMA();
}

void Device::CreateFactory() noexcept {
    u32 dxgi_factory_flags = 0;
#ifndef NDEBUG
    dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    CHECK_DX_RESULT(CreateDXGIFactory2(dxgi_factory_flags,
                                       IID_PPV_ARGS(&render_context.factory)));
}

void Device::PickGPU(IDXGIFactory6 *factory, IDXGIAdapter4 **gpu) noexcept {
    u32 gpu_count = 0;
    IDXGIAdapter4 *adapter = NULL;
    IDXGIFactory6 *factory6;

    CHECK_DX_RESULT(factory->QueryInterface(IID_PPV_ARGS(&factory6)))

    // Find number of usable GPUs
    // Use DXGI6 interface which lets us specify gpu preference so we dont need
    // to use NVOptimus or AMDPowerExpress exports
    for (u32 i = 0;
         DXGI_ERROR_NOT_FOUND !=
         factory6->EnumAdapterByGpuPreference(
             i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
         ++i) {
        DXGI_ADAPTER_DESC3 desc;
        adapter->GetDesc3(&desc);
        if (SUCCEEDED(D3D12CreateDevice(adapter, DX12_API_VERSION,
                                        _uuidof(ID3D12Device), nullptr))) {
            break;
        }
    }
    *gpu = adapter;
}

void Device::CreateDevice() noexcept {

    // pick gpu
    PickGPU(render_context.factory, &render_context.active_gpu);

    CHECK_DX_RESULT(D3D12CreateDevice(render_context.active_gpu,
                                      DX12_API_VERSION,
                                      IID_PPV_ARGS(&render_context.device)));

    // create command queue

    D3D12_COMMAND_QUEUE_DESC graphics_command_queue_desc{};
    graphics_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    graphics_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    CHECK_DX_RESULT(render_context.device->CreateCommandQueue(
        &graphics_command_queue_desc,
        IID_PPV_ARGS(&render_context.queues[CommandQueueType::GRAPHICS])));

#ifdef USE_ASYNC_COMPUTE_TRANSFER
    D3D12_COMMAND_QUEUE_DESC compute_command_queue_desc{};
    compute_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    compute_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

    CHECK_DX_RESULT(render_context.device->CreateCommandQueue(
        &compute_command_queue_desc,
        IID_PPV_ARGS(&render_context.queues[CommandQueueType::COMPUTE])));

    D3D12_COMMAND_QUEUE_DESC transfer_command_queue_desc{};
    transfer_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    transfer_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

    CHECK_DX_RESULT(render_context.device->CreateCommandQueue(
        &transfer_command_queue_desc,
        IID_PPV_ARGS(&render_context.queues[CommandQueueType::TRANSFER])));
    LOG_INFO("using async compute & transfer");
#endif // USE_ASYNC_COMPUTE_TRANSFER
}

void Device::InitializeD3DMA() noexcept {
    D3D12MA::ALLOCATOR_DESC allocatorDesc{};
    allocatorDesc.pDevice = render_context.device;
    allocatorDesc.pAdapter = render_context.active_gpu;

    CHECK_DX_RESULT(D3D12MA::CreateAllocator(&allocatorDesc,
                                             &render_context.d3dma_allocator));
}

Buffer *Device::CreateBuffer(const BufferCreateInfo &buffer_create_info,
                             MemoryFlag memory_flag) {
    return Memory::Alloc<Buffer>(render_context, buffer_create_info, memory_flag);
}

Texture *Device::CreateTexture(const TextureCreateInfo &texture_create_info) {
    return Memory::Alloc<Texture>(render_context, texture_create_info);
}

SwapChain *
Device::CreateSwapChain(const SwapChainCreateInfo& swap_chain_create_info,
                        Window *window) {
    return new SwapChain(render_context, swap_chain_create_info, window);
}

CommandList *Device::GetCommandList(CommandQueueType type) {
    if (!thread_command_context) {
        thread_command_context = Memory::Alloc<CommandContext>(render_context);
    }
    return thread_command_context->GetCommandList(type);
}

Pipeline *
Device::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo &create_info) {
    return nullptr;
}

Pipeline *
Device::CreateComputePipeline(const ComputePipelineCreateInfo &create_info) {
    return nullptr;
    //render_context.device->CreateComputePipelineState();
}

void Device::SubmitCommandLists(const QueueSubmitInfo &queue_submit_info) {
    auto stack_memory = Memory::GetStackMemoryResource(4096);
    Container::Array<ID3D12CommandList *> command_lists{};
    for (auto &cmd : queue_submit_info.command_lists) {
        command_lists.push_back(cmd->get());
    }
    render_context.queues[queue_submit_info.queue_type]->ExecuteCommandLists(
        static_cast<u32>(command_lists.size()), command_lists.data());
}

void Device::WaitAll() {}

void Device::WaitGpuExecution(CommandQueueType type) {
    // render_context.queues[type]->Signal();
    //auto command_queue = render_context.queues[type];
    //command_queue->Signal(fence, fence_value[current_back_buffer]);

    //hres = fence->SetEventOnCompletion(fence_value[current_back_buffer],
    //                                   fence_event);

    //WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
    //fence_value[current_back_buffer]++;
}

RenderTarget *Device::CreateRenderTarget(
    const RenderTargetCreateInfo &render_target_create_info) {
    return Memory::Alloc<RenderTarget>(render_context,
                                       render_target_create_info);
}


} // namespace Fract