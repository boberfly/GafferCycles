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
//      * Neither the name of Image Engine Design nor the names of
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

#include "GafferOIDN/Denoise.h"

#include "GafferImage/ImageAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "OpenImageDenoise/oidn.hpp"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace Gaffer;
using namespace GafferImage;
using namespace GafferOIDN;

namespace
{

const IECore::InternedString g_layerNameKey( "oidn:denoise:__layerName" );

// Default layer names
IECore::InternedString g_rgbLayerName( "[RGB]" );
IECore::InternedString g_albedoLayerName( "albedo" );
IECore::InternedString g_normalLayerName( "normal" );
// Default filter name
IECore::InternedString g_RTFilterName( "RT" );
IECore::InternedString g_RTLightmapFilterName( "RTLightmap" );

bool interleave( 
	const IECore::FloatVectorData *rd, 
	const IECore::FloatVectorData *gd, 
	const IECore::FloatVectorData *bd,
	int width, int height, 
	IECore::FloatVectorDataPtr outData 
	)
{
	if( !(rd && gd && bd) )
	{
		return false;
	}

	const std::vector<float> &r = rd->readable();
	const std::vector<float> &g = gd->readable();
	const std::vector<float> &b = bd->readable();

	std::vector<float> &d = outData->writable();
	d.resize( width * height * 3 );

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		const float *rr = &r[y*width];
		const float *rg = &g[y*width];
		const float *rb = &b[y*width];

		for( unsigned int x=0; x<width; x++ )
		{
			d[i++] = rr[x];
			d[i++] = rg[x];
			d[i++] = rb[x];
		}
	}
	return true;
}

bool deinterleave( 
	IECore::FloatVectorData *rd, 
	IECore::FloatVectorData *gd, 
	IECore::FloatVectorData *bd,
	int width, int height, 
	const IECore::FloatVectorDataPtr data 
	)
{
	if( !(rd && gd && bd) )
	{
		return false;
	}

	std::vector<float> &r = rd->writable();
	std::vector<float> &g = gd->writable();
	std::vector<float> &b = bd->writable();

	r.resize( width * height );
	g.resize( width * height );
	b.resize( width * height );

	const std::vector<float> &d = data->readable();

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		float *rr = &r[y*width];
		float *rg = &g[y*width];
		float *rb = &b[y*width];

		for( int x=0; x<width; x++ )
		{
			rr[x] = d[i++];
			rg[x] = d[i++];
			rb[x] = d[i++];
		}
	}
	return true;
}

}

IE_CORE_DEFINERUNTIMETYPED( Denoise );

size_t Denoise::g_firstPlugIndex = 0;

Denoise::Denoise( const std::string &name )
	:   ImageProcessor( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );
	// Common
	addChild( new IntPlug( "verbose", Gaffer::Plug::In, 0 ) );
	addChild( new IntPlug( "numThreads", Gaffer::Plug::In, 0 ) );
	addChild( new BoolPlug( "setAffinity", Gaffer::Plug::In, true ) );
	addChild( new IntPlug( "deviceType", Gaffer::Plug::In, 0 ) ); // OIDN_DEVICE_TYPE_DEFAULT
	addChild( new StringPlug( "filterType", Gaffer::Plug::In, g_RTFilterName ) );

	addChild( new StringPlug( "channels", Gaffer::Plug::In, g_rgbLayerName ) );

	addChild(
		new ObjectPlug(
			"__colorData",
			Gaffer::Plug::Out,
			new ObjectVector
		)
	);
	// Filter-specific
	addChild( new StringPlug( "albedo", Gaffer::Plug::In, g_albedoLayerName ) );
	addChild( new StringPlug( "normal", Gaffer::Plug::In, g_normalLayerName ) );

	addChild( new BoolPlug( "hdr", Gaffer::Plug::In, true ) );
	addChild( new FloatPlug( "hdrScale", Gaffer::Plug::In, 0.0f ) );
	addChild( new BoolPlug( "srgb", Gaffer::Plug::In, false ) );
	addChild( new IntPlug( "maxMemoryMB", Gaffer::Plug::In, 6000 ) );

	// We don't ever want to change these, so we make pass-through connections.
	outPlug()->formatPlug()->setInput( inPlug()->formatPlug() );
	outPlug()->dataWindowPlug()->setInput( inPlug()->dataWindowPlug() );
	outPlug()->metadataPlug()->setInput( inPlug()->metadataPlug() );
	outPlug()->channelNamesPlug()->setInput( inPlug()->channelNamesPlug() );
}

