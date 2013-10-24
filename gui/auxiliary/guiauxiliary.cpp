
#include "guiauxiliary.h"
using namespace elabtsaot;

#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>

using std::string;

QString guiauxiliary::askFileName(QString ext, bool read){
  QString fileName, caption, filter;
  if( read ){
    // Get a filename for reading
    caption = QString("Open %0 file").arg(ext);
    filter = QString(".%0 files (*.%1)").arg(ext).arg(ext);
    fileName = QFileDialog::getOpenFileName(0, caption, QDir::current().path(), filter);

  } else{
    // Get a filename for writing
    caption = QString("Save %0 file").arg(ext);
    filter = QString(".%0 files (*.%1)").arg(ext).arg(ext);
    fileName = QFileDialog::getSaveFileName(0, caption, QDir::current().path(), filter);
  }
  return fileName;
}

int guiauxiliary::askInt(QString const& title, int& val, int const& min, int const& max){

  QDialog dialog;
  dialog.setWindowTitle(title);
  QVBoxLayout mainLay;
  dialog.setLayout(&mainLay);

  QSpinBox intForm;
  mainLay.addWidget(&intForm);
  intForm.setRange(min,max);
  intForm.setValue(val);

  QHBoxLayout buttonsLay;
  mainLay.addLayout(&buttonsLay);
  QPushButton okBut("Ok");
  buttonsLay.addWidget(&okBut);
  dialog.connect(&okBut, SIGNAL(clicked()), &dialog, SLOT(accept()));
  QPushButton cancelBut("Cancel");
  buttonsLay.addWidget(&cancelBut);
  dialog.connect(&cancelBut, SIGNAL(clicked()), &dialog, SLOT(reject()));

  bool ans = dialog.exec();
  if (!ans) // Dialog cancelled or not executed correctly
    return 1;
  val = intForm.value();
  return 0;
}

int guiauxiliary::askDouble(QString const& title, double& value){

  QDialog dial;
  dial.setWindowTitle( title );
  QVBoxLayout bottomLay;
  dial.setLayout(&bottomLay);
  QDoubleSpinBox num;
  bottomLay.addWidget(&num);
  num.setDecimals(3);
  num.setRange( 0.0, 100000.0 );
  num.setSingleStep( 1000.0 );
  num.setValue(value);

  QHBoxLayout switchesLay;
  bottomLay.addLayout(&switchesLay);
  QPushButton ok("Ok");
  switchesLay.addWidget(&ok);
  dial.connect( &ok, SIGNAL(clicked()), &dial, SLOT(accept()) );
  QPushButton cancel("Cancel");
  switchesLay.addWidget(&cancel);
  dial.connect( &cancel, SIGNAL(clicked()), &dial, SLOT(reject()) );

  // Excecute dialog
  if( dial.exec() ){
    // Dialog excecuted properly
    value = num.value();
    return 0;
  } else{
    // Dialog cancelled or not executed correctly
    return 1;
  }
  return 0;
}

#include <QLabel>

int guiauxiliary::askString(QString const& title, std::string& v){
  // Dialog itself
  QDialog* dialog = new QDialog();
  dialog->setWindowTitle(title);
  QVBoxLayout* mainLay = new QVBoxLayout();
  dialog->setLayout(mainLay);

  // String
  QHBoxLayout* stringLay = new QHBoxLayout();
  mainLay->addLayout( stringLay );
  QLabel* stringLabel = new QLabel("Input string",dialog);
  stringLay->addWidget( stringLabel );
  QLineEdit* stringForm = new QLineEdit(QString::fromStdString(v), dialog);
  stringLay->addWidget(stringForm);

  // Buttons
  QHBoxLayout* buttonLay = new QHBoxLayout();
  mainLay->addLayout(buttonLay);
  QPushButton* okb = new QPushButton("OK");
  buttonLay->addWidget(okb);
  dialog->connect(okb, SIGNAL(clicked()), dialog, SLOT(accept()));
  QPushButton* cancelb = new QPushButton("Cancel");
  buttonLay->addWidget(cancelb);
  dialog->connect(cancelb, SIGNAL(clicked()), dialog, SLOT(reject()));

  // Excecute dialog
  if ( dialog->exec() ){
    // Dialog executed properly
    v = stringForm->text().toStdString();
    return 0;
  } else {
    // Dialog failed
    return 2;
  }
  return 0;
}
