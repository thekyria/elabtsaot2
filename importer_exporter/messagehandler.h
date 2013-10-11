/*!
\file messagehandler.h
\brief Definition file for class MessageHandler

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QAbstractMessageHandler>

namespace elabtsaot{

//! Implementation of Message Handler to catch XML errors
class MessageHandler : public QAbstractMessageHandler{

 public:

  MessageHandler();
  QString statusMessage() const;
  int line() const;
  int column() const;

 protected:

  virtual void handleMessage(QtMsgType type, QString const& description,
                             QUrl const& identifier,
                             QSourceLocation const& sourceLocation);

 private:

  QtMsgType m_messageType;
  QString m_description;
  QSourceLocation m_sourceLocation;

};

} // end of namespace elabtsaot

#endif // MESSAGEHANDLER_H
