/*!
\file schematicxmlhandler.h
\brief Definition file for class SchematicXMLHandler

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef SCHEMATICXMLHANDLER_H
#define SCHEMATICXMLHANDLER_H

#include <QXmlDefaultHandler>
#include <QVector>
class QString;

namespace elabtsaot{

class PwsSchematicModel;
class PwsSchematicModelElement;

// This class provides the XML parser for importing schematics from XML files
class SchematicXMLHandler : public QXmlDefaultHandler{

 public:

  SchematicXMLHandler( PwsSchematicModel* smd );
  // Implementation of virtual functions
  bool characters(QString const& ch);
  bool startElement(QString const& namespaceURI, QString const& localName,
                    QString const& qName, QXmlAttributes const& atts);
  bool endElement(QString const& namespaceURI, QString const& localName,
                  QString const& qName);

  // Getters & setters
  int set_smd( PwsSchematicModel* smd );
  PwsSchematicModel* smd() const;

 private:

  //! Reset handler to its first stage
  void _resetState();

  PwsSchematicModelElement* _el;
  PwsSchematicModel* _smd;       // pointer to a schematic model to be created
                                 // by parsing the xml file
  QVector<QString> _tags;        // Keep track of tags that apply in any moment

};

} // end of namespace elabtsaot

#endif // SCHEMATICXMLHANDLER_H
