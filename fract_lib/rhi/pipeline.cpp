/*****************************************************************//**
 * \file   pipeline.cpp
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#include "pipeline.h"

namespace Fract {

Pipeline::Pipeline(const RendererContext &context,
                   const GraphicsPipelineCreateInfo &create_info,
                   DescriptorSetAllocator &descriptor_set_manager) noexcept
    : m_context(context) {
    m_create_info.type = PipelineType::GRAPHICS;
    m_create_info.gpci =
        const_cast<GraphicsPipelineCreateInfo *>(std::move(&create_info));
}
Pipeline::Pipeline(const RendererContext &context,
                   const ComputePipelineCreateInfo &create_info,
                   DescriptorSetAllocator &descriptor_set_manager) noexcept
    : m_context(context) {
    m_create_info.type = PipelineType::COMPUTE;
    m_create_info.cpci =
        const_cast<ComputePipelineCreateInfo *>(std::move(&create_info));
}
void Pipeline::SetComputeShader(Shader *vs) {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};

    m_context.device->CreateComputePipelineState(&desc,
                                                 IID_PPV_ARGS(&gpu_pipeline));
}
void Pipeline::SetGraphicsShader(Shader *vs, Shader *ps) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

    m_context.device->CreateGraphicsPipelineState(&desc,
                                                  IID_PPV_ARGS(&gpu_pipeline));
}

} // namespace Fract
