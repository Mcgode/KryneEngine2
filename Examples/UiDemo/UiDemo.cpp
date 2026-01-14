/**
 * @file
 * @author Max Godefroy
 * @date 24/11/2025.
 */

#include "KryneEngine/Modules/TextRendering/Font.hpp"
#include "KryneEngine/Modules/TextRendering/MsdfAtlasManager.hpp"
#include "TextureGenerator.hpp"


#include <KryneEngine/Core/Graphics/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/RenderPass.hpp>
#include <KryneEngine/Core/Memory/Allocators/TlsfAllocator.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Core/Window/Window.hpp>
#include <KryneEngine/Modules/GuiLib/Context.hpp>
#include <KryneEngine/Modules/GuiLib/GuiRenderers/BasicGuiRenderer.hpp>
#include <KryneEngine/Modules/TextRendering/FontManager.hpp>

using KryneEngine::s32;


const Clay_Color COLOR_LIGHT = (Clay_Color) {224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color) {168, 66, 28, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color) {225, 138, 50, 255};

// Layout config is just a struct that can be declared statically, or inline
Clay_ElementDeclaration sidebarItemConfig = (Clay_ElementDeclaration) {
    .layout = {
        .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) }
    },
    .backgroundColor = COLOR_ORANGE
};

// Re-useable components are just normal functions
void SidebarItemComponent() {
    CLAY(sidebarItemConfig) {
        // children go here...
    }
}

