$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & .\run.ps1 -BuildOnly
    & .\build\emulator.exe | Out-Null
    & .\build\emulator.exe --vfs 'C:\VFS folder' | Out-Null
    & .\build\emulator.exe --script 'tests\fixtures\start.txt' | Out-Null
    & .\build\emulator.exe --vfs 'C:\VFS folder' --script 'tests\fixtures\start with spaces.txt' | Out-Null
} finally { Pop-Location }
