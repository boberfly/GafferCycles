#!/usr/bin/env python
##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
#  Modified by Alex Fuller for GafferCycles building.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import os
import sys
import distutils.util
import uuid
import json
import shutil
import argparse
import subprocess
import multiprocessing

# Set the platform.

platform = ""
if "linux" in sys.platform:
    platform = "linux"
elif "darwin" in sys.platform:
    platform = "osx"
elif "win32" in sys.platform:
    platform = "windows"

parser = argparse.ArgumentParser()

parser.add_argument(
	"--gafferVersion",
	default = "0.59.4.0",
	help = "The version of Gaffer to build against. "
)

parser.add_argument(
	"--version",
	help = "The version to build. Can either be a tag or SHA1 commit hash."
)

parser.add_argument(
	"--cyclesVersion",
	help = "The version to build. Can either be a tag or SHA1 commit hash."
)

parser.add_argument(
	"--pythonVariant",
	default = "2",
	choices = [ "2", "3" ],
	help = "The version of Python to build for."
)

parser.add_argument(
	"--upload",
	type = distutils.util.strtobool,
	default = "0",
	help = "Uploads the resulting package to the GitHub release page. You must "
	       "have manually created the release and release notes already."
)

parser.add_argument(
	"--docker",
	type = distutils.util.strtobool,
	default = "linux" in sys.platform,
	help = "Performs the build using a Docker container. This provides a "
	       "known build platform so that builds are repeatable."
)

parser.add_argument(
	"--interactive",
	type = distutils.util.strtobool,
	default = False,
	help = "When using docker, starts an interactive shell rather than "
		   "performing the build. This is useful for debugging."
)

parser.add_argument(
	"--platform",
	default = platform,
    choices = [ "linux", "osx", "windows" ],
	help = "The platform to build for. "
)

parser.add_argument(
	"--forceCxxCompiler",
	default = "g++",
	help = "Force a particular C++ compiler."
)

parser.add_argument(
	"--buildType",
	choices = ["release", "debug", "relWithDebInfo"],
	default = "release",
	help = "The build type eg. release, debug, relWithDebInfo (relWithDebInfo is CMake only, reverts to release on other build systems). Default is release."
)

parser.add_argument(
	"--output",
	default = "out",
	help = "The output directory."
)

parser.add_argument(
	"--optix",
	type = distutils.util.strtobool,
	default = False,
	help = "Build with OptiX."
)

parser.add_argument(
	"--optixPath",
	default = "/optix",
	help = "OptiX path."
)

parser.add_argument(
	"--experimental",
	type = distutils.util.strtobool,
	default = False,
	help = "Enable experimental build. Currently compiles light groups and texture cache modes."
)

args = parser.parse_args()

buildType = ""
if "release" in args.buildType:
    buildType = "Release"
elif "debug" in args.buildType:
    buildType = "Debug"
elif "relWithDebInfo" in args.buildType:
    buildType = "RelWithDebInfo"

cmakeGenerator = "\"Unix Makefiles\""
dockerFile = "Dockerfile"
termCmd = "bash"
pyCmd = "./build.py"
if "windows" in args.platform:
    cmakeGenerator = "\"NMake Makefiles JOM\""
    dockerFile = "Dockerfile.wine"
    termCmd = "cmd.exe"
    pyCmd = "python build.py"

if args.interactive :
    if not args.docker :
        parser.exit( 1, "--interactive requires --docker\n" )
    if args.version or args.upload :
        parser.exit( 1, "--interactive can not be used with other flags\n" )
else :
    if not args.version :
        parser.exit( "--version argument is required")

# Check that our environment contains everything we need to do a build.

for envVar in ( "GITHUB_RELEASE_TOKEN", ) :
	if envVar not in os.environ	:
		parser.exit( 1,  "{0} environment variable not set".format( envVar ) )

# Build a little dictionary of variables we'll need over and over again
# in string formatting operations, and use it to figure out what
# package we will eventually be generating.

