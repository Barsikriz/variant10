#ifndef SHELL_H
#define SHELL_H
#include <wchar.h>

typedef enum { SHELL_OK, SHELL_EXIT, SHELL_ERROR } ShellResult;
ShellResult shell_execute(const wchar_t *line, wchar_t **output);
#endif
