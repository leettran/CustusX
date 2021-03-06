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

#include "cxRegistrationServiceProxy.h"

#include <boost/bind.hpp>
#include <ctkPluginContext.h>
#include "cxNullDeleter.h"
#include "cxLogger.h"
#include <QDateTime>

namespace cx
{

RegistrationServiceProxy::RegistrationServiceProxy(ctkPluginContext *context) :
	mPluginContext(context),
	mRegistrationService(RegistrationService::getNullObject())
{
	this->initServiceListener();
}

void RegistrationServiceProxy::initServiceListener()
{
	mServiceListener.reset(new ServiceTrackerListener<RegistrationService>(
								 mPluginContext,
								 boost::bind(&RegistrationServiceProxy::onServiceAdded, this, _1),
								 boost::function<void (RegistrationService*)>(),
								 boost::bind(&RegistrationServiceProxy::onServiceRemoved, this, _1)
								 ));
	mServiceListener->open();
}
void RegistrationServiceProxy::onServiceAdded(RegistrationService* service)
{
	mRegistrationService.reset(service, null_deleter());
	connect(mRegistrationService.get(), SIGNAL(fixedDataChanged(QString)), this, SIGNAL(fixedDataChanged(QString)));
	connect(mRegistrationService.get(), SIGNAL(movingDataChanged(QString)), this, SIGNAL(movingDataChanged(QString)));
	if(mRegistrationService->isNull())
		reportWarning("RegistrationServiceProxy::onServiceAdded mRegistrationService->isNull()");

	emit fixedDataChanged(mRegistrationService->getFixedDataUid());
	emit movingDataChanged(mRegistrationService->getMovingDataUid());
}

void RegistrationServiceProxy::onServiceRemoved(RegistrationService *service)
{
	disconnect(service, SIGNAL(fixedDataChanged(QString)), this, SIGNAL(fixedDataChanged(QString)));
	disconnect(service, SIGNAL(movingDataChanged(QString)), this, SIGNAL(movingDataChanged(QString)));
	mRegistrationService = RegistrationService::getNullObject();

	emit fixedDataChanged("");
	emit movingDataChanged("");
}

void RegistrationServiceProxy::setMovingData(DataPtr data)
{
	mRegistrationService->setMovingData(data);
}


void RegistrationServiceProxy::setFixedData(DataPtr data)
{
	mRegistrationService->setFixedData(data);
}

DataPtr RegistrationServiceProxy::getMovingData()
{
	return mRegistrationService->getMovingData();
}

DataPtr RegistrationServiceProxy::getFixedData()
{
	return mRegistrationService->getFixedData();
}

void RegistrationServiceProxy::doPatientRegistration()
{
	return mRegistrationService->doPatientRegistration();
}

void RegistrationServiceProxy::doFastRegistration_Translation()
{
	return mRegistrationService->doFastRegistration_Translation();
}

void RegistrationServiceProxy::doFastRegistration_Orientation(const Transform3D &tMtm, const Transform3D &prMt)
{
	return mRegistrationService->doFastRegistration_Orientation(tMtm, prMt);
}

void RegistrationServiceProxy::doImageRegistration(bool translationOnly)
{
	return mRegistrationService->doImageRegistration(translationOnly);
}

void RegistrationServiceProxy::addImage2ImageRegistration(Transform3D delta_pre_rMd, QString description)
{
	mRegistrationService->addImage2ImageRegistration(delta_pre_rMd, description);
}

void RegistrationServiceProxy::updateImage2ImageRegistration(Transform3D delta_pre_rMd, QString description)
{
	mRegistrationService->updateImage2ImageRegistration(delta_pre_rMd, description);
}

void RegistrationServiceProxy::addPatientRegistration(Transform3D rMpr_new, QString description)
{
	mRegistrationService->addPatientRegistration( rMpr_new, description);
}

void RegistrationServiceProxy::updatePatientRegistration(Transform3D rMpr_new, QString description)
{
	mRegistrationService->updatePatientRegistration( rMpr_new, description);
}

void RegistrationServiceProxy::applyPatientOrientation(const Transform3D &tMtm, const Transform3D &prMt)
{
	mRegistrationService->applyPatientOrientation(tMtm, prMt);
}

QDateTime RegistrationServiceProxy::getLastRegistrationTime()
{
	return mRegistrationService->getLastRegistrationTime();
}

void RegistrationServiceProxy::setLastRegistrationTime(QDateTime time)
{
	mRegistrationService->setLastRegistrationTime(time);
}

bool RegistrationServiceProxy::isNull()
{
	return mRegistrationService->isNull();
}

} //cx
