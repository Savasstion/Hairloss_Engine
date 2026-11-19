# AGENTS.md — Hairloss Engine

Rules for anyone (AI or human) editing this repo. **Read it all before changing code.** When unsure,
copy the patterns in `src/window/` and `src/memory/cpuHeap/`.

---

## 1. Project

| | |
|---|---|
| Goal | Cross-platform, cross-framework, modular C++ game engine |
| Language | C++23, no compiler extensions |
| Build | CMake ≥ 3.28 (≥ 4.2 for VS2026), presets in `CMakePresets.json`, output in `project/<preset>/` |
| Targets | Now: Windows x64 (`win64`) via SDL3. Planned: Linux, Xbox One, Xbox Series X\|S |
| Graphics API (planned) | Vulkan on Windows/Linux, DX12 on Xbox |
| Vendored | `vendor/SDL` (static), `vendor/Catch2` (submodules). **Never edit `vendor/`** |
| Tests | Catch2 v3 in `tests/`, target `HairlossTest`, run with `ctest` |

```
src/
├── platformConfig/          # Platform macros, primitive typedefs, global include hub
│   └── platform/win64/      # win64PlatformDefines/Includes/Types.h (OS + framework headers)
├── app/                     # Entry point
│   ├── app.h/.cpp           # GetAppState(): the single static AppState (platform agnostic)
│   ├── appDefines.h / appTypes.h / appIncludes.h
│   └── platform/win64/      # win64App.h/.cpp (SDL main callbacks)
├── window/                  # Window + platform/win64/win64WindowImpl
├── logging/                 # Log / DebugLog family
└── memory/                  # memoryTypes.h / memoryUtils.h / memoryIncludes.h
    ├── cpuHeap/             # CpuHeap (owns its allocator) + platform/win64/win64CpuHeapImpl (VirtualAlloc)
    └── allocator/           # CpuHeapAllocator (abstract, pmr), CpuHeapBumpAllocator, CpuHeapFreeListAllocator
        └── memoryResource/  # NoGrowthMemoryResource (pmr upstream, aborts when the heap is full),
                             # FreeListMemoryResource (best fit), memoryResourceTypes.h
```

---

## 2. Cross-platform architecture

### 2.1 Compile-time macros only
No runtime dispatch, factories or virtual interfaces for platforms. All macros live in
`src/platformConfig/platformConfigDefines.h`:

| Prefix | Meaning | Values |
|---|---|---|
| `HW_` | CPU / hardware / console | `HW_X64`, `HW_X86`, `HW_ARM64`, `HW_ARM32`, `HW_64BITS`, `HW_32BITS`, `HW_XONE`, `HW_XSX` |
| `OS_` | Operating system | `OS_WINDOWS`, `OS_LINUX` |
| `FW_` | Framework | `FW_SDL3` |
| `GAPI_` | Graphics API | `GAPI_VULKAN`, `GAPI_DX12` |
| `BUILD_` | Build mode | `BUILD_DEBUG` |

- Define as `1`. Test only with `#if defined(X)`, never `#ifdef`, never the value.
- Raw compiler macros (`_WIN64`, `__linux__`, `_DEBUG`, ...) appear **only** in `platformConfigDefines.h`.
- Frameworks are chosen in its `ENGINE_FRAMEWORKS` region and graphics APIs in `GRAPHICS_API`, never locally.
- **Platform tag:** a short name for an `OS_`/`HW_` combination, used in folder and file names.
  `win64` = `defined(OS_WINDOWS) && defined(HW_X64)`. New platforms add a tag and its condition.
- Framework defines needed before framework headers (e.g. `SDL_MAIN_USE_CALLBACKS`) go in
  `platform/<tag>/<tag>PlatformDefines.h`, inside `#if defined(FW_...)`.

### 2.2 Abstraction + platform impl (compile-time pimpl)
Each platform-dependent subsystem has two classes.

