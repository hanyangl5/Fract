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
    CreateShaderCompiler();
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

void Device::CreateShaderCompiler() {
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&render_context.dxc_utils));
    DxcCreateInstance(CLSID_DxcCompiler,
                      IID_PPV_ARGS(&render_context.dxc_compiler));
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

Shader *Device::CreateShader(ShaderType type, u32 compile_flags,
                             const std::filesystem::path &file_name) {
    return Memory::Alloc<Shader>(render_context, type, file_name);
}

Pipeline *
Device::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo &create_info) {
    return nullptr;
}

Pipeline *
Device::CreateComputePipeline(const ComputePipelineCreateInfo &create_info) {
    return Memory::Alloc<Pipeline>(render_context, create_info,
                                   *descriptor_set_allocator);
}

void Device::SubmitCommandLists(const QueueSubmitInfo &queue_submit_info) {
    auto stack_memory = Memory::GetStackMemoryResource(4096);
    Container::Array<ID3D12CommandList *> command_lists{};
    for (auto &cmd : queue_submit_info.command_lists) {
        command_lists.push_back(cmd->get());
    }
    render_context.queues[queue_submit_info.queue_type]->ExecuteCommandLists(
        static_cast<u32>(command_lists.size()), command_lists.data());

    GetFence(queue_submit_info.queue_type);
}

void Device::WaitAll() {}

void Device::WaitGpuExecution(CommandQueueType type) {
    
    assert(fence_index[type] != UINT_MAX); // no need to wait twice
    if (fence_index[type] == 0)
        return;
    
    for (u32 i = 0; i < fence_index[type]; ++i) {
        fences[type][i]->Signal();
    }
    fence_index[type] = UINT_MAX;
}

void Device::Present(const QueuePresentInfo &present_info) {
    present_info.swap_chain->get()->Present(1, 0);
}

void Device::AcquireNextFrame(SwapChain *swap_chain) {
    swap_chain->AcquireNextFrame();

    for (u32 queue_type = 0; queue_type < 3; queue_type++) {
        if (fence_index[queue_type] == 0)
            continue;
        fence_index[queue_type] = 0;
        for (auto &fence : fences[queue_type]) {
            Memory::Free(fence);
            fence = nullptr;
        }
        fences[queue_type].clear();
    }

    if (thread_command_context) {
        thread_command_context->Reset();
    }
}

Fence *Device::GetFence(CommandQueueType type) {
    Fence *fence;
    if (fence_index[type] < fences[type].size()) {
        fence = fences[type][fence_index[type]];
    } else {
        fence = Memory::Alloc<Fence>(render_context);
        fences[type].push_back(fence);
    }
    fence_index[type]++;
    return fence;
}

RenderTarget *Device::CreateRenderTarget(
    const RenderTargetCreateInfo &render_target_create_info) {
    return Memory::Alloc<RenderTarget>(render_context,
                                       render_target_create_info);
}


} // namespace Fract