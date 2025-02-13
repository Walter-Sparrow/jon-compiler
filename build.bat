@echo off

cd ./src
gcc -c -o ..\out\src\compiler.o compiler.cpp
gcc -o ..\out\src\compiler ..\out\src\compiler.o -lws2_32

del /F ..\out\src\compiler.o
