/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#include <fstream>

#include <KryneEngine/Core/Graphics/Common/Buffer.hpp>
#include <KryneEngine/Core/Graphics/Common/Drawing.hpp>
#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/Common/RenderPass.hpp>
#include <KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Core/Window/Window.hpp>

using namespace KryneEngine;

void PrepareRenderPasses(GraphicsContext& _graphicsContext, DynamicArray<RenderPassHandle>& _handles)
{
    _handles.Resize(_graphicsContext.GetFrameContextCount());
    for (auto i = 0u; i < _handles.Size(); i++)
    {
        RenderPassDesc desc;
        desc.m_colorAttachments.push_back(RenderPassDesc::Attachment {
            KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear,
            KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store,
            TextureLayout::Unknown,
            TextureLayout::Present,
            _graphicsContext.GetPresentRenderTargetView(i),
            float4(0, 1, 1, 1) // Cyan color
        });
#if !defined(KE_FINAL)
        desc.m_debugName.sprintf("PresentRenderPass[%d]", i);
#endif
        _handles[i] = _graphicsContext.CreateRenderPass(desc);
    }
}

void PreparePso(
    GraphicsContext& _graphicsContext,
    eastl::vector<u8>& _vsBytecode,
    eastl::vector<u8>& _psBytecode,
    ShaderModuleHandle& _vsModule,
    ShaderModuleHandle& _psModule,
    PipelineLayoutHandle& _layout,
    GraphicsPipelineHandle& _pso,
    RenderPassHandle _renderPass)
{
    // Load shader bytecode
    {
        constexpr auto readShaderFile = [](const auto& _path, auto& _vec)
        {
            std::ifstream file(_path.c_str(), std::ios::binary);
            VERIFY_OR_RETURN_VOID(file);

            file.seekg(0, std::ios::end);
            _vec.resize(file.tellg());
            file.seekg(0, std::ios::beg);

            KE_VERIFY(file.read(reinterpret_cast<char*>(_vec.data()), _vec.size()));
        };

        readShaderFile(
            eastl::string("Shaders/Triangle_MainVS.") + GraphicsContext::GetShaderFileExtension(),
            _vsBytecode);
        readShaderFile(
            eastl::string("Shaders/Triangle_MainPS.") + GraphicsContext::GetShaderFileExtension(),
            _psBytecode);
    }

    // Register modules
    {
        _vsModule = _graphicsContext.RegisterShaderModule(_vsBytecode.data(), _vsBytecode.size());
        _psModule = _graphicsContext.RegisterShaderModule(_psBytecode.data(), _psBytecode.size());
    }

    // Register layout
    {
        _layout = _graphicsContext.CreatePipelineLayout({});
    }

    {
        _pso = _graphicsContext.CreateGraphicsPipeline({
            .m_stages = {
                GraphicsShaderStage {
                    .m_shaderModule = _vsModule,
                    .m_stage = KryneEngine::GraphicsShaderStage::Stage::Vertex,
                    .m_entryPoint = "MainVS",
                },
                GraphicsShaderStage {
                    .m_shaderModule = _psModule,
                    .m_stage = KryneEngine::GraphicsShaderStage::Stage::Fragment,
                    .m_entryPoint = "MainPS",
                },
            },
            .m_vertexInput = {
                .m_elements = {
                    // Position element
                    VertexLayoutElement {
                        .m_semanticName = KryneEngine::VertexLayoutElement::SemanticName::Position,
                        .m_semanticIndex = 0,
                        .m_format = KryneEngine::TextureFormat::RGB32_Float,
                        .m_offset = 0,
                        .m_location = 0,
                    },
                    // Color element
                    VertexLayoutElement {
                        .m_semanticName = KryneEngine::VertexLayoutElement::SemanticName::Color,
                        .m_semanticIndex = 0,
                        .m_format = KryneEngine::TextureFormat::RGB32_Float,
                        .m_offset = sizeof(float3),
                        .m_location = 1,
                    },
                },
                .m_bindings = {
                    VertexBindingDesc { .m_stride = sizeof(float3) * 2 },
                },
            },
            .m_colorBlending = {
                .m_attachments = {
                    ColorAttachmentBlendDesc {},
                }
            },
            .m_depthStencil = { .m_depthTest = false, .m_depthWrite = false },
            .m_renderPass = _renderPass,
            .m_pipelineLayout = _layout,
#if !defined(KE_FINAL)
            .m_debugName = "Triangle PSO",
#endif
        });
    }
}

