Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$paths = @(
    "build",
    "target"
)

foreach ($path in $paths) {
    $fullPath = Join-Path $repoRoot $path
    if (Test-Path $fullPath) {
        Remove-Item -LiteralPath $fullPath -Recurse -Force
    }
}
