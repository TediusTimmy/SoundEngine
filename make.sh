#!/bin/bash

# This is, in fact, how I make my code. It is probably more useful to me than you, but here it is for your edification.
# As you can see, I am using the MinGW w64 cross compiler for Cygwin. Native windows programs, Linux build tools. (Yes, they are garbage, but they're comfortable garbage.)
# I don't generally use -Weffc++ : it generates a lot of noise.

x86_64-w64-mingw32-g++.exe -s -O2 -std=c++17 -o Program -Wall -Wextra -Wpedantic main.cpp -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm -Wl,-subsystem,windows
#x86_64-w64-mingw32-g++.exe -s -O2 -std=c++17 -o Program -Weffc++ -Wall -Wextra -Wpedantic main.cpp -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm -Wl,-subsystem,windows
#x86_64-w64-mingw32-g++.exe -g -std=c++17 -o Program -Wall -Wextra -Wpedantic main.cpp -luser32 -lgdi32 -lgdiplus -lopengl32 -lSHlwapi -ldwmapi -lstdc++fs -lwinmm
