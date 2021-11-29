QT += gui network widgets sql

CONFIG += c++11
CONFIG -= app_bundle
#MYSQL_LIBS=-lmysql
DEFINES += COMMONWIDGETS_LIB
DEFINES += FORMULAPAINTER_LIB
DEFINES += MATHEMATICS_LIB
DEFINES += TASKFILEMANAGER_LIB

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
        DataServer.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
  DataServer.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CommonWidgets/release/ -lCommonWidgets
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CommonWidgets/debug/ -lCommonWidgets

INCLUDEPATH += $$PWD/../CommonWidgets
DEPENDPATH += $$PWD/../CommonWidgets

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../FormulaPainter/release/ -lFormulaPainter
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../FormulaPainter/debug/ -lFormulaPainter

INCLUDEPATH += $$PWD/../FormulaPainter
DEPENDPATH += $$PWD/../FormulaPainter

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Mathematics/release/ -lMathematics
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Mathematics/debug/ -lMathematics

INCLUDEPATH += $$PWD/../Mathematics
DEPENDPATH += $$PWD/../Mathematics

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TaskFileManager/release/ -lTaskFileManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TaskFileManager/debug/ -lTaskFileManager

INCLUDEPATH += $$PWD/../TaskFileManager
DEPENDPATH += $$PWD/../TaskFileManager
