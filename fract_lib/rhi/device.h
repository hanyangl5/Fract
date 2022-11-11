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

#include <D3D12MemAlloc.h>
#include "stdafx.h"

#include <utils/defination.h>
#include <utils/math/Math.h>
#include <utils/window/Window.h>
#include "rhi_utils.h"

namespace Fract {

struct RendererContext {
    ID3D12Device *device;
    IDXGIFactory6 *factory;
    IDXGIAdapter4 *active_gpu;
    D3D12MA::Allocator *d3dma_allocator;
    //ID3D12CommandQueue *graphics_queue, *compute_queue, *transfer_queue;
    Container::FixedArray<ID3D12CommandQueue *, 3> queues;
    IDXGISwapChain3 *swap_chain;
};


class Buffer {
  public:
    Buffer(const RendererContext &render_context,
           const BufferCreateInfo &buffer_create_info,
           MemoryFlag memory_flag) noexcept;
    ~Buffer() noexcept;

    inline void *GetResource() const noexcept {
        return m_allocation->GetResource();
    };

  public:
    DescriptorTypes m_descriptor_types{};
    ResourceState m_resource_state{};
    u64 m_size{};

  private:
    const RendererContext &m_context{};
    D3D12MA::Allocation *m_allocation{};
};

class Texture {
  public:
  private:
    ID3D12Resource *gpu_texture;
};

class RenderTarget {
  public:
  private:
    ID3D12Resource *gpu_render_target;
};

class Fence {
  public:
  private:
    ID3D12Fence *gpu_fence;
};

class Pipeline {
  public:
    ID3D12PipelineState *gpu_pipeline;
};


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



class SwapChain {
  public:
    SwapChain(const RendererContext &render_context,
              const SwapChainCreateInfo &swap_chain_create_info,
              Window *window) noexcept;
     ~SwapChain() noexcept = default;

    //void AcquireNextFrame(
    //    SwapChainSemaphoreContext *recycled_sempahores) noexcept;

    RenderTarget *GetRenderTarget() noexcept {
        return render_targets[current_frame_index];
    }

  public:

    Container::Array<RenderTarget *> render_targets{};
    u32 m_back_buffer_count{};
    u32 current_frame_index{0};
    u32 image_index{};
    u32 width{};
    u32 height{};

  private:
    const RendererContext &m_context{};
    IDXGISwapChain3 *gpu_swap_chain;

  protected:
};


class DescriptorSet {
  public:
};

class DescriptorSetAllocator {
  public:

};

struct RenderTargetInfo {
    RenderTarget *data{};
    std::variant<ClearColorValue, ClearValueDepthStencil> clear_color{};
};

struct RenderPassBeginInfo {
    u32 render_target_count{};
    Container::FixedArray<RenderTargetInfo, MAX_RENDER_TARGET_COUNT>
        render_targets{};
    RenderTargetInfo depth_stencil{};
    Rect render_area{};
};

class Semaphore {
  public:
};

enum QueueOp { IGNORED, RELEASE, ACQUIRE };

struct BufferBarrierDesc {
    Buffer *buffer{};
    // u32 offset;
    // u64 size;
    // u32 src_access_mask, dst_access_mask;
    // CommandQueueType src_queue, dst_queue;
    ResourceState src_state{}, dst_state{};
    CommandQueueType queue{}; // only the other queue type is need
    QueueOp queue_op{QueueOp::IGNORED};
};

struct TextureBarrierDesc {
    Texture *texture;
    // MemoryAccessFlags src_access_mask, dst_access_mask;
    // TextureUsage src_usage, dst_usage; // transition image layout
    ResourceState src_state{}, dst_state{};
    u32 first_mip_level{};
    u32 mip_level_count{1};
    u32 first_layer{};
    u32 layer_count{1};
    // CommandQueueType src_queue, dst_queue;
    CommandQueueType queue; // only the other queue type is need
    QueueOp queue_op{QueueOp::IGNORED};
};

struct BarrierDesc {
    // u32 src_stage, dst_stage;
    Container::Array<BufferBarrierDesc> buffer_memory_barriers{};
    Container::Array<TextureBarrierDesc> texture_memory_barriers{};
};

class CommandList {
  public:
    CommandList(const RendererContext &context, CommandQueueType type,
                ID3D12CommandAllocator *allocator,
                ID3D12GraphicsCommandList *command_list) noexcept;

