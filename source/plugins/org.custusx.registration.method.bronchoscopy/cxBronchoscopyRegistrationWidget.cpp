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

#include "cxBronchoscopyRegistrationWidget.h"
#include <vtkPolyData.h>
#include "cxTransform3D.h"
#include "cxDataSelectWidget.h"
#include "cxTrackingService.h"
#include "cxMesh.h"
#include "cxAcquisitionData.h"
#include "cxSelectDataStringDataAdapter.h"
#include "cxTrackedCenterlineWidget.h"
#include "cxRecordSessionWidget.h"
#include "cxRecordSession.h"
#include "cxView.h"
#include "cxToolRep3D.h"
#include "cxToolTracer.h"
#include "cxBronchoscopyRegistration.h"
#include "cxReporter.h"
#include "cxTypeConversions.h"
#include "cxThresholdPreview.h"
#include "cxPatientModelService.h"
#include "cxRegistrationService.h"
#include "cxVisualizationService.h"
#include "cxStringDataAdapterXml.h"
#include "cxLabeledComboBoxWidget.h"
#include "cxTrackingService.h"
#include "cxDoubleDataAdapterXml.h"
#include "cxLegacySingletons.h"
#include "cxDataLocations.h"
#include "cxDataAdapterHelper.h"
#include "cxBoolDataAdapterXml.h"
#include "cxCheckBoxWidget.h"
#include "cxRepManager.h"
#include "cxViewGroupData.h"


namespace cx
{
BronchoscopyRegistrationWidget::BronchoscopyRegistrationWidget(RegServices services, QWidget* parent) :
	RegistrationBaseWidget(services, parent, "BronchoscopyRegistrationWidget",
						   "Bronchoscopy Registration"), mVerticalLayout(new QVBoxLayout(this)),
	mBronchoscopyRegistration(new BronchoscopyRegistration())
{

	mOptions = XmlOptionFile(DataLocations::getXmlSettingsFile()).descend("bronchoscopyregistrationwidget");

	mSelectMeshWidget = SelectMeshStringDataAdapter::New(services.patientModelService);
	mSelectMeshWidget->setValueName("Centerline: ");

	AcquisitionDataPtr mAcquisitionData;
	mAcquisitionData.reset(new AcquisitionData());

	connect(services.patientModelService.get(), &PatientModelService::isSaving, this, &BronchoscopyRegistrationWidget::duringSavePatientSlot);
	connect(services.patientModelService.get(), &PatientModelService::isLoading, this, &BronchoscopyRegistrationWidget::duringLoadPatientSlot);
	connect(services.patientModelService.get(), &PatientModelService::cleared, this, &BronchoscopyRegistrationWidget::duringClearPatientSlot);

	mProcessCenterlineButton = new QPushButton("Process centerline");
	connect(mProcessCenterlineButton, SIGNAL(clicked()), this, SLOT(processCenterlineSlot()));
	mProcessCenterlineButton->setToolTip(this->defaultWhatsThis());

	mRegisterButton = new QPushButton("Register");
	connect(mRegisterButton, SIGNAL(clicked()), this, SLOT(registerSlot()));
	mRegisterButton->setToolTip(this->defaultWhatsThis());

	mAcquisition.reset(new Acquisition(mAcquisitionData, this));
	this->initSessionSelector(mAcquisition->getPluginData());

	connect(mAcquisition.get(), &Acquisition::started, this, &BronchoscopyRegistrationWidget::acquisitionStarted);
	connect(mAcquisition.get(), &Acquisition::acquisitionStopped, this, &BronchoscopyRegistrationWidget::acquisitionStopped, Qt::QueuedConnection);
	connect(mAcquisition.get(), &Acquisition::cancelled, this, &BronchoscopyRegistrationWidget::acquisitionCancelled);

//    mTrackedCenterLine = new TrackedCenterlineWidget(mAcquisitionData, this);

	mRecordSessionWidget.reset(new RecordSessionWidget(mAcquisition, this, "Bronchoscope path"));

	mVerticalLayout->setMargin(0);
	mVerticalLayout->addWidget(new DataSelectWidget(services.visualizationService, services.patientModelService, this, mSelectMeshWidget));
//    mVerticalLayout->addWidget(mTrackedCenterLine);

	this->selectSubsetOfBranches(mOptions.getElement());
	this->createMaxNumberOfGenerations(mOptions.getElement());
	this->useLocalRegistration(mOptions.getElement());

	mVerticalLayout->addWidget(new LabeledComboBoxWidget(this, mSessionSelector));
	mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseSubsetOfGenerations));
	mVerticalLayout->addWidget(createDataWidget(services.visualizationService, services.patientModelService, this, mMaxNumberOfGenerations));
	mVerticalLayout->addWidget(mProcessCenterlineButton);
    mVerticalLayout->addWidget(mRecordSessionWidget.get());
	mVerticalLayout->addWidget(new CheckBoxWidget(this, mUseLocalRegistration));
	mVerticalLayout->addWidget(mRegisterButton);

	mVerticalLayout->addStretch();

    boost::shared_ptr<WidgetObscuredListener> mObscuredListener;

    mObscuredListener.reset(new WidgetObscuredListener(this));
    connect(mObscuredListener.get(), SIGNAL(obscured(bool)), this, SLOT(obscuredSlot(bool)));
}

