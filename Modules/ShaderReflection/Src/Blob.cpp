/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#include "KryneEngine/Modules/ShaderReflection/Blob.hpp"

namespace KryneEngine::Modules::ShaderReflection
{
    bool Blob::IsShaderReflectionBlob(const std::byte* _data)
    {
        return *reinterpret_cast<const u32*>(_data) == kMagicNumber;
    }
}