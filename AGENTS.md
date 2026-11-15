# AGENTS.md — Hairloss Engine

Guidance for AI agents (and humans) reading, writing, or editing code in this repository.
**Read this whole file before making changes.** When in doubt, imitate the existing code in
`src/window/` and `src/memory/heap/`. They are the reference implementations of every pattern below.

---

## 1. Project at a glance

| | |
|---|---|
| Goal | Cross-platform, cross-framework, modular C++ game engine |
| Language | C++23 (`CMAKE_CXX_STANDARD 23`, no compiler extensions) |
| Build | CMake ≥ 3.28 (≥ 4.2 for VS2026), presets in `CMakePresets.json`, output in `project/<preset>/` |
| Current target | Windows x64 (`win64`) via SDL3 |
| Planned targets | Linux, Xbox One, Xbox Series X\|S; ARM and x86 are detected too |
| Frameworks | SDL3 (`FW_SDL3`). The architecture leaves room for others |
| Graphics API (planned) | Vulkan on Windows/Linux, DX12 on Xbox |
| Vendored deps | `vendor/SDL` (static), `vendor/Catch2` (git submodules). **Never edit vendor code.** |
| Tests | Catch2 v3, `tests/`, target `HairlossTest`, run with `ctest` |

```
src/
├── platformConfig/          # Platform/framework detection, primitive typedefs, global include hub
│   ├── platformConfigDefines.h
│   ├── platformConfigTypes.h
│   ├── platformConfigIncludes.h
│   └── platform/win64/      # win64PlatformDefines.h / Includes.h / Types.h
├── app/                     # Entry point + AppState
│   └── platform/win64/      # win64App.h/.cpp (SDL main callbacks)
├── window/                  # Window abstraction
│   ├── window.h/.cpp        # Platform-agnostic abstraction
│   ├── windowDefines.h / windowTypes.h / windowIncludes.h
│   └── platform/win64/      # win64WindowImpl.h/.cpp (platform implementation)
├── logging/                 # Log / DebugLog family (std::print based)
└── memory/                  # Memory types, byte helpers
    └── heap/                # Heap (sub-module with its own Includes header)
```

---

## 2. Cross-platform architecture

### 2.1 Three axes, chosen at compile time by macros

Platform selection is **compile-time only**. There are no virtual interfaces, no runtime
dispatch, and no factory. Every axis is a macro defined in
`src/platformConfig/platformConfigDefines.h`:

| Prefix | Meaning | Current values |
|---|---|---|
| `HW_` | CPU architecture / hardware / console | `HW_X64`, `HW_X86`, `HW_ARM64`, `HW_ARM32`, `HW_64BITS`, `HW_32BITS`, `HW_XONE`, `HW_XSX` |
| `OS_` | Operating system | `OS_WINDOWS`, `OS_LINUX` |
| `FW_` | Engine framework / middleware layer | `FW_SDL3` |
| `GAPI_` | Graphics API | `GAPI_VULKAN`, `GAPI_DX12` |
| `BUILD_` | Build mode | `BUILD_DEBUG` |

Rules:
- Macros are defined as `1` (`#define OS_WINDOWS 1`) and **always tested with
  `#if defined(X)`**. Never use `#ifdef` and never test the value.
- Raw compiler macros (`_WIN64`, `__linux__`, `_M_X64`, `_DEBUG`, ...) may appear **only** in
  `platformConfigDefines.h`. Everywhere else, use the engine macros.
- A **platform** is a combination of `OS_`/`HW_` macros. It is identified by a short
  lowercase **platform tag** used in folder and file names:

  | Platform tag | Condition |
  |---|---|
  | `win64` | `defined(OS_WINDOWS) && defined(HW_X64)` |
  | *(future)* e.g. `linux64`, `xone`, `xsx` | add a matching condition |

- Which **framework** a platform compiles is decided centrally in the `ENGINE_FRAMEWORKS`
  region of `platformConfigDefines.h` (e.g. `FW_SDL3` for Windows/Linux/Xbox). Which **graphics
  API** is decided in the `GRAPHICS_API` region. Code never decides this locally.
- Framework-specific `#define`s that must exist before framework headers are included go in
  `platform/<tag>/<tag>PlatformDefines.h`, inside a `#if defined(FW_...)` block. For example,
  `SDL_MAIN_USE_CALLBACKS`.

### 2.2 Abstraction class + platform implementation class (compile-time pimpl)

