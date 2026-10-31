# CMake build (Ubuntu 24.04)

The legacy Makefiles/configure scripts are intentionally retained during the transition.
The checked-in bison++/flex++ generated C++ sources are compiled directly; CMake
does not regenerate them. Grammar modernisation is a separate task.

Install prerequisites:

```sh
sudo apt install build-essential cmake ninja-build pkg-config tcl-dev tk-dev libx11-dev libxpm-dev
```

Build and test:

```sh
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug
```

Release:

```sh
cmake --preset release
cmake --build --preset release --parallel
```

Sanitizers (GCC/Clang):

```sh
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan
```

Install with a custom prefix:

```sh
cmake -S . -B build/install -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build/install --parallel
cmake --install build/install
```

The `ffgraph3D` symlink is installed alongside `ffgraph`. Runtime assets
are installed under the library directory as in the original Makefiles.
Check application-specific runtime asset lookup on your target installation.
