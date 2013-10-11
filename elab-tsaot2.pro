
#------------------ General ------------------
include( elabtsaot2config.pri )

#------------------ Version ------------------
VER_MAJ = 2
VER_MIN = 0
VER_PAT = 0
VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

#----------------- Resources -----------------
# Platform dependend application icon resource
win32:RC_FILE = \
  $$ELABTSAOT2_ROOT/resources/elab-tsaot2.rc
RESOURCES += \
  $$ELABTSAOT2_ROOT/resources/elab-tsaot2.qrc

#----------------- Include paths -----------------
INCLUDEPATH += \
  $$ELABTSAOT2_ROOT/ \
  $$ELABTSAOT2_ROOT/auxiliary \
  $$ELABTSAOT2_ROOT/auxiliary/sha1 \
  $$ELABTSAOT2_ROOT/powersystem \
  $$ELABTSAOT2_ROOT/powersystem/pwsmodel \
  $$ELABTSAOT2_ROOT/importer_exporter \
  $$ELABTSAOT2_ROOT/ss_analysis \
  $$ELABTSAOT2_ROOT/td_simulation \
  $$ELABTSAOT2_ROOT/emulator_hw \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model \
  $$ELABTSAOT2_ROOT/emulator_hw/powersystem_model \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios \
  $$ELABTSAOT2_ROOT/gui \
  $$ELABTSAOT2_ROOT/gui/auxiliary \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystem_model \
  $$ELABTSAOT2_ROOT/gui/ssanalysis_editor \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor \
  $$ELABTSAOT2_ROOT/gui/scenario_editor \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor \
  $$ELABTSAOT2_ROOT/gui/console \
  $$CYUSB_PATH/include \
  $$BOOST_PATH/boost_1_48_0 \

DEPENDPATH += \
  $$ELABTSAOT2_ROOT/ \
  $$ELABTSAOT2_ROOT/auxiliary \
  $$ELABTSAOT2_ROOT/auxiliary/sha1 \
  $$ELABTSAOT2_ROOT/powersystem \
  $$ELABTSAOT2_ROOT/powersystem/pwsmodel \
  $$ELABTSAOT2_ROOT/importer_exporter \
  $$ELABTSAOT2_ROOT/ss_analysis \
  $$ELABTSAOT2_ROOT/td_simulation \
  $$ELABTSAOT2_ROOT/emulator_hw \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model \
  $$ELABTSAOT2_ROOT/emulator_hw/powersystem_model \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios \
  $$ELABTSAOT2_ROOT/gui \
  $$ELABTSAOT2_ROOT/gui/auxiliary \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystem_model \
  $$ELABTSAOT2_ROOT/gui/ssanalysis_editor \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor \
  $$ELABTSAOT2_ROOT/gui/scenario_editor \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor \
  $$ELABTSAOT2_ROOT/gui/console \