Every platform-dependent subsystem is split into two classes:

1. **Abstraction class**, e.g. `Window` in `src/<module>/<module>.h/.cpp`
   - Platform-agnostic. Contains **no** `OS_`/`HW_` checks and ideally no `FW_` checks.
   - Owns its implementation through a pointer member: `std::shared_ptr<WindowImpl> m_pWindowImpl;`
   - Creates the impl in its constructor and releases it in `Release()`.
   - Its public API **forwards** to the impl:
     ```cpp
     b8 Window::CreateAppWindow(const WindowCreationDesc& desc)
     {
     	return m_pWindowImpl->CreateAppWindow(desc);
     }
     ```
   - This is the class the rest of the engine and the tests use.

2. **Platform implementation class**, e.g. `WindowImpl` in
   `src/<module>/platform/<tag>/<tag><Module>Impl.h/.cpp`
   - **The class name is the same on every platform** (`WindowImpl`, not `Win64WindowImpl`).
     Only the file name and folder carry the platform tag. Because exactly one platform's header
     is ever included, the abstraction class binds to the right implementation with no
     inheritance or virtuals.
   - Has the same method names and signatures that the abstraction forwards to.
   - The whole `.cpp` body is wrapped in the platform guard:
     ```cpp
     #include "../../windowIncludes.h"

     #if defined(OS_WINDOWS) && defined(HW_X64)
     ...
     #endif	// OS_WINDOWS && HW_X64
     ```
   - **Framework code lives here**, nested inside `#if defined(FW_SDL3)` blocks, including
     framework-typed members (`std::shared_ptr<SDL_Window> m_pWindow; SDL_WindowFlags m_flags;`).
   - Platform-common members (`u32 m_width; u32 m_height;`) sit outside framework blocks.

3. **Selection happens in the module's Includes header**, never in the abstraction:
   ```cpp
   // windowIncludes.h
   #pragma once

   #include "../platformConfig/platformConfigIncludes.h"
   #include "windowDefines.h"
   #include "windowTypes.h"

   #if defined(OS_WINDOWS) && defined(HW_X64)
   	#include "platform/win64/win64WindowImpl.h"
   #endif

   #include "window.h"
   ```
   The impl header is included **before** the abstraction header, so the abstraction sees
   the complete `WindowImpl` type.

Why: adding a platform means adding a new `platform/<tag>/` folder plus one `#elif` line per
module. The abstraction class and every caller stay untouched. That is how the engine stays
highly reusable.

### 2.3 Where framework (`FW_`) code may appear

| Location | Allowed? |
|---|---|
| `platform/<tag>/` implementation files | ✅ Primary home for framework code |
| `platformConfig/platform/<tag>/` | ✅ Framework defines and framework headers |
| `<module>Types.h` / `<module>Defines.h` | ✅ Only for framework-typed fields/constants in descs, guarded (`SDL_WindowFlags m_flags;`) |
| `app/` entry point, `AppState` | ✅ Currently (SDL main callbacks, temporary `SDL_Renderer`) |
| Tests | ✅ Only to fill framework fields in descs |
| Abstraction class `.h/.cpp` | ❌ Keep framework-free |
| `logging/`, `memory/` | ❌ They are included *before* platform/framework headers (see §3.3) |

Do not reach through the abstraction into the impl from outside the module
(e.g. `m_pWindow->m_pWindowImpl->m_pWindow`). Expose what's needed on the abstraction instead.

When framework code outside the module needs the underlying framework object (e.g. SDL needs
the `SDL_Window*` to create a renderer), the abstraction exposes it **type-erased** as `void*`,
so the abstraction stays framework-free. Only the caller, which is already inside a
`#if defined(FW_...)` block, casts it back:
```cpp
// window.h / win64WindowImpl.h
void* GetNativeWindowHandle() const;

// win64WindowImpl.cpp
void* WindowImpl::GetNativeWindowHandle() const
{
#if defined(FW_SDL3)
	return m_pWindow.get();
#else
	return nullptr;
#endif	// FW_SDL3
}

// win64App.cpp (inside #if defined(FW_SDL3))
SDL_Window* pWindow = static_cast<SDL_Window*>(pState->m_pWindow->GetNativeWindowHandle());
```
Name these getters `GetNative<Thing>Handle()`.

---

## 3. File and header organisation

### 3.1 Per-module file set

