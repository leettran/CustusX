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

#include <cxTransform3DWidget.h>

#include <QTextEdit>
#include <QLayout>
#include <QLabel>
#include <QFontMetrics>

#include "cxTypeConversions.h"

#include "cxDoubleProperty.h"
#include "cxDoubleWidgets.h"
#include <cmath>
#include "cxMousePadWidget.h"

#include <QtWidgets>


#include "boost/bind.hpp"
#include "libQtSignalAdapters/Qt2Func.h"
#include "libQtSignalAdapters/ConnectionFactories.h"

namespace cx
{

class MatrixTextEdit : public QTextEdit
{
public:
  MatrixTextEdit(QWidget* parent=NULL) : QTextEdit(parent)  {}
  QSize minimumSizeHint() const { return sizeHint(); }
  QSize sizeHint() const
  {
    Transform3D M = createTransformRotateX(M_PI_4) *
        createTransformRotateZ(M_PI_4) *
        createTransformTranslate(Vector3D(1,2,M_PI));

    QString text = qstring_cast(M).split("\n")[0];
    QRect rect = QFontMetrics(this->font()).boundingRect(text);
    QSize s(rect.width()*1.0+5, 4*rect.height()*1.2+5);
    return s;
  }
};

//template<class T>
//QAction* Transform3DWidget::createAction(QLayout* layout, QString iconName, QString text, QString tip, T slot)
//{
//  QAction* action = new QAction(QIcon(iconName), text, this);
//  action->setStatusTip(tip);
//  action->setToolTip(tip);
//  connect(action, SIGNAL(triggered()), this, slot);
//  QToolButton* button = new QToolButton();
//  button->setDefaultAction(action);
//  layout->addWidget(button);
//  return action;
//}


Transform3DWidget::Transform3DWidget(QWidget* parent) :
	BaseWidget(parent, "transform_3d_widget", "Transform 3D")
{
	this->setToolTip("Display and manipulate an affine (rotation+translation) matrix");
  recursive = false;
  mBlockChanges = false;
  //layout
  QVBoxLayout* toptopLayout = new QVBoxLayout(this);
  toptopLayout->setMargin(4);
  QHBoxLayout* mLayout = new QHBoxLayout;
  mLayout->setMargin(0);
  toptopLayout->addLayout(mLayout);

  mTextEdit = new MatrixTextEdit;
  mTextEdit->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);
  mTextEdit->setLineWrapMode(QTextEdit::NoWrap);
  mTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  mTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(mTextEdit, SIGNAL(textChanged()), this, SLOT(textEditChangedSlot()));

  mLayout->addWidget(mTextEdit, 1);

  QVBoxLayout* buttonLayout = new QVBoxLayout;
  mLayout->addLayout(buttonLayout);
  buttonLayout->setMargin(0);

  mEditAction = this->createAction(this,
      QIcon(":/icons/open_icon_library/system-run-5.png"),
      "Edit",
      "Toggle Edit Matrix",
      SLOT(toggleEditSlot()),
	  buttonLayout);
  mEditAction->setCheckable(true);

  mInvertAction = this->createAction(this,
	  QIcon(":/icons/matrix_inverse.png"),
	  "Invert",
	  "Toggle Invert Matrix",
	  SLOT(toggleInvertSlot()),
	  buttonLayout);
  mInvertAction->setCheckable(true);
  mInvertAction->setChecked(false);
  this->updateInvertAction();

//  mLayout->addStretch();

  aGroupBox = new QFrame(this);
  QVBoxLayout* aLayout = new QVBoxLayout;
  aGroupBox->setLayout(aLayout);
  aGroupBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  aGroupBox->setLineWidth(3);
  aLayout->setMargin(4);
  toptopLayout->addWidget(aGroupBox);

  this->addAngleControls("xAngle", "X Angle", 0, aLayout);
  this->addAngleControls("yAngle", "Y Angle", 1, aLayout);
  this->addAngleControls("zAngle", "Z Angle", 2, aLayout);

