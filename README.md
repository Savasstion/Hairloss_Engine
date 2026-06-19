# Hairloss_Engine
A game engine built on SDL3, hopes and dreams, and the great sacrifice of hair loss.

## Building

**Prerequisites:** CMake 3.25+, Visual Studio 2022/2026, or Ninja.

### Configure

```bash
cmake --preset vs2022
# or
cmake --preset vs2026
# or
cmake --preset ninja
```

### Build

```bash
cmake --build build/vs2022 --config Release
# or
cmake --build build/vs2026 --config Release
# or
cmake --build build/ninja
```

## Tests

### Build & Run

```bash
cmake --build build/vs2022 --config Debug --target HairlossTest
ctest --test-dir build/vs2022 -C Debug
```

> **Note:** VS2026 requires CMake 4.2 or newer.