Every module (`src/<module>/`) and sub-module (`src/<module>/<sub>/`) uses this set:

| File | Contents |
|---|---|
| `<module>Includes.h` | **The only header anyone includes for this module.** Pulls in dependencies in the correct order. |
| `<module>Defines.h` | `constexpr` constants, `constexpr` helper functions, module macros (e.g. `defaultWindowWidth`, `KB()/MB()/GB()`) |
| `<module>Types.h` | `enum class`es, plain `struct`s / creation descs (e.g. `WindowCreationDesc`, `AllocatorType`) |
| `<module>.h` / `<module>.cpp` | The abstraction class |
| `platform/<tag>/<tag><Module>Impl.h/.cpp` | Platform implementation class |

Files are created only when needed. For example, `logging/` has no Defines/Types, and
`heap/` only has `heap.h/.cpp` + `heapIncludes.h`.

### 3.2 Include rules

- Every header starts with `#pragma once` (no include guards).
- **Leaf headers** (`<module>.h`, `<module>Types.h`, `<module>Defines.h`, impl headers) contain
  **no `#include`s**. They rely on the module's `Includes.h` having included everything first.
  Don't add includes to them.
- **Every `.cpp` includes exactly one header: its own module's `Includes.h`**, by relative path:
  - `window.cpp` → `#include "windowIncludes.h"`
  - `platform/win64/win64WindowImpl.cpp` → `#include "../../windowIncludes.h"`
  - `heap/heap.cpp` → `#include "heapIncludes.h"`
- All engine includes use **relative paths in quotes**. External headers use angle brackets
  (`<SDL3/SDL.h>`, `<memory>`).
- `<module>Includes.h` order is always:
  1. `../platformConfig/platformConfigIncludes.h` (or the parent module's Includes for a
     sub-module, e.g. `heapIncludes.h` → `../memoryIncludes.h`)
  2. Other engine modules' `Includes.h` this module depends on
  3. Framework headers if this module specifically needs them (inside `#if defined(FW_...)`)
  4. `<module>Defines.h`, then `<module>Types.h`
  5. Platform impl header(s) inside platform guards
  6. `<module>.h` (abstraction) last
  7. Sub-module `Includes.h` files (e.g. `memoryIncludes.h` includes `heap/heapIncludes.h`)
- Platform guards inside Includes headers use an indented include:
  ```cpp
  #if defined(OS_WINDOWS) && defined(HW_X64)
  	#include "platform/win64/win64WindowImpl.h"
  #endif
  ```

### 3.3 The global include hub: `platformConfigIncludes.h`

Everything ultimately includes `platformConfigIncludes.h`, which sets up the world in this order:

1. `platformConfigDefines.h`: all `HW_/OS_/FW_/GAPI_/BUILD_` macros
2. `platformConfigTypes.h`: primitive typedefs
3. Standard library headers used engine-wide (`<memory>`, `<print>`, `<format>`,
   `<string_view>`, `<cstddef>`). Add new engine-wide std headers here.
4. Core engine services: `logging/loggingIncludes.h`, `memory/memoryIncludes.h`
5. The platform hub: `platform/<tag>/<tag>PlatformIncludes.h`, which in turn includes
   `<tag>PlatformDefines.h` → OS headers (`<Windows.h>`) → framework headers (`<SDL3/SDL.h>`, ...)
   → `<tag>PlatformTypes.h`

Consequences:
- Logging and memory are available everywhere, but **they are included before OS/framework
  headers**, so they must not use OS/framework types.
- Circular includes (e.g. `loggingIncludes.h` ↔ `platformConfigIncludes.h`) are expected and
  resolved by `#pragma once`. Respect the ordering above when adding includes.
- OS and framework headers are included **only** via `platformConfig/platform/<tag>/`, so
  modules never include `<Windows.h>` or `<SDL3/...>` directly. The one exception is
  `SDL_main.h` in `appIncludes.h`.

---

## 4. Code formatting

- **Indentation: tabs**, everywhere. Spaces are fine for alignment *after* code, such as in
  column-aligned comment tables like `//  HW_X86      - 32-bit x86`.
- **Line endings: LF**, enforced by `.gitattributes` (`* text=auto eol=lf`) so every OS and
  IDE gets the same files no matter what `core.autocrlf` is set to. Only Windows-only formats
  (`.bat`, `.cmd`, `.ps1`, `.sln`, `.vcxproj`) use CRLF.
