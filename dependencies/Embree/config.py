{

	"downloads" : [

		"https://github.com/embree/embree/archive/v3.8.0.tar.gz"

	],

	"license" : "LICENSE.txt",

	"commands" : [

		"mkdir gafferBuild",
		"cd gafferBuild &&"
			" cmake"
			" -G {cmakeGenerator}"
			" -D CMAKE_CXX_COMPILER={cxxCompiler}"
			" -D CMAKE_INSTALL_PREFIX={buildDir}"
			" -D CMAKE_PREFIX_PATH={gafferRoot}"
			" -D CMAKE_BUILD_TYPE={cmakeBuildType}"
			" -D {libLinkType}"
			" -D EMBREE_ISPC_SUPPORT=OFF"
			" -D EMBREE_TUTORIALS=OFF"
			" -D EMBREE_RAY_MASK=ON"
			" -D EMBREE_FILTER_FUNCTION=ON"
			" -D EMBREE_BACKFACE_CULLING=OFF"
			" -D EMBREE_TASKING_SYSTEM=TBB"
			" -D EMBREE_TBB_ROOT={gafferRoot}"
			" -D EMBREE_MAX_ISA=AVX2"
			" ..",
		"cd gafferBuild && cmake --build . --config {cmakeBuildType} --target install -- -j {jobs}",

	],

	"libLinkType" : {

		"static" : "EMBREE_STATIC_LIB=ON",
		"shared" : "EMBREE_STATIC_LIB=OFF",
		"default" : "EMBREE_STATIC_LIB=ON",

	},

}
