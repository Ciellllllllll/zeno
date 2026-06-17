param(
    [string]$PackageVersion = "0.1.0-dev",
    [string]$ValidationDir = "build/sdk-package-consumption-qa",
    [string]$CMakeExe = "cmake"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)

    Write-Host ""
    Write-Host "==> $Message"
}

function Assert-PathExists {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Missing ${Description}: $Path"
    }
}

function Assert-FileNonEmpty {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Description
    )

    Assert-PathExists -Path $Path -Description $Description
    $item = Get-Item -LiteralPath $Path
    if ($item.Length -le 0) {
        throw "${Description} is empty: $Path"
    }
}

function Assert-PublicHeaderSet {
    param([Parameter(Mandatory = $true)][string]$IncludeRoot)

    $expected = @(
        "game_module_abi.h",
        "game_module.hpp",
        "math.hpp",
        "zeno.hpp",
        "zeno_abi.h",
        "zeno_native_backend.h"
    ) | Sort-Object
    $actual = Get-ChildItem -LiteralPath (Join-Path $IncludeRoot "zeno") -File |
        ForEach-Object { $_.Name } |
        Sort-Object

    $missing = $expected | Where-Object { $_ -notin $actual }
    $unexpected = $actual | Where-Object { $_ -notin $expected }
    if ($missing -or $unexpected) {
        throw "Packaged public header set mismatch. Missing: $($missing -join ', '); unexpected: $($unexpected -join ', ')"
    }
}

function Assert-NoPrivateHeaders {
    param([Parameter(Mandatory = $true)][string]$IncludeRoot)

    $privateHeaderHits = Get-ChildItem -LiteralPath $IncludeRoot -Recurse -File |
        Where-Object {
            $_.FullName -match "\\src\\" -or
            $_.Name -eq "native_backend.h" -or
            $_.Name -eq "sample_module.h" -or
            $_.Name -eq "template_module.h" -or
            $_.Name -like "*_smoke.*" -or
            $_.FullName -match "\\testdata\\" -or
            $_.FullName -match "\\.codex\\"
        }

    if ($privateHeaderHits) {
        $hitList = ($privateHeaderHits | ForEach-Object { $_.FullName }) -join "`n"
        throw "Private/internal headers were found in packaged include/:`n$hitList"
    }
}

function Assert-NoForbiddenPackageEntries {
    param([Parameter(Mandatory = $true)][string]$SdkRoot)

    $forbiddenEntries = Get-ChildItem -LiteralPath $SdkRoot -Recurse -Force |
        Where-Object {
            $_.Name -in @(".git", ".codex", "target", "CMakeCache.txt", "CMakeFiles", ".vs", ".vscode") -or
            $_.Extension -in @(".obj", ".pdb", ".ilk", ".exe")
        }

    if ($forbiddenEntries) {
        $hitList = ($forbiddenEntries | ForEach-Object { $_.FullName }) -join "`n"
        throw "Forbidden generated/local entries were found in extracted SDK:`n$hitList"
    }
}

function Assert-NoLocalPathLeaks {
    param(
        [Parameter(Mandatory = $true)][string]$SdkRoot,
        [Parameter(Mandatory = $true)][string]$RepoRoot
    )

    $escapedRepoRoot = [regex]::Escape($RepoRoot)
    $textFiles = Get-ChildItem -LiteralPath $SdkRoot -Recurse -File |
        Where-Object { $_.Extension -in @(".md", ".txt", ".cmake", ".json", ".ps1", ".cpp", ".hpp", ".h") }
    $hits = $textFiles | Select-String -Pattern "D:\\", "D:/", "\\Git\\zeno", $escapedRepoRoot -ErrorAction Stop
    if ($hits) {
        $hitList = ($hits | ForEach-Object { "$($_.Path):$($_.LineNumber): $($_.Line)" }) -join "`n"
        throw "Local absolute path leakage was found in extracted SDK text files:`n$hitList"
    }
}

