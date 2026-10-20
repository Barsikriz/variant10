@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
cl /nologo /std:c11 /utf-8 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /Isrc tests\test_shell.c src\shell.c /Fe:build\test_shell.exe /Fo:build\
if errorlevel 1 exit /b 1
build\test_shell.exe
exit /b %errorlevel%
