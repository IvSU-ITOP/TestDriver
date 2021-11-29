#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QSettings>

int main(int argc, char *argv[])
  {
  QCoreApplication a(argc, argv);
  QString Path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("maintenancetool.exe");
  QProcess Process;
  QStringList Args;
  Args.append("--checkupdates");
  Process.start(Path, Args);
  Process.waitForFinished();
  QByteArray Data = Process.readAllStandardOutput();
  if(!Data.isEmpty())
    {
    Args.clear();
    Args.append("--updater");
    Process.start(Path, Args);
    Process.waitForFinished();
    }
  QSettings S("HKEY_CLASSES_ROOT\\Installer\\Products\\1926E8D15D0BCE53481466615F760A7F", QSettings::NativeFormat);
  if( S.allKeys().isEmpty() )
    {
    Path = '"' + QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("vcredist_x64.exe") + '"';
    system(Path.toLocal8Bit());
    }
  Path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("TestDriver.exe");
  Args.clear();
  QProcess::startDetached(Path, Args);
  return 0;
  }
