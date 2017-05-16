#!/bin/bash

rm -rf CMakeFiles
rm cmake_install.cmake
rm CMakeCache.txt

cmake -DCMAKE_CXX_COMPILER=mpic++ CMakeLists.txt
make
