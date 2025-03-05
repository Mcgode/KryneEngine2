/**
 * @file
 * @author Max Godefroy
 * @date 15/11/2024.
 */

#include "KryneEngine/Modules/RenderGraph/RenderGraph.hpp"

#include <KryneEngine/Core/Graphics/Common/GraphicsContext.hpp>
#include <KryneEngine/Core/Profiling/TracyHeader.hpp>
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
        {
            KE_ZoneScoped("Build and cull render DAG");
            m_builder->PrintBuildResult();
        }

        {
            KE_ZoneScoped("Prepare render jobs");

            const auto initJobData = [&](JobData& _jobData, u32 _start)
            {
                _jobData.m_renderGraph = this;
                _jobData.m_passExecutionData = PassExecutionData {
                    .m_graphicsContext = &_graphicsContext,
                    .m_commandList = _graphicsContext.BeginGraphicsCommandList()
                };
                _jobData.m_passRangeStart = _start;
                _jobData.m_passRangeCount = 0;
            };

            JobData* currentJob = nullptr;

            constexpr double maxOverfillRatio = 1.25; // Accept up to 25% longer command list from one long pass.
            const u64 maxOverfillDuration = static_cast<u64>(m_targetTimePerCommandList * maxOverfillRatio * 1'000'000);
            const u64 targetDuration = static_cast<u64>(m_targetTimePerCommandList * 1'000'000);
            u64 cumulativeDuration = 0;

            for (size_t i = 0; i < m_builder->m_declaredPasses.size(); i++)
            {
                if (!m_builder->m_passAlive[i])
                {
                    continue;
                }

                const PassDeclaration& pass = m_builder->m_declaredPasses[i];
                const u64 estimatedPassDuration = m_previousFramePassPerformance[pass.m_name];

                // Prevent the current job from overfilling beyond a certain threshold.
                if (cumulativeDuration + estimatedPassDuration > maxOverfillDuration)
                {
                    cumulativeDuration = 0;
                    currentJob = nullptr;
                }

                if (currentJob == nullptr)
                {
                    currentJob = &m_jobs.emplace_back();
                    initJobData(*currentJob, i);
                }

                currentJob->m_passRangeCount = i - currentJob->m_passRangeStart + 1;
                cumulativeDuration += estimatedPassDuration;

                // Reserve entry in map, so that duration saving is thread-safe
                m_currentFramePassPerformance.emplace(pass.m_name, 0);

                // If beyond the target duration, move to the next job
                if (cumulativeDuration > targetDuration)
                {
                    cumulativeDuration = 0;
                    currentJob = nullptr;
                }
            }
        }

        {
            KE_ZoneScoped("Dispatch & execute render jobs");

            // Execute the last job in this thread/fiber, schedule the other ones for dispatch.
            // Small optimization.
            if (m_jobs.size() > 1)
            {
                const SyncCounterId jobsCounter = _fibersManager.InitAndBatchJobs(
                    m_jobs.size() - 1,
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
        }

        {
            KE_ZoneScoped("Cleanup");

            eastl::swap(m_previousFramePassPerformance, m_currentFramePassPerformance);
            m_currentFramePassPerformance.clear();

            m_previousFrameTotalDuration = m_currentFrameTotalDuration.load(std::memory_order_acquire);
            m_currentFrameTotalDuration.store(0, std::memory_order_relaxed);

            m_builder.reset();
        }
    }

    void RenderGraph::ExecuteJob(void* _userData)
    {
        auto* jobData = static_cast<JobData*>(_userData);

        KE_ZoneScopedF("Execute render job #%u", eastl::distance(jobData->m_renderGraph->m_jobs.begin(), jobData));

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

        jobData->m_passExecutionData.m_graphicsContext->EndGraphicsCommandList(jobData->m_passExecutionData.m_commandList);
    }

    RenderPassHandle RenderGraph::FetchRenderPass(
        GraphicsContext& _graphicsContext,
        PassDeclaration& _passDeclaration)
    {
        const u64 hash = _passDeclaration.GetRenderPassHash();

        const auto it = m_renderPassCache.find(hash);
        if (it != m_renderPassCache.end())
        {
            return it->second;
        }

        RenderPassDesc desc;
        for (const auto& attachment : _passDeclaration.m_colorAttachments)
        {
            desc.m_colorAttachments.push_back(RenderPassDesc::Attachment {
                .m_loadOperation = attachment.m_loadOperation,
                .m_storeOperation = attachment.m_storeOperation,
                .m_initialLayout = attachment.m_layoutBefore,
                .m_finalLayout = attachment.m_layoutAfter,
                .m_rtv = { GenPool::kInvalidHandle },
                .m_clearColor = attachment.m_clearColor,
            });
        }
        if (_passDeclaration.m_depthAttachment.has_value())
        {
            const PassAttachmentDeclaration attachment = _passDeclaration.m_depthAttachment.value();
            desc.m_depthStencilAttachment = RenderPassDesc::DepthStencilAttachment {
                .m_stencilLoadOperation = attachment.m_stencilLoadOperation,
                .m_stencilStoreOperation = attachment.m_stencilStoreOperation,
                .m_stencilClearValue = attachment.m_clearStencil,
            };
            desc.m_depthStencilAttachment.value().m_loadOperation = attachment.m_loadOperation;
            desc.m_depthStencilAttachment.value().m_storeOperation = attachment.m_storeOperation;
            desc.m_depthStencilAttachment.value().m_initialLayout = attachment.m_layoutBefore;
            desc.m_depthStencilAttachment.value().m_finalLayout = attachment.m_layoutAfter;
            desc.m_depthStencilAttachment.value().m_rtv = { GenPool::kInvalidHandle };
            desc.m_depthStencilAttachment.value().m_clearColor = float4(attachment.m_clearDepth, 0.0f, 0.0f, 0.0f);
        }

        const RenderPassHandle handle = _graphicsContext.CreateRenderPass(desc);
        m_renderPassCache.emplace(hash, handle);
        return handle;
    }

    void RenderGraph::ResetRenderPassCache()
    {
        m_renderPassCache.clear();
    }
} // namespace KryneEngine::Modules::RenderGraph