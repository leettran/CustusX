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


#ifndef CXVTKHELPERCLASSES_H_
#define CXVTKHELPERCLASSES_H_

#include "cxResourceVisualizationExport.h"

#include "cxTransform3D.h"
#include "cxGraphicalPrimitives.h"

#include "vtkForwardDeclarations.h"

#include <QColor>

// --------------------------------------------------------
namespace cx
{
// --------------------------------------------------------

/** Set the RGB and alpha components of QColor into a vtkProperty
  */
void setColorAndOpacity(vtkPropertyPtr property, QColor color);
/** Convert a QColor to Vector3D using 0..1 scale, ignoring alpha.
  */
Vector3D getColorAsVector3D(QColor color) ;

/**\brief RGB color data.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT RGBColor : public boost::array<double,3>
{
public:
	RGBColor();
	RGBColor(double r, double g, double b);
	explicit RGBColor(const double* rgb);
	RGBColor(const RGBColor& t);
	RGBColor& operator=(const RGBColor& t);
	RGBColor(QColor c);
	RGBColor& operator=(const QColor& c);
	~RGBColor() {}
};

/**\brief Helper for drawing a point in 2D.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT OffsetPoint
{
	public:
		OffsetPoint( vtkRendererPtr renderer );
		~OffsetPoint();
		void setRadius ( int radius );
		void setValue(Vector3D point, RGBColor color);
		void setValue(Vector3D point, Vector3D color);
		void update( const Vector3D& point );
		vtkActor2DPtr getActor();
	private:
		vtkSphereSourcePtr source;
		vtkPolyDataMapper2DPtr mapper;
		vtkActor2DPtr actor;
		vtkRendererPtr mRenderer;
};
typedef boost::shared_ptr<OffsetPoint> OffsetPointPtr;

/**\brief Helper for drawing a line in 2D.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT LineSegment
{
	public:
		LineSegment(vtkRendererPtr renderer);
		~LineSegment();
		void setPoints(Vector3D point1, Vector3D point2, RGBColor color, int stipplePattern = 0xFFFF);
		void setResolution(int res);
		void updatePosition(Vector3D point1, Vector3D point2);
		vtkActor2DPtr getActor();
		void setColor(RGBColor color);
		void setPattern(int stipplePattern);
		void setWidth(float width);

	private:
		vtkPolyDataMapper2DPtr mapper2d;
		vtkActor2DPtr actor2d;
		vtkRendererPtr mRenderer;
		vtkLineSourcePtr source;
};

/**\brief Helper for drawing text in 2D.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT TextDisplay
{
	public:
		TextDisplay(const QString& text, const QColor& color, int fontsize);
		~TextDisplay();
		void setRenderer( vtkRendererPtr renderer );
		void setPosition( float x, float y);
		void setPosition( const Vector3D& pos );
		void setCentered();
		void updateText(const QString& text );
		void setColor(QColor color);
		vtkTextProperty* textProperty();
		vtkActor2DPtr getActor();
		void setMaxWidth( int width, vtkViewport *vp );
		int getMaxWidth();
		int getWidth( vtkViewport *vp );
		vtkTextMapperPtr getMapper() { return mapper; }

		static void forceUseVtkTextMapper();

	private:
		vtkTextMapperPtr mapper;
		vtkActor2DPtr actor;
		vtkRendererPtr mRenderer;
		int maxWidth;
		QString text;

		static void verifyVtkTextMapper();
};
typedef boost::shared_ptr<class TextDisplay> TextDisplayPtr;


/**\brief Helper for drawing a crosshair in 2D.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT CrossHair2D
{
	public:
		CrossHair2D( vtkRendererPtr renderer);
		~CrossHair2D();
		void setValue(const Vector3D& focalPoint, int width, int height, double bordarOffset, const RGBColor& color);
		//void update( const Vector3D& crossPos );
		void update( const Vector3D& crossPos, const DoubleBoundingBox3D& vp);
		void updateRegCross( const Vector3D& crossPos );
		vtkActor2DPtr getActor();
	private:
		vtkCursor2DPtr mCursor2D;
		vtkPolyDataMapper2DPtr mapper;
		vtkActor2DPtr actor;
		vtkRendererPtr mRenderer;
};
typedef boost::shared_ptr<class CrossHair2D> CrossHair2DPtr;

/**\brief Helper for drawing a coordinate axis in 3D.
 *
 * Wrapper class for a vtkAxes. Sets up some nice defaults and draws the
 * axes in the world coordinate system.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT Axes3D
{
public:
	Axes3D(vtkRendererPtr renderer = vtkRendererPtr());
	~Axes3D();
	void setPosition(const Transform3D& pos);
	vtkProp3DPtr getProp();
private:
 	void setCaption(vtkCaptionActor2D* captionActor, const QString& caption, RGBColor color);

 	vtkAxesActorPtr mAxes;
	vtkRendererPtr mRenderer;
};
typedef boost::shared_ptr<Axes3D> Axes3DPtr;

// --------------------------------------------------------
}//	namespace vm
// --------------------------------------------------------


#endif // CXVTKHELPERCLASSES_H_

