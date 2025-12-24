/**
 * @file
 * @author Max Godefroy
 * @date 24/11/2025.
 */

#include <KryneEngine/Core/Graphics/GraphicsContext.hpp>
#include <KryneEngine/Core/Graphics/RenderPass.hpp>
#include <KryneEngine/Core/Memory/Allocators/TlsfAllocator.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
#include <KryneEngine/Core/Window/Window.hpp>
#include <KryneEngine/Modules/GuiLib/Context.hpp>
#include <KryneEngine/Modules/GuiLib/GuiRenderers/BasicGuiRenderer.hpp>

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

    KryneEngine::Modules::GuiLib::Context clayContext { allocatorInstance };
    KryneEngine::Modules::GuiLib::BasicGuiRenderer guiRenderer {
        allocatorInstance,
        *graphicsContext,
        renderPassHandles[0]
    };
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

        clayContext.BeginLayout({
            graphicsContext->GetApplicationInfo().m_displayOptions.m_width,
            graphicsContext->GetApplicationInfo().m_displayOptions.m_height
        });

        // An example of laying out a UI with a fixed width sidebar and flexible width main content
        CLAY({ .id = CLAY_ID("OuterContainer"), .layout = { .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = CLAY_PADDING_ALL(16), .childGap = 16 }, .backgroundColor = {250,250,255,255} }) {
            CLAY({
                .id = CLAY_ID("SideBar"),
                .layout = { .sizing = { .width = CLAY_SIZING_FIXED(300), .height = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(16), .childGap = 16, .layoutDirection = CLAY_TOP_TO_BOTTOM },
                .backgroundColor = COLOR_LIGHT
            }) {
                CLAY({ .id = CLAY_ID("ProfilePictureOuter"), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(16), .childGap = 16, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } }, .backgroundColor = COLOR_RED }) {
                    CLAY({ .id = CLAY_ID("ProfilePicture"), .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60), .height = CLAY_SIZING_FIXED(60) }}, .image = { .imageData = nullptr } }) {}
                    CLAY_TEXT(CLAY_STRING("Clay - UI Library"), CLAY_TEXT_CONFIG({  .textColor = {255, 255, 255, 255}, .fontSize = 24 }));
                }

                // Standard C code like loops etc work inside components
                for (int i = 0; i < 5; i++) {
                    SidebarItemComponent();
                }

                CLAY({ .id = CLAY_ID("MainContent"), .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } }, .backgroundColor = COLOR_LIGHT }) {}
            }
        }

        const KryneEngine::RenderPassHandle currentPass = renderPassHandles[graphicsContext->GetCurrentPresentImageIndex()];
        graphicsContext->BeginRenderPass(renderCommandList, currentPass);
        clayContext.EndLayout(*graphicsContext, transferCommandList, renderCommandList);
        graphicsContext->EndRenderPass(renderCommandList);
    }
    while (graphicsContext->EndFrame());

    clayContext.Destroy();
    KryneEngine::GraphicsContext::Destroy(graphicsContext);

    return 0;
}