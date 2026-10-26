#include "config.h"
#include "script.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static wchar_t log_text[8192];
static void capture(const wchar_t *text)
{
    assert(wcslen(log_text) + wcslen(text) < 8192);
    wcscat(log_text, text);
}

static void check_config(void)
{
    Config config;
    const wchar_t *error;
    wchar_t *valid[] = {L"app", L"--script", L"мой скрипт.txt", L"--vfs", L"C:/my data"};
    assert(config_parse(5, valid, &config, &error));
    assert(!error && wcscmp(config.script_path, valid[2]) == 0);
    assert(wcscmp(config.vfs_path, valid[4]) == 0);
    assert(config_parse(1, valid, &config, &error));
    assert(!config.vfs_path && !config.script_path);
    wchar_t *missing[] = {L"app", L"--vfs"};
    assert(!config_parse(2, missing, &config, &error) && error);
    wchar_t *unknown[] = {L"app", L"--bad", L"x"};
    assert(!config_parse(3, unknown, &config, &error));
    wchar_t *duplicate[] = {L"app", L"--vfs", L"a", L"--vfs", L"b"};
    assert(!config_parse(5, duplicate, &config, &error));
    wchar_t *empty[] = {L"app", L"--script", L""};
    assert(!config_parse(3, empty, &config, &error));
    wchar_t *option[] = {L"app", L"--vfs", L"--script", L"a"};
    assert(!config_parse(4, option, &config, &error));
}

static void check_script(void)
{
    wchar_t text[] = L"\xfeffls\r\nunknown\ncd Документы\rexit now\nls -l";
    assert(script_run_text(text, capture) == SHELL_ERROR);
    assert(wcsstr(log_text, L"$ ls\r\nls\r\n"));
    assert(wcsstr(log_text, L"Ошибка в строке 2"));
    assert(wcsstr(log_text, L"Ошибка в строке 4"));
    assert(wcsstr(log_text, L"cd Документы\r\n"));
    assert(wcsstr(log_text, L"ls -l\r\n"));
    log_text[0] = 0;
    wchar_t stop[] = L"exit\nunknown";
    assert(script_run_text(stop, capture) == SHELL_EXIT);
    assert(!wcsstr(log_text, L"unknown"));
    log_text[0] = 0;
    wchar_t blank[] = L"\n   \r\n";
    assert(script_run_text(blank, capture) == SHELL_OK);
    wchar_t empty[] = L"";
    assert(script_run_text(empty, capture) == SHELL_OK);
}

int main(void)
{
    check_config();
    check_script();
    puts("Config and script tests passed.");
    return 0;
}
