$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & .\run.ps1 -BuildOnly
    & .\scripts\create-vfs.ps1
    & .\build\emulator.exe | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/several' | Out-Null
    & .\build\emulator.exe --script 'tests\fixtures\start.txt' | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/several' --script 'tests\fixtures\start with spaces.txt' | Out-Null
} finally { Pop-Location }
