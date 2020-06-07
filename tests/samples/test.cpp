/**
 * Test file for C/C++ source.
 *
 * In its current state it should count 18 total lines, 8 code lines, 6 comment lines, 4 empty lines.
 */
#include <iostream>

int main()
{
    // this comment should be counted as one line!

    std::cout << "hello world!" << std::endl; // this comment should not be counted as it's preceeded by a source line!

    std::cout << "woah here's a line comment delimiter in a string //" << std::endl; // that embeded comment shouldn't count
    std::cout << "woah here's a block comment /**/ in a string!" << std::endl;

    return 0;
}