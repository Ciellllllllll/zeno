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

function Is-SafeRelativeAssetPath {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [bool]$AllowDot = $false
    )

    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $false
    }
    if ($AllowDot -and $Path -eq ".") {
        return $true
    }
    if ($Path -eq "." -or $Path -eq ".." -or [System.IO.Path]::IsPathRooted($Path) -or $Path.Contains(":")) {
        return $false
    }

    $parts = $Path -split "[/\\]"
    foreach ($part in $parts) {
        if ([string]::IsNullOrWhiteSpace($part) -or $part -eq "." -or $part -eq "..") {
            return $false
        }
    }

    return $true
}

function Read-ZenoKeyValueFile {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Header
    )

    Assert-FileNonEmpty -Path $Path -Description "$Header file"
    $entries = @{}
    $sawHeader = $false
    foreach ($rawLine in Get-Content -LiteralPath $Path) {
        $line = $rawLine.Trim()
        if ($line.Length -eq 0 -or $line.StartsWith("#")) {
            continue
        }
        if (-not $sawHeader) {
            if ($line -ne $Header) {
                throw "Expected $Header header in $Path"
            }
            $sawHeader = $true
            continue
        }

        $separator = $line.IndexOf("=")
        if ($separator -le 0) {
            continue
        }
        $key = $line.Substring(0, $separator).Trim()
        $value = $line.Substring($separator + 1).Trim()
        if ($key.Length -gt 0 -and $value.Length -gt 0) {
            $entries[$key] = $value
        }
    }

    if (-not $sawHeader) {
        throw "Missing $Header header in $Path"
    }
    return $entries
}

function Assert-ZenoProjectAsset {
    param(
        [Parameter(Mandatory = $true)][string]$AssetRoot,
        [Parameter(Mandatory = $true)][string]$RelativePath
    )

    $projectPath = Join-Path $AssetRoot $RelativePath
    $project = Read-ZenoKeyValueFile -Path $projectPath -Header "zeno_project"
    if ($project["version"] -ne "1") {
        throw "Unsupported SDK sample project version in ${RelativePath}: $($project["version"])"
    }
    if (-not (Is-SafeRelativeAssetPath -Path $project["asset_root"] -AllowDot $true)) {
        throw "Unsafe asset_root in ${RelativePath}: $($project["asset_root"])"
    }
    if (-not (Is-SafeRelativeAssetPath -Path $project["initial_scene"])) {
        throw "Unsafe initial_scene in ${RelativePath}: $($project["initial_scene"])"
    }
    if ([int]$project["window_width"] -le 0 -or [int]$project["window_height"] -le 0) {
        throw "Invalid window dimensions in $RelativePath"
    }

    Assert-ZenoSceneAsset -AssetRoot $AssetRoot -RelativePath $project["initial_scene"]
}

function Assert-ZenoSceneAsset {
    param(
        [Parameter(Mandatory = $true)][string]$AssetRoot,
        [Parameter(Mandatory = $true)][string]$RelativePath
    )

    if (-not (Is-SafeRelativeAssetPath -Path $RelativePath)) {
        throw "Unsafe SDK sample scene path: $RelativePath"
    }

    $scenePath = Join-Path $AssetRoot $RelativePath
    Assert-FileNonEmpty -Path $scenePath -Description "SDK sample scene $RelativePath"
    $text = Get-Content -LiteralPath $scenePath -Raw
    $logicalLines = @(Get-Content -LiteralPath $scenePath | ForEach-Object { $_.Trim() } | Where-Object { $_.Length -gt 0 -and -not $_.StartsWith("#") })
    if ($logicalLines.Count -lt 2 -or $logicalLines[0] -ne "zeno_scene" -or $logicalLines[1] -ne "version=1") {
        throw "Invalid SDK sample scene header/version: $RelativePath"
    }

    $references = @([regex]::Matches($text, "(?m)^reference=(.+)$") | ForEach-Object { $_.Groups[1].Value.Trim() })
    if ($references.Count -eq 0) {
        throw "SDK sample scene has no object references: $RelativePath"
    }
    foreach ($reference in $references) {
        if ($reference -in @("builtin:cube", "builtin:triangle")) {
            continue
        }
        if (-not (Is-SafeRelativeAssetPath -Path $reference)) {
            throw "Unsafe SDK sample scene reference in ${RelativePath}: $reference"
        }
        Assert-FileNonEmpty -Path (Join-Path $AssetRoot $reference) -Description "SDK sample scene reference $reference"
    }
}

function Assert-BmpFixture {
    param([Parameter(Mandatory = $true)][string]$Path)

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 54 -or $bytes[0] -ne 0x42 -or $bytes[1] -ne 0x4d) {
        throw "SDK sample BMP is not a BMP file: $Path"
    }
    $declaredSize = [System.BitConverter]::ToUInt32($bytes, 2)
    $pixelOffset = [System.BitConverter]::ToUInt32($bytes, 10)
    $width = [System.BitConverter]::ToInt32($bytes, 18)
    $height = [System.BitConverter]::ToInt32($bytes, 22)
    $planes = [System.BitConverter]::ToUInt16($bytes, 26)
    $bitsPerPixel = [System.BitConverter]::ToUInt16($bytes, 28)
    $compression = [System.BitConverter]::ToUInt32($bytes, 30)
    if ($declaredSize -le 0 -or $pixelOffset -ge $bytes.Length -or $width -ne 2 -or [Math]::Abs($height) -ne 2 -or $planes -ne 1 -or $bitsPerPixel -ne 24 -or $compression -ne 0) {
        throw "SDK sample BMP fixture mismatch: $Path"
    }
}

