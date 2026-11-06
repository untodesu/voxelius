$ErrorActionPreference = "Stop"
$script = Join-Path $PSScriptRoot "scripts/setup_environment.py"
if (Get-Command py -ErrorAction SilentlyContinue) {
    & py -3 $script clangd @args
} else {
    & python $script clangd @args
}
exit $LASTEXITCODE
