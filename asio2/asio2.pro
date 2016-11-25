QMAKE_CXXFLAGS += -std=c++1y
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

LIBS += -lboost_coroutine -lboost_context -lboost_system -lpthread

