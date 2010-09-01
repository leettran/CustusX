#include "cxViewWrapper.h"

#include <QMenu>
#include "sscMessageManager.h"
#include "cxDataManager.h"
#include "cxViewGroup.h" //for class Navigation

namespace cx
{

SyncedValue::SyncedValue(QVariant val) : mValue(val) {}
SyncedValuePtr SyncedValue::create(QVariant val)
{
  return SyncedValuePtr(new SyncedValue(val));
}
void SyncedValue::set(QVariant val)
{
  //std::cout << "emit changed new=" << val.toDouble() << ", old=" << mValue.toDouble() << std::endl;
  if (mValue==val)
    return;
  mValue = val;
  //std::cout << "emit changed" << std::endl;
  emit changed();
}
QVariant SyncedValue::get() const
{
  return mValue;
}


void ViewWrapper::contextMenuSlot(const QPoint& point)
{
  QWidget* sender = dynamic_cast<QWidget*>(this->sender());
  QPoint pointGlobal = sender->mapToGlobal(point);
  QMenu contextMenu(sender);

  //add actions to the actiongroups and the contextmenu
  std::map<std::string, std::string> imageUidsAndNames = ssc::dataManager()->getImageUidsAndNames();
  std::map<std::string, std::string> meshUidsAndNames = ssc::dataManager()->getMeshUidsWithNames();

  std::vector<ssc::ImagePtr> images = this->getImages();
  std::map<std::string, std::string>::iterator imageIt = imageUidsAndNames.begin();
  while(imageIt != imageUidsAndNames.end())
  {
    const QString uid = imageIt->first.c_str();
    const QString name = imageIt->second.c_str();

    QAction* imageAction = new QAction(name, &contextMenu);
    imageAction->setData(QVariant(uid));
    imageAction->setCheckable(true);
    connect(imageAction, SIGNAL(triggered()), this, SLOT(imageActionSlot()));
    contextMenu.addAction(imageAction);
    imageAction->setChecked(std::count(images.begin(), images.end(), ssc::dataManager()->getImage(imageIt->first)));

    imageIt++;
  }
  
  contextMenu.addSeparator();

  std::vector<ssc::MeshPtr> meshes = this->getMeshes();
  std::map<std::string, std::string>::iterator meshIt = meshUidsAndNames.begin();
  while(meshIt != meshUidsAndNames.end())
  {
    const QString uid = meshIt->first.c_str();
    const QString name = meshIt->second.c_str();
    
    QAction* meshAction = new QAction(name, &contextMenu);
    meshAction->setData(QVariant(uid));
    meshAction->setCheckable(true);
    connect(meshAction, SIGNAL(triggered()), this, SLOT(meshActionSlot()));
    contextMenu.addAction(meshAction);
    meshAction->setChecked(std::count(meshes.begin(), meshes.end(), ssc::dataManager()->getMesh(meshIt->first)));
    
    meshIt++;
  }

  //append specific info from derived classes
  this->appendToContextMenu(contextMenu);

  contextMenu.exec(pointGlobal);
}

void ViewWrapper::imageActionSlot()
{
  //ssc::messageManager()->sendInfo("ViewWrapper::imageActionSlot()");
  QAction* theAction = static_cast<QAction*>(sender());
  if(!theAction)
    return;

  QString imageUid = theAction->data().toString();
  ssc::ImagePtr image = ssc::dataManager()->getImage(imageUid.toStdString());

  if (theAction->isChecked())
  {
    this->addImage(image);
    ssc::dataManager()->setActiveImage(image);
  }
  else
  {
    //ssc::messageManager()->sendInfo("ViewGroup::ViewWrapper - remove - [" 
    //                                + string_cast(imageUid) + "]" 
    //                                + string_cast(image.get()));
    this->removeImage(image);
    ssc::dataManager()->setActiveImage(ssc::ImagePtr());
  }

  Navigation().centerToGlobalImageCenter(); // reset center for convenience
}
  
void ViewWrapper::meshActionSlot()
{
  ssc::messageManager()->sendInfo("ViewWrapper::meshActionSlot()");
  QAction* theAction = static_cast<QAction*>(sender());
  if(!theAction)
    return;
  
  QString meshUid = theAction->data().toString();
  ssc::MeshPtr mesh= ssc::dataManager()->getMesh(meshUid.toStdString());

  if (theAction->isChecked())
  {
    this->addMesh(mesh);
    //ssc::dataManager()->setActiveMesh(mesh);
  }
  else
  {
    this->removeMesh(mesh);
    //ssc::dataManager()->setActiveMesh(ssc::MeshPtr);
    //theAction->setChecked(false);
  }
  
  Navigation().centerToGlobalImageCenter(); // reset center for convenience
}
  
void ViewWrapper::connectContextMenu(ssc::View* view)
{
   connect(view, SIGNAL(customContextMenuRequested(const QPoint &)),
       this, SLOT(contextMenuSlot(const QPoint &)));
}
}//namespace cx
