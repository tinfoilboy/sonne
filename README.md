# computare

A fast and simple program for counting lines in files as an alternative to
programs like cloc written in C++.
Designed to be more extensible respecting files like `.gitignore` or a custom
`.computareignore` file.

## Building

Computare uses CMake for building.

## Dependencies

Computare depends on [cxxopts](https://github.com/jarro2783/cxxopts) for parsing
command line arguments for the program.

## License

Computare is licensed under the MIT License, the terms of which can be seen
[here](https://github.com/tinfoilboy/computare/blob/master/LICENSE).