# canvil

## A C implementation of amboso, a simple build tool wrapping make.

## Table of Contents

+ [What is this thing?](#witt)
+ [Supported amboso features](#supported_amboso)
+ [Dependencies](#deps)
+ [Installing dependencies](#prep)
+ [Configuration](#config)
+ [Building](#building)
+ [Installing](#installing)
+ [Todo](#todo)

## What is this thing? <a name = "witt"></a>

  This is a C port of [amboso](https://github.com/jgabaut/amboso), a basic build tool wrapping make and supporting git tags.

## Supported amboso features <a name = "supported_amboso"></a>

  The current build is mostly compliant with `amboso` `2.1`.

Flags support status:

  - [x] Basic env flags:  `-D`, `-K`, `-M`, `-S`, `-E`, `-O`
  - [ ] Clock flag: `-Y <startTime>`
  - [x] Linter mode: `-x`
    - [x] Lint only: `-l`
    - [x] Report lex: `-L`
  - [x] C header gen mode: `-G`
  - [x] Verbose flag: `-V`
  - [x] Test macro: `-t`
  - [x] Test mode: `-T`
  - [x] Git mode: `-g`
  - [x] Base mode: `-B`
  - [x] Build: `-b`
  - [x] Run: `-r`
  - [x] Init: `-i`
  - [x] Delete: `-d`
  - [x] Purge: `-p`
  - [x] Help: `-h`
  - [ ] Big Help: `-H`
  - [x] Version: `-v`
  - [x] List tags for current mode: `-l`
  - [x] List tags for git/base mode: `-L`
  - [x] Quiet flag: `-q`
  - [ ] Watch flag: `-w`
  - [x] Warranty flag: `-W`
  - [x] Ignore gitcheck flag: `-X`
  - [x] Silent: `-s`
  - [x] Pass config argument: `-C`
  - [ ] Run make pack: `-z`
  - [x] No rebuild: `-R`
  - [x] Logged run: `-J`
  - [x] No color: `-P`
  - [x] Force build: `-F`
  - [x] Turn off extensions: `-e` (Only relative to 2.0.0)
  - [x] Run make when no arguments are provided

## Dependencies <a name = "deps"></a>

- `tomlc17` for parsing TOML
- `git2` for the commit info header generation
- `libarchive` for `anvilPy` kern, to extract `.tar.gz` files
- `koliseo` for arena allocator
- `spuro` for logging
- `komando` for running commands and test mode
- `dumbtimer` for the timer functionality

## Installing dependencies <a name = "prep"></a>

You can avoid depending on `git2.h` by using the `-DCANVIL_NOGIT2` macro. See [this section](#config) for more info.

To install `git2.h` and `archive.h`, needed to build:

- On Ubuntu:
```sh
  sudo apt install libgit2-dev libarchive-dev
```

Make sure to initialise the submodules:

```sh
  git submodule update --init
```

After doing that, you may need to generate the Makefile for koliseo:

```sh
  cd koliseo || { printf "Failed cd\n"; exit 1; };
  aclocal; autoconf; automake --add-missing; ./configure
  cd -
```

## Configuration <a name = "config"></a>

  To prepare the files needed by `autotools`, run:

  ```sh
  aclocal
  autoconf
  automake --add-missing
  ./configure # Optionally, with --enable-nogit or --host
  ```

  You will get a `./configure` script, which you can use to enable debug mode or other features.

  - Run `./configure --host x86-64-w64-mingw32` to setup the `Makefile` appropriately for a `x86_64-w64-mingw32` build.
  - Run `./configure --enable-nogit` to setup the `Makefile` appropriately and build with `-DCANVIL_NOGIT2` flag.

## Building <a name = "building"></a>

To build:
```sh
  make
```

## Installing <a name = "installing"></a>

To install/unintall:
```sh
  sudo make install
  sudo make uninstall
```