- **`.editorconfig`** sets indentation, line endings, UTF-8, the final newline and trailing
  whitespace for every editor (Visual Studio, VS Code, CLion, Rider). Don't override it with
  IDE-specific settings. JSON/YAML use 2-space indentation, and `vendor/` keeps its own style.
- **Braces: Allman.** Opening brace on its own line for classes, structs, enums, functions,
  `if`/`switch`/loops.
  ```cpp
  b8 Heap::Release()
  {
  	if (m_pHeapStart)
  	{
  		delete m_pHeapStart;
  	}

  	Clear();

  	return true;
  }
  ```
- Single-statement early returns may skip braces (`if (!x)\n    return false;`, as in
  `win64App.cpp`). Engine classes generally brace them. Prefer braces inside class methods.
- A blank line before `return` and between logical steps.
- **Align `=` in consecutive member assignments** with tabs (see `Clear()` methods):
  ```cpp
  m_pHeapStart		= nullptr;
  m_capacity			= 0;
  m_isAllocated		= false;
  ```
- `switch` `case` labels sit at the same indentation as `switch`.
- **Multi-line call layout for logging.** Put each argument on its own line:
  ```cpp
  DebugLogError(
  	"{}: SDL_CreateWindow failed: {}",
  	__FUNCTION__,
  	SDL_GetError()
  );
  ```
- **Aggregate init of descs:** braces on their own lines, related values grouped per line:
  ```cpp
  WindowCreationDesc windowDesc
  {
  	defaultWindowWidth, defaultWindowHeight,
  	pDefaultWindowTitle,
  	defaultWindowFlags
  };
  ```
- **Preprocessor:**
  - Directives inside `#if` blocks are indented with a tab (`#if defined(FW_SDL3)` /
    `\t#include <SDL3/SDL.h>`). Inside class/function bodies, `#if`/`#endif` sit at column 0
    or match the surrounding indentation.
  - **Every `#endif` that closes a non-trivial block gets a comment naming its condition:**
    `#endif	// FW_SDL3`, `#endif	// OS_WINDOWS && HW_X64`.
  - Group related code with regions, closing with a matching comment:
    ```cpp
    #pragma region BYTE_COUNT_OPERATIONS
    ...
    #pragma endregion	// BYTE_COUNT_OPERATIONS
    ```
    Region names are `UPPER_SNAKE_CASE`.
- **Comments:** `//` only. A short comment line above a logical step (`// Window creation`,
  `// Renderer creation`). Enum values may have trailing comments (`CPU,		// in RAM`).
  Templates/helpers get a one-line description plus `// Example use: ...`.
- **TODOs:** `// TODO : <description>` (note the space before the colon). TODOs mark intended
  future direction (e.g. "Use own heap allocation", "Make own Graphic System"). Don't delete
  them unless you implement them.

---

## 5. Naming conventions

### 5.1 Primitive types: always use the engine typedefs

Defined in `platformConfigTypes.h`. **Use these instead of the raw keywords:**

| Typedef | Underlying | | Typedef | Underlying |
|---|---|---|---|---|
| `s8` | `signed char` | | `u8` | `unsigned char` |
| `s16` | `short` | | `u16` | `unsigned short` |
| `s32` | `int` | | `u32` | `unsigned int` |
| `s64` | `long long` | | `u64` | `unsigned long long` |
| `b8` | `bool` | | `f32` / `f64` | `float` / `double` |
| `c8` | `char` | | | |

- Signed integers use the **`s` prefix** (`s32`), **never `i`**.
- `size_t`, `std::byte` and `const char*` are currently used as-is.
- Exceptions: signatures dictated by external APIs keep their types (`int, char**` in
  `SDL_AppInit`; SDL enum/handle types).
- New platform-specific typedefs go in `platformConfig/platform/<tag>/<tag>PlatformTypes.h`.

### 5.2 Identifiers

