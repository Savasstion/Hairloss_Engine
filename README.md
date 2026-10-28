# Hairloss_Engine
A game engine built on SDL3, hopes and dreams, and the great sacrifice of hair loss.

## Building

**Prerequisites:** CMake 3.28+, and either Visual Studio 2022/2026 or Ninja (with the MSVC toolchain).

> **Note:** VS2026 requires CMake 4.2 or newer.

### Presets

Configure presets are named `<generator>-<arch>`. Build presets add the configuration: `<generator>-<arch>-<config>`.

| Generator | x64 | x86 |
|---|---|---|
| Visual Studio 2022 | `vs2022-x64` | `vs2022-x86` |
| Visual Studio 2026 | `vs2026-x64` | `vs2026-x86` |
| Ninja Multi-Config | `ninja-x64` | `ninja-x86` |

Available configurations: `Debug`, `Release`.

### Configure

```bash
cmake --preset vs2022-x64
# or
cmake --preset vs2026-x64
# or
cmake --preset ninja-x64
```

Use the `-x86` variants for 32-bit builds.

> **Note:** Ninja presets need the MSVC compiler environment loaded. Run them from the matching
> "x64 Native Tools Command Prompt" / "x86 Native Tools Command Prompt" (or Developer PowerShell),
> or open the folder directly in Visual Studio, which sets this up for you. Use a fresh shell per architecture.

### Build

```bash
cmake --build --preset vs2026-x64-release
# or
cmake --build --preset ninja-x64-debug
```

Visual Studio presets generate a solution in `build/<preset>/`, which you can also open directly.

## Tests

### Build & Run

```bash
cmake --build --preset vs2026-x64-debug --target HairlossTest
ctest --test-dir build/vs2026-x64 -C Debug
```

The same works with any other preset, for example `ninja-x64-debug` and `build/ninja-x64`.