/**
 * @file
 * @author Max Godefroy
 * @date 08/12/2025.
 */

#include "KryneEngine/Modules/GuiLib/GuiRenderers/BasicGuiRenderer.hpp"

#include <KryneEngine/Core/Common/Types.hpp>
#include <KryneEngine/Core/Graphics/Drawing.hpp>
#include <KryneEngine/Core/Graphics/ShaderPipeline.hpp>
#include <KryneEngine/Core/Math/Color.hpp>
#include <KryneEngine/Core/Math/Float16.hpp>
#include <KryneEngine/Core/Math/Matrix.hpp>

#include <clay.h>
#include <fstream>

namespace KryneEngine
{
    struct PackedInstanceData
    {
        uint2 m_packedRect;
        uint4 m_packedData;
    };
}

namespace KryneEngine::Modules::GuiLib
{
    BasicGuiRenderer::BasicGuiRenderer(
        AllocatorInstance _allocator,
        GraphicsContext& _graphicsContext,
        RenderPassHandle _renderPass)
        : m_instanceDataBuffer(_allocator)
        , m_commonConstantBuffer(_allocator)
        , m_commonConstantBufferViews(_allocator)
    {
        const u8 frameContextCount = _graphicsContext.GetFrameContextCount();

        {
            m_instanceDataBuffer.Init(
               &_graphicsContext,
               BufferCreateDesc {
                   .m_desc = {
                       .m_size = 256,
                       .m_debugName = "BasicGuiRenderer instance data buffer"
                   },
                   .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::VertexBuffer
               },
               frameContextCount);
        }

        {
            m_commonConstantBuffer.Init(
                &_graphicsContext,
                BufferCreateDesc {
                    .m_desc = {
                        .m_size = sizeof(ViewportConstants),
                        .m_debugName = "BasicGuiRenderer common constant buffer"
                    },
                    .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::ConstantBuffer
                },
                frameContextCount);

            m_commonConstantBufferViews.Resize(frameContextCount);
            for (auto i = 0u; i < frameContextCount; ++i)
            {
                m_commonConstantBufferViews[i] = _graphicsContext.CreateBufferView(
                    BufferViewDesc {
                        .m_buffer = m_commonConstantBuffer.GetBuffer(i),
                        .m_size = sizeof(ViewportConstants),
                        .m_offset = 0,
                        .m_stride = sizeof(ViewportConstants),
                        .m_accessType = BufferViewAccessType::Constant,
#if !defined(KE_FINAL)
                        .m_debugName = "BasicGuiRenderer common constant buffer view",
#endif
                    });
            }
        }

        DescriptorSetLayoutHandle commonDescriptorSetLayout;
        DescriptorSetLayoutHandle texturesDescriptorSetLayout;
        {
            constexpr DescriptorBindingDesc descriptorSet0Bindings[] = {
                {
                    .m_type = DescriptorBindingDesc::Type::ConstantBuffer,
                    .m_visibility = ShaderVisibility::Vertex | ShaderVisibility::Fragment
                }
            };
            commonDescriptorSetLayout = _graphicsContext.CreateDescriptorSetLayout(
                { .m_bindings = descriptorSet0Bindings },
                m_commonDescriptorSetIndices.data());

            constexpr DescriptorBindingDesc descriptorSet1Bindings[] = {
                {
                    .m_type = DescriptorBindingDesc::Type::SampledTexture,
                    .m_visibility = ShaderVisibility::Fragment,
                    .m_count = kMaxTextureSlots,
                },
                {
                    .m_type = DescriptorBindingDesc::Type::Sampler,
                    .m_visibility = ShaderVisibility::Fragment,
                    .m_count = kMaxSamplerSlots,
                }
            };
            texturesDescriptorSetLayout = _graphicsContext.CreateDescriptorSetLayout(
                { .m_bindings = descriptorSet1Bindings },
                m_texturesDescriptorSetIndices.data());

            const DescriptorSetLayoutHandle descriptorSetLayouts[] = { commonDescriptorSetLayout, texturesDescriptorSetLayout };
            m_commonPipelineLayout = _graphicsContext.CreatePipelineLayout({
                .m_descriptorSets = descriptorSetLayouts,
            });

            m_commonDescriptorSet = _graphicsContext.CreateDescriptorSet(commonDescriptorSetLayout);
            m_texturesDescriptorSet = _graphicsContext.CreateDescriptorSet(texturesDescriptorSetLayout);
        }

        constexpr VertexLayoutElement commonVertexElements[] = {
            // Packed rect
            {
                .m_semanticName = VertexLayoutElement::SemanticName::Position,
                .m_semanticIndex = 0,
                .m_bindingIndex = 0,
                .m_format = TextureFormat::RG32_UInt,
                .m_offset = 0,
                .m_location = 0,
            },
            // Packed data
            {
                .m_semanticName = VertexLayoutElement::SemanticName::Uv,
                .m_semanticIndex = 0,
                .m_bindingIndex = 0,
                .m_format = TextureFormat::RGBA32_UInt,
                .m_offset = sizeof(uint2),
                .m_location = 1,
            },
        };

        constexpr VertexBindingDesc commonVertexBindings[] = {
            {
                .m_stride = sizeof(PackedInstanceData),
                .m_inputRate = VertexInputRate::Instance,
            }
        };

        const auto readShaderFile = [_allocator](const eastl::string_view _filePath) -> eastl::span<char>
        {
            std::ifstream file(_filePath.data(), std::ios::binary);
            VERIFY_OR_RETURN(file, {});

            file.seekg(0, std::ios::end);
            const std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            auto* buffer = _allocator.Allocate<char>(size);
            KE_VERIFY(file.read(buffer, size));

            file.close();

            return { buffer, static_cast<size_t>(size) };
        };

        // Rectangle PSO
        {
            const eastl::span<char> vertexShaderSource = readShaderFile(
                eastl::string("Shaders/BasicGuiRenderer/Rectangle_RectangleVs.", _allocator) + GraphicsContext::GetShaderFileExtension());
            const eastl::span<char> fragmentShaderSource = readShaderFile(
                eastl::string("Shaders/BasicGuiRenderer/Rectangle_RectangleFs.", _allocator) + GraphicsContext::GetShaderFileExtension());

            const ShaderModuleHandle vertexShaderModule = _graphicsContext.RegisterShaderModule(vertexShaderSource.data(), vertexShaderSource.size());
            const ShaderModuleHandle fragmentShaderModule = _graphicsContext.RegisterShaderModule(fragmentShaderSource.data(), fragmentShaderSource.size());

            const ShaderStage stages[] = {
                {
                    .m_shaderModule = vertexShaderModule,
                    .m_stage = ShaderStage::Stage::Vertex,
                    .m_entryPoint = "RectangleVs",
                },
                {
                    .m_shaderModule = fragmentShaderModule,
                    .m_stage = ShaderStage::Stage::Fragment,
                    .m_entryPoint = "RectangleFs",
                },
            };

            const GraphicsPipelineDesc pipelineDesc {
                .m_stages = stages,
                .m_vertexInput = {
                    .m_elements = commonVertexElements,
                    .m_bindings = commonVertexBindings
                },
                .m_colorBlending = {
                    .m_attachments = { kDefaultColorAttachmentAlphaBlendDesc }
                },
                .m_depthStencil = {
                    .m_depthTest = false,
                    .m_depthWrite = false,
                },
                .m_renderPass =  _renderPass,
                .m_pipelineLayout = m_commonPipelineLayout,
            };
            m_rectanglePipeline = _graphicsContext.CreateGraphicsPipeline(pipelineDesc);

            _graphicsContext.FreeShaderModule(fragmentShaderModule);
            _graphicsContext.FreeShaderModule(vertexShaderModule);
            _allocator.deallocate(fragmentShaderSource.data(), fragmentShaderSource.size());
            _allocator.deallocate(vertexShaderSource.data(), vertexShaderSource.size());
        }

        _graphicsContext.DestroyDescriptorSetLayout(texturesDescriptorSetLayout);
        _graphicsContext.DestroyDescriptorSetLayout(commonDescriptorSetLayout);
    }

