#include "shell.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void check_command(const wchar_t *line, ShellResult expected, const wchar_t *text)
{
    wchar_t *output = NULL;
    assert(shell_execute(line, &output) == expected);
    assert(output != NULL);
    assert(wcscmp(output, text) == 0);
    free(output);
}

int main(void)
{
    check_command(L"ls", SHELL_OK, L"ls\r\n");
    check_command(L"cd /tmp", SHELL_OK, L"cd /tmp\r\n");
    check_command(L"exit", SHELL_EXIT, L"");
    check_command(L"", SHELL_OK, L"");
    check_command(L"     ", SHELL_OK, L"");
    check_command(L"  ls   -l  /home  ", SHELL_OK, L"ls -l /home\r\n");
    check_command(L"cd", SHELL_OK, L"cd\r\n");
    check_command(L"ls Документы", SHELL_OK, L"ls Документы\r\n");
    check_command(L"ls \"a b\"", SHELL_OK, L"ls \"a b\"\r\n");
    check_command(L"ls $HOME", SHELL_OK, L"ls $HOME\r\n");
    check_command(L"unknown", SHELL_ERROR, L"Ошибка: неизвестная команда: unknown\r\n");
    check_command(L"exit now", SHELL_ERROR, L"Ошибка: exit не принимает аргументы.\r\n");
    check_command(L"ls", SHELL_OK, L"ls\r\n");
    check_command(L"  exit  ", SHELL_EXIT, L"");
    wchar_t *long_line = calloc(100004, sizeof(wchar_t));
    assert(long_line != NULL);
    wmemcpy(long_line, L"ls ", 3);
    wmemset(long_line + 3, L'a', 100000);
    wchar_t *output = NULL;
    assert(shell_execute(long_line, &output) == SHELL_OK);
    assert(output != NULL && wcslen(output) == 100005);
    assert(wcsncmp(output, long_line, 100003) == 0);
    assert(wcscmp(output + 100003, L"\r\n") == 0);
    free(output);
    free(long_line);
    puts("All tests passed.");
    return 0;
}
