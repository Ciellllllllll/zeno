# Create An Empty SDK Project

This tutorial starts from the packaged SDK created by the repository scripts. It does not require repository include paths or private engine headers.

## Create The Package

From the repository root:

```powershell
.\scripts\package-sdk.ps1
```

This creates:

```text
build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1/
build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1.zip
```

For a ZIP-only workflow, extract the ZIP and use the extracted `ZenoEngine-SDK-v0.1.0-rc.1/` directory as the SDK root.

## Start From The Template

The package includes:

```text
templates/cpp_empty/
```

This template is a minimal external CMake project. It includes only the public SDK header:

```cpp
#include <zeno/game_module.hpp>
```

It creates `zeno::Engine`, runs one headless frame with `step_frame`, prints the frame index, and exits.

## Minimal CMake Shape

External SDK projects should use this CMake pattern:

```cmake
find_package(ZenoEngine CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE ZenoEngine::zeno_sdk_cpp)
```

The package config provides include paths and imported library locations for Debug and Release.

## Runtime DLL

The executable needs `zeno_abi.dll` beside it at runtime. The template copies the matching configuration DLL with:

```cmake
"${ZenoEngine_BIN_DIR}/$<CONFIG>/zeno_abi.dll"
```

Use the same pattern in your own external project.
