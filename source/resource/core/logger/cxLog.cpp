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

#include "cxLog.h"

#include "cxLogger.h"
#include <QtGlobal>
#include <QThread>
#include <iostream>
#include "boost/shared_ptr.hpp"
#include <QString>
#include <QMutex>
#include <QSound>
#include <QDir>
#include <QTextStream>
#include "cxTypeConversions.h"
#include "cxDefinitionStrings.h"
#include "cxTime.h"
#include "cxProfile.h"
#include "cxMessageListener.h"
#include "internal/cxLogThread.h"
#include "QApplication"

namespace cx
{

class EventProcessingThread : public QThread
{
public:
	EventProcessingThread()
	{
	}
	virtual ~EventProcessingThread()
	{

	}

	virtual void run()
	{
		this->exec();
		qApp->processEvents(); // exec() docs doesn't guarantee that the posted events are processed. - do that here.
	}
};


Log::Log()
{
	mLogPath = this->getDefaultLogPath();
}

Log::~Log()
{
	this->stopThread();
}

QString Log::getDefaultLogPath() const
{
	QString isoDateFormat("yyyy-MM-dd");
	QString isoDate = QDateTime::currentDateTime().toString(isoDateFormat);
	QString retval = ProfileManager::getInstance()->getSettingsPath()+"/Logs/"+isoDate;
	return retval;
}

void Log::initializeObject()
{
	if (mThread)
		return;

	this->stopThread();
	this->startThread();
}

void Log::startThread()
{
	if (mThread)
		return;

	mThread.reset(new EventProcessingThread());
	mThread->setObjectName("org.custusx.resource.core.logger");

	mWorker = this->createWorker();
	mWorker->moveToThread(mThread.get());
	if (!mLogPath.isEmpty())
		mWorker->setLoggingFolder(mLogPath);
	connect(mWorker.get(), &LogThread::emittedMessage, this, &Log::onEmittedMessage);

	mThread->start();
}

void Log::stopThread()
{
	if (!mThread)
		return;

	disconnect(mWorker.get(), &LogThread::emittedMessage, this, &Log::onEmittedMessage);
	LogThreadPtr tempWorker = mWorker;
	mWorker.reset();

	mThread->quit();
	mThread->wait(); // forever or until dead thread

	mThread.reset();
	tempWorker.reset();
}

void Log::setLoggingFolder(QString absoluteLoggingFolderPath)
{
	mLogPath = absoluteLoggingFolderPath;
	if (mWorker)
		mWorker->setLoggingFolder(mLogPath);
	emit loggingFolderChanged();
}

QString Log::getLoggingFolder() const
{
	return mLogPath;
}

void Log::installObserver(MessageObserverPtr observer, bool resend)
{
	if (mWorker)
		mWorker->installObserver(observer, resend);
}

void Log::uninstallObserver(MessageObserverPtr observer)
{
	if (mWorker)
		mWorker->uninstallObserver(observer);
}


} //End namespace cx