void BronchoscopyRegistrationWidget::initSessionSelector(AcquisitionDataPtr acquisitionData)
{
	QStringList sessionUids = getSessionList(acquisitionData);
	QString defaultValue;
	if(!sessionUids.isEmpty())
		defaultValue = sessionUids.last();
	mSessionSelector = StringDataAdapterXml::initialize("bronchoscopy_session", "Bronchoscope path", "Select bronchoscope path for registration", defaultValue, sessionUids, QDomNode());

	//TODO: Let mSessionSelector display displaynames instead of uids (StringDataAdapterXml::setDisplayNames)
}

QStringList BronchoscopyRegistrationWidget::getSessionList(AcquisitionDataPtr acquisitionData)
{
	std::vector<RecordSessionPtr> sessions = acquisitionData->getRecordSessions();
	std::vector<RecordSessionPtr>::iterator it = sessions.begin();
	QStringList sessionUids;

	for(; it != sessions.end(); ++it)
	{
		QString uid = (*it)->getUid();
		sessionUids << uid;
	}

	sessionUids.sort();
	return sessionUids;
}

QString BronchoscopyRegistrationWidget::defaultWhatsThis() const
{
	return QString();
}

void BronchoscopyRegistrationWidget::processCenterlineSlot()
{
	if(!mSelectMeshWidget->getMesh())
	{
		reportError("No centerline");
		return;
	}
	vtkPolyDataPtr centerline = mSelectMeshWidget->getMesh()->getVtkPolyData();//input
	Transform3D rMd = mSelectMeshWidget->getMesh()->get_rMd();
	vtkPolyDataPtr processedCenterline;
	if (mUseSubsetOfGenerations->getValue())
		processedCenterline = mBronchoscopyRegistration->processCenterline(centerline, rMd, mMaxNumberOfGenerations->getValue());
	else
		processedCenterline = mBronchoscopyRegistration->processCenterline(centerline, rMd);

	if (!mMesh)
	{
		QString uid = mSelectMeshWidget->getMesh()->getUid() + "_cl%1";
		QString name = mSelectMeshWidget->getMesh()->getName()+" cl_processed%1";
		mMesh = patientService()->createSpecificData<Mesh>(uid, name);
	}
	mMesh->setVtkPolyData(processedCenterline);
	mMesh->setColor(QColor(0, 255, 0, 255));
	mServices.patientModelService->insertData(mMesh);
	mServices.visualizationService->autoShowData(mMesh);
}

void BronchoscopyRegistrationWidget::registerSlot()
{
	if(!mBronchoscopyRegistration->isCenterlineProcessed())
	{
		reportError("Centerline not processed");
		return;
	}

	Transform3D old_rMpr = mServices.patientModelService->get_rMpr();//input to registrationAlgorithm
    //std::cout << "rMpr: " << std::endl;
    //std::cout << old_rMpr << std::endl;

    if(!mTool)
        mTool = mServices.trackingService->getActiveTool();

    if(!mTool)
    {
        reportError("No tool");
		return;
    }
    std::cout << "Tool name: " << mTool->getName() << std::endl;

	RecordSessionPtr session;
	QString sessionUid = mSessionSelector->getValue();
	if(!sessionUid.isEmpty())
		session = mAcquisition->getPluginData()->getRecordSession(sessionUid);
//	else
//		session = mAcquisition->getLatestSession();

    if(!session)
        reportError("No session");

    TimedTransformMap trackerRecordedData_prMt = RecordSession::getToolHistory_prMt(mTool, session);//input
//    TimedTransformMap trackerRecordedData = mTrackedCenterLine->getRecording();

    if(trackerRecordedData_prMt.size() == 0)
	{
        reportError("No positions");
        return;
    }

	Transform3D new_rMpr;
	double maxDistanceForLocalRegistration = 30; //mm
	if(mUseLocalRegistration->getValue())
		new_rMpr = Transform3D(mBronchoscopyRegistration->runBronchoscopyRegistration(trackerRecordedData_prMt,old_rMpr,maxDistanceForLocalRegistration));
	else
		new_rMpr = Transform3D(mBronchoscopyRegistration->runBronchoscopyRegistration(trackerRecordedData_prMt,old_rMpr,0));

    new_rMpr = new_rMpr*old_rMpr;//output
	mServices.registrationService->applyPatientRegistration(new_rMpr, "Bronchoscopy centerline to tracking data");

    Eigen::Matrix4d display_rMpr = Eigen::Matrix4d::Identity();
            display_rMpr = new_rMpr*display_rMpr;
    std::cout << "New prMt: " << std::endl;
        for (int i = 0; i < 4; i++)
            std::cout << display_rMpr.row(i) << std::endl;

    ToolRep3DPtr activeRep3D = getToolRepIn3DView(mTool);
    if(activeRep3D->getTracer())
        activeRep3D->getTracer()->clear();

    QColor colorGreen = QColor(0, 255, 0, 255);
    activeRep3D->getTracer()->setColor(colorGreen);
    activeRep3D->getTracer()->addManyPositions(trackerRecordedData_prMt);


}

