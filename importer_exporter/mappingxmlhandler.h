/*!
\file mappingxmlhandler.h
\brief Definition file for class MappingXMLHandler

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef MAPPINGXMLHANDLER_H
#define MAPPINGXMLHANDLER_H

#include <QXmlDefaultHandler>
#include <QVector>
class QString;

namespace elabtsaot{

class PwsMapperModel;
class PwsMapperModelElement;

// This class provides the XML parser for importing mappings from XML files
class MappingXMLHandler : public QXmlDefaultHandler{

 public:

  MappingXMLHandler(PwsMapperModel* mmd=0);
  // Implementation of virtual functions
  bool characters(QString const& ch);
  bool startElement(QString const& namespaceURI, QString const& localName,
                    QString const& qName, QXmlAttributes const& atts);
  bool endElement(QString const& namespaceURI, QString const& localName,
                  QString const& qName);

  // Getters & setters
  int set_mmd( PwsMapperModel* mmd );
  PwsMapperModel* mmd() const;

 private:

  //! Reset handler to its first stage
  void _resetState();

  PwsMapperModelElement* _el;
  PwsMapperModel* _mmd;       // pointer to a virgin powersystem mapping model
                               // to be created by parsing the xml file
  QVector<QString> _tags;      // Keep track of tags that apply in a any moment

};

} // end of namespace elabtsaot

#endif // MAPPINGXMLHANDLER_H
