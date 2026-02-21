# tmux-windows Release Pipeline

## Stage 1: GitHub Actions CI (build + test on every push)

- [ ] Create `.github/workflows/ci.yml`
- [ ] Install libevent via vcpkg on `windows-latest` runner
- [ ] Install WinFlexBison via vcpkg (or choco)
- [ ] Build Debug with CMake + MSVC
- [ ] Run `regress/win32-basic.sh` via Git Bash
- [ ] Run `regress/win32-claude-swarm.sh` via Git Bash
- [ ] Cache vcpkg packages for faster CI runs

## Stage 2: GitHub Release on tag push

- [ ] Add release job to CI workflow (triggered on `v*` tags)
- [ ] Build Release configuration
- [ ] Zip `tmux.exe` + `event_core.dll` into `tmux-windows-<version>.zip`
- [ ] Generate SHA256 hash of the zip
- [ ] Create GitHub Release with zip + hash as assets
- [ ] Include brief install instructions in release notes

## Stage 3: Winget manifest (after Stage 2 is stable)

- [ ] Create winget manifest YAML (`arndawg.tmux-windows`)
- [ ] Use `portable` installer type (same as fzf, ripgrep)
- [ ] Point `InstallerUrl` at GitHub Release asset
- [ ] Include SHA256 hash
- [ ] Set `Commands: [tmux]` so winget creates the symlink
- [ ] Submit PR to `microsoft/winget-pkgs`
- [ ] Wait for Microsoft review (1-3 days typical)
- [ ] Verify `winget install arndawg.tmux-windows` works

## Target User Experience

```powershell
# Future goal:
winget install arndawg.tmux-windows

# Immediate goal (Stage 2):
# Download tmux-windows-v3.5a.zip from GitHub Releases
# Extract to a folder in %PATH%
```