**Abstraction** (`Window` in `<module>.h/.cpp`). It's what the engine and tests use.
- Platform-agnostic: no `OS_`/`HW_`/`FW_` checks.
- Holds a non-owning `CpuHeapAllocator* m_pAllocator` (passed to the constructor) and a raw `WindowImpl* m_pWindowImpl`.
- The constructor does `m_pWindowImpl = m_pAllocator->NewObject<WindowImpl>();`.
  `Release()` does `m_pAllocator->DeleteObject(m_pWindowImpl);`.
- Public methods forward to the impl. If the impl is `nullptr`, they log and return `false` / `nullptr`.

**Impl** (`WindowImpl` in `platform/<tag>/<tag><Module>Impl.h/.cpp`)
- **Same class name on every platform.** Only the file and folder carry the tag.
- Same method names and signatures as the abstraction.
- The whole `.cpp` body sits inside the platform guard:
  `#if defined(OS_WINDOWS) && defined(HW_X64)` ... `#endif	// OS_WINDOWS && HW_X64`.
- Framework code and framework-typed members go inside `#if defined(FW_SDL3)`. Platform-common
  members go outside it.

**Selection** happens in `<module>Includes.h`, which includes the impl header before the abstraction:
```cpp
#if defined(OS_WINDOWS) && defined(HW_X64)
	#include "platform/win64/win64WindowImpl.h"
#endif

#include "window.h"
```
A new platform means one `platform/<tag>/` folder plus one `#elif` per module. Callers don't change.

### 2.3 Where framework (`FW_`) code may go
| Location | Allowed |
|---|---|
| `platform/<tag>/` impls, `platformConfig/platform/<tag>/` | ✅ |
| `<module>Types.h` / `<module>Defines.h` | ✅ only guarded framework-typed desc fields / constants |
| `app/` entry point, `AppState` | ✅ (SDL callbacks, temporary `SDL_Renderer`) |
| Tests | ✅ only to fill framework desc fields |
| Abstraction `.h/.cpp` | ❌ |
| `logging/`, `memory/` | ❌ (included before OS/framework headers, §3.3). Exception: OS calls, not OS types in headers, in `memory/**/platform/<tag>/*.cpp` (e.g. `VirtualAlloc`) |

- Never reach through an abstraction into its impl (`m_pWindow->m_pWindowImpl->...`). Add a method instead.
- Expose framework objects type-erased as `void* GetNative<Thing>Handle() const`, returning `nullptr`
  without the framework. Callers cast it back inside their own `FW_` block:
  `static_cast<SDL_Window*>(pWindow->GetNativeWindowHandle())`.

---

## 3. Files and includes

### 3.1 Module file set
A module (`src/<module>/`) or sub-module (`src/<module>/<sub>/`) creates only the files it needs:

| File | Contents |
|---|---|
| `<module>Includes.h` | **The only header anyone includes for the module.** Sets the include order |
| `<module>Defines.h` | Macros and `constexpr` constants only, **no functions** |
| `<module>Types.h` | `enum class`es, plain structs, creation descs |
| `<module>Utils.h` | Free helpers defined in the header (`template` / `constexpr` / `inline`), e.g. `KB()` |
| `<module>.h/.cpp` | Abstraction class |
| `platform/<tag>/<tag><Module>Impl.h/.cpp` | Platform impl |

Called like a function → Utils. Names a value or a switch → Defines.

### 3.2 Include rules
- Every header starts with `#pragma once`.
- **Leaf headers** (`<module>.h`, Defines, Types, Utils, impl headers) have **no `#include`s**.
- **Every `.cpp` includes exactly one header, its module's `Includes.h`**, by relative path
  (`#include "../../windowIncludes.h"` from `platform/win64/`). The only exception is `SDL_main.h` (§3.3).
- Engine includes use relative paths in quotes. External headers use `<...>`.
- `<module>Includes.h` order:
  1. `../platformConfig/platformConfigIncludes.h`, or the parent's Includes for a sub-module
  2. Other modules' Includes it depends on
  3. Module-specific framework headers (inside `FW_` guards)
  4. Defines → Types → Utils
  5. Platform impl headers (inside platform guards, indented `#include`)
  6. `<module>.h`
  7. Sub-module Includes: `memoryIncludes.h` → `cpuHeap/cpuHeapIncludes.h` and `allocator/allocatorIncludes.h`;
     `allocatorIncludes.h` → `memoryResource/memoryResourceIncludes.h` (see the exception below)