formatVariables = {
	"version" : args.version,
	"upload" : args.upload,
	"platform" : args.platform,
	"gafferVersion" : args.gafferVersion,
	"cyclesVersion" : args.cyclesVersion,
	"pythonVariant" : args.pythonVariant,
	"releaseToken" : os.environ["GITHUB_RELEASE_TOKEN"],
	"auth" : '-H "Authorization: token {}"'.format( os.environ["GITHUB_RELEASE_TOKEN"] ),
	"buildType" : buildType,
	"cmakeGenerator" : cmakeGenerator,
	"cxx" : args.forceCxxCompiler,
	"output" : os.path.abspath( args.output ),
	"optix" : args.optix,
	"optixPath" : args.optixPath,
	"experimental" : args.experimental,
}

exp = ""
if formatVariables["experimental"] :
	exp = "-experimental"

packageName = "gafferCycles-{version}{exp}-gaffer-{gafferVersion}-{platform}-python{pythonVariant}".format( exp=exp, **formatVariables )
formatVariables["uploadFile"] = "%s.tar.gz" % packageName

# If we're going to be doing an upload, then check that the release exists. Better
# to find out now than at the end of a lengthy build.

def releaseId() :

	release = subprocess.check_output(
		"curl -s {auth} https://api.github.com/repos/boberfly/GafferCycles/releases/tags/{version}".format(
			**formatVariables
		),
		shell = True
	)
	release = json.loads( release )
	return release.get( "id" )

if args.upload and releaseId() is None :
	parser.exit( 1, "Release {version} not found\n".format( **formatVariables ) )

# Restart ourselves inside a Docker container so that we use a repeatable
# build environment.

if args.docker and not os.path.exists( "/.dockerenv" ) :

	imageCommand = "docker build -f {dockerFile} -t gaffercycles-build .".format( dockerFile=dockerFile, **formatVariables )
	sys.stderr.write( imageCommand + "\n" )
	subprocess.check_call( imageCommand, shell = True )

	#containerMounts = "-v {output}:/out:rw,Z".format( **formatVariables )
	containerEnv = "GITHUB_RELEASE_TOKEN={releaseToken}".format( **formatVariables )
	containerName = "gaffercycles-build-{id}".format( id = uuid.uuid1() )

	if args.interactive :
		containerBashCommand = "{env} {termCmd}".format( env = containerEnv, termCmd = termCmd )
	else :
		containerBashCommand = "{env} {pyCmd} --gafferVersion {gafferVersion} --cyclesVersion {cyclesVersion} --pythonVariant {pythonVariant} --version {version} --upload {upload} --platform {platform} --optix {optix} --experimental {experimental} --output=./out".format( pyCmd = pyCmd, env = containerEnv, **formatVariables )

	containerCommand = "docker run --name {name} -i -t gaffercycles-build -c '{command}'".format(
		name = containerName,
		#mounts = containerMounts,
		command = containerBashCommand
	)

	sys.stderr.write( containerCommand + "\n" )
	subprocess.check_call( containerCommand, shell = True )

	if not args.interactive :
		# Copy out the generated package.
		copyCommand = "docker cp {container}:/out/{uploadFile} {output}".format(
			container = containerName,
			**formatVariables
		)
		sys.stderr.write( copyCommand + "\n" )
		subprocess.check_call( copyCommand, shell = True )

	sys.exit( 0 )

# Output directory

if not os.path.isdir( formatVariables["output"] ) :
	os.makedirs( formatVariables["output"] )

# Download Gaffer

gafferURL = "https://github.com/GafferHQ/gaffer/releases/download/{gafferVersion}/gaffer-{gafferVersion}-{platform}-python{pythonVariant}.tar.gz".format( **formatVariables )
if platform == "windows" :
	gafferURL = "https://github.com/hypothetical-inc/gaffer/releases/download/{gafferVersion}-beta/gaffer-{gafferVersion}-{platform}-python{pythonVariant}.zip".format( **formatVariables )

sys.stderr.write( "Downloading gaffer \"%s\"\n" % gafferURL )

