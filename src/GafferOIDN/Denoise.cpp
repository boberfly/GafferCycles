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

#include "boost/algorithm/string.hpp"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "OpenImageDenoise/oidn.hpp"

using namespace Imath;
using namespace Gaffer;
using namespace GafferImage;
using namespace GafferOIDN;

namespace
{

// Channels to look for
static const char *redNames[] = { "r", "R", "red", nullptr };
static const char *greenNames[] = { "g", "G", "green", nullptr };
static const char *blueNames[] = { "b", "B", "blue", nullptr };

// Default pass names
IECore::InternedString g_rgbPassName( "[RGB]" );
IECore::InternedString g_albedoPassName( "albedo" );
IECore::InternedString g_normalPassName( "normal" );
// Default filter name (OIDN only has one for now)
IECore::InternedString g_defaultFilterName( "RT" );

const std::string findChannelName( const IECoreImage::ImagePrimitive *image, const char **names, const char *passName )
{
	const IECoreImage::ImagePrimitive::ChannelMap &channels = image->channels;
	std::string name;
	if( passName && passName != g_rgbPassName.c_str() )
	{
		name = std::string( passName ) + ".";
	}

	while( *names != nullptr )
	{
		std::string fullName = name + std::string( *names );
		const auto it = channels.find( fullName );
		if( it != channels.end() )
		{
			return fullName;
		}
		names++;
	}
	return "";
}

bool interleave( const IECoreImage::ImagePrimitive *image, IECore::FloatVectorDataPtr outData, const char *passName = nullptr )
{
	const std::string redName = findChannelName( image, redNames, passName );
	const std::string greenName = findChannelName( image, greenNames, passName );
	const std::string blueName = findChannelName( image, blueNames, passName );
	const IECore::FloatVectorData *rd = image->getChannel<float>( redName );
	const IECore::FloatVectorData *gd = image->getChannel<float>( greenName );
	const IECore::FloatVectorData *bd = image->getChannel<float>( blueName );

	if( !(rd && gd && bd) )
	{
		return false;
	}

	int width = image->getDataWindow().size().x + 1;
	int height = image->getDataWindow().size().y + 1;

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

bool deinterleave( IECoreImage::ImagePrimitive *image, const IECore::FloatVectorDataPtr data, const char *passName = nullptr )
{
	const std::string redName = findChannelName( image, redNames, passName );
	const std::string greenName = findChannelName( image, greenNames, passName );
	const std::string blueName = findChannelName( image, blueNames, passName );
	IECore::FloatVectorData *rd = image->getChannel<float>( redName );
	IECore::FloatVectorData *gd = image->getChannel<float>( greenName );
	IECore::FloatVectorData *bd = image->getChannel<float>( blueName );

	if( !(rd && gd && bd) )
	{
		return false;
	}

	int width = image->getDataWindow().size().x + 1;
	int height = image->getDataWindow().size().y + 1;

	std::vector<float> &r = rd->writable();
	std::vector<float> &g = gd->writable();
	std::vector<float> &b = bd->writable();

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
	
	addChild( new IntPlug( "deviceType", Gaffer::Plug::In, 0 ) ); // OIDN_DEVICE_TYPE_DEFAULT
	addChild( new StringPlug( "filterType", Gaffer::Plug::In, g_defaultFilterName ) );

	addChild( new StringPlug( "channels", Gaffer::Plug::In, g_rgbPassName ) );
	addChild( new StringPlug( "albedo", Gaffer::Plug::In, g_albedoPassName ) );
	addChild( new StringPlug( "normal", Gaffer::Plug::In, g_normalPassName ) );

	// We don't ever want to change these, so we make pass-through connections.
	outPlug()->formatPlug()->setInput( inPlug()->formatPlug() );
	outPlug()->dataWindowPlug()->setInput( inPlug()->dataWindowPlug() );
	outPlug()->metadataPlug()->setInput( inPlug()->metadataPlug() );
	outPlug()->channelNamesPlug()->setInput( inPlug()->channelNamesPlug() );
}

Denoise::~Denoise()
{
}

Gaffer::IntPlug *Denoise::deviceTypePlug()
{
	return getChild<IntPlug>( g_firstPlugIndex );
}

const Gaffer::IntPlug *Denoise::deviceTypePlug() const
{
	return getChild<IntPlug>( g_firstPlugIndex );
}

Gaffer::StringPlug *Denoise::filterTypePlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::StringPlug *Denoise::filterTypePlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 1 );
}

