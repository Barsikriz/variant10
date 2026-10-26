#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "vfs.h"

enum { MAX_FILE_BYTES = 64 * 1024 * 1024, MAX_TOTAL_BYTES = 256 * 1024 * 1024,
       MAX_NODES = 100000, MAX_DEPTH = 128 };

static wchar_t *join_path(const wchar_t *parent, const wchar_t *name, wchar_t separator)
{
    size_t length = wcslen(parent);
    size_t count = length + wcslen(name) + 2;
    wchar_t *result = calloc(count, sizeof(wchar_t));
    if (!result) { return NULL; }
    wcscpy(result, parent);
    if (length && parent[length - 1] != separator) { result[length++] = separator; }
    wcscpy(result + length, name);
    return result;
}

static int load_file(Vfs *vfs, const wchar_t *disk_path, const wchar_t *virtual_path)
{
    HANDLE file = CreateFileW(disk_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (file == INVALID_HANDLE_VALUE) { return 0; }
    LARGE_INTEGER size;
    if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 || size.QuadPart > MAX_FILE_BYTES
        || vfs->bytes + (size_t)size.QuadPart > MAX_TOTAL_BYTES) {
        CloseHandle(file);
        return 0;
    }
    DWORD length = (DWORD)size.QuadPart;
    unsigned char *data = malloc((size_t)length + 1);
    DWORD received = 0;
    BOOL ok = data && ReadFile(file, data, length, &received, NULL);
    CloseHandle(file);
    int result = ok && received == length && vfs_add(vfs, virtual_path, 0, data, length);
    free(data);
    return result;
}

static int load_directory(Vfs *vfs, const wchar_t *disk, const wchar_t *virtual_path, size_t depth);

static int load_entry(Vfs *vfs, const wchar_t *disk, const wchar_t *virtual_path, size_t depth)
{
    DWORD attributes = GetFileAttributesW(disk);
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
        || depth > MAX_DEPTH || vfs->count >= MAX_NODES) { return 0; }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        return load_directory(vfs, disk, virtual_path, depth);
    }
    return load_file(vfs, disk, virtual_path);
}

static int load_directory(Vfs *vfs, const wchar_t *disk, const wchar_t *virtual_path, size_t depth)
{
    if (!vfs_add(vfs, virtual_path, 1, NULL, 0)) { return 0; }
    wchar_t *pattern = join_path(disk, L"*", L'\\');
    if (!pattern) { return 0; }
    WIN32_FIND_DATAW entry;
    HANDLE search = FindFirstFileW(pattern, &entry);
    DWORD error = GetLastError();
    free(pattern);
    if (search == INVALID_HANDLE_VALUE) { return error == ERROR_FILE_NOT_FOUND; }
    int ok = 1;
    do {
        if (wcscmp(entry.cFileName, L".") == 0 || wcscmp(entry.cFileName, L"..") == 0) { continue; }
        wchar_t *child_disk = join_path(disk, entry.cFileName, L'\\');
        wchar_t *child_virtual = join_path(virtual_path, entry.cFileName, L'/');
        ok = child_disk && child_virtual && load_entry(vfs, child_disk, child_virtual, depth + 1);
        free(child_disk);
        free(child_virtual);
        if (!ok) { break; }
    } while (FindNextFileW(search, &entry));
    if (ok && GetLastError() != ERROR_NO_MORE_FILES) { ok = 0; }
    FindClose(search);
    return ok;
}

int vfs_load(Vfs *vfs, const wchar_t *path, wchar_t *error, size_t capacity)
{
    Vfs loaded = {0};
    int ok;
    if (!path) {
        ok = vfs_add(&loaded, L"/", 1, NULL, 0);
    } else {
        DWORD attributes = GetFileAttributesW(path);
        ok = attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)
            && !(attributes & FILE_ATTRIBUTE_REPARSE_POINT) && load_entry(&loaded, path, L"/", 0);
    }
    if (!ok) {
        vfs_free(&loaded);
        if (capacity) {
            swprintf(error, capacity, L"VFS load failed. Check path, access, links and size limits.");
        }
        return 0;
    }
    vfs_free(vfs);
    *vfs = loaded;
    if (capacity) { error[0] = 0; }
    return 1;
}