- **Exception:** an Includes header may include a sub-module leaf early when one of its own leaves holds
  that type by value. `allocatorIncludes.h` includes `memoryResource/freeListMemoryResource.h` right
  before `cpuHeapFreeListAllocator.h`.

### 3.3 Global include hub: `platformConfigIncludes.h`
It includes, in order:
1. `platformConfigDefines.h`
2. `platformConfigTypes.h`
3. Engine-wide std headers. Add new ones here and don't rely on them arriving transitively.
4. `logging/loggingIncludes.h`, `memory/memoryIncludes.h`
5. `platform/<tag>/<tag>PlatformIncludes.h`: PlatformDefines → `<Windows.h>` → `<SDL3/SDL.h>` → PlatformTypes

- Logging and memory come **before** OS/framework headers, so their headers can't use OS/framework types.
- Circular includes are expected and resolved by `#pragma once`. Keep this order.
- Headers can be read in a different order depending on which Includes header a `.cpp` starts from.
  Forward-declare types where needed (see `memoryTypes.h`, e.g. `FreeListNode` for the early-read
  `freeListMemoryResource.h`).
- Templates in headers read before `logging.h` can't call the log functions. Put logging in a non-template
  helper defined in the `.cpp` (see `CpuHeapAllocator::CheckInitialized`).
- OS/framework headers come in **only** through `platformConfig/platform/<tag>/`. **One exception:**
  `SDL_main.h` defines `main`, so only the platform entry `.cpp` includes it, inside its platform +
  framework guard, never from an Includes header. Every `App.cpp` may be compiled on any build, but
  only the matching one gets `main`:
  ```cpp
  #include "../../appIncludes.h"

  #if defined(OS_WINDOWS) && defined(HW_X64)
  #if defined(FW_SDL3)

  #include <SDL3/SDL_main.h>
  ```

---

## 4. Formatting

- **Tabs** for indentation. Spaces only for alignment after code.
- **LF** line endings (`.gitattributes`). CRLF only for `.bat`, `.cmd`, `.ps1`, `.sln`, `.vcxproj`.
- `.editorconfig` sets indentation, UTF-8 without BOM, the final newline and trailing whitespace.
  Don't override it. JSON/YAML use 2 spaces.
- **Allman braces** everywhere.
- Single-statement early returns may skip braces (as in `win64App.cpp`). Prefer braces in class methods.
- A blank line before `return` and between logical steps.
- `case` labels sit at the same indentation as `switch`.
- **Align `=` with tabs** in consecutive member assignments (`Clear()`):
  ```cpp
  m_pBase			= nullptr;
  m_capacity		= 0;
  m_isAllocated	= false;
  ```
- **Log calls:** one argument per line:
  ```cpp
  DebugLogError(
  	"{}: SDL_CreateWindow failed: {}",
  	__FUNCTION__,
  	SDL_GetError()
  );
  ```
- **Desc aggregate init:** braces on their own lines, related values grouped per line:
  ```cpp
  WindowCreationDesc windowDesc
  {
  	defaultWindowWidth, defaultWindowHeight,
  	pDefaultWindowTitle,
  	defaultWindowFlags
  };
  ```
- **Preprocessor:**
  - Indent directives inside `#if` blocks with a tab. In function bodies, `#if`/`#endif` sit at column 0.
  - Every non-trivial `#endif` names its condition: `#endif	// FW_SDL3`, `#endif	// OS_WINDOWS && HW_X64`.
  - Regions: `#pragma region UPPER_SNAKE_CASE` ... `#pragma endregion	// UPPER_SNAKE_CASE`.
