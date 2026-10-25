#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "script.h"

enum { MAX_SCRIPT_BYTES = 16 * 1024 * 1024 };

static wchar_t *load_script(const wchar_t *path)
{
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) { return NULL; }
    LARGE_INTEGER size;
    if (!GetFileSizeEx(file, &size) || size.QuadPart > MAX_SCRIPT_BYTES || size.QuadPart < 0) {
        CloseHandle(file);
        return NULL;
    }
    DWORD length = (DWORD)size.QuadPart;
    char *bytes = malloc((size_t)length + 1);
    DWORD received = 0;
    BOOL ok = bytes && ReadFile(file, bytes, length, &received, NULL);
    CloseHandle(file);
    if (!ok || received != length) { free(bytes); return NULL; }
    bytes[length] = 0;
    for (DWORD i = 0; i < length; ++i) {
        if (bytes[i] == 0) { free(bytes); return NULL; }
    }
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, -1, NULL, 0);
    wchar_t *text = count ? calloc((size_t)count, sizeof(wchar_t)) : NULL;
    if (text && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, -1, text, count)) {
        free(text);
        text = NULL;
    }
    free(bytes);
    return text;
}

ShellResult script_run_file(const wchar_t *path, ScriptOutput output)
{
    wchar_t *text = load_script(path);
    if (!text) {
        output(L"[script] Ошибка чтения скрипта: ");
        output(path);
        output(L"\r\nПроверьте доступность, UTF-8, отсутствие NUL и размер до 16 МиБ.\r\n");
        return SHELL_ERROR;
    }
    ShellResult result = script_run_text(text, output);
    free(text);
    return result;
}