void BronchoscopyRegistrationWidget::acquisitionStarted()
{
    std::cout << "acquisitionStarted" << std::endl;

	mTool = mServices.trackingService->getActiveTool();
    ToolRep3DPtr activeRep3D = this->getToolRepIn3DView(mTool);
	if (!activeRep3D)
		return;

    activeRep3D->getTracer()->start();
}
void BronchoscopyRegistrationWidget::acquisitionStopped()
{
    std::cout << "acquisitionStopped" << std::endl;

	this->acquisitionCancelled();
	QString newUid = mAcquisition->getLatestSession()->getUid();
	QStringList range = mSessionSelector->getValueRange();
	range << newUid;
	mSessionSelector->setValueRange(range);
	mSessionSelector->setValue(newUid);

    mServices.patientModelService->autoSave();

//	this->saveSessions();
}

void BronchoscopyRegistrationWidget::acquisitionCancelled()
{
	ToolRep3DPtr activeRep3D = this->getToolRepIn3DView(mTool);
	if (!activeRep3D)
		return;

	activeRep3D->getTracer()->stop();
}

void BronchoscopyRegistrationWidget::duringSavePatientSlot()
{
	QDomElement sessionsNode = mServices.patientModelService->getCurrentWorkingElement("bronchoscopySessions");
	if(sessionsNode.isNull())
		reportWarning("BronchoscopyRegistrationWidget::duringSavePatientSlot() Try bronchoscopySessions node. Got null node");

	mAcquisition->getPluginData()->addXml(sessionsNode);
}

void BronchoscopyRegistrationWidget::duringLoadPatientSlot()
{
	mAcquisition->getPluginData()->clear();

	QDomElement sessionsNode = mServices.patientModelService->getCurrentWorkingElement("bronchoscopySessions/stateManager");
	mAcquisition->getPluginData()->parseXml(sessionsNode);

	QStringList sessionUids = getSessionList(mAcquisition->getPluginData());
	mSessionSelector->setValueRange(sessionUids);

	if(!sessionUids.isEmpty())
		mSessionSelector->setValue(sessionUids.last());
}

void BronchoscopyRegistrationWidget::duringClearPatientSlot()
{
	mAcquisition->getPluginData()->clear();
	QStringList sessionUids = getSessionList(mAcquisition->getPluginData());
	mSessionSelector->setValueRange(sessionUids);
}

ToolRep3DPtr BronchoscopyRegistrationWidget::getToolRepIn3DView(ToolPtr tool)
{
	ViewPtr view = mServices.visualizationService->get3DView();
    ToolRep3DPtr retval = RepManager::findFirstRep<ToolRep3D>(view->getReps(),tool);
	return retval;
}

void BronchoscopyRegistrationWidget::obscuredSlot(bool obscured)
{
    //std::cout << "Checking slot" << std::endl;
    if (!obscured)
        return;

    ToolRep3DPtr activeRep3D = this->getToolRepIn3DView(mTool);
    if (!activeRep3D)
        return;
    //std::cout << "Slot is cleared" << std::endl;
    activeRep3D->getTracer()->clear();
}

void BronchoscopyRegistrationWidget::createMaxNumberOfGenerations(QDomElement root)
{
	mMaxNumberOfGenerations = DoubleDataAdapterXml::initialize("Max number of generations in centerline", "",
	"Set max number of generations centerline", 4, DoubleRange(0, 10, 1), 0,
					root);
	mMaxNumberOfGenerations->setGuiRepresentation(DoubleDataAdapter::grSLIDER);
}

void BronchoscopyRegistrationWidget::selectSubsetOfBranches(QDomElement root)
{
	mUseSubsetOfGenerations = BoolDataAdapterXml::initialize("Select branch generations to be used in registration", "",
																			"Select branch generations to be used in registration", false,
																				root);
}

void BronchoscopyRegistrationWidget::useLocalRegistration(QDomElement root)
{
	mUseLocalRegistration = BoolDataAdapterXml::initialize("Use local registration", "",
																			"Use local registration", false,
																				root);
}

} //namespace cx
