/*****************************************************************//**
 * \file   device.h
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

#include <array>

#include <D3D12MemAlloc.h>
#include "stdafx.h"

#include <utils/defination.h>

namespace Ori {

enum CommandQueueType { GRAPHICS, COMPUTE, TRANSFER };

class Device {
  public:
    Device() noexcept;
    ~Device() noexcept;

    void Initialize();
    void CreateFactory() noexcept;
    void PickGPU(IDXGIFactory6 *factory, IDXGIAdapter4 **gpu) noexcept;
    void CreateDevice() noexcept;
    void InitializeD3DMA() noexcept;

  private:
    ID3D12Device *device;
    IDXGIFactory6 *factory;
    IDXGIAdapter4 *active_gpu;
    D3D12MA::Allocator *d3dma_allocator;
    ID3D12CommandQueue *graphics_queue, *compute_queue, *transfer_queue;
    std::array<ID3D12CommandQueue *, 3> queues;
    std::array<ID3D12Fence *, 3> fences;
    IDXGISwapChain3 *swap_chain;
};

} // namespace Ori