Denoise::~Denoise()
{
}

Gaffer::IntPlug *Denoise::verbosePlug()
{
	return getChild<IntPlug>( g_firstPlugIndex );
}

const Gaffer::IntPlug *Denoise::verbosePlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex );
}

Gaffer::IntPlug *Denoise::numThreadsPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::IntPlug *Denoise::numThreadsPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 1 );
}

Gaffer::BoolPlug *Denoise::setAffinityPlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::BoolPlug *Denoise::setAffinityPlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 2 );
}

Gaffer::IntPlug *Denoise::deviceTypePlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::IntPlug *Denoise::deviceTypePlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 3 );
}

Gaffer::StringPlug *Denoise::filterTypePlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::StringPlug *Denoise::filterTypePlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 4 );
}

Gaffer::StringPlug *Denoise::channelsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 5 );
}

const Gaffer::StringPlug *Denoise::channelsPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 5 );
}

Gaffer::ObjectPlug *Denoise::colorDataPlug()
{
	return getChild<ObjectPlug>( g_firstPlugIndex + 6 );
}

const Gaffer::ObjectPlug *Denoise::colorDataPlug() const
{
	return getChild<ObjectPlug>( g_firstPlugIndex + 6 );
}

Gaffer::StringPlug *Denoise::albedoPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 7 );
}

const Gaffer::StringPlug *Denoise::albedoPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 7 );
}

Gaffer::StringPlug *Denoise::normalPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 8 );
}

const Gaffer::StringPlug *Denoise::normalPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 8 );
}

Gaffer::BoolPlug *Denoise::hdrPlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 9 );
}

const Gaffer::BoolPlug *Denoise::hdrPlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 9 );
}

Gaffer::FloatPlug *Denoise::hdrScalePlug()
{
	return getChild<FloatPlug>( g_firstPlugIndex + 10 );
}

const Gaffer::FloatPlug *Denoise::hdrScalePlug() const
{
	return getChild<FloatPlug>( g_firstPlugIndex + 10 );
}

Gaffer::BoolPlug *Denoise::srgbPlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 11 );
}

const Gaffer::BoolPlug *Denoise::srgbPlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 11 );
}

Gaffer::IntPlug *Denoise::maxMemoryMBPlug()
{
	return getChild<IntPlug>( g_firstPlugIndex + 12 );
}

const Gaffer::IntPlug *Denoise::maxMemoryMBPlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex + 12 );
}

void Denoise::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	ImageProcessor::affects( input, outputs );

	if( 
		input == inPlug()->channelDataPlug() ||
		input == inPlug()->channelNamesPlug() ||
		input == deviceTypePlug() ||
		input == filterTypePlug() ||
		input == channelsPlug() ||
		input == albedoPlug() ||
		input == normalPlug() ||
		input == hdrPlug() ||
		input == hdrScalePlug() ||
		input == srgbPlug() 
	)
	{
		outputs.push_back( colorDataPlug() );
	}
	else if( input == colorDataPlug() )
	{
		outputs.push_back( outPlug()->channelDataPlug() );
	}
}

void Denoise::hash( const ValuePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	ImageProcessor::hash( output, context, h );

	if( output == colorDataPlug() )
	{
		ConstStringVectorDataPtr channelNamesData;
		{
			ImagePlug::GlobalScope globalScope( context );
			channelNamesData = inPlug()->channelNamesPlug()->getValue();
		}
		const vector<string> &channelNames = channelNamesData->readable();

		const string &layerName = context->get<string>( g_layerNameKey );
		{
			ImagePlug::GlobalScope globalScope( context );
			int i = 0;
			for( const auto &baseName : { "R", "G", "B" } )
			{
				string channelName = ImageAlgo::channelName( layerName, baseName );
				if( ImageAlgo::channelExists( channelNames, channelName ) )
				{
					inPlug()->image()->getChannel<float>( channelName )->hash( h );
				}
			}
		}

		deviceTypePlug()->hash( h );
		filterTypePlug()->hash( h );

		channelsPlug()->hash( h );
		albedoPlug()->hash( h );
		normalPlug()->hash( h );

		hdrPlug()->hash( h );
		//hdrScalePlug()->hash( h );
		srgbPlug()->hash( h );

	}
}

