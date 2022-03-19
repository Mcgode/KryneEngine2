
# Parse build type
string(REPLACE " " ";" BUILD_TYPE ${CMAKE_BUILD_TYPE})

list(LENGTH BUILD_TYPE TOTAL_BUILD_ARGS_LENGTH)

if (${TOTAL_BUILD_ARGS_LENGTH} GREATER_EQUAL 2)
    list(GET BUILD_TYPE 1 Arg)

    if (Arg STREQUAL "DX12")
        set(GraphicsApi "DX12")
    endif()
endif()

if (NOT DEFINED GraphicsApi)
    set(GraphicsApi "VK")
endif()

message(STATUS "Using API: " ${GraphicsApi})
add_definitions("-DKE_GRAPHICS_API_${GraphicsApi}")

list(GET BUILD_TYPE 0 TypeName)
set(CMAKE_BUILD_TYPE "${TypeName}")

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})