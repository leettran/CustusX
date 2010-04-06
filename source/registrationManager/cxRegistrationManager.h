#ifndef CXREGISTRATIONMANAGER_H_
#define CXREGISTRATIONMANAGER_H_

#include <map>
#include <QObject>
#include "vtkSmartPointer.h"
#include "sscImage.h"

typedef vtkSmartPointer<class vtkDoubleArray> vtkDoubleArrayPtr;
typedef vtkSmartPointer<class vtkPoints> vtkPointsPtr;
typedef vtkSmartPointer<class vtkLandmarkTransform> vtkLandmarkTransformPtr;

namespace cx
{
class ToolManager;
class MessageManager;

/**\
 * \class RegistrationManager
 *
 * \brief This class manages (image- and patient-) registration specific details.
 *
 * \date Feb 4, 2009
 * \author: Janne Beate Bakeng, SINTEF
 */
class RegistrationManager : public QObject
{
  Q_OBJECT

public:
  typedef std::pair<std::string, bool> StringBoolPair; ///< name and if the point is active or not
  typedef std::map<int, StringBoolPair> NameListType; ///< landmarkindex, name and if point is active or not

  static RegistrationManager* getInstance(); ///< get the only instance of this class

  void setMasterImage(ssc::ImagePtr image); ///< set a master image used when registrating
  ssc::ImagePtr getMasterImage(); ///< get the master image
  bool isMasterImageSet(); ///< check if the master image is set

  void setGlobalPointSet(vtkDoubleArrayPtr pointset); ///< set a global point set used to register against
  vtkDoubleArrayPtr getGlobalPointSet(); ///< get the global point set

  void setGlobalPointSetNameList(NameListType nameList); ///< set user specific names on the global points
  NameListType getGlobalPointSetNameList(); ///< get a map of the names

  void setManualPatientRegistration(ssc::Transform3DPtr patientRegistration); ///< used for when a user wants to
  ssc::Transform3DPtr getManualPatientRegistration(); ///< get the manually set patient registration
  void resetManualPatientientRegistration(); ///< tells the system not to use a manually added patient registration, after it uses landmarks for patient registration instead

  ssc::Transform3DPtr getManualPatientRegistrationOffset(); ///< get the offset transform that moves the patient registration
  void resetOffset(); ///< removes the offset, after it uses landmarks for patient registration instead

  void doPatientRegistration(); ///< registrates the master image to the patient
  void doImageRegistration(ssc::ImagePtr image); ///< registrates the image to the master image

public slots:
  void setGlobalPointsNameSlot(int index, std::string name); ///< set the points (user) name
  void setGlobalPointsActiveSlot(int index, bool active); ///< set if the point should be used in matrix calc or not
  void setManualPatientRegistrationOffsetSlot(ssc::Transform3DPtr offset); ///< transform for (slightly) moving a patient registration

signals:
  void imageRegistrationPerformed();
  void patientRegistrationPerformed();

protected:
  RegistrationManager(); ///< use getInstance instead
  ~RegistrationManager(); ///< destructor

  static RegistrationManager* mCxInstance; ///< the only instance of this class

  ToolManager* mToolManager; ///< interface to the navigation system

  ssc::ImagePtr mMasterImage; ///< the master image used to register all other images against
  vtkDoubleArrayPtr mGlobalPointSet; ///< the pointset used when doing the registration
  NameListType mGlobalPointSetNameList; ///< names of the points in the global point set

  ssc::Transform3DPtr mPatientRegistrationOffset; ///< manually set offset for that will be added to the patientregistration
  ssc::Transform3DPtr mManualPatientRegistration; ///< patient registration loaded from file

private:
  RegistrationManager(RegistrationManager const&); ///< not implemented
  RegistrationManager& operator=(RegistrationManager const&); ///< not implemented
};
/**Shortcut for accessing the registrationmanager instance.
 */
RegistrationManager* registrationManager();
}//namespace cx

#endif /* CXREGISTRATIONMANAGER_H_ */
