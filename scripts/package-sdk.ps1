param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    $preset = if ($Configuration -eq "Release") { "windows-msvc-release" } else { "windows-msvc-debug" }
    $cargoProfileArgs = if ($Configuration -eq "Release") { @("--release") } else { @() }
    $cargoProfileDir = if ($Configuration -eq "Release") { "release" } else { "debug" }
    $buildDir = Join-Path $repoRoot "build/$preset"
    $packageDir = Join-Path $repoRoot "build/package-sdk/$preset"
    $includeDir = Join-Path $packageDir "include"
    $libDir = Join-Path $packageDir "lib"
    $binDir = Join-Path $packageDir "bin"
    $cmakeDir = Join-Path $packageDir "cmake"
    $packageVersion = "0.1.0"

    cargo build -p zeno_abi @cargoProfileArgs
    cmake --preset $preset
    cmake --build --preset $preset --target zeno_sdk_cpp zeno_native

    if (Test-Path $packageDir) {
        Remove-Item -LiteralPath $packageDir -Recurse -Force
    }

    New-Item -ItemType Directory -Force -Path $includeDir, $libDir, $binDir, $cmakeDir | Out-Null

    Copy-Item -Path (Join-Path $repoRoot "sdk/cpp/include/zeno") -Destination $includeDir -Recurse
    Copy-Item -Path (Join-Path $repoRoot "native/zeno_native/include/zeno/*.h") -Destination (Join-Path $includeDir "zeno")

    Copy-Item -LiteralPath (Join-Path $buildDir "lib/$Configuration/zeno_sdk_cpp.lib") -Destination $libDir
    Copy-Item -LiteralPath (Join-Path $buildDir "lib/$Configuration/zeno_native.lib") -Destination $libDir
    Copy-Item -LiteralPath (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll.lib") -Destination $libDir
    Copy-Item -LiteralPath (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll") -Destination $binDir

    $config = @'
get_filename_component(_ZENO_PREFIX "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT TARGET ZENO::zeno_abi_rust)
    add_library(ZENO::zeno_abi_rust SHARED IMPORTED)
    set_target_properties(ZENO::zeno_abi_rust PROPERTIES
        IMPORTED_IMPLIB "${_ZENO_PREFIX}/lib/zeno_abi.dll.lib"
        IMPORTED_LOCATION "${_ZENO_PREFIX}/bin/zeno_abi.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${_ZENO_PREFIX}/include"
    )
endif()

if(NOT TARGET ZENO::zeno_native)
    add_library(ZENO::zeno_native STATIC IMPORTED)
    set_target_properties(ZENO::zeno_native PROPERTIES
        IMPORTED_LOCATION "${_ZENO_PREFIX}/lib/zeno_native.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${_ZENO_PREFIX}/include"
        INTERFACE_LINK_LIBRARIES "ZENO::zeno_abi_rust;user32;d3d11;dxgi;d3dcompiler;windowscodecs;ole32;xaudio2"
    )
endif()

if(NOT TARGET ZENO::zeno_sdk_cpp)
    add_library(ZENO::zeno_sdk_cpp STATIC IMPORTED)
    set_target_properties(ZENO::zeno_sdk_cpp PROPERTIES
        IMPORTED_LOCATION "${_ZENO_PREFIX}/lib/zeno_sdk_cpp.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${_ZENO_PREFIX}/include"
        INTERFACE_LINK_LIBRARIES "ZENO::zeno_native;ZENO::zeno_abi_rust"
    )
endif()

set(ZENO_INCLUDE_DIR "${_ZENO_PREFIX}/include")
set(ZENO_BIN_DIR "${_ZENO_PREFIX}/bin")
set(ZENO_LIB_DIR "${_ZENO_PREFIX}/lib")
set(ZENO_VERSION "0.1.0")
set(ZENO_FOUND TRUE)
'@
    Set-Content -LiteralPath (Join-Path $cmakeDir "ZENOConfig.cmake") -Value $config -Encoding ascii

    $versionConfig = @"
set(PACKAGE_VERSION "$packageVersion")

if(PACKAGE_FIND_VERSION)
    if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
        set(PACKAGE_VERSION_EXACT TRUE)
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
    elseif(PACKAGE_FIND_VERSION VERSION_LESS PACKAGE_VERSION)
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
    else()
        set(PACKAGE_VERSION_COMPATIBLE FALSE)
    endif()
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
"@
    Set-Content -LiteralPath (Join-Path $cmakeDir "ZENOConfigVersion.cmake") -Value $versionConfig -Encoding ascii

    Write-Host "Packaged SDK to $packageDir"
}
finally {
    Pop-Location
}
