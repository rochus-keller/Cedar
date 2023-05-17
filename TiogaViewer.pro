QT       += core gui widgets

TARGET = TiogaViewer
TEMPLATE = app


SOURCES += \
    TiogaReader.cpp \
    TiogaViewer.cpp \
    CedarHighlighter.cpp \
    CedarLexer.cpp \
    CedarToken.cpp \
    CedarTokenType.cpp

HEADERS  += \
    TiogaReader.h \
    TiogaViewer.h \
    CedarHighlighter.h \
    CedarLexer.h \
    CedarRowCol.h \
    CedarToken.h \
    CedarTokenType.h

CONFIG(debug, debug|release) {
        DEFINES += _DEBUG
}
