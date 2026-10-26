#ifndef VFS_H
#define VFS_H
#include <stddef.h>
#include <wchar.h>

typedef struct VfsNode {
    wchar_t *path;
    int is_directory;
    unsigned char *data;
    size_t size;
    struct VfsNode *next;
} VfsNode;

typedef struct {
    VfsNode *nodes;
    size_t count;
    size_t bytes;
} Vfs;

int vfs_add(Vfs *vfs, const wchar_t *path, int directory, const void *data, size_t size);
const VfsNode *vfs_find(const Vfs *vfs, const wchar_t *path);
void vfs_free(Vfs *vfs);
int vfs_load(Vfs *vfs, const wchar_t *path, wchar_t *error, size_t capacity);
#endif