Gaffer::StringPlug *Denoise::channelsPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

const Gaffer::StringPlug *Denoise::channelsPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 2 );
}

Gaffer::StringPlug *Denoise::albedoPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 3 );
}

const Gaffer::StringPlug *Denoise::albedoPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 3 );
}

Gaffer::StringPlug *Denoise::normalPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex + 4 );
}

const Gaffer::StringPlug *Denoise::normalPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex + 4 );
}

void Denoise::affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const
{
	ImageProcessor::affects( input, outputs );

	if( input == inPlug()->channelDataPlug() ||
		input == deviceTypePlug() ||
		input == filterTypePlug() ||
		input == channelsPlug() ||
		input == albedoPlug() ||
		input == normalPlug() )
	{
		outputs.push_back( outPlug()->channelDataPlug() );
	}
}

void Denoise::hash( const ValuePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const
{
	ImageProcessor::hash( output, context, h );

	inPlug()->channelDataPlug()->hash( h );

	deviceTypePlug()->hash( h );
	filterTypePlug()->hash( h );

	channelsPlug()->hash( h );
	albedoPlug()->hash( h );
	normalPlug()->hash( h );
}

void Denoise::compute( ValuePlug *output, const Context *context ) const
{
	if( output->parent<ValuePlug>() == channelsPlug() )
	{
		oidn::DeviceRef device = oidn::newDevice( static_cast<oidn::DeviceType>( deviceTypePlug()->getValue() ) );
		if( device )
		{
			device.commit();

			int width = inPlug()->format().width();
			int height = inPlug()->format().height();
			int size = width * height * 3; // 3-channel
			auto image = inPlug()->image();

			IECore::FloatVectorDataPtr albedoData = new IECore::FloatVectorData();
			IECore::FloatVectorDataPtr normalData = new IECore::FloatVectorData();

			bool hasAlbedo = false;
			bool hasNormal = false;

			if( albedoPlug() )
			{
				hasAlbedo = interleave( image.get(), albedoData, albedoPlug()->getValue().c_str() );
			}

			if( normalPlug() )
			{
				hasNormal = interleave( image.get(), normalData, normalPlug()->getValue().c_str() );
			}

			std::vector<std::string> passNames;
			std::string channels = channelsPlug()->getValue();
			boost::split( passNames, channels, boost::is_any_of( " " ) );

			for( auto passName : passNames )
			{
				IECore::FloatVectorDataPtr colorData = new IECore::FloatVectorData();
				IECore::FloatVectorDataPtr outputData = new IECore::FloatVectorData();

				if( interleave( image.get(), colorData, passName.c_str() ) )
				{
					if( !colorData )
					{
						return;
					}
					std::vector<float> &colorOutput = outputData->writable();
					colorOutput.resize( size );

					std::vector<float> &color = colorData->writable(); // readable
					std::vector<float> &albedo = albedoData->writable(); // readable
					std::vector<float> &normal = normalData->writable(); // readable

					oidn::FilterRef filter = device.newFilter( filterTypePlug()->getValue().c_str() );
					filter.setImage( "color", &color[0], oidn::Format::Float3, width, height );

					if( hasAlbedo )
						filter.setImage( "albedo", &albedo[0], oidn::Format::Float3, width, height );

					if( hasNormal )
						filter.setImage( "normal", &normal[0], oidn::Format::Float3, width, height );

					filter.setImage( "output", &colorOutput[0], oidn::Format::Float3, width, height );
					filter.set( "hdr", true );
					filter.commit();
					filter.execute();

					deinterleave( outPlug()->image().get(), outputData, passName.c_str() );
				}
			}
		}
	}

	ImageProcessor::compute( output, context );
}
