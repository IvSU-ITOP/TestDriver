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
  Path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("TestDriver.exe");
  Args.clear();
  QProcess::startDetached(Path, Args);
  return 0;
  }
