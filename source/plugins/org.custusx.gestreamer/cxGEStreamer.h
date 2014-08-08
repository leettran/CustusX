#ifndef CXIMAGESTREAMERGE_H_
#define CXIMAGESTREAMERGE_H_

#include <QObject>

#ifdef CX_USE_ISB_GE
#include "boost/shared_ptr.hpp"
#include <QTcpSocket>
#include <QDateTime>
#include <QSize>
#include <QStringList>
#include "igtlImageMessage.h"
#include "cxIGTLinkImageMessage.h"
#include "cxCyclicActionLogger.h"
#include "GEStreamer.h"
#include "cxImageStreamer.h"

class QTimer;

namespace cx
{

QString findOpenCLPath(QString additionalLocation); ///< Find GEStreamer OpenCL kernel code


typedef boost::shared_ptr<class GEStreamer> GEStreamerPtr;
/**An object sending images out on an ip port.
 * In order to operate within a nongui thread,
 * it must be created within the run() method
 * of a qthread.
 *
 * \ingroup cx_resource_videoserver
 * \author Ole Vegard Solberg, SINTEF
 * \date Sep 19, 2012
 *
 * This version uses the NTNU ISB data streamer module (provided by Gabriel Kiss)
 * to grab images from the E9 EG scanner (and similar)
 */
class GEStreamer: public Streamer
{
	Q_OBJECT

public:
	struct Options{
		std::string IP;
		int streamPort;
		int commandPort;
		int bufferSize;
		long imageSize;
		data_streaming::OutputSizeComputationType computationType;
		std::string	testMode;
		bool useOpenCL;
		bool scanconvertedStream;
		bool tissueStream;
		bool bandwidthStream;
		bool frequencyStream;
		bool velocityStream;
	};

public:
	GEStreamer();
	virtual ~GEStreamer() {}

	void setOptions(const Options& options);
	virtual bool startStreaming(SenderPtr sender);
	virtual void stopStreaming();

	virtual QString getType();

private slots:
	virtual void streamSlot();
	void grab();
	void send();

private:
	void applyOptions();
	bool initialize_local();
	void deinitialize_local();

	ProbeDefinitionPtr getFrameStatus(QString uid, data_streaming::frame_geometry geometry, vtkSmartPointer<vtkImageData> img);
	void send(const QString& uid, const vtkImageDataPtr& img, data_streaming::frame_geometry geometry, bool geometryChanged);

	//Compare to geometry structs
	bool equal(data_streaming::frame_geometry a, data_streaming::frame_geometry b);
	void printTimeIntervals();

	QTimer* mGrabTimer;
	CyclicActionLoggerPtr mRenderTimer;

	//The GE Connection code from ISB
	data_streaming::GEStreamer mGEStreamer;

	vtkSmartPointer<data_streaming::vtkExportedStreamData> mImgExportedStream;//Last image from GE

	igstk::RealTimeClock::TimeStampType mLastGrabTime;
	data_streaming::frame_geometry mFrameGeometry;///<Frame geometry from GE
	bool mFrameGeometryChanged; ///< Have frame geometry changed since last frame

	data_streaming::frame_geometry mFlowGeometry;///<Frame geometry for flow data (doppler) from GE
	bool mFlowGeometryChanged; ///< Have flow data frame geometry changed since last frame

	//What kind of video streams are requested?
//   	bool mExportScanconverted;
//   	bool mExportTissue;
//   	bool mExportBandwidth;
//   	bool mExportFrequency;
//   	bool mExportVelocity;

   	Options mOptions;

};

}

#endif // CX_USE_ISB_GE
#endif // CXIMAGESTREAMERGE_H_
