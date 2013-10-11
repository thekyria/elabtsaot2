
#include "console.h"
using namespace elabtsaot;

#include "qdebugstream.h"

#include <QTextEdit>
#include <QScrollBar>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QPushButton>

#include <string>
#include <iostream>
using std::cout;
using std::endl;

Console::Console(QWidget *parent) :
    QSplitter( Qt::Vertical, parent ){

  this->setMinimumHeight(128);

  // Container box
  QGroupBox* containerwidget = new QGroupBox("Console", this);
  this->addWidget(containerwidget);
  containerwidget->setMaximumHeight(256);

  // Layout
  QVBoxLayout* main= new QVBoxLayout(containerwidget);
  containerwidget->setLayout(main);

  // Console text box
  outBox = new QTextEdit(this);
  main->addWidget(outBox);
  outBox->setReadOnly( true );
  QFont* font = new QFont();
  font->setStyleHint( QFont::TypeWriter );
  outBox->setFont( *font );

  //Console input
  QHBoxLayout* inputlayout = new QHBoxLayout;
  QLabel* inlabel = new QLabel(" >> ");
  inBox = new QLineEdit;
  connect(inBox,SIGNAL(returnPressed()),this,SLOT(interpretCommand()));
  inputlayout->addWidget(inlabel);
  inputlayout->addWidget(inBox);
  main->addLayout(inputlayout);

  // Progress bar
  progressBar = new QProgressBar();
  progressBar->setLayoutDirection(Qt::LeftToRight);
//  progressBar->setInvertedAppearance(true);
  progressBar->setValue(0);
  setStyleSheet( "QProgressBar {border: 1px solid grey;border-radius: 4px;text-align: center; } QProgressBar::chunk {background-color: #559CFF; }");
  main->addWidget(progressBar);

  // Fix width
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

  // Re-direct standard output to widget
  _redirector = new QDebugStream(std::cout, outBox);
  connect( outBox, SIGNAL(textChanged()), this, SLOT(scrollDown()) );

}

Console::~Console(){
  delete _redirector;
}

void Console::notifyProgress(double val){
  progressBar->setValue( val );
  progressBar->update();
  return;
}

void Console::scrollDown(){
  outBox->verticalScrollBar()->setValue(
                                 outBox->verticalScrollBar()->maximum() );
}

void Console::interpretCommand(){

  QString inputstring;
  inputstring = inBox->text();
  inBox->clear();
  cout << "Executing:  "<< inputstring.toStdString() << endl;
  QStringList stringwords;
  stringwords=inputstring.split(' ');

  if (stringwords.at(0)=="add")
    cout<<"result: "<<stringwords.at(1).toDouble(0) + stringwords.at(2).toDouble(0)<<endl;
  if (stringwords.at(0)=="sub")
    cout<<"result: "<<stringwords.at(1).toDouble(0) - stringwords.at(2).toDouble(0)<<endl;

  return;
}
