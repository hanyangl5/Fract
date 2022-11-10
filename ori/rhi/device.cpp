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

namespace Ori {

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
    CHECK_DX_RESULT(
        CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory)));
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
    PickGPU(factory, &active_gpu);

    CHECK_DX_RESULT(
        D3D12CreateDevice(active_gpu, DX12_API_VERSION, IID_PPV_ARGS(&device)));

    // create command queue

    D3D12_COMMAND_QUEUE_DESC graphics_command_queue_desc{};
    graphics_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    graphics_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    CHECK_DX_RESULT(device->CreateCommandQueue(
        &graphics_command_queue_desc,
        IID_PPV_ARGS(&queues[CommandQueueType::GRAPHICS])));

#ifdef USE_ASYNC_COMPUTE_TRANSFER
    D3D12_COMMAND_QUEUE_DESC compute_command_queue_desc{};
    compute_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    compute_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

    CHECK_DX_RESULT(device->CreateCommandQueue(
        &compute_command_queue_desc,
        IID_PPV_ARGS(&queues[CommandQueueType::COMPUTE])));

    D3D12_COMMAND_QUEUE_DESC transfer_command_queue_desc{};
    transfer_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    transfer_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

    CHECK_DX_RESULT(device->CreateCommandQueue(
        &transfer_command_queue_desc,
        IID_PPV_ARGS(&queues[CommandQueueType::TRANSFER])));
    LOG_INFO("using async compute & transfer");
#endif // USE_ASYNC_COMPUTE_TRANSFER
}

void Device::InitializeD3DMA() noexcept {
    D3D12MA::ALLOCATOR_DESC allocatorDesc{};
    allocatorDesc.pDevice = device;
    allocatorDesc.pAdapter = active_gpu;

    CHECK_DX_RESULT(D3D12MA::CreateAllocator(&allocatorDesc, &d3dma_allocator));
}

} // namespace Ori