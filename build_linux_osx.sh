#!/usr/bin/env bash

set -e

if [[ -z "${GAFFER_ROOT}" ]]; then
    echo "ERROR : GAFFER_ROOT environment variable not set"
    exit 1
fi

# Packaging variables
VERSION=0.10.2
GAFFERVERSION=0.54.2.0

if [[ -z "${GAFFER_BUILD_TYPE}" ]]; then
    echo "WARNING : GAFFER_BUILD_TYPE environment variable not set, defaulting to release"
    GAFFER_BUILD_TYPE="release"
fi

if [[ $GAFFER_BUILD_TYPE == "release" ]]; then
    CMAKE_BUILD_TYPE=Release
elif [[ $GAFFER_BUILD_TYPE == "debug" ]]; then
    CMAKE_BUILD_TYPE=Debug
fi

if [[ `uname` = "Linux" ]] ; then
	SHLIBSUFFIX=".so"
	PLATFORM="linux"
    GAFFERCYCLES=linux_$GAFFER_BUILD_TYPE
    CC=gcc-6
    CXX=g++-6
else
	SHLIBSUFFIX=".dylib"
	PLATFORM="osx"
    GAFFERCYCLES=osx_$GAFFER_BUILD_TYPE
    CC=clang
    CXX=clang++
fi
GAFFERCYCLES_INSTALL=$PWD/install/$GAFFERCYCLES

# To build OSL shaders
export LD_LIBRARY_PATH=$GAFFER_ROOT/lib${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH:-}

# Dependencies
cmake -E make_directory install/$GAFFERCYCLES
cd dependencies
$GAFFER_ROOT/bin/python ./build/build.py --project Gflags --buildDir $GAFFERCYCLES_INSTALL --forceCCompiler $CC --forceCxxCompiler $CXX
$GAFFER_ROOT/bin/python ./build/build.py --project Glog --buildDir $GAFFERCYCLES_INSTALL --forceCCompiler $CC --forceCxxCompiler $CXX
$GAFFER_ROOT/bin/python ./build/build.py --project Embree --buildDir $GAFFERCYCLES_INSTALL --forceCCompiler $CC --forceCxxCompiler $CXX
$GAFFER_ROOT/bin/python ./build/build.py --project OpenSubdiv --buildDir $GAFFERCYCLES_INSTALL --forceCCompiler $CC --forceCxxCompiler $CXX
cd ..

# CMake build
cmake -E make_directory build/$GAFFERCYCLES
cd build/$GAFFERCYCLES
cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DGAFFER_ROOT=$GAFFER_ROOT -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_INSTALL_PREFIX=$GAFFERCYCLES_INSTALL -DWITH_CYCLES_EMBREE=ON -DWITH_CYCLES_OPENSUBDIV=ON -DWITH_CYCLES_LOGGING=ON $@ ../..
make -j `getconf _NPROCESSORS_ONLN`
make install

manifest="

    doc/*
    include/*
    lib/*.cubin
    lib/libembree3$SHLIBSUFFIX*
    lib/libosdCPU$SHLIBSUFFIX*
    lib/libGafferCycles$SHLIBSUFFIX
    license/*
    python/*
    shader/*
    source/*
    startup/*

"

cd $GAFFERCYCLES_INSTALL
packageName=gafferCycles-$VERSION-gaffer-$GAFFERVERSION-$PLATFORM
if [[ $GAFFER_BUILD_TYPE == "debug" ]]; then
    packageName=$packageName-$GAFFER_BUILD_TYPE
fi
archiveName=$packageName.tar.gz

tar -c -z -f /tmp/intermediate.tar $manifest
rm -rf /tmp/$packageName
mkdir /tmp/$packageName
cd /tmp/$packageName
tar -x -f /tmp/intermediate.tar
cd /tmp
tar -c -z -f $GAFFERCYCLES_INSTALL/$archiveName $packageName
