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

#include "cxManualPatientRegistrationWidget.h"

#include <QLabel>
#include "cxPatientModelService.h"
#include "cxRegistrationService.h"
#include "cxTransform3DWidget.h"

namespace cx
{

ManualPatientRegistrationWidget::ManualPatientRegistrationWidget(RegServicesPtr services, QWidget* parent, QString objectName) :
				BaseWidget(parent, objectName, "Manual Patient Registration"),
				mVerticalLayout(new QVBoxLayout(this)),
				mServices(services)
{
	this->setToolTip("Set patient registration directly");
	mVerticalLayout->setMargin(0);
	mLabel = new QLabel("Patient Registration matrix rMpr");
	mVerticalLayout->addWidget(mLabel);
	mMatrixWidget = new Transform3DWidget(this);
	mVerticalLayout->addWidget(mMatrixWidget);
	connect(mMatrixWidget, SIGNAL(changed()), this, SLOT(matrixWidgetChanged()));
	connect(services->patient().get(), SIGNAL(rMprChanged()), this, SLOT(patientMatrixChanged()));
	mMatrixWidget->setEditable(true);

	mVerticalLayout->addStretch();

}


void ManualPatientRegistrationWidget::showEvent(QShowEvent* event)
{
	this->patientMatrixChanged();
}

///** Called when the matrix in the widget has changed.
// *  Perform registration.
// *
// */
void ManualPatientRegistrationWidget::matrixWidgetChanged()
{
	Transform3D rMpr = mMatrixWidget->getMatrix();
	mServices->registration()->addPatientRegistration(rMpr, "Manual Patient");
}

///** Called when the valid patient registration matrix in the system has changed.
// *  Perform registration.
// *
// */
void ManualPatientRegistrationWidget::patientMatrixChanged()
{
	mLabel->setText(this->getDescription());
	mMatrixWidget->blockSignals(true);
	mMatrixWidget->setMatrix(mServices->patient()->get_rMpr());
	mMatrixWidget->blockSignals(false);
}

bool ManualPatientRegistrationWidget::isValid() const
{
	return true;
}

QString ManualPatientRegistrationWidget::getDescription()
{
	if (this->isValid())
		return QString("<b>Patient Registration matrix rMpr</b>");
	else
		return "<Invalid matrix>";
}

} //cx
