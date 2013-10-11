
#include "fittereditor.h"
using namespace elabtsaot;

#include "powersystem.h"
#include "auxiliary.h"
#include "emulator.h"
#include "nodedialog.h"

#include "guiauxiliary.h"
#include "fitteremulatoreditor.h"

#include <QToolBar>
#include <QAction>

#include <ctime>
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

FitterEditor::FitterEditor( Emulator* emu, QWidget* parent ) :
    QSplitter(Qt::Vertical, parent), _emu(emu), _emuhw(emu->emuhw()){

  // -- Emulator editor (fitter edition) --
  // fee does not exist, so create it
  fee = new FitterEmulatorEditor( _emuhw, this );
  this->addWidget( fee );
  connect( fee, SIGNAL(fittingPositionClicked(int,int,int,int)),
           this, SLOT(fittingPositionClickedSlot(int,int,int,int)) );

  // -- Fitter emulator toolbar --
  QToolBar* fitToolbar = new QToolBar("Fitter editor toolbar", this);
  this->addWidget( fitToolbar );
  fitToolbar->setFixedHeight(30);

  // Update view act
  QAction* updtAct = new QAction( QIcon(":/images/update.png"),
                                  "Update fitter editor view", fitToolbar );
  fitToolbar->addAction( updtAct );
  connect( updtAct, SIGNAL(triggered()),
           this, SLOT(updt()) );

  fitToolbar->addSeparator(); // -- Separator

  // Reset fitting act
  QAction* resetFittingAct = new QAction( QIcon(":/images/reset.png"),
                                          "Reset fitting", fitToolbar );
  fitToolbar->addAction(resetFittingAct);
  connect( resetFittingAct, SIGNAL(triggered()),
           this, SLOT(resetFittingSlot()) );

  // Auto fitting act
  QAction* autoFittingAct = new QAction( QIcon(":/images/execute.png"),
                                         "Auto fitting", fitToolbar );
  fitToolbar->addAction(autoFittingAct);
  connect( autoFittingAct, SIGNAL(triggered()),
           this, SLOT(autoFittingSlot()) );

  // Validate fitting act
  QAction* validateFittingAct = new QAction( QIcon(":/images/validate.png"),
                                               "Validate fitting", fitToolbar );
  fitToolbar->addAction(validateFittingAct);
  connect( validateFittingAct, SIGNAL(triggered()),
           this, SLOT(validateFittingSlot()) );

  fitToolbar->addSeparator(); // -- Separator

  // Toggle Re/Im act
  QAction* toggleRealAct = new QAction( QIcon(":/images/reim.png"),
                                        "Toggle Re/Im", fitToolbar );
  fitToolbar->addAction(toggleRealAct);
  connect( toggleRealAct, SIGNAL(triggered()), fee, SLOT(toggleReal()) );

  // Zoom in act (button)
  QAction* zoomInAct = new QAction( QIcon(":/images/zoom-in.png"),
                                    "Zoom in", fitToolbar );
  fitToolbar->addAction(zoomInAct);
  connect( zoomInAct, SIGNAL(triggered()), fee, SLOT(zoomIn()) );

  // Zoom out act (button)
  QAction* zoomOutAct = new QAction( QIcon(":/images/zoom-out.png"),
                                     "Zoom out", fitToolbar );
  fitToolbar->addAction(zoomOutAct);
  connect( zoomOutAct, SIGNAL(triggered()), fee, SLOT(zoomOut()) );

  // Zoom fit act (button)
  QAction* zoomFitAct = new QAction( QIcon(":/images/zoom-fit.png"),
                                     "Zoom fit", fitToolbar );
  fitToolbar->addAction(zoomFitAct);
  connect( zoomFitAct, SIGNAL(triggered()), fee, SLOT(zoomFit()) );
}

int FitterEditor::init(){
  //_encoding.clear();
  return fee->init();
}

void FitterEditor::updt(){
  return fee->updt();
}

void FitterEditor::resetFittingSlot(){

  int ans = _emu->resetEmulator( false );
  if ( ans )
    cout << "Reset the emulator failed with code: " << ans << endl;
  else
    cout << "Reseting the emulator succeded" << endl;

  fee->updt();

  return;
}

