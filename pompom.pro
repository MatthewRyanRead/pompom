QT += core
QT -= gui
QT += xmlpatterns

CONFIG += c++11

TARGET = pompom
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    pompom.cpp

HEADERS += \
    pompom.h \
    graphnode.h
