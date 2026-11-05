# Ama standard library

This directory contains normal Ama modules and their C implementations.
The compiler has no table of their names, signatures, or arithmetic operators.
The existing `sys/*` module paths remain the public interface.

```text
stdlib/
    Makefile
    modules/
        sys/
            math/
                add
                add.c
                sub
                sub.c
                ...
            io/
                print
                print.c
```

Each extensionless file declares parameters and results and selects `@C`.
Its sibling `.c` implements the generated `ama_native_*` function. Values are
`size_t`; results are written through the generated `size_t*` arguments. See
[the contributor guide](../CONTRIBUTORS.md#standard-library-contributions).

## Build and link

From the repository root:

```sh
make
mkdir -p generated
./ama project main generated/program.c
gcc -std=c11 -O3 generated/program.c build/stdlib/libama_std.a -o generated/program
```

Add `-Igenerated` and the corresponding companion sources when the project
also declares its own C modules. Link the archive after those sources/objects.
`make standard-library` rebuilds the library; `make -C stdlib` also builds it
and bootstraps the compiler if it is missing.

The archive and objects live under `build/stdlib/`. Generated headers are under
`build/stdlib/include/`. The build invokes `--generate-c-headers` with implicit
library loading disabled. It never creates or edits an implementation source.
A missing C implementation is a build error. `C_ARCHIVER` selects the archiver
(default `ar`); library objects do not require compiler LTO support.

## Published behavior

- `sys/math/add`, `sub`, and `mul` use unsigned arithmetic with wraparound.
- `sys/math/div` and `mod` set the numeric result to 0 and `error` to 1 for a
  zero divisor; otherwise `error` is 0.
- `sys/math/lt`, `le`, `gt`, `ge`, `eq`, and `ne` perform unsigned comparisons.
- `sys/math/and`, `or`, and `not` return exactly 0 or 1.
- `sys/io/print` writes an unsigned decimal value followed by a newline and
  currently returns `error = 0`.

The interface files define argument order, names, and defaults. Adding a module
uses the same native mechanism available to project code. Add new build
categories to `MODULE_DIRECTORIES` in this directory's Makefile.

## Library selection

The default module directory is embedded when building the compiler with
`STANDARD_LIBRARY_DIRECTORY` (the repository's `stdlib/modules` by default).
Keep this source tree available, or set `AMA_STDLIB` after moving the compiler:

```sh
AMA_STDLIB=/path/to/modules ./ama project main generated/program.c
AMA_STDLIB= ./ama project main generated/without-library.c
```

An empty value disables library loading; a nonexistent directory is an error.
Use a separate output directory and link the implementations matching the
selected interfaces when using another library. Duplicate module paths across
project and library are rejected. The compiler never creates C stubs inside an
imported library; intentional library stub creation is a separate operation:

```sh
AMA_STDLIB= ./ama --generate-c-stubs stdlib/modules build/stdlib/include
```
