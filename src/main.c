#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include "config.h"
#include "script.h"
#include "vfs.h"
#include "motd.h"
#include <stdlib.h>
#include <wchar.h>
#include "shell.h"

enum { INPUT_ID = 101, RUN_ID = 102, MARGIN = 10, INPUT_HEIGHT = 28 };
static Vfs virtual_fs;
static HWND output_window;
static HWND input_window;
static HWND run_button;
static WNDPROC original_input_proc;

static void show_error(HWND owner, const wchar_t *text)
{
    MessageBoxW(owner, text, L"Ошибка", MB_OK | MB_ICONERROR);
}

static BOOL make_title(wchar_t *title, size_t capacity)
{
    wchar_t username[257];
    wchar_t hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD user_size = (DWORD)(sizeof(username) / sizeof(username[0]));
    DWORD host_size = (DWORD)(sizeof(hostname) / sizeof(hostname[0]));
    if (!GetUserNameW(username, &user_size) || !GetComputerNameW(hostname, &host_size)) {
        return FALSE;
    }
    return swprintf(title, capacity, L"Эмулятор - [%ls@%ls]", username, hostname) >= 0;
}

static void append_output(const wchar_t *text)
{
    int length = GetWindowTextLengthW(output_window);
    SendMessageW(output_window, EM_SETSEL, (WPARAM)length, (LPARAM)length);
    SendMessageW(output_window, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageW(output_window, EM_SCROLLCARET, 0, 0);
}

static void submit_command(HWND window)
{
    int length = GetWindowTextLengthW(input_window);
    wchar_t *line = calloc((size_t)length + 1, sizeof(wchar_t));
    if (!line) {
        show_error(window, L"Недостаточно памяти.");
        return;
    }
    GetWindowTextW(input_window, line, length + 1);
    append_output(L"$ ");
    append_output(line);
    append_output(L"\r\n");
    wchar_t *output = NULL;
    ShellResult result = shell_execute(line, &output);
    append_output(output ? output : L"Ошибка: недостаточно памяти.\r\n");
    free(output);
    free(line);
    SetWindowTextW(input_window, L"");
    SetFocus(input_window);
    if (result == SHELL_EXIT) {
        DestroyWindow(window);
    }
}

static LRESULT CALLBACK input_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_KEYDOWN && wparam == VK_RETURN) {
        SendMessageW(GetParent(window), WM_COMMAND, MAKEWPARAM(RUN_ID, BN_CLICKED), 0);
        return 0;
    }
    if (message == WM_CHAR && wparam == L'\r') {
        return 0;
    }
    return CallWindowProcW(original_input_proc, window, message, wparam, lparam);
}

static BOOL create_controls(HWND window)
{
    HINSTANCE instance = GetModuleHandleW(NULL);
    output_window = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        0, 0, 0, 0, window, NULL, instance, NULL);
    input_window = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        0, 0, 0, 0, window, (HMENU)(INT_PTR)INPUT_ID, instance, NULL);
    run_button = CreateWindowExW(0, L"BUTTON", L"Выполнить",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        0, 0, 0, 0, window, (HMENU)(INT_PTR)RUN_ID, instance, NULL);
    if (!output_window || !input_window || !run_button) {
        return FALSE;
    }
    SendMessageW(output_window, EM_SETLIMITTEXT, 0x7FFFFFFE, 0);
    SendMessageW(input_window, EM_SETLIMITTEXT, 65535, 0);
    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(output_window, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(input_window, WM_SETFONT, (WPARAM)font, TRUE);
    SendMessageW(run_button, WM_SETFONT, (WPARAM)font, TRUE);
    SetLastError(0);
    original_input_proc = (WNDPROC)SetWindowLongPtrW(input_window, GWLP_WNDPROC, (LONG_PTR)input_proc);
    return original_input_proc != NULL;
}

