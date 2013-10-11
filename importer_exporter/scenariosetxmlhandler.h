/*!
\file scenariosetxmlhandler.h
\brief Definition file for class ScenarioSetXMLHandler

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCENARIOSETXMLHANDLER_H
#define SCENARIOSETXMLHANDLER_H

#include <QXmlDefaultHandler>
#include <QVector>
class QString;

namespace elabtsaot{

class Event;
class Scenario;
class ScenarioSet;

// This class provides the XML parser for importing schematics from XML files
class ScenarioSetXMLHandler : public QXmlDefaultHandler{

 public:

  ScenarioSetXMLHandler( ScenarioSet* scs );
  // Implementation of virtual functions
  bool characters(QString const& ch);
  bool startElement(QString const& namespaceURI, QString const& localName,
                    QString const& qName, QXmlAttributes const& atts);
  bool endElement(QString const& namespaceURI, QString const& localName,
                  QString const& qName);

  // Getters & setters
  int set_scs( ScenarioSet* scs );
  ScenarioSet* scs() const;

 private:

  //! Reset handler to its first stage
  void _resetScenario();
  void _resetEvent();

  Scenario* _sce;
  Event* _evn;
  ScenarioSet* _scs;             // pointer to a scenario set to be created
                                 // by parsing the xml file
  QVector<QString> _tags;        // Keep track of tags that apply in any moment

};

} // end of namespace elabtsaot

#endif // SCENARIOSETXMLHANDLER_H
