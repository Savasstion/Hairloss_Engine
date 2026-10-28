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
//  HW_WINDOWS  - Windows
//  HW_LINUX    - Linux

#if defined(_WIN64) || defined(_WIN32)

    #define HW_WINDOWS 1


#elif defined(__linux__)

    #define HW_LINUX 1

#endif

#pragma endregion   //  OS