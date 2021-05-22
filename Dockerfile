# We start with CentOS 7, because it is commonly used in
# production, and meets the glibc requirements of VFXPlatform 2018
# (2.17 or lower).

FROM centos:7.6.1810

# As we don't want to inadvertently grab newer versions of our yum-installed
# packages, we use yum-versionlock to keep them pinned. We track the list of
# image packages here, then compare after our install steps to see what was
# added, and only lock those. This saves us storing redundant entires for
# packages installed in the base image.

# To unlock versions, just make sure yum-versionlock.list is empty in the repo
#COPY versionlock.sh ./
#COPY yum-versionlock.list /etc/yum/pluginconf.d/versionlock.list

#RUN yum install -y yum-versionlock && \
#	./versionlock.sh list-installed /tmp/packages && \
#
#
# NOTE: If you add a new yum package here, make sure you update the version
# lock files as follows and commit the changes to yum-versionlock.list:
#
#   ./build-docker.py --update-package-versions --new-only
#
# We have to install scl as a separate yum command for some reason
# otherwise we get `scl not found` errors...
#
RUN yum install -y centos-release-scl && \
	sed -i 's/7/7.6.1810/g; s|^#\s*\(baseurl=http://\)mirror|\1vault|g; /mirrorlist/d' /etc/yum.repos.d/CentOS-SCLo-*.repo && \
	yum install -y devtoolset-6 && \
#
#	Install CMake, SCons, and other miscellaneous build tools.
#	We install SCons via `pip install --egg` rather than by
#	`yum install` because this prevents a Cortex build failure
#	caused by SCons picking up the wrong Python version and being
#	unable to find its own modules.
#
	yum install -y epel-release && \
#
	yum install -y cmake3 && \
	ln -s /usr/bin/cmake3 /usr/bin/cmake && \
#
	yum install -y python2-pip.noarch && \
	pip install --egg scons==3.0.5 && \
#
	yum install -y \
		git \
		patch \
		doxygen && \
#
#	Install boost dependencies (needed by boost::iostreams)
#
	yum install -y bzip2-devel && \
#
#	Install JPEG dependencies
#
	yum install -y nasm && \
#
#	Install PNG dependencies && \
#
	yum install -y zlib-devel && \
#
#	Install GLEW dependencies
#
	yum install -y \
		libX11-devel \
		mesa-libGL-devel \
		mesa-libGLU-devel \
		libXmu-devel \
		libXi-devel && \
#
#	Install OSL dependencies
#
	yum install -y \
		flex \
		bison && \
#
#	Install Qt dependencies
#
	yum install -y \
		xkeyboard-config.noarch \
		fontconfig-devel.x86_64 && \
#
#	Install Appleseed dependencies
#
	yum install -y \
		lz4 lz4-devel
#
# Install packages needed to generate the
# Gaffer documentation.

#RUN yum install -y \
#		xorg-x11-server-Xvfb \
#		mesa-dri-drivers.x86_64 \
#		metacity \
#		gnome-themes-standard && \
# Note: When updating these, also update gaffer/config/azure/build.yaml
#	pip install \
#		sphinx==1.8.0 \
#		sphinx_rtd_theme==0.4.3 \
#		recommonmark==0.5.0 \
#		docutils==0.12 && \
#
#	yum install -y inkscape

# Now we've installed all our packages, update yum-versionlock for all the
# new packages so we can copy the versionlock.list out of the container when we
# want to update the build env.
# If there were already locks in the list from the source checkout then the
# correct version will already be installed and we just ignore this...
#	./versionlock.sh lock-new /tmp/packages

RUN yum install -y wget && \
#
#
# CUDA 11.3.1
#
	wget -O cuda.rpm https://developer.download.nvidia.com/compute/cuda/11.3.1/local_installers/cuda-repo-rhel7-11-3-local-11.3.1_465.19.01-1.x86_64.rpm --progress=dot:mega \
	&& rpm -i cuda.rpm && yum install -y cuda && rm cuda.rpm && \
#
# ISPC 1.15
#
	wget -O ispc.tar.gz https://github.com/ispc/ispc/releases/download/v1.15.0/ispc-v1.15.0-linux.tar.gz -- \
	&& mkdir /ispc && tar xf ispc.tar.gz -C /ispc --strip-components=1 && mv /ispc/bin/ispc /usr/bin/ispc && rm -rf /ispc

# OptiX 7.3.0

COPY NVIDIA-OptiX-SDK-7.3.0-linux64-x86_64.sh /
RUN mkdir /optix && ./NVIDIA-OptiX-SDK-7.3.0-linux64-x86_64.sh --skip-license --prefix=/optix --exclude-subdir

# Copy over build script and set an entry point that
# will use the compiler we want.

COPY build.py ./

ENTRYPOINT [ "scl", "enable", "devtoolset-6", "--", "bash" ]
