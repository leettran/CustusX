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

#ifndef CXTRACKINGIMPLSERVICE_H_
#define CXTRACKINGIMPLSERVICE_H_

#include "cxTrackingService.h"
#include "org_custusx_core_tracking_Export.h"
class ctkPluginContext;

namespace cx
{
typedef boost::shared_ptr<class ToolManager> TrackingServiceOldPtr;

/**
 * Implementation of TrackingService.
 *
 * \ingroup org_custusx_core_tracking
 *
 * \date 2014-09-19
 * \author Ole Vegard Solberg
 */
class org_custusx_core_tracking_EXPORT TrackingImplService : public TrackingService
{
	Q_INTERFACES(cx::TrackingService)
public:
	TrackingImplService(ctkPluginContext* context);
	virtual ~TrackingImplService();

	virtual Tool::State getState() const;
	virtual void setState(const Tool::State val);

	virtual ToolPtr getTool(const QString& uid);
	virtual ToolPtr getActiveTool();
	virtual void setActiveTool(const QString& uid);
	virtual ToolPtr getFirstProbe();
	ToolPtr getReferenceTool() const;
	ToolPtr getManualTool();

	virtual ToolMap getTools(); ///< get configured and initialized tools

	virtual bool isPlaybackMode() const;
	virtual void setPlaybackMode(PlaybackTimePtr controller);

	virtual void savePositionHistory();
	virtual void loadPositionHistory();
	virtual void addXml(QDomNode& parentNode);
	virtual void parseXml(QDomNode& dataNode);
	virtual void clear();
	virtual SessionToolHistoryMap getSessionHistory(double startTime, double stopTime);
	virtual void setLoggingFolder(QString loggingFolder);
	virtual void runDummyTool(DummyToolPtr tool);
	virtual QStringList getSupportedTrackingSystems();
	virtual TrackerConfigurationPtr getConfiguration();

	virtual bool isNull();

private:
	ctkPluginContext *mContext;
	TrackingServiceOldPtr getOld() const;
	TrackingServiceOldPtr mOld;

};
typedef boost::shared_ptr<TrackingImplService> TrackingImplServicePtr;

} /* namespace cx */

#endif /* CXTRACKINGIMPLSERVICE_H_ */

