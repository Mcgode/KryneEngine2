/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#pragma once

#include <KryneEngine/Core/Common/Types.hpp>

#include "KryneEngine/Modules/ShaderReflection/Input/EntryPoint.hpp"

namespace KryneEngine::Modules::ShaderReflection
{
    /**
     * @brief A binary blob containing the shader reflection data for a specific shader
     *
     * @details
     * The blob is formatted as such:
     *      - header
     *      - entry point indirection table
     *      - entry point blobs
     *      - strings table
     * Strings are kept in a separate table, as most of them may not be accessed at all, and could thus take up some
     * cache space for nothing.
     */
    struct alignas(8) Blob
    {
        static constexpr u32 kMagicNumber = 0x91eb21ad; // 'keshrf' in base64
        static constexpr u32 kVersion = 0;

        struct Header
        {
            u32 m_magic;
            u32 m_version;
            u32 m_entryPointsCount;
            u32 m_stringsOffset;
        } m_header;

        [[nodiscard]] static bool IsShaderReflectionBlob(const std::byte* _data);
        [[nodiscard]] static Blob* CreateBlob(
            AllocatorInstance _allocator,
            eastl::span<const EntryPointInput> _entryPoints);
    };
}
