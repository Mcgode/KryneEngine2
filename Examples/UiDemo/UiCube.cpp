/**
 * @file
 * @author Max Godefroy
 * @date 14/01/2026.
 */

#include "UiCube.hpp"

#include <KryneEngine/Core/Graphics/Drawing.hpp>
#include <KryneEngine/Core/Graphics/ShaderPipeline.hpp>
#include <KryneEngine/Core/Math/Projection.hpp>
#include <KryneEngine/Core/Math/Transform.hpp>
#include <fstream>

static const float3 positions[8] = {
    { -1, -1, -1 },
    {  1, -1, -1 },
    { -1,  1, -1 },
    {  1,  1, -1 },
    { -1, -1,  1 },
    {  1, -1,  1 },
    { -1,  1,  1 },
    {  1,  1,  1 },
};

const u16 indices[36] = {
    0, 1, 2, 2, 1, 3,
    4, 5, 6, 6, 5, 7,
    0, 4, 1, 1, 4, 5,
    2, 6, 3, 3, 6, 7,
    0, 2, 4, 4, 2, 6,
    1, 3, 5, 5, 3, 7,
};

UiCube::UiCube(
    AllocatorInstance _allocator,
    GraphicsContext& _graphicsContext,
    Modules::TextRendering::FontManager* _fontManager,
    RenderPassHandle _renderPass)
        : m_allocator(_allocator)
        , m_guiContext(_allocator, _fontManager)
        , m_guiRenderer(_allocator, _graphicsContext, _renderPass)
        , m_constantBuffer(_allocator)
        , m_constantBufferViews(_allocator)
{
    m_vertexBuffer = _graphicsContext.CreateBuffer({
        .m_desc = {
            .m_size = sizeof(positions),
#if !defined(KE_FINAL)
            .m_debugName = "UI Cube vertex buffer"
#endif
        },
        .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::VertexBuffer | MemoryUsage::TransferDstBuffer,
    });

    m_indexBuffer = _graphicsContext.CreateBuffer({
        .m_desc = {
            .m_size = sizeof(indices),
#if !defined(KE_FINAL)
            .m_debugName = "UI Cube index buffer"
#endif
        },
        .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndexBuffer | MemoryUsage::TransferDstBuffer,
    });

    m_transferBuffer = _graphicsContext.CreateBuffer({
        .m_desc = {
            .m_size = sizeof(positions) + sizeof(indices),
#if !defined(KE_FINAL)
            .m_debugName = "UI Cube transfer buffer"
#endif
        },
        .m_usage = MemoryUsage::StageOnce_UsageType | MemoryUsage::TransferSrcBuffer,
    });

    BufferMapping mapping { m_transferBuffer };
    _graphicsContext.MapBuffer(mapping);
    memcpy(mapping.m_ptr, positions, sizeof(positions));
    memcpy(mapping.m_ptr + sizeof(positions), indices, sizeof(indices));
    _graphicsContext.UnmapBuffer(mapping);

    const u8 frameContextCount = _graphicsContext.GetFrameContextCount();
    {
        m_constantBuffer.Init(
            &_graphicsContext,
            BufferCreateDesc {
                .m_desc = {
                    .m_size = sizeof(UiCubeData),
#if !defined(KE_FINAL)
                    .m_debugName = "UiCube constant buffer",
#endif
                },
                .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::ConstantBuffer
            },
            frameContextCount);

        m_constantBufferViews.Resize(frameContextCount);
        for (auto i = 0u; i < frameContextCount; ++i)
        {
            m_constantBufferViews[i] = _graphicsContext.CreateBufferView(
                BufferViewDesc {
                    .m_buffer = m_constantBuffer.GetBuffer(i),
                    .m_size = sizeof(UiCubeData),
                    .m_offset = 0,
                    .m_stride = sizeof(UiCubeData),
                    .m_accessType = BufferViewAccessType::Constant,
#if !defined(KE_FINAL)
                    .m_debugName = "UiCube constant buffer view",
#endif
                });
        }
    }

    {
        constexpr DescriptorBindingDesc bindings[] = {
            {
                .m_type = DescriptorBindingDesc::Type::ConstantBuffer,
                .m_visibility = ShaderVisibility::Vertex
            }
        };
        DescriptorSetLayoutHandle descriptorSetLayout = _graphicsContext.CreateDescriptorSetLayout(
            { .m_bindings = bindings },
            &m_descriptorSetIndex);

        m_pipelineLayout = _graphicsContext.CreatePipelineLayout({
            .m_descriptorSets = { &descriptorSetLayout, 1 },
        });

        m_descriptorSet = _graphicsContext.CreateDescriptorSet(descriptorSetLayout);
    }

    {
        const auto readShaderFile = [_allocator](const eastl::string_view _filePath) -> eastl::span<char>
        {
            std::ifstream file(_filePath.data(), std::ios::binary);
            KE_ASSERT(file);

            file.seekg(0, std::ios::end);
            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            auto* buffer = static_cast<char*>(_allocator.allocate(size));
            if (!file.read(buffer, size)) return {};

            file.close();

            return { buffer, static_cast<size_t>(size) };
        };

        const eastl::span<char> vertexShaderSource = readShaderFile(
            eastl::string("Shaders/UiCube_MainVS.", _allocator) + GraphicsContext::GetShaderFileExtension());
        const eastl::span<char> fragmentShaderSource = readShaderFile(
            eastl::string("Shaders/UiCube_MainFS.", _allocator) + GraphicsContext::GetShaderFileExtension());

        const ShaderModuleHandle vertexShaderModule = _graphicsContext.RegisterShaderModule(vertexShaderSource.data(), vertexShaderSource.size());
        const ShaderModuleHandle fragmentShaderModule = _graphicsContext.RegisterShaderModule(fragmentShaderSource.data(), fragmentShaderSource.size());

        const ShaderStage stages[] = {
            {
                .m_shaderModule = vertexShaderModule,
                .m_stage = ShaderStage::Stage::Vertex,
                .m_entryPoint = "MainVS",
            },
            {
                .m_shaderModule = fragmentShaderModule,
                .m_stage = ShaderStage::Stage::Fragment,
                .m_entryPoint = "MainFS",
            },
        };

        constexpr VertexLayoutElement vertexElements[] = {
            {
                .m_semanticName = VertexLayoutElement::SemanticName::Position,
                .m_semanticIndex = 0,
                .m_bindingIndex = 0,
                .m_format = TextureFormat::RGB32_Float,
                .m_offset = 0,
                .m_location = 0,
            },
        };

        constexpr VertexBindingDesc vertexBindings[] = {
            {
                .m_stride = sizeof(float3),
                .m_inputRate = VertexInputRate::Vertex,
            }
        };

        GraphicsPipelineDesc pipelineDesc {
            .m_stages = stages,
            .m_vertexInput = {
                .m_elements = vertexElements,
                .m_bindings = vertexBindings
            },
            .m_colorBlending = {
                .m_attachments = { kDefaultColorAttachmentAlphaBlendDesc }
            },
            .m_depthStencil = {
                .m_depthTest = false,
                .m_depthWrite = false,
            },
            .m_renderPass =  _renderPass,
            .m_pipelineLayout = m_pipelineLayout,
        };

        m_pso = _graphicsContext.CreateGraphicsPipeline(pipelineDesc);

        _graphicsContext.FreeShaderModule(fragmentShaderModule);
        _graphicsContext.FreeShaderModule(vertexShaderModule);
        _allocator.deallocate(fragmentShaderSource.data());
        _allocator.deallocate(vertexShaderSource.data());
    }

    m_guiContext.Initialize(&m_guiRenderer, {});
}


