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
			" -DEMBREE_ISPC_SUPPORT=OFF"
			" -DEMBREE_TUTORIALS=OFF"
			" -DEMBREE_RAY_MASK=ON"
			" -DEMBREE_FILTER_FUNCTION=ON"
			" -DEMBREE_BACKFACE_CULLING=OFF"
			" -DEMBREE_TASKING_SYSTEM=INTERNAL"
			" ..",
		"cd gafferBuild && cmake --build . --config {cmakeBuildType} --target install -- -j {jobs}",

	],

	"libLinkType" : {

		"static" : "EMBREE_STATIC_LIB=ON",
		"shared" : "EMBREE_STATIC_LIB=OFF",
		"default" : "EMBREE_STATIC_LIB=OFF",

	},

}
