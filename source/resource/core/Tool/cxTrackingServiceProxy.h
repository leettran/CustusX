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

#ifndef CXTRACKINGSERVICEPROXY_H
#define CXTRACKINGSERVICEPROXY_H

#include "cxResourceExport.h"

#include "cxTrackingService.h"
#include "cxServiceTrackerListener.h"
class ctkPluginContext;

namespace cx
{

/**
 *  \ingroup cx_resource_core_tool
 */
class cxResource_EXPORT TrackingServiceProxy : public TrackingService
{
public:
	static TrackingServicePtr create(ctkPluginContext *pluginContext);
	TrackingServiceProxy(ctkPluginContext *pluginContext);

	virtual Tool::State getState() const;
	virtual void setState(const Tool::State val);

	virtual ToolPtr getTool(const QString& uid);
	virtual ToolPtr getActiveTool();
	virtual void setActiveTool(const QString& uid);
	virtual ToolPtr getFirstProbe();

	virtual ToolPtr getReferenceTool() const;
	virtual ToolPtr getManualTool();

	virtual ToolMap getTools(); ///< get configured and initialized tools

	virtual bool isPlaybackMode() const;
	virtual void setPlaybackMode(PlaybackTimePtr controller);

	virtual SessionToolHistoryMap getSessionHistory(double startTime, double stopTime);
	virtual void runDummyTool(DummyToolPtr tool);
	virtual TrackerConfigurationPtr getConfiguration();

	virtual void installTrackingSystem(TrackingSystemServicePtr system);
	virtual void unInstallTrackingSystem(TrackingSystemServicePtr system);
	virtual std::vector<TrackingSystemServicePtr> getTrackingSystems();

	bool isNull();

private:
	void initServiceListener();
	void onServiceAdded(TrackingService* service);
	void onServiceRemoved(TrackingService *service);

	ctkPluginContext *mPluginContext;
	TrackingServicePtr mTrackingService;
	boost::shared_ptr<ServiceTrackerListener<TrackingService> > mServiceListener;
};
} //cx
#endif // CXTRACKINGSERVICEPROXY_H
