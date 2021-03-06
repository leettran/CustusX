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

#ifndef CXLOGFILE_H
#define CXLOGFILE_H

#include "cxResourceExport.h"
#include "cxLogMessage.h"

namespace cx
{

/**\brief Log file, format, read and write.
 *
 * \addtogroup cx_resource_core_logger
 */
class LogFile
{
public:
	explicit LogFile();
	static LogFile fromChannel(QString path, QString channel);
	static LogFile fromFilename(QString filename);
	virtual ~LogFile() {}

	void writeHeader();
	void write(Message message);
	bool isWritable() const;
	QString getFilename() const;

	std::vector<Message> readMessages();

private:
	QString mPath;
	QString mChannel;
	int mFilePosition;
	QDateTime mInitTimestamp;

	Message readMessageFirstLine(QString line);
	MESSAGE_LEVEL readMessageLevel(QString line);
	QRegExp getRX_Timestamp() const;
	QString formatMessage(Message msg);
	bool appendToLogfile(QString filename, QString text);
	QString readFileTail();
//	QString removeEarlierSessionsAndSetStartTime(QString text);
//	std::vector<std::pair<QDateTime, QString> > splitIntoSessions(QString text);
	QString timestampFormat() const;
	QDateTime readTimestampFromSessionStartLine(QString text);

	void parseTimestamp(QString text, Message* retval);
	void parseThread(QString text, Message* retval);
	void parseSourceFileLine(QString text, Message* retval);
	void parseSourceFunction(QString text, Message* retval);
	QString getIndex(const QStringList& list, int index);

};

} //namespace cx


#endif // CXLOGFILE_H
