# OS shell emulator - Variant 10

## Overview

A C application with a native Win32 GUI implementing stages 1-3.
The title contains the real Windows user name and computer name.
Enter commands in the input field and press Enter or click the button.
The output area displays commands, results and errors.

## Commands and configuration

| Command | Behavior |
| --- | --- |
| `ls [arguments]` | Prints its name and arguments (stub). |
| `cd [arguments]` | Prints its name and arguments (stub). |
| `exit` | Closes the application; arguments are rejected. |

The command parser splits input on spaces and ignores repeated spaces.
Quotes and environment variables have no special meaning inside commands.
Empty input does nothing. Unknown commands report errors without ending the session.
Actual `ls` and `cd` operations belong to stage 4.

| Launch option | Meaning |
| --- | --- |
| `--vfs PATH` | Directory copied into an in-memory VFS at startup. |
| `--script PATH` | Startup script to execute after VFS initialization. |

Both options are optional. Without `--vfs`, the VFS contains only an empty root directory.
Without `--script`, the application enters interactive mode immediately.
Paths with spaces must be quoted in the Windows command line.
Both settings are printed at startup. Unknown options, duplicate options and missing values are errors.
Relative paths are resolved against the process working directory.

Startup scripts use UTF-8 with an optional BOM and one command per line.
LF, CRLF and CR line endings are accepted. Each command and its output are displayed.
Invalid commands report a line number and are skipped; following commands still execute.
`exit` stops the script and closes the window. After script completion or a read error,
the interactive session remains available. Comments are not supported.
Script files must contain no NUL bytes and must not exceed 16 MiB.

The root file `motd`, when present, is displayed from memory before the startup script.
It must be UTF-8 text, optionally with a BOM, and contain no NUL bytes.
A missing or empty `motd` produces no message. A directory named `motd` or invalid text
produces a nonfatal error; the startup script and interactive session remain available.
`vfs_show_motd` looks up `/motd` in memory, decodes UTF-8 and normalizes line endings for the GUI.

### VFS

The source directory is read once at startup. Directory names, virtual paths and all file bytes
are kept in memory, including empty files, empty directories and binary files.
Virtual paths use `/` and are case-sensitive. The root has the path `/`.
A startup message reports the number of loaded nodes (including root) and file bytes.
No source files are created, overwritten, extracted or deleted by the emulator.
An invalid or unreadable source causes a startup error; a partial VFS is discarded.
Symbolic links and other Windows reparse points are rejected.
Limits: 64 MiB per file, 256 MiB total file content, 100000 nodes and 128 directory-entry levels.
These limits and allocation failures cause a load error.

## Functions

| Function | Purpose |
| --- | --- |
| `copy_text`, `message` | Allocate command response strings and return a status. |
| `next_token`, `shell_execute` | Parse and execute a command. The caller frees the response. |
| `config_parse`, `show_config` | Validate launch options and print settings. |
| `load_script` | Read a UTF-8 script through the Windows API. |
| `script_run_file`, `script_run_text` | Execute scripts and report line errors. |
| `make_title`, `show_error` | Build the window title and display error dialogs. |
| `append_output`, `submit_command` | Display output and process interactive input. |
| `input_proc`, `window_proc` | Handle keyboard input and window messages. |
| `create_controls`, `resize_controls` | Create and arrange GUI controls. |
| `wWinMain` | Initialize the application, load VFS, execute a script and run the message loop. |
| `vfs_add` | Copy a directory entry or file bytes into the VFS. |
| `vfs_find` | Look up an exact virtual path in memory. |
| `vfs_free` | Release all VFS memory. |
| `join_path` | Join physical or virtual path components. |
| `load_file`, `load_directory`, `load_entry` | Read source files and recursively traverse directories. |
| `vfs_load` | Load a complete VFS; preserve the previous VFS if loading fails. |
| `check_command`, `check_config`, `check_script` | Test commands, configuration and script execution. |
| `capture` | Collect script output for assertions. |
| Test `main` functions | Run test suites. |

The VFS must be zero-initialized before its first use. `vfs_find` returns a borrowed pointer
valid until the VFS is freed or replaced. Command output can be NULL on allocation failure.
The GUI input limit is 65535 UTF-16 code units.

## Build and tests

Requires Windows and MinGW-w64 with `gcc` available in PATH.
Run the following commands in PowerShell from the project root:

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1 -BuildOnly
powershell -ExecutionPolicy Bypass -File .\run.ps1 -Test
.\build\emulator.exe --vfs "C:\My VFS" --script "tests\fixtures\start.txt"
```

`run.ps1` builds and launches the application. `-BuildOnly` builds without launching;
`-Test` runs tests. `-VfsPath` and `-ScriptPath` pass the corresponding launch options.
The helper runs from the project root, so relative paths passed to it are project-relative.
Build outputs are stored in `build` and ignored by Git.

MSVC users can run `run.bat` or `run.bat test` from the x64 Native Tools Command Prompt
with the C++ build tools and Windows SDK installed.

## Usage examples

Enter `ls`, `ls -l /home`, `cd /tmp`, `unknown`, `exit now`, then `ls`.
The errors must not prevent the final command from running. Enter `exit` to close the window.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\check-parameters.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check-errors.ps1
```

These demonstration scripts open windows sequentially. Close each window to continue.
The demonstration scripts generate their own VFS directories under `build/vfs`.

### Stage 3 demonstration

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\check-vfs.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check-vfs-errors.ps1
```

Both scripts exercise an empty directory, a directory containing several files (including binary data),
and a tree with three nested directory levels. The error script also checks a missing source,
a file used as a directory, invalid `motd` text and `exit` inside a startup script.
`create-vfs.ps1` creates test data under `build/vfs`; it is a fixture generator, separate from the emulator.
`stage3.txt` demonstrates all current commands, arguments, blank input and error recovery.
`exit.txt` tests termination. Since `cd` is still a stub, `cd /missing` only echoes its arguments.

To start one case manually:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\create-vfs.ps1
.\build\emulator.exe --vfs "build/vfs/deep" --script "tests/fixtures/stage3.txt"
```

The startup order is settings, VFS statistics, root `motd`, then script commands.
After the script, enter `exit` to close the window.

The test suite covers commands, configuration, scripts, memory ownership, binary content,
empty files and directories, nested loading, failed-load rollback and `motd` decoding.
`capture_motd` collects messages for the Windows-specific assertions.
Windows loader and `motd` tests require Windows; memory tests use standard C only.
