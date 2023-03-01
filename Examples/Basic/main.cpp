#include <iostream>
#include <Graphics/Common/GraphicsContext.hpp>
#include <Threads/FibersManager.hpp>
#include <Threads/Semaphore.hpp>
#include <Threads/RwMutex.hpp>

std::atomic<KryneEngine::u32> counter = 0;

static constexpr KryneEngine::u32 kCount = 100;
KryneEngine::FiberJob counterJobs[kCount];

void Job0(void* _data)
{
    counter++;
}

void Job1(void*)
{
    std::cout << "Counter value: " << counter << std::endl;

    auto* fibersManager = KryneEngine::FibersManager::GetInstance();

    auto syncCounter = fibersManager->InitAndBatchJobs(counterJobs, Job0, nullptr, kCount);

    fibersManager->WaitForCounterAndReset(syncCounter);

    std::cout << "Counter value: " << counter << std::endl;
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    {
        KryneEngine::BusySpinSemaphore semaphore(2);
        KryneEngine::SpinLock lock;

        lock.Lock();

        std::thread j0([&]() {
            auto sLock = semaphore.AutoLock();
            std::cout << "J 0 semaphore go" << std::endl;
            lock.Lock();
            semaphore.SignalOnce();
            lock.Unlock();
        });
        std::thread j1([&]() {
            semaphore.Wait();
            std::cout << "J 1 semaphore go" << std::endl;
            lock.Lock();
            semaphore.SignalOnce();
            lock.Unlock();
        });
        std::thread j2([&]() {
            semaphore.Wait();
            std::cout << "J 2 semaphore go" << std::endl;
            lock.Unlock();
            semaphore.SignalOnce();
        });

        j0.join();
        j1.join();
        j2.join();
    }

    {
        auto fibersManager = KryneEngine::FibersManager(6);

        KryneEngine::FiberJob initJob;
        auto syncCounter = fibersManager.InitAndBatchJobs(&initJob, Job1, nullptr);

        fibersManager.ResetCounter(syncCounter);

        auto appInfo = KryneEngine::GraphicsCommon::ApplicationInfo();
        appInfo.m_api = KryneEngine::GraphicsCommon::Api::Vulkan_1_2;
        KryneEngine::GraphicsContext graphicsContext(appInfo);

        while (graphicsContext.EndFrame());
    }

    return 0;
}
