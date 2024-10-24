/**
 * @file
 * @author Max Godefroy
 * @date 18/08/2024.
 */

#include "Device.hpp"

#include "../Public/RpsRuntime.hpp"
#include "Backend.hpp"
#include "Helpers.hpp"

#include <core/rps_device.hpp>
#include <core/rps_util.hpp>

#include <runtime/common/rps_runtime_device.hpp>
#include <runtime/common/rps_runtime_util.hpp>

#include <runtime/common/phases/rps_access_dag_build.hpp>
#include <runtime/common/phases/rps_cmd_dag_print.hpp>
#include <runtime/common/phases/rps_cmd_print.hpp>
#include <runtime/common/phases/rps_dag_build.hpp>
#include <runtime/common/phases/rps_dag_schedule.hpp>
#include <runtime/common/phases/rps_lifetime_analysis.hpp>
#include <runtime/common/phases/rps_memory_schedule.hpp>
#include <runtime/common/phases/rps_pre_process.hpp>
#include <runtime/common/phases/rps_schedule_print.hpp>

namespace KryneEngine::Modules::RpsRuntime
{
    Device::Device(rps::Device* _pDevice, const RuntimeDeviceCreateInfo* _createInfo)
        : rps::RuntimeDevice(_pDevice, _createInfo->m_runtimeCreateInfo)
        , m_graphicsContext(_createInfo->m_graphicsContext)
    {
    }

    RpsResult Device::BuildDefaultRenderGraphPhases(rps::RenderGraph& _renderGraph)
    {
        RPS_V_RETURN(_renderGraph.ReservePhases(16));
        RPS_V_RETURN(_renderGraph.AddPhase<rps::PreProcessPhase>());
        RPS_V_RETURN(_renderGraph.AddPhase<rps::CmdDebugPrintPhase>());
        RPS_V_RETURN(_renderGraph.AddPhase<rps::DAGBuilderPass>());
        RPS_V_RETURN(_renderGraph.AddPhase<rps::AccessDAGBuilderPass>(_renderGraph));
        RPS_V_RETURN(_renderGraph.AddPhase<rps::DAGPrintPhase>(_renderGraph));
        RPS_V_RETURN(_renderGraph.AddPhase<rps::DAGSchedulePass>(_renderGraph));
        if (!rpsAnyBitsSet(_renderGraph.GetCreateInfo().renderGraphFlags, RPS_RENDER_GRAPH_NO_LIFETIME_ANALYSIS))
        {
            RPS_V_RETURN(_renderGraph.AddPhase<rps::LifetimeAnalysisPhase>());
        }
        RPS_V_RETURN(_renderGraph.AddPhase<rps::MemorySchedulePhase>(_renderGraph));
        RPS_V_RETURN(_renderGraph.AddPhase<rps::ScheduleDebugPrintPhase>());
        RPS_V_RETURN(_renderGraph.AddPhase<Backend>(*this, _renderGraph));

        return RPS_OK;
    }

    RpsResult Device::InitializeSubresourceInfos(rps::ArrayRef<rps::ResourceInstance> _resInstances)
    {
        for (auto& resourceInstance: _resInstances)
        {
            {
                u32 aspectMask = 0;
                if (resourceInstance.desc.IsImage())
                {
                    aspectMask = static_cast<u32>(GetAspectMaskFromFormat(resourceInstance.desc.image.format));
                }

                rps::GetFullSubresourceRange(
                    resourceInstance.fullSubresourceRange,
                    resourceInstance.desc,
                    aspectMask);
            }

            if (resourceInstance.desc.IsBuffer())
            {
                resourceInstance.numSubResources = 1;
            }
            else
            {
                const auto& imageInfo = resourceInstance.desc.image;

                resourceInstance.numSubResources = imageInfo.mipLevels;

                if (resourceInstance.desc.type != RPS_RESOURCE_TYPE_IMAGE_3D)
                {
                    resourceInstance.numSubResources *= imageInfo.arrayLayers;
                }

                // Two planes for depth-stencil resources
                if (imageInfo.format == RPS_FORMAT_D24_UNORM_S8_UINT || imageInfo.format == RPS_FORMAT_D32_FLOAT_S8X24_UINT)
                {
                    resourceInstance.numSubResources *= 2;
                }
            }
        }

        return RPS_OK;
    }

    RpsResult Device::InitializeResourceAllocInfos(rps::ArrayRef<rps::ResourceInstance> _resInstances)
    {
        for (auto& resourceInfo : _resInstances)
        {
            if (!resourceInfo.isPendingCreate)
            {
                continue;
            }

            // TODO: Handle RPS memory management ?
            return RPS_ERROR_NOT_IMPLEMENTED;
        }
        return RPS_OK;
    }

    RpsResult Device::GetSubresourceRangeFromImageView(
        rps::SubresourceRangePacked& _outRange,
        const rps::ResourceInstance& _resourceInfo,
        const RpsAccessAttr& _accessAttr,
        const RpsImageView& _imageView)
    {
        return RPS_ERROR_NOT_IMPLEMENTED;
    }
} // namespace KryneEngine::Modules::RpsRuntime