  tGroupBox = new QFrame(this);
  QVBoxLayout* tLayout = new QVBoxLayout;
  tGroupBox->setLayout(tLayout);
  tGroupBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  tGroupBox->setLineWidth(3);
  tLayout->setMargin(4);
  toptopLayout->addWidget(tGroupBox);

  this->addTranslationControls("xTranslation", "X", 0, tLayout);
  this->addTranslationControls("yTranslation", "Y", 1, tLayout);
  this->addTranslationControls("zTranslation", "Z", 2, tLayout);

  this->setMatrixInternal(Transform3D::Identity());

  toptopLayout->addStretch();

  this->setEditable(false);
}

void Transform3DWidget::textEditChangedSlot()
{
  bool ok = false;
  Transform3D M = Transform3D::fromString(mTextEdit->toPlainText(), &ok);
  // ignore setting if invalid matrix or no real change done (hopefully, this allows trivial editing without text reset)
  if (!ok)
    return;
  if (similar(M, this->getMatrixInternal()))
    return;

  this->setMatrixInternal(M);
}

void Transform3DWidget::toggleEditSlot()
{
  bool visible = tGroupBox->isVisible();
  this->setEditable(!visible);
}


void Transform3DWidget::toggleInvertSlot()
{
	// the interpretation of matrix is dependent on mInvertAction->isChecked()!
	mDecomposition.reset(mDecomposition.getMatrix().inverse());
	this->setModified();
//	this->updateInvertAction();
}

void Transform3DWidget::updateInvertAction()
{
	if (mInvertAction->isChecked())
	{
		this->setActionText(mInvertAction, "Inverted Matrix", "The matrix is shown inverted");
	}
	else
	{
		this->setActionText(mInvertAction, "Noninverted Matrix", "The matrix is shown as is. Press to show inverted");
	}
}

void Transform3DWidget::setActionText(QAction* action, QString text, QString tip)
{
	if (tip.isEmpty())
		tip = text;
  action->setText(text);
  action->setStatusTip(tip);
  action->setWhatsThis(tip);
  action->setToolTip(tip);
}

void Transform3DWidget::setEditable(bool edit)
{
  mTextEdit->setReadOnly(!edit);
  aGroupBox->setVisible(edit);
  tGroupBox->setVisible(edit);
}

void Transform3DWidget::addAngleControls(QString uid, QString name, int index, QVBoxLayout* layout)
{
  QHBoxLayout* hLayout = new QHBoxLayout;

  DoublePropertyPtr adapter = DoubleProperty::initialize(uid, name, "", 0, DoubleRange(-M_PI,M_PI,M_PI/180),1);
  connect(adapter.get(), SIGNAL(changed()), this, SLOT(changedSlot()));
  adapter->setInternal2Display(180/M_PI);
  hLayout->addWidget(new SpinBoxGroupWidget(this, adapter));

  QSize mMinBarSize = QSize(20,20);
  MousePadWidget* pad = new MousePadWidget(this, mMinBarSize);
  pad->setFixedYPos(true);
  hLayout->addWidget(pad);

  // use QtSignalAdapters library to work magic:
  QtSignalAdapters::connect1<void(QPointF)>(pad, SIGNAL(mouseMoved(QPointF)),
      boost::bind(&Transform3DWidget::rotateSlot, this, _1, index));

  layout->addLayout(hLayout);
  mAngleAdapter[index] = adapter;
}

void Transform3DWidget::addTranslationControls(QString uid, QString name, int index, QVBoxLayout* layout)
{
  QHBoxLayout* hLayout = new QHBoxLayout;

  DoublePropertyPtr adapter = DoubleProperty::initialize(uid, name, "", 0, DoubleRange(-10000,10000,0.1),1);
  connect(adapter.get(), SIGNAL(changed()), this, SLOT(changedSlot()));
  adapter->setInternal2Display(1.0);
  hLayout->addWidget(new SpinBoxGroupWidget(this, adapter));

  QSize mMinBarSize = QSize(20,20);
  MousePadWidget* pad = new MousePadWidget(this, mMinBarSize);
  pad->setFixedYPos(true);
  hLayout->addWidget(pad);

  // use QtSignalAdapters library to work magic:
  QtSignalAdapters::connect1<void(QPointF)>(pad, SIGNAL(mouseMoved(QPointF)),
      boost::bind(&Transform3DWidget::translateSlot, this, _1, index));

  layout->addLayout(hLayout);
  mTranslationAdapter[index] = adapter;
}