    ~CommandList() noexcept;

    CommandList(const CommandList &rhs) noexcept = delete;
    CommandList &
    operator=(const CommandList &rhs) noexcept = delete;
    CommandList(CommandList &&rhs) noexcept = delete;
    CommandList &operator=(CommandList &&rhs) noexcept = delete;

    void BeginRecording();
    void EndRecording();

    void BindVertexBuffers(u32 buffer_count, Buffer **buffers,
                                   u32 *offsets);
    void BindIndexBuffer(Buffer *buffer, u32 offset);

    void BeginRenderPass(const RenderPassBeginInfo &begin_info);
    void EndRenderPass();
    void DrawInstanced(u32 vertex_count, u32 first_vertex,
                               u32 instance_count = 1,
                               u32 first_instance = 0);
    void DrawIndexedInstanced(u32 index_count, u32 first_index,
                                      u32 first_vertex, u32 instance_count = 1,
                                      u32 first_instance = 0);
    void DrawIndirect();
    void DrawIndirectIndexedInstanced(Buffer *buffer, u32 offset,
                                              u32 draw_count, u32 stride);
    void Dispatch(u32 group_count_x, u32 group_count_y,
                          u32 group_count_z);
    void DispatchIndirect();

    void UpdateBuffer(Buffer *buffer, void *data, u64 size);
    void UpdateTexture(Texture *texture,
                               const TextureUpdateDesc &texture_data);
    void CopyBuffer(Buffer *dst_buffer, Buffer *src_buffer);
    void CopyTexture(Texture *src_texture, Texture *dst_texture);
    // void CopyBufferToTexture(Buffer *src_buffer, Texture
    // *dst_texture);

    void InsertBarrier(const BarrierDesc &desc);

    void BindPipeline(Pipeline *pipeline);

    void BindPushConstant(Pipeline *pipeline,
                                  const Container::String &name,
                                  void *data);

    void ClearBuffer(Buffer *buffer, f32 clear_value);

    void ClearTextrue(Texture *texture,
                              const Math::float4 &clear_value);

    // bind by index save string lookup
    void BindPushConstant(Pipeline *pipeline, u32 index,
                                  void *data);

    void BindDescriptorSets(Pipeline *pipeline, DescriptorSet *set);

    void GenerateMipMap(Texture *texture, bool alllevels = true);

public:
    ID3D12GraphicsCommandList *get() const noexcept { return gpu_command_list; }

  private:
    ID3D12CommandAllocator *command_allocator{};
    ID3D12GraphicsCommandList *gpu_command_list{};
    bool is_recoring{false};
    CommandQueueType m_type{};
};

class CommandContext {
  public:
    CommandContext(const RendererContext &context) noexcept;

    ~CommandContext() noexcept;

    CommandContext(const CommandContext &command_list) noexcept = delete;
    CommandContext(CommandContext &&command_list) noexcept = delete;

    CommandContext &operator=(const CommandContext &rhs) noexcept = delete;

    CommandContext &operator=(CommandContext &&rhs) noexcept = delete;

    CommandList *GetCommandList(CommandQueueType type);
    void Reset();

  private:
    const RendererContext &m_context{};
    // each thread has pools to allocate graphics/compute/transfer commandlist
    Container::FixedArray<ID3D12CommandAllocator*, 3> m_command_allocators{};

    Container::FixedArray<Container::Array<CommandList *>, 3> m_command_lists{};
    Container::FixedArray<u32, 3> m_command_lists_count{};
};

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

    SwapChain *
    CreateSwapChain(const SwapChainCreateInfo &swap_chain_create_info,
                    Window *window);
    CommandList *GetCommandList(CommandQueueType type);

    void SubmitCommandLists(const QueueSubmitInfo &queue_submit_info);

    void WaitAll();
    void WaitGpuExecution(CommandQueueType type);
  private:
    RendererContext render_context{};
    DescriptorSetAllocator *descriptor_set_allocator{};
};

} // namespace Fract