| Kind | Style | Examples |
|---|---|---|
| Classes / structs | `PascalCase` | `Window`, `WindowImpl`, `Heap`, `AppState` |
| Platform impl class | `<Name>Impl`, **no platform tag** | `WindowImpl` |
| Creation / config structs | `<Thing>CreationDesc` (or `<Thing>Desc`) | `WindowCreationDesc` |
| Functions & methods | `PascalCase`, verb-first | `CreateAppWindow`, `AllocateHeap`, `AllocateHeapOnCPU` |
| Member variables | `m_` + `camelCase` | `m_width`, `m_capacity`, `m_memoryLocation` |
| Pointer variables (members, locals, params, constants) | `p` prefix; `pp` for pointer-to-pointer. Smart pointers count. | `m_pWindow`, `m_pWindowImpl`, `pState`, `ppAppState`, `pDefaultWindowTitle` |
| Booleans | `is` prefix | `m_isAllocated`, `isWindowCreated`, `isSuccess` |
| Locals / params | `camelCase`, units in the name where relevant | `capacityInBytes`, `sizeInBytes`, `windowDesc` |
| `constexpr` constants | `camelCase` (not ALL_CAPS); defaults use a `default` prefix | `defaultWindowWidth`, `defaultWindowFlags` |
| `enum class` | type `PascalCase`, values `UPPER_SNAKE_CASE`, **`UNDEFINED` as the last value** | `MemoryPhysicalLocation::CPU`, `AllocatorType::BUMP_ALLOCATOR` |
| Macros | `UPPER_SNAKE_CASE` with an axis prefix | `OS_WINDOWS`, `FW_SDL3`, `BUILD_DEBUG` |
| Acronyms in names | keep upper case | `AllocateHeapOnCPU`, `AllocateHeapOnGPU` |
| Files | `camelCase`; platform files prefixed by the tag | `windowIncludes.h`, `win64WindowImpl.cpp` |
| Folders | `camelCase` module names; `platform/<tag>/` | `platformConfig/`, `memory/heap/`, `window/platform/win64/` |

Always use `enum class`, never plain `enum`.

---

## 6. Class layout and function signatures

### 6.1 Class shape

Reference: `WindowImpl` in `src/window/platform/win64/win64WindowImpl.h`, shown here with
section comments added:

```cpp
#pragma once

class WindowImpl
{
public:
	// 1. Data members first (public): platform-common, then framework-specific
	u32 m_width;
	u32 m_height;
#if defined(FW_SDL3)
	std::shared_ptr<SDL_Window> m_pWindow;
	SDL_WindowFlags m_flags;
#endif	// FW_SDL3

	// 2. Lifecycle
	WindowImpl();
	~WindowImpl();
	b8 Clear();
	b8 Release();

	// 3. Public API
	b8 CreateAppWindow(const WindowCreationDesc& desc);
	void* GetNativeWindowHandle() const;
};
```

- Data members are **public** and declared first, without in-class initialisers.
  `Clear()` sets their values. (Plain data structs such as `AppState` may use in-class
  initialisers like `= nullptr`.)
- Framework-specific members are wrapped in `#if defined(FW_...)` ... `#endif	// FW_...`,
  after the platform-common members.
- Private helpers, if any, go last under `private:` (see `Heap` in `src/memory/heap/heap.h`).
- No virtual functions or inheritance for platform abstraction (see §2.2).
- Declare only, never define, in the header. Templates are the exception (`logging.h`).
- Write method definitions in the `.cpp` in the same order as the header.

### 6.2 Lifecycle idiom: Clear / Release

Every engine class follows this pattern:

| Method | Responsibility |
|---|---|
| Constructor | Calls `Clear()`, then creates owned sub-objects (e.g. `m_pWindowImpl = std::make_shared<WindowImpl>();`) |
| Destructor | Calls `Release()` |
| `Clear()` | Resets **every** member to its default/empty value (`nullptr`, `0`, `false`, `::UNDEFINED`). Frees nothing. |
| `Release()` | Frees owned resources (`delete`, `.reset()`), **then calls `Clear()`** |

`Clear()` and `Release()` return `b8` (`true` on success), in both abstraction and impl classes.

### 6.3 Function signatures

- **Return `b8` for operations that can fail** (`true` = success). Callers check and propagate:
  ```cpp
  if (!AllocateHeapCapacity())
  {
  	return false;
  }
  ```
- Pass structs and descs **by `const&`**: `b8 CreateAppWindow(const WindowCreationDesc& desc);`.
- Group creation parameters into a `<Thing>CreationDesc` struct in `<module>Types.h` instead of
  writing long parameter lists.
