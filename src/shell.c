#include "shell.h"
#include <stdlib.h>
#include <string.h>

static wchar_t *copy_text(const wchar_t *text)
{
    size_t size = (wcslen(text) + 1) * sizeof(wchar_t);
    wchar_t *copy = malloc(size);
    if (copy) {
        memcpy(copy, text, size);
    }
    return copy;
}

static ShellResult message(wchar_t **output, const wchar_t *text, ShellResult result)
{
    *output = copy_text(text);
    return *output ? result : SHELL_ERROR;
}

static wchar_t *next_token(wchar_t **cursor)
{
    wchar_t *start = *cursor;
    while (*start == L' ') {
        ++start;
    }
    if (!*start) {
        *cursor = start;
        return NULL;
    }
    wchar_t *end = start;
    while (*end && *end != L' ') {
        ++end;
    }
    if (*end) {
        *end++ = L'\0';
    }
    *cursor = end;
    return start;
}

ShellResult shell_execute(const wchar_t *line, wchar_t **output)
{
    *output = NULL;
    wchar_t *copy = copy_text(line);
    if (!copy) {
        return SHELL_ERROR;
    }
    wchar_t *cursor = copy;
    wchar_t *command = next_token(&cursor);
    if (!command) {
        free(copy);
        return message(output, L"", SHELL_OK);
    }
    ShellResult result;
    if (wcscmp(command, L"exit") == 0) {
        result = next_token(&cursor)
            ? message(output, L"Ошибка: exit не принимает аргументы.\r\n", SHELL_ERROR)
            : message(output, L"", SHELL_EXIT);
    } else if (wcscmp(command, L"ls") == 0 || wcscmp(command, L"cd") == 0) {
        *output = calloc(wcslen(line) + 3, sizeof(wchar_t));
        if (!*output) {
            free(copy);
            return SHELL_ERROR;
        }
        wcscpy(*output, command);
        wchar_t *argument;
        while ((argument = next_token(&cursor)) != NULL) {
            wcscat(*output, L" ");
            wcscat(*output, argument);
        }
        wcscat(*output, L"\r\n");
        result = SHELL_OK;
    } else {
        const wchar_t *prefix = L"Ошибка: неизвестная команда: ";
        *output = calloc(wcslen(prefix) + wcslen(command) + 3, sizeof(wchar_t));
        if (*output) {
            wcscpy(*output, prefix);
            wcscat(*output, command);
            wcscat(*output, L"\r\n");
        }
        result = SHELL_ERROR;
    }
    free(copy);
    return result;
}
