/*!
\file rawreadfromdevicedialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef RAWREADFROMDEVICEDIALOG_H
#define RAWREADFROMDEVICEDIALOG_H

#include <QObject>
class QLineEdit;
class QComboBox;
class QPushButton;

#include <string>
#include <map>

namespace elabtsaot{

enum RawReadFromDeviceDialogTarget{
  RawReadFromDeviceDialogTarget_toConsole,
  RawReadFromDeviceDialogTarget_toFile
};

enum RawReadFromDeviceDialogMode{
  RawReadFromDeviceDialogMode_none,
  RawReadFromDeviceDialogMode_dec,
  RawReadFromDeviceDialogMode_hex,
  RawReadFromDeviceDialogMode_both
};

class RawReadFromDeviceDialog : public QObject {

  Q_OBJECT

public:

  RawReadFromDeviceDialog( size_t deviceCount,
                           size_t& devId,
                           unsigned int& startAddress,
                           size_t& wordCount,
                           int& target,
                           std::string& mode,
                           std::string& fname,
                           QWidget* parent = 0 );
  int exec();

public slots:

 void fnameDialog();
 void targetChanged(int);

private:

  size_t const& _deviceCount;
  size_t& _devId;
  unsigned int& _startAddress;
  size_t& _wordCount;
  int& _target;    // according to LogEncodingDialogTarget
  std::string& _mode;   // "none", "dec", "hex", "both"
  std::string& _fname;

  std::map<int,std::string> _targetOptions;
  std::map<int,std::string> _modeOptions;

  QComboBox* targetCombo;
  QLineEdit* fnameLine;
  QPushButton* fnameBut;

};

} // end of namespace elabtsaot

#endif // RAWREADFROMDEVICEDIALOG_H
