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


#ifndef CXTEXTURE3DSLICERPROXY_H_
#define CXTEXTURE3DSLICERPROXY_H_

#include "cxResourceVisualizationExport.h"

#include "cxRepImpl.h"
#include <vector>
#include "cxTransform3D.h"
#include "cxBoundingBox3D.h"

#include "vtkForwardDeclarations.h"
#include "cxForwardDeclarations.h"
#include "cxConfig.h"

typedef vtkSmartPointer<class vtkPolyDataAlgorithm> vtkPolyDataAlgorithmPtr;
//---------------------------------------------------------
namespace cx
{

typedef vtkSmartPointer<class TextureSlicePainter> TextureSlicePainterPtr;
typedef boost::shared_ptr<class Texture3DSlicerProxy> Texture3DSlicerProxyPtr;

/**
 * \brief Helper class for GPU-based slicing.
 *
 * This class is a dummy that only provide interface.
 * For OS that don't implement the GPU slicer, use this dummy.
 * See Texture3DSlicerProxyImpl for the real implementation.
 *
 * \sa Texture3DSlicerProxyImpl for the real implementation.
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT Texture3DSlicerProxy: public QObject
{
Q_OBJECT
public:
	static Texture3DSlicerProxyPtr New();

	virtual ~Texture3DSlicerProxy() {}
	virtual void setShaderPath(QString shaderFile) {}
	virtual void setViewportData(const Transform3D& vpMs, const DoubleBoundingBox3D& vp) {}
	virtual void setImages(std::vector<ImagePtr> images) {}
	virtual std::vector<ImagePtr> getImages() { return std::vector<ImagePtr>(); }
	virtual void setSliceProxy(SliceProxyPtr slicer) {}
	virtual SliceProxyPtr getSliceProxy() { return SliceProxyPtr(); }
	virtual void update() {}
	virtual void setTargetSpaceToR(){}
	virtual vtkActorPtr getActor() { return vtkActorPtr(); }

	static bool isSupported(vtkRenderWindowPtr window);

};

//#ifndef CX_VTK_OPENGL2
//#ifndef WIN32

#if !defined(CX_VTK_OPENGL2) && !defined(WIN32)

/**
 * \brief Slice volumes using a SliceProxy.
 *
 * The functionality is equal to SlicedImageProxy, but the actual slicing
 * is performed by loading the image into the GPU as a 3D texture and
 * slicing it there, using the GPU.
 *
 * Used by Sonowand and Sintef.
 *
 *  Created on: Oct 13, 2011
 *      Author: christiana
 *
 * \ingroup cx_resource_view
 */
class cxResourceVisualization_EXPORT Texture3DSlicerProxyImpl: public Texture3DSlicerProxy
{
	Q_OBJECT
public:
	static Texture3DSlicerProxyPtr New();
	virtual ~Texture3DSlicerProxyImpl();
	void setShaderPath(QString shaderFile);
	void setViewportData(const Transform3D& vpMs, const DoubleBoundingBox3D& vp); // DEPRECATED: use zoomfactor in View and the object will auto-update
	void setImages(std::vector<ImagePtr> images);
	void setSliceProxy(SliceProxyPtr slicer);
	SliceProxyPtr getSliceProxy() { return mSliceProxy; }
	void update();
	void setTargetSpaceToR(); ///< use to draw the slice in 3D r space instead of in 2D s space.
	vtkActorPtr getActor();
	std::vector<ImagePtr> getImages() { return mImages; }

protected:
	Texture3DSlicerProxyImpl();
	void createGeometryPlane(Vector3D point1_s, Vector3D point2_s, Vector3D origin_s);

protected slots:
	void transformChangedSlot();
	void updateColorAttributeSlot();
	void imageChanged();

private:
	void resetGeometryPlane();
	void updateCoordinates(int index);
	QString getTCoordName(int index);
	void setColorAttributes(int i);
	std::vector<ImagePtr> processImages(std::vector<ImagePtr> images_raw);

	DoubleBoundingBox3D mBB_s;
	std::vector<ImagePtr> mImages;
	std::vector<ImagePtr> mRawImages;
	SliceProxyPtr mSliceProxy;
	bool mTargetSpaceIsR;

	TextureSlicePainterPtr mPainter;
	vtkActorPtr mActor;
	vtkPolyDataPtr mPolyData;
	vtkPlaneSourcePtr mPlaneSource;
	vtkPainterPolyDataMapperPtr mPainterPolyDatamapper;
	vtkPolyDataAlgorithmPtr mPolyDataAlgorithm;

	static const int mMaxImages = 4;// This class is hardcoded for a maximum of 4 images
	bool isNewInputImages(std::vector<ImagePtr> images_raw);
};

//#endif // WIN32
#else
//Dummy code to make file compile with Qt moc. This is needed because Qt moc ignores ifdef.
class cxResourceVisualization_EXPORT Texture3DSlicerProxyImpl: public Texture3DSlicerProxy
{
	Q_OBJECT
protected slots:
	void transformChangedSlot() {}
	void updateColorAttributeSlot() {}
	void imageChanged() {}
};
#endif //CX_VTK_OPENGL2


//---------------------------------------------------------
}//end namespace
//---------------------------------------------------------
#endif /* CXTEXTURE3DSLICERPROXY_H_ */
