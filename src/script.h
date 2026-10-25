#ifndef SCRIPT_H
#define SCRIPT_H
#include "shell.h"
typedef void (*ScriptOutput)(const wchar_t *text);
ShellResult script_run_text(wchar_t *text, ScriptOutput output);
ShellResult script_run_file(const wchar_t *path, ScriptOutput output);
#endif
