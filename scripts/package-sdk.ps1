param(
    [ValidateSet("Debug", "Release", "All")]
    [string]$Configuration = "All",
    [string]$PackageVersion = "0.1.0-dev",
    [string]$CMakeExe = "cmake",
    [switch]$NoZip
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Copy-RequiredFile {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source)) {
        throw "Required SDK package input not found: $Source"
    }

    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Destination) | Out-Null
    Copy-Item -LiteralPath $Source -Destination $Destination -Force
}

function Copy-RequiredDirectory {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source)) {
        throw "Required SDK package directory not found: $Source"
    }

    if (Test-Path -LiteralPath $Destination) {
        Remove-Item -LiteralPath $Destination -Recurse -Force
    }

    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Destination) | Out-Null
    Copy-Item -LiteralPath $Source -Destination $Destination -Recurse -Force
}

function Copy-DirectoryContents {
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [Parameter(Mandatory = $true)][string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source)) {
        throw "Required SDK package directory not found: $Source"
    }

    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
    Copy-Item -Path (Join-Path $Source "*") -Destination $Destination -Recurse -Force
}

function Get-ExpectedSdkSampleAssetPaths {
    @(
        "sample_manifest.txt",
        "project.zproj",
        "projects/2d_input_audio.zproj",
        "projects/3d_mesh.zproj",
        "scenes/sample_scene.zscene",
        "scenes/2d_input_audio.zscene",
        "scenes/3d_mesh.zscene",
        "shaders/sample_triangle.hlsl",
        "audio/sample_click.wav",
        "textures/sample_sprite_2x2.bmp"
    )
}

function Assert-FileNonEmpty {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Missing ${Description}: $Path"
    }

    $item = Get-Item -LiteralPath $Path
    if ($item.Length -le 0) {
        throw "${Description} is empty: $Path"
    }
}

