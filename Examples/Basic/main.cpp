#include <Graphics/Common/GraphicsContext.hpp>
#include <Graphics/Common/RenderPass.hpp>
#include <ImGui/Context.hpp>
#include <Threads/FibersManager.hpp>
#include <Window/Window.hpp>
#include <iostream>

using namespace KryneEngine;
namespace KEModules = KryneEngine::Modules;

void Job0(void* _counterPtr)
{
    (*static_cast<std::atomic<u32>*>(_counterPtr))++;
}

void Job1(void*)
{
    std::atomic<u32> counter = 0;

    std::cout << "Counter value: " << counter << std::endl;

    auto* fibersManager = FibersManager::GetInstance();

    static constexpr u32 kCount = 100;
    FiberJob counterJobs[kCount];

    const auto syncCounter = fibersManager->InitAndBatchJobs(counterJobs, Job0, &counter, kCount);

    fibersManager->WaitForCounterAndReset(syncCounter);

    std::cout << "Counter value: " << counter << std::endl;
}

int main() {
    std::cout << "Hello, World!" << std::endl;

//    {
//	    BusySpinSemaphore semaphore(2);
//	    SpinLock lock;
//
//        lock.Lock();
//
//        std::thread j0([&]() {
//            {
//                auto sLock = semaphore.AutoLock();
//            }
//            std::cout << "J 0 semaphore go" << std::endl;
//            lock.Lock();
//            semaphore.SignalOnce();
//            lock.Unlock();
//        });
//        std::thread j1([&]() {
//            semaphore.Wait();
//            std::cout << "J 1 semaphore go" << std::endl;
//            lock.Lock();
//            semaphore.SignalOnce();
//            lock.Unlock();
//        });
//        std::thread j2([&]() {
//            semaphore.Wait();
//            std::cout << "J 2 semaphore go" << std::endl;
//            lock.Unlock();
//            semaphore.SignalOnce();
//        });
//
//        j0.join();
//        j1.join();
//        j2.join();
//    }

    {
        auto fibersManager = FibersManager(6);

//        FiberJob initJob;
//        const auto syncCounter = fibersManager.InitAndBatchJobs(&initJob, Job1, nullptr);
//
//        fibersManager.ResetCounter(syncCounter);

        auto appInfo = GraphicsCommon::ApplicationInfo();
//        appInfo.m_features.m_validationLayers = false;
        appInfo.m_applicationName = "Basic Example - Kryne Engine 2";
#if defined(KE_GRAPHICS_API_VK)
        appInfo.m_api = GraphicsCommon::Api::Vulkan_1_3;
        appInfo.m_applicationName += " - Vulkan";
#elif defined(KE_GRAPHICS_API_DX12)
        appInfo.m_api = KryneEngine::GraphicsCommon::Api::DirectX12_1;
        appInfo.m_applicationName += " - DirectX 12";
#endif
        Window mainWindow(appInfo);
        GraphicsContext* graphicsContext = mainWindow.GetGraphicsContext();

        DynamicArray<RenderPassHandle> renderPassHandles;
        renderPassHandles.Resize(graphicsContext->GetFrameContextCount());
        for (auto i = 0u; i < renderPassHandles.Size(); i++)
        {
            RenderPassDesc desc;
            desc.m_colorAttachments.push_back(RenderPassDesc::Attachment {
                KryneEngine::RenderPassDesc::Attachment::LoadOperation::Clear,
                KryneEngine::RenderPassDesc::Attachment::StoreOperation::Store,
                TextureLayout::Unknown,
                TextureLayout::Present,
                graphicsContext->GetPresentRenderTargetView(i),
                float4(0, 1, 1, 1)
            });
#if !defined(KE_FINAL)
            desc.m_debugName.sprintf("PresentRenderPass[%d]", i);
#endif
            renderPassHandles[i] = graphicsContext->CreateRenderPass(desc);
        }

        KEModules::ImGui::Context imGuiContext{&mainWindow, renderPassHandles[0]};

        do
        {
            CommandList commandList = graphicsContext->BeginGraphicsCommandList();

            imGuiContext.NewFrame(&mainWindow, commandList);

            {
                static bool open;
                ImGui::ShowDemoWindow(&open);
            }

            imGuiContext.PrepareToRenderFrame(graphicsContext, commandList);

            const u8 index = graphicsContext->GetCurrentPresentImageIndex();
            graphicsContext->BeginRenderPass(commandList, renderPassHandles[index]);

            imGuiContext.RenderFrame(graphicsContext, commandList);

            graphicsContext->EndRenderPass(commandList);

            graphicsContext->EndGraphicsCommandList();
        }
        while (graphicsContext->EndFrame());

        graphicsContext->WaitForLastFrame();

        imGuiContext.Shutdown(&mainWindow);

        for (auto handle: renderPassHandles)
        {
            graphicsContext->DestroyRenderPass(handle);
        }
    }

    return 0;
}