    void BasicGuiRenderer::BeginLayout(const float4x4& _viewportTransform, const uint2& _viewportSize)
    {
        Clay_BeginLayout();

        m_viewportConstants.ndcProjectionMatrix = _viewportTransform;
        m_viewportConstants.viewportSize = float2(_viewportSize);
    }

    void BasicGuiRenderer::EndLayoutAndRender(
        GraphicsContext& _graphicsContext,
        CommandListHandle _transferCommandList,
        CommandListHandle _renderCommandList)
    {
        const Clay_RenderCommandArray renderCommandArray = Clay_EndLayout();

        const u8 frameIndex = _graphicsContext.GetCurrentFrameContextIndex();

        {
            KE_ASSERT(m_commonConstantBuffer.GetSize(frameIndex) == sizeof(ViewportConstants));

            memcpy(m_commonConstantBuffer.Map(&_graphicsContext, frameIndex), &m_viewportConstants, sizeof(ViewportConstants));
            m_commonConstantBuffer.Unmap(&_graphicsContext);
            m_commonConstantBuffer.PrepareBuffers(&_graphicsContext, _transferCommandList, BarrierAccessFlags::ConstantBuffer, frameIndex);

            const DescriptorSetWriteInfo::DescriptorData descriptorData { .m_handle = m_commonConstantBufferViews[frameIndex].m_handle };
            const DescriptorSetWriteInfo writes[] = {
                {
                    .m_index = m_commonDescriptorSetIndices[0],
                    .m_descriptorData = { &descriptorData, 1 },
                }
            };
            _graphicsContext.UpdateDescriptorSet(m_commonDescriptorSet, writes, true);
            _graphicsContext.DeclarePassBufferViewUsage(_renderCommandList, { &m_commonConstantBufferViews[frameIndex], 1 }, BufferViewAccessType::Read);
        }

        const size_t sizeEstimation = sizeof(PackedInstanceData) * renderCommandArray.length;
        const u64 sizeRequirement = Alignment::NextPowerOfTwo(sizeEstimation);
        if (m_instanceDataBuffer.GetSize(frameIndex) < sizeRequirement)
        {
            m_instanceDataBuffer.RequestResize(sizeRequirement);
        }
        auto* buffer = static_cast<std::byte*>(m_instanceDataBuffer.Map(&_graphicsContext, frameIndex));
        size_t offset = 0;

        constexpr auto packRect = [](const Clay_BoundingBox& _boundingBox)
        {
            uint2 packedRect {};
            const float2 halfSize = float2(0.5f * _boundingBox.width, 0.5f * _boundingBox.height);
            const float2 center = float2(_boundingBox.x, _boundingBox.y) + halfSize;

            using Math::Float16;
            packedRect.x = Float16::PackFloat16x2(center.x, center.y);
            packedRect.y = Float16::PackFloat16x2(halfSize.x, halfSize.y);

            return packedRect;
        };
        constexpr auto packCornerRadii = [](const Clay_CornerRadius& _cornerRadius)
        {
            uint2 packedRadii {};
            using Math::Float16;
            packedRadii.x = Float16::PackFloat16x2(_cornerRadius.topLeft, _cornerRadius.topRight);
            packedRadii.y = Float16::PackFloat16x2(_cornerRadius.bottomLeft, _cornerRadius.bottomRight);
            return packedRadii;
        };

        Clay_RenderCommandType previous = CLAY_RENDER_COMMAND_TYPE_NONE;

        const BufferSpan bufferView {
            .m_size = sizeEstimation,
            .m_stride = sizeof(PackedInstanceData),
            .m_buffer = m_instanceDataBuffer.GetBuffer(frameIndex),
        };
        _graphicsContext.SetVertexBuffers(_renderCommandList, { &bufferView, 1 });

        const DescriptorSetHandle descriptorSets[] = { m_commonDescriptorSet, m_texturesDescriptorSet };
        _graphicsContext.SetGraphicsDescriptorSets(_renderCommandList, m_commonPipelineLayout, descriptorSets);

        _graphicsContext.SetViewport(_renderCommandList, {
            .m_width = static_cast<s32>(m_viewportConstants.viewportSize.x),
            .m_height = static_cast<s32>(m_viewportConstants.viewportSize.y)
        });
        _graphicsContext.SetScissorsRect(_renderCommandList, {
            .m_left = 0,
            .m_top = 0,
            .m_right = static_cast<u32>(m_viewportConstants.viewportSize.x),
            .m_bottom = static_cast<u32>(m_viewportConstants.viewportSize.y)
        });

        for (u32 i = 0; i < renderCommandArray.length; i++)
        {
            const Clay_RenderCommand& renderCommand = renderCommandArray.internalArray[i];

            switch (renderCommand.commandType)
            {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            {
                auto* packedInstanceData = reinterpret_cast<PackedInstanceData*>(buffer + offset);
                packedInstanceData->m_packedRect = packRect(renderCommand.boundingBox);
                const uint2 packedRadii = packCornerRadii(renderCommand.renderData.rectangle.cornerRadius);
                packedInstanceData->m_packedData.x = packedRadii.x;
                packedInstanceData->m_packedData.y = packedRadii.y;
                packedInstanceData->m_packedData.z = Color(
                    renderCommand.renderData.rectangle.backgroundColor.r / 255.f,
                    renderCommand.renderData.rectangle.backgroundColor.g / 255.f,
                    renderCommand.renderData.rectangle.backgroundColor.b / 255.f,
                    renderCommand.renderData.rectangle.backgroundColor.a / 255.f).ToSrgb().ToRgba8();
                offset += sizeof(PackedInstanceData);

                if (previous != CLAY_RENDER_COMMAND_TYPE_RECTANGLE)
                {
                    _graphicsContext.SetGraphicsPipeline(_renderCommandList, m_rectanglePipeline);
                }
                _graphicsContext.DrawInstanced(_renderCommandList, DrawInstancedDesc {
                    .m_vertexCount = 6,
                    .m_instanceOffset = static_cast<u32>(offset / sizeof(PackedInstanceData)),
                });

                previous = CLAY_RENDER_COMMAND_TYPE_RECTANGLE;
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER:
                previous = CLAY_RENDER_COMMAND_TYPE_BORDER;
                break;
            case CLAY_RENDER_COMMAND_TYPE_TEXT:
                previous = CLAY_RENDER_COMMAND_TYPE_TEXT;
                break;
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:
                previous = CLAY_RENDER_COMMAND_TYPE_IMAGE;
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
                previous = CLAY_RENDER_COMMAND_TYPE_SCISSOR_START;
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                previous = CLAY_RENDER_COMMAND_TYPE_SCISSOR_END;
                break;
            case CLAY_RENDER_COMMAND_TYPE_NONE:
                previous = CLAY_RENDER_COMMAND_TYPE_NONE;
                break;
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
                previous = CLAY_RENDER_COMMAND_TYPE_CUSTOM;
                break;
            }
        }

        m_instanceDataBuffer.Unmap(&_graphicsContext);
        m_instanceDataBuffer.PrepareBuffers(&_graphicsContext, _transferCommandList, BarrierAccessFlags::VertexBuffer, frameIndex);
    }

} // namespace KryneEngine