#------------------ Source files -----------------
SOURCES += \
  $$ELABTSAOT2_ROOT/main.cpp \
  $$ELABTSAOT2_ROOT/auxiliary/sha1/sha1.cpp \
  $$ELABTSAOT2_ROOT/auxiliary/auxiliary.cpp \
  $$ELABTSAOT2_ROOT/auxiliary/graph.cpp \
  $$ELABTSAOT2_ROOT/auxiliary/physics.cpp \
  $$ELABTSAOT2_ROOT/auxiliary/propertyholder.cpp \
  $$ELABTSAOT2_ROOT/powersystem/powersystem.cpp \
  $$ELABTSAOT2_ROOT/powersystem/load.cpp \
  $$ELABTSAOT2_ROOT/powersystem/generator.cpp \
  $$ELABTSAOT2_ROOT/powersystem/bus.cpp \
  $$ELABTSAOT2_ROOT/powersystem/branch.cpp \
  $$ELABTSAOT2_ROOT/powersystem/pwsmodel/pwsmodel.cpp \
  $$ELABTSAOT2_ROOT/importer_exporter/importerexporter.cpp \
  $$ELABTSAOT2_ROOT/importer_exporter/messagehandler.cpp \
  $$ELABTSAOT2_ROOT/importer_exporter/powersystemxmlhandler.cpp \
  $$ELABTSAOT2_ROOT/importer_exporter/schematicxmlhandler.cpp \
  $$ELABTSAOT2_ROOT/importer_exporter/mappingxmlhandler.cpp \
  $$ELABTSAOT2_ROOT/importer_exporter/scenariosetxmlhandler.cpp \
  \
  $$ELABTSAOT2_ROOT/ss_analysis/ssengine.cpp \
  $$ELABTSAOT2_ROOT/ss_analysis/moteurrenard.cpp \
  \
  $$ELABTSAOT2_ROOT/td_simulation/tdengine.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/tdresults.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/tdresultsidentifier.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/tdresultsbank.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/simulator_sw.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios/scenario.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios/event.cpp \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios/scenarioset.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/tdemulator.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/moteurfengtian.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/emulator.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/encoder.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/slice.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/sliceanalog.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/slicedigital.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/potentiometer.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/dac.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/node.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/emulatorbranch.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/emulatorhw.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/atom.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/pqpipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/slackpipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/generatorpipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/constzloadpipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/constploadpipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/constiloadpipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/pipeline.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/powersystem_model/pwsmappermodel.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/powersystem_model/pwsmappermodelelement.cpp \
  $$ELABTSAOT2_ROOT/emulator_hw/usbfpgainterface.cpp \
  \
  $$ELABTSAOT2_ROOT/gui/mainwindow.cpp \
  $$ELABTSAOT2_ROOT/gui/sidepane.cpp \
  $$ELABTSAOT2_ROOT/gui/auxiliary/guiauxiliary.cpp \
  $$ELABTSAOT2_ROOT/gui/auxiliary/propertydialog.cpp \
  $$ELABTSAOT2_ROOT/gui/console/console.cpp \
  $$ELABTSAOT2_ROOT/gui/console/qdebugstream.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystemeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/schematicviewer.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/schematicmover.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/schematiceditor.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/tabularpowersysteminterface.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/tabularpowersystemview.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/tabularpowersystemeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystemeditordialogs.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystem_model/pwsschematicmodel.cpp \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystem_model/pwsschematicmodelelement.cpp \
  $$ELABTSAOT2_ROOT/gui/ssanalysis_editor/ssanalysiseditor.cpp \
  $$ELABTSAOT2_ROOT/gui/ssanalysis_editor/loadfloweditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/slicewidget.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/mapperslicewidget.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/mapperemulatoreditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/mappereditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitterslicewidget.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitterslicetab.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitteremutab.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitteremulatoreditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fittereditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/tabularmapperinterface.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/tabularmapperview.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/tabularmappereditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/nodedialog.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/calibrationeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/communicationeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/auxiliaryeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/rawwritetodevicedialog.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/rawreadfromdevicedialog.cpp \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/logencodingdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/scenario_editor/scenarioeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/scenario_editor/autogeneratescenariosdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/tdanalysiseditor.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrequesteditor.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsbankeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/plotpropertiesdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/deletecurvedialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/curvepropertiesdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/tdanalysiseditordialogs.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrequestdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrenamedialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrequestdetailsdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsvisualizationeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsdbbrowser.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/addplotdialog.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/gtdaeditor.cpp \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/gtdaviewer.cpp \