s32 main(s32 argc, const char** argv)
{
    auto* allocator = KryneEngine::TlsfAllocator::Create({}, 16 << 20);
    KryneEngine::AllocatorInstance allocatorInstance {allocator};

    auto appInfo = KryneEngine::GraphicsCommon::ApplicationInfo {
        .m_applicationName { "UiDemo - Kryne Engine 2", allocator }
    };
#if defined(KE_GRAPHICS_API_VK)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::Vulkan_1_3;
    appInfo.m_applicationName += " - Vulkan";
#elif defined(KE_GRAPHICS_API_DX12)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::DirectX12_1;
    appInfo.m_applicationName += " - DirectX 12";
#elif defined(KE_GRAPHICS_API_MTL)
    appInfo.m_api = KryneEngine::GraphicsCommon::Api::Metal_3;
    appInfo.m_applicationName += " - Metal";
#endif
    KryneEngine::Window mainWindow(appInfo, allocator);
    KryneEngine::GraphicsContext* graphicsContext = mainWindow.GetGraphicsContext();

    TextureGenerator textureGenerator { allocatorInstance, 33 };
    KryneEngine::SamplerHandle sampler = graphicsContext->CreateSampler({
        .m_minFilter = SamplerDesc::Filter::Point,
        .m_magFilter = SamplerDesc::Filter::Point,
    });

    KryneEngine::DynamicArray<KryneEngine::RenderPassHandle> renderPassHandles(allocator);
    renderPassHandles.Resize(graphicsContext->GetFrameContextCount());
    for (auto i = 0u; i < renderPassHandles.Size(); i++)
    {
        renderPassHandles[i] = graphicsContext->CreateRenderPass({
            .m_colorAttachments = {
                KryneEngine::RenderPassDesc::Attachment {
                    .m_loadOperation = KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear,
                    .m_storeOperation = KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store,
                    .m_finalLayout = KryneEngine::TextureLayout::Present,
                    .m_rtv = graphicsContext->GetPresentRenderTargetView(i),
                }
            },
#if !defined(KE_FINAL)
            .m_debugName = "Main render pass",
#endif
        });
    }

    KryneEngine::Modules::TextRendering::FontManager fontManager(allocatorInstance);
    KryneEngine::Modules::TextRendering::MsdfAtlasManager msdfAtlasManager(allocatorInstance, *graphicsContext, &fontManager, 1024, 32);
    KryneEngine::Modules::TextRendering::Font* font = fontManager.LoadFont("Resources/Modules/TextRendering/NotoSerif-Regular.ttf");

    KryneEngine::Modules::GuiLib::Context clayContext { allocatorInstance, &fontManager };
    KryneEngine::Modules::GuiLib::BasicGuiRenderer guiRenderer {
        allocatorInstance,
        *graphicsContext,
        renderPassHandles[0],
        sampler
    };
    guiRenderer.SetAtlasManager(&msdfAtlasManager);
    clayContext.Initialize(
        &guiRenderer,
        {
            graphicsContext->GetApplicationInfo().m_displayOptions.m_width,
            graphicsContext->GetApplicationInfo().m_displayOptions.m_height
        });

    do
    {
        KE_ZoneScoped("Render loop");

        KryneEngine::CommandListHandle transferCommandList = graphicsContext->BeginGraphicsCommandList();
        KryneEngine::CommandListHandle renderCommandList = graphicsContext->BeginGraphicsCommandList();

        {
            KE_ZoneScoped("Texture upload");
            textureGenerator.HandleUpload(*graphicsContext, transferCommandList);
        }

        clayContext.BeginLayout({
            graphicsContext->GetApplicationInfo().m_displayOptions.m_width,
            graphicsContext->GetApplicationInfo().m_displayOptions.m_height
        });

        // An example of laying out a UI with a fixed width sidebar and flexible width main content
        CLAY({ .id = CLAY_ID("OuterContainer"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = CLAY_PADDING_ALL(16), .childGap = 16 }, .backgroundColor = {250,250,255,255}, .cornerRadius = { 10, 20, 40, 0 } }) {
            CLAY({
                .id = CLAY_ID("SideBar"),
                .layout = { .sizing = { .width = CLAY_SIZING_FIXED(300), .height = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(16), .childGap = 16, .layoutDirection = CLAY_TOP_TO_BOTTOM },
                .backgroundColor = COLOR_LIGHT,
                .cornerRadius = {
                    .topLeft = 10,
                    .topRight = 20,
                    .bottomLeft = 40,
                    .bottomRight = 0
                }
            }) {
                CLAY({ .id = CLAY_ID("ProfilePictureOuter"), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(16), .childGap = 16, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } }, .backgroundColor = COLOR_RED }) {
                    CLAY({
                        .id = CLAY_ID("ProfilePicture"),
                        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(64), .height = CLAY_SIZING_FIXED(64) }},
                        .image = { .imageData = clayContext.RegisterTextureRegion({
                            .m_textureView = textureGenerator.GetTextureView(32),
                        })}
                    })
                    {}
                    CLAY_TEXT(CLAY_STRING("Clay - UI Library"), CLAY_TEXT_CONFIG({  .textColor = {255, 255, 255, 255}, .fontId = font->GetId(), .fontSize = 20 }));
                }

                // Standard C code like loops etc work inside components
                for (int i = 0; i < 5; i++) {
                    SidebarItemComponent();
                }
            }

            CLAY({
                .id = CLAY_ID("MainContent"),
                .layout = {
                    .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                    .padding = { 16, 16, 16, 16 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
                .backgroundColor = COLOR_LIGHT,
                .cornerRadius = {
                    .topLeft = 10,
                    .topRight = 20,
                    .bottomLeft = 40,
                    .bottomRight = 0
                },
                .border = {
                    .color = Clay_Color { 10, 0, 0, 255 },
                    .width = {
                        .left = 1,
                        .right = 1,
                        .top = 1,
                        .bottom = 10,
                    }
                },
            })
            {
                CLAY({ .layout = { .sizing = { .height = CLAY_SIZING_GROW() } } })
                {
                    CLAY_TEXT(CLAY_STRING("Mangé, à, è"), CLAY_TEXT_CONFIG({
                        .textColor = { 255, 80, 80, 255 },
                        .fontId = font->GetId(),
                        .fontSize = 60,
                        .letterSpacing = 2,
                        .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                    }));
                }

                for (auto i = 0; i < 4; i++)
                {
                    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW() }, .childGap = 16, .layoutDirection = CLAY_LEFT_TO_RIGHT } })
                    {
                        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW() } } }) {}
                        for (auto j = 0; j < 8; j++)
                        {
                            Clay_CornerRadius cornerRadius = {};

                            if (i == 0)
                            {
                                if (j == 0)
                                    cornerRadius.topLeft = 10;
                                else if (j == 7)
                                    cornerRadius.topRight = 10;
                            }
                            else if (i == 3)
                            {
                                if (j == 0)
                                    cornerRadius.bottomLeft = 10;
                                else if (j == 7)
                                    cornerRadius.bottomRight = 10;
                            }

                            CLAY({
                                .layout = { .sizing = { .width = CLAY_SIZING_FIXED(64), .height = CLAY_SIZING_FIXED(64) } },
                                .backgroundColor = Clay_Color( 180, 180, 180, 255 ),
                                .cornerRadius = cornerRadius,
                                .image = {
                                    .imageData = clayContext.RegisterTextureRegion({
                                        .m_textureView = textureGenerator.GetTextureView(i * 8 + j),
                                    }),
                                },
                            }) {}
                        }
                        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW() } } }) {}
                    }
                }
                CLAY({ .layout = { .sizing = { .height = CLAY_SIZING_GROW() } } })
                {
                    CLAY_TEXT(CLAY_STRING("日本語のグリフも表示できます!"), CLAY_TEXT_CONFIG({
                        .textColor = { 255, 80, 80, 255 },
                        .fontId = font->GetId(),
                        .fontSize = 60,
                        .letterSpacing = 2,
                        .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                    }));
                }
            }
        }

        const KryneEngine::RenderPassHandle currentPass = renderPassHandles[graphicsContext->GetCurrentPresentImageIndex()];
        graphicsContext->BeginRenderPass(renderCommandList, currentPass);
        clayContext.EndLayout(*graphicsContext, transferCommandList, renderCommandList);
        graphicsContext->EndRenderPass(renderCommandList);

        msdfAtlasManager.FlushLoads(*graphicsContext, transferCommandList);

        graphicsContext->EndGraphicsCommandList(transferCommandList);
        graphicsContext->EndGraphicsCommandList(renderCommandList);
    }
    while (graphicsContext->EndFrame());

    clayContext.Destroy();
    KryneEngine::GraphicsContext::Destroy(graphicsContext);

    return 0;
}