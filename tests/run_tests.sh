#!/bin/bash
# Host-side unit tests for Windows-free logic. Builds with the native g++.
set -e
cd "$(dirname "$0")/.."
g++ -std=c++11 -Wall -Wextra -o tests/test_world_format \
    tests/test_world_format.cpp src/world_format.cpp
./tests/test_world_format
