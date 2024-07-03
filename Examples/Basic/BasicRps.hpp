#pragma once

#include <Graphics/Common/GraphicsContext.hpp>
#include <RPS/rps.h>

using namespace KryneEngine;

struct GraphBuildParameters
{
	u32 m_width;
	u32 m_height;
};

RpsResult BuildBasicImGui(RpsRenderGraphBuilder _builder, const RpsConstant* _arguments, u32 _argumentCount);

class BasicRps
{
public:
	explicit BasicRps(const GraphicsContext* _context);

	void UpdateGraph(const GraphicsContext* _context, PFN_rpsRenderGraphBuild _buildFunction);

private:
	GraphBuildParameters m_parameters;

	RpsRenderGraph m_renderGraph = nullptr;
};