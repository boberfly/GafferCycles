//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Alex Fuller. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
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

#include "GafferCycles/IECoreCyclesPreview/CameraAlgo.h"
#include "GafferCycles/IECoreCyclesPreview/ObjectAlgo.h"
#include "GafferCycles/IECoreCyclesPreview/SocketAlgo.h"

#include "IECoreScene/Camera.h"

#include "IECore/SimpleTypedData.h"

// Cycles
#include "render/camera.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreCycles;

namespace
{

ccl::Camera *convertCommon( const IECoreScene::Camera *camera, const std::string &nodeName, int frame )
{
	assert( camera->typeId() == IECoreScene::Camera::staticTypeId() );
	ccl::Camera *ccam = new ccl::Camera();
	ccam->name = ccl::ustring(nodeName.c_str());

	// Projection type
	const string &projection = camera->getProjection();
	if( projection == "perspective" )
	{
		ccam->type = ccl::CAMERA_PERSPECTIVE;
		ccam->fov = M_PI_2;
		if( camera->getFStop() > 0.0f )
		{
			ccam->aperturesize = 0.5f * camera->getFocalLength() * camera->getFocalLengthWorldScale() / camera->getFStop();
			ccam->focaldistance = camera->getFocusDistance();
		}
	}
	else if( projection == "orthographic" )
	{
		ccam->type = ccl::CAMERA_ORTHOGRAPHIC;
	}
	else if( projection == "panorama" )
	{
		ccam->type = ccl::CAMERA_PANORAMA;
		// TODO: Spec out panorama data
	}
	else
	{
		ccam->type = ccl::CAMERA_PERSPECTIVE;
		ccam->fov = M_PI_2;
	}

	// Screen window/resolution TODO: full_ might be something to do with cropping?
	const Imath::Box2f &frustum = camera->frustum();
	const Imath::V2i &resolution = camera->renderResolution();
	const float pixelAspectRatio = camera->getPixelAspectRatio();
	ccam->width = resolution[0];
	ccam->height = resolution[1];
	ccam->full_width = resolution[0];
	ccam->full_height = resolution[1];
	ccam->viewplane.left = frustum.min.x;
	ccam->viewplane.right = frustum.max.x;
	// Invert the viewplane in Y so Gaffer's aperture offsets and overscan are applied in the correct direction
	ccam->viewplane.bottom = -frustum.max.y;
	ccam->viewplane.top = -frustum.min.y;
	ccam->aperture_ratio = pixelAspectRatio; // This is more for the bokeh, maybe it should be a separate parameter?

	// Clipping planes
	const Imath::V2f &clippingPlanes = camera->getClippingPlanes();
	ccam->nearclip = clippingPlanes.x;
	ccam->farclip = clippingPlanes.y;

	// Crop window
	if ( camera->hasCropWindow() )
	{
		const Imath::Box2f &cropWindow = camera->getCropWindow();
		ccam->border.left = cropWindow.min.x;
		ccam->border.right = cropWindow.max.x;
		ccam->border.top = cropWindow.max.y;
		ccam->border.bottom = cropWindow.min.y;
		ccam->border.clamp();
	}
	
	// Shutter TODO: Cycles also has a shutter curve...
	const Imath::V2f &shutter = camera->getShutter();
	ccam->shuttertime = abs( shutter.y - shutter.x );

	// Set the correct motion position.
	const Imath::V2f relativeShutter = shutter - Imath::V2f( frame );
	if ( ( relativeShutter.x >= 0.0f ) && ( relativeShutter.y > 0.0f ) )
	{
		ccam->motion_position = ccl::Camera::MOTION_POSITION_START;
	}
	else if ( ( relativeShutter.x < 0.0f ) && ( relativeShutter.y <= 0.0f ) )
	{
		ccam->motion_position = ccl::Camera::MOTION_POSITION_END;
	}
	else
	{
		ccam->motion_position = ccl::Camera::MOTION_POSITION_CENTER;
	}

	return ccam;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API
//////////////////////////////////////////////////////////////////////////

namespace IECoreCycles

{

namespace CameraAlgo

{

ccl::Camera *convert( const IECoreScene::Camera *camera, const std::string &nodeName, int frame )
{
	return convertCommon( camera, nodeName, frame );
}

} // namespace CameraAlgo

} // namespace IECoreCycles
