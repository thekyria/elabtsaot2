
#include "guiauxiliary.h"
using namespace elabtsaot;

#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>

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
