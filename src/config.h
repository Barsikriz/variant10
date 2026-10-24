#ifndef CONFIG_H
#define CONFIG_H
#include <wchar.h>
typedef struct {
    const wchar_t *vfs_path;
    const wchar_t *script_path;
} Config;
int config_parse(int argc, wchar_t **argv, Config *config, const wchar_t **error);
#endif
