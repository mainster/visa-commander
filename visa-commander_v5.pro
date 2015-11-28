#-------------------------------------------------
#
# Project created by QtCreator 2015-10-05T03:25:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

QMAKE_CXX = ccache g++
TARGET = visa-commander_v5
TEMPLATE = app
#QMAKE_CXXFLAGS_RELEASE -= -O2
#CONFIG += console

SOURCES += main.cpp\
        mainwindow.cpp \
    visa.cpp \
    visareg.cpp \
    driver.cpp \
    ioedit.cpp \
    portdialog.cpp \
    register.cpp \
    hled.cpp \
    hwreg.cpp \
    dvm.cpp \
    calc.cpp \
    power.cpp \
    fugen.cpp \
    mqtimer.cpp \
    mdstatebar.cpp

HEADERS  += mainwindow.h \
    visareg.h \
    driver.h \
    visa.h \
    ioedit.h \
    globals.h \
    portdialog.h \
    register.h \
    sleeper.h \
    hled.h \
    hwreg.h \
    dvm.h \
    calc.h \
    filebackup.h \
    utype.h \
    power.h \
    mqtimer.h \
    fugen.h \
    mdstatebar.h

FORMS    += mainwindow.ui \
    portdialog.ui \
    register.ui \
    dvm.ui \
    power.ui \
    fugen.ui \
    visa.ui

RESOURCES += \
    visa-commander_v5.qrc

DISTFILES +=

OTHER_FILES +=

