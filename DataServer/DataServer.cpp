#include "DataServer.h"
#include "../FormulaPainter/InEdit.h"
#include <qsqldatabase.h>
#include <QSqlQuery>
#include <QSqlError>
#include "../Mathematics/Algebra.h"
#include "../Mathematics/SolChain.h"
#include "../Mathematics/Analysis.h"

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

void Thread::Solve( Solver *pS)
  {
  auto Final = [&]()
    {
    delete pS;
    return;
    };
  try
    {
    TExpr::sm_CalcOnly = true;
    ExpStore::sm_pExpStore->Init_var();
    pS->SetExpression( m_Expression );
    TExpr::sm_CalcOnly = false;
    if (s_GlobalInvalid) return Final();
    }
  catch( ErrParser )
    {
    TExpr::sm_CalcOnly = false;
    return Final();
    }
  MathExpr Result = pS->Result();
  if( Result.IsEmpty() ) return Final();
  bool V;
  if( Result.Boolean_( V ) )
    {
    if( !V ) return Final();
    }
  else
    if( !pS->Success() ) return Final();
  m_SolvIndexes.append(pS->Code());
  return Final();
  }

void Thread::SearchSolve(QByteArray& Formula)
  {
  if( Formula.isEmpty() ) return;
  s_GlobalInvalid = false;
  m_Expression = Parser::StrToExpr( Formula );
  if( m_Expression.IsEmpty() || s_GlobalInvalid ) return;
//    {
//    WinTesting::sm_pOutWindow->AddComm( X_Str( "MsyntaxErr", "Syntax error!" ) );
//    return;
//    }
  TSolutionChain::sm_SolutionChain.Clear();
  TSolutionChain::sm_SolutionChain.m_Accumulate = false;

  QString Prompt;
//  auto Final = [&] ( bool Solve )
  auto Final = [&] ()
    {
    TSolutionChain::sm_SolutionChain.m_Accumulate = true;
//    if (m_pButtonBox->m_ButtonCount > 0) m_pButtonBox->m_pPrompt->setText( Prompt );
//    if( !Solve ) return;
//    if( !m_pButtonBox->SolveDefault() )
//    WinTesting::sm_pOutWindow->AddComm( X_Str( "MNotSuitableExpr", "Not suitable expression!" ) );
    };
  m_Formula = Formula;
  if( m_Expression.ConstExpr() || m_Expression.HasComplex() || m_Expression.HasMatrix() )
    {
    if( IsType( TConstant, m_Expression ) )
      {
      Solve( new TSin );
      Solve( new TCos );
      Solve( new TTan );
      Solve( new TLg );
      Solve( new TLn );
      Solve( new TDegRad );
      Solve( new TRadDeg );
      Solve( new TSciCalc );
      Prompt = X_Str( "MCanEvaluate", "For this value you can calculate:" );
      return Final();
      }
    Solve( new TSciCalc );
    return Final();
    }

  bool TestNumerical = false;
  auto SearchSystem = [&] ( const MathExpr& Ex )
    {
    PExMemb pMemb;
    if( !Ex.Listex( pMemb ) ) return false;
    MathExpr Left, Right;
    uchar Sign;
    if( !pMemb->m_Memb.Binar_( Sign, Left, Right ) ) return true;
    if( Sign != '=' )
      {
      Solve( new SysInEqXY );
      Solve( new SysInEq );
      Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
      return true;
      }
    PExMemb pNext = pMemb->m_pNext;
    if( pNext.isNull() ) return true;
    if( TestNumerical && pNext->m_Memb.IsNumerical() )
      {
      pNext = pNext->m_pNext;
      if( pNext.isNull() || !pNext->m_Memb.IsNumerical() ) return true;
      Solve( new TSolvCalcEquation );
      return true;
      }
    if( !pNext->m_Memb.Binar( '=', Left, Right ) ) return true;
    Solve( new MakeSubstitution );
    Solve( new MakeExchange );
    Solve( new SolveLinear );
    Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
    return true;
    };

  MathExpr SysExpr;
  if( m_Expression.Syst_( SysExpr ) )
    {
    SearchSystem( SysExpr );
    return Final();
    }

  TestNumerical = true;
  if( SearchSystem( m_Expression ) ) return Final();

  MathExpr Left, Right;
  uchar Sign;
  if( m_Expression.Binar_( Sign, Left, Right ) )
    {
    if( Sign != '=' )
      {
      Solve( new RatInEq );
      Solve( new SysInEq );
      Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
      return Final();
      }
    Solve( new TSolvDetLinEqu );
    if( m_SolvIndexes.count() == 1) return Final();

    Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
    Solve( new TSolvDetQuaEqu );
    Solve( new TSolvDisQuaEqu );
    Solve( new TSolvDetVieEqu );
    Solve( new TSolvQuaEqu );
    if( m_SolvIndexes.count() > 0 ) return Final();

    Solve( new TSolvCalcPolinomEqu );
    Solve( new Log1Eq );
    Solve( new ExpEq );
    Solve( new TSolvCalcDetBiQuEqu );
    Solve( new TSolvFractRatEq );
    Solve( new TSolvCalcIrratEq );
    if( Formula.indexOf( "sin" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq );
    if( Formula.indexOf( "tan" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq );
    if( Formula.indexOf( "cos" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq );
    if( Formula.indexOf( "cot" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq );
    Solve( new TSolvCalcSinCosEqu );
    Solve( new TSolvCalcTrigoEqu );
    Solve( new TSolvFractRatEq );
    Solve( new TSolvCalcHomogenTrigoEqu );
    return Final();
    }
  Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
  int PosPow = Formula.indexOf( '^' );
  if( PosPow != -1 )
    {
    if( Formula.indexOf( '^', PosPow + 1 ) != -1 )
      {
      Solve( new TSolvSubSqr );
      Solve( new TSolvSumCub );
      Solve( new TSolvSubCub );
      if( m_SolvIndexes.count() > 0 ) return Final();
      }
    else
      {
      Solve( new TSolvSqrSubSum );
      Solve( new TSolvCubSubSum );
      }
    }
  Solve( new TSolvReToMult );
  Solve( new TSolvExpand );
  Solve( new TTrinom );
//  if( m_SolvIndexes.count() > 0 ) return Final();
  Solve( new Alg1Calculator );
  if( m_SolvIndexes.count() > 0 ) return Final();
  Solve( new TDerivative);
  if( m_SolvIndexes.count() > 0 ) return Final();
  Solve( new TDefIntegrate);
  if( m_SolvIndexes.count() > 0 ) return Final();
  Solve( new TIndefIntegr);
  if( m_SolvIndexes.count() > 0 ) return Final();
  Solve( new TLimitCreator );
  Final();
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
        if(Ref.left(9) != "GET /Task") throw ErrParser( "Error; Bad task", ParserErr::peNewErr );
        int iSpace = Ref.indexOf(" ", 9);
        if(iSpace == -1) throw ErrParser( "Error; Bad task", ParserErr::peNewErr );
        sTskId = Ref.mid(10, iSpace - 10);
        int iTskId = sTskId.toInt();
        if(iTskId <= 0) throw ErrParser( "Error; Bad task", ParserErr::peNewErr );
        DB.setDatabaseName( "pictures" );
        DB.setUserName( "root" );
        DB.setHostName( "localhost" );
        DB.setPassword( s_PWD );
        if( !DB.open() )
          throw ErrParser( "Error; Can't open database: pictures", ParserErr::peNewErr );
        QSqlQuery Query( DB );
        QString Q( "Select Type, Formula, Name From pictures Where Id=" + sTskId );
        Query.exec( Q );
        if(Query.size() <= 0) throw ErrParser( "Error; Bad Query", ParserErr::peNewErr );
        Query.next();
        if(Query.value( 0 ).toByteArray() != "Task") throw ErrParser( "Error; Bad data base task record", ParserErr::peNewErr );
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
        if(iSlash == -1) throw ErrParser( "Error; Bad list answers", ParserErr::peNewErr );
        int iPrevSlash = Ref.lastIndexOf('/', iSlash - 1);
        if(iPrevSlash == -1) throw ErrParser( "Error; Bad list answers", ParserErr::peNewErr );
        QByteArray P, PP(Parms[1]);
        for(int i = PP.length() - 1; i >= 0; P += PP[i--] );
        Parms[prmPassword] = P;
        Parms[prmPathFile] = Parms[2] + Ref;
        QFile F(Parms[prmPathFile]);
        if( !F.open(QIODevice::ReadOnly) ) throw ErrParser( "Error; Can't open task file", ParserErr::peNewErr );
        Task = F.readAll();
        F.close();
        int iTask = Task.indexOf("TASK");
        if(iTask == -1) throw ErrParser( "Error; Bad task file", ParserErr::peNewErr );
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
        QByteArrayList CalcParms = Parms[0].split('#');
        if(CalcParms[0] =="SearchSolve")
          {
          TStr::sm_Server = true;
          SearchSolve(Parms[1]);
          TStr::sm_Server = false;
          QByteArray Result;
          int N = m_SolvIndexes.count();
          if( N == 0 )
            Result = "0";
          else
            for(int i = 0; i < N; i++)
              {
              QByteArray Code = QByteArray::number(m_SolvIndexes[i]);
              if(i == 0)
                Result = Code;
              else
                Result += '#' + Code;
              }
          m_pSocket->write(Result + "\n\n");
          m_pSocket->flush();
          return;
          }
        if(CalcParms[0] == "Plot")
          {
          double X_start(CalcParms[1].toDouble()), X_end(CalcParms[2].toDouble()), X_step;
          int NumberX = X_end - X_start;
          X_step = NumberX / 250.0;
          X_end += 0.1 * X_step;
          QByteArray Formula = Parms[1];
          int PosEq = Formula.indexOf('=');
          if(PosEq != -1)
            Formula = Formula.mid(PosEq + 1);
          MathExpr Expr = MathExpr( Parser::StrToExpr( Formula));
          if(s_GlobalInvalid || Expr.IsEmpty()) throw ErrParser( "Bad formula: " + Formula, ParserErr::peNewErr );
          QByteArray Result;
          double MaxY, MinY = 0;
          bool bFirstValue = true;
          double OldAccuracy = TExpr::sm_Accuracy;
          TExpr::sm_Accuracy = 0.001;
          for( double X = X_start; X <= X_end; X += X_step)
            {
            MathExpr Value;
            try
              {
              Value = Expr.Substitute("x", Constant(X) ).SimplifyFull();
              }
            catch( ErrParser& ErrMsg )
              {
              }
            Result += QByteArray::number(X) + ',';
            if( !(IsType( TConstant, Value )) || s_GlobalInvalid && s_LastError=="INFVAL")
              Result += "1.38162e-31;";
            else
              {
              Result += Value.WriteE() + ";";
              if(bFirstValue)
                {
                bFirstValue = false;
                Value.Constan(MaxY);
                if(MaxY < 0) MinY = MaxY;
                }
              else
                {
                double Y;
                Value.Constan(Y);
                if( Y > MaxY)
                  MaxY = Y;
                else
                  if(Y < MinY) MinY = Y;
                }
              }
            }
          TExpr::sm_Accuracy = OldAccuracy;
          Result += QByteArray::number(X_start,'g', 2) + "," + QByteArray::number(X_end, 'g', 2) +
              ";" + QByteArray::number(MinY, 'g', 2) + "," + QByteArray::number(MaxY, 'g', 2);
//          QByteArrayList Li = Result.split(';');
          m_pSocket->write(Result + "\n\n");
          m_pSocket->flush();
          return;
          }
        if(CalcParms[0] == "Calc")
          {
          s_CalcOnly = false;
          Solvers ESolv = (Solvers) CalcParms[1].toInt();
          double OldPrecision = TExpr::sm_Accuracy;
          TExpr::sm_Accuracy = CalcParms[2].toDouble();
          if(CalcParms[3] == "Degree")
            TExpr::sm_TrigonomSystem = TExpr::tsDeg;
          else
            TExpr::sm_TrigonomSystem = TExpr::tsRad;
          Solver *pSolv = nullptr;
          switch(ESolv)
            {
            case ESolvReToMult:
              pSolv = new TSolvReToMult;
              break;
            case ESolvSubSqr:
              pSolv = new TSolvSubSqr;
              break;
            case ESolvExpand:
              pSolv = new TSolvExpand;
              break;
            case ESolvSqrSubSum:
              pSolv = new TSolvSqrSubSum;
              break;
            case ESolvSumCub:
              pSolv = new TSolvSumCub;
              break;
            case ETrinom:
              pSolv = new TTrinom;
              break;
            case ESolvCubSubSum:
              pSolv = new TSolvCubSubSum;
              break;
            case ESolvSubCub:
              pSolv = new TSolvSubCub;
              break;
            case EAlg1Calculator:
              pSolv = new Alg1Calculator;
              break;
            case ELg:
              pSolv = new TLg;
              break;
            case ELog1Eq:
              pSolv = new Log1Eq;
              break;
            case ESysInEq:
              pSolv = new SysInEq;
              break;
            case ERatInEq:
              pSolv = new RatInEq;
              break;
            case EExpEq:
              pSolv = new ExpEq;
              break;
            case EMakeSubstitution:
              pSolv = new MakeSubstitution;
              break;
            case ESolveLinear:
              pSolv = new SolveLinear;
              break;
            case EMakeExchange:
              pSolv = new MakeExchange;
              break;
            case ESolvDetLinEqu:
              pSolv = new TSolvDetLinEqu;
              break;
            case ESolvQuaEqu:
              pSolv = new TSolvQuaEqu;
              break;
            case ESolvDetQuaEqu:
              pSolv = new TSolvDetQuaEqu;
              break;
            case ESolvDisQuaEqu:
              pSolv = new TSolvDisQuaEqu;
              break;
            case ESolvDetVieEqu:
              pSolv = new TSolvDetVieEqu;
              break;
            case ESolvCalcDetBiQuEqu:
              pSolv = new TSolvCalcDetBiQuEqu;
              break;
            case EAlgFrEquat:
              pSolv = new TSolvFractRatEq;
              break;
            case ESqLogEq:
              pSolv = new TSolvCalcIrratEq;
              break;
            case ESolvCalcPolinomEqu:
              pSolv = new TSolvCalcPolinomEqu;
              break;
            case ESolvCalcSimpleTrigoEq:
              pSolv = new TSolvCalcSimpleTrigoEq;
              break;
            case ESinCosEq:
              pSolv = new TSolvCalcSinCosEqu;
              break;
            case ESolvCalcTrigoEqu:
              pSolv = new TSolvCalcTrigoEqu;
              break;
            case ESolvCalcHomogenTrigoEqu:
              pSolv = new TSolvCalcHomogenTrigoEqu;
              break;
            case ESolvCalcEquation:
              pSolv = new TSolvCalcEquation;
              break;
            case ESin:
              pSolv = new TSin;
              break;
            case ECos:
              pSolv = new TCos;
              break;
            case ETan:
              pSolv = new TTan;
              break;
            case ECotan:
              pSolv = new TCotan;
              break;
            case ELn:
              pSolv = new TLn;
              break;
            case EDegRad:
              pSolv = new TDegRad;
              break;
            case ERadDeg:
              pSolv = new TRadDeg;
              break;
            case ESciCalc:
              pSolv = new TSciCalc;
              break;
            case EDeriv:
              pSolv = new TDerivative;
              break;
            case EIndefInt:
              pSolv = new TIndefIntegr;
              break;
            case EDefInt:
              pSolv = new TDefIntegrate;
              break;
            case EDerivFun:
              pSolv = new TDiff;
              break;
            case EEigenVals:
              pSolv = new TEigen;
              break;
            case EDeterminant:
              pSolv = new TDeterminant;
              break;
            case ETranspose:
              pSolv = new TTransposer;
              break;
            case EMatrixInv:
              pSolv = new TInverter;
              break;
            case EAngle2:
              pSolv = new TAngle2;
              break;
            case EAngle3:
              pSolv = new TAngle3;
              break;
            case ETan2:
              pSolv = new TTan2;
              break;
            case EAlpha2:
              pSolv = new TAlpha2;
              break;
            case EPriv:
              pSolv = new TPriv;
              break;
            case ETrigoOfSumma:
              pSolv = new TTrigoOfSumma;
              break;
            case ESumm_Mult:
              pSolv = new TSumm_Mult;
              break;
            case ETrigo_Summ:
              pSolv = new TTrigo_Summ;
              break;
            case EPermutations:
              pSolv = new Permutations;
              break;
            case EBinomCoeff:
              pSolv = new BinomCoeff;
              break;
            case EAccomodations:
              pSolv = new Accomodations;
              break;
            case EStatistics:
              pSolv = new Statistics;
              break;
            case ECorrelation:
              pSolv = new Correlation;
              break;
            case ELineProg:
              pSolv = new SysInEqXY;
              break;
            case EAlgToTrigo:
              pSolv = new AlgToTrigo;
              break;
            case EComplexOper:
              pSolv = new ComplexOper;
              break;
            case ELimit:
              pSolv = new TLimitCreator;
            break;
            }
          /*
                          EAlg2InEqXYGrph,
                          ESqLogEq??,
                          ESinEq?, ETanEq,  ECosEq, ECtnEq, ETrigoBiQudrEq, EAlgFrEq,
                          EPi,
                          ELimit, EDerivFun, EEigenVals, EDeterminant, ETranspose, EMatrixInv
EAngle2, EAngle3, ETan2,
  EAlpha2, EPriv, ETrigoOfSumma, ESumm_Mult, ETrigo_Summ, EPermutations, EBinomCoeff,
  EAccomodations, EStatistics, ECorrelation, ELineProg, EAlgToTrigo, EComplexOper
          */
//          TStr::sm_Server = true;
          QByteArray Result;
          auto Final = [&] ()
            {
            delete pSolv;
            s_CalcOnly = true;
            TExpr::sm_Accuracy = OldPrecision;
            Result = Result.replace("\\neq", "\\seq").replace("\\newline", "\\sewline");
            m_pSocket->write(Result.replace( '\\', "\\\\\\\\" ).replace( '\n', "\\\\n" ).replace( '\'', "\\'" ) + "\n\n" );
            m_pSocket->flush();
            };
          try {
              ExpStore::sm_pExpStore->Init_var();
              TSolutionChain::sm_SolutionChain.Clear();
              if(pSolv == nullptr ) throw ErrParser( "Error; Undefined Solv Type", ParserErr::peNewErr );
              if(Parms[1].isEmpty() ) throw ErrParser( "Error; Undefined Task", ParserErr::peNewErr );
              pSolv->SetExpression(Parms[1]);
              }
          catch (ErrParser& Err)
            {
            MathExpr EResult;
            if(pSolv != nullptr ) EResult = pSolv->Result();
            MathExpr StepsResult = TSolutionChain::sm_SolutionChain.GetChain();
            if( !StepsResult.IsEmpty() )
              Result = "Exp#" + StepsResult.SWrite() + '#' ;
            if( EResult.IsEmpty() )
              Result += "ED";
            else
              Result += "Exp#" + EResult.SWrite() ;
            Result += '#' + Err.Message();
            return Final();
            }
          MathExpr EResult = pSolv->Result();
//          QString Comment = TSolutionChain::sm_SolutionChain.GetLastComment();
          QString Comment;
          if( EResult.IsEmpty() )
            {
            Result = s_LastError.toUtf8();
            return Final();
            }
          bool V;
          if( EResult.Boolean_( V ) )
            {
            EResult = TSolutionChain::sm_SolutionChain.GetChain();
            if( EResult.IsEmpty() )
              Result = X_Str( "MNotSuitableExpr", "Not suitable expression!" ).toUtf8();
            else
              Result = "Exp&" + EResult.SWrite();
            if(!Comment.isEmpty())
              Result += '&' + Comment.toUtf8();
            return Final();
            }
          QString Comm;
          Result = "Exp&" + EResult.SWrite();
          if( pSolv->Success() )
            Comm = "";
//            Comm = s_XPStatus.GetCurrentMessage();
          else
            Comm = s_LastError;
          Result += "&" + Comm.toUtf8();
          return Final();
          }
        DataTask Task;
        QByteArray Formulas;
        for( int i = 0; i < Parms.count(); i++ )
          {
          if( i > 0 ) Formulas += "; ";
          Formulas += Parms[i];
          }
        qDebug() << "Start to compare" << m_SocketDescriptor << " Compared formulas" << Formulas << " Connections Count : "
          << ++s_pDataServer->m_ConnectionsCount;
        QString Text;
        MathExpr Expr;
        if(Parms[0].left(3) == "\"##")
          {
          Text = Parms[0];
          if(Text.right(5) == "0000\"")
            Text.remove(Text.length() - 5, 4);
          }
        else
          {
          TStr::sm_Server = true;
          Expr = MathExpr( Parser::StrToExpr( Parms[0] ) );
          TStr::sm_Server = false;
          }
        if( s_GlobalInvalid || ( Expr.IsEmpty() && Text.isEmpty() ) ) throw ErrParser( "Syntax Error in: " + Parms[0], ParserErr::peNewErr );
#ifdef DEBUG_TASK
        qDebug() << "Contents Expr" << (Expr.IsEmpty() ? Text : CastPtr( TExpr, Expr )->m_Contents);
#endif
        int iEqual = 1;
        for( ; iEqual < Parms.count(); iEqual++ )
          {
          QByteArray  Parm(Parms[iEqual]);
          if(Expr.IsEmpty())
            {
            if(Parm == Text) break;
            }
          else
            {
            TStr::sm_Server = true;
            QByteArrayList Formuls;
            if(Parm.left(6) == "Table(")
              {
              int iTable = 0;
              while(true)
                {
                int iNextTable = Parm.indexOf("#Table", iTable);
                if(iNextTable == -1)
                  {
                  Formuls.append(Parm.mid(iTable));
                  break;
                  }
                Formuls.append(Parm.mid(iTable, iNextTable - iTable));
                iTable = iNextTable + 1;
                }
              }
            else
              {
              QByteArray Formula;
              for( int iStartPack = Parm.indexOf("##"); iStartPack != -1; iStartPack = Parm.indexOf("##") )
                {
                Formula += Parm.left(iStartPack);
                try
                  {
                  QByteArray S(TStr::UnpackValue(Parm.mid(iStartPack)));
                  Formula += S;
                  Parm = Parm.mid(iStartPack + S.length() * 4 + 6);
                  }
                catch( ErrParser& ErrMsg )
                  {
                  Parm = Formula = "";
                  break;
                  }
                }
              Formula += Parm;
              Formuls = Formula.split( '#' );
              }
            int i = 0;
            for( ; i < Formuls.count(); i++ )
              {
              if( Formuls[i].isEmpty()) continue;
              Text = "Error";
              MathExpr ExprT = MathExpr( Parser::StrToExpr( Formuls[i] ) );
              if( s_GlobalInvalid || ExprT.IsEmpty() ) throw ErrParser( "Error; Bad task, formula: " + Formuls[i], ParserErr::peNewErr );
#ifdef DEBUG_TASK
              qDebug() << "Contents expT" << CastPtr( TExpr, ExprT )->m_Contents;
#endif
              double Precision = TExpr::sm_Precision;
              TExpr::sm_Precision = TExpr::sm_Accuracy;
              bool bResult;
              if(Task.ExactCompare())
                bResult = Expr.Eq( ExprT );
              else
                {
                bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
                try {
                bResult = Expr.Equal( ExprT );
                }  catch (ErrParser)
                  {
                  bResult = false;
                  }
                MathExpr::sm_NoReduceByCompare = false;
                MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
                }
              TExpr::sm_Precision = Precision;
              if( bResult ) break;
              }
            TStr::sm_Server = false;
            if( i < Formuls.count() ) break;
             }
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
      QByteArray Help(CC.GetContent( Task.m_pQuestion ));
//      QFile FHelp("C:\\Files\\Temp\\Help.htm");
//      FHelp.open( QIODevice::WriteOnly );
//      FHelp.write(Help);
//      FHelp.close();
      Query.addBindValue( Help );
//      Query.addBindValue( CC.GetContent( Task.m_pQuestion ) );
      if( !Query.exec() )
        throw ErrParser( "Error; Cant't add Content for QuestionWindow, topic: " + Parms[prmTopic], ParserErr::peNewErr );
      int iHelp = 0;
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
          QByteArray Help(CC.GetContent( Task.m_pMethodL ));
//          QFile FHelp("C:\\Files\\Temp\\Help" + QByteArray::number(++iHelp) + ".htm");
//          FHelp.open( QIODevice::WriteOnly );
//          FHelp.write(Help);
//          FHelp.close();
          Query.addBindValue( Help );
//          Query.addBindValue( CC.GetContent( Task.m_pMethodL ) );
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
            QByteArray Help(CC.GetContent( pStep->m_pMethodL, true ));
//            QFile FHelp("C:\\Files\\Temp\\HelPStep" + QByteArray::number(iStep) + ".htm");
//            FHelp.open( QIODevice::WriteOnly );
//            FHelp.write(Help);
//            FHelp.close();
            Query.addBindValue( Help );
//            Query.addBindValue( CC.GetContent( pStep->m_pMethodL, true ) );
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
//          if( Expr.HasStr() )
//            {
            TStr::sm_Server = true;
            EdFormula = Expr.SWrite();
            TStr::sm_Server = false;
//            }
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

         auto GetFalseComment = [&] (const PDescrMemb& pMemb)
           {
           QString Comment;
           for( PDescrMemb pM = pMemb; !pM.isNull(); pM = pM->m_pNext )
             {
             if( pM->m_Content.isEmpty() )
               {
               if(!Comment.isEmpty())
                 Comment += "<br>";
               continue;
               }
             if( pM->m_Kind == tXDexpress )
               {
               MathExpr AExpr = MathExpr( Parser::StrToExpr( pM->m_Content ) );
               if( s_GlobalInvalid || AExpr.IsEmpty() ) throw ErrParser( "Error; Bad task, formula: " + pM->m_Content, ParserErr::peNewErr );
               TStr::sm_Server = true;
               Comment += '#' + AExpr.SWrite().replace( '\\', "\\\\" ).replace( '\n', "\\n" ).replace( '\'', "\\'" );
               TStr::sm_Server = false;
               }
             else
               Comment += ToLang( pM->m_Content );
             }
           while( Comment.length() - Comment.lastIndexOf("<br>") == 4 )
             Comment = Comment.left(Comment.length() - 4);
           return Comment;
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
            QString FalseComm = GetFalseComment(pNext);
            Q = "Insert Into SubHint( idHintTask, Comment) Values(" + idHint + ",'" + FalseComm + "')";
            if( !Query.exec( Q ) )
              throw ErrParser( "Error; Cant't add SubHint, query: " + Q, ParserErr::peNewErr );
            AddFormula( "SubHint", pStep->m_pF1->m_pFirst, "idSubHint" );
            pNext = pStep->m_pF2->m_pFirst->m_pNext;
            FalseComm = GetFalseComment(pNext);
            Q = "Insert Into SubHint( idHintTask, Comment) Values(" + idHint + ",'" + FalseComm + "')";
            if( !Query.exec( Q ) )
              throw ErrParser( "Error; Cant't add SubHint, query: " + Q, ParserErr::peNewErr );
            AddFormula( "SubHint", pStep->m_pF2->m_pFirst, "idSubHint" );
            pNext = pStep->m_pF3->m_pFirst->m_pNext;
            FalseComm = GetFalseComment(pNext);
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

QByteArray ContentCreator::GetContent( PDescrList List, bool Center )
  {
//  qDebug() << "ContentCreator";
  SetContent( List );
//  qDebug() << "ContentCreator1";
  QByteArray  Content = toHtml().toUtf8(), Result;
  int iNoPict = Content.lastIndexOf("width=\"20%\"></td></tr>");
  if(iNoPict != -1)
    {
    Content.replace( iNoPict, 11, "width=\"0%\"", 10 );
    Content.replace( Content.indexOf("width=\"80%\""), 11, "width=\"100%\"", 12 );
    }
  Content.remove( 0, Content.indexOf( "<style" ) );
  Content.remove( Content.indexOf( "</head>" ), 7);
  Content.remove( Content.lastIndexOf( "</html>" ), 7 );
  Content.replace( Content.indexOf( "body" ), 4, "span", 4 );
  Content.replace( Content.lastIndexOf( "body" ), 4, "span", 4 );
  Content.insert( Content.indexOf( '>', Content.indexOf( "<table" ) + 7 ), " width=\"100%\"" );
  Content.insert( Content.indexOf( "<td", Content.indexOf( "<td" ) + 10 ) + 3, " style=\"vertical-align:top\" " );
  Content.insert( Content.indexOf( "<td" ) + 3, " nowrap style=\"vertical-align:top\" " );
  if( Center )
    {
    int iMg = Content.indexOf("<img");
    if( iMg != -1 && Content.indexOf("80%") > iMg )
    Content.insert( Content.lastIndexOf( "<p ", iMg ) + 3, " align='center' " );
    }

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
