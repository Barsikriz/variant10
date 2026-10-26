#include "vfs.h"
#include <stdlib.h>
#include <string.h>

const VfsNode *vfs_find(const Vfs *vfs, const wchar_t *path)
{
    for (const VfsNode *node = vfs->nodes; node; node = node->next) {
        if (wcscmp(node->path, path) == 0) { return node; }
    }
    return NULL;
}

int vfs_add(Vfs *vfs, const wchar_t *path, int directory, const void *data, size_t size)
{
    if (vfs_find(vfs, path) || (size && !data) || (directory && size)) { return 0; }
    VfsNode *node = calloc(1, sizeof(*node));
    if (!node) { return 0; }
    size_t path_bytes = (wcslen(path) + 1) * sizeof(wchar_t);
    node->path = malloc(path_bytes);
    node->data = size ? malloc(size) : NULL;
    if (!node->path || (size && !node->data)) {
        free(node->path);
        free(node->data);
        free(node);
        return 0;
    }
    memcpy(node->path, path, path_bytes);
    if (size) { memcpy(node->data, data, size); }
    node->size = size;
    node->is_directory = directory;
    node->next = vfs->nodes;
    vfs->nodes = node;
    ++vfs->count;
    vfs->bytes += size;
    return 1;
}

void vfs_free(Vfs *vfs)
{
    VfsNode *node = vfs->nodes;
    while (node) {
        VfsNode *next = node->next;
        free(node->path);
        free(node->data);
        free(node);
        node = next;
    }
    *vfs = (Vfs){0};
}
