///*!
//\file calibrationeditor.h

//This class is part of the elab-tsaot project in the Electronics Laboratory of
//the Ecole Polytechnique Federal de Lausanne.
//\author Lilis Georgios, georgios.lilis at epfl dot ch, Elab
//EPFL
//*/

//#ifndef CALIBRATIONXMLHANDLER_H
//#define CALIBRATIONXMLHANDLER_H

//#include <QXmlDefaultHandler>
//#include <QVector>
//class QString;

//namespace elabtsaot{

//class CalibrationEditor;
//class Logger;
//class Emulator;

//// This class provides the XML parser for importing calibration values from XML files
//class CalibrationXMLHandler : public QXmlDefaultHandler{

// public:

//  // ------------ functions ------------
//  CalibrationXMLHandler(CalibrationEditor* cal );
//  // Implementation of virtual functions
//  bool characters(QString const& ch);
//  bool startElement(QString const& namespaceURI, QString const& localName,
//                    QString const& qName, QXmlAttributes const& atts);
//  bool endElement(QString const& namespaceURI, QString const& localName,
//                  QString const& qName);

// private:

//  //! Reset handler to its first stage
//  void _resetState();

//  // ------------ variables ------------
//  CalibrationEditor* cal;     // pointer to a virgin calibration editor to be created by
//                                    // parsing the xml file
//  QVector<QString> _tags; // Keep track of tags that apply in a any moment

//};

//} // end of namespace elabtsaot

//#endif // CALIBRATIONXMLHANDLER_H
