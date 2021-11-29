#include "WinTesting.h"
#include "SelectTask.h"


//#ifdef LEAK_DEBUG

QFile s_LogFile( "C:\\ProgramData\\Halomda\\Log.txt" );
QDebug s_Debug( &s_LogFile );
QApplication *pA;

void MessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
  {
  QByteArray localMsg = msg.toLocal8Bit();
  static int LogCount = 0;
  if( ++LogCount >= 10000 )
    {
    s_LogFile.resize( 0 );
    LogCount = 1;
    }
  switch( type ) {
    case QtDebugMsg:
      s_LogFile.write( "Debug: " + localMsg + "\r\n" );
      break;
    case QtInfoMsg:
      s_LogFile.write( "Info: " + localMsg + "\r\n" );
      break;
    case QtWarningMsg:
      s_LogFile.write( "Warning: " + localMsg + "\r\n" );
      break;
    case QtCriticalMsg:
      s_LogFile.write( "Critical: " + localMsg + "\r\n" );
      break;
    case QtFatalMsg:
      s_LogFile.write( "Fatal: " + localMsg + "\r\n" );
      abort();
    }
  s_LogFile.flush();
  }
//#endif

int main( int argc, char *argv[] )
  {
  QApplication a( argc, argv );
  int iA[] = {2, 1, 4, -2, 3}, iProd;
  int *pA = iA, *pEnd = pA + sizeof( iA ) / sizeof(int);
  for(iProd = 1; pA  <= pEnd  && *pA > 0; iProd *= *pA++);
//  pA = &a;
//#ifdef LEAK_DEBUG
  QDir Dir("C:\\ProgramData");
  if( !Dir.exists("Halomda")) Dir.mkdir("Halomda");
  s_LogFile.open( QIODevice::WriteOnly );
  qInstallMessageHandler( MessageOutput );
//#endif
  CreateRecode();
  WinTesting::sm_ApplicationArguments = QCoreApplication::arguments();
  QStringList &Args = WinTesting::sm_ApplicationArguments;
  bool bTaskFromBrowser = QCoreApplication::arguments().count() == 7;
  QString Url;
  if( bTaskFromBrowser ) Url = Args[2].left( Args[2].lastIndexOf( '/' ) + 1 );
  bool bContinue;
  enum ExecStage{ exStart, exSelectTest, exSelectChapter, exSelectTopic };
  ExecStage Stage = exStart;
  QByteArray UsrId, TaskMode, TopicId, ChapId, TestDir, TestId, PrmId;
  QString ChapterName, TestName;
  bool bLocalWork = false;
  do
    {
    bool bTestingMode; 
    try
      {
      if( bTaskFromBrowser )
        {
        bTestingMode = bTaskFromBrowser && Args[5] == "wrkExam";
        bContinue = false;
        //        throw Args.join( ' ' );
        QByteArray TopicId = Args[4].toLocal8Bit();
        Connector C( Url + "GetTaskMode.php", "TopicId=" + TopicId );
        TaskMode = C.Connect();
        if( TaskMode.isEmpty() || TaskMode == "Error" ) throw "Get Task Name Error, URL: " + Url + "GetTaskMode.php, TopicId=" + TopicId;
        }
      else
        {
        switch( Stage )
          {
          case exStart:
            {
            PasswordDialog Dlg;
            if( Dlg.exec() == QDialog::Rejected ) return 0;
            bLocalWork = Dlg.m_LokalWork;
            bContinue = !bLocalWork;
            if( bLocalWork ) break;
            UsrId = Dlg.m_UsrId;
            if( UsrId == "Moodle user" )
              {
              CreateMoodleBank MDlg;
              MDlg.exec();
              return 0;
              }
            }
          case exSelectTest:
            {
            SelectTest ST( UsrId );
            if( ST.exec() == QDialog::Rejected ) return 0;
            QByteArray Parms( ST.m_pListTests->currentData().toByteArray() );
            QByteArrayList LParms( Parms.split( ',' ) );
            if( LParms.count() != 3 ) throw QString( "Main Menu parameters error: " + Parms );
            PrmId = LParms[1];
            TestId = LParms[0];
            TestDir = LParms[2];
            TestName = ST.m_pListTests->currentText();
            }
          case exSelectChapter:
            {
            MainTestDlg MDlg( TestId, UsrId, TestName );
            if( MDlg.exec() == QDialog::Rejected ) return 0;
            Stage = exSelectTest;
            if( MDlg.m_BackToSelectTest ) continue;
            ChapId = MDlg.m_SelectedChapter;
            ChapterName = MDlg.m_ChapterName;
            }
          case exSelectTopic:
            {
            SelectTopicDlg TDlg( PrmId, UsrId, ChapId, ChapterName );
            Stage = exSelectChapter;
            if( !TDlg.m_NoSelection )
              {
              if( TDlg.exec() == QDialog::Rejected ) return 0;
              if( TDlg.m_BackToSelectChapter ) continue;
              Stage = exSelectTopic;
              }
            QByteArrayList LParms( TDlg.m_SelectedTopic.split( ',' ) );
            TopicId = LParms[0];
            TaskMode = LParms[1];
            }
          }
        }
      if( !bLocalWork )
        {
        if( TaskMode == "Error" ) throw QString( "Error by open task" );
        if( !bTaskFromBrowser )
          {
          bTestingMode = ChapId[0] == 'E';
          Args.clear();
          Args << "TestingDriver.exe" << "3844" << ( PasswordDialog::sm_RootUrl + TestDir ) <<
            "HE" << TopicId << ( bTestingMode ? "wrkExam" : ( ChapId[0] == 'L' ? "wrkLearn" : "wrkTrain" ) ) << UsrId;
          }
        if( TaskMode == "Old" )
          {
          Args.removeAt( 0 );
          OldDriverStarter Starter;
          Starter.Start( Args.join( ' ' ) );
          continue;
          }
        }
      }   
    catch( QString Err )
      {
      QMessageBox::critical( nullptr, "Net Error", Err );
      return 1;
      }
    try {
    WinTesting WT;
    WT.resize( ScreenSize );
    Panel::sm_pPanel->setExam( bTestingMode && !bLocalWork );
    WT.show();
#ifdef Q_OS_ANDROID
    WT.setWindowState( Qt::WindowFullScreen );
#else
    WT.setWindowState( Qt::WindowMaximized );
//    SetWindowPos( ( HWND ) WT.winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
#endif
    a.exec();
    }
    catch(...)
      {
      return 0;
      }
      } while( bContinue );
  return 0;
  }

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
  {
  return main(0, nullptr);
  }
