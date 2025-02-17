@echo off

cd ./src
gcc -Wall -Wextra -c -o ..\out\src\parser.o parser.c
gcc -Wall -Wextra -c -o ..\out\src\stack.o stack.c
gcc -Wall -Wextra -c -o ..\out\src\jon.o jon.c
gcc -Wall -Wextra -c -o ..\out\src\interpreter.o interpreter.c
gcc -Wall -Wextra -c -o ..\out\src\compiler.o compiler.c
gcc -o ..\out\src\compiler ^
  ..\out\src\compiler.o ^
  ..\out\src\parser.o ^
  ..\out\src\stack.o ^
  ..\out\src\jon.o ^
  ..\out\src\interpreter.o ^
  -lws2_32

del /F ..\out\src\compiler.o ..\out\src\stack.o
