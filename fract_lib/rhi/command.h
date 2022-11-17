/*****************************************************************//**
 * \file   command.h
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

#include <variant>

#include "utils/math/Math.h"
#include "stdafx.h"
#include "rhi_utils.h"
#include "resources.h"
#include "barrier.h"
#include "pipeline.h"

namespace Fract {

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

class CommandList {
  public:
    CommandList(const RendererContext &context,
                    CommandQueueType type,
                ID3D12CommandAllocator *allocator,
                ID3D12GraphicsCommandList *command_list) noexcept;

    ~CommandList() noexcept;

    CommandList(const CommandList &rhs) noexcept = delete;
    CommandList &operator=(const CommandList &rhs) noexcept = delete;
    CommandList(CommandList &&rhs) noexcept = delete;
    CommandList &operator=(CommandList &&rhs) noexcept = delete;

    void BeginRecording();
    void EndRecording();

    void BindVertexBuffers(u32 buffer_count, Buffer **buffers, u32 *offsets);
    void BindIndexBuffer(Buffer *buffer, u32 offset);

    void BeginRenderPass(const RenderPassBeginInfo &begin_info);
    void EndRenderPass();
    void DrawInstanced(u32 vertex_count, u32 first_vertex,
                       u32 instance_count = 1, u32 first_instance = 0);
    void DrawIndexedInstanced(u32 index_count, u32 first_index,
                              u32 first_vertex, u32 instance_count = 1,
                              u32 first_instance = 0);
    void DrawIndirect();
    void DrawIndirectIndexedInstanced(Buffer *buffer, u32 offset,
                                      u32 draw_count, u32 stride);
    void Dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z);
    void DispatchIndirect();

    void UpdateBuffer(Buffer *buffer, void *data, u64 size);
    void UpdateTexture(Texture *texture, const TextureUpdateDesc &texture_data);
    void CopyBuffer(Buffer *dst_buffer, Buffer *src_buffer);
    void CopyTexture(Texture *src_texture, Texture *dst_texture);
    // void CopyBufferToTexture(Buffer *src_buffer, Texture
    // *dst_texture);

    void InsertBarrier(const BarrierDesc &desc);

    void BindPipeline(Pipeline *pipeline);

    void BindPushConstant(Pipeline *pipeline, const Container::String &name,
                          void *data);

    void ClearBuffer(Buffer *buffer, f32 clear_value);

    void ClearTextrue(Texture *texture, const Math::float4 &clear_value);

    // bind by index save string lookup
    void BindPushConstant(Pipeline *pipeline, u32 index, void *data);

    void BindDescriptorSets(Pipeline *pipeline, DescriptorSet *set);

    void GenerateMipMap(Texture *texture, bool alllevels = true);

  public:
    ID3D12GraphicsCommandList *get() const noexcept { return gpu_command_list; }

  private:
    void reset_root_signature(ID3D12RootSignature *rs, PipelineType type);
  private:
    const RendererContext &m_context;
    const DescriptorSetAllocator &descriptor_set_allocator;
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
    Container::FixedArray<ID3D12CommandAllocator *, 3> m_command_allocators{};

    Container::FixedArray<Container::Array<CommandList *>, 3> m_command_lists{};
    Container::FixedArray<u32, 3> m_command_lists_count{};
};

}
