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
Pipeline::~Pipeline() noexcept {}

void Pipeline::SetComputeShader(Shader *cs) {
    if (m_cs == nullptr) {
        m_cs = cs;
        CreateRootSignature();
        CreateComputePipeline();
    }
}
void Pipeline::SetGraphicsShader(Shader *vs, Shader *ps) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    m_context.device->CreateGraphicsPipelineState(&desc,
                                                  IID_PPV_ARGS(&gpu_pipeline));
}

void Pipeline::CreateComputePipeline() {
    D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
    desc.CS = CD3DX12_SHADER_BYTECODE(m_cs->get()->GetBufferPointer(),
                                      m_cs->get()->GetBufferSize());
    desc.pRootSignature = root_signature;
    CHECK_DX_RESULT(m_context.device->CreateComputePipelineState(&desc,
                                                 IID_PPV_ARGS(&gpu_pipeline)));
}

void Pipeline::CreateRootSignature() {
    // Create a root signature consisting of a descriptor table with a single
    // CBV.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If
        // CheckFeatureSupport succeeds, the HighestVersion returned will not be
        // greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_context.device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE,
                                                 &featureData,
                                                 sizeof(featureData)))) {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];

        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0,
                       D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0],
                                                D3D12_SHADER_VISIBILITY_VERTEX);

        // Allow input layout and deny uneccessary access to certain pipeline
        // stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        //rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0,
        //                           nullptr, rootSignatureFlags);
        rootSignatureDesc.Init_1_1(0, nullptr, 0,
                                   nullptr, rootSignatureFlags);
        ID3DBlob* signature;
        ID3DBlob* error;
        CHECK_DX_RESULT(D3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc, featureData.HighestVersion, &signature,
            &error));
        if (error != nullptr && error->GetBufferSize() > 0) {
            LOG_ERROR("failed to compile shader: {}",
                      std::string{(char *)error->GetBufferPointer(),
                                  error->GetBufferSize()});
        }

        CHECK_DX_RESULT(m_context.device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature)));

    }
}

Shader::Shader(const RendererContext &context, ShaderType type,
               const std::filesystem::path &file_name) noexcept
    : m_context(context) {

    std::string s = file_name.string();


    ID3DBlob *error_msg1{};
    D3DCompileFromFile(std::wstring(s.begin(), s.end()).c_str(), nullptr,
                       nullptr,
                       "Main", "cs_5_0", 0, 0, &fxc_blob, &error_msg1);
    if (error_msg1 != nullptr && error_msg1->GetBufferSize() > 0) {
        LOG_ERROR("failed to compile shader: {}",
                  std::string{(char *)error_msg1->GetBufferPointer(),
                              error_msg1->GetBufferSize()});
    }
    return;
    // read source file
    IDxcBlobEncoding *source_file = nullptr;
    auto str = file_name.string();

    context.dxc_utils->LoadFile(std::wstring(str.begin(), str.end()).c_str(),
                                nullptr, &source_file);

    DxcBuffer source_buffer;
    source_buffer.Ptr = source_file->GetBufferPointer();
    source_buffer.Size = source_file->GetBufferSize();
    source_buffer.Encoding =
        DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

    IDxcIncludeHandler *pIncludeHandler;
    context.dxc_utils->CreateDefaultIncludeHandler(&pIncludeHandler);

    // fill compile args
    std::vector<LPCWSTR> compile_args;

    std::string typestr;
    switch (type) {
    case ShaderType::VERTEX_SHADER:
        typestr = "vs";
        break;
    case ShaderType::PIXEL_SHADER:
        typestr = "ps";
        break;
    case ShaderType::COMPUTE_SHADER:
        typestr = "cs";
        break;
        // case ShaderType::GEOMETRY_SHADER:
        //     typestr = "gs";
        break;
    default:
        break;
    }

    auto tp = "-T " + typestr + "_6_0";
    auto ep = Container::String("-E Main");

    std::wstring wtp(tp.begin(), tp.end());
    std::wstring wep(ep.begin(), ep.end());

    compile_args.emplace_back(wep.c_str());
    compile_args.emplace_back(wtp.c_str());

    // compile
    IDxcResult *compile_result;
    CHECK_DX_RESULT(context.dxc_compiler->Compile(
        &source_buffer, compile_args.data(), compile_args.size(), nullptr,
        IID_PPV_ARGS(&compile_result)));
    IDxcBlobUtf8 *error_msg;
    compile_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error_msg),
                              nullptr);

    if (error_msg && error_msg->GetBufferSize() > 0) {
        LOG_ERROR("failed to compile shader: {}",
                  std::string{(char *)error_msg->GetBufferPointer(),
                              error_msg->GetBufferSize()});
        // return nullptr;
    }

    IDxcBlob *result_code;
    compile_result->GetResult(&result_code);

    {
        error_msg->Release();
        compile_result->Release();
        source_file->Release();
        pIncludeHandler->Release();
    }

    shader_blob = result_code;
}

Shader::~Shader() noexcept {}

} // namespace Fract
