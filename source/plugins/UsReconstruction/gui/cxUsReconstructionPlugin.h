/*
 * cxUsReconstructionPlugin.h
 *
 *  Created on: Jun 15, 2011
 *      Author: christiana
 */

#ifndef CXUSRECONSTRUCTIONPLUGIN_H_
#define CXUSRECONSTRUCTIONPLUGIN_H_

#include "cxPluginBase.h"

namespace ssc
{
	typedef boost::shared_ptr<class ReconstructManager> ReconstructManagerPtr;
}

namespace cx
{

class UsReconstructionPlugin : public PluginBase
{
	Q_OBJECT
public:
	UsReconstructionPlugin();
	virtual ~UsReconstructionPlugin();

  ssc::ReconstructManagerPtr getReconstructer() { return mReconstructer; }
  std::vector<PluginWidget> createWidgets() const;

private:
  ssc::ReconstructManagerPtr mReconstructer;

private slots:
	void patientChangedSlot();

};

typedef boost::shared_ptr<UsReconstructionPlugin> UsReconstructionPluginPtr;

}

#endif /* CXUSRECONSTRUCTIONPLUGIN_H_ */
