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

GafferCycles should be stable enough to try out now, I have made builds for Linux under releases. Try 0.3.0 onwards, GPU devices should work but I haven't tested on Nvidia/CUDA yet. OpenCL works.

The runtime just needs $GAFFERCYCLES set to the extracted path (so that shaders/kernels can be found) and $GAFFER_EXTENSION_PATHS so that Gaffer will detect GafferCycles.

### Docker Instructions

To build GafferCycles against the VFX Platform and be 100% consistent with upstream GafferHQ, run:
```
./build_docker.sh
```
If you have docker correctly installed, this should make a CentOS 7-based image, build GafferCycles and place the resulting package into the out/ directory (can be another path using the --output flag) eg.
```
./build_docker.sh --output /path/to/output
```

Docker Cheatsheet
-----------------

From https://github.com/GafferHQ/build :

Remove stopped containers :

`docker ps -aq --no-trunc | xargs docker rm`

Remove old images :

`docker images -q --filter dangling=true | xargs docker rmi`
