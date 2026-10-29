#pragma once

#pragma region CPU_ARCHITECTURE
//  HW_X86      - 32-bit x86
//  HW_X64      - 64-bit x86 (x86-64 / AMD64)
//  HW_ARM32    - 32-bit ARM
//  HW_ARM64    - 64-bit ARM (AArch64)

#if (defined(_M_X64) && !defined(_M_ARM64EC)) || defined(__x86_64__) || defined(__amd64__)
    #define HW_X64 1
#elif defined(_M_IX86) || defined(__i386__)
    #define HW_X86 1
#elif defined(_M_ARM64) || defined(_M_ARM64EC) || defined(__aarch64__)
    #define HW_ARM64 1
#elif defined(_M_ARM) || defined(__arm__)
    #define HW_ARM32 1
#else
    #error "Unsupported CPU architecture"
#endif

#pragma endregion   //  CPU_ARCHITECTURE

#pragma region OS
//  OS_WINDOWS  - Windows
//  OS_LINUX    - Linux

#if defined(_WIN64) || defined(_WIN32)
    #define OS_WINDOWS 1
#elif defined(__linux__)
    #define OS_LINUX 1
#endif

#pragma endregion   //  OS

#pragma region  //   Console
//  HW_XONE - Xbox One
//  HW_XSX  - Xbox Series X|S

#if defined(_GAMING_XBOX_XBOXONE)
    #define HW_XONE 1
#elif defined(_GAMING_XBOX_SCARLETT)
    #define HW_XSX 1
#endif

#pragma endregion   //   Console

#pragma region ENGINE_FRAMEWORKS
//  FW_SDL3 - Simple DirectMedia Layer 3 (SDL3)

#if defined(OS_WINDOWS) || defined(OS_LINUX) || defined(HW_XONE) || defined(HW_XSX)
    #define FW_SDL3 1
#endif

#pragma endregion   //  ENGINE_FRAMEWORKS

#pragma region GRAPHICS_API
//  API_DX12    - DirectX 12
//  API_VULKAN  - Vulkan

#if defined(HW_XONE) || defined(HW_XSX)
    #define API_DX12 1
#elif defined(OS_WINDOWS) || defined(OS_LINUX)
    #define API_VULKAN 1
#endif

#pragma endregion   //  GRAPHICS_API