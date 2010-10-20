#ifndef MESSAGEMANAGER_H_
#define MESSAGEMANAGER_H_

#include <QObject>
#include <QDateTime>
#include <QFile>
#include "boost/shared_ptr.hpp"
#include "sscDefinitions.h"

class QString;
class QDomNode;
class QDomDocument;
class QFile;
class QTextStream;

namespace ssc
{

class Message
{
public:
  Message(QString text, MESSAGE_LEVEL messageLevel=mlDEBUG, int timeoutTime=0);
  ~Message();
  QString getPrintableMessage();
  MESSAGE_LEVEL getMessageLevel();
  QString getText();
  QDateTime* getTimeStamp();
  int getTimeout();

private:
  QString mText;
  MESSAGE_LEVEL mMessageLevel;
  int mTimeoutTime;
  QDateTime mTimeStamp;
};


/**
 * \class MessageManager
 *
 * \brief This class handles delivering messages to a statusbar from all the other
 * classes.
 *
 * \author Janne Beate Lervik Bakeng, SINTEF Health Research
 * \date 16.10.2008
 *
 *TODO Sender should be added to the message
 *TODO Enable/disable info, warnings and errors
 *
 */
class MessageManager : public QObject
{
  Q_OBJECT

public:
  static MessageManager* getInstance(); ///< Returns a reference to the only MessageManager that exists.
  static void destroyInstance(); ///< Should be called by the object that made the MessageManager.

  void setLoggingFolder(QString absoluteLoggingFolderPath);

  void sendInfo(QString info); ///< Used to report successful operations.
  void sendWarning(QString warning); ///< The program does not need to terminate, but the user might need to do something.
  void sendError(QString error); ///< The program (might) need to terminate
  void sendDebug(QString debug); ///< Used to output debug info

  void sendMessage(QString text, MESSAGE_LEVEL messageLevel=mlDEBUG, int timeout=0);

signals:
  void emittedMessage(const QString& text, int timeout); ///< The signal the owner of a statusbar should listen to. DEPRECATED!
  void emittedMessage(Message message); ///< The signal the user should listen to!

private:
  void initialize();
  MessageManager();
  ~MessageManager();
  MessageManager(const MessageManager&);
  MessageManager& operator=(const MessageManager&);

  void sendMessage(QString text, int timeout); ///< Emits the signal that actually sends the message.
  bool openLogging(QFile::OpenMode mode);

  typedef boost::shared_ptr<class SingleStreamerImpl> SingleStreamerImplPtr;
  SingleStreamerImplPtr mCout;
  SingleStreamerImplPtr mCerr;

  QString mAbsoluteLoggingFolderPath;
  QFile* mConsoleFile;
  QTextStream* mConsoleStream;

  static MessageManager *mTheInstance; ///< The unique MessageManager.
};

/**Shortcut for accessing the message manager instance.
 */
MessageManager* messageManager();

} //namespace ssc

#endif /* MESSAGEMANAGER_H_ */