gafferDirName = "gaffer-{gafferVersion}-{platform}-python{pythonVariant}".format( **formatVariables )
tarFileName = "{0}.tar.gz".format( gafferDirName )
downloadCommand = "curl -L {0} > {1}".format( gafferURL, tarFileName )
sys.stderr.write( downloadCommand + "\n" )
subprocess.check_call( downloadCommand, shell = True )

sys.stderr.write( "Decompressing gaffer to \"%s\"\n" % gafferDirName )

shutil.rmtree( gafferDirName, ignore_errors = True )
os.makedirs( gafferDirName )
subprocess.check_call( "tar xf %s -C %s --strip-components=1" % ( tarFileName, gafferDirName ), shell = True )
gafferDirName = os.path.abspath( gafferDirName )

# Download GafferCycles

gafferCyclesURL = "https://github.com/boberfly/GafferCycles/archive/{version}.tar.gz".format( **formatVariables )
sys.stderr.write( "Downloading GafferCycles \"%s\"\n" % gafferCyclesURL )

gafferCyclesDirName = "gaffercycles-{version}-source".format( **formatVariables )
tarFileName = "{0}.tar.gz".format( gafferCyclesDirName )
downloadCommand = "curl -L {0} > {1}".format( gafferCyclesURL, tarFileName )
sys.stderr.write( downloadCommand + "\n" )
subprocess.check_call( downloadCommand, shell = True )

sys.stderr.write( "Decompressing GafferCycles to \"%s\"\n" % gafferCyclesDirName )

shutil.rmtree( gafferCyclesDirName, ignore_errors = True )
os.makedirs( gafferCyclesDirName )
subprocess.check_call( "tar xf %s -C %s --strip-components=1" % ( tarFileName, gafferCyclesDirName ), shell = True )
os.chdir( gafferCyclesDirName )

# Download Cycles stand-alone

cyclesURL = "https://github.com/boberfly/cycles/archive/{cyclesVersion}.tar.gz".format( **formatVariables )
sys.stderr.write( "Downloading Cycles \"%s\"\n" % cyclesURL )

cyclesDirName = "cycles"
tarFileName = "{0}.tar.gz".format( cyclesDirName )
downloadCommand = "curl -L {0} > {1}".format( cyclesURL, tarFileName )
sys.stderr.write( downloadCommand + "\n" )
subprocess.check_call( downloadCommand, shell = True )

sys.stderr.write( "Decompressing Cycles to \"%s\"\n" % cyclesDirName )

subprocess.check_call( "tar xf %s -C %s --strip-components=1" % ( tarFileName, cyclesDirName ), shell = True )

gafferCyclesDirName = os.getcwd()

# Perform the build.

withOptix = "OFF"
if os.path.isfile( formatVariables["optixPath"] + "/include/optix.h" ) :
	withOptix = "ON"

manifest = " ".join( [
	"doc/*",
	"include/*",
	"lib/*.cubin",
	"lib/*.ptx",
	"lib/libGafferCycles*",
	"license/*",
	"python/*",
	"shader/*",
	"source/*",
	"startup/*"
	] )

depCommands = [
	"cmake -E make_directory install/{platform}_{buildType}".format( **formatVariables ),

	"cd dependencies && "
	"./build/build.py --project Gflags --gafferRoot {gafferRoot} --buildDir {gafferCyclesRoot}/install/{platform}_{buildType} --forceCxxCompiler {cxx} && "
	"./build/build.py --project Glog --gafferRoot {gafferRoot} --buildDir {gafferCyclesRoot}/install/{platform}_{buildType} --forceCxxCompiler {cxx} && "
	"./build/build.py --project Embree --gafferRoot {gafferRoot} --buildDir {gafferCyclesRoot}/install/{platform}_{buildType} --forceCxxCompiler {cxx} && "
	"./build/build.py --project OpenSubdiv --gafferRoot {gafferRoot} --buildDir {gafferCyclesRoot}/install/{platform}_{buildType} --forceCxxCompiler {cxx} && "
	"./build/build.py --project OpenImageDenoise --gafferRoot {gafferRoot} --buildDir {gafferCyclesRoot}/install/{platform}_{buildType} --forceCxxCompiler {cxx}".format( 
		gafferCyclesRoot=gafferCyclesDirName, gafferRoot=gafferDirName, **formatVariables ),
]

