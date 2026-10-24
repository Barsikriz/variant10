#include "config.h"

int config_parse(int argc, wchar_t **argv, Config *config, const wchar_t **error)
{
    config->vfs_path = NULL;
    config->script_path = NULL;
    *error = NULL;
    for (int i = 1; i < argc; ++i) {
        const wchar_t **target;
        if (wcscmp(argv[i], L"--vfs") == 0) {
            target = &config->vfs_path;
        } else if (wcscmp(argv[i], L"--script") == 0) {
            target = &config->script_path;
        } else {
            *error = L"Неизвестный параметр. Используйте --vfs PATH и --script PATH.";
            return 0;
        }
        if (*target) {
            *error = L"Параметр указан повторно.";
            return 0;
        }
        if (++i >= argc || !argv[i][0] || wcsncmp(argv[i], L"--", 2) == 0) {
            *error = L"После параметра требуется непустой путь.";
            return 0;
        }
        *target = argv[i];
    }
    return 1;
}
