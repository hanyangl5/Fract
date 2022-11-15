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
      command_allocator(allocator) {}

CommandList::~CommandList() noexcept {}

void CommandList::BeginRecording() {
    gpu_command_list->Reset(command_allocator, nullptr);
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

        auto cmd = Memory::Alloc<CommandList>(
            m_context, type, m_command_allocators[type], dx_command_list);

        m_command_lists[type].emplace_back(cmd);
    }

    m_command_lists_count[type]++;
    return m_command_lists[type][count];
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
}

}
