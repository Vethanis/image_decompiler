#!/bin/bash

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

rm -rf bin

if [ "$1" == "clean" ]; then
    rm -rf build
fi

if [ ! -d "./build" ]; then
    mkdir build
    cd build
    cmake .. -G "Visual Studio 15 Win64"
    cd ../
fi

TYPE="Release"
if [ "$1" == "debug" ] || [ "$2" == "debug" ] ; then
    TYPE="Debug"
fi

cmake --build build --config "$TYPE"

if [[ $? > 0 ]]; then
    exit 1
fi

GLSL="src/*.glsl"
DLL="lib/*.dll"

if [ $TYPE == "Debug" ]; then
    mkdir -p bin/Debug
    mkdir -p bin/Debug/screenshots
    cp $GLSL bin/Debug
    cp $DLL bin/Debug
    cp run.* bin/Debug
    cp assets/* bin/Debug
else 
    mkdir -p bin/Release
    mkdir -p bin/Release/screenshots
    cp $GLSL bin/Release
    cp $DLL bin/Release
    cp run.* bin/Release
    cp assets/* bin/Release
    rm bin/Release/thumbs.db
fi
