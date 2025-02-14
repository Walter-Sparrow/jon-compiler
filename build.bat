@echo off

cd ./src
gcc -c -o ..\out\src\stack.o stack.cpp
gcc -c -o ..\out\src\compiler.o compiler.cpp
gcc -o ..\out\src\compiler ..\out\src\compiler.o ..\out\src\stack.o -lws2_32

del /F ..\out\src\compiler.o ..\out\src\stack.o
