#ifndef CXSIMULATEDIMAGESTREAMER_H_
#define CXSIMULATEDIMAGESTREAMER_H_

#include "vtkSmartPointer.h"
#include "sscTransform3D.h"
#include "cxImageStreamer.h"

typedef vtkSmartPointer<class vtkPNGWriter> vtkPNGWriterPtr;
namespace cx
{
typedef ssc::Transform3D Transform3D;

/**
 * SimulatedImageStreamer delivers a stream of 2D images
 * sliced from the incoming volume based on the tools positions.
 *
 * \date May 21, 2013
 * \author Janne Beate Bakeng, SINTEF
 */

class SimulatedImageStreamer : public ImageStreamer
{
	Q_OBJECT

public:
	SimulatedImageStreamer();
	virtual ~SimulatedImageStreamer();

	void initialize(ssc::ImagePtr image, ssc::ToolPtr tool);
	virtual bool startStreaming(SenderPtr sender);
	virtual void stopStreaming();

	virtual QString getType();

private slots:
	virtual void streamSlot();
	void sliceSlot();

private:
	ssc::ImagePtr getSlice(ssc::ImagePtr source);
	vtkMatrix4x4Ptr calculateSliceAxes();
	vtkImageDataPtr getSliceUsingProbeDefinition(ssc::ImagePtr source, vtkMatrix4x4Ptr sliceAxes);
	ssc::ImagePtr createSscImage(vtkImageDataPtr slice, ssc::ImagePtr volume);
	vtkImageReslicePtr createReslicer(ssc::ImagePtr source, vtkMatrix4x4Ptr sliceAxes);
	ssc::Transform3D getTransformFromProbeSectorImageSpaceToImageSpace();

	ssc::ImagePtr mSourceImage;
	ssc::ToolPtr mTool;
	ssc::ImagePtr mImageToSend;

};
typedef boost::shared_ptr<SimulatedImageStreamer> SimulatedImageStreamerPtr;

} /* namespace cx */
#endif /* CXSIMULATEDIMAGESTREAMER_H_ */