- **Comments:**
  - `//` only. **Short, simple, clear:** one line, a few words (`// Window creation`, `// not owned`).
  - Say *what*. Add *why* only when it isn't obvious, briefly. No multi-line explanations.
  - Helpers and templates get a one-line description plus `// Example use: ...`.
  - **Never mention AGENTS.md, docs or section numbers in code.** Rules live here.
- **TODOs:** `// TODO : <description>` (space before the colon). Keep them until implemented.

---

## 5. Naming

### 5.1 Primitive types
Use the `platformConfigTypes.h` typedefs, not raw keywords:
`s8 s16 s32 s64` (signed, **never `i`**), `u8 u16 u32 u64`, `b8` (bool), `f32 f64`, `c8` (char).
- `size_t`, `std::byte` and `const char*` are used as-is.
- External API signatures keep their types (`int, char**` in `SDL_AppInit`, the standard `std::pmr` overrides).
- Platform typedefs go in `platformConfig/platform/<tag>/<tag>PlatformTypes.h`.

### 5.2 Identifiers
| Kind | Style | Examples |
|---|---|---|
| Classes / structs | `PascalCase` | `Window`, `CpuHeap`, `AppState` |
| Platform impl | `<Name>Impl`, no tag | `WindowImpl` |
| Creation structs | `<Thing>CreationDesc` | `WindowCreationDesc` |
| Functions / methods | `PascalCase`, verb first | `CreateAppWindow`, `GetBasePointer` |
| Members | `m_` + `camelCase` | `m_width`, `m_capacity` |
| Pointers (any kind) | `p` prefix, `pp` for pointer-to-pointer | `m_pWindowImpl`, `pState`, `ppAppState` |
| Booleans | `is` prefix | `m_isAllocated`, `isSuccess` |
| `static` variables | **No `s_` / `g_` prefix**, named like any other variable | `static AppState appState;` |
| Locals / params | `camelCase`, units in names | `capacityInBytes`, `windowDesc` |
| `constexpr` constants | `camelCase`, defaults prefixed `default` | `defaultWindowWidth` |
| `enum class` | `PascalCase` type, `UPPER_SNAKE_CASE` values, **`UNDEFINED` last** | `AllocatorType::MONOTONIC` |
| Macros | `UPPER_SNAKE_CASE` with an axis prefix | `OS_WINDOWS`, `FW_SDL3` |
| Acronyms | Upper case in functions; `Cpu`/`Gpu` in type and file names | `GetNativeWindowHandle`, `CpuHeap`, `cpuHeap.h` |
| Files / folders | `camelCase`; platform files prefixed by tag | `windowIncludes.h`, `win64WindowImpl.cpp`, `memory/cpuHeap/` |

- Always use `enum class`.
- American spelling in identifiers and logs: `Initialize`, `initialized`.
- Avoid names that clash with OS macros: `CreateAppWindow`, not `CreateWindow`. Don't use `<Windows.h>`'s
  lowercase macros `small`, `near`, `far`, `min`, `max` as variable names.

---

## 6. Classes and functions

