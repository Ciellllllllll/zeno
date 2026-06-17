param(
    [ValidateSet("Debug", "Release", "All")]
    [string]$Configuration = "All",
    [string]$PackageVersion = "0.1.0-dev",
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
        cmake --preset $preset
        cmake --build --preset $preset --target zeno_sdk_cpp zeno_native zeno_sample_2d_input_audio_cpp zeno_sample_3d_mesh_cpp

        Copy-RequiredFile -Source (Join-Path $buildDir "lib/$config/zeno_sdk_cpp.lib") -Destination (Join-Path $packageRoot "lib/$config/zeno_sdk_cpp.lib")
        Copy-RequiredFile -Source (Join-Path $buildDir "lib/$config/zeno_native.lib") -Destination (Join-Path $packageRoot "lib/$config/zeno_native.lib")
        Copy-RequiredFile -Source (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll.lib") -Destination (Join-Path $packageRoot "lib/$config/zeno_abi.dll.lib")
        Copy-RequiredFile -Source (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll") -Destination (Join-Path $packageRoot "bin/$config/zeno_abi.dll")
    }

    $packagedFeatureSampleDir = Join-Path $packageRoot "samples/sdk_feature_samples_cpp"
    Copy-RequiredDirectory -Source (Join-Path $repoRoot "samples/sdk_feature_samples_cpp") -Destination $packagedFeatureSampleDir
    Copy-DirectoryContents -Source (Join-Path $repoRoot "samples/sample_game_cpp/assets") -Destination (Join-Path $packagedFeatureSampleDir "assets")
    Copy-DirectoryContents -Source (Join-Path $repoRoot "samples/sdk_feature_samples_cpp/assets") -Destination (Join-Path $packagedFeatureSampleDir "assets")
    Copy-RequiredDirectory -Source (Join-Path $repoRoot "templates/cpp_empty") -Destination (Join-Path $packageRoot "templates/cpp_empty")

    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/getting-started.md") -Destination (Join-Path $packageRoot "docs/getting-started.md")
    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/sdk-layout.md") -Destination (Join-Path $packageRoot "docs/sdk-layout.md")
    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/vs2022.md") -Destination (Join-Path $packageRoot "docs/vs2022.md")
    Copy-RequiredFile -Source (Join-Path $repoRoot "docs/vscode-cmake.md") -Destination (Join-Path $packageRoot "docs/vscode-cmake.md")
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

    $readmeTemplate = @'
# ZENO Engine SDK {{PACKAGE_VERSION}}

This SDK package contains the public C++ headers, Windows MSVC static libraries, Rust ABI runtime DLL, focused SDK samples, a minimal external project template, documentation, and CMake package configuration for ZENO Engine.

Start with `docs/getting-started.md`.

## Layout

- `include/zeno/` - public SDK and public C ABI headers.
- `lib/Debug/`, `lib/Release/` - MSVC import/static libraries.
- `bin/Debug/`, `bin/Release/` - runtime DLLs required beside executables.
- `samples/sdk_feature_samples_cpp/` - Phase43 focused SDK samples.
- `templates/cpp_empty/` - minimal external CMake project.
- `docs/` - setup and IDE notes.
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
