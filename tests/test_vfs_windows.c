#include "vfs.h"
#include "motd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static wchar_t captured[4096];
static void capture_motd(const wchar_t *text)
{
    assert(wcslen(captured) + wcslen(text) < 4096);
    wcscat(captured, text);
}

int main(void)
{
    Vfs vfs = {0};
    wchar_t error[256];
    assert(vfs_load(&vfs, NULL, error, 256));
    assert(vfs.count == 1 && vfs_find(&vfs, L"/")->is_directory);
    assert(vfs_load(&vfs, L"build/vfs/minimal", error, 256));
    assert(vfs.count == 1);
    vfs_show_motd(&vfs, capture_motd);
    assert(!captured[0]);
    assert(vfs_load(&vfs, L"build/vfs/several", error, 256));
    const unsigned char expected[] = {0, 1, 127, 128, 255};
    const VfsNode *binary = vfs_find(&vfs, L"/binary.dat");
    assert(binary && binary->size == sizeof(expected));
    assert(memcmp(binary->data, expected, sizeof(expected)) == 0);
    assert(vfs_find(&vfs, L"/empty.txt")->size == 0);
    vfs_show_motd(&vfs, capture_motd);
    assert(wcsstr(captured, L"Welcome to VFS!\r\n"));
    assert(vfs_load(&vfs, L"build/vfs/deep", error, 256));
    assert(vfs_find(&vfs, L"/level1/level2/level3/data.txt"));
    assert(vfs_find(&vfs, L"/level1/empty")->is_directory);
    size_t count = vfs.count;
    assert(!vfs_load(&vfs, L"build/vfs/not-present", error, 256));
    assert(error[0] && vfs.count == count);
    assert(!vfs_load(&vfs, L"build/vfs/several/motd", error, 256));
    assert(vfs.count == count);
    vfs_free(&vfs);
    const unsigned char invalid[] = {255};
    assert(vfs_add(&vfs, L"/motd", 0, invalid, sizeof(invalid)));
    captured[0] = 0;
    vfs_show_motd(&vfs, capture_motd);
    assert(wcsstr(captured, L"Error"));
    vfs_free(&vfs);
    assert(vfs_add(&vfs, L"/motd", 1, NULL, 0));
    captured[0] = 0;
    vfs_show_motd(&vfs, capture_motd);
    assert(wcsstr(captured, L"directory"));
    vfs_free(&vfs);
    const unsigned char bom[] = {239, 187, 191, 'H', 'i', '\n'};
    assert(vfs_add(&vfs, L"/motd", 0, bom, sizeof(bom)));
    captured[0] = 0;
    vfs_show_motd(&vfs, capture_motd);
    assert(wcscmp(captured, L"Hi\r\n") == 0);
    vfs_free(&vfs);
    puts("Windows VFS and motd tests passed.");
    return 0;
}