commands = [
	"cmake -E make_directory build/{platform}_{buildType}".format( **formatVariables ),

	"cd build/{platform}_{buildType} &&"
		" cmake"
		" -G {cmakeGenerator}"
		" -D CMAKE_INSTALL_PREFIX={gafferCyclesRoot}/install/{platform}_{buildType}"
		" -D CMAKE_BUILD_TYPE={buildType}"
		" -D GAFFER_ROOT={gafferRoot}"
		" -D CMAKE_CXX_COMPILER={cxx}"
		" -D WITH_CYCLES_DEVICE_CUDA=ON"
		" -D WITH_CYCLES_CUDA_BINARIES=ON"
		" -D OPTIX_ROOT_DIR={optixPath}"
		" -D WITH_CYCLES_DEVICE_OPTIX={withOptix}"
		" -D WITH_CYCLES_EMBREE=ON"
		" -D WITH_CYCLES_OPENSUBDIV=ON"
		" -D WITH_CYCLES_LOGGING=ON"
		" -D WITH_CYCLES_TEXTURE_CACHE={withExperimental}"
		" -D WITH_CYCLES_LIGHTGROUPS={withExperimental}"
		" -D WITH_OPENIMAGEDENOISE=ON"
		" -D PYTHON_VARIANT={pythonVariant}"
		" ../..".format( gafferCyclesRoot=gafferCyclesDirName, gafferRoot=gafferDirName, withOptix=withOptix, withExperimental=str( int( formatVariables["experimental"] ) ), **formatVariables ),

	"cd build/{platform}_{buildType} && cmake --build . --config {buildType} --target install -- -j {jobs}".format( jobs=multiprocessing.cpu_count(), **formatVariables ),
	"mv install/{platform}_{buildType}/lib/cmake /tmp/cmake && "
	"if [ -d \"install/{platform}_{buildType}/lib64\" ]; then mv install/{platform}_{buildType}/lib64/* install/{platform}_{buildType}/lib; fi && "
	"mv /tmp/cmake/* install/{platform}_{buildType}/lib/cmake".format( **formatVariables ),

	"cd install/{platform}_{buildType} && "
	"tar -c -z -f /tmp/intermediate.tar {manifest} && "
	"rm -rf /tmp/{packageName} && "
	"mkdir /tmp/{packageName} && "
	"cd /tmp/{packageName} && "
	"tar -x -f /tmp/intermediate.tar && "
	"cd /tmp && "
	"tar -c -z -f {output}/{uploadFile} {packageName}".format( 
		manifest=manifest, packageName=packageName, gafferCyclesRoot=gafferCyclesDirName, **formatVariables ),
]

env = os.environ.copy()
env["LD_LIBRARY_PATH"] = gafferDirName + os.sep + "lib" + os.pathsep + env.get( "LD_LIBRARY_PATH", "" )

for command in depCommands :
	sys.stderr.write( command + "\n" )
	if (formatVariables["pythonVariant"] == "3"):
		subprocess.check_call( command, shell = True, env = env )
	else:
		subprocess.check_call( command, shell = True)

for command in commands :
	sys.stderr.write( command + "\n" )
	subprocess.check_call( command, shell = True, env = env )

# Upload the build

if args.upload :

	uploadCommand = (
		'curl {auth}'
		' -H "Content-Type: application/zip"'
		' --data-binary @{uploadFile} "{uploadURL}"'
		' -o /tmp/curlResult.txt' # Must specify output file in order to get progress output
	).format(
		uploadURL = "https://uploads.github.com/repos/boberfly/gaffercycles/releases/{id}/assets?name={uploadName}".format(
			id = releaseId(),
			uploadName = os.path.basename( formatVariables["uploadFile"] ),
			**formatVariables
		),
		**formatVariables
	)

	sys.stderr.write( "Uploading package\n" )
	sys.stderr.write( uploadCommand + "\n" )

	subprocess.check_call( uploadCommand, shell = True )
