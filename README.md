# bin2ea
Tool for converting binary files to Labeled EA Code or EA Definitions. The purpose of this tool is *not* really to be used with EA `#inctext`, since you can have the same effect with `#incbin` in a much proper way.

I made this tool to help me build MSG definitions directly from asm files (as opposed to manually having to write the short values in the definitions).

# Building
Install CMake and your C++ Compiler of choice (needs C++14 support pls thx) ([MinGW-w64](https://sourceforge.net/projects/mingw-w64/) for Windows for example) and Run CMake.

# Usage
```
bin2ea <input> [output/--to-stdout] [-byte/-short/-word] [-define <name>/-label <name>] [-before <before>] [-after <after>] [-no-newline]
```
