#! /usr/bin/env python

import argparse
import glob
import os
import multiprocessing
import re
import subprocess
import shutil
import sys
import tarfile
import urllib
import zipfile

def __projects() :

	configFiles = glob.glob( "*/config.py" )
	return [ os.path.split( f )[0] for f in configFiles ]

def __decompress( archive ) :

	if os.path.splitext( archive )[1] == ".zip" :
		with zipfile.ZipFile( archive ) as f :
			for info in f.infolist() :
				extracted = f.extract( info.filename )
				os.chmod( extracted, info.external_attr >> 16 )
			files = f.namelist()
	elif archive.endswith( ".tar.xz" ) :
		## \todo When we eventually move to Python 3, we can use
		# the `tarfile` module for this too.
		command = ""
		if sys.platform == "win32":
			command = "cmake -E tar xvf {archive}".format( archive=archive )
		else:
			command = "tar -xvf {archive}".format( archive=archive )
		sys.stderr.write( command + "\n" )
		files = subprocess.check_output( command, stderr=subprocess.STDOUT, shell = True )
		files = [ f for f in files.split( "\n" ) if f ]
		files = [ f[2:] if f.startswith( "x " ) else f for f in files ]
	else :
		with tarfile.open( archive, "r:*" ) as f :
			f.extractall()
			files = f.getnames()

	dirs = { f.split( "/" )[0] for f in files if re.search( "warning:", f ) == None }
	if len( dirs ) == 1 :
		# Well behaved archive with single top-level
		# directory.
		return next( iter( dirs ) )
	else :
		# Badly behaved archive
		return "./"

def __loadConfig( project, gafferRoot, buildDir, buildType, forceLibLinkType, forceCCompiler, forceCxxCompiler ) :

	# Load file. Really we want to use JSON to
	# enforce a "pure data" methodology, but JSON
	# doesn't allow comments so we use Python
	# instead. Because we use `eval` and don't expose
	# any modules, there's not much beyond JSON
	# syntax that we can use in practice.

	with open( project + "/config.py" ) as f :
		config =f.read()

	config = eval( config )

	# Apply platform-specific config overrides.

	config["platform"] = "platform:{}".format({ "darwin": "osx", "linux":"linux", "win32": "windows"}.get( sys.platform, "linux" ))
	platformOverrides = config.pop( config["platform"], {} )

	unused_keys = [key for key, value in config.items() if "platform:" in key]
	for key in unused_keys:
		config.pop( key, None )

	for key, value in platformOverrides.items() :

		if isinstance( value, dict ) and key in config :
			config[key].update( value )
		else :
			config[key] = value

	# Apply variable substitutions.

	variables = config.get( "variables", {} ).copy()
	libLinkType = ""
	libLinkTypes = config.get( "libLinkType", {} ).copy()
	if( libLinkTypes ) :
		if( forceLibLinkType == "shared" ) :
			libLinkType = libLinkTypes.get("shared", "")
		elif( forceLibLinkType == "static" ) :
			libLinkType = libLinkTypes.get("static", "")
		elif( forceLibLinkType == "default" ) :
			libLinkType = libLinkTypes.get("default", "")
	else :
		if( forceLibLinkType != "default" ) :
			print( "Warning --forceLibLinkType %s is set but there is no config for this" % forceLibLinkType )
	cmake_generator = "\"NMake Makefiles JOM\"" if config["platform"] == "platform:windows" else "\"Unix Makefiles\""
	boostBuildType = "release"
	cmakeBuildType = "Release"
	if buildType == "debug":
		cmakeBuildType = "Debug"
		boostBuildType = "debug"
	elif buildType == "relWithDebInfo":
		cmakeBuildType = "RelWithDebInfo"
	
	if gafferRoot == "" :
		gafferRoot = buildDir

	default_variables = {
		"buildDir" : buildDir,
		"buildDirFwd" : buildDir.replace("\\", "/"),
		"jobs" : multiprocessing.cpu_count(),
		"cmakeGenerator" : cmake_generator,
		"cmakeBuildType": buildType,
		"boostBuildType" : buildType,
		"libLinkType" : libLinkType,
		"cCompiler" : forceCCompiler,
		"cxxCompiler" : forceCxxCompiler,
		"gafferRoot" : gafferRoot,
	}
	missing_variables = { k:v for (k, v) in default_variables.items() if k not in variables }
	variables.update( missing_variables )

	if config["platform"] == "platform:windows":
		# make sure JOM is in the path
		path_variable = ""
		if "environment" in config:
			path_variable = os.path.expandvars(config["environment"].get("PATH", "%PATH%"))
			config["environment"].update( { "PATH": path_variable + ";%ROOT_DIR%\\winbuild\\jom" } )
		else:
			config["environment"] = { "PATH": "%PATH%;%ROOT_DIR%\\winbuild\\jom" }

	def __substitute( o ) :

		if isinstance( o, dict ) :
			return { k : __substitute( v ) for k, v in o.items() }
		elif isinstance( o, list ) :
			return [ __substitute( x ) for x in o ]
		elif isinstance( o, tuple ) :
			return tuple( __substitute( x ) for x in o )
		elif isinstance( o, str ) :
			while True :
				s = o.format( **variables )
				if s == o :
					return s
				else :
					o = s

	return __substitute( config )

