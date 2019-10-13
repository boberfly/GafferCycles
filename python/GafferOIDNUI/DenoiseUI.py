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

		"deviceType" : [

			"description",
			"""
			The hardware device to use for OpenImageDenoise. Default uses
			the approximately fastest device. CPU requires SSE4.1 support.
			""",

			"preset:Default", 0,
			"preset:CPU", 1,

			"plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",

		],

		"filterType" : [

			"description",
			"""
			The filter type to use.

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
			""",

			"preset:RT", "RT",

			"plugValueWidget:type", "GafferUI.PresetsPlugValueWidget",

		],

		"channels" : [

			"description",
			"""
			The channels/passes to denoise. Must be a 3-channel pass. Default beauty is
			[RGB] but you can specify multiple ones here.
			"""

		],

		"albedo" : [

			"description",
			"""
			The albedo pass to use (optional).
			"""

		],

		"normal" : [

			"description",
			"""
			The normal pass to use (optional).
			"""

		],

	}

)
