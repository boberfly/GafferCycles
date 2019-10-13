{

	"downloads" : [

		"https://github.com/OpenImageDenoise/oidn/releases/download/v1.0.0/oidn-1.0.0.src.tar.gz"

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
			" ..",
		"cd gafferBuild && cmake --build . --config {cmakeBuildType} --target install -- -j {jobs}",

	],

	"libLinkType" : {

		"static" : "OIDN_STATIC_LIB=ON",
		"shared" : "OIDN_STATIC_LIB=OFF",
		"default" : "OIDN_STATIC_LIB=OFF",

	},

}
