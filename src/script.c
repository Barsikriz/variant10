#include "script.h"
#include <stdlib.h>

ShellResult script_run_text(wchar_t *text, ScriptOutput output)
{
    size_t number = 0;
    ShellResult overall = SHELL_OK;
    if (*text == 0xfeff) { ++text; }
    while (*text) {
        wchar_t *line = text;
        while (*text && *text != L'\n' && *text != L'\r') { ++text; }
        if (*text) {
            wchar_t separator = *text;
            *text++ = 0;
            if (separator == L'\r' && *text == L'\n') { ++text; }
        }
        ++number;
        output(L"$ ");
        output(line);
        output(L"\r\n");
        wchar_t *answer = NULL;
        ShellResult result = shell_execute(line, &answer);
        output(answer ? answer : L"Ошибка: недостаточно памяти.\r\n");
        free(answer);
        if (result == SHELL_EXIT) { return SHELL_EXIT; }
        if (result == SHELL_ERROR) {
            wchar_t message[128];
            swprintf(message, 128, L"[script] Ошибка в строке %zu; выполнение продолжается.\r\n", number);
            output(message);
            overall = SHELL_ERROR;
        }
    }
    return overall;
}
