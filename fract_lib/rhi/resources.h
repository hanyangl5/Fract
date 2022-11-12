/*****************************************************************//**
 * \file   resources.h
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

#include <D3D12MemAlloc.h>

#include "utils/window/Window.h"

#include "stdafx.h"
#include "rhi_utils.h"

namespace Fract {

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
    Texture(const RendererContext &render_context,
            const TextureCreateInfo &texture_create_info) noexcept;
    ~Texture() noexcept;

  private:
    DescriptorTypes m_descriptor_types;
    ResourceState m_state{};
    const TextureType m_type{};
    const TextureFormat m_format{};

    const u32 m_width{}, m_height{}, m_depth{};
    u32 mip_map_level{};
    const u32 m_byte_per_pixel{};

    const RendererContext &m_context{};
    D3D12MA::Allocation *m_allocation{};
};

class Sampler {
  public:
    Sampler(const RendererContext &render_context,
            const SamplerDesc &create_info) noexcept;
    ~Sampler() noexcept;
  private:
    const RendererContext &m_context{};
    D3D12MA::Allocation *m_allocation{};
};

class RenderTarget {
  public:
    RenderTarget(const RendererContext &context,
                 const RenderTargetCreateInfo &create_info) noexcept;

  private:
    const RendererContext &m_context{};
    ID3D12Resource *gpu_render_target{};
};

class Fence {
  public:
  private:
    ID3D12Fence *gpu_fence;
};



class SwapChain {
  public:
    SwapChain(const RendererContext &render_context,
              const SwapChainCreateInfo &swap_chain_create_info,
              Window *window) noexcept;
    ~SwapChain() noexcept = default;

    // void AcquireNextFrame(
    //     SwapChainSemaphoreContext *recycled_sempahores) noexcept;

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


class Semaphore {
  public:
};


}
