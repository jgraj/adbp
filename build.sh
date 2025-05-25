#!/bin/bash
set -e

g++ adbp.cpp -o adbp-0.3 -std=c++20 -Wall -O3 -static -s -Iinc