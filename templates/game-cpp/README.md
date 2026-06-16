# ZENO C++ Game Template

This is the smallest standalone C++ game target that uses the ZENO SDK `GameApp` runtime.

It is intentionally plain CMake: the target is part of the repository-level `windows-msvc-debug` and `windows-msvc-release` presets, so CLI, Visual Studio 2022 Open Folder, and VS Code CMake Tools all build the same code.

## Build

From the repository root:

```powershell
cargo build -p zeno_abi
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug --target zeno_template_game_cpp
```

## Run

```powershell
.\build\windows-msvc-debug\bin\Debug\zeno_template_game_cpp.exe
```

The executable expects its copied `assets/` directory beside it. The CMake target copies `assets/project.zproj` and `assets/scenes/template_scene.zscene` after build.

## Package

From the repository root:

```powershell
.\scripts\package-runtime.ps1
```

The packaged executable and assets are written under `build\package\windows-msvc-debug\bin\template-game\`.

## Template Shape

- `src/main.cpp` creates `zeno::GameApp`.
- `src/template_module.cpp` implements static-linked `GameModule` callbacks.
- `assets/project.zproj` selects window size and startup scene.
- `assets/scenes/template_scene.zscene` proves `GameApp` loads startup project/scene data for a non-sample target. The template module validates that loaded scene context, then renders a hardcoded triangle to keep the starting point small.

This template is not a project generator, installer, hot reload system, or plugin loader.
