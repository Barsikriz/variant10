$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & .\run.ps1 -BuildOnly
    & .\scripts\create-vfs.ps1
    & .\build\emulator.exe --vfs 'build/vfs/several' --script 'tests\fixtures\errors.txt' | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/several' --script 'tests\fixtures\missing.txt' | Out-Null
    & .\build\emulator.exe --vfs 'build/vfs/several' --script 'tests\fixtures\exit.txt' | Out-Null
    & .\build\emulator.exe --vfs | Out-Null
    & .\build\emulator.exe --script | Out-Null
    & .\build\emulator.exe --vfs a --vfs b | Out-Null
    & .\build\emulator.exe --unknown x | Out-Null
} finally { Pop-Location }
