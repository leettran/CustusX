#ifndef CXOPENCLRECONSTRUCTIONHOST_H_
#define CXOPENCLRECONSTRUCTIONHOST_H_

#include "sscReconstructAlgorithm.h"
#include "cxOpenCLUtilities.h"

namespace cx
{
/**
 * \brief 
 *
 * \date Jan 22, 2014
 * \author Janne Beate Bakeng, SINTEF
 */

class OpenCLReconstructionHost
{
public:
	OpenCLReconstructionHost(OpenCL::ocl* opencl, ProcessedUSInputDataPtr input, vtkImageDataPtr outputData);

	bool reconstruct();

private:
	bool findClosesInputPlanesForAllOutputVoxels(int numberOfPlanesToFind, int searchRadiusInMm); //run a kernel
	bool fillOutputVoxelWithApropiateIntensity(); //run a kernel

	cl::Kernel getKernelWithName(QString name);

	OpenCL::ocl* mOpenCL;

	ProcessedUSInputDataPtr mInput;
	vtkImageDataPtr mOutput;

//	std::map<int, Transform3D> mP

	QString mKernelPath;

};
} /* namespace cx */

#endif /* CXOPENCLRECONSTRUCTIONHOST_H_ */