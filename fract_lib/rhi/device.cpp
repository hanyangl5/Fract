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
    return new Buffer(render_context, buffer_create_info, memory_flag);
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

Buffer::Buffer(const RendererContext &render_context,
               const BufferCreateInfo &buffer_create_info,
               MemoryFlag memory_flag) noexcept
    : m_context(render_context) {
    m_size = buffer_create_info.size;
    m_descriptor_types = buffer_create_info.descriptor_types;
    m_resource_state = buffer_create_info.initial_state;
    D3D12_RESOURCE_FLAGS usage{}; // TODO:
    // Alignment must be 64KB (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) or 0,
    // which is effectively 64KB.
    auto _buffer_create_info = CD3DX12_RESOURCE_DESC::Buffer(
        buffer_create_info.size, usage,
        D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

    D3D12MA::ALLOCATION_DESC allocation_desc{};
    D3D12_RESOURCE_STATES initial_state{};
    if (memory_flag == MemoryFlag::CPU_VISABLE_MEMORY) {
        allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        initial_state = D3D12_RESOURCE_STATE_GENERIC_READ;
    } else if (memory_flag == MemoryFlag::DEDICATE_GPU_MEMORY) {
        allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        initial_state = D3D12_RESOURCE_STATE_COMMON;
    }

    CHECK_DX_RESULT(render_context.d3dma_allocator->CreateResource(
        &allocation_desc, &_buffer_create_info, initial_state, NULL,
        &m_allocation, IID_NULL, NULL));
}

Buffer::~Buffer() noexcept { m_allocation->Release(); }

SwapChain::SwapChain(const RendererContext &render_context,
                     const SwapChainCreateInfo &swap_chain_create_info,
                     Window *window) noexcept
    : m_context(render_context) {
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = swap_chain_create_info.back_buffer_count;
    swapChainDesc.Width = window->GetWidth();
    swapChainDesc.Height = window->GetHeight();
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;


    IDXGISwapChain1 *swap_chain;
    CHECK_DX_RESULT(m_context.factory->CreateSwapChainForHwnd(
        m_context.queues[CommandQueueType::GRAPHICS],
        window->GetWin32Window(), &swapChainDesc, nullptr, nullptr, &swap_chain));
    // Swap chain needs the queue so that it
    // can force
    // a flush on it.
    // This sample does not support fullscreen transitions.
    CHECK_DX_RESULT(m_context.factory->MakeWindowAssociation(
        window->GetWin32Window(), DXGI_MWA_NO_ALT_ENTER));

    gpu_swap_chain = static_cast<IDXGISwapChain3 *>(swap_chain);
    current_frame_index = gpu_swap_chain->GetCurrentBackBufferIndex();
}


CommandList::CommandList(const RendererContext &context, CommandQueueType type,
                         ID3D12CommandAllocator *allocator,
                         ID3D12GraphicsCommandList *command_list) noexcept
    : gpu_command_list(command_list), m_type(type),
      command_allocator(allocator) {}

CommandList::~CommandList() noexcept {}

void CommandList::BeginRecording() {
    gpu_command_list->Reset(command_allocator, nullptr);
}

void CommandList::EndRecording() { gpu_command_list->Close(); }

void CommandList::BindVertexBuffers(u32 buffer_count, Buffer **buffers,
                                    u32 *offsets) {
    //gpu_command_list->IASetVertexBuffers();
}

void CommandList::BindIndexBuffer(Buffer *buffer, u32 offset) {
    //gpu_command_list->IASetIndexBuffer();
}

CommandContext::CommandContext(const RendererContext &context) noexcept
    : m_context(context) {
    m_command_lists_count.fill(0);
}

CommandContext::~CommandContext() noexcept {}

CommandList *CommandContext::GetCommandList(CommandQueueType type) {

    auto command_list_type = to_dx_command_list_type(type);

    // lazy create command pool
    if (!m_command_allocators[type]) {
        CHECK_DX_RESULT(m_context.device->CreateCommandAllocator(
            command_list_type, IID_PPV_ARGS(&m_command_allocators[type])));
    }

    u32 count = m_command_lists_count[type];

    if (count >= m_command_lists[type].size()) {
        //VkCommandBuffer command_buffer;
        //VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        //command_buffer_allocate_info.sType =
        //    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        //command_buffer_allocate_info.commandPool = m_command_pools[type];
        //command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        //command_buffer_allocate_info.commandBufferCount = 1;
        //CHECK_VK_RESULT(vkAllocateCommandBuffers(
        //     m_context.device, &command_buffer_allocate_info,
        //     &command_buffer));
        ID3D12GraphicsCommandList *dx_command_list;
        ID3D12PipelineState *initialState{};
        CHECK_DX_RESULT(m_context.device->CreateCommandList(
            0, command_list_type, m_command_allocators[type], nullptr,
            IID_PPV_ARGS(&dx_command_list)));
        
        auto cmd = Memory::Alloc<CommandList>(
            m_context, type, m_command_allocators[type], dx_command_list);
        
        m_command_lists[type].emplace_back(cmd);
    }

    m_command_lists_count[type]++;
    return m_command_lists[type][count];
}


void CommandContext::Reset() {}

void CommandList::DrawInstanced(u32 vertex_count, u32 first_vertex,
                                u32 instance_count, u32 first_instance) {
    gpu_command_list->DrawInstanced(vertex_count, instance_count, first_vertex,
                                    first_instance);
}

void CommandList::DrawIndexedInstanced(u32 index_count, u32 first_index,
                                       u32 first_vertex, u32 instance_count,
                                       u32 first_instance) {
    gpu_command_list->DrawIndexedInstanced(
        index_count, instance_count, first_index, first_vertex, first_instance);
}

void CommandList::Dispatch(u32 group_count_x, u32 group_count_y,
                           u32 group_count_z) {
    gpu_command_list->Dispatch(group_count_x, group_count_y, group_count_z);
}

void CommandList::InsertBarrier(const BarrierDesc &desc) {
    auto stack_memory = Memory::GetStackMemoryResource(1024);
    Container::Array<D3D12_RESOURCE_BARRIER> barriers(&stack_memory);
    barriers.reserve(desc.buffer_memory_barriers.size() +
                     desc.texture_memory_barriers.size());

    for (auto &buffer_barreir : desc.buffer_memory_barriers) {
        D3D12_RESOURCE_BARRIER b{};
        barriers.push_back(b);
    }
    for (auto &texture_barreir : desc.texture_memory_barriers) {
        D3D12_RESOURCE_BARRIER t{};
        barriers.push_back(t);
    }
    gpu_command_list->ResourceBarrier(static_cast<u32>(barriers.size()),
                                      barriers.data());
}

void CommandList::BindPipeline(Pipeline *pipeline) {
    gpu_command_list->SetPipelineState(pipeline->gpu_pipeline);
}

} // namespace Fract