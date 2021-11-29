QT       += widgets
TARGET = Mathematics
TEMPLATE = lib
DEFINES += MATHEMATICS_LIB
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Algebra.cpp \
    DxAnalytics.cpp \
    DxObjects.cpp \
    ExpObj.cpp \
    ExpObjBin.cpp \
    ExpObjOp.cpp \
    ExpStore.cpp \
    ExprTool.cpp \
    Factors.cpp \
    GlobalVars.cpp \
    LogExp.cpp \
    MathTool.cpp \
    Multfrac.cpp \
    Parser.cpp \
    SolChain.cpp \
    Statistics.cpp \
    SystLinIneq.cpp \
    mathematics.cpp

HEADERS += \
    mathematics.h\
    Mathematics_global.h \
    Algebra.h \
    DxObjects.h \
    ExpOb.h \
    ExpObBin.h \
    ExpObjOp.h \
    ExpStore.h \
    Factors.h \
    Globalvars.h \
    LogExp.h \
    MathTool.h \
    Parser.h \
    ResultReceiver.h \
    SolChain.h \
    Statistics.h \

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../FormulaPainter/release/ -lFormulaPainter
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../FormulaPainter/debug/ -lFormulaPainter

INCLUDEPATH += $$PWD/../FormulaPainter
DEPENDPATH += $$PWD/../FormulaPainter