function Assert-WavFixture {
    param([Parameter(Mandatory = $true)][string]$Path)

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 44) {
        throw "SDK sample WAV is too small: $Path"
    }
    $ascii = [System.Text.Encoding]::ASCII
    if ($ascii.GetString($bytes, 0, 4) -ne "RIFF" -or $ascii.GetString($bytes, 8, 4) -ne "WAVE") {
        throw "SDK sample WAV is not a RIFF/WAVE file: $Path"
    }

    $offset = 12
    $fmtOk = $false
    $dataOk = $false
    while ($offset + 8 -le $bytes.Length) {
        $chunkId = $ascii.GetString($bytes, $offset, 4)
        $chunkSize = [System.BitConverter]::ToUInt32($bytes, $offset + 4)
        $chunkData = $offset + 8
        if ($chunkData + $chunkSize -gt $bytes.Length) {
            throw "SDK sample WAV chunk exceeds file length: $Path"
        }
        if ($chunkId -eq "fmt " -and $chunkSize -ge 16) {
            $format = [System.BitConverter]::ToUInt16($bytes, $chunkData)
            $channels = [System.BitConverter]::ToUInt16($bytes, $chunkData + 2)
            $bitsPerSample = [System.BitConverter]::ToUInt16($bytes, $chunkData + 14)
            $fmtOk = $format -eq 1 -and $channels -eq 1 -and $bitsPerSample -eq 16
        } elseif ($chunkId -eq "data") {
            $dataOk = $chunkSize -gt 0
        }
        $offset = $chunkData + $chunkSize + ($chunkSize % 2)
    }

    if (-not $fmtOk -or -not $dataOk) {
        throw "SDK sample WAV fixture mismatch: $Path"
    }
}

function Assert-SdkSampleAssets {
    param([Parameter(Mandatory = $true)][string]$AssetRoot)

    foreach ($relativePath in Get-ExpectedSdkSampleAssetPaths) {
        Assert-FileNonEmpty -Path (Join-Path $AssetRoot $relativePath) -Description "SDK sample asset $relativePath"
    }

    Assert-ZenoProjectAsset -AssetRoot $AssetRoot -RelativePath "project.zproj"
    Assert-ZenoProjectAsset -AssetRoot $AssetRoot -RelativePath "projects/2d_input_audio.zproj"
    Assert-ZenoProjectAsset -AssetRoot $AssetRoot -RelativePath "projects/3d_mesh.zproj"
    Assert-ZenoSceneAsset -AssetRoot $AssetRoot -RelativePath "scenes/sample_scene.zscene"
    Assert-BmpFixture -Path (Join-Path $AssetRoot "textures/sample_sprite_2x2.bmp")
    Assert-WavFixture -Path (Join-Path $AssetRoot "audio/sample_click.wav")

    $shader = Get-Content -LiteralPath (Join-Path $AssetRoot "shaders/sample_triangle.hlsl") -Raw
    if ($shader -notmatch "\bvs_main\b" -or $shader -notmatch "\bps_main\b") {
        throw "SDK sample shader fixture is missing required entry points"
    }

    $manifest = Read-ZenoKeyValueFile -Path (Join-Path $AssetRoot "sample_manifest.txt") -Header "ZENO sample asset manifest"
    if ($manifest["audio"] -ne "audio/sample_click.wav") {
        throw "SDK sample manifest audio entry mismatch"
    }
    Assert-FileNonEmpty -Path (Join-Path $AssetRoot $manifest["audio"]) -Description "SDK sample manifest audio target"
}

function Assert-FilesEqual {
    param(
        [Parameter(Mandatory = $true)][string]$Expected,
        [Parameter(Mandatory = $true)][string]$Actual,
        [Parameter(Mandatory = $true)][string]$Description
    )

    Assert-FileNonEmpty -Path $Expected -Description "$Description expected file"
    Assert-FileNonEmpty -Path $Actual -Description "$Description actual file"
    $expectedHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Expected).Hash
    $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Actual).Hash
    if ($expectedHash -ne $actualHash) {
        throw "$Description content mismatch: $Expected -> $Actual"
    }
}

function Assert-SdkSampleAssetCopyIntegrity {
    param(
        [Parameter(Mandatory = $true)][string]$ExpectedAssetRoot,
        [Parameter(Mandatory = $true)][string]$ActualAssetRoot
    )

    foreach ($relativePath in Get-ExpectedSdkSampleAssetPaths) {
        Assert-FilesEqual `
            -Expected (Join-Path $ExpectedAssetRoot $relativePath) `
            -Actual (Join-Path $ActualAssetRoot $relativePath) `
            -Description "SDK sample asset $relativePath"
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
    $samplesAssetDir = Join-Path $samplesDir "assets"
    $sourceMergedAssetRoot = Join-Path $packageRoot "samples/sdk_feature_samples_cpp/assets"

    Assert-SdkSampleAssets -AssetRoot $samplesAssetDir
    Assert-SdkSampleAssetCopyIntegrity -ExpectedAssetRoot $sourceMergedAssetRoot -ActualAssetRoot $samplesAssetDir

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
        $runtimeSampleAssetDir = Join-Path $samplesBuildDir "$config/assets"
        Assert-PathExists -Path $runtimeSampleAssetDir -Description "$config sample assets directory"
        Assert-SdkSampleAssets -AssetRoot $runtimeSampleAssetDir
        Assert-SdkSampleAssetCopyIntegrity -ExpectedAssetRoot $samplesAssetDir -ActualAssetRoot $runtimeSampleAssetDir

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
