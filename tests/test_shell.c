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
    puts("All tests passed.");
    return 0;
}
