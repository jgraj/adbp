@echo off & setlocal

mingw64\bin\g++ adbp.cpp -o adbp-0.3.exe -std=c++20 -Wall -O3 -static -s -mconsole -Iinc