void Denoise::compute( ValuePlug *output, const Context *context ) const
{
	if( output == colorDataPlug() )
	{
		oidn::DeviceRef device = oidn::newDevice( static_cast<oidn::DeviceType>( deviceTypePlug()->getValue() ) );
		if( device )
		{
			device.commit();
		}
		else
		{
			return;
		}

		InternedString filterType( filterTypePlug()->getValue() );

		ConstStringVectorDataPtr channelNamesData;
		{
			ImagePlug::GlobalScope globalScope( context );
			channelNamesData = inPlug()->channelNamesPlug()->getValue();
		}
		const vector<string> &channelNames = channelNamesData->readable();

		const string &layerName = context->get<string>( g_layerNameKey );

		IECore::FloatVectorDataPtr colorIn[3];
		{
			ImagePlug::GlobalScope globalScope( context );
			int i = 0;
			for( const auto &baseName : { "R", "G", "B" } )
			{
				string channelName = ImageAlgo::channelName( layerName, baseName );
				if( ImageAlgo::channelExists( channelNames, channelName ) )
				{
					colorIn[i] = inPlug()->image()->getChannel<float>( channelName );
				}
				i++;
			}
		}

		int width = inPlug()->format().width();
		int height = inPlug()->format().height();

		bool hasAlbedo = false;
		IECore::FloatVectorDataPtr albedoIn[3];
		IECore::FloatVectorDataPtr albedoData = new IECore::FloatVectorData();
		if( filterType == g_RTFilterName )
		{
			ImagePlug::GlobalScope globalScope( context );
			int i = 0;
			for( const auto &baseName : { "R", "G", "B" } )
			{
				string channelName = ImageAlgo::channelName( albedoPlug()->getValue(), baseName );
				if( ImageAlgo::channelExists( channelNames, channelName ) )
				{
					albedoIn[i] = inPlug()->image()->getChannel<float>( channelName );
				}
				i++;
			}
			hasAlbedo = interleave( albedoIn[0].get(), albedoIn[1].get(), albedoIn[2].get(), width, height, albedoData );
		}

		bool hasNormal = false;
		IECore::FloatVectorDataPtr normalIn[3];
		IECore::FloatVectorDataPtr normalData = new IECore::FloatVectorData();
		if( filterType == g_RTFilterName )
		{
			ImagePlug::GlobalScope globalScope( context );
			int i = 0;
			for( const auto &baseName : { "R", "G", "B" } )
			{
				string channelName = ImageAlgo::channelName( normalPlug()->getValue(), baseName );
				if( ImageAlgo::channelExists( channelNames, channelName ) )
				{
					normalIn[i] = inPlug()->image()->getChannel<float>( channelName );
				}
				i++;
			}
			hasNormal = interleave( normalIn[0].get(), normalIn[1].get(), normalIn[2].get(), width, height, normalData );
		}

		IECore::FloatVectorDataPtr outputData = new IECore::FloatVectorData();
		IECore::FloatVectorDataPtr colorInData = new IECore::FloatVectorData();
		IECore::FloatVectorDataPtr colorOut[3] = new IECore::FloatVectorData();

		if( interleave( colorIn[0].get(), colorIn[1].get(), colorIn[2].get(), width, height, colorInData ) )
		{
			std::vector<float> &output = outputData->writable();
			output.resize( width * height * 3 );

			std::vector<float> &color = colorInData->writable(); // readable, but oidn wants a non-const ptr
			std::vector<float> &albedo = albedoData->writable(); // readable, but oidn wants a non-const ptr
			std::vector<float> &normal = normalData->writable(); // readable, but oidn wants a non-const ptr

			oidn::FilterRef filter = device.newFilter( filterTypePlug()->getValue().c_str() );

			filter.setImage( "color", &color[0], oidn::Format::Float3, width, height );

			if( hasAlbedo )
				filter.setImage( "albedo", &albedo[0], oidn::Format::Float3, width, height );

			if( hasNormal )
				filter.setImage( "normal", &normal[0], oidn::Format::Float3, width, height );

			filter.setImage( "output", &output[0], oidn::Format::Float3, width, height );

			if( filterType == g_RTFilterName )
			{
				filter.set( "hdr", hdrPlug()->getValue() );
				filter.set( "srgb", srgbPlug()->getValue() );
			}

			// OIDN 1.1.0
			//float hdrScale = hdrScalePlug()->getValue();
			//if( hdrScale > 0.0f )
			//filter.set( "hdrScale", hdrScale );
			filter.set( "maxMemoryMB", maxMemoryMBPlug()->getValue() );

			filter.commit();
			filter.execute();

			const char* errorMessage;
			if( device.getError( errorMessage ) != oidn::Error::None )
				IECore::msg( IECore::Msg::Error, "GafferOIDN::Denoise", boost::format( "%s" ) % errorMessage );

			deinterleave( colorOut[0].get(), colorOut[1].get(), colorOut[2].get(), width, height, outputData );
		}

		IECore::ObjectVectorPtr result = new IECore::ObjectVector();
		result->members().push_back( colorOut[0] );
		result->members().push_back( colorOut[1] );
		result->members().push_back( colorOut[2] );

		static_cast<ObjectPlug *>( output )->setValue( result );
		return;
	}

	ImageProcessor::compute( output, context );

}