void UiCube::Render(
    GraphicsContext& _graphicsContext,
    CommandListHandle _transferCommandList,
    CommandListHandle _renderCommandList)
{
    if (m_transferBuffer != GenPool::kInvalidHandle)
    {
        if (m_transferFrameId == ~0u)
        {
            _graphicsContext.CopyBuffer(_transferCommandList, {
                .m_copySize = sizeof(positions),
                .m_bufferSrc = m_transferBuffer,
                .m_bufferDst = m_vertexBuffer,
                .m_offsetSrc = 0,
                .m_offsetDst = 0,
            });

            _graphicsContext.CopyBuffer(_transferCommandList, {
                .m_copySize = sizeof(indices),
                .m_bufferSrc = m_transferBuffer,
                .m_bufferDst = m_indexBuffer,
                .m_offsetSrc = sizeof(positions),
                .m_offsetDst = 0,
            });

            m_transferFrameId = _graphicsContext.GetFrameId();
        }
        else if (_graphicsContext.IsFrameExecuted(m_transferFrameId))
        {
            _graphicsContext.DestroyBuffer(m_transferBuffer);
            m_transferBuffer = GenPool::kInvalidHandle;
        }
    }

    const u8 frameIndex = _graphicsContext.GetCurrentFrameContextIndex();

    {
        m_constantBuffer.PrepareBuffers(&_graphicsContext, _transferCommandList, BarrierAccessFlags::ConstantBuffer, frameIndex);

        auto* data = static_cast<UiCubeData*>(m_constantBuffer.Map(&_graphicsContext, frameIndex));

        const GraphicsCommon::ApplicationInfo& appInfo = _graphicsContext.GetApplicationInfo();
        const float aspect = static_cast<float>(appInfo.m_displayOptions.m_width) / static_cast<float>(appInfo.m_displayOptions.m_height);
        const float4x4 projection = Math::PerspectiveProjection(60.f * M_PI / 180.f, aspect, 0.1f, 100.f, true);

        const float time = static_cast<float>(_graphicsContext.GetFrameId()) / 60.f;
        Math::Quaternion rotation;
        rotation.FromAxisAngle(float3(0, 1, 0), time);

        // Move the cube to the bottom left third of the screen
        const float3 position = { -1.5f, -1.5f, -5.f };
        const auto model = Math::ComputeTransformMatrix<float4x4>(position, rotation, float3(1));

        data->m_mvpMatrix = (projection * model).Transposed();

        m_constantBuffer.Unmap(&_graphicsContext);
        m_constantBuffer.PrepareBuffers(&_graphicsContext, _transferCommandList, BarrierAccessFlags::ConstantBuffer, frameIndex);
    }

    {
        const DescriptorSetWriteInfo::DescriptorData descriptorData {
            .m_handle = m_constantBufferViews[frameIndex].m_handle,
        };
        const DescriptorSetWriteInfo writeInfo {
            .m_index = m_descriptorSetIndex,
            .m_descriptorData = { &descriptorData, 1 },
        };
        _graphicsContext.UpdateDescriptorSet(m_descriptorSet, { &writeInfo, 1 }, true);
    }

    _graphicsContext.SetGraphicsPipeline(_renderCommandList, m_pso);
    _graphicsContext.SetGraphicsDescriptorSets(_renderCommandList, m_pipelineLayout, { &m_descriptorSet, 1 });

    const BufferSpan vbSpan { .m_size = sizeof(positions), .m_offset = 0, .m_stride = sizeof(float3), .m_buffer = m_vertexBuffer };
    _graphicsContext.SetVertexBuffers(_renderCommandList, { &vbSpan, 1 });
    
    const BufferSpan ibSpan { .m_size = sizeof(indices), .m_offset = 0, .m_stride = sizeof(u16), .m_buffer = m_indexBuffer };
    _graphicsContext.SetIndexBuffer(_renderCommandList, ibSpan, true);

    _graphicsContext.DrawIndexedInstanced(_renderCommandList, { .m_elementCount = 36, });

    m_guiContext.BeginLayout({});
    // TODO: render small ui
    m_guiContext.EndLayout(_graphicsContext, _transferCommandList, _renderCommandList);
}
