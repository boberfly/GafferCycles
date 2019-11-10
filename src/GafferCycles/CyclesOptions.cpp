//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Alex Fuller. All rights reserved.
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
//      * Neither the name of John Haddon nor the names of
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

#include "GafferCycles/CyclesOptions.h"
#include "boost/algorithm/string.hpp"

// Cycles
#include "device/device.h"

using namespace Imath;
using namespace GafferCycles;

IE_CORE_DEFINERUNTIMETYPED( CyclesOptions );

CyclesOptions::CyclesOptions( const std::string &name )
	:	GafferScene::Options( name )
{
	Gaffer::CompoundDataPlug *options = optionsPlug();

	// Log
	options->addChild( new Gaffer::NameValuePlug( "ccl:log_level", new IECore::IntData( 0 ), false, "logLevel" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:progress_level", new IECore::IntData( 2 ), false, "progressLevel" ) );

	// Device

	options->addChild( new Gaffer::NameValuePlug( "ccl:device", new IECore::StringData( "CPU" ), false, "device" ) );

	// Session and scene
	options->addChild( new Gaffer::NameValuePlug( "ccl:shadingsystem", new IECore::StringData( "SVM" ), false, "shadingSystem" ) );

	// Session/Render

	options->addChild( new Gaffer::NameValuePlug( "ccl:session:experimental", new IECore::BoolData( false ), false, "featureSet" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:session:progressive_refine", new IECore::BoolData( false ), false, "progressiveRefine" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:progressive", new IECore::BoolData( false ), false, "method" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:samples", new IECore::IntData( 8 ), false, "samples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:tile_size", new IECore::V2iData( Imath::V2i( 64, 64 ) ), false, "tileSize" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:tile_order", new IECore::IntData( 0 ), false, "tileOrder" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:start_resolution", new IECore::IntData( 64 ), false, "startResolution" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:pixel_size", new IECore::IntData( 64 ), false, "pixelSize" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:threads", new IECore::IntData( 0 ), false, "numThreads" ) );
	//options->addChild( new Gaffer::NameValuePlug( "ccl:session:display_buffer_linear", new IECore::BoolData( true ), false, "displayBufferLinear" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:run_denoising", new IECore::BoolData( false ), false, "runDenoising" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:write_denoising_passes", new IECore::BoolData( false ), false, "writeDenoisingPasses" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:full_denoising", new IECore::BoolData( false ), false, "fullDenoising" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:cancel_timeout", new IECore::FloatData( 0.1f ), false, "cancelTimeout" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:reset_timeout", new IECore::FloatData( 0.1f ), false, "resetTimeout" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:text_timeout", new IECore::FloatData( 1.0f ), false, "textTimeout" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:session:progressive_update_timeout", new IECore::FloatData( 1.0f ), false, "progressiveUpdateTimeout" ) );

	// Denoising
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoise:radius", new IECore::IntData( 8 ), false, "denoiseRadius" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoise:strength", new IECore::FloatData( 0.5f ), false, "denoiseStrength" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoise:feature_strength", new IECore::FloatData( 0.5f ), false, "denoiseFeatureStrength" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoise:relative_pca", new IECore::BoolData( false ), false, "denoiseRelativePca" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoise:neighbor_frames", new IECore::IntData( 2 ), false, "denoiseNeighborFrames" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoise:clamp_input", new IECore::BoolData( true ), false, "denoiseClampInput" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_diffuse_direct",        new IECore::BoolData( true ), false, "denoisingDiffuseDirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_diffuse_indirect",      new IECore::BoolData( true ), false, "denoisingDiffuseIndirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_glossy_direct",         new IECore::BoolData( true ), false, "denoisingGlossyDirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_glossy_indirect",       new IECore::BoolData( true ), false, "denoisingGlossyIndirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_transmission_direct",   new IECore::BoolData( true ), false, "denoisingTransmissionDirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_transmission_indirect", new IECore::BoolData( true ), false, "denoisingTransmissionIndirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_subsurface_direct",     new IECore::BoolData( true ), false, "denoisingSubsurfaceDirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:denoising_subsurface_indirect",   new IECore::BoolData( true ), false, "denoisingSubsurfaceIndirect" ) );

	// Scene/BVH

	//options->addChild( new Gaffer::NameValuePlug( "ccl:scene:bvh_type", new IECore::IntData( 0 ), false, "bvhType" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:scene:bvh_layout", new IECore::IntData( 0 | 1 << 1 ), false, "bvhLayout" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:scene:use_bvh_spatial_split", new IECore::BoolData( false ), false, "useBvhSpatialSplit" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:scene:use_bvh_unaligned_nodes", new IECore::BoolData( true ), false, "useBvhUnalignedNodes" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:scene:num_bvh_time_steps", new IECore::IntData( 0 ), false, "numBvhTimeSteps" ) );

	//options->addChild( new Gaffer::NameValuePlug( "ccl:scene:persistent_data", new IECore::BoolData( true ), false, "persistentData" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:scene:texture_limit", new IECore::IntData( 0 ), false, "textureLimit" ) );

	// Integrator

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:max_bounce", new IECore::IntData( 7 ), false, "maxBounce" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:max_diffuse_bounce", new IECore::IntData( 7 ), false, "maxDiffuseBounce" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:max_glossy_bounce", new IECore::IntData( 7 ), false, "maxGlossyBounce" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:max_transmission_bounce", new IECore::IntData( 7 ), false, "maxTransmissionBounce" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:max_volume_bounce", new IECore::IntData( 7 ), false, "maxVolumeBounce" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:transparent_max_bounce", new IECore::IntData( 7 ), false, "transparentMaxBounce" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:ao_bounces", new IECore::IntData( 0 ), false, "aoBounces" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:volume_max_steps", new IECore::IntData( 1024 ), false, "volumeMaxSteps" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:volume_step_size", new IECore::FloatData( 0.1f ), false, "volumeStepSize" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:caustics_reflective", new IECore::BoolData( true ), false, "causticsReflective" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:caustics_refractive", new IECore::BoolData( true ), false, "causticsRefractive" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:filter_glossy", new IECore::FloatData( 0.0f ), false, "filterGlossy" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:seed", new IECore::IntData( 0 ), false, "seed" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:sample_clamp_direct", new IECore::FloatData( 0.0f ), false, "sampleClampDirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:sample_clamp_indirect", new IECore::FloatData( 0.0f ), false, "sampleClampIndirect" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:aa_samples", new IECore::IntData( 0 ), false, "aaSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:diffuse_samples", new IECore::IntData( 1 ), false, "diffuseSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:glossy_samples", new IECore::IntData( 1 ), false, "glossySamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:transmission_samples", new IECore::IntData( 1 ), false, "transmissionSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:ao_samples", new IECore::IntData( 1 ), false, "aoSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:mesh_light_samples", new IECore::IntData( 1 ), false, "meshlightSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:subsurface_samples", new IECore::IntData( 1 ), false, "subsurfaceSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:volume_samples", new IECore::IntData( 1 ), false, "volumeSamples" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:start_sample", new IECore::IntData( 0 ), false, "startSample" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:sample_all_lights_direct", new IECore::BoolData( true ), false, "sampleAllLightsDirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:sample_all_lights_indirect", new IECore::BoolData( true ), false, "sampleAllLightsIndirect" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:light_sampling_threshold", new IECore::FloatData( 0.05f ), false, "lightSamplingThreshold" ) );

	options->addChild( new Gaffer::NameValuePlug( "ccl:integrator:sampling_pattern", new IECore::IntData( 0 ), false, "samplingPattern" ) );

	// Curves
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:primitive", new IECore::IntData( 0 ), false, "curvePrimitive" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:shape", new IECore::IntData( 0 ), false, "curveShape" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:line_method", new IECore::IntData( 0 ), false, "curveLineMethod" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:triangle_method", new IECore::IntData( 0 ), false, "curveTriangleMethod" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:resolution", new IECore::IntData( 0 ), false, "curveResolution" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:subdivisions", new IECore::IntData( 0 ), false, "curveSubdivisions" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:use_curves", new IECore::BoolData( false ), false, "useCurves" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:use_encasing", new IECore::BoolData( false ), false, "curveUseEncasing" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:use_backfacing", new IECore::BoolData( false ), false, "curveUseBackfacing" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:curve:use_tangent_normal_geometry", new IECore::BoolData( false ), false, "curveUseTangentNormalGeo" ) );
	// Background
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:ao_factor", new IECore::FloatData( 0.0f ), false, "aoFactor" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:ao_distance", new IECore::FloatData( FLT_MAX ), false, "aoDistance" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:use_shader", new IECore::BoolData( true ), false, "bgUseShader" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:use_ao", new IECore::BoolData( false ), false, "useAO" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:transparent", new IECore::BoolData( true ), false, "bgTransparent" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:transparent_glass", new IECore::BoolData( false ), false, "bgTransparentGlass" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:transparent_roughness_threshold", new IECore::FloatData( 0.0f ), false, "bgTransparentRoughnessThreshold" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:visibility:camera", new IECore::BoolData( true ), false, "bgCameraVisibility" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:visibility:diffuse", new IECore::BoolData( true ), false, "bgDiffuseVisibility" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:visibility:glossy", new IECore::BoolData( true ), false, "bgGlossyVisibility" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:visibility:transmission", new IECore::BoolData( true ), false, "bgTransmissionVisibility" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:visibility:shadow", new IECore::BoolData( true ), false, "bgShadowVisibility" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:background:visibility:scatter", new IECore::BoolData( true ), false, "bgScatterVisibility" ) );

	// Film
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:exposure", new IECore::FloatData( 0.8f ), false, "exposure" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:pass_alpha_threshold", new IECore::FloatData( 0.5f ), false, "passAlphaThreshold" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:filter_type", new IECore::IntData( 0 ), false, "filterType" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:filter_width", new IECore::FloatData( 1.0f ), false, "filterWidth" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:mist_start", new IECore::FloatData( 0.0f ), false, "mistStart" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:mist_depth", new IECore::FloatData( 100.0f ), false, "mistDepth" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:mist_falloff", new IECore::FloatData( 1.0f ), false, "mistFalloff" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:use_sample_clamp", new IECore::BoolData( false ), false, "useSampleClamp" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:denoising_data_pass", new IECore::BoolData( false ), false, "denoisingDataPass" ) );
	options->addChild( new Gaffer::NameValuePlug( "ccl:film:denoising_clean_pass", new IECore::BoolData( false ), false, "denoisingCleanPass" ) );

	// Multi-Device
	ccl::vector<ccl::DeviceInfo> devices = ccl::Device::available_devices( ccl::DEVICE_MASK_CPU | ccl::DEVICE_MASK_OPENCL | ccl::DEVICE_MASK_CUDA
#ifdef WITH_OPTIX
		| ccl::DEVICE_MASK_OPTIX
#endif
	 );
	int indexCuda = 0;
	int indexOpenCL = 0;
	int indexOptiX = 0;
	for( const ccl::DeviceInfo &device : devices ) 
	{
		if( device.type == ccl::DEVICE_CPU )
		{
			options->addChild( new Gaffer::NameValuePlug( "ccl:multidevice:CPU", new IECore::BoolData( true ), false, "multideviceCPU" ) );
			continue;
		}
		if( device.type == ccl::DEVICE_CUDA )
		{
			auto internalName = boost::format( "ccl:multidevice:CUDA%02i" ) % indexCuda;
			auto optionName = boost::format( "multideviceCUDA%02i" ) % indexCuda;
			options->addChild( new Gaffer::NameValuePlug( internalName.str(), new IECore::BoolData( false ), false, optionName.str() ) );
			++indexCuda;
			continue;
		}
		if( device.type == ccl::DEVICE_OPENCL )
		{
			auto internalName = boost::format( "ccl:multidevice:OPENCL%02i" ) % indexOpenCL;
			auto optionName = boost::format( "multideviceOPENCL%02i" ) % indexOpenCL;
			options->addChild( new Gaffer::NameValuePlug( internalName.str(), new IECore::BoolData( false ), false, optionName.str() ) );
			++indexOpenCL;
			continue;
		}
#ifdef WITH_OPTIX
		if( device.type == ccl::DEVICE_OPTIX )
		{
			auto internalName = boost::format( "ccl:multidevice:OPTIX%02i" ) % indexOptiX;
			auto optionName = boost::format( "multideviceOPTIX%02i" ) % indexOptiX;
			options->addChild( new Gaffer::NameValuePlug( internalName.str(), new IECore::BoolData( false ), false, optionName.str() ) );
			++indexOptiX;
			continue;
		}
#endif
	}

	// Dicing camera
	options->addChild( new Gaffer::NameValuePlug( "ccl:dicing_camera", new IECore::StringData(), false, "dicingCamera" ) );
}

CyclesOptions::~CyclesOptions()
{
}
