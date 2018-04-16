#!/bin/bash

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

if [ "$1" == "clean" ]; then
    ./clean.sh
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

mkdir -p bin/Release
mkdir -p bin/Debug
mkdir -p bin/Release/screenshots
mkdir -p bin/Debug/screenshots

cp $GLSL bin/Release
cp $GLSL bin/Debug
cp $DLL bin/Release
cp $DLL bin/Debug
cp run.* bin/Release
cp run.* bin/Debug
cp assets/* bin/Release
cp assets/* bin/Debug
