##########################################################################
#
#  Copyright (c) 2019, Alex Fuller. All rights reserved.
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

import Gaffer
import GafferUI
import GafferOIDN

# Command suitable for use with `NodeMenu.append()`.
def nodeMenuCreateCommand( menu ) :

	denoise = GafferOIDN.Denoise()

	return denoise

Gaffer.Metadata.registerNode(

	GafferOIDN.Denoise,

	"description",
	"""
	Applies Intel's OpenImageDenoise to the specified channels.
	""",

	plugs = {

		"verbose" : [

			"description",
			"""
			Verbosity level of the console output between 0-3; when set to 0, no output is
			printed, when set to a higher level more output is printed.
			""",

			"preset:No Output", 0,
			"preset:1", 1,
			"preset:2", 2,
			"preset:3", 3,

			"plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",

			"layout:section", "Device",

		],

		"numThreads" : [

			"description",
			"""
			Maximum number of threads which the library should use; 0 will set it automatically 
			to get the best performance.
			""",

			"layout:section", "Device",

		],

		"setAffinity" : [

			"description",
			"""
			Bind software threads to hardware threads if set to true (improves performance); 
			false disables binding.
			""",

			"layout:section", "Device",

		],

		"deviceType" : [

			"description",
			"""
			The hardware device to use for OpenImageDenoise. Default uses
			the approximately fastest device. CPU requires SSE4.1 support.
			""",

			"preset:Default", 0,
			"preset:CPU", 1,

			"plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",

			"layout:section", "Device",

		],

		"filterType" : [

			"description",
			"""
			The filter type to use.

			RT
			The RT (ray tracing) filter is a generic ray tracing denoising filter which 
			is suitable for denoising images rendered with Monte Carlo ray tracing methods 
			like unidirectional and bidirectional path tracing. It supports depth of field 
			and motion blur as well, but it is not temporally stable. The filter is based 
			on a deep learning based denoising algorithm, and it aims to provide a good 
			balance between denoising performance and quality for a wide range of samples 
			per pixel.

			It accepts either a low dynamic range (LDR) or high dynamic range (HDR) color 
			image as input. Optionally, it also accepts auxiliary feature images, e.g. 
			albedo and normal, which improve the denoising quality, preserving more details 
			in the image.

			The RT filter has certain limitations regarding the supported input images. Most 
			notably, it cannot denoise images that were not rendered with ray tracing. 
			Another important limitation is related to anti-aliasing filters. Most renderers 
			use a high-quality pixel reconstruction filter instead of a trivial box filter 
			to minimize aliasing artifacts (e.g. Gaussian, Blackman-Harris). The RT filter 
			does support such pixel filters but only if implemented with importance sampling. 
			Weighted pixel sampling (sometimes called splatting) introduces correlation 
			between neighboring pixels, which causes the denoising to fail (the noise will 
			not be filtered), thus it is not supported.

			RTLightmap
			The RTLightmap filter is a variant of the RT filter optimized for denoising HDR 
			lightmaps. It does not support LDR images.
			""",

			"preset:RT", "RT",
			"preset:RTLightmap", "RTLightmap",

			"plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",

		],

		"channels" : [

			"description",
			"""
			The channels/passes to denoise. Must be a 3-channel pass. Default beauty is
			[RGB] but you can specify multiple ones here.
			"""

		],

		"hdr" : [

			"description",
			"""
			Whether the color is HDR. Always on for RTLightmap.
			""",

		],

		"hdrScale" : [

			"description",
			"""
			HDR color values are interpreted such that, multiplied by this scale, a value 
			of 1 corresponds to a luminance level of 100 cd/m^2 (this affects the quality 
			of the output but the output color values will not be scaled); if set to NaN, 
			the scale is computed automatically (default).
			""",

		],

		"maxMemoryMB" : [

			"description",
			"""
			Approximate maximum amount of scratch memory to use in megabytes (actual 
			memory usage may be higher); limiting memory usage may cause slower denoising 
			due to internally splitting the image into overlapping tiles, but cannot cause 
			the denoising to fail.
			"""

		],

		"albedo" : [

			"description",
			"""
			The albedo pass to use (optional).
			""",

			"layout:section", "RT",

		],

		"normal" : [

			"description",
			"""
			The normal pass to use (optional).
			""",

			"layout:section", "RT",

		],

	}

)
