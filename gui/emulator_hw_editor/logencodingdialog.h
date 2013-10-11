/*!
\file logencodingdialog.h

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef LOGENCODINGDIALOG_H
#define LOGENCODINGDIALOG_H

#include <QObject>
class QLineEdit;
class QComboBox;
class QPushButton;

#include <string>
#include <map>

namespace elabtsaot{

enum LogEncodingDialogTarget{
  LogEncodingDialogTarget_toConsole,
  LogEncodingDialogTarget_toFile
};

enum LogEncodingDialogMode{
  LogEncodingDialogMode_none,
  LogEncodingDialogMode_dec,
  LogEncodingDialogMode_hex,
  LogEncodingDialogMode_both
};

class LogEncodingDialog : public QObject {

  Q_OBJECT

 public:

  LogEncodingDialog( size_t sliceCount,
                     size_t& sliceId,
                     int& target,
                     std::string& mode,
                     std::string& fname );
  int exec();

 public slots:

  void fnameDialog();
  void targetChanged(int);

 private:

  size_t _sliceCount;
  size_t& _sliceId;
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

#endif // LOGENCODINGDIALOG_H
