#include "DataServer.h"
#include "../FormulaPainter/InEdit.h"
#include <qsqldatabase.h>
#include <QSqlQuery>
#include <QSqlError>

const int s_FontSize = 16;
const int s_PowDecrease = 5;
DataServer *s_pDataServer;
QByteArray s_MainUrl;
QSqlDatabase DB;
QMutex s_Critical;

QFile s_LogFile( QString( s_Temp ) + "Log.txt" );
QDebug s_Debug( &s_LogFile );
QMutex s_Mutex;
QByteArray s_PWD;
QByteArray s_MainDir;

void MessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
  {
  QMutexLocker locker(&s_Mutex);
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


void Message( const QString& Msg )
  {
  throw ErrParser( Msg, ParserErr::peNewErr );
  }

DataServer::DataServer() : QTcpServer(), m_ConnectionsCount(0), m_Port(5060)
  {
  s_pDataServer = this;
  s_CalcOnly = true;
  CreateRecode();
  QString FontMath( "Cambria Math" );
  QFont MainFont( FontMath, s_FontSize, QFont::Normal );
  QFont PowerFont( MainFont );
  PowerFont.setPointSize( MainFont.pointSize() - s_PowDecrease );
  BaseTask::sm_pEditSets = new EditSets( MainFont, PowerFont, PowerFont, QString( "white" ), QString( "black" ), QString( "black" ) );
  XPInEdit::sm_AddHeight = 2;
  };

bool DataServer::StartServer()
  {
  if( !listen( QHostAddress::Any, m_Port ) ) return false;
  qDebug() << "Listen! Port:" << m_Port;
  XPInEdit::sm_Message = Message;
  return true;
  }

void DataServer::incomingConnection( qintptr socketDescriptor )
  {
  qInstallMessageHandler( MessageOutput );
  qDebug() << socketDescriptor << " socketDescriptor";
  Thread *pThread = new Thread(socketDescriptor, this);
  connect( pThread, SIGNAL( finished() ), pThread, SLOT( deleteLater() ) );
  pThread->start();
  }

void Thread::run()
  {
  qInstallMessageHandler( MessageOutput );
  qDebug() << m_SocketDescriptor << " Starting thread";
  m_pSocket = new QTcpSocket();
  if( !m_pSocket->setSocketDescriptor( m_SocketDescriptor ) ) //1
    {
    emit Error( m_pSocket->error() );
    return;
    }
  m_Time.start();
  connect( m_pSocket, SIGNAL( readyRead() ), this, SLOT( ReadyRead() ), Qt::DirectConnection ); //2
  connect( m_pSocket, SIGNAL( disconnected() ), this, SLOT( Disconnected() ), Qt::DirectConnection ); //3
  qDebug() << m_SocketDescriptor << " Client connected";
  exec(); //4
  }

  void Thread::ReadyRead()
  {
  QByteArrayList Parms( m_pSocket->readAll().split( '&' ) );
  QMutexLocker locker(&s_Critical);
  qDebug() << "StartReadyRead";
  if( !DB.isValid() ) DB = QSqlDatabase::addDatabase( "QMYSQL" );
  qDebug() << "QMYSQL Added";
  s_MemorySwitch = SWtask;
  ExpStore::sm_pExpStore->Clear();
  QByteArray Ext;
  try
    {
    uint iHiperRef = 0;
    if( !DB.isValid() ) throw ErrParser( "Error in: QMYSQL", ParserErr::peNewErr );
    if(Parms.count() < 9 && Parms.count() != 2 && Parms.count() != 4)
      {
      QByteArray sTskId;
      QByteArray Task;
      QByteArray TaskName;
      QByteArray Ref = Parms[0];
      for(; Parms.count() < 9; Parms.append(""));
      Parms[prmTaskType] = "wrkTrain";
      Parms[prmDatabase] = "testingdriver";
      if(Parms.count() == 1)
        {
        Parms[prmPassword] = s_PWD;
        iHiperRef = 1;
        if(Ref.left(9) != "GET /Task") return;
        int iSpace = Ref.indexOf(" ", 9);
        if(iSpace == -1) return;
        sTskId = Ref.mid(10, iSpace - 10);
        int iTskId = sTskId.toInt();
        if(iTskId <= 0) return;
        DB.setDatabaseName( "pictures" );
        DB.setUserName( "root" );
        DB.setHostName( "localhost" );
        DB.setPassword( s_PWD );
        if( !DB.open() )
          throw ErrParser( "Error; Can't open database: pictures", ParserErr::peNewErr );
        QSqlQuery Query( DB );
        QString Q( "Select Type, Formula, Name From pictures Where Id=" + sTskId );
        Query.exec( Q );
        if(Query.size() <= 0) return;
        Query.next();
        if(Query.value( 0 ).toByteArray() != "Task") return;
        QByteArray BaseTask(Query.value( 1 ).toByteArray());
        int Offset = 0;
        for( int iPos = BaseTask.indexOf("#H:"); iPos != -1; iPos = BaseTask.indexOf("#H:", Offset ) )
          {
          Task += BaseTask.mid(Offset, iPos - Offset ) + QByteArray::fromHex(BaseTask.mid(iPos + 3, 2 ) );
          Offset = iPos + 5;
          }
        Task += BaseTask.mid(Offset);
        TaskName = Query.value( 2 ).toByteArray();
        QByteArray MainDir(s_MainDir + "TasksByReference/");
        QByteArray TskDir =  MainDir + sTskId + '/';
        Parms[prmPathFile] = TskDir + TaskName;
        QDir Dir(MainDir);
        if(!Dir.exists())
          {
          Dir.setPath(s_MainDir);
          Dir.mkdir("TasksByReference");
          Dir.setPath(MainDir);
          }
        if(!Dir.exists(Parms[prmPathFile]))
          {
          Dir.mkdir(sTskId);
          QFile F(Parms[prmPathFile]);
          F.open(QIODevice::WriteOnly);
          F.write(Task);
          F.close();
          Q = "Select FileName, Picture From taskpictures Where TaskId=" + sTskId;
          Query.exec( Q );
          while( Query.next() )
            {
            QByteArray Picture(QByteArray::fromBase64(Query.value( 1 ).toByteArray()));
            F.setFileName(TskDir + Query.value( 0 ).toByteArray());
            F.open(QIODevice::WriteOnly);
            F.write(Picture);
            F.close();
            }
          }
        DB.close();
        }
      else
        {
        int iSlash = Ref.lastIndexOf('/');
        if(iSlash == -1) return;
        int iPrevSlash = Ref.lastIndexOf('/', iSlash - 1);
        if(iPrevSlash == -1) return;
        QByteArray P, PP(Parms[1]);
        for(int i = PP.length() - 1; i >= 0; P += PP[i--] );
        Parms[prmPassword] = P;
        Parms[prmPathFile] = Parms[2] + Ref;
        QFile F(Parms[prmPathFile]);
        if( !F.open(QIODevice::ReadOnly) ) return;
        Task = F.readAll();
        F.close();
        int iTask = Task.indexOf("TASK");
        if(iTask == -1) return;
        TaskName = Ref.mid(iSlash + 1);
        sTskId = Ref.mid(iPrevSlash + 1, iSlash - iPrevSlash - 1);
        iHiperRef = 2;
        }
      DB.setDatabaseName( Parms[prmDatabase] );
      DB.setUserName( "root" );
      DB.setHostName( "localhost" );
      DB.setPassword( Parms[prmPassword] );
      if( !DB.open() ) throw ErrParser( "Error; Can't open database: pictures", ParserErr::peNewErr );
      QByteArray Q = "Select usr_id From user Where usr_Id_Card='Default Id Card'";
      QSqlQuery TDQuery( DB );
      TDQuery.exec( Q );
      TDQuery.next();
      Parms[prmUser] = TDQuery.value( 0 ).toByteArray();
      Q = "Select Test_id From tests Where test_name='TestsByReference'";
      QByteArray TestId;
      TDQuery.exec( Q );
      if(TDQuery.next())
        TestId = TDQuery.value( 0 ).toByteArray();
      else
        {
        TDQuery.exec("Insert Into Tests(test_dir, test_name, test_month, test_day) Values('TasksByReference','TestsByReference', 12, 31)" );
        TestId = TDQuery.lastInsertId().toByteArray();
        }
      Q = "Select tpc_id From topic Inner Join chapter On topic.chp_id = chapter.chp_id Where chp_name='" + sTskId + "' and tst_id = " + TestId;
      TDQuery.exec( Q );
      if(TDQuery.next())
        Parms[prmTopic] = TDQuery.value( 0 ).toByteArray();
      else
        {
        Q = "Insert Into chapter(tst_id, chp_name, TestTopicCount) Values(" +
        TestId + ",'" + sTskId + "',0)";
        TDQuery.exec( Q );
        QByteArray ChapterId = TDQuery.lastInsertId().toByteArray();
        QByteArray TopicName;
        int iNameStart = Task.indexOf("'") + 1;
        if(iNameStart != 0 )
          {
          int iNameEnd = Task.indexOf("'", iNameStart);
          TopicName = Task.mid(iNameStart, iNameEnd - iNameStart);
          }
        QString  SQ = "Insert Into topic(chp_id, tpc_name, tpc_task_file, tpc_col, FullResult) Values(" +
            ChapterId + ",'" + ToLang(TopicName) + "','" + TaskName + "',0,10)";
        TDQuery.exec( SQ );
        Parms[prmTopic] = TDQuery.lastInsertId().toByteArray();
        }
      Parms[prmHid] = "0";
      Parms[prmURL] = "192.168.1.1";
      Ext = TaskName.mid(TaskName.indexOf('.') + 1);
      if(Ext == "heb" || Ext == "HEB")
        Parms[prmCharset] = "Windows-1255";
      else
        Parms[prmCharset] = "Windows-1251";
      }
    else
      if( Parms.count() < prmCharset + 1 )
        {
        DataTask Task;
        QByteArray Formulas;
        for( int i = 0; i < Parms.count(); i++ )
          {
          if( i > 0 ) Formulas += "; ";
          Formulas += Parms[i];
          }
        qDebug() << "Start to compare" << m_SocketDescriptor << " Compared formulas" << Formulas << " Connections Count : "
          << ++s_pDataServer->m_ConnectionsCount;
        TStr::sm_Server = true;
        MathExpr Expr = MathExpr( Parser::StrToExpr( Parms[0] ) );
        TStr::sm_Server = false;
        if( s_GlobalInvalid || Expr.IsEmpty() ) throw ErrParser( "Syntax Error in: " + Parms[0], ParserErr::peNewErr );
#ifdef DEBUG_TASK
        qDebug() << "Contents " << CastPtr( TExpr, Expr )->m_Contents;
#endif
        int iEqual = 1;
        for( ; iEqual < Parms.count(); iEqual++ )
          {
          QByteArray  Parm(Parms[iEqual]), Formula;
          TStr::sm_Server = true;
          for( int iStartPack = Parm.indexOf("##"); iStartPack != -1; iStartPack = Parm.indexOf("##") )
            {
            Formula += Parm.left(iStartPack);
            QByteArray S(TStr::UnpackValue(Parm.mid(iStartPack)));
            Formula += S;
            Parm = Parm.mid(iStartPack + S.length() * 4 + 6);
            }
          TStr::sm_Server = false;
          Formula += Parm;
          QByteArrayList Formuls(Formula.split( '#' ));
          int i = 0;
          for( ; i < Formuls.count(); i++ )
            {
            if( Formuls[i].isEmpty() ) continue;
            MathExpr ExprT = MathExpr( Parser::StrToExpr( Formuls[i] ) );
            if( s_GlobalInvalid || ExprT.IsEmpty() ) throw ErrParser( "Error; Bad task, formula: " + Formuls[i], ParserErr::peNewErr );
          //        m_pSocket->write( Expr.WriteE() + ";;" + ExprT.WriteE() + "\n\n" );
          //        m_pSocket->flush();
          //        return;
#ifdef DEBUG_TASK
            qDebug() << "Contents expT" << CastPtr( TExpr, ExprT )->m_Contents;
#endif

            double Precision = TExpr::sm_Precision;
            TExpr::sm_Precision = 0.01;
            bool bResult = Expr.Equal( ExprT );
            TExpr::sm_Precision = Precision;
            if( bResult ) break;
            }
            if( i < Formuls.count() ) break;
          }
#ifdef DEBUG_TASK
        qDebug() << "Soket " << m_SocketDescriptor << " Count " << Parms.count() << " Equal " << iEqual;
#endif
        m_pSocket->write( QByteArray::number( iEqual ) + "\n\n" );
        m_pSocket->flush();
        return;
        }
    if( !DB.isValid() ) DB = QSqlDatabase::addDatabase( "QMYSQL" );
    qDebug() << "QMYSQL Added";
    DB.setDatabaseName( Parms[prmDatabase] );
    DB.setUserName( "root" );
    DB.setHostName( "localhost" );
    DB.setPassword( Parms[prmPassword] );
    s_PWD = Parms[prmPassword];
    s_MainDir = Parms[prmPathFile].left(Parms[prmPathFile].indexOf("TestingDriverMy") + 16);
//    DB.setPassword( "Jozefa,Niedzw." );
    if( !DB.open() ) throw ErrParser( "Error; Can't open database: " + Parms[prmDatabase] + "; " + Parms[prmPassword], ParserErr::peNewErr );
    QSqlQuery Query( DB );
    QString Q( "Select Distinct RndValues From BusyTopic Where tpc_id =" + Parms[prmTopic] );
    qDebug() << "Start to create task" << m_SocketDescriptor << "; Topic: " << Parms[prmTopic] << "; Work Type: " << Parms[prmTaskType] <<
      ", URL:" << Parms[prmURL] << ", Connections Count: " << ++s_pDataServer->m_ConnectionsCount;
    if( Parms.count() != prmCharset + 1 ) throw ErrParser( "Error; Number of parameters must be 8", ParserErr::peNewErr );
    Query.exec( Q );
    ArrBusy Busy;
    DataTask Task;
    while( Query.next() )
      {
      QByteArrayList Values( Query.value( 0 ).toByteArray().split( ',' ) );
      if( Busy.isEmpty() )
        {
        Busy.reserve( Values.count() );
        for( int i = 0; i < Values.count(); i++ )
          Busy.enqueue( BusyValues() << Values[i].toInt() );
        }
      else
        for( int i = 0; i < Values.count(); i++ )
          Busy[i] << Values[i].toInt();
      Task.SetBusy( Busy );
      }
    m_Critical = true;
    EdStr::sm_pCodec = QTextCodec::codecForName( Parms[prmCharset] );
    if(EdStr::sm_pCodec == nullptr ) throw ErrParser( "Error; Invalid Charset", ParserErr::peNewErr );
    s_MemorySwitch = SWtask;
    ExpStore::sm_pExpStore->Init_var();
    QByteArray TaskType(Parms[prmTaskType]);
    QByteArrayList PathFiles;
    if( BaseTask::GetLangByFileName( Parms[prmPathFile] ) == lngAll )
      {
      TaskType = "wrkKids";
      QByteArray Path(Parms[prmPathFile].left(Parms[prmPathFile].lastIndexOf('/') + 1 ));
      QFile TaskListFile(Parms[prmPathFile].replace("Tasks", "Series"));
      TaskListFile.open( QIODevice::ReadOnly );
      if( !TaskListFile.isOpen() )
        throw ErrParser( "Can't open task list file " + Parms[prmPathFile], ParserErr::peNewErr );
      QByteArray Line;
      TaskListFile.readLine();
      do
        {
        QByteArray FName = TaskListFile.readLine().trimmed();
        if( !FName.isEmpty() )  PathFiles.append(Path + FName + ".HEB");
       } while( !TaskListFile.atEnd());
      }
    else
      PathFiles.append(Parms[prmPathFile]);
    QByteArray FirstTask;
    char InstDots = 'a', PrevDot = '.';
    for(int iFile = 0; iFile < PathFiles.length(); iFile++)
      {
      Q = "Delete From Task where usr_id=" + Parms[prmUser] + " and URL='" + Parms[prmURL] + "'";
      Query.exec( Q );
      Task.SetWorkMode( TaskType );
      Task.SetFileName( PathFiles[iFile] );
      XPInEdit::sm_Language = Task.GetLanguage();
      Task.CalcRead();
      QString Comment( ToLang( Task.GetComment() ) );
      QString TrackDescription;
      QString TrackNames;
      QByteArray CurrentTrack( "0" );
      if( Task.m_pTrack->m_MultyTrack && Parms[prmTaskType] != "wrkExam" )
        {
        QByteArray TrackDescr;
        QByteArray TrackName;
        CurrentTrack = "-1";
        TrackDescr = Task.m_pTrack->m_TracksDescription->GetText();
        if(!TrackDescr.isEmpty()) TrackDescription = ToLang(TrackDescr);
        QByteArrayList &TaskTrackNames = Task.m_pTrack->m_NameOfTrack;
        for( int iTrack = 0; iTrack < TaskTrackNames.count(); )
          {
          TrackName += TaskTrackNames[iTrack];
          if( ++iTrack < TaskTrackNames.count() ) TrackName += "\n";
          }
        if(!TrackName.isEmpty()) TrackNames = ToLang(TrackName);
        }
      Q = "Insert Into Task( usr_id,tpc_id,H_Id,TaskType,Comment, URL, Description_tracks, TrackNames, CurrentTrack) Values(" + Parms[prmUser] + ',' +
        Parms[prmTopic] + ',' + Parms[prmHid] + ",'" + TaskType + "','" + Comment +
        "','" + Parms[prmURL] + "','" + TrackDescription + "','" + TrackNames + "'," + CurrentTrack + ')';
      Query.exec( Q );
      QByteArray idTask = Query.lastInsertId().toByteArray();
      qDebug() << "IdTask:" << idTask;
      if( idTask == "0" || idTask == "")
        {
        qDebug() << "Query:" << Q;
        Query.exec( "Select max(idTask) From Task Where usr_id=" + Parms[prmUser] + " and URL='" + Parms[prmURL] + "'");
        Query.exec( Q );
        Query.next();
        idTask = Query.value( 0 ).toByteArray();
        qDebug() << idTask << "2";
        }
      if( idTask == "0" || idTask  == "" ) throw ErrParser( "Error; Cant't add Task, query: " + Q, ParserErr::peNewErr );
      if(FirstTask.isEmpty()) FirstTask = idTask;
      Query.prepare( "Update Task Set QuestionWindow = ? where idTask = " + idTask );
      m_TempPath = QString( s_Temp ) + Parms[prmUser] + Parms[prmURL].replace( PrevDot, InstDots );
      PrevDot = InstDots++;
      QDir().mkpath( m_TempPath );
      ContentCreator CC( m_TempPath );
      Query.addBindValue( CC.GetContent( Task.m_pQuestion ) );
      if( !Query.exec() )
        throw ErrParser( "Error; Cant't add Content for QuestionWindow, topic: " + Parms[prmTopic], ParserErr::peNewErr );
      for( Task.m_pTrack->m_SelectedTrack = Task.m_pTrack->m_MultyTrack ? 1 : 0; Task.m_pTrack->m_SelectedTrack <= Task.m_pTrack->m_NameOfTrack.count(); Task.m_pTrack->m_SelectedTrack++ )
        {
        Task.ClearTrackDependent();
        Task.LoadTrackDependentFromFile();
        QString TrackId( QString::number( Task.m_pTrack->m_SelectedTrack ) );
        QString Q( "Insert Into HelpTask( idTask,TaskName,TrackId,Step) Values(" + idTask + ",'" + ToLang( Task.m_Name ) +
          "'," + TrackId + ",0)" );
        if( !Query.exec( Q ) )
          throw ErrParser( "Error; Cant't add HelpTask, query: " + Q, ParserErr::peNewErr );
        if( Parms[prmTaskType] != "wrkExam" )
          {
          Query.prepare( "Update HelpTask Set HelpText = ? where idHelpTask = " + Query.lastInsertId().toByteArray() );
          Query.addBindValue( CC.GetContent( Task.m_pMethodL ) );
          if( !Query.exec() )
            throw ErrParser( "Error; Cant't add Content for HelpTask, topic: " + Parms[prmTopic] + " Step = 0", ParserErr::peNewErr );
          }
        char iStep = '1';
        for( PStepMemb pStep = Task.m_pStepsL->m_pFirst; !pStep.isNull(); pStep = pStep->m_pNext, iStep++ )
          {
          Q = "Insert Into HelpTask( idTask,TaskName,TrackId,Step) Values(" + idTask + ",'" + ToLang( pStep->m_Name ) + "'," + TrackId + ',' + iStep + ')';
          if( !Query.exec( Q ) )
            throw ErrParser( "Error; Cant't add HelpTask, query: " + Q, ParserErr::peNewErr );
          if( TaskType != "wrkExam" )
            {
            Query.prepare( "Update HelpTask Set HelpText = ? where idHelpTask = " + Query.lastInsertId().toByteArray() );
            Query.addBindValue( CC.GetContent( pStep->m_pMethodL ) );
            if( !Query.exec() )
              throw ErrParser( "Error; Cant't add Content for HelpTask, topic: " + Parms[prmTopic] + " Step = " + iStep, ParserErr::peNewErr );
            }
          }

         auto AddFormula = [&] ( const QByteArray& Table, const PDescrMemb& pMemb, const QByteArray& Id, const QString& IdValue = QString()  )
          {
          MathExpr Expr;
          QByteArray Answer;
          for( PDescrMemb pM = pMemb; !pM.isNull(); pM = pM->m_pNext )
            {
            if( pM->m_Content.isEmpty() || pM->m_Kind != tXDexpress ) continue;
            MathExpr AExpr = MathExpr( Parser::StrToExpr( pM->m_Content ) );
            if( s_GlobalInvalid || AExpr.IsEmpty() ) throw ErrParser( "Error; Bad task, formula: " + pM->m_Content, ParserErr::peNewErr );
            if( Answer.isEmpty() )
              Expr = AExpr;
            else
              Answer += '#';
            TStr::sm_Server = true;
            Answer += AExpr.WriteE();
            TStr::sm_Server = false;
            if( Answer.isEmpty() ) Answer = pM->m_Content;
            }
          if( Answer.isEmpty() ) throw ErrParser( "Error; Bad task, all content is empty", ParserErr::peNewErr );
          QByteArray EdFormula( Expr.SWrite() );
          XPInEdit InEd( EdFormula, *BaseTask::sm_pEditSets, CC.ViewSettings() );
          if( Expr.HasStr() )
            {
            TStr::sm_Server = true;
            EdFormula = Expr.SWrite();
            TStr::sm_Server = false;
            }
          QByteArray FormulaPic;
          QBuffer Buffer( &FormulaPic );
          Buffer.open( QIODevice::WriteOnly );
          InEd.GetImage()->save( &Buffer, "JPG" );
          QString LastId = IdValue;
          if(LastId.isEmpty()) LastId = Query.lastInsertId().toString();
          QString Q( "Update " + Table + " Set Formula = '" +
            EdFormula.replace( '\\', "\\\\\\\\" ).replace( '\n', "\\\\n" ).replace( '\'', "\\'" ) + "', Image = '" + FormulaPic.toBase64() + "', Answer='" +
            Encode( Answer ).replace( '\'', "\\'" ) + "' where " + Id + " = " + LastId );
          if( !Query.exec( Q ) )
            throw ErrParser( "Error; Cant't add Formula, Query: " + Q, ParserErr::peNewErr );
          return LastId;
          };

        iStep = '1';
        bool bFirstStep = true;
        for( PStepMemb pStep = Task.m_pStepsL->m_pFirst; !pStep.isNull(); pStep = pStep->m_pNext, iStep++ )
          {
          if( bFirstStep )
            Query.exec( QString( "Update Task Set CurrentStep =" ) + iStep + " where idTask = " + idTask );
          bFirstStep = false;
          QString Q, IdHintValue;
          if( Parms[prmTaskType] == "wrkExam" )
            {
            if( pStep->m_Mark == 0 && !pStep->m_pNext.isNull() )
              {
              bFirstStep = true;
              continue;
              }
            Q = "Insert Into HintTask( idTask,Comment,Template,Step,Mark,NoHint,HeightEditorWindow) Values(" + idTask + ",'" +
              ToLang( pStep->GetComment() ) + "','" + Task.GetTemplate( iStep - '0' ) +
              "'," + iStep + ',' + QString::number( pStep->m_Mark ) + ',' + ('0' + pStep->m_ShowParms.m_NoHint) +
              ',' + QString::number( pStep->m_ShowParms.m_HeightEditorWindow ) + ')';
            if (!Query.exec(Q))
              throw ErrParser("Error; Cant't add HintTask, query: " + Q, ParserErr::peNewErr);
            if (!pStep->m_pAnswerPrompt->m_pFirst.isNull())
              {
              IdHintValue = Query.lastInsertId().toString();
              Query.prepare("Update HintTask Set AnswerPrompt = ? where idHintTask = " + Query.lastInsertId().toByteArray());
              Query.addBindValue(CC.GetContent(pStep->m_pAnswerPrompt));
              if (!Query.exec())
                throw ErrParser("Error; Cant't add Content for AnswerPrompt, topic: " + Parms[prmTopic] + " Step = " + iStep, ParserErr::peNewErr);
              }
            }
          else
            {
            Q = "Insert Into HintTask( idTask,Comment,Template,Step,Track, HeightEditorWindow) Values(" + idTask + ",'" +
              ToLang(pStep->GetComment()) + "','" + Task.GetTemplate(iStep - '0') + "'," + iStep + ',' + TrackId +
                ',' + QString::number( pStep->m_ShowParms.m_HeightEditorWindow ) + ')';
            if (!Query.exec(Q))
              throw ErrParser("Error; Cant't add HintTask, query: " + Q, ParserErr::peNewErr);
            }
          QString idHint = AddFormula( "hinttask", pStep->m_pResultE->m_pFirst, "idHintTask", IdHintValue );
          if( TaskType != "wrkLearn" )
            {
            PDescrMemb pNext = pStep->m_pF1->m_pFirst->m_pNext;
            QString FalseComm = pNext.isNull() ? "" : ToLang( pNext->m_Content );
            Q = "Insert Into SubHint( idHintTask, Comment) Values(" + idHint + ",'" + FalseComm + "')";
            if( !Query.exec( Q ) )
              throw ErrParser( "Error; Cant't add SubHint, query: " + Q, ParserErr::peNewErr );
            AddFormula( "SubHint", pStep->m_pF1->m_pFirst, "idSubHint" );
            pNext = pStep->m_pF2->m_pFirst->m_pNext;
            FalseComm = pNext.isNull() ? "" : ToLang( pNext->m_Content );
            Q = "Insert Into SubHint( idHintTask, Comment) Values(" + idHint + ",'" + FalseComm + "')";
            if( !Query.exec( Q ) )
              throw ErrParser( "Error; Cant't add SubHint, query: " + Q, ParserErr::peNewErr );
            AddFormula( "SubHint", pStep->m_pF2->m_pFirst, "idSubHint" );
            pNext = pStep->m_pF3->m_pFirst->m_pNext;
            FalseComm = pNext.isNull() ? "" : ToLang( pNext->m_Content );
            Q = "Insert Into SubHint( idHintTask, Comment) Values(" + idHint + ",'" + FalseComm + "')";
            if( !Query.exec( Q ) )
              throw ErrParser( "Error; Cant't add SubHint, query: " + Q, ParserErr::peNewErr );
            AddFormula( "SubHint", pStep->m_pF3->m_pFirst, "idSubHint" );
            }
          }
        if( TaskType == "wrkExam" ) break;
        }
      }
    if( iHiperRef != 1)
      {
      m_pSocket->write( FirstTask + "\n\n" );
      m_pSocket->flush();
      if( TaskType == "wrkExam" )
        {
        QByteArray Q( "Insert Into BusyTopic( tpc_id, idTask, RndValues ) Values(" + Parms[prmTopic] + ',' + FirstTask + ",'" + Task.GetBusy() + "')" );
        Query.exec( Q );
        }
      }
    else
      {
      QByteArray Html = "<html><head><script>function Initiate(){window.location.href='" + s_MainUrl;
      Html += "/WebTestManager/StartTest.php?FirstTask=" + FirstTask + "&ext=" + Ext.toLower() + "';}";
      Html += "</script></head><body onload='Initiate()'></body></html>\r\n\r\n\r\n";
      m_pSocket->write( "HTTP/1.1 200 OK\r\nContent-Length: " +
        QByteArray::number(Html.length()) + "\r\nContent-Type: text/html\r\n\r\n");
      m_pSocket->write(Html);
      m_pSocket->flush();
      }
    }
  catch( ErrParser& ErrMsg )
    {
    m_pSocket->write( "HTTP/1.1 200 OK\r\n\r\n" );
//    qCritical() << ErrMsg.Message();
//    m_pSocket->write( '#' + ToLang(ErrMsg.Message()).toUtf8() + "\n\n" );
    m_pSocket->flush(); 
    TStr::sm_Server = false;
    }
#ifdef LEAK_DEBUG
  ExpStore::sm_pExpStore->Clear();
  qDebug() << "GrEl Created: " << TXPGrEl::sm_CreatedCount << ", GrElDeleted: " << TXPGrEl::sm_DeletedCount;
  qDebug() << "Expr Created: " << TExpr::sm_CreatedCount << ", deleted: " << TExpr::sm_DeletedCount << " Created List count: " << TExpr::sm_CreatedList.count();
  if( TExpr::sm_CreatedCount > TExpr::sm_DeletedCount )
    {
    qDebug() << "Unremoved expressions";
    for( auto pExpr = TExpr::sm_CreatedList.begin(); pExpr != TExpr::sm_CreatedList.end(); pExpr++ )
      qDebug() << ( *pExpr )->m_Contents;
    }
#endif
  }

void Thread::Disconnected()
  {
  s_pDataServer->m_ConnectionsCount--;
  if( !m_TempPath.isEmpty() ) QDir( m_TempPath ).removeRecursively();
  qDebug() << "End " << m_SocketDescriptor << " msk elapsed: " << m_Time.elapsed();
  exit( 0 );
  }

QByteArray ContentCreator::GetContent( PDescrList List )
  {
//  qDebug() << "ContentCreator";
  SetContent( List );
//  qDebug() << "ContentCreator1";
  QByteArray  Content = toHtml().toUtf8(), Result;
  Content.remove( 0, Content.indexOf( "<style" ) );
  Content.remove( Content.indexOf( "</head>" ), 7);
  Content.remove( Content.lastIndexOf( "</html>" ), 7 );
  Content.replace( Content.indexOf( "body" ), 4, "span", 4 );
  Content.replace( Content.lastIndexOf( "body" ), 4, "span", 4 );
  Content.insert( Content.indexOf( '>', Content.indexOf( "<table" ) + 7 ), " width=\"100%\"" );
  int iStart = 0;
  QFile File;
//  qDebug() << "ContentCreator2";
  for( int iEnd; ( iEnd = Content.indexOf( "<img", iStart ) ) != -1;  )
    {
    int iStartPath = iEnd + 10;
    iEnd = Content.indexOf( '"', iStartPath );
    File.setFileName( Content.mid( iStartPath, iEnd - iStartPath ) );
    File.open( QIODevice::ReadOnly );
    Result += Content.mid( iStart, iStartPath - iStart );
    Result += "data:image/jpg;base64,";
    Result += File.readAll().toBase64();
//    qDebug() << "Result" << Result;

    File.close();
    Result += '"';
    iStart = Content.indexOf( "/>", ++iEnd ) + 2;
    Result += Content.mid( iEnd, iStart - iEnd );
    }
  Result += Content.mid( iStart );
//  qDebug() << Result;
  return Result;
//  return Result += Content.mid( iStart );
  }

RefServer::RefServer() : DataServer()
  {
  m_Port = 26491;
  }
/*
void RefServer::incomingConnection( qintptr socketDescriptor )
  {

  }
*/
