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
#include "cxNetworkDataTransfer.h"

#include "cxNetworkConnection.h"
#include "cxBoolProperty.h"
#include "cxProfile.h"
#include "cxHelperWidgets.h"
#include "cxPatientModelServiceProxy.h"
#include "cxViewServiceProxy.h"
#include "cxSelectDataStringProperty.h"
#include "cxStringProperty.h"
#include "cxMesh.h"
#include "cxIGTLinkConversionPolyData.h"
#include "cxVideoServiceProxy.h"
#include "cxVideoSource.h"
#include "cxNetworkConnectionHandle.h"
#include "boost/bind.hpp"

namespace cx {

NetworkDataTransfer::NetworkDataTransfer(ctkPluginContext *context, NetworkConnectionHandlePtr connection, QObject* parent) :
	QObject(parent),
	mContext(context),
	mOpenIGTLink(connection)
{
	mOptions = profile()->getXmlSettings().descend(this->getConfigUid());

	mPatientModelService = PatientModelServiceProxy::create(context);
	mViewService = ViewServiceProxy::create(context);
	mVideoService = VideoServiceProxy::create(context);

//	mOpenIGTLink.reset(new NetworkConnectionHandle(this->getConfigUid()));

	connect(mOpenIGTLink->getNetworkConnection(), &NetworkConnection::image, this, &NetworkDataTransfer::onImageReceived);
	connect(mOpenIGTLink->getNetworkConnection(), &NetworkConnection::mesh, this, &NetworkDataTransfer::onMeshReceived);

	mDataToSend = StringPropertySelectData::New(mPatientModelService);

	mAcceptIncomingData = BoolProperty::initialize("acceptIncoming", "Accept Incoming",
												   "Accept incoming data and add to Patient",
												   true, mOptions.getElement());

	mStreamActiveVideoSource = BoolProperty::initialize("stream", "Stream Video",
												   "Stream the active Video Source over the connection",
												   false, QDomNode());
	connect(mStreamActiveVideoSource.get(), &BoolProperty::changed,
			this, &NetworkDataTransfer::onStreamActiveVideoSourceChanged);

}

NetworkDataTransfer::~NetworkDataTransfer()
{
	mOpenIGTLink.reset(); //
}

NetworkConnectionHandlePtr NetworkDataTransfer::getOpenIGTLink()
{
	return mOpenIGTLink;
}

QString NetworkDataTransfer::getConfigUid() const
{
	return "org.custusx.core.openigtlink.datatransfer";
}

void NetworkDataTransfer::onImageReceived(ImagePtr image)
{
	this->onDataReceived(image);
}

void NetworkDataTransfer::onMeshReceived(MeshPtr mesh)
{
	this->onDataReceived(mesh);
}

void NetworkDataTransfer::onDataReceived(DataPtr data)
{
	QString actionText = mAcceptIncomingData->getValue() ? "inserting" : "ignoring";
	QString nameText = data ? data->getName() : "NULL";
	CX_LOG_CHANNEL_INFO(this->getConfigUid()) << QString("Received image [%1] over IGTLink, %2")
														   .arg(nameText)
														   .arg(actionText);

	if (mAcceptIncomingData->getValue())
	{
		mPatientModelService->insertData(data);
		mViewService->autoShowData(data);
	}
}


void NetworkDataTransfer::onSend()
{
	DataPtr data = mDataToSend->getData();
	ImagePtr image = boost::dynamic_pointer_cast<Image>(data);
	if (image)
	{
		boost::function<void()> message = boost::bind(&NetworkConnection::sendImage, mOpenIGTLink->getNetworkConnection(), image);
		mOpenIGTLink->getNetworkConnection()->invoke(message);
		return;
	}
	MeshPtr mesh = boost::dynamic_pointer_cast<Mesh>(data);
	if (mesh)
	{
//		// test begin
//		IGTLinkConversionPolyData polyConverter;
//		igtl::PolyDataMessage::Pointer msg = polyConverter.encode(mesh, pcsRAS);
//		CX_LOG_CHANNEL_DEBUG(CX_OPENIGTLINK_CHANNEL_NAME) << "Debbuging mesh: " << data->getName();

//		MeshPtr retval = polyConverter.decode(msg, pcsRAS);
//		mPatientModelService->insertData(retval);
//		return;
//		// test end

		boost::function<void()> message = boost::bind(&NetworkConnection::sendMesh, mOpenIGTLink->getNetworkConnection(), mesh);
		mOpenIGTLink->getNetworkConnection()->invoke(message);
		return;
	}

	QString name = data ? data->getName() : "NULL";
	CX_LOG_CHANNEL_INFO(this->getConfigUid()) << QString("Failed to send data %1 over igtl: Unsupported type")
												 .arg(name);
}

void NetworkDataTransfer::onStreamActiveVideoSourceChanged()
{
	if (mStreamActiveVideoSource->getValue())
	{
		this->startStream();
	}
	else
	{
		this->stopStream();
	}

}

void NetworkDataTransfer::startStream()
{
	if (mStreamingVideoSource)
	{
		CX_LOG_WARNING() << QString("Already emitting VideoSource %1 on igtl").arg(mStreamingVideoSource->getName());
		return;
	}

	mStreamingVideoSource = mVideoService->getActiveVideoSource();
	connect(mStreamingVideoSource.get(), &VideoSource::newFrame,
			this, &NetworkDataTransfer::onNewStreamFrame);
	CX_LOG_INFO() << QString("Started emitting VideoSource %1 on igtl").arg(mStreamingVideoSource->getName());
}

void NetworkDataTransfer::stopStream()
{
	if (!mStreamingVideoSource)
		return;

	CX_LOG_INFO() << QString("Stopped emitting VideoSource %1 on igtl").arg(mStreamingVideoSource->getName());
	disconnect(mStreamingVideoSource.get(), &VideoSource::newFrame,
			   this, &NetworkDataTransfer::onNewStreamFrame);
	mStreamingVideoSource.reset();
}

void NetworkDataTransfer::onNewStreamFrame()
{
	vtkImageDataPtr data = mStreamingVideoSource->getVtkImageData();

	ImagePtr image(new Image(mStreamingVideoSource->getUid()+"_snapshot",
							 data,
							 mStreamingVideoSource->getName()));

	boost::function<void()> message = boost::bind(&NetworkConnection::streamImage, mOpenIGTLink->getNetworkConnection(), image);
	mOpenIGTLink->getNetworkConnection()->invoke(message);
}

} // namespace cx