static void resize_controls(HWND window)
{
    RECT rect;
    GetClientRect(window, &rect);
    int width = rect.right;
    int height = rect.bottom;
    int button_width = 110;
    int input_y = height - MARGIN - INPUT_HEIGHT;
    MoveWindow(output_window, MARGIN, MARGIN, width - 2 * MARGIN, input_y - 2 * MARGIN, TRUE);
    MoveWindow(input_window, MARGIN, input_y, width - button_width - 3 * MARGIN, INPUT_HEIGHT, TRUE);
    MoveWindow(run_button, width - button_width - MARGIN, input_y, button_width, INPUT_HEIGHT, TRUE);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message) {
    case WM_CREATE:
        if (!create_controls(window)) {
            show_error(window, L"Не удалось создать элементы окна.");
            return -1;
        }
        return 0;
    case WM_SIZE:
        if (wparam != SIZE_MINIMIZED) {
            resize_controls(window);
        }
        return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lparam)->ptMinTrackSize.x = 420;
        ((MINMAXINFO *)lparam)->ptMinTrackSize.y = 240;
        return 0;
    case WM_SETFOCUS:
        SetFocus(input_window);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wparam) == RUN_ID && HIWORD(wparam) == BN_CLICKED) {
            submit_command(window);
        }
        return 0;
    case WM_DESTROY:
        vfs_free(&virtual_fs);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wparam, lparam);
    }
}

static void show_config(const Config *config)
{
    append_output(L"[config] vfs = ");
    append_output(config->vfs_path ? config->vfs_path : L"(не задан)");
    append_output(L"\r\n[config] script = ");
    append_output(config->script_path ? config->script_path : L"(не задан)");
    append_output(L"\r\n");
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR command_line, int show)
{
    (void)previous;
    (void)command_line;
    wchar_t title[512];
    if (!make_title(title, sizeof(title) / sizeof(title[0]))) {
        show_error(NULL, L"Не удалось получить имя пользователя или компьютера.");
        return EXIT_FAILURE;
    }
    WNDCLASSW window_class = {0};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    window_class.lpszClassName = L"Variant10Shell";
    if (!RegisterClassW(&window_class)) {
        show_error(NULL, L"Не удалось зарегистрировать класс окна.");
        return EXIT_FAILURE;
    }
    HWND window = CreateWindowExW(0, window_class.lpszClassName, title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 500, NULL, NULL, instance, NULL);
    if (!window) {
        show_error(NULL, L"Не удалось создать окно.");
        return EXIT_FAILURE;
    }
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    Config config;
    const wchar_t *error = NULL;
    if (!argv || !config_parse(argc, argv, &config, &error)) {
        show_error(window, error ? error : L"Не удалось прочитать параметры запуска.");
        LocalFree(argv);
        DestroyWindow(window);
        return EXIT_FAILURE;
    }
    show_config(&config);
    wchar_t vfs_error[256];
    if (!vfs_load(&virtual_fs, config.vfs_path, vfs_error, 256)) {
        show_error(window, vfs_error);
        LocalFree(argv);
        DestroyWindow(window);
        return EXIT_FAILURE;
    }
    wchar_t summary[128];
    swprintf(summary, 128, L"[vfs] Loaded %zu nodes, %zu bytes into memory.\r\n",
        virtual_fs.count, virtual_fs.bytes);
    append_output(summary);
    vfs_show_motd(&virtual_fs, append_output);
    ShowWindow(window, show);
    UpdateWindow(window);
    ShellResult script_result = config.script_path
        ? script_run_file(config.script_path, append_output) : SHELL_OK;
    LocalFree(argv);
    if (script_result == SHELL_EXIT) {
        DestroyWindow(window);
        return EXIT_SUCCESS;
    }
    MSG message;
    int status;
    while ((status = (int)GetMessageW(&message, NULL, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    if (status == -1) {
        show_error(window, L"Ошибка получения сообщения Windows.");
        return EXIT_FAILURE;
    }
    return (int)message.wParam;
}
