# Simplified Circuit Visualizer

## Build Configuration

### System Requirements

Development tools:

- `cmake` (3.13+)
- a build tool (`GNU make`, `ninja`, IDE-provided, etc.)
- C++17 compatible compiler (e.g. `GCC` (8+) or `clang` (5+))

Preinstalled libraries:

- `SDL2`[^sdl2]
- `SDL2_ttf` (2.20.1+)[^sdl2ttf]

### General build instructions

```
$ cmake -S . -B build # -G Ninja, etc.
$ cd build
$ make                # ninja, etc.
```

### Linux

If your distribution provides the latest release version of `SDL2_ttf`
library, you may use the system package manager to install it. Otherwise,
get the source disribution from the distribution page[^sdl2ttf], build and
install the library manually. When installing to non-standard location
`<sdl2ttf-dir>`, provide the path to the `cmake` configuration step:

```
$ cmake -S . -B build -DCMAKE_PREFIX_PATH=<sdl2ttf-dir>
```

### Windows

Get the appropirate development releases from the distribution
page[^sdl2win], unpack and copy contents of the archived directories to the
same location `<install-prefix>`. Then provide the path to the `cmake`
configuration step:

```
$ cmake -S . -B build -DCMAKE_PREFIX_PATH=<install-prefix>
```

If an IDE is in use, find the IDE `cmake` configuration options and provide
the `-DCMAKE_PREFIX_PATH=<install-prefix>` option accordingly.

### MacOS

`Homebrew` package manager provides the latest library versions:

```
$ brew install sdl2 sdl2_ttf
```

[^sdl2]: https://libsdl.org

[^sdl2ttf]: https://github.com/libsdl-org/SDL_ttf/releases

[^sdl2win]: https://github.com/libsdl-org/SDL/releases

    Development releases for Windows are prefixed with `SDL2-devel`, but the
    actual release depends on the target development environment. If `mingw`
    is not in use, the package for VC will probably suffice.

    The same applies to `SDL2_ttf` packages[^sdl2ttf].
