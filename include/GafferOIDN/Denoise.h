//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2019, Alex Fuller. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of Alex Fuller nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#ifndef GAFFEROIDN_DENOISE_H
#define GAFFEROIDN_DENOISE_H

#include "GafferOIDN/Export.h"
#include "GafferOIDN/TypeIds.h"

#include "Gaffer/PlugType.h"

#include "GafferImage/ImageProcessor.h"

namespace GafferOIDN
{

IE_CORE_FORWARDDECLARE( Denoise )

class GAFFEROIDN_API Denoise : public GafferImage::ImageProcessor
{
	public :

		Denoise( const std::string &name=defaultName<Denoise>() );
		~Denoise() override;

		GAFFER_GRAPHCOMPONENT_DECLARE_TYPE( GafferOIDN::Denoise, GafferOIDN::DenoiseTypeId, GafferImage::ImageProcessor );

		Gaffer::IntPlug *verbosePlug();
		const Gaffer::IntPlug *verbosePlug() const;

		Gaffer::IntPlug *numThreadsPlug();
		const Gaffer::IntPlug *numThreadsPlug() const;

		Gaffer::BoolPlug *setAffinityPlug();
		const Gaffer::BoolPlug *setAffinityPlug() const;

		Gaffer::IntPlug *deviceTypePlug();
		const Gaffer::IntPlug *deviceTypePlug() const;
		
		Gaffer::StringPlug *filterTypePlug();
		const Gaffer::StringPlug *filterTypePlug() const;

		Gaffer::StringPlug *channelsPlug();
		const Gaffer::StringPlug *channelsPlug() const;

		Gaffer::StringPlug *albedoPlug();
		const Gaffer::StringPlug *albedoPlug() const;

		Gaffer::StringPlug *normalPlug();
		const Gaffer::StringPlug *normalPlug() const;

		Gaffer::BoolPlug *hdrPlug();
		const Gaffer::BoolPlug *hdrPlug() const;

		Gaffer::FloatPlug *hdrScalePlug();
		const Gaffer::FloatPlug *hdrScalePlug() const;

		Gaffer::BoolPlug *srgbPlug();
		const Gaffer::BoolPlug *srgbPlug() const;

		Gaffer::IntPlug *maxMemoryMBPlug();
		const Gaffer::IntPlug *maxMemoryMBPlug() const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		void hash( const Gaffer::ValuePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		void hashChannelData( const GafferImage::ImagePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;

		/// Implemented to process the color data and stash the results on colorDataPlug()
		/// format, dataWindow, metadata, and channelNames are passed through via direct connection to the input values.
		void compute( Gaffer::ValuePlug *output, const Gaffer::Context *context ) const override;
		Gaffer::ValuePlug::CachePolicy computeCachePolicy( const Gaffer::ValuePlug *output ) const override;

		/// Implemented to use the results of colorDataPlug() via compute()
		IECore::ConstFloatVectorDataPtr computeChannelData( const std::string &channelName, const Imath::V2i &tileOrigin, const Gaffer::Context *context, const GafferImage::ImagePlug *parent ) const override;

	private :

		// Used to store the result of compute(), so that it can be reused in computeChannelData().
		// Evaluated in a context with an "image:colorProcessor:__layerName" variable, so we can cache
		// different results per layer.
		Gaffer::ObjectPlug *colorDataPlug();
		const Gaffer::ObjectPlug *colorDataPlug() const;

		static size_t g_firstPlugIndex;

};

IE_CORE_DECLAREPTR( Denoise )

} // namespace GafferOIDN

#endif // GAFFEROIDN_DENOISE_H
