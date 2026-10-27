@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
if /I "%~1"=="test" goto test
if not "%~1"=="" (
    echo Usage: run.bat [test]
    exit /b 1
)
cl /nologo /std:c11 /utf-8 /W4 /WX /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /Isrc ^
    src\main.c src\shell.c src\config.c src\script.c src\script_file.c ^
    src\vfs.c src\vfs_windows.c src\motd.c /Fe:build\emulator.exe /Fo:build\ ^
    /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib advapi32.lib shell32.lib
if errorlevel 1 exit /b 1
start "" /wait build\emulator.exe
exit /b %errorlevel%
:test
cl /nologo /std:c11 /utf-8 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /Isrc ^
    tests\test_shell.c src\shell.c /Fe:build\test_shell.exe /Fo:build\
if errorlevel 1 exit /b 1
build\test_shell.exe
if errorlevel 1 exit /b 1
cl /nologo /std:c11 /utf-8 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /Isrc ^
    tests\test_config_script.c src\config.c src\script.c src\shell.c ^
    /Fe:build\test_config_script.exe /Fo:build\
if errorlevel 1 exit /b 1
build\test_config_script.exe
exit /b %errorlevel%
