$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & .\run.ps1 -BuildOnly
    & .\build\emulator.exe --vfs 'C:\VFS folder' --script 'tests\fixtures\errors.txt' | Out-Null
    & .\build\emulator.exe --vfs 'C:\VFS folder' --script 'tests\fixtures\missing.txt' | Out-Null
    & .\build\emulator.exe --vfs 'C:\VFS folder' --script 'tests\fixtures\exit.txt' | Out-Null
    & .\build\emulator.exe --vfs | Out-Null
    & .\build\emulator.exe --script | Out-Null
    & .\build\emulator.exe --vfs a --vfs b | Out-Null
    & .\build\emulator.exe --unknown x | Out-Null
} finally { Pop-Location }
