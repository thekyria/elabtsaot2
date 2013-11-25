/*!
\file calibrationxmlhandler.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.
\author Lilis Georgios, georgios.lilis at epfl dot ch, Elab
EPFL
*/

#ifndef CALIBRATIONXMLHANDLER_H
#define CALIBRATIONXMLHANDLER_H
#include <QFile>
#include <QDomDocument>
#include <QVector>
#include <QString>
#include "calibrationeditor.h"

namespace elabtsaot{
class CalibrationEditor;

class CalibrationXMLHandler{

private:
    CalibrationEditor* _cal;
    QFile* _xmlfile;
    QDomDocument _doc;

public:

  CalibrationXMLHandler(QFile* file, CalibrationEditor* cal );
  ~CalibrationXMLHandler();
  int importfile();
  int importXML();

};

}

#endif // CALIBRATIONXMLHANDLER_H