#------------------ Header files -----------------
HEADERS += \
  $$ELABTSAOT2_ROOT/mainpage.h \
  $$ELABTSAOT2_ROOT/logger.h \
  $$ELABTSAOT2_ROOT/auxiliary/precisiontimer.h \
  $$ELABTSAOT2_ROOT/auxiliary/auxiliary.h \
  $$ELABTSAOT2_ROOT/auxiliary/graph.h \
  $$ELABTSAOT2_ROOT/auxiliary/sha1/sha1.h \
  $$ELABTSAOT2_ROOT/auxiliary/physics.h \
  $$ELABTSAOT2_ROOT/auxiliary/property.h \
  $$ELABTSAOT2_ROOT/auxiliary/property_t.h \
  $$ELABTSAOT2_ROOT/auxiliary/propertyholder.h \
  $$ELABTSAOT2_ROOT/powersystem/powersystem.h \
  $$ELABTSAOT2_ROOT/powersystem/load.h \
  $$ELABTSAOT2_ROOT/powersystem/generator.h \
  $$ELABTSAOT2_ROOT/powersystem/bus.h \
  $$ELABTSAOT2_ROOT/powersystem/branch.h \
  $$ELABTSAOT2_ROOT/powersystem/pwsmodel/pwsmodel.h \
  $$ELABTSAOT2_ROOT/powersystem/pwsmodel/pwsmodelelement.h \
  $$ELABTSAOT2_ROOT/importer_exporter/importerexporter.h \
  $$ELABTSAOT2_ROOT/importer_exporter/messagehandler.h \
  $$ELABTSAOT2_ROOT/importer_exporter/powersystemxmlhandler.h \
  $$ELABTSAOT2_ROOT/importer_exporter/schematicxmlhandler.h \
  $$ELABTSAOT2_ROOT/importer_exporter/mappingxmlhandler.h \
  $$ELABTSAOT2_ROOT/importer_exporter/scenariosetxmlhandler.h \
  \
  $$ELABTSAOT2_ROOT/ss_analysis/ssengine.h \
  $$ELABTSAOT2_ROOT/ss_analysis/moteurrenard.h \
  \
  $$ELABTSAOT2_ROOT/td_simulation/tdengine.h \
  $$ELABTSAOT2_ROOT/td_simulation/tdresults.h \
  $$ELABTSAOT2_ROOT/td_simulation/tdresultsidentifier.h \
  $$ELABTSAOT2_ROOT/td_simulation/tdresultsbank.h \
  $$ELABTSAOT2_ROOT/td_simulation/simulator_sw.h \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios/scenario.h \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios/event.h \
  $$ELABTSAOT2_ROOT/td_simulation/scenarios/scenarioset.h \
  $$ELABTSAOT2_ROOT/emulator_hw/tdemulator.h \
  $$ELABTSAOT2_ROOT/emulator_hw/moteurfengtian.h \
  $$ELABTSAOT2_ROOT/emulator_hw/emulator.h \
  $$ELABTSAOT2_ROOT/emulator_hw/encoder.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/slice.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/sliceanalog.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/slicedigital.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/potentiometer.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/dac.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/node.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/emulatorbranch.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/emulatorhw.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/atom.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/pqpipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/slackpipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/generatorpipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/constzloadpipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/constploadpipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/constiloadpipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/hardware_model/pipeline.h \
  $$ELABTSAOT2_ROOT/emulator_hw/powersystem_model/pwsmappermodel.h \
  $$ELABTSAOT2_ROOT/emulator_hw/powersystem_model/pwsmappermodelelement.h \
  $$ELABTSAOT2_ROOT/emulator_hw/usbfpgainterface.h \
  \
  $$ELABTSAOT2_ROOT/gui/mainwindow.h \
  $$ELABTSAOT2_ROOT/gui/sidepane.h \
  $$ELABTSAOT2_ROOT/gui/auxiliary/guiauxiliary.h \
  $$ELABTSAOT2_ROOT/gui/auxiliary/propertydialog.h \
  $$ELABTSAOT2_ROOT/gui/console/console.h \
  $$ELABTSAOT2_ROOT/gui/console/qdebugstream.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystemeditor.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/schematicviewer.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/schematicmover.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/schematiceditor.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/tabularpowersysteminterface.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/tabularpowersystemview.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/tabularpowersystemeditor.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystemeditordialogs.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystem_model/pwsschematicmodel.h \
  $$ELABTSAOT2_ROOT/gui/powersystem_editor/powersystem_model/pwsschematicmodelelement.h \
  $$ELABTSAOT2_ROOT/gui/ssanalysis_editor/ssanalysiseditor.h \
  $$ELABTSAOT2_ROOT/gui/ssanalysis_editor/loadfloweditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/slicewidget.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/mapperslicewidget.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/mapperemulatoreditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/mappereditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitterslicewidget.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitterslicetab.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitteremutab.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fittereditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/fitteremulatoreditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/tabularmapperinterface.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/tabularmapperview.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/tabularmappereditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/nodedialog.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/calibrationeditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/communicationeditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/auxiliaryeditor.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/rawwritetodevicedialog.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/rawreadfromdevicedialog.h \
  $$ELABTSAOT2_ROOT/gui/emulator_hw_editor/logencodingdialog.h \
  $$ELABTSAOT2_ROOT/gui/scenario_editor/scenarioeditor.h \
  $$ELABTSAOT2_ROOT/gui/scenario_editor/autogeneratescenariosdialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/tdanalysiseditor.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrequesteditor.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsbankeditor.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/plotpropertiesdialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/deletecurvedialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/curvepropertiesdialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/tdanalysiseditordialogs.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrequestdialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrenamedialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsrequestdetailsdialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsvisualizationeditor.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/resultsdbbrowser.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/addplotdialog.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/tdresultsplotproperties.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/gtdaeditor.h \
  $$ELABTSAOT2_ROOT/gui/tdanalysis_editor/gtdaviewer.h \

#------------------ Libraries ------------------
CONFIG( debug, debug|release ) {
  LIBS += -L$$CYUSB_PATH/lib \
          -lCyUSBd
} else {
  LIBS += -L$$CYUSB_PATH/lib \
          -lCyUSB
}

#------------------ Qwt ------------------
QWT_CONFIG += QwtDll QwtPlot QwtWidgets QwtSvg QwtOpenGL
# Taken from qwt.prf
contains(QWT_CONFIG, QwtDll) {
  DEFINES *= QWT_DLL
}
contains(QWT_CONFIG, QwtSvg) {
  QT      *= svg
} else {
  DEFINES *= QWT_NO_SVG
}
# Update include file
INCLUDEPATH += $$QWT_PATH/qwt-$$QWT_VERSION/include
# Link against the libraries proper
CONFIG( debug, debug|release ) {
  LIBS += -L$$QWT_PATH/qwt-$$QWT_VERSION/lib/ \
          -lqwtd
} else {
  LIBS += -L$$QWT_PATH/qwt-$$QWT_VERSION/lib/ \
          -lqwt
}
#------------------ Disable spies ------------------
nospies{
  DEFINES += BUILD_NOSPIES
}
