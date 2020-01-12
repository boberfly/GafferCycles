{

	"downloads" : [

		"https://github.com/PixarAnimationStudios/OpenSubdiv/archive/v3_3_3.tar.gz"

	],

	"license" : "LICENSE.txt",

	"commands" : [

		"mkdir gafferBuild",
		"cd gafferBuild &&"
			" cmake"
			" -G {generator}"
			" -D CMAKE_CXX_COMPILER={cxxCompiler}"
			" -D CMAKE_INSTALL_PREFIX={buildDir}"
			" -D CMAKE_PREFIX_PATH={gafferRoot}"
			" -D CMAKE_BUILD_TYPE={cmakeBuildType}"
			" -D NO_EXAMPLES=1"
			" -D NO_TUTORIALS=1"
			" -D NO_REGRESSION=1"
			" -D NO_PTEX=1"
			" -D NO_DOC=1"
			" -D NO_OMP=1"
			" -D NO_CUDA=1"
			" -D NO_OPENCL=1"
			" -D NO_CLEW=1"
			" -D NO_METAL=1"
			" -D NO_DX=1"
			" -D NO_TESTS=1"
			" -D NO_GLTESTS=1"
			" -D NO_GLFW=1"
			" -D NO_GLFW_X11=1"
			" -D GLEW_LOCATION={gafferRoot}"
			" ..",
		"cd gafferBuild && cmake --build . --config {cmakeBuildType} --target install",

	],

	"platform:linux" : {

		"variables" : {

			"generator" : "\"Unix Makefiles\""

		},

	},

	"platform:osx" : {

		"variables" : {

			"generator" : "\"Unix Makefiles\""

		},

	},

	"platform:windows" : {

		"variables" : {

			"generator" : "\"Visual Studio 15 2017 Win64\""

		},

	},

}
