# We start with CentOS 7, because it is commonly used in
# production, and meets the glibc requirements of VFXPlatform 2018
# (2.17 or lower).

FROM centos:7

# Make GCC 6.3.1 the default compiler, as per VFXPlatform 2018

RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-6

# Install CMake, SCons, and other miscellaneous build tools.
# We install SCons via `pip install --egg` rather than by
# `yum install` because this prevents a Cortex build failure
# caused by SCons picking up the wrong Python version and being
# unable to find its own modules.

RUN yum install -y epel-release

RUN yum install -y cmake3
RUN ln -s /usr/bin/cmake3 /usr/bin/cmake

RUN yum install -y python2-pip.noarch
RUN pip install --egg scons

RUN yum install -y patch
RUN yum install -y doxygen

# Install boost dependencies (needed by boost::iostreams)

RUN yum install -y bzip2-devel

# Install JPEG dependencies

RUN yum install -y nasm

# Install PNG dependencies

RUN yum install -y zlib-devel

# Install GLEW dependencies

RUN yum install -y libX11-devel
RUN yum install -y mesa-libGL-devel
RUN yum install -y mesa-libGLU-devel
RUN yum install -y libXmu-devel
RUN yum install -y libXi-devel

# Install OSL dependencies

RUN yum install -y flex
RUN yum install -y bison

# Install Qt dependencies

RUN yum install -y xkeyboard-config.noarch
RUN yum install -y fontconfig-devel.x86_64

# Install packages needed to generate the
# Gaffer documentation. Note that we are
# limited to Sphinx 1.4 because recommonmark
# is incompatible with later versions. And
# we are limited to docutils 0.12 because
# Sphinx 1.4 is incompatible with later
# versions.

RUN yum install -y xorg-x11-server-Xvfb
RUN yum install -y mesa-dri-drivers.x86_64
RUN yum install -y metacity
RUN yum install -y gnome-themes-standard

RUN pip install sphinx==1.4 sphinx_rtd_theme recommonmark
RUN pip install docutils==0.12

RUN yum install -y inkscape

RUN yum install -y wget

# CUDA 10.1

RUN wget -O cuda.rpm https://developer.nvidia.com/compute/cuda/10.1/Prod/local_installers/cuda-repo-rhel7-10-1-local-10.1.105-418.39-1.0-1.x86_64.rpm --progress=dot:mega \
    && rpm -i cuda.rpm && yum install -y cuda && rm cuda.rpm

# Copy over build script and set an entry point that
# will use the compiler we want.

COPY build.py ./

ENTRYPOINT [ "scl", "enable", "devtoolset-6", "--", "bash" ]