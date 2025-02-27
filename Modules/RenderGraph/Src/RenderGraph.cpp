/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#include "KryneEngine/Modules/RenderGraph/RenderGraph.hpp"

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Threads/FibersManager.hpp>

#include "KryneEngine/Modules/RenderGraph/Builder.hpp"
#include "KryneEngine/Modules/RenderGraph/Registry.hpp"

namespace KryneEngine::Modules::RenderGraph
{
    RenderGraph::RenderGraph()
    {
        m_registry = eastl::make_unique<Registry>();
    }

    RenderGraph::~RenderGraph() = default;

    Builder& RenderGraph::BeginFrame(GraphicsContext& _graphicsContext)
    {
        m_builder = eastl::make_unique<Builder>(GetRegistry());
        return *m_builder;
    }

    void RenderGraph::SubmitFrame(GraphicsContext& _graphicsContext, FibersManager& _fibersManager)
    {
        m_builder->PrintBuildResult();

        PassExecutionData passExecutionData = {
            .m_commandList = _graphicsContext.BeginGraphicsCommandList(),
        };

        JobData& currentJob = m_jobs.emplace_back();

        currentJob.m_renderGraph = this;
        currentJob.m_graphicsContext = &_graphicsContext;
        currentJob.m_passExecutionData = passExecutionData;
        currentJob.m_passRangeStart = 0;
        currentJob.m_passRangeCount = 0;

        for (size_t i = 0; i < m_builder->m_declaredPasses.size(); i++)
        {
            if (!m_builder->m_passAlive[i])
            {
                continue;
            }

            const PassDeclaration& pass = m_builder->m_declaredPasses[i];
            currentJob.m_passRangeCount = i - currentJob.m_passRangeStart + 1;

            // Reserve entry in map, so that duration saving is thread-safe
            m_currentFramePassPerformance.emplace(pass.m_name, 0);
        }

        // Execute the last job in this thread/fiber, schedule the other ones for dispatch.
        // Small optimization.
        if (m_jobs.size() > 1)
        {
            DynamicArray<FiberJob> fiberJobs(m_jobs.size() - 1);
            fiberJobs.InitAll();
            const SyncCounterId jobsCounter = _fibersManager.InitAndBatchJobs(
                fiberJobs.Size(),
                fiberJobs.Data(),
                ExecuteJob,
                m_jobs.data());
            ExecuteJob(&m_jobs.back());
            _fibersManager.WaitForCounterAndReset(jobsCounter);
        }
        else if (!m_jobs.empty())
        {
            ExecuteJob(&m_jobs.back());
        }
        m_jobs.clear();

        eastl::swap(m_previousFramePassPerformance, m_currentFramePassPerformance);
        m_currentFramePassPerformance.clear();

        m_previousFrameTotalDuration = m_currentFrameTotalDuration.load(std::memory_order_acquire);
        m_currentFrameTotalDuration.store(0, std::memory_order_relaxed);

        m_builder.reset();
    }

    void RenderGraph::ExecuteJob(void* _userData)
    {
        auto* jobData = static_cast<JobData*>(_userData);

        for (auto i = jobData->m_passRangeStart; i < jobData->m_passRangeStart + jobData->m_passRangeCount; i++)
        {
            if (!jobData->m_renderGraph->m_builder->m_passAlive[i])
                continue;

            const PassDeclaration& pass = jobData->m_renderGraph->m_builder->m_declaredPasses[i];

            std::chrono::time_point start = std::chrono::steady_clock::now();
            KE_ASSERT(pass.m_executeFunction != nullptr);
            pass.m_executeFunction(*jobData->m_renderGraph, jobData->m_passExecutionData);
            std::chrono::time_point end = std::chrono::steady_clock::now();

            const u64 duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            jobData->m_renderGraph->m_currentFramePassPerformance.find(pass.m_name)->second = duration;
            jobData->m_renderGraph->m_currentFrameTotalDuration.fetch_add(duration, std::memory_order_relaxed);
        }

        jobData->m_graphicsContext->EndGraphicsCommandList(jobData->m_passExecutionData.m_commandList);
    }
} // namespace KryneEngine::Modules::RenderGraph