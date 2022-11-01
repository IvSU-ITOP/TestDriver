#include "WinTesting.h"
#include "SelectTask.h"
#include <QFileDialog>


//#ifdef LEAK_DEBUG

QFile s_LogFile( "C:\\ProgramData\\Halomda\\Log.txt" );
QDebug s_Debug( &s_LogFile );
QApplication *pA;
QString s_AppPath;

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
  s_AppPath = a.applicationDirPath();
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
  bool bContinue = false;
  enum ExecStage{ exStart, exSelectTest, exSelectChapter, exSelectTopic, exGetGroup };
  ExecStage Stage = exStart;
  QByteArray UsrId, TaskMode, TopicId, ChapId, TestDir, TestId, PrmId;
  QString ChapterName, TestName, StudentsList, UserLogin;
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
            if(UsrId[0] == 'L')
              {
              UsrId = UsrId.mid(1);
              Stage = exGetGroup;
              UserLogin = Dlg.m_UsrLogin;
              continue;
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
            break;
            }
          case exGetGroup:
            {
            SelectGroup SG( UsrId, UserLogin );
            if( SG.exec() == QDialog::Rejected ) return 0;
            QByteArray GroupCode;
            int Added = 0, Restored = 0, Existed = 0, AddedToGroup = 0;
            auto AddStudent = [&](QString& FName, QString& LName, QString& ID)
              {
              QByteArray Command = "FirstName=" + EdStr::sm_pCodec->fromUnicode(FName) +
                "&LastName=" + EdStr::sm_pCodec->fromUnicode(LName) +
                "&Id=" + EdStr::sm_pCodec->fromUnicode(ID);
              Connector C( PasswordDialog::sm_RootUrl + "AddStudent.php", Command + "&Group=" + GroupCode );
              QByteArray Response = C.Connect();
              Response = Response.mid(Response.lastIndexOf('\n') + 1);
              if( Response == "Error" )
                QMessageBox::information(nullptr, "Error", "For student " + FName + " " + LName + " password was not defined" );
              else
                {
                switch( Response[0] )
                  {
                  case '0':
                    Existed++;
                    break;
                  case '1':
                    Added++;
                    break;
                  case '2':
                    Restored++;
                  }
                if(Response.length() == 2 ) AddedToGroup++;
                }
              };
            auto AddStudents = [&] ()
              {
              QString FileName = QFileDialog::getOpenFileName(nullptr, "Select students List");
              if(FileName == "") return false;
              QFile F(FileName);
              F.open(QIODevice::ReadOnly);
              QTextStream Stream(&F);
              QString Line, FName, LName, ID;
              do
                {
                Stream >> Line;
                } while( !Stream.atEnd() && Line.right(1) != ')');
              if(Stream.atEnd()) throw "This File is not students list!";
              while(true)
                {
                Stream >> FName;
                if( Stream.atEnd()) break;
                Stream >> LName;
                Stream >> ID;
                AddStudent(FName, LName, ID);
                }
              return true;
              };
            QString GroupName = SG.m_NewGroup;
            if(GroupName.isEmpty())
              {
              GroupCode = SG.m_pListGroups->currentData().toByteArray();
              GroupName = SG.m_pListGroups->currentText();
              SelectWork SW(GroupName);
              if( SW.exec() == QDialog::Rejected )
                continue;
              if(GroupName != SW.m_pGroupName->text())
                {
                QByteArray Command = "GroupName=" + EdStr::sm_pCodec->fromUnicode(SW.m_pGroupName->text());
                Connector C( PasswordDialog::sm_RootUrl + "ChangeGroupName.php", Command + "&GroupId=" + GroupCode );
                QByteArray Response = C.Connect();
                if(Response == "OK")
                  {
                  QMessageBox::information(nullptr, "Change group name", "Group name was successfully changed");
                  GroupName = SW.m_pGroupName->text();
                  }
                else
                  {
                  if(Response == "Exists")
                    Response = "Can't change groupname\r\nFor this Lecturer group with this name already exists";
                  QMessageBox::information(nullptr, "Error by change group name", Response);
                  }
                }
              QString FirstName = SW.m_pFirstName->text();
              if(FirstName.isEmpty())
                {
                if(SW.m_pLoadResult->isChecked())
                  {
                  try {
                  SelectWork::LoadResult(GroupCode.toInt());
                  }
                  catch(...)
                    {
                    }
                  continue;
                  }
                if( AddStudents() == false ) continue;
                }
              else
                {
                QString LastName = SW.m_pLastName->text(), ID = SW.m_pPassword->text();
                AddStudent(FirstName, LastName, ID);
                }
              }
            else
              {
              QByteArray Command = "GroupName=" + EdStr::sm_pCodec->fromUnicode(GroupName);
              Connector C( PasswordDialog::sm_RootUrl + "AddGroup.php", Command + "&LecturerId=" + UsrId );
              QByteArray Response = C.Connect();
              bool bOK;
              Response.toLongLong(&bOK);
              if(!bOK)
                {
                if(Response == "Exists")
                  Response = "Can't add new group\r\nFor this Lecturer group with this name already exists";
                QMessageBox::information(nullptr, "Error", Response);
                continue;
                }
              GroupCode = Response;
              if(!AddStudents()) continue;
              }
           QMessageBox::information(nullptr, "Load Result",
             "Students new:" + QString::number(Added) +
             " Existed:" + QString::number(Existed) +
             " Restored:" + QString::number(Restored) +
             " Added to Group:" + QString::number(AddedToGroup) );
             Stage = exStart;
             continue;
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
