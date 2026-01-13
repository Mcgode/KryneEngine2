# Toolchain for cross-compiling from macOS -> Windows (x86_64 mingw-w64)

# Tell CMake this is a cross build for Windows
set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target system name" FORCE)
set(CMAKE_SYSTEM_PROCESSOR x86_64 CACHE STRING "Target processor" FORCE)

# Clear any macOS-specific architecture settings that CLion / bundled CMake may inject.
# This prevents '-arch arm64' from being added to tries when using a non-Apple compiler.
set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "" FORCE)

# Point to mingw-w64 toolchain (adjust paths if your brew prefix differs).
set(MINGW_PREFIX x86_64-w64-mingw32 CACHE STRING "mingw prefix")

# Compilers
find_program(MINGW_C_COMPILER NAMES ${MINGW_PREFIX}-gcc ${MINGW_PREFIX}-cc)
find_program(MINGW_CPP_COMPILER NAMES ${MINGW_PREFIX}-g++ ${MINGW_PREFIX}-c++)

if(NOT MINGW_C_COMPILER)
    message(FATAL_ERROR "Could not find mingw-w64 C compiler. Install mingw-w64 and ensure ${MINGW_PREFIX}-gcc is in PATH.")
endif()
if(NOT MINGW_CPP_COMPILER)
    message(FATAL_ERROR "Could not find mingw-w64 C++ compiler. Install mingw-w64 and ensure ${MINGW_PREFIX}-g++ is in PATH.")
endif()

set(CMAKE_C_COMPILER ${MINGW_C_COMPILER} CACHE PATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER ${MINGW_CPP_COMPILER} CACHE PATH "C++ compiler" FORCE)

set(CMAKE_FIND_ROOT_PATH /tmp/mingw)

# Ensure CMake doesn't try macOS frameworks
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Use Release by default unless overridden
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# (Optional) do not try to use apple-specific compilers/wrappers
set(CMAKE_C_COMPILER_WORKS TRUE CACHE BOOL "force compiler works" FORCE)
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE BOOL "force compiler works" FORCE)