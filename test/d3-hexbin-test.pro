TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

include(../src/d3_hexbin.pri)

SOURCES += \
    hexbin-test.cpp
 

HEADERS += \
    _regex_replace.hpp \
    pathEqual.hpp