### 6.1 Class shape
```cpp
class WindowImpl
{
public:
	// 1. Public data members: platform-common, then framework (guarded)
	u32 m_width;
	u32 m_height;
#if defined(FW_SDL3)
	SDL_Window* m_pWindow;		// owned by SDL
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
- No in-class initializers: `Clear()` sets members. Plain data structs (`AppState`) may use `= nullptr`.
- Private helpers go last under `private:`, protected ones under `protected:`.
- **No virtuals or inheritance for platforms.** The one exception is runtime strategy hierarchies:
  abstract `CpuHeapAllocator` → `CpuHeapBumpAllocator` / `CpuHeapFreeListAllocator`, via the pure
  virtuals `GetMemoryResource()`, `Reset()` and protected `Initialize()`, plus a virtual `Release()`.
  - Overrides use `override`.
  - Constructors, destructors and `Release()` call their own class's `Clear()` explicitly
    (`CpuHeapBumpAllocator::Clear()`), because virtual dispatch doesn't reach the child there.
- Classes owning a raw pointer or resource (`Window`, `CpuHeap`, allocators) `= delete` their copy
  constructor and copy assignment.
- Headers declare only. The exceptions are templates (`logging.h`, `CpuHeapAllocator::NewObject` /
  `DeleteObject`) and Utils helpers.
- Define methods in the `.cpp` in header order.

### 6.2 Lifecycle: Clear / Release
| Method | Does |
|---|---|
| Constructor | `Clear()`, then creates owned objects (`m_pAllocator->NewObject<...>()`) |
| Destructor | `Release()` |
| `Clear()` | Resets **every** member (`nullptr`, `0`, `false`, `::UNDEFINED`). Frees nothing |
| `Release()` | Frees owned resources (`DeleteObject(...)`, `SDL_DestroyWindow`, ...), **then** `Clear()` |

Both return `b8` (`true` on success).

### 6.3 Functions
- Fallible functions **return `b8`** (`true` = success). Callers check and propagate.
- Guard clauses first: validate, log, `return false`. Then do the work and `return true`.
- **No exceptions.** Report failures with a `b8` return and a log line.
- Pass structs **by `const&`**. Group creation parameters into a `<Thing>CreationDesc` in `<module>Types.h`.
- **Always brace-initialize descs** (`Desc desc{ ... }` or `Desc desc{}`), so unlisted members are zeroed.
  Never leave them uninitialized (`Desc desc;`), and never `memset` them. Descs stay plain aggregates:
  public members, no constructors, no virtuals.
- Put units in parameter names (`sizeInBytes`). Leave unused parameters unnamed.
- Abstraction and impl use identical method names.
- **Generic helpers use one constrained template, not overloads or SFINAE.** Put the `requires`
  clause on its own tab-indented line and branch with `if constexpr` (see `memoryUtils.h`):
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

### 6.4 Memory and ownership

**Engine objects live on the engine CPU heap.**
- Never use `new`, `delete`, `make_shared`, `shared_ptr` or `unique_ptr` for engine objects.
- Use raw pointers. The owner frees them in `Release()`:
  ```cpp
  m_pWindowImpl = m_pAllocator->NewObject<WindowImpl>();	// constructor
  m_pAllocator->DeleteObject(m_pWindowImpl);				// Release(), nulls the pointer
  ```
- **Pass the allocator in.** There's no global memory system. A class that allocates takes a
  `CpuHeapAllocator*` (constructor or desc) and keeps it as a non-owning `m_pAllocator`, which must
  outlive its allocations.

**`CpuHeap` owns the memory and its one allocator. The allocator hands it out through `std::pmr`.**
- `AllocateHeap` creates the allocator picked by `CpuHeapCreationDesc::m_allocatorType` (zeroed = `MONOTONIC`)
  and initializes it over the memory. `UNDEFINED` fails and gives the memory back.
- Allocate through `heap.GetAllocator()->NewObject<T>()`. `GetAllocator()` is `nullptr` before
  `AllocateHeap` and after `Release()`.
- One allocator per heap. `Initialize` is a protected pure virtual and `CpuHeapAllocator` declares
  `friend class CpuHeap;`, so only the heap initializes its allocator (subclasses keep their override
  private). Two allocators over one heap would hand out the same memory.
- **Choosing an allocator:**
  - `MONOTONIC`: fastest. `DeleteObject` frees nothing until `Reset()`. For per-frame or load-once data.
  - `FREE_LIST`: best fit. `DeleteObject` frees for reuse and merges free neighbours. Costs a 16-byte
    header per allocation and O(free blocks) per allocate and free. For objects created and destroyed
    in any order.
- `CpuHeap` has no `NewObject` template, because `cpuHeap.h` can be read before the allocator headers.
- `CpuHeapAllocator` is abstract and can't be instantiated. Pure virtuals: `GetMemoryResource()`, `Reset()`,
  `Initialize()`.
- `Release()` is virtual, so it's safe through the `GetAllocator()` base pointer.
- Each subclass sets its `AllocatorType` tag and owns its concrete pmr resource as a member
  (`CpuHeapBumpAllocator`: `std::pmr::monotonic_buffer_resource`, `MONOTONIC`;
  `CpuHeapFreeListAllocator`: our `FreeListMemoryResource`, `FREE_LIST`).
  - The resource is built over the heap's start and size in `Initialize(pCpuHeap)` and exposed by `GetMemoryResource()`.
  - std resources take their buffer only at construction, so `Initialize` / `Reset` / `Release` rebuild them
    in place (`std::destroy_at` + `std::construct_at`). MSVC's `release()` doesn't rewind the initial buffer.
    Our own resources (`FreeListMemoryResource`) have `Initialize` / `Reset` / `Release` instead.
- Upstream is always `GetNoGrowthMemoryResource()`: an exhausted heap logs and aborts. It never grows
  onto the CRT heap and never throws.
- `NewObject<T>` / `DeleteObject` wrap `polymorphic_allocator::new_object` / `delete_object`.
  - `DeleteObject` needs the exact created type, because a base pointer would free the wrong address
    and size. Polymorphic classes it deletes must be `final` (checked by a `static_assert`).
  - It accepts `const T*`.
- **Teardown: `DeleteObject` engine objects before `CpuHeap::Release()`.**
  - `CpuHeap::Release()` deletes its allocator before freeing the memory, so that order is automatic.
  - Allocator released early by hand: `DeleteObject` still runs the destructor but frees nothing.
- **Dangling pointers are left as is. Be wary of object lifetimes.**
  - After `CpuHeap::Release()`, every `CpuHeapAllocator*` copied from `GetAllocator()` (e.g. `Window::m_pAllocator`)
    and every pointer into the heap dangles. Nothing checks or nulls them.
  - Before using a stored pointer, make sure its heap and allocator still outlive it.
- **`Reset()` / `CpuHeap::Release()` with live objects is fine when everything is in that heap.**
  - For both allocators, these invalidate **every** allocation at once, without running destructors (pmr containers on
    `GetMemoryResource()` too).
  - Fine when every live object, its impl and everything they allocate came from that same heap,
    so it is all dropped together.
  - Not fine when a live object owns something outside the heap (framework objects such as
    `SDL_Window`, other heaps): `DeleteObject` it first, or that resource leaks.
  - Don't touch the dropped objects or their pointers afterwards.
- For STL containers, use `std::pmr` containers on `GetMemoryResource()`.

**Exceptions** (the only places allowed to skip the engine heap)
- `CpuHeap` allocates its `CpuHeapImpl` and its allocator with `new` / `delete`, because the heap
  can't allocate itself.
- The root `AppState` is static (`GetAppState()` in `app/app.cpp`, platform agnostic) and owns the
  root `CpuHeap` by value. Entry points get it from `GetAppState()` and never declare their own.

**Framework memory belongs to the framework.** Create and destroy framework objects through their
API (`SDL_CreateWindow` / `SDL_DestroyWindow`), store raw pointers, and destroy them in `Release()`.
Never route framework allocations through the engine heap.

**Sizes:** use `KB()/MB()/GB()`, never `1024 * 1024`. Integers are bit-shifted, and floats are
multiplied then truncated to `u64`. Never pass negative values (UB for floats, wraps for integers).

---

## 7. Logging
`src/logging/logging.h`, available everywhere. Compile-time checked `std::format_string` → `std::println`.

| Always | Debug only (`BUILD_DEBUG`) |
|---|---|
| `Log` → `[LOG]` | `DebugLog` → `[DEBUG][LOG]` |
| `LogWarning` → `[WARNING]` | `DebugLogWarning` → `[DEBUG][WARNING]` |
| `LogError` → `[ERROR]` | `DebugLogError` → `[DEBUG][ERROR]` |

- The first placeholder is always the caller: `"{}: ..."` with `__FUNCTION__`.
- Log every failure path before returning `false`, plus lifecycle events (created, released).
- Never use `printf` or `std::cout`.

---

## 8. Application entry point
- SDL3 callback entry (`SDL_MAIN_USE_CALLBACKS` in `win64PlatformDefines.h`). `SDL_AppInit`,
  `SDL_AppEvent`, `SDL_AppIterate` and `SDL_AppQuit` live in `app/platform/<tag>/<tag>App.cpp`, inside
  platform + framework guards, which also include `SDL_main.h` (§3.3).
- `win64App.h` declares the callbacks `extern "C"` with `SDLCALL`, to match SDL.
- **`SDL_AppInit`:** gets `GetAppState()`, allocates the root heap, takes its `GetAllocator()`,
  creates engine objects with `NewObject`, and hands the state over via `*ppAppState`. The other
  callbacks use `static_cast<AppState*>(pAppState)`.
- **`SDL_AppQuit`:** destroys in reverse: framework objects, then engine objects (`DeleteObject`), then
  `m_appHeap.Release()`, which also deletes the allocator.
- `AppState::m_pRenderer` is temporary (`// TODO : Make own Graphic System`). New rendering work
  follows §2.2 and uses the `GAPI_` macros.

