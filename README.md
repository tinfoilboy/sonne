# sonne
### (formerly known as computare)

[![Github Releases](https://img.shields.io/github/release/tinfoilboy/sonne.svg)](https://github.com/tinfoilboy/sonne/releases)

sonne is a simple and fast lines of code counter that is very configurable and easy to use. It is able to count plain text lines, as well as code lines including comments using configurable language definitions. These language definitions can be added to any config at will, allowing for custom code counting at any scope.

## Installing

Download the latest binary release in the [releases](https://github.com/tinfoilboy/sonne/releases) page.

## Building

First, clone the repository using `--recurse-submodules` to get all of the submoduled dependencies
Then create a `build` or `out` folder and run CMake in it.
Then compile and run!

## Dependencies

sonne depends on [fmt](https://github.com/fmtlib/fmt) for formatting strings, [json](https://github.com/nlohmann/json) for parsing configs, [Catch2](https://github.com/catchorg/Catch2) for testing, and [cxxopts](https://github.com/jarro2783/cxxopts) for parsing commands.

## Usage

To use sonne, just provide the path to the file or folder you wish to run it on.

    sonne <path>

## Configuration

sonne is meant to be configured to change languages supported or files to ignore. These options are configured with a file named `.sonne.yml`. These are the supported options. A default configuration is placed into your home directory at `~/.sonne.yml` with language definitions and default settings.

### `ignore-hidden`

Specifies whether or not hidden files and directories should be ignored.
Default is true.

### `languages`

An array of language blocks containing information on language name, associated extensions, and comment tokens.
Check the root `.sonne.json` to see how to structure these blocks and add your own.

### `ignore`

An array of files and/or directories to ignore and skip over when counting lines.
Default is blank.

## License

sonne is licensed under the MIT License, the terms of which can be seen [here](https://github.com/tinfoilboy/sonne/blob/master/LICENSE).
