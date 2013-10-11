/*!
\file powersystemxmlhandler.h
\brief Definition file for class PowersystemXMLHandler

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef POWERSYSTEMXMLHANDLER_H
#define POWERSYSTEMXMLHANDLER_H

#include <QXmlDefaultHandler>
#include <QVector>
class QString;

namespace elabtsaot{

class Powersystem;
class Bus;
class Branch;
class Generator;
class Load;

// This class provides the XML parser for importing powersystems from XML files
class PowersystemXMLHandler : public QXmlDefaultHandler{

 public:

  // ------------ functions ------------
  PowersystemXMLHandler( Powersystem* p_ps );
  // Implementation of virtual functions
  bool characters(QString const& ch);
  bool startElement(QString const& namespaceURI, QString const& localName,
                    QString const& qName, QXmlAttributes const& atts);
  bool endElement(QString const& namespaceURI, QString const& localName,
                  QString const& qName);

  // Getters & setters
  int set_p_ps( Powersystem* p_ps );
  Powersystem* p_ps() const;

 private:

  //! Reset handler to its first stage
  void _resetState();

  // ------------ variables ------------
  Powersystem* _p_ps;     // pointer to a virgin powersystem to be created by
                          // parsing the xml file
  Bus* _p_bus;
  Branch* _p_br;
  Generator* _p_gen;
  Load* _p_load;
  QVector<QString> _tags; // Keep track of tags that apply in a any moment

};

} // end of namespace elabtsaot

#endif // POWERSYSTEMXMLHANDLER_H
