#!/usr/bin/env bash
# Collect an ffgraph build environment inventory without changing packages or building.
# Run from the repository root (or pass a repository path as argument).
set -eu

root="${1:-.}"
root="$(cd "$root" && pwd)"
out="$root/build/dependency-report"
mkdir -p "$out"

{
  printf '%s\n' '# ffgraph local build environment' "capture_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)" "source_root=$root"
  printf '\n## OS\n'
  if [ -r /etc/os-release ]; then grep -E '^(NAME|VERSION_ID|PRETTY_NAME)=' /etc/os-release; fi
  uname -srmo || true
  printf '\n## Git\n'
  if command -v git >/dev/null 2>&1 && git -C "$root" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf 'commit='; git -C "$root" rev-parse HEAD
    printf 'branch='; git -C "$root" branch --show-current
    printf 'working_tree_status:\n'; git -C "$root" status --short
  else
    echo 'No readable Git checkout'
  fi
  printf '\n## Tool versions\n'
  for tool in cmake ninja gcc g++ clang pkg-config make git valgrind Xvfb; do
    if command -v "$tool" >/dev/null 2>&1; then
      printf '\n%s: ' "$tool"
      case "$tool" in
        gcc|g++|clang) "$tool" --version | head -n 1 ;;
        pkg-config) "$tool" --version ;;
        *) "$tool" --version 2>&1 | head -n 1 ;;
      esac
    else
      printf '%s: not installed\n' "$tool"
    fi
  done
  printf '\n## pkg-config libraries\n'
  if command -v pkg-config >/dev/null 2>&1; then
    for mod in tcl tk x11 xpm; do
      if pkg-config --exists "$mod"; then
        printf '%s version=' "$mod"; pkg-config --modversion "$mod"
        printf '%s flags=' "$mod"; pkg-config --cflags --libs "$mod"
      else
        printf '%s: no pkg-config module (may be found through CMake another way)\n' "$mod"
      fi
    done
  fi
} > "$out/environment.txt" 2>&1

if command -v dpkg-query >/dev/null 2>&1; then
  dpkg-query -W -f='${Package}\t${Version}\t${Status}\n' \
    build-essential cmake ninja-build pkg-config gcc g++ \
    tcl-dev tk-dev tcl8.6-dev tk8.6-dev \
    libx11-dev libxpm-dev libx11-6 libxpm4 \
    xvfb xauth valgrind gdb bison flex \
    > "$out/apt-package-versions.txt" 2>&1 || true
fi

for name in debug release asan; do
  cache="$root/build/$name/CMakeCache.txt"
  if [ -f "$cache" ]; then
    grep -E '^(CMAKE_(BUILD_TYPE|CXX_COMPILER|C_COMPILER|GENERATOR|INSTALL_PREFIX)|FFGRAPH_SANITIZERS|TK_|X11_|XPM_|PKG_CONFIG_EXECUTABLE)' "$cache" \
      > "$out/cmake-$name-dependencies.txt" || true
  fi
done

for binary in "$root/build/debug/ffgraph" "$root/build/release/ffgraph"; do
  if [ -f "$binary" ] && command -v ldd >/dev/null 2>&1; then
    name="$(basename "$(dirname "$binary")")"
    ldd "$binary" > "$out/ldd-$name.txt" 2>&1 || true
  fi
done

if [ -f "$root/CMakePresets.json" ]; then
  cp "$root/CMakePresets.json" "$out/CMakePresets.json"
fi

printf 'Dependency snapshot written to: %s\n' "$out"
printf 'Review the contents (paths, host/platform details, Git status) before sharing.\n'