- **Always create desc structs with curly-brace initialization** so every member is
  zeroed. Any member without an explicit value is value-initialized (`0`, `nullptr`, `false`,
  or the first enum value), while a plain `WindowCreationDesc windowDesc;` leaves members as
  indeterminate garbage that the impl would then read.
  ```cpp
  // Good: listed values, every remaining member zeroed
  WindowCreationDesc windowDesc
  {
  	defaultWindowWidth, defaultWindowHeight,
  	pDefaultWindowTitle,
  	defaultWindowFlags
  };

  // Good: everything zeroed, then fill in fields
  WindowCreationDesc windowDesc{};
  windowDesc.m_width		= defaultWindowWidth;
  windowDesc.m_height		= defaultWindowHeight;

  // Bad: members are uninitialized
  WindowCreationDesc windowDesc;
  ```
  To keep this working, desc structs stay plain aggregates: public members only, no
  constructors, no virtuals. Never zero them with `memset`.
- Include units in parameter names (`capacityInBytes`, `sizeInBytes`).
- Leave unused parameters **unnamed** (`SDL_AppResult SDL_AppInit(void** ppAppState, int, char**)`).
- Use guard clauses: validate, log, `return false`. Then do the work, then `return true`.
- No exceptions. Failures are reported with a `b8` return and a log line.
- The abstraction and impl expose identical method names (`CreateAppWindow` on both).
- **Generic helpers use C++20 concepts and `if constexpr`, not overload sets or SFINAE.**
  Don't write separate overloads for different numeric types (e.g. `f64` and `u64`
  versions of one function): an `int` argument converts equally well to both, so the call is
  ambiguous. Write one template and constrain it with a `requires` clause on its own
  tab-indented line. Branch on the type inside the body (see `memoryDefines.h`):
  ```cpp
  template<typename T>
  	requires std::integral<T> || std::floating_point<T>
  constexpr u64 KB(T kb)
  {
  	if constexpr (std::floating_point<T>)
  	{
  		return static_cast<u64>(kb * 1024);
  	}
  	else
  	{
  		return static_cast<u64>(kb) << 10;
  	}
  }
  ```
  Standard concepts come from `<concepts>`. Make sure it's included through the std-header
  block of `platformConfigIncludes.h`; don't rely on it arriving through another header.
- Avoid names that clash with OS/framework macros or functions. For example, use
  `CreateAppWindow`, **not** `CreateWindow`, which is a Win32 macro.

### 6.4 Memory and ownership

- Current state: `std::shared_ptr` / `std::make_shared` for owned objects. Framework handles are
  wrapped in `shared_ptr` with a custom deleter:
  ```cpp
  m_pWindow = std::shared_ptr<SDL_Window>(pWindowRawPtr, [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); });
  ```
- **Intended direction:** allocations will move to the engine's own heap/allocator system
  (`memory/heap`, `AllocatorType`, `MemoryPhysicalLocation`). Existing `// TODO : Use own heap
  allocation` markers show where. Keep allocation sites easy to find, and don't spread new
  allocation strategies around.
- Use `KB()/MB()/GB()` from `memoryDefines.h` for byte sizes. Never write `1024 * 1024` by hand.
  They are constrained templates, so any integer or floating-point argument works
  (`KB(4)`, `MB(someU32)`, `GB(1.5)`). Integer arguments use a bit shift
  (`<< 10 / 20 / 30`), and floating-point arguments multiply and then truncate to `u64`.
  Don't pass negative values: converting a negative float to `u64` is undefined behaviour,
  and a negative integer wraps around.

---

## 7. Logging

Defined in `src/logging/logging.h`, available everywhere through `platformConfigIncludes.h`.
These are compile-time checked `std::format_string` templates printing via `std::println`:

| Always | Debug builds only (`BUILD_DEBUG`) |
|---|---|
| `Log(...)` → `[LOG]` | `DebugLog(...)` → `[DEBUG][LOG]` |
| `LogWarning(...)` → `[WARNING]` | `DebugLogWarning(...)` → `[DEBUG][WARNING]` |
| `LogError(...)` → `[ERROR]` | `DebugLogError(...)` → `[DEBUG][ERROR]` |

Convention: **the first placeholder is always the calling function**, and the multi-line layout
from §4 applies:
```cpp
DebugLogWarning(
	"{}: Heap is already allocated, skipping reallocation",
	__FUNCTION__
);
```
Log every failure path before returning `false`, along with significant lifecycle events (created,
released). Never use `printf`/`std::cout` directly.

---

## 8. Application entry point

