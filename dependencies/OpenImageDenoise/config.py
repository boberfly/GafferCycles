{

	"downloads" : [

		"https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.0/oidn-1.4.0.src.tar.gz"

	],

	"license" : "LICENSE.txt",

	"environment" : {

		"PATH" : "{buildDir}/bin:$PATH",
		"LD_LIBRARY_PATH" : "{buildDir}/lib:$LD_LIBRARY_PATH",

	},

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
		"default" : "OIDN_STATIC_LIB=ON",

	},

	"platform:linux" : {

		"environment" : {

			"LD_LIBRARY_PATH" : "{gafferRoot}/lib:$LD_LIBRARY_PATH",

		},

	},

	"platform:osx" : {

		"environment" : {

			"LD_LIBRARY_PATH" : "{gafferRoot}/lib:$LD_LIBRARY_PATH",

		},

	},

	"platform:windows" : {

		"environment" : {

			"PATH" : "{gafferRoot}/lib;%PATH",

		},

	},

}
