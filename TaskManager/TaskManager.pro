QT += widgets network axcontainer
QT+=charts

TARGET = TestDriver
TEMPLATE = app

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
        MultiPlot.cpp \
        OptionMenuPlotter.cpp \
        OutWindow.cpp \
        QuestWindow.cpp \
        SelectTask.cpp \
        SubTask.cpp \
        TaskTools.cpp \
        TaskWork.cpp \
        WinTesting.cpp \
        XPressTests.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  Resorurce.qrc

HEADERS += \
  MultiPlot.h \
  OptionMenuPlotter.h \
  OutWindow.h \
  QuestWindow.h \
  SelectTask.h \
  TaskTools.h \
  TaskWork.h \
  WinTesting.h \
  XPressTests.h

INCLUDEPATH += $$PWD/../CommonWidgets
DEPENDPATH += $$PWD/../CommonWidgets

INCLUDEPATH += $$PWD/../FormulaPainter
DEPENDPATH += $$PWD/../FormulaPainter

INCLUDEPATH += $$PWD/../Mathematics
DEPENDPATH += $$PWD/../Mathematics

INCLUDEPATH += $$PWD/../TaskFileManager
DEPENDPATH += $$PWD/../TaskFileManager

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CommonWidgets/release/ -lCommonWidgets
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CommonWidgets/debug/ -lCommonWidgets

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../FormulaPainter/release/ -lFormulaPainter
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../FormulaPainter/debug/ -lFormulaPainter

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Mathematics/release/ -lMathematics
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Mathematics/debug/ -lMathematics

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TaskFileManager/release/ -lTaskFileManager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TaskFileManager/debug/ -lTaskFileManager

win32:CONFIG(release, debug|release) : OPENSSL_LIBS += -Lc:/Qt/Tools/OpenSSL/Win_x64/lib/ -llibssl
win32:CONFIG(release, debug|release) : OPENSSL_LIBS += -Lc:/Qt/Tools/OpenSSL/Win_x64/lib/ -llibcrypto

FORMS += \
    OptionMenuPlotter.ui