- `SDL_MAIN_USE_CALLBACKS` is defined in `win64PlatformDefines.h`, so the engine uses SDL3's
  callback entry (`SDL_AppInit`, `SDL_AppEvent`, `SDL_AppIterate`, `SDL_AppQuit`) implemented in
  `app/platform/<tag>/<tag>App.cpp`, guarded by platform **and** framework.
- Global state lives in `AppState` (`app/appTypes.h`), passed through the `void*` app-state pointer
  and recovered with `static_cast<AppState*>(pAppState)`.
- `AppState::m_pRenderer` (raw `SDL_Renderer*`) is temporary until the engine's own graphics
  system exists (`// TODO : Make own Graphic System`). New rendering work should aim for an
  abstraction + `platform/<tag>` impl following §2.2, using the `GAPI_` macros.

---

## 9. Build system (CMake)

- **Sources are listed explicitly, not globbed.** When you add files:
  - `.cpp` → `add_executable(${PROJECT_NAME} ...)` in `src/CMakeLists.txt`
  - `.h` → `target_sources(${PROJECT_NAME} PRIVATE ...)` in the same file, grouped with their
    module (this keeps them visible in the VS Solution Explorer via `source_group(TREE ...)`)
  - If tests need the code, add the `.cpp` to `add_executable(HairlossTest ...)` in
    `tests/CMakeLists.txt` too (tests compile engine sources directly rather than linking a lib)
- Third-party libraries are added in `vendor/CMakeLists.txt` as submodules and exposed through
  the `vendor` INTERFACE target (`tests` INTERFACE target for test-only deps).
- Platform `.cpp` files are currently listed unconditionally. They compile to nothing on other
  platforms because their bodies are platform-guarded. When a second platform lands, gate
  platform source lists in CMake as well (e.g. `if(WIN32)`).
- **Source encoding is UTF-8.** The root `CMakeLists.txt` adds `/utf-8` for MSVC so non-ASCII
  text (e.g. Japanese string literals) compiles the same on every machine, whatever its system
  code page. GCC/Clang already default to UTF-8. Save files as UTF-8 without a BOM (see
  `.editorconfig`). Engine-wide compile flags go in the root `CMakeLists.txt` *after*
  `add_subdirectory(vendor)`, so vendored code keeps its own flags.
- Configure/build via presets (`cmake --preset vs2026-x64`, `cmake --build --preset vs2026-x64-debug`).
  Build output goes to `project/<preset>/`. Don't commit it.

---

## 10. Tests

- Catch2 v3 in `tests/`. Shared includes are in `tests/testIncludes.h`, which includes Catch2 headers
  and the relevant engine `<module>Includes.h`.
- One file per module: `<module>Test.cpp`. Each starts with `#include "testIncludes.h"`.
- `TEST_CASE("Human Readable Name", "[module]")` with Allman braces. Use `REQUIRE` on `b8`
  results. Tests use the **abstraction class**, never the impl directly.
- Framework-specific desc fields are filled inside `#if defined(FW_...)` blocks.

---

## 11. Agent checklist (before you finish an edit)

- [ ] Used `s8..s64 / u8..u64 / b8 / f32 / f64 / c8`, not raw primitives
- [ ] Tabs, LF line endings, Allman braces, aligned `Clear()` assignments, multi-line log calls
- [ ] `m_` members, `p`/`pp` pointers, `is` booleans, PascalCase methods, `UNDEFINED` last in enums
- [ ] `.cpp` includes only its module's `Includes.h`. Leaf headers have no includes
- [ ] No raw compiler/OS macros outside `platformConfigDefines.h`; `#if defined(...)` everywhere
- [ ] No `OS_`/`HW_` checks in abstraction classes. Framework code only in impls/platformConfig
- [ ] Every `#endif` commented with its condition. Regions closed with a matching comment
- [ ] Constructor→`Clear()`, destructor→`Release()`, `Release()`→`Clear()`
- [ ] Desc structs created with `{ ... }` / `{}` initialization, never left uninitialized
- [ ] Fallible functions return `b8`, log failures with `"{}: ..."` + `__FUNCTION__`
- [ ] New files registered in `src/CMakeLists.txt` (and `tests/CMakeLists.txt` if relevant)
- [ ] No edits under `vendor/` or `project/`. Existing `// TODO :` markers preserved

---

## 12. Known issues in the current code (don't copy these patterns)

None open right now. When you spot a bug or a deviation from this guide that you aren't fixing
in your current change, list it here so the next agent doesn't copy it.
