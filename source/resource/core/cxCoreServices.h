/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/
#ifndef CXCORESERVICES_H
#define CXCORESERVICES_H

#include "cxResourceExport.h"
#include <boost/shared_ptr.hpp>
class ctkPluginContext;

namespace cx
{

typedef boost::shared_ptr<class PatientModelService> PatientModelServicePtr;
typedef boost::shared_ptr<class TrackingService> TrackingServicePtr;
typedef boost::shared_ptr<class VideoService> VideoServicePtr;
typedef boost::shared_ptr<class SpaceProvider> SpaceProviderPtr;
typedef boost::shared_ptr<class CoreServices> CoreServicesPtr;
typedef boost::shared_ptr<class SessionStorageService> SessionStorageServicePtr;
typedef boost::shared_ptr<class StateService> StateServicePtr;

/**
 * Convenience class combining all services in resource/core.
 *
 * \ingroup cx_resource_core
 *
 * \date Nov 14 2014
 * \author Ole Vegard Solberg, SINTEF
 */
class cxResource_EXPORT CoreServices
{
public:
	static CoreServicesPtr create(ctkPluginContext* context);
	CoreServices(ctkPluginContext* context);
	static CoreServicesPtr getNullObjects();

	PatientModelServicePtr patient() { return mPatientModelService; }
	TrackingServicePtr tracking() { return mTrackingService; }
	VideoServicePtr video() { return mVideoService; }
	SpaceProviderPtr spaceProvider() { return mSpaceProvider; }
	SessionStorageServicePtr session() { return mSessionStorageService; }
	StateServicePtr state() { return mStateService; }

protected:
	PatientModelServicePtr mPatientModelService;
	TrackingServicePtr mTrackingService;
	VideoServicePtr mVideoService;
	SpaceProviderPtr mSpaceProvider;
	SessionStorageServicePtr mSessionStorageService;
	StateServicePtr mStateService;

protected:
	CoreServices();
};

}


#endif // CXCORESERVICES_H
