#-------------------------------------------------
#
# Project created by QtCreator 2013-11-15T14:00:31
#
#-------------------------------------------------

QT       += core gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release build_all

CONFIG(debug, release|debug):TARGET = ../$$PWD/RsaToolboxd
else:TARGET = ../$$PWD/RsaToolbox

include(base.pri)
include(Bus/Rsib/rsib.pri)
include(Bus/Visa/visa.pri)
include(Instruments/Vna/vna.pri)
include(QCustomPlot/qcustomplot.pri)

#SOURCES += main.cpp