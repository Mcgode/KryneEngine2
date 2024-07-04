#include "BasicRps.hpp"

#include "EASTL/array.h"

//RpsResult BuildBasicImGui(RpsRenderGraphBuilder _builder, const RpsConstant* _arguments, u32 _argumentCount)
//{
//	KE_ASSERT(_argumentCount > 0);
//
//	RpsNodeDeclId drawImGui = rpsRenderGraphDeclareDynamicNode(
//		_builder,
//		"DrawImGui",
//		RPS_NODE_FLAG_NONE,
//		{
//			rps::ParameterDesc::Make<rps::ImageView>(rps::SemanticAttr(RPS_SEMANTIC_RENDER_TARGET), "backBuffer"),
//			rps::ParameterDesc::Make<RpsClearValue>(rps::SemanticAttr(RPS_SEMANTIC_COLOR_CLEAR_VALUE), "clearValue"),
//		}
//	);
//
//	struct GraphVariables
//	{
//		rps::ImageView m_backBufferView{};
//		RpsClearValue m_clearValue{};
//	};
//
//	auto* graphVariables = rpsRenderGraphAllocateData<GraphVariables>(_builder);
//	KE_ASSERT(graphVariables != nullptr);
//
//	const auto backBufferResId = rpsRenderGraphGetParamResourceId(_builder, 0);
//	graphVariables->m_backBufferView = rps::ImageView{ backBufferResId };
//	graphVariables->m_clearValue = RpsClearValue{{{0.1f, 0.1f, 0.3f, 1.f}}};
//
//	rpsRenderGraphAddNode(
//		_builder,
//		drawImGui,
//		0,
//		nullptr,
//		nullptr,
//		RPS_CMD_CALLBACK_FLAG_NONE,
//		{ &graphVariables->m_backBufferView, &graphVariables->m_clearValue }
//	);
//
//	return RPS_OK;
//}
//
//BasicRps::BasicRps(const GraphicsContext* _context)
//	: m_parameters {
//		_context->GetApplicationInfo().m_displayOptions.m_width,
//		_context->GetApplicationInfo().m_displayOptions.m_height,
//	}
//{
//	const auto rpsDevice = _context->GetRpsDevice();
//
//	const eastl::array graphParameters =
//	{
//		RpsParameterDesc {
//			.typeInfo = rpsTypeInfoInitFromType(RpsResourceDesc),
//			.name = "backBuffer",
//			.flags = RPS_PARAMETER_FLAG_RESOURCE_BIT,
//		},
//		RpsParameterDesc {
//			.typeInfo = rpsTypeInfoInitFromType(void*),
//			.name = "userContext",
//		}
//	};
//
//	const RpsRenderGraphSignatureDesc entryInfo =
//	{
//		.numParams = graphParameters.size(),
//		.pParamDescs = graphParameters.data(),
//		.name = "BasicRps",
//	};
//
//	const RpsRenderGraphCreateInfo createInfo = { .mainEntryCreateInfo = { .pSignatureDesc = &entryInfo } };
//	KE_ASSERT(rpsRenderGraphCreate(rpsDevice, &createInfo, &m_renderGraph) == RPS_OK);
//}
//
//void BasicRps::UpdateGraph(const GraphicsContext* _context, PFN_rpsRenderGraphBuild _buildFunction)
//{
//	const RpsResourceDesc backBufferResourceDesc =
//	{
//		.type = RPS_RESOURCE_TYPE_IMAGE_2D,
//		.temporalLayers = 1,
//		.image = {
//			.width = m_parameters.m_width,
//			.height = m_parameters.m_height,
//			.arrayLayers = 1,
//			.mipLevels = 1,
//			.format = RPS_FORMAT_R8G8B8A8_UNORM_SRGB,
//			.sampleCount = 1,
//		}
//	};
//
//	eastl::array<RpsConstant, 2> arguments = { &backBufferResourceDesc, &m_parameters };
//
//	const RpsRenderGraphUpdateInfo updateInfo =
//	{
//		.gpuCompletedFrameIndex = RPS_GPU_COMPLETED_FRAME_INDEX_NONE,
//		.diagnosticFlags = RPS_DIAGNOSTIC_ENABLE_ALL,
//		.numArgs = arguments.size(),
//		.ppArgs = arguments.data(),
//		.pfnBuildCallback = _buildFunction,
//	};
//
//	KE_ASSERT(rpsRenderGraphUpdate(m_renderGraph, &updateInfo) == RPS_OK);
//}
