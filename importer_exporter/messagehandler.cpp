
#include "messagehandler.h"
using namespace elabtsaot;

MessageHandler::MessageHandler() :
    QAbstractMessageHandler(0) {}

QString MessageHandler::statusMessage() const { return m_description; }
int MessageHandler::line() const { return m_sourceLocation.line(); }
int MessageHandler::column() const { return m_sourceLocation.column(); }

void MessageHandler::handleMessage(QtMsgType type,
                                   QString const& description,
                                   QUrl const& identifier,
                                   QSourceLocation const& sourceLocation){
  Q_UNUSED(type);
  Q_UNUSED(identifier);
  m_messageType = type;
  m_description = description;
  m_sourceLocation = sourceLocation;
}
