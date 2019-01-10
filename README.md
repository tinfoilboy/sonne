# computare

A simple and configurable for counting lines in files as an alternative to programs like cloc written in C++. I was becoming bothered with how cloc handled ignoring files, as I'll integrate some libraries directly in my source folder as opposed to an external folder, thus I wanted to mainly ignore specific files. I also wanted something faster and more extensible, so I made this.

## Building

Computare uses CMake for building. Build using your favorite compiler chain and run the executable that results from the build.

## Dependencies

Computare depends on [cxxopts](https://github.com/jarro2783/cxxopts) for parsing command line arguments for the program as well as [fmt](https://github.com/fmtlib/fmt) for nice print formatting and [yaml-cpp](https://github.com/jbeder/yaml-cpp) for the config.

## Usage

To use computare, you can run the program with a single file using the `-f` switch and then the path to the file, an example is below:

    computare -f main.cpp

Though, the main function of this program is the directory walker for a project's source code, this can be specified with the `-d` flag with the path to the directory to walk recursively. An example is below to walk the current directory:

    computare -d .

You may use the `-h` or `--help` flags to see the full list of switches for the program.

## Configuration

Computare is meant to be configured to change languages supported or files to ignore. These options are configured with a file named `.computare.yml`. These are the supported options. A default configuration is placed into your home directory at "~/.computare.yml" with language definitions and default settings.

### `block-size`

Changes the size of blocks to be read from a file when counting lines.
Default:

    block-size: 131072

### `ignore-hidden`

Specifies whether or not hidden files and directories should be ignored.
Default:

    ignore-hidden: true

### `languages`

An array of language blocks containing information on language name, associated extensions, and comment tokens.
Check the root `.computare.yml` to see how to structure these blocks and add your own.

### `ignore`

An array of files and/or directories to ignore and skip over when counting lines.
Default is blank.

## License

Computare is licensed under the MIT License, the terms of which can be seen [here](https://github.com/tinfoilboy/computare/blob/master/LICENSE).
