#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkImagePlaneWidget.h>
#include <vtkRenderer.h>
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include "sscTestUtilities.h"
#include "sscDataManager.h"
#include "sscImage.h"
#include "sscAxesRep.h"
#include "sscSliceRep.h"

#include "sscVolumetricRep.h"
#include "sscSliceComputer.h"
#include "sscVector3D.h"
#include "sscTransform3D.h"
#include "sscToolRep3D.h"
#include "sscDummyToolManager.h"
#include "sscDummyTool.h"
#include "sscSliceProxy.h"
#include "sscSlicerRepSW.h"
#include "sscTexture3DSlicerRep.h"
#include "sscViewsWindow.h"

using ssc::Vector3D;
using ssc::Transform3D;


//---------------------------------------------------------
//---------------------------------------------------------

//---------------------------------------------------------
//---------------------------------------------------------

namespace {
	ssc::DummyToolPtr dummyTool()
	{
		return boost::dynamic_pointer_cast<ssc::DummyTool>(ssc::ToolManager::getInstance()->getDominantTool());
	}
}


ViewsWindow::ViewsWindow(QString displayText, bool showSliders) : mDisplayText(displayText)
{
	mDumpSpeedData = false;
	mRenderCount = 0;
	mTotalRender = 0;
	mTotalOther = 0;
	mLastRenderEnd = QTime::currentTime();
	
	QRect screen = qApp->desktop()->screen()->rect();
	//this->show();
	this->setGeometry(QRect(screen.bottomRight()*0.15, screen.bottomRight()*0.85));
	this->setCentralWidget( new QWidget(this) );

	start(showSliders);
}

void ViewsWindow::setDescription(const QString& desc)
{
	mAcceptanceBox->setText(desc);
}


ViewsWindow::~ViewsWindow()
{
}

ssc::View* ViewsWindow::generateGPUSlice(const QString& uid, ssc::ToolPtr tool, ssc::ImagePtr image, ssc::PLANE_TYPE plane)
{
	ssc::View* view = new ssc::View(centralWidget());
	mLayouts.insert(view);

	ssc::SliceProxyPtr proxy(new ssc::SliceProxy());
	proxy->setTool(tool);

	proxy->initializeFromPlane(plane, false, Vector3D(0,0,-1), false, 1, 0);

	ssc::Texture3DSlicerRepPtr rep = ssc::Texture3DSlicerRep::New(uid);
	//rep->setImage(image);
	rep->setViewportData(get_vpMs(view), getViewport(view));
	rep->setImages(std::vector<ssc::ImagePtr>(1, image));
	rep->setSliceProxy(proxy);

	view->addRep(rep);
	return view;
}

Transform3D ViewsWindow::get_vpMs(ssc::View* view) const
{
	Vector3D center_vp = getViewport(view).center();

	double zoomFactor = 1; // real magnification
	double scale = zoomFactor/mmPerPix(view);
	Transform3D S = createTransformScale(Vector3D(scale, scale, scale));
	Transform3D T = createTransformTranslate(center_vp);// center of viewport in viewport coordinates
	Transform3D M_vp_w = T*S; // first scale , then translate to center.
	return M_vp_w;
}

/**return the pixel viewport.
 */
ssc::DoubleBoundingBox3D ViewsWindow::getViewport(ssc::View* view) const
{
	QSize size = view->size();
	return ssc::DoubleBoundingBox3D(0, size.width(), 0, size.height(), 0, 1);
}

double ViewsWindow::mmPerPix(ssc::View* view) const
{
	QWidget* screen = qApp->desktop()->screen(qApp->desktop()->screenNumber(view));
	double r_h = (double)screen->heightMM()/(double)screen->geometry().height();
	double r_w = (double)screen->widthMM()/(double)screen->geometry().width();
	double retval = (r_h+r_w)/2.0;
	return retval;
}

void ViewsWindow::defineGPUSlice(const QString& uid, const QString& imageFilename, ssc::PLANE_TYPE plane, int r, int c)
{
	ssc::ToolManager* mToolmanager = ssc::DummyToolManager::getInstance();
	ssc::ToolPtr tool = mToolmanager->getDominantTool();

	ssc::ImagePtr image = loadImage(imageFilename);

	ssc::View* view = generateGPUSlice(uid, tool, image, plane);
	insertView(view, uid, imageFilename, r, c);
}


ssc::View* ViewsWindow::generateSlice(const QString& uid, ssc::ToolPtr tool, ssc::ImagePtr image, ssc::PLANE_TYPE plane)
{
	ssc::View* view = new ssc::View(centralWidget());
	mLayouts.insert(view);

	ssc::SliceProxyPtr proxy(new ssc::SliceProxy());
	proxy->setTool(tool);
	
	proxy->initializeFromPlane(plane, false, Vector3D(0,0,-1), false, 1, 0);
	
	ssc::SliceRepSWPtr rep = ssc::SliceRepSW::New(uid);
	rep->setImage(image);
	rep->setSliceProxy(proxy);

	view->addRep(rep);
	return view;
}

void ViewsWindow::defineSlice(const QString& uid, const QString& imageFilename, ssc::PLANE_TYPE plane, int r, int c)
{
	ssc::ToolManager* mToolmanager = ssc::DummyToolManager::getInstance();
	ssc::ToolPtr tool = mToolmanager->getDominantTool();

	ssc::ImagePtr image = loadImage(imageFilename);
	
	ssc::View* view = generateSlice(uid, tool, image, plane);
	insertView(view, uid, imageFilename, r, c);
}