function Invoke-CMake {
    param([Parameter(Mandatory = $true)][string[]]$Arguments)

    & $CMakeExe @Arguments
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$buildRoot = [System.IO.Path]::GetFullPath((Join-Path $repoRoot "build"))
$validationRoot = if ([System.IO.Path]::IsPathRooted($ValidationDir)) {
    [System.IO.Path]::GetFullPath($ValidationDir)
} else {
    [System.IO.Path]::GetFullPath((Join-Path $repoRoot $ValidationDir))
}

$buildRootPrefix = $buildRoot + [System.IO.Path]::DirectorySeparatorChar
if ($validationRoot -eq $buildRoot -or -not $validationRoot.StartsWith($buildRootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "ValidationDir must stay under the ignored build/ directory: $ValidationDir"
}

$packageName = "ZenoEngine-SDK-v$PackageVersion"
$packageRoot = Join-Path $repoRoot "build/package-sdk/$packageName"
$zipPath = Join-Path $repoRoot "build/package-sdk/$packageName.zip"
$extractedSdkRoot = Join-Path $validationRoot $packageName

Push-Location $repoRoot
try {
    Write-Step "Generate SDK package and ZIP"
    & (Join-Path $PSScriptRoot "package-sdk.ps1") -Configuration All -PackageVersion $PackageVersion -CMakeExe $CMakeExe
    Assert-PathExists -Path $packageRoot -Description "SDK package layout"
    Assert-PathExists -Path $zipPath -Description "SDK package ZIP"

    Write-Step "Extract ZIP into clean validation directory"
    if (Test-Path -LiteralPath $validationRoot) {
        Remove-Item -LiteralPath $validationRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $validationRoot | Out-Null
    Expand-Archive -LiteralPath $zipPath -DestinationPath $validationRoot -Force
    $topLevelEntries = @(Get-ChildItem -LiteralPath $validationRoot -Force)
    if ($topLevelEntries.Count -ne 1 -or $topLevelEntries[0].Name -ne $packageName) {
        $entryList = ($topLevelEntries | ForEach-Object { $_.Name }) -join ", "
        throw "Expected ZIP to extract exactly one top-level $packageName directory, found: $entryList"
    }
    Assert-PathExists -Path $extractedSdkRoot -Description "extracted SDK root"

    Write-Step "Verify extracted SDK package artifacts"
    foreach ($config in @("Debug", "Release")) {
        Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "lib/$config/zeno_sdk_cpp.lib") -Description "$config zeno_sdk_cpp.lib"
        Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "lib/$config/zeno_native.lib") -Description "$config zeno_native.lib"
        Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "lib/$config/zeno_abi.dll.lib") -Description "$config zeno_abi.dll.lib"
        Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "bin/$config/zeno_abi.dll") -Description "$config zeno_abi.dll"
    }
    Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "cmake/ZenoEngineConfig.cmake") -Description "ZenoEngineConfig.cmake"
    Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "cmake/ZenoEngineConfigVersion.cmake") -Description "ZenoEngineConfigVersion.cmake"
    Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "cmake/ZENOConfig.cmake") -Description "ZENOConfig.cmake"
    Assert-FileNonEmpty -Path (Join-Path $extractedSdkRoot "cmake/ZENOConfigVersion.cmake") -Description "ZENOConfigVersion.cmake"
    Assert-PathExists -Path (Join-Path $extractedSdkRoot "templates/cpp_empty/CMakePresets.json") -Description "template CMakePresets.json"
    Assert-PathExists -Path (Join-Path $extractedSdkRoot "samples/sdk_feature_samples_cpp/CMakePresets.json") -Description "sample CMakePresets.json"
    Assert-PublicHeaderSet -IncludeRoot (Join-Path $extractedSdkRoot "include")
    Assert-NoPrivateHeaders -IncludeRoot (Join-Path $extractedSdkRoot "include")
    Assert-NoForbiddenPackageEntries -SdkRoot $extractedSdkRoot
    Assert-NoLocalPathLeaks -SdkRoot $extractedSdkRoot -RepoRoot $repoRoot

    $templateDir = Join-Path $extractedSdkRoot "templates/cpp_empty"
    $samplesDir = Join-Path $extractedSdkRoot "samples/sdk_feature_samples_cpp"

    foreach ($config in @("Debug", "Release")) {
        $preset = if ($config -eq "Release") { "windows-msvc-release" } else { "windows-msvc-debug" }

        Write-Step "Build and run packaged template ($config)"
        Invoke-CMake -Arguments @("--preset", $preset, "-S", $templateDir)
        $templateBuildDir = Join-Path $templateDir "build/$preset"
        Invoke-CMake -Arguments @("--build", $templateBuildDir, "--config", $config)
        $templateExe = Join-Path $templateBuildDir "$config/zeno_cpp_empty.exe"
        Assert-PathExists -Path $templateExe -Description "$config template executable"
        Assert-PathExists -Path (Join-Path $templateBuildDir "$config/zeno_abi.dll") -Description "$config template runtime DLL"
        & $templateExe

        Write-Step "Build packaged SDK feature samples ($config)"
        Invoke-CMake -Arguments @("--preset", $preset, "-S", $samplesDir)
        $samplesBuildDir = Join-Path $samplesDir "build/$preset"
        Invoke-CMake -Arguments @("--build", $samplesBuildDir, "--config", $config)
        foreach ($sampleTarget in @("zeno_sample_2d_input_audio_cpp", "zeno_sample_3d_mesh_cpp")) {
            Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/$sampleTarget.exe") -Description "$config $sampleTarget executable"
        }
        Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/zeno_abi.dll") -Description "$config sample runtime DLL"
        Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/assets") -Description "$config sample assets directory"
        Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/assets/projects/2d_input_audio.zproj") -Description "$config 2D sample project asset"
        Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/assets/projects/3d_mesh.zproj") -Description "$config 3D sample project asset"
        Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/assets/audio/sample_click.wav") -Description "$config sample audio asset"
        Assert-PathExists -Path (Join-Path $samplesBuildDir "$config/assets/textures/sample_sprite_2x2.bmp") -Description "$config sample texture asset"

        Write-Step "Build and run external game through extracted SDK ($config)"
        $externalBuildDir = Join-Path $validationRoot "external-game-$config"
        if (Test-Path -LiteralPath $externalBuildDir) {
            Remove-Item -LiteralPath $externalBuildDir -Recurse -Force
        }
        Invoke-CMake -Arguments @("-S", "examples/external-game", "-B", $externalBuildDir, "-DZenoEngine_DIR=$(Join-Path $extractedSdkRoot "cmake")")
        Invoke-CMake -Arguments @("--build", $externalBuildDir, "--config", $config)
        $externalExe = Join-Path $externalBuildDir "$config/zeno_external_game.exe"
        Assert-PathExists -Path $externalExe -Description "$config external game executable"
        Assert-PathExists -Path (Join-Path $externalBuildDir "$config/zeno_abi.dll") -Description "$config external game runtime DLL"
        & $externalExe
    }

    Write-Step "SDK package consumption QA passed"
}
finally {
    Pop-Location
}
