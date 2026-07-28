QT += core gui widgets xml
CONFIG += c++17 warn_on
TEMPLATE = app
TARGET = PAO

INCLUDEPATH += model view repo persistenza attivita

SOURCES += \
    attivita/compleanno.cpp \
    attivita/impegno.cpp \
    attivita/lavoro.cpp \
    attivita/promemoria.cpp \
    attivita/rowadapter.cpp \
    attivita/scadenza.cpp \
    attivita/universita.cpp \
    main.cpp \
    persistenza/jsonsavevisitor.cpp \
    persistenza/xmlsavevisitor.cpp \
    view/mainwindow.cpp

HEADERS += \
    attivita/attivita.h \
    attivita/attivitavisitor.h \
    attivita/compleanno.h \
    attivita/impegno.h \
    attivita/lavoro.h \
    attivita/promemoria.h \
    attivita/rowadapter.h \
    attivita/scadenza.h \
    attivita/universita.h \
    persistenza/jsonsavevisitor.h \
    persistenza/xmlsavevisitor.h \
    view/mainwindow.h

FORMS += forms/mainwindow.ui

QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

RESOURCES += \
    assets.qrc

DISTFILES += \
    esempio.json \
    esempio.xml
