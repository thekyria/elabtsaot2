
#------------------ Paths and additional configs ------------------
include( ./elabtsaot2local.pri )

#------------------ Qwt ------------------
QWT_VER_MAJ = 6
QWT_VER_MIN = 1
QWT_VER_PAT = 0-rc3
QWT_VERSION = $${QWT_VER_MAJ}.$${QWT_VER_MIN}.$${QWT_VER_PAT}

#------------------ General ------------------
CONFIG += qt
QT += core gui xml xmlpatterns

#------------------ Build ------------------
QMAKE_LFLAGS *= -Wl,-enable-stdcall-fixup \
                -Wl,-enable-auto-import \
                -Wl,-enable-runtime-pseudo-reloc
				
#------------------ Target ------------------
BUILD_SUFFIX =
TARGET = elab-tsaot2
nospies{
  message("Building a version with no SPY support ... ")
  NOSPIES_DIR_SUFFIX = _nospies

  BUILD_SUFFIX = $${BUILD_SUFFIX}$${NOSPIES_DIR_SUFFIX}
}
TARGET = $${TARGET}$${BUILD_SUFFIX}
TEMPLATE = app

CONFIG( debug, debug|release ) {
  DEBUGORRELEASE = debug
#  QMAKE_CXXFLAGS_DEBUG += -O0
} else {
  DEBUGORRELEASE = release
#  QMAKE_CXXFLAGS_RELEASE += -O3
}
OBJECTS_DIR = $${BUILD_DIR}/$${DEBUGORRELEASE}/.obj$${BUILD_SUFFIX}
MOC_DIR = $${BUILD_DIR}/$${DEBUGORRELEASE}/.moc$${BUILD_SUFFIX}
RCC_DIR = $${BUILD_DIR}/$${DEBUGORRELEASE}/.rcc$${BUILD_SUFFIX}
UI_DIR = $${BUILD_DIR}/$${DEBUGORRELEASE}/.ui$${BUILD_SUFFIX}
