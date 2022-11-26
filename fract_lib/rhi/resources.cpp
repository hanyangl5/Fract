/*****************************************************************//**
 * \file   resources.cpp
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#include "resources.h"

#include <algorithm>

namespace Fract {

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
    auto _buffer_create_info =
        CD3DX12_RESOURCE_DESC::Buffer(buffer_create_info.size, usage, 0);
    _buffer_create_info.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    if (m_resource_state == ResourceState::RESOURCE_STATE_UNORDERED_ACCESS) {
        _buffer_create_info.Flags |=
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
            D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    D3D12MA::ALLOCATION_DESC allocation_desc{};
    D3D12_RESOURCE_STATES initial_state{};
    if (memory_flag == MemoryFlag::CPU_TO_GPU) {
        allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        initial_state = D3D12_RESOURCE_STATE_GENERIC_READ;
    } else if (memory_flag == MemoryFlag::GPU_ONLY) {
        allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        initial_state = D3D12_RESOURCE_STATE_COPY_DEST;
    } else if (memory_flag == MemoryFlag::GPU_TO_CPU) {
        allocation_desc.HeapType = D3D12_HEAP_TYPE_READBACK;
        initial_state = D3D12_RESOURCE_STATE_COPY_SOURCE;
    }

    CHECK_DX_RESULT(render_context.d3dma_allocator->CreateResource(
        &allocation_desc, &_buffer_create_info, initial_state, NULL,
        &m_allocation, IID_NULL, NULL));

    if ((buffer_create_info.descriptor_types &
         DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER) ==
        DescriptorType::DESCRIPTOR_TYPE_CONSTANT_BUFFER) {
        assert((m_size % 256) == 0);
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
        cbv_desc.BufferLocation =
            m_allocation->GetResource()->GetGPUVirtualAddress();
        cbv_desc.SizeInBytes = AlignUp(m_size, 256);

        auto &heap = m_context.descriptor_heaps[CBV_SRV_UAV];

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle(
            heap->cpu_handle, heap->descriptor_offset * heap->descriptor_size);

        m_context.device->CreateConstantBufferView(&cbv_desc, cpu_handle);
        heap->descriptor_offset++;

    } else if ((buffer_create_info.descriptor_types &
                DescriptorType::DESCRIPTOR_TYPE_BUFFER) ==
               DescriptorType::DESCRIPTOR_TYPE_BUFFER) {
        // D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;

    } else if ((buffer_create_info.descriptor_types &
                DescriptorType::DESCRIPTOR_TYPE_RW_BUFFER) ==
               DescriptorType::DESCRIPTOR_TYPE_RW_BUFFER) {

        auto &heap = m_context.descriptor_heaps[CBV_SRV_UAV];

        D3D12_UNORDERED_ACCESS_VIEW_DESC
        uav_desc{};
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.NumElements = 1;
        uav_desc.Buffer.StructureByteStride = 256;
        uav_desc.Buffer.CounterOffsetInBytes = 0;
        uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle(
            heap->cpu_handle, heap->descriptor_offset * heap->descriptor_size);
        m_context.device->CreateUnorderedAccessView(
            m_allocation->GetResource(), nullptr, &uav_desc, cpu_handle);
        heap->descriptor_offset++;
    }
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
        m_context.queues[CommandQueueType::GRAPHICS], window->GetWin32Window(),
        &swapChainDesc, nullptr, nullptr, &swap_chain));
    // Swap chain needs the queue so that it
    // can force
    // a flush on it.
    // This sample does not support fullscreen transitions.
    CHECK_DX_RESULT(m_context.factory->MakeWindowAssociation(
        window->GetWin32Window(), DXGI_MWA_NO_ALT_ENTER));

    gpu_swap_chain = static_cast<IDXGISwapChain3 *>(swap_chain);
    current_frame_index = gpu_swap_chain->GetCurrentBackBufferIndex();
}

void SwapChain::AcquireNextFrame() noexcept {
    current_frame_index = gpu_swap_chain->GetCurrentBackBufferIndex();
    ;
}

RenderTarget::RenderTarget(const RendererContext &context,
                           const RenderTargetCreateInfo &create_info) noexcept
    : m_context(context) {
    // D3D12_RENDER_TARGET_VIEW_DESC desc{};
    // CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
    //     m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    // context.device->CreateRenderTargetView(gpu_render_target, );
}

Texture::Texture(const RendererContext &render_context,
                 const TextureCreateInfo &texture_create_info) noexcept
    : m_context(render_context), m_width(texture_create_info.width),
      m_height(texture_create_info.height),
      m_type(texture_create_info.texture_type),
      m_format(texture_create_info.texture_format) {

    mip_map_level =
        texture_create_info.enanble_mipmap == true
            ? Min(MAX_MIP_LEVEL, static_cast<uint32_t>(std::floor(
                                     std::log2(Max(m_width, m_height))))) +
                  1
            : 1;

    m_descriptor_types = texture_create_info.descriptor_types;
    m_resource_state = texture_create_info.initial_state;

    auto dxgi_format = ToDx12TextureFormat(m_format);

    CD3DX12_RESOURCE_DESC _texture_create_info = CD3DX12_RESOURCE_DESC::Tex2D(
        dxgi_format, m_width, m_height, texture_create_info.array_layer,
        mip_map_level);

    if (texture_create_info.descriptor_types &
        DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE) {
        _texture_create_info.Flags |=
            D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    D3D12MA::ALLOCATION_DESC allocation_desc = {};
    allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    CHECK_DX_RESULT(m_context.d3dma_allocator->CreateResource(
        &allocation_desc, &_texture_create_info, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // TODO:
        nullptr, &m_allocation, IID_NULL, nullptr));
    
    auto &heap = m_context.descriptor_heaps[CBV_SRV_UAV];
    if ((texture_create_info.descriptor_types &
         DescriptorType::DESCRIPTOR_TYPE_TEXTURE) ==
        DescriptorType::DESCRIPTOR_TYPE_TEXTURE) {

        // CreateShaderResourceView();
        // CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle(
        //    heap->cpu_handle, heap->descriptor_offset *
        //    heap->descriptor_size);
        // heap->descriptor_offset++;

    } else if ((texture_create_info.descriptor_types &
                DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE) ==
               DescriptorType::DESCRIPTOR_TYPE_RW_TEXTURE) {

        D3D12_UNORDERED_ACCESS_VIEW_DESC
        uav_desc{};
        uav_desc.Format = dxgi_format;
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D;

        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle(
            heap->cpu_handle, heap->descriptor_offset * heap->descriptor_size);
        m_context.device->CreateUnorderedAccessView(
            m_allocation->GetResource(), nullptr, &uav_desc, cpu_handle);
        heap->descriptor_offset++;
    }
}

Fence::Fence(const RendererContext &context) noexcept : m_context(context) {
    // Create synchronization objects and wait until assets have been uploaded
    // to the GPU.
    {
        CHECK_DX_RESULT(m_context.device->CreateFence(
            0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gpu_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        wait_idle_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (wait_idle_fence_event == nullptr) {
            CHECK_DX_RESULT(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

Fence::~Fence() noexcept {
    (gpu_fence->Release());
    CloseHandle(wait_idle_fence_event);
}

void Fence::Signal() {

    // Wait for the command list to execute; we are reusing the same command
    // list in our main loop but for now, we just want to wait for setup to
    // complete before continuing.
    const UINT64 fence = m_fenceValue;
    CHECK_DX_RESULT(
        m_context.queues[CommandQueueType::GRAPHICS]->Signal(gpu_fence, fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (gpu_fence->GetCompletedValue() < fence) {
        CHECK_DX_RESULT(
            gpu_fence->SetEventOnCompletion(fence, wait_idle_fence_event));
        WaitForSingleObject(wait_idle_fence_event, INFINITE);
    }
}

} // namespace Fract
