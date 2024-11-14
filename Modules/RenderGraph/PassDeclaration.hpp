/**
 * @file
 * @author Max Godefroy
 * @date 13/11/2024.
 */

#pragma once

#include <Graphics/Common/RenderPass.hpp>
#include <Memory/SimplePool.hpp>

namespace KryneEngine::Modules::RenderGraph
{
    enum class PassType
    {
        Render,
        Compute,
    };

    struct PassDeclaration
    {
        friend class Builder;

    public:
        explicit PassDeclaration(PassType _type)
            : m_type(_type)
        {}

        PassDeclaration& SetName(const eastl::string_view& _name)
        {
            m_name = _name;
            return *this;
        }

        PassDeclaration& AddColorAttachment(
            SimplePoolHandle _texture,
            RenderPassDesc::Attachment::LoadOperation _loadOperation,
            RenderPassDesc::Attachment::StoreOperation _storeOperation)
        {
            m_colorAttachments.push_back({_texture, _loadOperation, _storeOperation});

            return *this;
        }

    private:
        struct AttachmentDeclaration
        {
            SimplePoolHandle m_texture;
            RenderPassDesc::Attachment::LoadOperation m_loadOperation;
            RenderPassDesc::Attachment::StoreOperation m_storeOperation;
        };

        PassType m_type;
        eastl::string m_name;
        eastl::fixed_vector<AttachmentDeclaration, RenderPassDesc::kMaxSupportedColorAttachments, false> m_colorAttachments = {};
        eastl::optional<AttachmentDeclaration> m_depthAttachment;
    };
} // namespace KryneEngine::Module::RenderGraph


