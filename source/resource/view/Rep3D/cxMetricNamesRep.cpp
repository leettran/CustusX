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
#include "cxMetricNamesRep.h"

#include <vtkRenderer.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>

#include "cxView.h"
#include "cxVtkHelperClasses.h"
#include "cxTypeConversions.h"
#include "cxDataMetric.h"
#include "cxLogger.h"


namespace cx
{

MetricNamesRep::MetricNamesRep():
	RepImpl()
{
	mFontSize = 20;
}

MetricNamesRep::~MetricNamesRep()
{
}

MetricNamesRepPtr MetricNamesRep::New(const QString& uid)
{
	return wrap_new(new MetricNamesRep(), uid);
}

void MetricNamesRep::addRepActorsToViewRenderer(ViewPtr view)
{
	for(unsigned i =0; i<mDisplayText.size(); ++i)
	{
		mDisplayText[i]->setRenderer(view->getRenderer());
	}
}

void MetricNamesRep::removeRepActorsFromViewRenderer(ViewPtr view)
{
	for(unsigned i =0; i<mDisplayText.size(); ++i)
	{
		mDisplayText[i]->setRenderer(NULL);
	}
}

void MetricNamesRep::setData(std::vector<DataPtr> data)
{
	std::vector<DataMetricPtr> metrics = this->convertToMetrics(data);

	if (this->equal(metrics, mMetrics))
		return;

	for (unsigned i=0; i<mMetrics.size(); ++i)
	{
		disconnect(mMetrics[i].get(), SIGNAL(transformChanged()), this, SLOT(setModified()));
		disconnect(mMetrics[i].get(), SIGNAL(propertiesChanged()), this, SLOT(setModified()));
	}

	mMetrics = metrics;

	for (unsigned i=0; i<mMetrics.size(); ++i)
	{
		connect(mMetrics[i].get(), SIGNAL(transformChanged()), this, SLOT(setModified()));
		connect(mMetrics[i].get(), SIGNAL(propertiesChanged()), this, SLOT(setModified()));
	}

	this->setColoredTextList(this->getAllMetricTexts(), Eigen::Array2d(0.98, 0.98));
}

std::vector<DataMetricPtr> MetricNamesRep::convertToMetrics(std::vector<DataPtr> data)
{
	std::vector<DataMetricPtr> metrics;
	for (unsigned i=0; i<data.size(); ++i)
	{
		DataMetricPtr metric = boost::dynamic_pointer_cast<DataMetric>(data[i]);
		if (metric)
			metrics.push_back(metric);
	}
	return metrics;
}

bool MetricNamesRep::equal(std::vector<DataMetricPtr> a, std::vector<DataMetricPtr> b) const
{
	if (a.size()!=b.size())
		return false;
	for (unsigned i=0; i<a.size(); ++i)
		if (a[i] != b[i])
			return false;
	return true;
}

void MetricNamesRep::onModifiedStartRender()
{
	std::vector<std::pair<QColor, QString> > text;
	text = this->getAllMetricTexts();
	CX_ASSERT(text.size()==mDisplayText.size());

	for (unsigned i=0; i<mDisplayText.size(); ++i)
	{
		mDisplayText[i]->updateText(text[i].second);
		mDisplayText[i]->setColor(text[i].first);
	}

}

void MetricNamesRep::setColoredTextList(std::vector<std::pair<QColor, QString> > text, Eigen::Array2d pos, vtkViewport *vp)
{
	mDisplayText.clear();

	if (vp==0)
	{
		vp = this->getRenderer();
	}
//	if (vp==0)
//	{
//		return;
//	}

	DoubleBoundingBox3D bb;
	bb = this->findNormalizedBoundingBoxAroundText(text, pos, vp);
	double meanHeight = bb.range()[1]/text.size();

	for (unsigned i=0; i<text.size(); ++i)
	{
		TextDisplayPtr rep;
		rep.reset( new TextDisplay( text[i].second, text[i].first, 	mFontSize) );
		rep->textProperty()->SetJustificationToLeft();
		rep->getActor()->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();

		Vector3D currentPos = bb.bottomLeft() + (meanHeight*i)*Eigen::Vector3d::UnitY();
		rep->setPosition(currentPos);
		rep->setRenderer(this->getRenderer());

		mDisplayText.push_back(rep);
	}
}

DoubleBoundingBox3D MetricNamesRep::findNormalizedBoundingBoxAroundText(std::vector<std::pair<QColor, QString> > text, Eigen::Array2d pos, vtkViewport *vp)
{
	QStringList fullText;
	for (unsigned i=0; i<text.size(); ++i)
		fullText << text[i].second;
	TextDisplay test(fullText.join("\n"), QColor(Qt::red), mFontSize);
	test.textProperty()->SetJustificationToLeft();
	double width_p = test.getMapper()->GetWidth(vp);
	double height_p = test.getMapper()->GetHeight(vp);
	height_p *= 1.1; // increase spacing

	Eigen::Array2i size_vp(vp->GetSize());
	Eigen::Array2d normsize_box(width_p/size_vp[0], height_p/size_vp[1]);

	// move box into viewport:
	double border = 0.02;
	DoubleBoundingBox3D bb_vp(border, 1.0-border, border, 1.0-border, 0, 0);
	DoubleBoundingBox3D bb_box(pos[0], pos[0]+normsize_box[0], pos[1], pos[1]+normsize_box[1], 0, 0);
	bb_box = this->moveBoxIntoAnother(bb_box, bb_vp);

	return bb_box;
}

DoubleBoundingBox3D MetricNamesRep::moveBoxIntoAnother(DoubleBoundingBox3D box, DoubleBoundingBox3D another)
{
	// move negative direction
	for (unsigned i=0; i<2; ++i)
	{
		double shift = another[2*i+1] - box[2*i+1];
		if (shift < 0)
		{
			box[2*i] += shift;
			box[2*i+1] += shift;
		}
	}

	// move positive direction
	for (unsigned i=0; i<3; ++i)
	{
		double shift = another[2*i] - box[2*i];
		if (shift > 0)
		{
			box[2*i] -= shift;
			box[2*i+1] -= shift;
		}
	}

	return box;
}

void MetricNamesRep::setFontSize(int size)
{
	mFontSize = size;
}

QString MetricNamesRep::getText(DataMetricPtr metric, bool showLabel) const
{
	QStringList text;
	if (showLabel)
		text << metric->getName();
	if (metric->showValueInGraphics())
		text << metric->getValueAsString();
	return text.join(" = ");
}

std::vector<std::pair<QColor, QString> > MetricNamesRep::getAllMetricTexts() const
{
	std::vector<std::pair<QColor, QString> > retval;

	for (unsigned i = 0; i < mMetrics.size(); ++i)
	{
		DataMetricPtr metric = mMetrics[i];
		QString line = metric->getName();

		if (!metric->showValueInGraphics())
			continue;

		QString text = this->getText(metric, true);
		retval.push_back(std::make_pair(metric->getColor(), text));
	}
	std::reverse(retval.begin(), retval.end());
	return retval;
}


} // namespace cx

