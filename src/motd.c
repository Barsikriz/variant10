#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "motd.h"

void vfs_show_motd(const Vfs *vfs, ScriptOutput output)
{
    const VfsNode *node = vfs_find(vfs, L"/motd");
    if (!node) { return; }
    if (node->is_directory) {
        output(L"[motd] Error: /motd is a directory.\r\n");
        return;
    }
    if (!node->size) { return; }
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        (const char *)node->data, (int)node->size, NULL, 0);
    if (!count || memchr(node->data, 0, node->size)) {
        output(L"[motd] Error: expected UTF-8 text without NUL bytes.\r\n");
        return;
    }
    wchar_t *text = calloc((size_t)count + 1, sizeof(wchar_t));
    wchar_t *display = calloc((size_t)count * 2 + 3, sizeof(wchar_t));
    if (!text || !display) {
        output(L"[motd] Error: insufficient memory.\r\n");
        free(text);
        free(display);
        return;
    }
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char *)node->data,
        (int)node->size, text, count);
    size_t pos = 0;
    for (size_t i = text[0] == 0xfeff ? 1 : 0; text[i]; ++i) {
        if (text[i] == L'\r' || text[i] == L'\n') {
            if (text[i] == L'\r' && text[i + 1] == L'\n') { ++i; }
            display[pos++] = L'\r';
            display[pos++] = L'\n';
        } else { display[pos++] = text[i]; }
    }
    if (pos && display[pos - 1] != L'\n') {
        display[pos++] = L'\r';
        display[pos++] = L'\n';
    }
    output(display);
    free(display);
    free(text);
}
