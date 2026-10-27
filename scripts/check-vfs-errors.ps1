$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & .\run.ps1 -BuildOnly
    & .\scripts\create-vfs.ps1
    foreach ($name in @('minimal', 'several', 'deep')) {
        & .\build\emulator.exe --vfs "build/vfs/$name" --script 'tests/fixtures/errors.txt' | Out-Null
    }
    & .\build\emulator.exe --vfs 'build/vfs/not-present' | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/several/motd' | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/bad-motd' --script 'tests/fixtures/stage3.txt' | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/deep' --script 'tests/fixtures/exit.txt' | Out-Null
} finally { Pop-Location }
