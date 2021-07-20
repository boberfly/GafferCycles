{

	"downloads" : [

		"http://github.com/zeux/pugixml/releases/download/v1.11/pugixml-1.11.tar.gz"

	],

	"url" : "https://pugixml.org",

	"license" : "LICENSE.md",

	"commands" : [

		"mkdir gafferBuild",
		"cd gafferBuild &&"
			" cmake"
			" -D CMAKE_INSTALL_PREFIX={buildDir}"
			" ..",
		"cd gafferBuild && make install -j {jobs} VERBOSE=1",

	],

	"platform:windows" : {

		"environment" : {

			"PATH" : "{buildDir}\\bin;%PATH%"

		},

		# using nmake instead of make causes an error "Makefile:35 missing separator. Stop."
		"commands" : [

			"mkdir gafferBuild",
			"cd gafferBuild &&"
				" cmake"
				" -Wno-dev -G {cmakeGenerator}"
				" -D CMAKE_CXX_STANDARD={c++Standard}"
				" -D CMAKE_BUILD_TYPE={cmakeBuildType}"
				" -D CMAKE_INSTALL_PREFIX={buildDir}"
				" ..",
			"cd gafferBuild && cmake --build . --config {cmakeBuildType} --target install -j {jobs}",

		],

	},

} 