---

## 9. Build (CMake)
- **List sources explicitly** (no globbing):
  - `.cpp` → `add_executable(${PROJECT_NAME} ...)` in `src/CMakeLists.txt`
  - `.h` → `target_sources(...)` there, grouped by module
  - Tests compile engine sources directly, so also add the `.cpp` to `tests/CMakeLists.txt` if tests use it
- Vendored libs go in `vendor/CMakeLists.txt`, exposed via the `vendor` INTERFACE target (`tests` for test-only).
- Platform `.cpp` files are listed unconditionally, and their bodies compile to nothing on other platforms.
  Gate them per platform (`if(WIN32)`) once a second platform lands.
- MSVC gets `/utf-8` (root `CMakeLists.txt`). Engine-wide flags go after `add_subdirectory(vendor)`.
- Configure and build: `cmake --preset vs2026-x64`, then `cmake --build --preset vs2026-x64-debug`.
  Reconfigure after adding or moving files. Never commit `project/`.

---

## 10. Tests
- One `<module>Test.cpp` per module, each starting with `#include "testIncludes.h"`.
- `testIncludes.h` includes Catch2 and the engine Includes, and provides `TestCpuHeap`:
  a test-owned heap (`TestCpuHeap(capacityInBytes, allocatorType = MONOTONIC)`) that `REQUIRE`s its own setup, `GetAllocator()` (forwards to the heap, `nullptr`
  after `Release()`), plus `IsInHeap()`.
