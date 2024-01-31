/**
 * @file
 * @author Max Godefroy
 * @date 31/01/2024.
 */

#include "HelperFunctions.hpp"
#include <iostream>

namespace KryneEngine
{
    void DebugLayerMessageCallback(
            D3D12_MESSAGE_CATEGORY _category,
            D3D12_MESSAGE_SEVERITY _severity,
            D3D12_MESSAGE_ID _id,
            LPCSTR _description,
            void* _context)
    {
        constexpr auto kMinimumSeverity = D3D12_MESSAGE_SEVERITY_WARNING;
        constexpr auto kMinimumAssertSeverity = D3D12_MESSAGE_SEVERITY_ERROR;

        if (_severity > kMinimumSeverity)
        {
            return;
        }

        eastl::string severityString;
        switch (_severity)
        {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                severityString = "corruption";
                break;
            case D3D12_MESSAGE_SEVERITY_ERROR:
                severityString = "error";
                break;
            case D3D12_MESSAGE_SEVERITY_WARNING:
                severityString = "warning";
                break;
            case D3D12_MESSAGE_SEVERITY_INFO:
                severityString = "info";
                break;
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                severityString = "message";
                break;
        }

        std::cout << "Validation layer (" << severityString.c_str() << "): " << _description << std::endl;

        KE_ASSERT(_severity > kMinimumAssertSeverity);
    }
}
