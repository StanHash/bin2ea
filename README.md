# `bin2ea`

Tool for converting binary files to EA code, optionally wrapped around a label or a definition.

~~The purpose of this tool is *not* to be used with EA `#inctext`, since you can have the same effect with `#incbin` but better~~. Actually, for *very* large files, I think you can gain a lot of processing time by using an #incbin variant that outputs `SHORT`s or `WORD`s (instead of `BYTÈ`s), since that would mean 2/4 times less integer literals to parse for EA (even if they would be longer, but it would still be better performance & memory-wise), and what do you know `bin2ea` can do that (`#inctext bin2ea -word "Your File.bin"`).

I made this tool a while back to help me build MSG helper definitions directly from asm files (as opposed to manually having to write the short values in the definitions), and now I'm reviving it because I *might* find another need for it real soon.

# Usage
```
bin2ea <input> [output/--to-stdout] [-byte/-short/-word] [-define <name>/-label <name>] [-before <before>] [-after <after>] [-no-newline]
```

# Building

Requirements:
- A C++ Compiler that supports C++11 and it's standard library ([MinGW-w64](https://sourceforge.net/projects/mingw-w64/) under Windows for example)
- [CMake](https://cmake.org/)

Goto the root folder of this repository (the one with `CMakeFiles.txt`), run something like the following:

```cmd
mkdir build
cd build
cmake ..
make
```

(Or use cmake-gui or another tool/IDE that can do CMake for you)

[See the CMake manual for more info](https://cmake.org/cmake/help/v3.10/manual/cmake.1.html).