- `TEST_CASE("Readable Name", "[module]")`, Allman braces, `REQUIRE` on `b8` results.
- Use abstraction classes, never impls. Fill framework desc fields inside `FW_` guards.

---

## 11. Checklist
- [ ] Engine typedefs (`s32`, `u64`, `b8`, ...), not raw primitives
- [ ] Tabs, LF, Allman, aligned `Clear()`, multi-line logs, named `#endif`s, closed regions
- [ ] `m_` / `p` / `is` prefixes, no `s_` / `g_`, PascalCase methods, `UNDEFINED` last, American spelling
- [ ] `.cpp` includes only its Includes header, and leaf headers include nothing
- [ ] Raw compiler macros only in `platformConfigDefines.h`, `#if defined(...)` everywhere
- [ ] No `OS_`/`HW_`/`FW_` in abstractions
- [ ] Constructor → `Clear()`, destructor → `Release()`, `Release()` → `Clear()`
- [ ] Descs brace-initialized
- [ ] Engine objects via `CpuHeapAllocator::NewObject` / `DeleteObject` with raw pointers (only the §6.4 exceptions)
- [ ] Fallible functions return `b8` and log failures with `"{}: ..."` + `__FUNCTION__`
- [ ] Short comments, no AGENTS.md references in code
- [ ] New files registered in CMake (and tests if relevant)
- [ ] No edits to `vendor/` or `project/`, existing `// TODO :` markers kept

---

## 12. Known issues
None open. List any bug or guide deviation you notice but don't fix here, so others don't copy it.
