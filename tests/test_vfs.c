#include "vfs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    Vfs vfs = {0};
    unsigned char bytes[] = {0, 255, 128, 42};
    assert(vfs_add(&vfs, L"/", 1, NULL, 0));
    assert(vfs_add(&vfs, L"/empty", 1, NULL, 0));
    assert(vfs_add(&vfs, L"/empty/file", 0, NULL, 0));
    assert(vfs_add(&vfs, L"/binary", 0, bytes, sizeof(bytes)));
    assert(vfs.count == 4 && vfs.bytes == 4);
    assert(!vfs_add(&vfs, L"/binary", 0, bytes, sizeof(bytes)));
    const VfsNode *node = vfs_find(&vfs, L"/binary");
    assert(node && !node->is_directory && node->size == sizeof(bytes));
    assert(memcmp(node->data, bytes, sizeof(bytes)) == 0);
    bytes[0] = 12;
    assert(node->data[0] == 0);
    assert(!vfs_find(&vfs, L"/Binary"));
    assert(vfs_find(&vfs, L"/empty")->is_directory);
    assert(vfs_find(&vfs, L"/empty/file")->size == 0);
    vfs_free(&vfs);
    assert(!vfs.nodes && !vfs.count && !vfs.bytes);
    vfs_free(&vfs);
    puts("VFS memory tests passed.");
    return 0;
}
