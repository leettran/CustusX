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

#include "cxManualRegistrationWidget.h"
#include <boost/bind.hpp>
#include <ctkPluginContext.h>
#include "cxLabeledComboBoxWidget.h"

#include "cxRegistrationService.h"
#include "cxData.h"
#include "cxTransform3DWidget.h"

namespace cx
{

ManualImageRegistrationWidget::ManualImageRegistrationWidget(RegServicesPtr services, QWidget* parent, QString objectName, QString windowTitle) :
	BaseWidget(parent, objectName, windowTitle),
	mVerticalLayout(new QVBoxLayout(this)),
	mServices(services)
{
	this->setToolTip("Set image position directly");
	mVerticalLayout->setMargin(0);
	mLabel = new QLabel("Data matrix rMd");
	mVerticalLayout->addWidget(mLabel);
	mMatrixWidget = new Transform3DWidget(this);
	mVerticalLayout->addWidget(mMatrixWidget);
	connect(mMatrixWidget, SIGNAL(changed()), this, SLOT(matrixWidgetChanged()));
	mMatrixWidget->setEditable(true);

	mVerticalLayout->addStretch();

	connect(mServices->registration().get(), SIGNAL(movingDataChanged(QString)), this, SLOT(movingDataChanged()));
}

void ManualImageRegistrationWidget::showEvent(QShowEvent* event)
{
	this->imageMatrixChanged();
	this->movingDataChanged();
}

/** Called when the moving data in the RegistrationManager has changed.
 *  Update connections.
 */
void ManualImageRegistrationWidget::movingDataChanged()
{
	if (mConnectedMovingImage)
		disconnect(mConnectedMovingImage.get(), SIGNAL(transformChanged()), this, SLOT(imageMatrixChanged()));

	mConnectedMovingImage = mServices->registration()->getMovingData();

	if (mConnectedMovingImage)
		connect(mConnectedMovingImage.get(), SIGNAL(transformChanged()), this, SLOT(imageMatrixChanged()));

	mLabel->setText(this->getDescription());

	mMatrixWidget->setEnabled(mConnectedMovingImage!=0);
	this->imageMatrixChanged();
}

/** Called when the matrix in the widget has changed.
 *  Perform registration.
 *
 */
void ManualImageRegistrationWidget::matrixWidgetChanged()
{
	if (!mConnectedMovingImage)
		return;
	this->setMatrixFromWidget(mMatrixWidget->getMatrix());
}

/** Called when moving image has changed.
 *  Updates the displayed matrix.
 */
void ManualImageRegistrationWidget::imageMatrixChanged()
{
	mMatrixWidget->blockSignals(true);
	if (mConnectedMovingImage)
		mMatrixWidget->setMatrix(this->getMatrixFromBackend());
	else
		mMatrixWidget->setMatrix(Transform3D::Identity());
	mMatrixWidget->blockSignals(false);
}

} /* namespace cx */