def __buildProject( project, gafferRoot, buildDir, buildType, forceLibLinkType, forceCCompiler, forceCxxCompiler ) :

	config = __loadConfig( project, gafferRoot, buildDir, buildType, forceLibLinkType, forceCCompiler, forceCxxCompiler )

	archiveDir = project + "/archives"
	if not os.path.exists( archiveDir ) :
		os.makedirs( archiveDir )

	archives = []
	for download in config["downloads"] :

		archivePath = os.path.join( archiveDir, os.path.basename( download ) )
		archives.append( archivePath )

		if os.path.exists( archivePath ) :
			continue

		sys.stderr.write( "Downloading {}".format( download ) + "\n" )
		urllib.urlretrieve( download, archivePath )

	workingDir = project + "/working"
	if os.path.exists( workingDir ) :
		shutil.rmtree( workingDir )
	os.makedirs( workingDir )
	os.chdir( workingDir )

	decompressedArchives = [ __decompress( "../../" + a ) for a in archives ]
	os.chdir( config.get( "workingDir", decompressedArchives[0] ) )

	if config.get("license") is not None :
		licenseDir = os.path.join( buildDir, "doc/licenses" )
		if not os.path.exists( licenseDir ) :
			os.makedirs( licenseDir )
		shutil.copy( config["license"], os.path.join( licenseDir, project ) )

	patch_command = "%ROOT_DIR%\\winbuild\\patch\\bin\\patch" if config["platform"] == "platform:windows" else "patch"
	for patch in glob.glob( "../../patches/*.patch" ) :
		subprocess.check_call( "{patch_command} -p1 < {patch}".format( patch = patch, patch_command = patch_command ), shell = True )
	for patch in glob.glob( "../../patches/{}/*.patch".format( config["platform"].lstrip( "platform:" ) ) ) :
		subprocess.check_call( "{patch_command} -p1 < {patch}".format( patch = patch, patch_command = patch_command ), shell = True )

	if config["platform"] == "platform:windows" and "LD_LIBRARY_PATH" in config.get( "environment", {} ) :
		config["environment"]["PATH"] = "{0};{1}".format( config["environment"]["LD_LIBRARY_PATH"], config["environment"].get( "PATH", "%PATH%" ) )

	environment = os.environ.copy()
	for k, v in config.get( "environment", {} ).items() :
		environment[k] = os.path.expandvars( v )

	for command in config["commands"] :
		sys.stderr.write( command + "\n" )
		subprocess.check_call( command, shell = True, env = environment )

	for link in config.get( "symbolicLinks", [] ) :
		if os.path.exists( link[0] ) :
			os.remove( link[0] )
		os.symlink( link[1], link[0] )

parser = argparse.ArgumentParser()

parser.add_argument(
	"--project",
	choices = __projects(),
	help = "The project to build."
)

parser.add_argument(
	"--buildDir",
	required = True,
	help = "The directory to put the builds in."
)

parser.add_argument(
	"--gafferRoot",
	default = "",
	help = "The directory where Gaffer is located. Defaults to buildDir"
)

parser.add_argument(
	"--buildType",
	choices = ["release", "debug", "relWithDebInfo"],
	default = "release",
	help = "The build type eg. release, debug, relWithDebInfo (relWithDebInfo is CMake only, reverts to release on other build systems). Default is release."
)

parser.add_argument(
	"--forceLibLinkType",
	choices = ["default", "shared", "static"],
	default = "default",
	help = "The library type for linking eg. default, shared, static (default will use the recommended linking)."
)

parser.add_argument(
	"--forceCCompiler",
	default = "gcc",
	help = "Force a particular C compiler."
)

parser.add_argument(
	"--forceCxxCompiler",
	default = "g++",
	help = "Force a particular C++ compiler."
)

args = parser.parse_args()
__buildProject( args.project, args.gafferRoot, args.buildDir, args.buildType, args.forceLibLinkType, args.forceCCompiler, args.forceCxxCompiler )