void Transform3DWidget::rotateSlot(QPointF delta, int index)
{
  double scale = M_PI_2;
  double factor = scale * delta.x();
  double current = mAngleAdapter[index]->getValue();
  mAngleAdapter[index]->setValue(current + factor);
}

void Transform3DWidget::translateSlot(QPointF delta, int index)
{
  double scale = 20;
  double factor = scale * delta.x();
  double current = mTranslationAdapter[index]->getValue();
  mTranslationAdapter[index]->setValue(current + factor);
}


Transform3DWidget::~Transform3DWidget()
{
}

void Transform3DWidget::setMatrix(const Transform3D& M)
{
	this->setMatrixInternal(this->convertToFromExternal(M));
}

Transform3D Transform3DWidget::getMatrix() const
{
  return this->convertToFromExternal(this->getMatrixInternal());
}

Transform3D Transform3DWidget::convertToFromExternal(const Transform3D& M) const
{
	if (mInvertAction->isChecked())
	{
		return M.inverse();
	}
	else
	{
		return M;
	}
}

void Transform3DWidget::setMatrixInternal(const Transform3D& M)
{
	mDecomposition.reset(M);
	this->setModified();
	emit changed();
}

Transform3D Transform3DWidget::getMatrixInternal() const
{
	return mDecomposition.getMatrix();
}

// http://en.wikipedia.org/wiki/Rotation_matrix
// http://en.wikipedia.org/wiki/Rotation_representation_(mathematics)#Conversion_formulae_between_representations

void Transform3DWidget::changedSlot()
{
  if (recursive || mBlockChanges)
    return;
  recursive = true;
  Vector3D xyz(mAngleAdapter[0]->getValue(),mAngleAdapter[1]->getValue(),mAngleAdapter[2]->getValue());
  mDecomposition.setAngles(xyz);

  Vector3D t(mTranslationAdapter[0]->getValue(),mTranslationAdapter[1]->getValue(),mTranslationAdapter[2]->getValue());
  mDecomposition.setPosition(t);

  this->setModified();
  emit changed();
  recursive = false;
}

namespace
{
	/**Wrap the angle into the interval -M_PI..M_PI
	 *
	 */
	double wrapAngle(double angle)
	{
		angle = fmod(angle, M_PI * 2);
		if (angle > M_PI)
			angle -= M_PI * 2;
		if (angle < -M_PI)
			angle += M_PI * 2;
		return angle;
	}
}

void Transform3DWidget::prePaintEvent()
{
  QString M = qstring_cast(this->getMatrixInternal());

  mTextEdit->blockSignals(true);
  int textPos = mTextEdit->textCursor().position();
  mTextEdit->setText(M);
  QTextCursor cursor = mTextEdit->textCursor();
  cursor.setPosition(textPos);
  mTextEdit->setTextCursor(cursor);
  mTextEdit->blockSignals(false);

  Vector3D xyz = mDecomposition.getAngles();

  mBlockChanges = true;

  mAngleAdapter[0]->setValue(wrapAngle(xyz[0]));
  mAngleAdapter[1]->setValue(wrapAngle(xyz[1]));
  mAngleAdapter[2]->setValue(wrapAngle(xyz[2]));

  Vector3D t = mDecomposition.getPosition();
  mTranslationAdapter[0]->setValue(t[0]);
  mTranslationAdapter[1]->setValue(t[1]);
  mTranslationAdapter[2]->setValue(t[2]);

  this->updateInvertAction();

  mBlockChanges = false;
}
}