Gaffer::ValuePlug::CachePolicy Denoise::computeCachePolicy( const Gaffer::ValuePlug *output ) const
{
	if( output == outPlug()->channelDataPlug() )
	{
		// Because our implementation of computeChannelData() is so simple,
		// just copying data out of our intermediate colorDataPlug(), it is
		// actually quicker not to cache the result.
		return ValuePlug::CachePolicy::Uncached;
	}
	else if( output == colorDataPlug() )
	{
		// This is so when we generate colorData from OpenImageDenoise,
		// it runs on a single-thread and the library will multi-thread
		// internally using TBB.
		return ValuePlug::CachePolicy::TaskCollaboration;
	}
	return ImageProcessor::computeCachePolicy( output );
}

void Denoise::hashChannelData( const GafferImage::ImagePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	const std::string &channels = channelsPlug()->getValue();
	const std::string &channel = context->get<std::string>( ImagePlug::channelNameContextName );
	const std::string &baseName = ImageAlgo::baseName( channel );

	if(
		( baseName != "R" && baseName != "G" && baseName != "B" ) ||
		!StringAlgo::matchMultiple( channel, channels )
	)
	{
		// Auxiliary channel, or not in channel mask. Pass through.
		h = inPlug()->channelDataPlug()->hash();
		return;
	}

	ImageProcessor::hashChannelData( output, context, h );
	h.append( baseName );
	{
		Context::EditableScope layerScope( context );
		layerScope.set( g_layerNameKey, ImageAlgo::layerName( channel ) );
		colorDataPlug()->hash( h );
	}
}

IECore::ConstFloatVectorDataPtr Denoise::computeChannelData( const std::string &channelName, const Imath::V2i &tileOrigin, const Gaffer::Context *context, const ImagePlug *parent ) const
{
	const std::string &channels = channelsPlug()->getValue();
	const std::string &channel = context->get<std::string>( ImagePlug::channelNameContextName );
	const std::string &baseName = ImageAlgo::baseName( channel );

	if(
		( baseName != "R" && baseName != "G" && baseName != "B" ) ||
		!StringAlgo::matchMultiple( channel, channels )
	)
	{
		// Auxiliary channel, or not in channel mask. Pass through.
		return inPlug()->channelDataPlug()->getValue();
	}

	ConstObjectVectorPtr colorData;
	{
		Context::EditableScope layerScope( context );
		layerScope.set( g_layerNameKey, ImageAlgo::layerName( channel ) );
		colorData = boost::static_pointer_cast<const ObjectVector>( colorDataPlug()->getValue() );
	}
	return boost::static_pointer_cast<const FloatVectorData>( colorData->members()[ImageAlgo::colorIndex( baseName )] );
}
