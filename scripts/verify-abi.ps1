Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$headers = @(
    Join-Path $repoRoot "native/zeno_native/include/zeno/zeno_abi.h"
    Join-Path $repoRoot "native/zeno_native/include/zeno/zeno_native_backend.h"
    Join-Path $repoRoot "sdk/cpp/include/zeno/game_module_abi.h"
)

$pattern = "std::string|std::vector|std::filesystem|template|throw|exception|class |HRESULT|ComPtr|IXAudio|XAUDIO|WAVEFORMAT|HWND|ID3D|IWIC|DXGI|D3D11|std::"
$matches = Select-String -Path $headers -Pattern $pattern -CaseSensitive
if ($matches) {
    $matches | ForEach-Object {
        Write-Error "$($_.Path):$($_.LineNumber): forbidden ABI surface text: $($_.Line.Trim())"
    }
    exit 1
}

Write-Host "ABI forbidden-type scan passed."