void FitterEditor::autoFittingSlot(){

  vector<string> outputMsg;
  int ans = _emu->autoFitting(&outputMsg);
  if ( ans ){
    cout << "Auto-fitting failed with code " << ans << endl;
  } else {
    cout << "Auto-fitting succeeded!" << endl;
    fee->updt();
  }
  cout << "Auto-fitting warning messages: " << endl;
  for ( size_t k = 0 ; k != outputMsg.size() ; ++k ){
    cout << outputMsg[k] << endl;
  }

  return;
}

void FitterEditor::validateFittingSlot(){

  int ans = _emu->validateFitting();
  if ( ans ){
    cout << "Validate fitting failed with code " << ans << endl;
  } else {
    cout << "Validate fitting succeeded!" << endl;
  }

  return;
}

void FitterEditor::fittingPositionClickedSlot( int emulator_tab,
                                               int emulator_row,
                                               int emulator_col,
                                               int emulator_elm ){

  /*
    emulator_elm follows the following convention for the element clicked:
    0b0..0zyyyxxx

    z   - 0: non-node; 1: node
    yyy - 0: near pot sw; 1: near pot res; 2: far pot sw; 3: far pot res;
          4: short circt sw; 5: mid gnd sw
    xxx - embr pos according to EmbrPosition 0: EMBRPOS_ U; 1: UR ...; 7: UL
  */
  int ans;
  int isNode = (1<<6) & emulator_elm;           // extracting z
  int embrPos = 7 & emulator_elm;               // extracting xxx
  int embrElm = ((7<<3) & emulator_elm) >> 3;   // extracting yyy
  bool valBool;
  double valDouble;
  bool real = fee->isShowingReal();
  if ( isNode ){
    Atom* p_atom = &_emuhw->sliceSet[emulator_tab]
                            .ana._atomSet[emulator_row][emulator_col];
    NodeDialog dialog( p_atom, real );
    dialog.exec();
    return;
  }

  QString text;
  switch ( embrElm ){
  case 0:
    valBool = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
              embr_pot_near_sw( embrPos, real );
    _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
        set_embr_pot_near_sw( embrPos, !valBool, real);
    break;

  case 1:
    text = QString("Near pot res of embr %0 of atom [tab,row,col]: [%1,%2,%3]")
            .arg(embrPos).arg(emulator_tab).arg(emulator_row).arg(emulator_col);
    valDouble = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
                embr_pot_near_r( embrPos, real );
    ans = guiauxiliary::askDouble( text, valDouble );
    if ( !ans ){
      ans = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
            set_embr_pot_near_r( embrPos, valDouble, real );
      if ( ans ){
        cout << "Failed to set: ";
        cout << text.toStdString() << endl;
      }
    }
    break;

  case 2:
    valBool = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
              embr_pot_far_sw( embrPos, real );
    _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
        set_embr_pot_far_sw( embrPos, !valBool, real);
    break;

  case 3:
    text = QString("Far pot res of embr %0 of atom [tab,row,col]: [%1,%2,%3]")
            .arg(embrPos).arg(emulator_tab).arg(emulator_row).arg(emulator_col);
    valDouble=_emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
                embr_pot_far_r( embrPos, real );
    ans = guiauxiliary::askDouble( text, valDouble );
    if ( !ans ){
      ans = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
            set_embr_pot_far_r( embrPos, valDouble, real );
      if ( ans ){
        cout << "Failed to set: ";
        cout << text.toStdString() << endl;
      }
    }
    break;

  case 4:
    valBool = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
              embr_sw_sc( embrPos, real );
    _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
        set_embr_sw_sc( embrPos, !valBool, real);
    break;

  case 5:
    valBool = _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
              embr_sw_mid( embrPos, real );
    _emuhw->sliceSet[emulator_tab].ana._atomSet[emulator_row][emulator_col].
        set_embr_sw_mid( embrPos, !valBool, real);
    break;

  default:
    break;
  }

  fee->updt();

  return;
}
