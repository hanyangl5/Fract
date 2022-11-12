/*****************************************************************//**
 * \file   device.h
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

#include <array>
#include <variant>
#include <filesystem>

#include <D3D12MemAlloc.h>
#include "stdafx.h"

#include <utils/defination.h>
#include <utils/math/Math.h>
#include <utils/window/Window.h>
#include "rhi_utils.h"
#include "resources.h"
#include "barrier.h"
#include "pipeline.h"
#include "command.h"

namespace Fract {

//    static const UINT FrameCount = 2;
//
//// Pipeline objects.
// CD3DX12_VIEWPORT m_viewport;
// CD3DX12_RECT m_scissorRect;
// ComPtr<IDXGISwapChain3> m_swapChain;
//
// ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
// ComPtr<ID3D12RootSignature> m_rootSignature;
// ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
// ComPtr<ID3D12PipelineState> m_pipelineState;
// ComPtr<ID3D12GraphicsCommandList> m_commandList;
// UINT m_rtvDescriptorSize;
//
//// App resources.
// ComPtr<ID3D12Resource> m_vertexBuffer;
// D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
//
//// Synchronization objects.
// UINT m_frameIndex;
// HANDLE m_fenceEvent;
// ComPtr<ID3D12Fence> m_fence;
// UINT64 m_fenceValue;




extern thread_local CommandContext *thread_command_context;

struct QueueSubmitInfo {
    CommandQueueType queue_type;
    Container::Array<CommandList *> command_lists;
    Container::Array<Semaphore *> wait_semaphores;
    Container::Array<Semaphore *> signal_semaphores;
    bool wait_image_acquired = false;
    bool signal_render_complete = false;
};

struct QueuePresentInfo {
    // Container::Array<Semaphore *> wait_semaphores; // we only need to wait
    // render complete semaphore
    SwapChain *swap_chain;
};

class Device {
  public:
    Device() noexcept;
    ~Device() noexcept;

    void Initialize();
    void CreateFactory() noexcept;
    void PickGPU(IDXGIFactory6 *factory, IDXGIAdapter4 **gpu) noexcept;
    void CreateDevice() noexcept;
    void InitializeD3DMA() noexcept;
    
    
    Buffer *CreateBuffer(const BufferCreateInfo &buffer_create_info,
                         MemoryFlag memory_flag);

    Texture *CreateTexture(const TextureCreateInfo &texture_create_info);
    SwapChain *
    CreateSwapChain(const SwapChainCreateInfo &swap_chain_create_info,
                    Window *window);
    RenderTarget *
    CreateRenderTarget(const RenderTargetCreateInfo &render_target_create_info);
    CommandList *GetCommandList(CommandQueueType type);
    Pipeline *
    CreateGraphicsPipeline(const GraphicsPipelineCreateInfo &create_info);

    Pipeline *
    CreateComputePipeline(const ComputePipelineCreateInfo &create_info);

    void SubmitCommandLists(const QueueSubmitInfo &queue_submit_info);

    void WaitAll();
    void WaitGpuExecution(CommandQueueType type);

  private:
    RendererContext render_context{};
    DescriptorSetAllocator *descriptor_set_allocator{};
};

} // namespace Fract