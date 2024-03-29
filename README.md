# GafferCycles

![GafferCycles](gaffercycles.jpg)
- ["Old Attic Interior Scene" by David Lesperance](https://developer.nvidia.com/usd) (Public Domain)
- ["GafferBot" from GafferHQ](https://github.com/GafferHQ/resources) (BSD)
- ["Cloud" from Walt Disney Animation Studios](https://www.technology.disneyanimation.com/clouds) (CC BY-SA 3.0)

Cycles for Gaffer. Cycles is a submodule to this project, and will grab the dependencies from
the Gaffer install directory. Most all of Cycles' dependencies are covered by Gaffer, except
for the optional dependencies OpenSubdiv and Embree.

### Build Instructions

**Requires:**

* cmake
* Gaffer Install from GafferHQ
* OpenGL and zlib from your distribution, for ubuntu it would be: ```sudo apt-get install mesa-common-dev libgl1-mesa-dev zlib1g-dev```

Check Optional dependency install down below for easy python scripts
* Gflags
* Glog
* Embree
* OpenImageDenoise
* NanoVDB
* PugiXML

The Nvidia website contains CUDA and OptiX to compile for Nvidia GPUs
* CUDA
* OptiX (optional)

**In a terminal (Linux/Ubuntu):**
```
export GAFFER_ROOT=<gaffer install path>
export GAFFERCYCLES=<install destination>
export LD_LIBRARY_PATH=$GAFFER_ROOT/lib

git clone --recurse-submodules https://github.com/boberfly/GafferCycles.git
cd GafferCycles
mkdir build
cd build
cmake -DGAFFER_ROOT=$GAFFER_ROOT -DPYTHON_VARIANT=3 -DCMAKE_INSTALL_PREFIX=$GAFFERCYCLES ..
make install -j <num cores>
```

**Optional dependencies+install:**

Run this after cloning the git repo and before building
```
cd dependencies
python ./build/build.py --project Gflags --buildDir $GAFFERCYCLES
python ./build/build.py --project Glog --buildDir $GAFFERCYCLES
python ./build/build.py --project Embree --buildDir $GAFFERCYCLES --gafferRoot $GAFFER_ROOT
python ./build/build.py --project OpenImageDenoise --buildDir $GAFFERCYCLES --gafferRoot $GAFFER_ROOT
python ./build/build.py --project NanoVDB --buildDir $GAFFERCYCLES
python ./build/build.py --project PugiXML --buildDir $GAFFERCYCLES --gafferRoot $GAFFER_ROOT
cd ../build
cmake -DGAFFER_ROOT=$GAFFER_ROOT -DPYTHON_VARIANT=3 -DCMAKE_INSTALL_PREFIX=$GAFFERCYCLES -DWITH_CYCLES_EMBREE=ON -DWITH_CYCLES_OPENSUBDIV=ON -DWITH_CYCLES_LOGGING=ON -DWITH_CYCLES_LIGHTGROUPS=ON -DWITH_CYCLES_CORNER_NORMALS=ON ..
make install -j <num cores>
```
For CUDA support, make sure it is installed and ```WITH_CYCLES_DEVICE_CUDA=ON``` added to the cmake line.
For OptiX, it will need to be installed from Nvidia's website and ```-DWITH_CYCLES_DEVICE_OPTIX=ON -DOPTIX_ROOT_DIR=$OPTIX_ROOT``` added to the cmake line.

### Runtime Instructions

Add to Gaffer extensions path:

`export GAFFER_EXTENSION_PATHS=$GAFFERCYCLES:$GAFFER_EXTENSION_PATHS`

Run Gaffer:

`gaffer`

GafferCycles should be stable enough to try out now, I have made builds for Linux under releases.

Make sure to append where GafferCycles is installed/extracted to by appending to $GAFFER_EXTENSION_PATHS so that Gaffer will detect GafferCycles.

### Docker Instructions

To build GafferCycles against the VFX Platform and be 100% consistent with upstream GafferHQ, run:
```
./build_docker.sh
```
If you have docker correctly installed, this should make a CentOS 7-based image, build GafferCycles and place the resulting package into the out/ directory (can be another path using the --output flag) eg.
```
./build_docker.sh --output /path/to/output
```
Alternatively, you can build with Nvidia OptiX support. You must download OptiX and copy it to this directory which needs to be named:
```
NVIDIA-OptiX-SDK-7.4.0-linux64-x86_64.sh
```
And run:
```
./build_docker_optix.sh
```

Docker Cheatsheet
-----------------

From https://github.com/GafferHQ/build :

Remove stopped containers :

`docker ps -aq --no-trunc | xargs docker rm`

Remove old images :

`docker images -q --filter dangling=true | xargs docker rmi`