ssc::ImagePtr ViewsWindow::loadImage(const QString& imageFilename)
{
	QString filename = ssc::TestUtilities::ExpandDataFileName(imageFilename);
	ssc::ImagePtr image = ssc::DataManager::getInstance()->loadImage(filename, filename, ssc::rtMETAIMAGE);
	Vector3D center = image->boundingBox().center();
	center = image->get_rMd().coord(center);
	ssc::DataManager::getInstance()->setCenter(center);
	
	// side effect: set tool movement box to data box,
	dummyTool()->setToolPositionMovementBB(image->boundingBox());
	
	return image;
}

void ViewsWindow::insertView(ssc::View* view, const QString& uid, const QString& volume, int r, int c)
{
	QVBoxLayout* layout = new QVBoxLayout;
	mSliceLayout->addLayout(layout, r,c);
	
	layout->addWidget(view);
	layout->addWidget(new QLabel(uid+" "+volume, this));
}

void ViewsWindow::define3D(const QString& imageFilename, int r, int c)
{
	QString uid = "3D";
	ssc::View* view = new ssc::View(centralWidget());
	mLayouts.insert(view);
	
	ssc::ImagePtr image = loadImage(imageFilename);

	// volume rep
	ssc::VolumetricRepPtr mRepPtr = ssc::VolumetricRep::New( image->getUid() );
	mRepPtr->setImage(image);
	mRepPtr->setName(image->getName());
	view->addRep(mRepPtr);
	
	// Tool 3D rep
	ssc::ToolManager* mToolmanager = ssc::DummyToolManager::getInstance();
	ssc::ToolPtr tool = mToolmanager->getDominantTool();
	ssc::ToolRep3DPtr toolRep = ssc::ToolRep3D::New( tool->getUid(), tool->getName() );
	toolRep->setTool(tool);
	view->addRep(toolRep);
	
	insertView(view, uid, imageFilename, r, c);
}


void ViewsWindow::start(bool showSliders)
{
	//ssc::AxesRepPtr axesRep = ssc::AxesRep::New("AxesRepUID");
	//view->addRep(axesRep);

	// Initialize dummy toolmanager.
	ssc::ToolManager* mToolmanager = ssc::DummyToolManager::getInstance();
	mToolmanager->configure();
	mToolmanager->initialize();
	mToolmanager->startTracking();

	ssc::ToolPtr tool = mToolmanager->getDominantTool();
	connect( tool.get(), SIGNAL( toolTransformAndTimestamp(Transform3D ,double) ), this, SLOT( updateRender()));

	//gui controll
	QVBoxLayout *mainLayout = new QVBoxLayout;
	this->centralWidget()->setLayout(mainLayout);

	mSliceLayout = new QGridLayout;

	mainLayout->addLayout(mSliceLayout);//Slice layout

	QHBoxLayout *controlLayout = new QHBoxLayout;
	controlLayout->addStretch();
	
	mAcceptanceBox = new ssc::AcceptanceBoxWidget(mDisplayText, this);
	controlLayout->addWidget(mAcceptanceBox);

	controlLayout->addStretch();
	mainLayout->addLayout(controlLayout); //Buttons

//	if (showSliders)
//	{
//		QVBoxLayout* slidersLayout = new QVBoxLayout;
//		mBrightnessSlider = new QSlider(Qt::Horizontal);
//		mBrightnessSlider->setTickInterval(10);
//		mContrastSlider= new QSlider(Qt::Horizontal);
//		mContrastSlider->setTickInterval(10);
//		slidersLayout->addWidget(new QLabel(tr("Brightness")) );
//		slidersLayout->addWidget(mContrastSlider);
//		slidersLayout->addWidget(new QLabel(tr("Contrasst")) );
//		slidersLayout->addWidget(mBrightnessSlider);
//		controlLayout->addLayout(slidersLayout);
//
//		connect(mContrastSlider, SIGNAL(sliderMoved(int)), this, SLOT(contrast(int)) );
//		connect(mBrightnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(brightness(int)) );
//		mContrastSlider->setMaximum(256);
//		mContrastSlider->setValue(256);
//		mBrightnessSlider->setMaximum(256);
//		mBrightnessSlider->setValue(128);
//	}
}

void ViewsWindow::contrast(int val)
{
//	ssc::ImagePtr image1 = ssc::DataManager::getInstance()->getImage(imageFileName1);	
//	
//	image1->getLookupTable2D().setWindow(val/1.0);	
}
	
void ViewsWindow::brightness(int val)
{
//	ssc::ImagePtr image1 = ssc::DataManager::getInstance()->getImage(imageFileName1);
//	image1->getLookupTable2D().setLevel(val/1.0);
}

void ViewsWindow::updateRender()
{
	if (mRenderCount==1)
	{
		mTotalRender = 0;
		mTotalOther = 0;
		++mRenderCount;
		return;
	}

	++mRenderCount;
	QTime pre = QTime::currentTime();
	int other = mLastRenderEnd.msecsTo(pre);

	for (LayoutMap::iterator iter=mLayouts.begin(); iter!=mLayouts.end(); ++iter)
		(*iter)->getRenderWindow()->Render();

	mLastRenderEnd = QTime::currentTime();

	int render = pre.msecsTo(mLastRenderEnd);
	int limit = 30;
	mTotalRender += render;
	mTotalOther += other;

	if (mDumpSpeedData)
	{
		if (render+other>limit) // warn if too much time spent rendering.
		{
			std::cout << "render: " << render << "/" << other << std::endl;
		}
		if (mRenderCount%50==0)
		{
			std::cout << "averagerender: " << mTotalRender/mRenderCount << "/" << mTotalOther/mRenderCount << std::endl;
		}
	}
}


