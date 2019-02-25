{

	"downloads" : [

		"https://github.com/embree/embree/archive/v3.5.0.tar.gz"

	],

	"license" : "LICENSE.txt",

	"commands" : [

		"mkdir gafferBuild",
		"cd gafferBuild &&"
			" cmake"
			" -G {cmakeGenerator}"
			" -D CMAKE_CXX_COMPILER={cxxCompiler}"
			" -D CMAKE_INSTALL_PREFIX={buildDir}"
			" -D CMAKE_PREFIX_PATH={buildDir}"
			" -D {libLinkType}"
			" -D EMBREE_ISPC_SUPPORT=OFF"
			" -D EMBREE_TUTORIALS=OFF"
			" ..",
		"cd gafferBuild && cmake --build . --config {cmakeBuildType} --target install -- -j {jobs}",

	],

	"libLinkType" : {

		"static" : "EMBREE_STATIC_LIB=ON",
		"shared" : "EMBREE_STATIC_LIB=OFF",
		"default" : "EMBREE_STATIC_LIB=OFF",

	},

}