void PrepareBuffers(
    GraphicsContext& _graphicsContext,
    BufferHandle& _vertexBuffer,
    BufferHandle& _indexBuffer,
    BufferView& _vertexBufferView,
    BufferView& _indexBufferView)
{
    constexpr float vertexData[] =
    {
        // Vertex 0
        -0.5f, -0.5f, 0.f, // Position
        1.f, 0.f, 0.f,     // Color
        // Vertex 1
        0.5f, -0.5f, 0.f,
        0.f, 1.f, 0.f,
        // Vertex 2
        0.f, 0.5f, 0.f,
        0.f, 0.f, 1.f,
    };
    constexpr s32 indexData[] = { 0, 1, 2 };

    // Initialize vertex buffer resources
    {
        _vertexBuffer = _graphicsContext.CreateBuffer({
            .m_desc =
                {
                    .m_size = sizeof(vertexData),
#if !defined(KE_FINAL)
                    .m_debugName = "Vertex buffer",
#endif
                },
            .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::VertexBuffer | MemoryUsage::TransferDstBuffer,
        });
        _vertexBufferView = {
            .m_size = sizeof(vertexData),
            .m_stride = sizeof(float) * (3 + 3),
            .m_buffer = _vertexBuffer,
        };
    }

    // Initialize index buffer resources
    {
        _indexBuffer = _graphicsContext.CreateBuffer({
            .m_desc =
                {
                    .m_size = sizeof(indexData),
#if !defined(KE_FINAL)
                    .m_debugName = "Index buffer",
#endif
                },
            .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::IndexBuffer | MemoryUsage::TransferDstBuffer,
        });
        _indexBufferView = {
            .m_size = sizeof(indexData),
            .m_stride = sizeof(u32),
            .m_buffer = _indexBuffer,
        };
    }

    // Upload buffer data
    // We'll use a single staging buffer for this, for demo purposes
    {
        BufferHandle stagingBuffer = _graphicsContext.CreateBuffer(
            {
                .m_desc = {
                    .m_size = _vertexBufferView.m_size + _indexBufferView.m_size,
#if !defined(KE_FINAL)
    .m_debugName = "Staging buffer",
#endif
                },
                .m_usage = MemoryUsage::StageEveryFrame_UsageType | MemoryUsage::TransferSrcBuffer
            });

        BufferMapping mapping { stagingBuffer };
        _graphicsContext.MapBuffer(mapping);
        memcpy(mapping.m_ptr, vertexData, sizeof(vertexData));
        memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(mapping.m_ptr) + sizeof(vertexData)), indexData, sizeof(indexData));
        _graphicsContext.UnmapBuffer(mapping);

        // Upload operation
        {
            // We create a single use command buffer, just for this operation, then discard it.
            // This is used here only for demonstration purposes. In a real-time scenario, you should avoid creating
            // command buffers for each trivial operations, and group them into one single buffer.

            CommandListHandle commandList = _graphicsContext.BeginGraphicsCommandList();

            _graphicsContext.CopyBuffer(
                commandList,
                {
                    .m_copySize = _vertexBufferView.m_size,
                    .m_bufferSrc = stagingBuffer,
                    .m_bufferDst = _vertexBuffer,
                });
            _graphicsContext.CopyBuffer(
                commandList,
                {
                    .m_copySize = _indexBufferView.m_size,
                    .m_bufferSrc = stagingBuffer,
                    .m_bufferDst = _indexBuffer,
                    .m_offsetSrc = _vertexBufferView.m_size,
                });

            _graphicsContext.EndGraphicsCommandList();
        }

        // Free staging buffer once everything is done.
        _graphicsContext.DestroyBuffer(stagingBuffer);
    }
}

int main()
{
    auto appInfo = GraphicsCommon::ApplicationInfo();
    appInfo.m_applicationName = "Hello triangle - Kryne Engine 2";
#if defined(KE_GRAPHICS_API_VK)
    appInfo.m_api = GraphicsCommon::Api::Vulkan_1_3;
    appInfo.m_applicationName += " - Vulkan";
#elif defined(KE_GRAPHICS_API_DX12)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::DirectX12_1;
    appInfo.m_applicationName += " - DirectX 12";
#elif defined(KE_GRAPHICS_API_MTL)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::Metal_3;
    appInfo.m_applicationName += " - Metal";
#endif

    Window mainWindow(appInfo, AllocatorInstance());
    GraphicsContext* graphicsContext = mainWindow.GetGraphicsContext();

    // Declare resources
    DynamicArray<RenderPassHandle> renderPassHandles;
    eastl::vector<u8> vsBytecode, psBytecode;
    ShaderModuleHandle vsModule, psModule;
    PipelineLayoutHandle trianglePipelineLayout;
    GraphicsPipelineHandle trianglePso;
    BufferHandle vertexBuffer, indexBuffer;
    BufferView vertexBufferView, indexBufferView;

    // Prepare resources
    PrepareRenderPasses(*graphicsContext, renderPassHandles);
    PreparePso(*graphicsContext, vsBytecode, psBytecode, vsModule, psModule, trianglePipelineLayout, trianglePso, renderPassHandles[0]);
    PrepareBuffers(*graphicsContext, vertexBuffer, indexBuffer, vertexBufferView, indexBufferView);

    do
    {
        KE_ZoneScoped("Main loop");

        CommandListHandle commandList = graphicsContext->BeginGraphicsCommandList();

        const u8 index = graphicsContext->GetCurrentPresentImageIndex();
        graphicsContext->BeginRenderPass(commandList, renderPassHandles[index]);

        graphicsContext->SetVertexBuffers(commandList, { &vertexBufferView, 1 });
        graphicsContext->SetIndexBuffer(commandList, indexBufferView);
        graphicsContext->SetGraphicsPipeline(commandList, trianglePso);
        graphicsContext->SetViewport(
            commandList,
            {
                .m_width = appInfo.m_displayOptions.m_width,
                .m_height = appInfo.m_displayOptions.m_height,
            });
        graphicsContext->SetScissorsRect(
            commandList,
            {
                .m_left = 0,
                .m_top = 0,
                .m_right = appInfo.m_displayOptions.m_width,
                .m_bottom = appInfo.m_displayOptions.m_height,
            });
        graphicsContext->DrawIndexedInstanced(
            commandList,
            {
                .m_elementCount = 3
            });

        graphicsContext->EndRenderPass(commandList);

        graphicsContext->EndGraphicsCommandList();
    }
    while (graphicsContext->EndFrame());
}