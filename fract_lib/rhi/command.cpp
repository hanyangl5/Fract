/*****************************************************************//**
 * \file   command.cpp
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#include "command.h"

namespace Fract {

CommandList::CommandList(const RendererContext &context, CommandQueueType type,
                         ID3D12CommandAllocator *allocator,
                         ID3D12GraphicsCommandList *command_list) noexcept
    : gpu_command_list(command_list), m_type(type),
      command_allocator(allocator), m_context(context),
      descriptor_set_allocator(descriptor_set_allocator) {}

CommandList::~CommandList() noexcept {}

void CommandList::BeginRecording() {
    gpu_command_list->Reset(command_allocator, nullptr);

    //	if (m_type != CommandQueueType::TRANSFER) {
    //    ID3D12DescriptorHeap *heaps[] = {
    //        descriptor_set_allocator.get(DescriptorHeapType::CBV_SRV_UAV)
    //            .gpu_heap,
    //        descriptor_set_allocator.get(DescriptorHeapType::SAMPLER).gpu_heap};
    //    gpu_command_list->SetDescriptorHeaps(2, heaps);

    //    //pCmd->mD3D12.mBoundHeapStartHandles[0] =
    //    //    pCmd->mD3D12.pBoundHeaps[0]
    //    //        ->pHeap->GetGPUDescriptorHandleForHeapStart();
    //    //pCmd->mD3D12.mBoundHeapStartHandles[1] =
    //    //    pCmd->mD3D12.pBoundHeaps[1]
    //    //        ->pHeap->GetGPUDescriptorHandleForHeapStart();
    //}

    // Reset CPU side data
    //pCmd->mD3D12.pBoundRootSignature = NULL;
    //for (uint32_t i = 0; i < DESCRIPTOR_UPDATE_FREQ_COUNT; ++i) {
    //    pCmd->mD3D12.pBoundDescriptorSets[i] = NULL;
    //    pCmd->mD3D12.mBoundDescriptorSetIndices[i] = -1;
    //}
}

void CommandList::EndRecording() { gpu_command_list->Close(); }

void CommandList::BindVertexBuffers(u32 buffer_count, Buffer **buffers,
                                    u32 *offsets) {
    // gpu_command_list->IASetVertexBuffers();
}

void CommandList::BindIndexBuffer(Buffer *buffer, u32 offset) {
    // gpu_command_list->IASetIndexBuffer();
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
        // VkCommandBuffer command_buffer;
        // VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        // command_buffer_allocate_info.sType =
        //     VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        // command_buffer_allocate_info.commandPool = m_command_pools[type];
        // command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        // command_buffer_allocate_info.commandBufferCount = 1;
        // CHECK_VK_RESULT(vkAllocateCommandBuffers(
        //      m_context.device, &command_buffer_allocate_info,
        //      &command_buffer));
        ID3D12GraphicsCommandList *dx_command_list;
        ID3D12PipelineState *initialState{};
        CHECK_DX_RESULT(m_context.device->CreateCommandList(
            0, command_list_type, m_command_allocators[type], nullptr,
            IID_PPV_ARGS(&dx_command_list)));
        dx_command_list->Close();
        auto cmd = Memory::Alloc<CommandList>(
            m_context, type, m_command_allocators[type], dx_command_list);

        m_command_lists[type].emplace_back(cmd);
    }

    m_command_lists_count[type]++;
    auto ret = m_command_lists[type][count];
    return ret;
}

void CommandContext::Reset() {
    for (auto &allocator : m_command_allocators) {
        if (allocator) {
            allocator->Reset();
            m_command_lists_count.fill(0);
        }
    }
    //for (auto &type : m_command_lists) {
    //    for (auto &cmd : type) {
    //        Memory::Free(cmd);
    //        cmd = nullptr;
    //    }
    //}
}

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

void CommandList::UpdateBuffer(Buffer *buffer, void *data, u64 size) {

    if (!buffer->m_stage_buffer)
        buffer->m_stage_buffer = Memory::Alloc<Buffer>(
            m_context,
            BufferCreateInfo{DescriptorType::DESCRIPTOR_TYPE_UNDEFINED,
                             ResourceState::RESOURCE_STATE_COPY_SOURCE,
                             buffer->m_size},
            MemoryFlag::CPU_VISABLE_MEMORY);

    const auto &stage_resource = buffer->m_stage_buffer->GetResource();

    void *mapped_data{};
    CHECK_DX_RESULT(stage_resource->Map(0, nullptr, &mapped_data));
    memcpy(mapped_data, data, size);
    stage_resource->Unmap(0, nullptr);

    // barrier for stage upload
    {
        // BufferBarrierDesc bmb{};
        // bmb.buffer = vk_buffer->m_stage_buffer;
        // bmb.src_state = RESOURCE_STATE_HOST_WRITE;
        // bmb.dst_state = RESOURCE_STATE_COPY_SOURCE;

        // BarrierDesc desc{};

        // desc.buffer_memory_barriers.emplace_back(bmb);
        // InsertBarrier(desc);
    }

    gpu_command_list->CopyResource(buffer->GetResource(), stage_resource);
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
    gpu_command_list->SetPipelineState(pipeline->get());

     //TODO(hylu) remove  this
        if (m_type != CommandQueueType::TRANSFER) {
        Container::FixedArray<ID3D12DescriptorHeap *, 2> heaps{
                m_context.descriptor_heaps[CBV_SRV_UAV]->
                gpu_heap,
                m_context.descriptor_heaps[SAMPLER]->gpu_heap};
        gpu_command_list->SetDescriptorHeaps(heaps.size(), heaps.data());
    }

    if (pipeline->GetType() == PipelineType::GRAPHICS) {
        gpu_command_list->SetGraphicsRootSignature(
            pipeline->get_root_signature());
    } else if (pipeline->GetType() == PipelineType::COMPUTE) {
        gpu_command_list->SetComputeRootSignature(
            pipeline->get_root_signature());
        gpu_command_list->SetComputeRootDescriptorTable(
            0,
            m_context.descriptor_heaps[CBV_SRV_UAV]->gpu_handle);
    }
}

void CommandList::BindDescriptorSets(Pipeline *pipeline, DescriptorSet *set) {
    if (m_type != CommandQueueType::TRANSFER) {
        Container::FixedArray<ID3D12DescriptorHeap *, 2> heaps{
            m_context.descriptor_heaps[CBV_SRV_UAV]->gpu_heap,
            m_context.descriptor_heaps[SAMPLER]->gpu_heap};
        gpu_command_list->SetDescriptorHeaps(heaps.size(), heaps.data());
    }

    // pCmd->mD3D12.mBoundHeapStartHandles[0] =
    //     pCmd->mD3D12.pBoundHeaps[0]
    //         ->pHeap->GetGPUDescriptorHandleForHeapStart();
    // pCmd->mD3D12.mBoundHeapStartHandles[1] =
    //     pCmd->mD3D12.pBoundHeaps[1]
    //         ->pHeap->GetGPUDescriptorHandleForHeapStart();

    //CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(
    //    descriptor_set_allocator.get(DescriptorHeapType::CBV_SRV_UAV).gpu_handle,
    //    srvIndex + threadIndex, m_srvUavDescriptorSize);

    //CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(
    //    m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(),
    //    uavIndex + threadIndex, m_srvUavDescriptorSize);

    //pCommandList->SetComputeRootConstantBufferView(
    //    ComputeRootCBV, m_constantBufferCS->GetGPUVirtualAddress());
    //m_commandList->SetGraphicsRootDescriptorTable(
    //    0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
    if(m_type == CommandQueueType::COMPUTE) {
        gpu_command_list->SetComputeRootDescriptorTable(
            0, m_context.descriptor_heaps[CBV_SRV_UAV]
                   ->gpu_handle);

    }
    //   ID3D12DescriptorHeap *ppHeaps[] = {m_srvUavHeap.Get()};
    //pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    //CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(
    //    m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(),
    //    srvIndex + threadIndex, m_srvUavDescriptorSize);
    //CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(
    //    m_srvUavHeap->GetGPUDescriptorHandleForHeapStart(),
    //    uavIndex + threadIndex, m_srvUavDescriptorSize);

    //pCommandList->SetComputeRootConstantBufferView(
    //    ComputeRootCBV, m_constantBufferCS->GetGPUVirtualAddress());
    //pCommandList->SetComputeRootDescriptorTable(ComputeRootSRVTable, srvHandle);
    //pCommandList->SetComputeRootDescriptorTable(ComputeRootUAVTable, uavHandle);

    //// Set root signature if the current one differs from pRootSignature
    //reset_root_signature(
    //    pCmd, (PipelineType)pDescriptorSet->mD3D12.mPipelineType,
    //    pDescriptorSet->mD3D12.pRootSignature->mD3D12.pDxRootSignature);

    //if (pCmd->mD3D12.mBoundDescriptorSetIndices[pDescriptorSet->mD3D12
    //                                                .mUpdateFrequency] !=
    //        index ||
    //    pCmd->mD3D12.pBoundDescriptorSets[pDescriptorSet->mD3D12
    //                                          .mUpdateFrequency] !=
    //        pDescriptorSet) {
    //    pCmd->mD3D12
    //        .pBoundDescriptorSets[pDescriptorSet->mD3D12.mUpdateFrequency] =
    //        pDescriptorSet;
    //    pCmd->mD3D12.mBoundDescriptorSetIndices[pDescriptorSet->mD3D12
    //                                                .mUpdateFrequency] = index;

    //    // Bind the descriptor tables associated with this DescriptorSet
    //    if (pDescriptorSet->mD3D12.mPipelineType == PIPELINE_TYPE_GRAPHICS) {
    //        if (pDescriptorSet->mD3D12.mCbvSrvUavHandle !=
    //            D3D12_DESCRIPTOR_ID_NONE) {
    //            pCmd->mD3D12.pDxCmdList->SetGraphicsRootDescriptorTable(
    //                pDescriptorSet->mD3D12.mCbvSrvUavRootIndex,
    //                descriptor_id_to_gpu_handle(
    //                    pCmd->mD3D12.pBoundHeaps[0],
    //                    pDescriptorSet->mD3D12.mCbvSrvUavHandle +
    //                        index * pDescriptorSet->mD3D12.mCbvSrvUavStride));
    //        }

    //        if (pDescriptorSet->mD3D12.mSamplerHandle !=
    //            D3D12_DESCRIPTOR_ID_NONE) {
    //            pCmd->mD3D12.pDxCmdList->SetGraphicsRootDescriptorTable(
    //                pDescriptorSet->mD3D12.mSamplerRootIndex,
    //                descriptor_id_to_gpu_handle(
    //                    pCmd->mD3D12.pBoundHeaps[1],
    //                    pDescriptorSet->mD3D12.mSamplerHandle +
    //                        index * pDescriptorSet->mD3D12.mSamplerStride));
    //        }
    //    } else {
    //        if (pDescriptorSet->mD3D12.mCbvSrvUavHandle !=
    //            D3D12_DESCRIPTOR_ID_NONE) {
    //            pCmd->mD3D12.pDxCmdList->SetComputeRootDescriptorTable(
    //                pDescriptorSet->mD3D12.mCbvSrvUavRootIndex,
    //                descriptor_id_to_gpu_handle(
    //                    pCmd->mD3D12.pBoundHeaps[0],
    //                    pDescriptorSet->mD3D12.mCbvSrvUavHandle +
    //                        index * pDescriptorSet->mD3D12.mCbvSrvUavStride));
    //        }

    //        if (pDescriptorSet->mD3D12.mSamplerHandle !=
    //            D3D12_DESCRIPTOR_ID_NONE) {
    //            pCmd->mD3D12.pDxCmdList->SetComputeRootDescriptorTable(
    //                pDescriptorSet->mD3D12.mSamplerRootIndex,
    //                descriptor_id_to_gpu_handle(
    //                    pCmd->mD3D12.pBoundHeaps[1],
    //                    pDescriptorSet->mD3D12.mSamplerHandle +
    //                        index * pDescriptorSet->mD3D12.mSamplerStride));
    //        }
    //    }
    //}
}

Buffer *
Fract::CommandList::GetStageBuffer(const BufferCreateInfo &buffer_create_info) {
    return Memory::Alloc<Buffer>(m_context, buffer_create_info,
                                       MemoryFlag::CPU_VISABLE_MEMORY);
}

void CommandList::reset_root_signature(ID3D12RootSignature *rs,
                                       PipelineType type) {
    // Set root signature if the current one differs from pRootSignature
    //if (pCmd->mD3D12.pBoundRootSignature != pRootSignature) {
    //    pCmd->mD3D12.pBoundRootSignature = pRootSignature;
    //    if (type == PIPELINE_TYPE_GRAPHICS)
    //        pCmd->mD3D12.pDxCmdList->SetGraphicsRootSignature(pRootSignature);
    //    else
    //        pCmd->mD3D12.pDxCmdList->SetComputeRootSignature(pRootSignature);

    //    for (uint32_t i = 0; i < DESCRIPTOR_UPDATE_FREQ_COUNT; ++i) {
    //        pCmd->mD3D12.pBoundDescriptorSets[i] = NULL;
    //        pCmd->mD3D12.mBoundDescriptorSetIndices[i] = -1;
    //    }
    //}
}

}