function Get-RelativeAssetFilePaths {
    param([Parameter(Mandatory = $true)][string]$AssetRoot)

    $fullRoot = [System.IO.Path]::GetFullPath($AssetRoot)
    $prefix = $fullRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar, [System.IO.Path]::AltDirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
    Get-ChildItem -LiteralPath $fullRoot -Recurse -File | ForEach-Object {
        [System.IO.Path]::GetFullPath($_.FullName).Substring($prefix.Length).Replace("\", "/")
    }
}

function Assert-NoAssetFileCollisions {
    param(
        [Parameter(Mandatory = $true)][string]$FirstAssetRoot,
        [Parameter(Mandatory = $true)][string]$SecondAssetRoot
    )

    $firstFiles = @(Get-RelativeAssetFilePaths -AssetRoot $FirstAssetRoot)
    $secondFiles = @(Get-RelativeAssetFilePaths -AssetRoot $SecondAssetRoot)
    $collisions = $firstFiles | Where-Object { $_ -in $secondFiles }
    if ($collisions) {
        throw "SDK sample asset source trees contain colliding relative files: $($collisions -join ', ')"
    }
}

function Assert-RequiredSdkSampleAssets {
    param([Parameter(Mandatory = $true)][string]$AssetRoot)

    foreach ($relativePath in Get-ExpectedSdkSampleAssetPaths) {
        Assert-FileNonEmpty -Path (Join-Path $AssetRoot $relativePath) -Description "SDK sample asset $relativePath"
    }
}

function Write-PackagedCMakePresets {
    param(
        [Parameter(Mandatory = $true)][string]$Destination
    )

    $presets = @'
{
  "version": 6,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 24,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "windows-msvc-debug",
      "displayName": "Windows MSVC Debug",
      "generator": "Visual Studio 17 2022",
      "binaryDir": "${sourceDir}/build/windows-msvc-debug",
      "architecture": {
        "value": "x64",
        "strategy": "external"
      },
      "cacheVariables": {
        "ZenoEngine_DIR": "${sourceDir}/../../cmake"
      }
    },
    {
      "name": "windows-msvc-release",
      "displayName": "Windows MSVC Release",
      "generator": "Visual Studio 17 2022",
      "binaryDir": "${sourceDir}/build/windows-msvc-release",
      "architecture": {
        "value": "x64",
        "strategy": "external"
      },
      "cacheVariables": {
        "ZenoEngine_DIR": "${sourceDir}/../../cmake"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "windows-msvc-debug",
      "displayName": "Build Windows MSVC Debug",
      "configurePreset": "windows-msvc-debug",
      "configuration": "Debug"
    },
    {
      "name": "windows-msvc-release",
      "displayName": "Build Windows MSVC Release",
      "configurePreset": "windows-msvc-release",
      "configuration": "Release"
    }
  ]
}
'@

    Set-Content -LiteralPath $Destination -Value $presets -Encoding ascii
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    $packageName = "ZenoEngine-SDK-v$PackageVersion"
    $packageRoot = Join-Path $repoRoot "build/package-sdk/$packageName"
    $zipPath = Join-Path $repoRoot "build/package-sdk/$packageName.zip"
    $configurations = if ($Configuration -eq "All") { @("Debug", "Release") } else { @($Configuration) }

    if (Test-Path -LiteralPath $packageRoot) {
        Remove-Item -LiteralPath $packageRoot -Recurse -Force
    }
    if (Test-Path -LiteralPath $zipPath) {
        Remove-Item -LiteralPath $zipPath -Force
    }

    New-Item -ItemType Directory -Force -Path `
        (Join-Path $packageRoot "include"), `
        (Join-Path $packageRoot "lib/Debug"), `
        (Join-Path $packageRoot "lib/Release"), `
        (Join-Path $packageRoot "bin/Debug"), `
        (Join-Path $packageRoot "bin/Release"), `
        (Join-Path $packageRoot "samples"), `
        (Join-Path $packageRoot "templates"), `
        (Join-Path $packageRoot "docs"), `
        (Join-Path $packageRoot "cmake") | Out-Null

    Copy-RequiredDirectory -Source (Join-Path $repoRoot "sdk/cpp/include/zeno") -Destination (Join-Path $packageRoot "include/zeno")
    Copy-Item -Path (Join-Path $repoRoot "native/zeno_native/include/zeno/*.h") -Destination (Join-Path $packageRoot "include/zeno") -Force

    foreach ($config in $configurations) {
        $preset = if ($config -eq "Release") { "windows-msvc-release" } else { "windows-msvc-debug" }
        $cargoProfileDir = if ($config -eq "Release") { "release" } else { "debug" }
        $buildDir = Join-Path $repoRoot "build/$preset"

        if ($config -eq "Release") {
            cargo build -p zeno_abi --release
        } else {
            cargo build -p zeno_abi
        }
        & $CMakeExe --preset $preset
        & $CMakeExe --build --preset $preset --target zeno_sdk_cpp zeno_native zeno_sample_2d_input_audio_cpp zeno_sample_3d_mesh_cpp

        Copy-RequiredFile -Source (Join-Path $buildDir "lib/$config/zeno_sdk_cpp.lib") -Destination (Join-Path $packageRoot "lib/$config/zeno_sdk_cpp.lib")
        Copy-RequiredFile -Source (Join-Path $buildDir "lib/$config/zeno_native.lib") -Destination (Join-Path $packageRoot "lib/$config/zeno_native.lib")
        Copy-RequiredFile -Source (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll.lib") -Destination (Join-Path $packageRoot "lib/$config/zeno_abi.dll.lib")
        Copy-RequiredFile -Source (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll") -Destination (Join-Path $packageRoot "bin/$config/zeno_abi.dll")
    }

    $packagedFeatureSampleDir = Join-Path $packageRoot "samples/sdk_feature_samples_cpp"
    Copy-RequiredDirectory -Source (Join-Path $repoRoot "samples/sdk_feature_samples_cpp") -Destination $packagedFeatureSampleDir
    Assert-NoAssetFileCollisions `
        -FirstAssetRoot (Join-Path $repoRoot "samples/sample_game_cpp/assets") `
        -SecondAssetRoot (Join-Path $repoRoot "samples/sdk_feature_samples_cpp/assets")
    Copy-DirectoryContents -Source (Join-Path $repoRoot "samples/sample_game_cpp/assets") -Destination (Join-Path $packagedFeatureSampleDir "assets")
    Copy-DirectoryContents -Source (Join-Path $repoRoot "samples/sdk_feature_samples_cpp/assets") -Destination (Join-Path $packagedFeatureSampleDir "assets")
    Assert-RequiredSdkSampleAssets -AssetRoot (Join-Path $packagedFeatureSampleDir "assets")
    Copy-RequiredDirectory -Source (Join-Path $repoRoot "templates/cpp_empty") -Destination (Join-Path $packageRoot "templates/cpp_empty")
    Write-PackagedCMakePresets -Destination (Join-Path $packageRoot "templates/cpp_empty/CMakePresets.json")

    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/getting-started.md") -Destination (Join-Path $packageRoot "docs/getting-started.md")
    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/sdk-layout.md") -Destination (Join-Path $packageRoot "docs/sdk-layout.md")
    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/vs2022.md") -Destination (Join-Path $packageRoot "docs/vs2022.md")
    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/vscode-cmake.md") -Destination (Join-Path $packageRoot "docs/vscode-cmake.md")
    Copy-RequiredDirectory -Source (Join-Path $repoRoot "docs/api") -Destination (Join-Path $packageRoot "docs/api")
    Copy-RequiredDirectory -Source (Join-Path $repoRoot "docs/tutorials") -Destination (Join-Path $packageRoot "docs/tutorials")
    Copy-RequiredFile -Source (Join-Path $repoRoot "cmake/ZenoEngineConfig.cmake") -Destination (Join-Path $packageRoot "cmake/ZenoEngineConfig.cmake")
    Copy-RequiredFile -Source (Join-Path $repoRoot "cmake/ZenoEngineConfigVersion.cmake") -Destination (Join-Path $packageRoot "cmake/ZenoEngineConfigVersion.cmake")

    $zenoCompatibilityConfig = @'
include("${CMAKE_CURRENT_LIST_DIR}/ZenoEngineConfig.cmake")
'@
    Set-Content -LiteralPath (Join-Path $packageRoot "cmake/ZENOConfig.cmake") -Value $zenoCompatibilityConfig -Encoding ascii
    Copy-RequiredFile -Source (Join-Path $repoRoot "cmake/ZenoEngineConfigVersion.cmake") -Destination (Join-Path $packageRoot "cmake/ZENOConfigVersion.cmake")

    $sampleCMake = @'
cmake_minimum_required(VERSION 3.24)

project(zeno_sdk_feature_samples_cpp LANGUAGES CXX)

find_package(ZenoEngine CONFIG REQUIRED)

set(ZENO_FEATURE_SAMPLE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

function(zeno_add_sdk_feature_sample target source)
    add_executable(${target} ${source})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target} PRIVATE ZenoEngine::zeno_sdk_cpp)

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ZenoEngine_BIN_DIR}/$<CONFIG>/zeno_abi.dll"
            "$<TARGET_FILE_DIR:${target}>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ZENO_FEATURE_SAMPLE_ASSET_DIR}"
            "$<TARGET_FILE_DIR:${target}>/assets"
    )
endfunction()

zeno_add_sdk_feature_sample(zeno_sample_2d_input_audio_cpp src/sample_2d_input_audio.cpp)
zeno_add_sdk_feature_sample(zeno_sample_3d_mesh_cpp src/sample_3d_mesh.cpp)
'@
    Set-Content -LiteralPath (Join-Path $packageRoot "samples/sdk_feature_samples_cpp/CMakeLists.txt") -Value $sampleCMake -Encoding ascii
    Write-PackagedCMakePresets -Destination (Join-Path $packageRoot "samples/sdk_feature_samples_cpp/CMakePresets.json")

    $readmeTemplate = @'
# ZENO Engine SDK {{PACKAGE_VERSION}}

This SDK package contains the public C++ headers, Windows MSVC static libraries, Rust ABI runtime DLL, focused SDK samples, a minimal external project template, documentation, and CMake package configuration for ZENO Engine.

Start with `docs/getting-started.md`, then use `docs/tutorials/` for project walkthroughs and `docs/api/` for public SDK concepts.

## Layout

- `include/zeno/` - public SDK and public C ABI headers.
- `lib/Debug/`, `lib/Release/` - MSVC import/static libraries.
- `bin/Debug/`, `bin/Release/` - runtime DLLs required beside executables.
- `samples/sdk_feature_samples_cpp/` - Phase43 focused SDK samples.
- `templates/cpp_empty/` - minimal external CMake project.
- `docs/` - setup, IDE notes, tutorials, and API concept docs.
- `cmake/ZenoEngineConfig.cmake` - imported CMake targets.

## Minimal CMake Use

```cmake
find_package(ZenoEngine CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE ZenoEngine::zeno_sdk_cpp)
```

Configure with:

```powershell
cmake -S . -B build -DZenoEngine_DIR="<sdk-root>/cmake"
cmake --build build --config Debug
```

Packaged `templates/cpp_empty` and `samples/sdk_feature_samples_cpp` also include `windows-msvc-debug` and `windows-msvc-release` CMake presets for CLI, Visual Studio 2022 Open Folder, and VS Code CMake Tools.
'@
    $readme = $readmeTemplate.Replace("{{PACKAGE_VERSION}}", $PackageVersion)
    Set-Content -LiteralPath (Join-Path $packageRoot "README.md") -Value $readme -Encoding ascii

    $privateHeaderHits = Get-ChildItem -LiteralPath (Join-Path $packageRoot "include") -Recurse -File |
        Where-Object { $_.FullName -match "\\src\\" -or $_.Name -eq "native_backend.h" }
    if ($privateHeaderHits) {
        $hitList = ($privateHeaderHits | ForEach-Object { $_.FullName }) -join "`n"
        throw "Private/internal headers were copied into the SDK include directory:`n$hitList"
    }

    if (-not $NoZip) {
        Compress-Archive -Path $packageRoot -DestinationPath $zipPath -Force
        Write-Host "Packaged SDK ZIP to $zipPath"
    }

    Write-Host "Packaged SDK layout to $packageRoot"
}
finally {
    Pop-Location
}
