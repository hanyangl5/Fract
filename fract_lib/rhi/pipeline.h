/*****************************************************************//**
 * \file   pipeline.h
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#pragma once

#include <filesystem>

#include "stdafx.h"
#include "rhi_utils.h"
#include "resources.h"

namespace Fract {


class DescriptorSet {
  public:
    DescriptorSet(ResourceUpdateFrequency frequency) noexcept
        : update_frequency(frequency){};
    ~DescriptorSet() noexcept {};

    DescriptorSet(const DescriptorSet &rhs) noexcept = delete;
    DescriptorSet &operator=(const DescriptorSet &rhs) noexcept = delete;
    DescriptorSet(DescriptorSet &&rhs) noexcept = delete;
    DescriptorSet &operator=(DescriptorSet &&rhs) noexcept = delete;

  public:
    void SetResource(Buffer *resource,
                             const Container::String &resource_name);
    void SetResource(Texture *resource,
                             const Container::String &resource_name);
    void SetResource(Sampler *resource,
                             const Container::String &resource_name);

    void
    SetBindlessResource(Container::Array<Buffer *> &resource,
                        const Container::String &resource_name);
    void
    SetBindlessResource(Container::Array<Texture *> &resource,
                        const Container::String &resource_name);
    void
    SetBindlessResource(Container::Array<Sampler *> &resource,
                        const Container::String &resource_name);
    void Update();

  public:
    ResourceUpdateFrequency update_frequency{};
};

class DescriptorSetAllocator {
  public:
};

class Shader {
  public:
    Shader(const RendererContext& context,ShaderType type, const std::filesystem::path &file) noexcept;
    ~Shader() noexcept;

    Shader(const Shader &rhs) noexcept = delete;
    Shader &operator=(const Shader &rhs) noexcept = delete;
    Shader(Shader &&rhs) noexcept = delete;
    Shader &operator=(Shader &&rhs) noexcept = delete;

    ShaderType GetType() const noexcept;

    ID3DBlob *get() const noexcept { return fxc_blob; }

    const std::filesystem::path &
    GetRootSignatureDescriptionPath() const noexcept {
        return m_rsd_path;
    }

  protected:
    const RendererContext &m_context;
    const ShaderType m_type{};
    const std::filesystem::path m_rsd_path; // lazy read
    IDxcBlob *shader_blob{};
    ID3DBlob* fxc_blob{};
};

class Pipeline {
  public:
    Pipeline(
        const RendererContext &context,
        const GraphicsPipelineCreateInfo &create_info,
        DescriptorSetAllocator &descriptor_set_manager) noexcept;
    Pipeline(
        const RendererContext &context,
        const ComputePipelineCreateInfo &create_info,
        DescriptorSetAllocator &descriptor_set_manager) noexcept;
    ~Pipeline() noexcept;

    Pipeline(const Pipeline &rhs) noexcept = delete;
    Pipeline &operator=(const Pipeline &rhs) noexcept = delete;
    Pipeline(Pipeline &&rhs) noexcept = delete;
    Pipeline &operator=(Pipeline &&rhs) noexcept = delete;

    PipelineType GetType() const noexcept;

    void SetComputeShader(Shader *vs);
    void SetGraphicsShader(Shader *vs, Shader *ps);

    DescriptorSet *GetDescriptorSet(ResourceUpdateFrequency frequency,
                                    u32 count = 1);
    DescriptorSet *GetBindlessDescriptorSet(ResourceUpdateFrequency frequency);

    ID3D12PipelineState *get() const noexcept { return gpu_pipeline; }

  private:
    void CreateGraphicsPipeline();
    void CreateComputePipeline();
    void CreateRootSignature();
    void ParseRootSignature();
    void ParseRootSignatureFromShader(Shader *shader);

    const RendererContext &m_context;

    // array contain all kinds of shaders
    Shader *m_vs{}, *m_ps{}, *m_cs{};
    PipelineCreateInfo m_create_info{};
    RootSignatureDesc rsd{};
    Container::FixedArray<u32, DESCRIPTOR_SET_UPDATE_FREQUENCIES>
        vk_binding_count{};
    ID3D12PipelineState *gpu_pipeline{};
    ID3D12RootSignature *root_signature{};
};

}
