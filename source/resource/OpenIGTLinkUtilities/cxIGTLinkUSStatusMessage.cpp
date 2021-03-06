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

#include "cxIGTLinkUSStatusMessage.h"
#include "igtl_header.h"
#include <string.h>
#include "igtl_us_status.h"

namespace cx
{

IGTLinkUSStatusMessage::IGTLinkUSStatusMessage():
		igtl::MessageBase(),
		mProbeType(0)
{
	AllocatePack();
	m_StatusMessage = m_Body;
	m_StatusMessage = NULL;

  for (int i = 0; i < 3; i ++)
  {
    mDataOrigin[i] = 0.0;
  }

  m_DefaultBodyType  = "CX_US_ST";
}

IGTLinkUSStatusMessage::~IGTLinkUSStatusMessage()
{}

void IGTLinkUSStatusMessage::SetProbeType(int probeType)
{
	mProbeType = probeType;
}
int IGTLinkUSStatusMessage::GetProbeType()
{
	return mProbeType;
}
void IGTLinkUSStatusMessage::SetOrigin(double o[3])
{
  mDataOrigin[0] = o[0];
  mDataOrigin[1] = o[1];
  mDataOrigin[2] = o[2];
}
void IGTLinkUSStatusMessage::SetOrigin(double oi, double oj, double ok)
{
  mDataOrigin[0] = oi;
  mDataOrigin[1] = oj;
  mDataOrigin[2] = ok;
}

void IGTLinkUSStatusMessage::GetOrigin(double  o[3])
{
  o[0] = mDataOrigin[0];
  o[1] = mDataOrigin[1];
  o[2] = mDataOrigin[2];
}

const double* IGTLinkUSStatusMessage::GetOrigin() const
{
	return mDataOrigin;
}

void IGTLinkUSStatusMessage::GetOrigin(double &oi, double &oj, double &ok)
{
  oi = mDataOrigin[0];
  oj = mDataOrigin[1];
  ok = mDataOrigin[2];
}

void IGTLinkUSStatusMessage::SetDepthStart(double depthStart)
{
	mDepthStart = depthStart;
}
double IGTLinkUSStatusMessage::GetDepthStart()
{
	return mDepthStart;
}
void IGTLinkUSStatusMessage::SetDepthEnd(double depthEnd)
{
	mDepthEnd = depthEnd;
}
double IGTLinkUSStatusMessage::GetDepthEnd()
{
	return mDepthEnd;
}
void IGTLinkUSStatusMessage::SetWidth(double width)
{
	mWidth = width;
}
double IGTLinkUSStatusMessage::GetWidth()
{
	return mWidth;
}
void IGTLinkUSStatusMessage::SetDataFormat(std::string dataFormat)
{
	mDataFormat = dataFormat;
}
std::string IGTLinkUSStatusMessage::GetDataFormat()
{
	return mDataFormat;
}

int IGTLinkUSStatusMessage::PackBody()
{
	// Allocate pack
	AllocatePack();
	m_StatusMessage = this->m_Body;

	// Set pointers
	igtl_us_status_message* statusMessage = (igtl_us_status_message*)this->m_StatusMessage;

	//Copy data
	statusMessage->originX    = static_cast<igtl_float64>(this->mDataOrigin[0]);
	statusMessage->originY    = static_cast<igtl_float64>(this->mDataOrigin[1]);
	statusMessage->originZ    = static_cast<igtl_float64>(this->mDataOrigin[2]);
	statusMessage->depthStart = static_cast<igtl_float64>(this->mDepthStart);
	statusMessage->depthEnd   = static_cast<igtl_float64>(this->mDepthEnd);
	statusMessage->width      = static_cast<igtl_float64>(this->mWidth);

	statusMessage->probeType  = static_cast<igtl_int32>(this->mProbeType);
	//TODO: string dataformat

	/*int originMemSpace = sizeof(igtl_float64)*3;
	memcpy((*void)this->m_StatusMessage, (void*) mDataOrigin, originMemSpace);
	memcpy((*void)this->m_StatusMessage[originMemSpace], (void*) mNewStatus, sizeof(igtl_uint8));*/

	// Convert byte order from host to network
	igtl_us_status_convert_byte_order(statusMessage);

	return 1;
}

int IGTLinkUSStatusMessage::GetBodyPackSize()
{
	return IGTL_US_STATUS_HEADER_SIZE;
}

int IGTLinkUSStatusMessage::UnpackBody()
{
	m_StatusMessage = this->m_Body;

	// Set pointers
	igtl_us_status_message* statusMessage = (igtl_us_status_message*)this->m_StatusMessage;

	// Convert byte order from network to host
	igtl_us_status_convert_byte_order(statusMessage);

	//Copy data
	this->mDataOrigin[0]  = statusMessage->originX;
	this->mDataOrigin[1]  = statusMessage->originY;
	this->mDataOrigin[2]  = statusMessage->originZ;
	this->mDepthStart     = statusMessage->depthStart;
	this->mDepthEnd       = statusMessage->depthEnd;
	this->mWidth          = statusMessage->width;
	this->mProbeType      = statusMessage->probeType;
	//TODO: dataformat

	return 1;
}
}
