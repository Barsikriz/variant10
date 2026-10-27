$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & .\run.ps1 -BuildOnly
    & .\scripts\create-vfs.ps1
    foreach ($name in @('minimal', 'several', 'deep')) {
        & .\build\emulator.exe --vfs "build/vfs/$name" --script 'tests/fixtures/stage3.txt' | Out-Null
    }
} finally { Pop-Location }
