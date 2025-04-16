#!/bin/bash

x86_64-w64-mingw32-gcc -o my_program.exe src/*.c -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
echo "Build complete"

