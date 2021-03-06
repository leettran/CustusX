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

#ifndef CXFILTER_H
#define CXFILTER_H

#include "cxResourceFilterExport.h"

#include <vector>
#include <QObject>

#include "cxPresets.h"
#include "cxForwardDeclarations.h"
class QDomElement;

#define FilterService_iid "cx::Filter"

namespace cx
{
typedef boost::shared_ptr<class SelectDataStringPropertyBase> SelectDataStringPropertyBasePtr;

typedef boost::shared_ptr<class Filter> FilterPtr;


/** Base class for CustusX filters/algorithms
 *
 * Intended for use in a multithreaded environment.
 *
 * The methods getHelp(), getName(), getInputTypes(), getOutputTypes(),
 * getOptions() can be used outside of actual filter execution for
 * initialization and gui interaction.
 *
 * preProcess(), execute() and postProcess() must be called in sequence
 * and together executes the algorithm. They work on a copy of the input
 * data (the input volumes themselved are not copied, only pointers and options).
 *
 *
 * \ingroup cxResourceAlgorithms
 * \date Nov 16, 2012
 * \author christiana
 */
class cxResourceFilter_EXPORT Filter : public QObject
{
	Q_OBJECT

public:
    Filter();
	virtual ~Filter() {}

	/**
	  *  Return a unique string for this algorithm.
	  */
	virtual QString getUid() const  = 0;
	/**
	  *  Initialize algorithm
	  *  Fill options for this algorithm.
	  *  Input is the root node for this algo, filled with stored settings (if any).
	  *
	  * Set uid - if no uid is given, a default based on class name is used.
	  */
	virtual void initialize(QDomElement root, QString uid = "") = 0;
	/**
	  *  Return the type of this algorithm. Need to match id string in user documentation
	  */
	virtual QString getType() const  = 0;
	/**
	  *  Return a descriptive name for this algorithm.
	  */
	virtual QString getName() const = 0;
	/**
	  *  Return a help text describing algorithm usage.
	  */
	virtual QString getHelp() const = 0;
	/**
	  *  Fill options for this algorithm.
	  *  The options use the input xml noe from init to fill.
	  */
	virtual std::vector<PropertyPtr> getOptions() = 0;
	/**
	  *  List of input arguments.
	  */
	virtual std::vector<SelectDataStringPropertyBasePtr> getInputTypes() = 0;
	/**
	  *  Return a help text describing algorithm usage.
	  */
	virtual std::vector<SelectDataStringPropertyBasePtr> getOutputTypes() = 0;

	/**
	 * Checks wheter the filter has defined a set of presets.
	 */
	virtual bool hasPresets() = 0;

	/**
	 * Returns the filters presets.
	 */
	virtual PresetsPtr getPresets() = 0;
	/**
	 * Generates a preset from the filters currently set options.
	 */
	virtual QDomElement generatePresetFromCurrentlySetOptions(QString name) = 0;
	/**
	  * Set Active state.
	  * Active filters are used by the ui (or similar) and can interact
	  * with the system, for example by showing a preview.
	  *
	  * Inactive filters should no interact with the system.
	  */
	virtual void setActive(bool on) = 0;
	/**
	  * Perform main thread preprocessing. Copies input data from options and
	  * the input adapters into thread-safe storage.
	  * Assumes getOptions(), getInputTypes(), getOutputTypes()
	  * has been called (this initializes options)
	  *
	  * \return success.
	  */
	virtual bool preProcess() = 0;
	/**
	  * Execute filter with the given inputs and produce an output.
	  * This method is threadable - can be executed in a secondary thread.
	  * Assumes preprocess has been called.
	  *
	  * \return success.
	  */
	virtual bool execute() = 0;
	/**
	  * Perform postprocessing tasks and set result from calculation
	  * into the output adapters.
	  *
	  * Must be called from the main thread.
	  * Assumes execute() has been called.
	  *
	  *  \return success.
	  *
	  */
	virtual bool postProcess() = 0;

public slots:
	/**
	 * Ask the filter to load a preset.
	 */
	virtual void requestSetPresetSlot(QString name) = 0;

signals:
	/**
	 * Signals that the filters internal structures has changed.
	 */
	void changed();

};

} // namespace cx

Q_DECLARE_INTERFACE(cx::Filter, FilterService_iid)

#endif // CXFILTER_H
