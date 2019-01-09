# computare

A simple and configurable for counting lines in files as an alternative to programs like cloc written in C++. I was becoming bothered with how cloc handled ignoring files, as I'll integrate some libraries directly in my source folder as opposed to an external folder, thus I wanted to mainly ignore specific files. I also wanted something faster and more extensible, so I made this.

## Building

Computare uses CMake for building. Build using your favorite compiler chain and run the executable that results from the build.

## Dependencies

Computare depends on [cxxopts](https://github.com/jarro2783/cxxopts) for parsing command line arguments for the program as well as [fmt](https://github.com/fmtlib/fmt) for nice print formatting and [yaml-cpp](https://github.com/jbeder/yaml-cpp) for the config.

## License

Computare is licensed under the MIT License, the terms of which can be seen [here](https://github.com/tinfoilboy/computare/blob/master/LICENSE).