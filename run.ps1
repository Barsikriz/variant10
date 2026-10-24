param(
    [switch]$Test,
    [switch]$BuildOnly,
    [string]$VfsPath,
    [string]$ScriptPath
)
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    New-Item -ItemType Directory -Force build | Out-Null
    if ($Test) {
        & gcc -std=c11 -Wall -Wextra -Werror -Isrc tests/test_shell.c src/shell.c -o build/test_shell.exe
        if ($LASTEXITCODE -ne 0) { throw 'Test build failed.' }
        & .\build\test_shell.exe
        if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
    } else {
        & gcc -std=c11 -Wall -Wextra -Werror -DUNICODE -D_UNICODE -municode -mwindows `
            src/main.c src/shell.c src/config.c -o build/emulator.exe -luser32 -lgdi32 -ladvapi32 -lshell32
        if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
        if (-not $BuildOnly) {
            $launchArgs = @()
            if ($VfsPath) { $launchArgs += @('--vfs', $VfsPath) }
            if ($ScriptPath) { $launchArgs += @('--script', $ScriptPath) }
            & .\build\emulator.exe @launchArgs | Out-Null
        }
    }
} finally { Pop-Location }
