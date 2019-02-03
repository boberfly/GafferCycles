# GafferCycles

Cycles for Gaffer. Cycles is a submodule to this project, and will grab the dependencies from
the Gaffer install directory. Most all of Cycles' dependencies are covered by Gaffer, except
for the optional dependencies OpenSubdiv and Embree.

### Build Instructions

**Requires:**

* cmake
* Gaffer Install from GafferHQ
* (optional)OpenSubdiv
* (optional)Embree

**In a terminal (Linux):**
```
export GAFFER_ROOT=<gaffer install path>
export GAFFERCYCLES=<install destination>
export LD_LIBRARY_PATH=$GAFFER_ROOT/lib

git clone --recurse-submodules https://github.com/boberfly/GafferCycles.git
cd GafferCycles
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=g++-6 -DGAFFER_ROOT=$GAFFER_ROOT -DCMAKE_INSTALL_PREFIX=$GAFFERCYCLES ..
make install -j <num cores>
```

### Runtime Instructions

Add to Gaffer extensions path:

`export GAFFER_EXTENSION_PATHS=$GAFFERCYCLES:$GAFFER_EXTENSION_PATHS`

Run Gaffer:

`gaffer`

> Caveat: This is still very much WIP, it will not function as-is.
