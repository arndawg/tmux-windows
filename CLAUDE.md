# Claude Code Project Notes — tmux-windows

## MSYS2 / Git Bash Shell Quirks

### taskkill syntax
MSYS2 converts single-slash flags (`/F`) into file paths. Always use **double slashes**:
```bash
taskkill //F //IM tmux.exe
```
Never use `taskkill /F /IM` — MSYS2 mangles `/F` into `F:/`.

### Before rebuilding tmux.exe
Always kill all tmux processes first, or the linker fails with `LNK1104: cannot open file 'tmux.exe'`:
```bash
taskkill //F //IM tmux.exe 2>/dev/null; sleep 2
```

### MSYS2 path conversion
MSYS2 converts Unix-style paths passed to native Windows executables. This affects:
- Relative paths like `regress/conf/foo.conf` may become `C\:/src/tmux/regress/conf/foo.conf`
- The escaped colon (`C\:`) has a literal backslash before the colon
- Code checking `path[1] == ':'` for Windows drive letters must also check `path[1] == '\\' && path[2] == ':'`

### Test temp variables
Never use `TMP` as a shell variable name in test scripts — it shadows Windows `%TMP%` and causes `GetTempFileName()` ACCESS_DENIED errors in child processes. Use `OUT` instead.

## Build Commands

```bash
# Debug build (from Git Bash)
taskkill //F //IM tmux.exe 2>/dev/null; sleep 2
powershell.exe -NoProfile -Command "& { cmd.exe /c '\"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat\" x64 >nul 2>&1 && cmake --build C:\src\tmux\build --config Debug' }"

# Run tests
TEST_TMUX=./build/Debug/tmux.exe bash regress/win32-basic.sh
```

## Test Suite

CI runs 8 test scripts from `regress/`:
- `win32-basic.sh` — core session/window/pane operations
- `win32-claude-swarm.sh` — multi-session concurrent usage
- `win32-format-strings.sh` — `#{...}` format engine (171 assertions)
- `win32-conf-syntax.sh` — config file parsing (21 files)
- `win32-keys.sh` — key handling (12 tests, tests effects not raw codes)
- `win32-has-session.sh` — exit code validation
- `win32-layout.sh` — pane dimension verification (7 tests)
- `win32-control-client.sh` — complex pane operations (6 tests)

## Architecture Notes

- IPC uses named pipes for discovery + TCP for data (libevent needs sockets)
- Pipe name: `\\.\pipe\tmux-<user>-<label>`
- ConPTY panes run `cmd.exe`, not a Unix shell
- Control mode (`-C`) not supported — libevent `evsig_init_` socketpair fails
- `source -n` hangs as a separate client on Windows (file-read protocol blocks)
