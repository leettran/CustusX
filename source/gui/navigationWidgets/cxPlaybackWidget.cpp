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

#include "cxPlaybackWidget.h"

#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <QLabel>
#include "cxTrackingService.h"
#include "cxHelperWidgets.h"
#include "cxTime.h"
#include "cxLogger.h"

#include "cxTypeConversions.h"
#include "cxTimelineWidget.h"
#include "cxData.h"
#include "cxRegistrationTransform.h"
#include "cxVideoService.h"
//#include "cxPlaybackUSAcquisitionVideo.h"
#include "cxSettings.h"
#include "cxPatientModelService.h"

namespace cx
{

PlaybackWidget::PlaybackWidget(QWidget* parent) :
				BaseWidget(parent, "playback_widget", "Playback")
{
	mOpen = false;
	this->setToolTip("Replay current session");

	mTimer.reset(new PlaybackTime());
	mTimer->initialize(QDateTime::currentDateTime(), 100000);
	connect(mTimer.get(), SIGNAL(changed()), SLOT(timeChangedSlot()));

	QVBoxLayout* topLayout = new QVBoxLayout(this);

	mStartTimeLabel = new QLabel;
	topLayout->addWidget(mStartTimeLabel);
	mTotalLengthLabel = new QLabel;
	topLayout->addWidget(mTotalLengthLabel);
	mLabel = new QLabel;
	topLayout->addWidget(mLabel);

	mToolTimelineWidget = new TimelineWidget(this);
	connect(mToolTimelineWidget, SIGNAL(positionChanged()), this, SLOT(timeLineWidgetValueChangedSlot()));
	topLayout->addWidget(mToolTimelineWidget);

	// create buttons bar
	QHBoxLayout* playButtonsLayout = new QHBoxLayout;
	topLayout->addLayout(playButtonsLayout);

	mOpenAction = this->createAction(this,
	        		QIcon(":/icons/open_icon_library/button-red.png"),
					"Open Playback", "",
	                SLOT(toggleOpenSlot()),
	                playButtonsLayout);
	this->createAction(this,
	                QIcon(":/icons/open_icon_library/media-seek-backward-3.png"),
					"Rewind", "",
	                SLOT(rewindSlot()),
	                playButtonsLayout);
	mPlayAction = this->createAction(this,
	                QIcon(":/icons/open_icon_library/media-playback-start-3.png"),
					"Play", "",
	                SLOT(playSlot()),
	                playButtonsLayout);
	this->createAction(this,
	                QIcon(":/icons/open_icon_library/media-seek-forward-3.png"),
					"Forward", "",
	                SLOT(forwardSlot()),
	                playButtonsLayout);
	this->createAction(this,
	                QIcon(":/icons/open_icon_library/media-playback-stop-3.png"),
					"Stop", "",
	                SLOT(stopSlot()),
	                playButtonsLayout);
	this->createAction(this,
	      QIcon(":/icons/open_icon_library/system-run-5.png"),
	      "Details", "Details",
	      SLOT(toggleDetailsSlot()),
	      playButtonsLayout);

	mSpeedAdapter = DoubleProperty::initialize(
					"speed",
					"Speed",
					"Set speed of playback, 0 is normal speed.", 0, DoubleRange(-5,5,1),0);
	connect(mSpeedAdapter.get(), SIGNAL(changed()), this, SLOT(speedChangedSlot()));
	playButtonsLayout->addWidget(sscCreateDataWidget(this, mSpeedAdapter));

	playButtonsLayout->addStretch();

	topLayout->addStretch();
	this->showDetails();
}

PlaybackWidget::~PlaybackWidget()
{
}

void PlaybackWidget::toggleDetailsSlot()
{
	settings()->setValue("playback/ShowDetails", !settings()->value("playback/ShowDetails", "true").toBool());
	this->showDetails();
}

void PlaybackWidget::showDetails()
{
	bool on = settings()->value("playback/ShowDetails").toBool();

	mStartTimeLabel->setVisible(on);
	mTotalLengthLabel->setVisible(on);
}

void PlaybackWidget::timeLineWidgetValueChangedSlot()
{
	mTimer->setTime(QDateTime::fromMSecsSinceEpoch(mToolTimelineWidget->getPos()));
}

void PlaybackWidget::toggleOpenSlot()
{
	if (trackingService()->isPlaybackMode())
	{
		mTimer->stop();
		trackingService()->setPlaybackMode(PlaybackTimePtr());
		videoService()->setPlaybackMode(PlaybackTimePtr());
	}
	else
	{
        trackingService()->setPlaybackMode(mTimer);
		if (!trackingService()->isPlaybackMode())
        {
            reportError("trackingService is not in playback mode");
			return;
        }
        videoService()->setPlaybackMode(mTimer);
		report(QString("Started Playback with start time [%1] and end time [%2]")
						.arg(mTimer->getStartTime().toString(timestampMilliSecondsFormatNice()))
						.arg(mTimer->getStartTime().addMSecs(mTimer->getLength()).toString(timestampMilliSecondsFormatNice())));

        this->toolManagerInitializedSlot();
	}
}

QColor PlaybackWidget::generateRandomToolColor() const
{
	std::vector<QColor> colors;
	int s = 255;
	int v = 192;
	colors.push_back(QColor::fromHsv(110, s, v));
	colors.push_back(QColor::fromHsv(80, s, v));
	colors.push_back(QColor::fromHsv(140, s, v));
	colors.push_back(QColor::fromHsv(95, s, v));
	colors.push_back(QColor::fromHsv(125, s, v));

	static int gCounter = 0;
	return colors[(gCounter++)%colors.size()];
}

std::vector<TimelineEvent> PlaybackWidget::convertHistoryToEvents(ToolPtr tool)
{
	std::vector<TimelineEvent> retval;
	TimedTransformMapPtr history = tool->getPositionHistory();
	if (!history || history->empty())
		return retval;
	double timeout = 200;
	TimelineEvent currentEvent(tool->getName() + " visible", history->begin()->first);
	currentEvent.mGroup = "tool";
	currentEvent.mColor = this->generateRandomToolColor(); // QColor::fromHsv(110, 255, 192);
//	std::cout << "first event start: " << currentEvent.mDescription << " " << currentEvent.mStartTime << " " << history->size() << std::endl;

	for(TimedTransformMap::iterator iter=history->begin(); iter!=history->end(); ++iter)
	{
		double current = iter->first;

		if (current - currentEvent.mEndTime > timeout)
		{
			retval.push_back(currentEvent);
			currentEvent.mStartTime = currentEvent.mEndTime = current;
		}
		else
		{
			currentEvent.mEndTime = current;
		}
	}
	if (!similar(currentEvent.mEndTime - currentEvent.mStartTime, 0))
		retval.push_back(currentEvent);

	return retval;
}

std::vector<TimelineEvent> PlaybackWidget::convertRegistrationHistoryToEvents(RegistrationHistoryPtr reg)
{
	std::vector<TimelineEvent> events;

	std::vector<RegistrationTransform> tr = reg->getData();
	for (unsigned i=0; i<tr.size(); ++i)
	{
		if (!tr[i].mTimestamp.isValid())
			continue;

		QString text = QString("Registraton %1, fixed=%2").arg(tr[i].mType).arg(tr[i].mFixed);
		if (!tr[i].mMoving.isEmpty())
			text = QString("%1, moving=%2").arg(text).arg(tr[i].mMoving);

		events.push_back(TimelineEvent(text,
						tr[i].mTimestamp.toMSecsSinceEpoch()));
	}

//	std::vector<ParentSpace> ps = reg->getParentSpaces();

	return events;
}


bool PlaybackWidget::isInterestingTool(ToolPtr tool) const
{
	return !tool->hasType(Tool::TOOL_MANUAL) && !tool->hasType(Tool::TOOL_REFERENCE);
}

std::vector<TimelineEvent> PlaybackWidget::createEvents()
{
	typedef std::vector<TimelineEvent> TimelineEventVector;

	// find all valid regions (i.e. time sequences with tool navigation)
	TimelineEventVector events;
	TrackingService::ToolMap tools = trackingService()->getTools();
	for (TrackingService::ToolMap::iterator iter=tools.begin(); iter!=tools.end(); ++iter)
	{
		if(this->isInterestingTool(iter->second))
		{
			TimelineEventVector current = convertHistoryToEvents(iter->second);
			copy(current.begin(), current.end(), std::back_inserter(events));
		}
	}

	std::map<QString, DataPtr> data = patientService()->getDatas();
	for (std::map<QString, DataPtr>::iterator iter=data.begin(); iter!=data.end(); ++iter)
	{
		QString desc("loaded " + iter->second->getName());
		if (iter->second->getAcquisitionTime().isValid())
		{
			double acqTime = iter->second->getAcquisitionTime().toMSecsSinceEpoch();
			events.push_back(TimelineEvent(desc, acqTime));
		}

		RegistrationHistoryPtr reg = iter->second->get_rMd_History();
		TimelineEventVector current = this->convertRegistrationHistoryToEvents(reg);
		copy(current.begin(), current.end(), std::back_inserter(events));
	}

	RegistrationHistoryPtr reg = patientService()->get_rMpr_History();
	TimelineEventVector current = this->convertRegistrationHistoryToEvents(reg);
	copy(current.begin(), current.end(), std::back_inserter(events));

	current = videoService()->getPlaybackEvents();
	copy(current.begin(), current.end(), std::back_inserter(events));


	return events;
}

/**Use the events to find the full time range to use.
 *
 */
std::pair<double,double> PlaybackWidget::findTimeRange(std::vector<TimelineEvent> events)
{
	if (events.empty())
	{
		double now = QDateTime::currentDateTime().toMSecsSinceEpoch();
		return std::make_pair(now, now+1000);
	}

	std::pair<double,double> timeRange(getMilliSecondsSinceEpoch(), 0);
//	std::pair<double,double> timeRange(events[0].mStartTime, events[0].mEndTime);

	for (unsigned i=0; i<events.size(); ++i)
	{
		timeRange.first = std::min(timeRange.first, events[i].mStartTime);
		timeRange.second = std::max(timeRange.second, events[i].mEndTime);
//		std::cout << events[i].mDescription  << std::endl;
//		std::cout << "===start " << QDateTime::fromMSecsSinceEpoch(events[i].mStartTime).toString(timestampMilliSecondsFormatNice()) << std::endl;
//		std::cout << "===  end " << QDateTime::fromMSecsSinceEpoch(events[i].mEndTime).toString(timestampMilliSecondsFormatNice()) << std::endl;
//		std::cout << "===start " << events[i].mStartTime << std::endl;
//		std::cout << "===  end " << events[i].mEndTime << std::endl;
//		std::cout << "======" << std::endl;
	}
//	std::cout << "======" << std::endl;
//	std::cout << "======" << std::endl;
//	std::cout << "======" << std::endl;

	return timeRange;
}

void PlaybackWidget::toolManagerInitializedSlot()
{
	if (trackingService()->isPlaybackMode())
	{
		mOpenAction->setText("Close Playback");
		mOpenAction->setIcon(QIcon(":/icons/open_icon_library/button-green.png"));
	}
	else
	{
		mOpenAction->setText("Open Playback");
		mOpenAction->setIcon(QIcon(":/icons/open_icon_library/button-red.png"));
		mToolTimelineWidget->setEvents(std::vector<TimelineEvent>());
		return;
	}

	if (trackingService()->getState() < Tool::tsINITIALIZED)
		return;

	std::vector<TimelineEvent> events = this->createEvents();
	std::pair<double,double> range = this->findTimeRange(events);
//	std::cout << "===start " << QDateTime::fromMSecsSinceEpoch(range.first).toString(timestampMilliSecondsFormatNice()) << std::endl;
//	std::cout << "===  end " << QDateTime::fromMSecsSinceEpoch(range.second).toString(timestampMilliSecondsFormatNice()) << std::endl;
	mTimer->initialize(QDateTime::fromMSecsSinceEpoch(range.first), range.second - range.first);

	//TODO merge into one initializer:
	mToolTimelineWidget->setRange(range.first, range.second);
	mToolTimelineWidget->setEvents(events);

	QString startDate = mTimer->getStartTime().toString("yyyy-MM-dd");
	QString startTime = mTimer->getStartTime().toString("hh:mm");
	QString endTime = mTimer->getStartTime().addMSecs(mTimer->getLength()).toString("hh:mm");
//	QString length = this->stripLeadingZeros(QTime(0,0,0,0).addMSecs(mTimer->getLength()).toString("hh:mm:ss"));
	QString length = this->convertMillisecsToNiceString(mTimer->getLength());
	mStartTimeLabel->setText(
					QString("Date:").leftJustified(15)    +"" + startDate+"\n" +
					QString("Time:").leftJustified(15)    +"" + startTime + " - " + endTime + "\n" +
					QString("Duration:").leftJustified(15)+"" + length);

	this->timeChangedSlot();
}

/**strip leading zeros from a hh:mm:ss-formatted time
 *
 */
//QString PlaybackWidget::stripLeadingZeros(QString time)
//{
//	QStringList split = time.split(":");
//	bool ok = false;
//	while (!split.empty() && (split.front().toInt(&ok)==0) && ok)
//	{
//		split.pop_front();
//	}
//	return split.join(":");
//}

/**Convert a millisecond count to a hh:mm:ss:zzz time.
 *
 */
QString PlaybackWidget::convertMillisecsToNiceString(qint64 length) const
{
	QString retval;

	qint64 ms = length % 1000;
	qint64 s = (length / 1000) % 60;
	qint64 m = (length / (1000*60)) % 60;
	qint64 h = (length / (1000*60*60));
	QChar c = '0';

	retval = QString("%1:%2.%3").arg(m,2,10,c).arg(s,2,10,c).arg(ms,3,10,c);
	if (h>0)
		retval = QString("%1:%2").arg(h).arg(retval);

	return retval;
}

void PlaybackWidget::speedChangedSlot()
{
	double speed = mSpeedAdapter->getValue();
	speed = pow(2,speed);
	mTimer->setSpeed(speed);
}


void PlaybackWidget::timeChangedSlot()
{
	QString color("green");
	int fontSize = 4;
	qint64 offset = mTimer->getOffset(); // SmStartTime.secsTo(QDateTime::currentDateTime());
	QString format = QString("<font size=%1 color=%2><b>%3</b></font>").arg(fontSize).arg(color);

	QString currentTime = mTimer->getTime().toString("hh:mm:ss");
//	QString currentOffset = this->stripLeadingZeros(QTime(0,0,0,0).addMSecs(offset).toString("hh:mm:ss.zzz"));
	QString currentOffset = this->convertMillisecsToNiceString(offset);

	mLabel->setText(format.arg("Elapsed: "+currentOffset+" \tTime: " + currentTime));

	if (mTimer->isPlaying())
	{
		mPlayAction->setIcon(QIcon(":/icons/open_icon_library/media-playback-pause-3.png"));
		mPlayAction->setText("Pause");
	}
	else
	{
		mPlayAction->setIcon(QIcon(":/icons/open_icon_library/media-playback-start-3.png"));
		mPlayAction->setText("Play");
	}

//	mTimeLineSlider->blockSignals(true);
////	mTimeLineSlider->setDoubleRange(dRange); // in case the image is changed
//	mTimeLineSlider->setRange(0, mTimer->getLength());
//	mTimeLineSlider->setSingleStep(1);
//
//	mTimeLineSlider->setValue(offset);
//	mTimeLineSlider->setToolTip(QString("Current time"));
//	mTimeLineSlider->blockSignals(false);

	mToolTimelineWidget->blockSignals(true);
	mToolTimelineWidget->setPos(mTimer->getTime().toMSecsSinceEpoch());
	mToolTimelineWidget->blockSignals(false);
}

void PlaybackWidget::playSlot()
{
	if (mTimer->isPlaying())
	{
		mTimer->pause();
	}
	else
	{
		mTimer->start();
	}
}
//void PlaybackWidget::pauseSlot()
//{
//	mTimer->pause();
//}
void PlaybackWidget::stopSlot()
{
	mTimer->stop();
}
void PlaybackWidget::forwardSlot()
{
	mTimer->forward(1000*mTimer->getSpeed());
}
void PlaybackWidget::rewindSlot()
{
	mTimer->rewind(1000*mTimer->getSpeed());
}


} /* namespace cx */
