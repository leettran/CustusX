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



#include "cxToolRep2D.h"

#include <vtkRenderer.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include "cxSliceProxy.h"
#include "cxTool.h"
#include "cxView.h"

#include "cxTypeConversions.h"
#include "cxProbeSector.h"
#include "cxSpaceProvider.h"
#include "cxSettings.h"

namespace cx
{

ToolRep2D::ToolRep2D(SpaceProviderPtr spaceProvider) :
	RepImpl(),
	mSpaceProvider(spaceProvider),
	m_vpMs(Transform3D::Identity()),
	mBB_vp(0, 1, 0, 1, 0, 1),
	mTooltipPointColor(0.96, 0.87, 0.17),
	mOffsetPointColor(0.96, 0.87, 0.17),
//	mTooltipLineColor(1, 0, 0),
	mTooltipLineColor(0.25, 0.87, 0.16),
	mOffsetLineColor(1.0, 0.8, 0.0),
	mStipplePattern(0xffff)
{
	mTooltipLineColor = settings()->value("View/tool2DColor").value<QColor>();
	mTooltipPointColor = settings()->value("View/toolTipPointColor").value<QColor>();
	mOffsetPointColor = settings()->value("View/toolOffsetPointColor").value<QColor>();
	mOffsetLineColor = settings()->value("View/toolOffsetLineColor").value<QColor>();

	mUseOffset = true;
	mUseCrosshair = false;
	mUseToolLine = true;
	mUseOffsetText = false;
	mMergeOffsetAndToolLine = false;
	mProbeSector.reset(new ProbeSector());
	mProbeSectorPolyDataMapper = vtkPolyDataMapperPtr::New();
	mProbeSectorActor = vtkActorPtr::New();
}

ToolRep2D::~ToolRep2D()
{
}

ToolRep2DPtr ToolRep2D::New(SpaceProviderPtr spaceProvider, const QString& uid)
{
	return wrap_new(new ToolRep2D(spaceProvider), uid);
}

QString ToolRep2D::getType() const
{
	return "vm::ToolRep2D";
}

double ToolRep2D::getOffset()
{
	if (mSlicer->getTool() && showOffset())
		return mSlicer->getTool()->getTooltipOffset();
	return 0.0;
}

/**Set transform from slice space to viewport, and the viewport size.
 * Some old code need this in order to render in viewport coordinates.
 */
void ToolRep2D::setViewportData(const Transform3D& vpMs, const DoubleBoundingBox3D& vp)
{
	if (similar(m_vpMs, vpMs) && similar(mBB_vp, vp))
		return;

	m_vpMs = vpMs;
	mBB_vp = vp;

	crossHairResized();
	this->setModified();
}

/**Set a slice proxy representing the plane on which to render.
 */
void ToolRep2D::setSliceProxy(SliceProxyPtr slicer)
{
	if (mSlicer)
	{
		disconnect(mSlicer.get(), SIGNAL(transformChanged(Transform3D)), this, SLOT(sliceTransformChangedSlot(Transform3D)));
		disconnect(mSlicer.get(), SIGNAL(toolTransformAndTimestamp(Transform3D,double)), this, SLOT(toolTransformAndTimestampSlot(Transform3D,double)));
		disconnect(mSlicer.get(), SIGNAL(toolVisible(bool)), this, SLOT(toolVisibleSlot(bool)));
	}

	mSlicer = slicer;

	connect(mSlicer.get(), SIGNAL(transformChanged(Transform3D)), this, SLOT(sliceTransformChangedSlot(Transform3D)));
	connect(mSlicer.get(), SIGNAL(toolTransformAndTimestamp(Transform3D,double)), this, SLOT(toolTransformAndTimestampSlot(Transform3D,double)));
	connect(mSlicer.get(), SIGNAL(toolVisible(bool)), this, SLOT(toolVisibleSlot(bool)));
}

/**Set display of the line from tool tip to the cross pos (this is the offset line).
 */
void ToolRep2D::setUseOffset(bool on)
{
	if (mUseOffset==on)
		return;
	mUseOffset = on;
	setVisibility();
}

/**Set display of a yellow crosshair centered on the cross pos.
 */
void ToolRep2D::setUseCrosshair(bool on)
{
	if (mUseCrosshair==on)
		return;
	mUseCrosshair = on;
	setVisibility();
}

/**Set display of the line from back infinity to the tool tip.
 */
void ToolRep2D::setUseToolLine(bool on)
{
	if (mUseToolLine==on)
		return;
	mUseToolLine = on;
	setVisibility();
}

/**Set display of the amount of offset, in the upper right corner.
 */
void ToolRep2D::setUseOffsetText(bool on)
{
	if (mUseOffsetText==on)
		return;
	mUseOffsetText = on;
	setVisibility();
}

/**Set to merge the rep of the tool and the tool offset into one line, thus making the
 * location of the physical tool tip invisible.
 *
 */
void ToolRep2D::setMergeOffsetAndToolLine(bool on)
{
	if (mMergeOffsetAndToolLine==on)
		return;
	mMergeOffsetAndToolLine = on;
	setVisibility();
}

void ToolRep2D::addRepActorsToViewRenderer(ViewPtr view)
{
	createToolLine(view->getRenderer(), Vector3D(0,0,0));
	createCrossHair(view->getRenderer() );
	createOffsetText(view->getRenderer(), Vector3D(0,0,0));
	view->getRenderer()->AddActor(mProbeSectorActor);
	setVisibility();
	this->setModified();
}

void ToolRep2D::removeRepActorsFromViewRenderer(ViewPtr view)
{
	cursor.reset();
	center2Tool.reset();
	tool2Back.reset();
	centerPoint.reset();
	toolPoint.reset();
	distanceText.reset();
	view->getRenderer()->RemoveActor(mProbeSectorActor);
}

void ToolRep2D::sliceTransformChangedSlot(Transform3D sMr)
{
	this->setModified();
}

void ToolRep2D::toolTransformAndTimestampSlot(Transform3D prMt, double timestamp)
{
	this->setModified();
}

void ToolRep2D::toolVisibleSlot(bool visible)
{
	setVisibility();
	this->setModified();
}

void ToolRep2D::onModifiedStartRender()
{
	this->update();
}

void ToolRep2D::update()
{
	if (!mSlicer->getTool())
		return;

	Transform3D prMt = Transform3D::Identity();
	if (mSlicer->getTool())
	{
		prMt = mSlicer->getTool()->get_prMt();
	}
	Transform3D rMpr = mSpaceProvider->get_rMpr();
	Transform3D sMr = mSlicer->get_sMr();
	Transform3D sMt = sMr*rMpr*prMt;
	Transform3D vpMt = m_vpMs*sMt;

	// only show probe if aligned with the slice plane:
	double dotted = dot(Vector3D(0,0,1),sMt.vector(Vector3D(1,0,0)));
	bool aligned = similar(fabs(dotted), 1.0, 0.1);
	if (this->showProbe() && aligned)
	{
		Transform3D T = createTransformTranslate(Vector3D(0, 0, 0.1));

		mProbeSector->setData(mSlicer->getTool()->getProbe()->getProbeDefinition());
		Transform3D tMu = mProbeSector->get_tMu();
		mProbeSectorPolyDataMapper->SetInputData(mProbeSector->getSectorLinesOnly());
		if (mProbeSectorPolyDataMapper->GetInput())
		{
			mProbeSectorActor->SetMapper(mProbeSectorPolyDataMapper);
		}
		mProbeSectorActor->SetUserMatrix((T*sMt*tMu).getVtkMatrix());
		mProbeSectorActor->SetVisibility(mSlicer->getTool()->getVisible());
	}
	else
	{
		mProbeSectorActor->SetVisibility(false);
	}

	Vector3D cross = vpMt.coord(getOffset() * Vector3D(0,0,1)); // zero position plus offset along z
	Vector3D tooltip = vpMt.coord(Vector3D(0,0,0)); // the zero position
	Vector3D toolback = vpMt.coord(Vector3D(0,0,-1000)); // a point 1m backwards in z

	if (cursor)
	{
		cursor->update(cross, mBB_vp); ///crosshair, shows in Navigation
	}
	updateOffsetText();
	updateToolLine(cross, tooltip, toolback);
}

bool ToolRep2D::showProbe()
{
	return mSlicer->getTool() && mSlicer->getTool()->hasType(Tool::TOOL_US_PROBE);
}
bool ToolRep2D::showOffset()
{
	return mSlicer->getTool() && mUseOffset;
}

void ToolRep2D::setVisibility()
{
//	Logger::log("vm.log", "ToolRep2D::setVisibility(), offset=" + string_cast(showOffset()));
	bool hasTool = mSlicer->getTool() ? true : false;

	if (cursor)
		cursor->getActor()->SetVisibility(mUseCrosshair && hasTool);
	if (center2Tool)
		center2Tool->getActor()->SetVisibility(showOffset());
	if (tool2Back)
		tool2Back->getActor()->SetVisibility(mUseToolLine && hasTool);
	if (centerPoint)
		centerPoint->getActor()->SetVisibility(mUseToolLine && hasTool);
	if (centerPoint)
		centerPoint->getActor()->SetVisibility(showOffset() && !mMergeOffsetAndToolLine);
	if (distanceText)
		distanceText->getActor()->SetVisibility(mUseOffsetText && showOffset() && !mMergeOffsetAndToolLine);
}

/**create a yellow crosshair centered at the tool offset pos. 
 * 
 */
void ToolRep2D::createCrossHair(vtkRendererPtr renderer)
{
	//Logger::log("vm.log", "<"+string_cast(__FUNCTION__)+">"+" new " );
	cursor.reset( new CrossHair2D(renderer) ) ;
	crossHairResized();
}

/**create a yellow crosshair centered at the tool offset pos. 
 * 
 */
void ToolRep2D::crossHairResized()
{
	if (cursor)
	{
		double bordarOffset = 30.0;
		QColor col =  settings()->value("View/toolCrossHairColor").value<QColor>();
//		RGBColor color(1.0, 0.8, 0.0);
		RGBColor color(col.redF(), col.greenF(), col.blueF());
//		RGBColor color(1.0, 0.2, 0.0);
		Vector3D focalPoint(0.0,0.0,0.0);
		//Logger::log("tool.log", "("+string_cast(__FUNCTION__)+")"+" mCross"+string_cast(cross));
		//cursor->setValue( focalPoint, mBB_vp.range()[0], mBB_vp.range()[1], bordarOffset, color  );
		cursor->setValue( focalPoint, int(mBB_vp.range()[0]), int(mBB_vp.range()[1]), bordarOffset, color  );
	}
}

/**create the line projection of the tool + offset onto the plane.
 */
void ToolRep2D::createToolLine(vtkRendererPtr renderer, const Vector3D& centerPos )
{
	// line from tooltip to offset point
	center2Tool.reset(new LineSegment(renderer));
	center2Tool->setPoints(centerPos, Vector3D(0.0, 0.0, 0.0), mOffsetLineColor, mStipplePattern);
	center2Tool->setWidth(2);

	// line from back infinity to tooltip
	tool2Back.reset(new LineSegment(renderer));
	tool2Back->setPoints(Vector3D(0.0, 0.0, 0.0), Vector3D(0.0, 0.0, 0.0), mTooltipLineColor);
	tool2Back->setWidth(2);

	// Add dot at offset point
	centerPoint.reset(new OffsetPoint(renderer));
	centerPoint->setValue(centerPos, mOffsetPointColor);
	centerPoint->setRadius(4);
	centerPoint->getActor()->VisibilityOff();

	// Add dot at tooltip point
	toolPoint.reset(new OffsetPoint(renderer));
	toolPoint->setValue(Vector3D(0.0, 0.0, 0.0), mTooltipPointColor);
	toolPoint->setRadius(3);
}

/**create the offset text
 */
void ToolRep2D::createOffsetText(vtkRendererPtr renderer, const Vector3D& pos )
{
	QColor color = QColor::fromRgbF(0.7372, 0.815, 0.6039);
	distanceText.reset( new TextDisplay( "---", color, 18) );
	distanceText->getActor()->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
	distanceText->textProperty()->SetJustificationToRight();
	distanceText->setPosition(Vector3D(0.95, 0.95, 0.0));
	renderer->AddActor( distanceText->getActor() );
	updateOffsetText();
}

/**
 * must update crosshair, offset, points and distance
 *  when in registration no update to tool
 */
void ToolRep2D::updateToolLine(const Vector3D& crossPos, const Vector3D& toolTipPos, const Vector3D toolTipBackPos)
{
	if (!center2Tool || !tool2Back || !centerPoint || !toolPoint)
		return;

	if (mMergeOffsetAndToolLine)	// for ACS only
	{
		// Make entire line look like an offset line, since tooltip point will become projected
		// into the planes set by the offset position, which may be seen as navigation error.
		center2Tool->getActor()->SetVisibility(false);
		tool2Back->updatePosition(crossPos, toolTipBackPos);
		if (getOffset() > 0.01)
		{
			toolPoint->getActor()->SetVisibility(false);
			centerPoint->getActor()->SetVisibility(true);
			tool2Back->setColor(mOffsetLineColor);
			tool2Back->setPattern(mStipplePattern);
		}
		else
		{
			toolPoint->getActor()->SetVisibility(true);
			centerPoint->getActor()->SetVisibility(false);
			tool2Back->setColor(mTooltipLineColor);
			tool2Back->setPattern(0xffff);
		}
	}
	else
	{
		center2Tool->updatePosition(crossPos, toolTipPos);
		tool2Back->updatePosition(toolTipPos, toolTipBackPos);
		if (getOffset() > 0.01)
		{
			centerPoint->getActor()->SetVisibility(true);
		}
		else
		{
			centerPoint->getActor()->SetVisibility(false);
		}
	}
	centerPoint->update(crossPos);
	toolPoint->update(toolTipPos);
}

void ToolRep2D::updateOffsetText()
{
	if (!distanceText)
	{
		return;
	}
	if (getOffset() > 2.0 && mUseOffsetText && showOffset())
	{
		distanceText->updateText("Offset: " + QString::number(getOffset(), 'g', 3) + " mm");
		distanceText->getActor()->VisibilityOn();
	}
	else
	{
		distanceText->getActor()->VisibilityOff(); // doesnt work...
		distanceText->updateText( "" ); // does work...
	}
}

} // namespace vm
