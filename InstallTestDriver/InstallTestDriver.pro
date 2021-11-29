TEMPLATE = aux
INSTALLER = installer

FROM=$$PWD/../TestDriverPack
PACK=$$PWD/packages
BIN=c:/Qt/QtIFW-3.2.2/bin

master.input = FROM
master.output = TestDriverInstaller
master.commands = $$BIN/archivegen $$PACK/TestDriverInstaller/data/Data.7z \
    $$FROM/platforms $$FROM/imageformats $$FROM/TestDriver.exe \
  $$escape_expand(\n\t) \
  $$BIN/archivegen $$PACK/UpdaterInstaller/data/Data.7z $$FROM/Updater.exe $$FROM/xpress99.ico \
  $$escape_expand(\n\t) \
  $$BIN/archivegen $$PACK/LibraryInstaller/data/Data.7z $$FROM/FormulaPainter.dll $$FROM/CommonWidgets.dll \
    $$FROM/Mathematics.dll $$FROM/TaskFileManager.dll $$FROM/vcredist_x64.exe \
  $$escape_expand(\n\t) \
  $$BIN/archivegen $$PACK/Qt515MinGWInstaller/data/Data.7z $$FROM/Qt5Widgets.dll $$FROM/Qt5Gui.dll $$FROM/Qt5Core.dll \
    $$FROM/Qt5Network.dll $$FROM/Qt5Multimedia.dll $$FROM/libgcc_s_seh-1.dll $$FROM/libstdc++-6.dll $$FROM/libwinpthread-1.dll \
    $$FROM/libssl-1_1-x64.dll $$FROM/libcrypto-1_1-x64.dll $$FROM/Qt5Charts.dll \
  $$escape_expand(\n\t) \
  $$BIN/repogen -p $$PACK --update repository \
  $$escape_expand(\n\t) \
  $$BIN/binarycreator --online-only -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT}
master.CONFIG += target_predeps no_link combine
QMAKE_EXTRA_COMPILERS += master
