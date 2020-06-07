/**
 * Test file for C/C++ source.
 *
 * In its current state it should count 22 total lines, 10 code lines, 7 comment lines, 5 empty lines.
 */
#include <iostream>

int main()
{
    // this comment should be counted as one line!

    std::cout << "hello world!" << std::endl; // this comment should not be counted as it's preceeded by a source line!

    std::cout << "woah here's a line comment delimiter in a string //" << std::endl; // that embeded comment shouldn't count
    std::cout << "woah here's a block comment /**/ in a string!" << std::endl;

    std::cout << "this has a block at the end!" << std::endl; /*
    the first line should still be counted as code, this line should be a comment, and the following should be code
    */std::cout << "i wrote this directly after a block comment end delimiter!" << std::endl;

    return 0;
}