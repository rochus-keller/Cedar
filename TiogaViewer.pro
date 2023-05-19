QT       += core gui widgets

TARGET = TiogaViewer
TEMPLATE = app

INCLUDEPATH += ..

SOURCES += \
    TiogaReader.cpp \
    TiogaViewer.cpp \
    CedarHighlighter.cpp \
    CedarLexer.cpp \
    CedarToken.cpp \
    CedarTokenType.cpp \
    CedarParser.cpp \
    CedarSynTree.cpp

HEADERS  += \
    TiogaReader.h \
    TiogaViewer.h \
    CedarHighlighter.h \
    CedarLexer.h \
    CedarRowCol.h \
    CedarToken.h \
    CedarTokenType.h \
    CedarParser.h \
    CedarSynTree.h

CONFIG(debug, debug|release) {
        DEFINES += _DEBUG
}
