#include "taskfilemanager.h"
#include <qdebug.h>
#include "Globa.h"
#include "../Mathematics/ExpObjOp.h"

TaskFileManager::TaskFileManager()
{
}

TaskFileManager::~TaskFileManager()
{
}

bool BaseTask::sm_NewHebFormat = false;
bool BaseTask::sm_GlobalShowUnarMinus = true;
bool BaseTask::sm_GlobalShowMinusByAddition = false;
bool BaseTask::sm_GlobalShowMultSign = false;
bool BaseTask::sm_GlobalShowRad = true;
int BaseTask::sm_LastCreatedTrack = 0;
EditSets* BaseTask::sm_pEditSets = nullptr;
QString RichTextDocument::sm_TempPath;
int RichTextDocument::sm_NumTmp = 0;
QHash<TLanguages, QString > BaseTask::sm_FileExtensions =
{ { lngHebrew, ".heb" }, { lngEnglish, ".tsk" }, { lngRussian, ".tru" }, { lngBulgarian, ".tbg" }, { lngAll, ".txt"} };

bool GetString( QByteArray& L, QByteArray& LO )
  {
  if(L.indexOf("ENDLEFT") == 0)
    {
    LO = "ENDLEFT";
    return true;
    }
  LO = "";
  int a = L.indexOf( '\'' );
  if( a == -1 ) return false;
  if( L.indexOf( "EXPR" ) >= 0 && a > L.indexOf( "EXPR" ) ) return false;
  if( L.indexOf( "PICT" ) >= 0 && a > L.indexOf( "PICT" ) ) return false;
  if( L.indexOf( "RAND" ) >= 0 && a > L.indexOf( "RAND" ) ) return false;
  if( a == L.length() - 1 )
    s_XPStatus.SetMessage( "Not found closing '" + ( " in string:" + L ) );
  LO = L.mid( a + 1 );
  int b = LO.indexOf( '\'' );
  if( b == -1 )
    {
    s_XPStatus.SetMessage( "Not found closing '" + ( " in string:" + L ) );
    b = 0;
    }
  if( LO.length() > b )
    L = LO.mid( b + 1 );
  LO = LO.left( b );
  return true;
  }

bool XPkeyword( const QByteArray& S )
  {
  return S.indexOf( "TASK" ) == 0 || S.indexOf( ";" ) == 0 || S.indexOf( "QUESTION" ) == 0 ||
    S.indexOf( "VALID" ) == 0 || S.indexOf( "DESCRIPTION_TRACKS" ) == 0 ||
    S.indexOf( "TRACK" ) == 0 || S.indexOf( "PROMPT" ) == 0 || S.indexOf( "METH" ) == 0 ||
    S.indexOf( "INIT" ) == 0 || S.indexOf( "CALC" ) == 0 || S.indexOf( "STEP" ) == 0 ||
    S.indexOf( "RESULT" ) == 0 || S.indexOf( "TEMPLATE" ) == 0 || S.indexOf( "FALSE" ) == 0 ||
    S.indexOf( "SUB_TASK" ) >= 0 || S.indexOf( "COMM" ) == 0;
  }

QByteArray TaskFile::ReadLine()
  {
  m_iLine++;
  return readLine();
  }

void TaskFile::Reset( int iLine )
  {
  reset();
  while( m_iLine != iLine ) ReadLine();
  }

bool TaskFile::Step_exist( const QByteArray& StepID )
  {
  reset();
  SkipLinesUntilSelectedTrack( "STEP" );
  while( !atEnd() )
    {
    QByteArray S = readLine();
    if( S.indexOf( "TRACK" ) == 0 ) return false;
    if( S.indexOf( "STEP" + StepID ) == 0 ) return true;
    }
  return false;
  }

void TaskFile::SkipLinesUntilSelectedTrack( const QByteArray& LKeyWord )
  {
  if( !m_pTask->m_pTrack->m_MultyTrack ) return;
  auto Skip = [=] ()
    {
    QByteArray line;
    QByteArray sTrackKeyWord = "TRACK" + QByteArray::number( m_pTask->m_pTrack->m_SelectedTrack );
    do
      line = readLine();
    while( !atEnd() && line.indexOf( sTrackKeyWord ) != 0 );
    };

  if( LKeyWord.indexOf( "METH" ) == 0 || LKeyWord.indexOf( "STEP" ) == 0 ||
    LKeyWord.indexOf( "RESULT" ) == 0 || LKeyWord.indexOf( "FALSE" ) == 0 ||
    LKeyWord.indexOf( "HIDEUNARMINUS" ) == 0 || LKeyWord.indexOf( "SHOWUNARMINUS" ) == 0 ||
    LKeyWord.indexOf( "HIDEMULTSIGN" ) == 0 || LKeyWord.indexOf( "SHOWMULTSIGN" ) == 0 ||
    LKeyWord.indexOf("EXACTCOMPARE") == 0 || LKeyWord.indexOf( "COMM" ) == 0 ) Skip();
  if( LKeyWord.indexOf( "CALC" ) == 0 && m_pTask->m_pTrack->m_TrackSelected ) Skip();
  }

bool TaskFile::LoadNameFromTaskFile( const QByteArray& LKeyWord, QByteArray& Name )
  {
  reset();
  SkipLinesUntilSelectedTrack( LKeyWord );
  QByteArray line;
  do
    {
    line = readLine();
    if( line.indexOf( LKeyWord ) == 0 )
      {
      if( !GetString( line, Name ) )
        {
        Name.clear();
        return false;
        }
      return true;
      }
    } while( !atEnd() && line.indexOf( "TRACK" ) != 0 );
    return false;
  }

  bool TaskFile::LoadRValueFromTaskFile( const QByteArray& LKeyWord, double& Value )
    {
    reset();
    SkipLinesUntilSelectedTrack( LKeyWord );
    do
      {
      QByteArray line( readLine() );
      if( line.indexOf( LKeyWord ) == 0 )
        {
        QString V = line.mid( LKeyWord.length() );
        Value = V.toDouble();
        return true;
        }
      } while( !atEnd() );
      return false;
    }

  bool TaskFile::LoadBooleanFromTaskFile( const QByteArray& LKeyWord, bool &Value )
    {
    reset();
    SkipLinesUntilSelectedTrack( LKeyWord );
    do
      {
      QByteArray line( readLine() );
      //    if( line.indexOf( LKeyWord ) == 0 )
      if( line.trimmed() == LKeyWord )
        {
        Value = false;
        return true;
        }
      } while( !atEnd() );
      return false;
    }

  void TaskFile::Save( QByteStream &Stream, const QString& FName )
    {
    if( isOpen() ) close();
    setFileName( FName );
    open( QIODevice::ReadWrite );
    resize( 0 );
    Stream.reset();
    while( !Stream.AtEnd() )
      {
      QByteArray Line;
      Stream >> Line;
      write( Line + "\r\n" );
      }
    flush();
    }

  void TXDescrList::ProcessMacro( QByteArray& S, char kwc )
    {
    for( int i = 0; i < S.length() - 1; i++ )
      {
      if( S[i] != kwc ) continue;
      S = S.left( i ) + S.mid( i + 1 );
      int j = i;
      for( ; j < S.length() && _inside_ident( S[j] ); j++ );
      MathExpr ExpT;
      if( ExpStore::sm_pExpStore->Known_var_expr( S.mid( i, j - i ), ExpT ) )
        {
        double R;
        if( ExpT.Constan( R ) )
          {
          S = S.left( i ) + ExpT.WriteE() + S.mid( j );
          continue;
          }
        }
      S = S.left( i ) + ( char ) msMetaSign + S.mid( i );
      }
    }

  MathExpr CalcList::Random_gen( const QByteArray& SArg )
    {
    QByteArray S = SArg.simplified();
    TXDescrList::ProcessMacro( S, msMetaSign );
    QByteArray Var_name;
    int AssPos = S.indexOf( '=' );
    if( AssPos != -1 )
      Var_name = S.left( AssPos ).trimmed();
    int OpBrackPos = S.indexOf( '[' );
    if( OpBrackPos == -1 ) return nullptr;
    S = S.mid( OpBrackPos + 1 );
    int CloseBrackPos = S.indexOf( ']' );
    if( CloseBrackPos == -1 ) return nullptr;
    double Delta, Left_b, Right_b;
    if( CloseBrackPos < S.length() - 1 )
      {
      bool Ok;
      Delta = S.mid( CloseBrackPos + 1 ).trimmed().toDouble( &Ok );
      if( !Ok ) return nullptr;
      S = S.left( CloseBrackPos );
      int PointsPos = S.indexOf( ".." );
      if( PointsPos == -1 ) return nullptr;
      Left_b = S.left( PointsPos ).toDouble( &Ok );
      if( !Ok ) return nullptr;
      QByteArray Expr = S.mid( PointsPos + 2 );
      if( Expr[0] == '@' )
        Right_b = ExpStore::sm_pExpStore->GetValue( Expr );
      else
        {
        Right_b = Expr.toDouble( &Ok );
        if( !Ok ) return nullptr;
        }
    double Arg = abs( Right_b - Left_b ) / Delta;
    long long R = Round(Arg);
    int iRandom = m_pTask->Random(R);
    MathExpr Result = new TConstant( Left_b + iRandom * Delta);
    if( !Var_name.isEmpty() ) ExpStore::sm_pExpStore->Store_var( Var_name, Result );
    return Result;
    }
  QByteArrayList Values = S.left( CloseBrackPos ).split( ';' );
  if( Values.count() == 0 ) return nullptr;
  int iRandom = m_pTask->Random( Values.count() - 1 );
  MathExpr exi = Parser::StrToExpr( Values[iRandom] );
  if( s_GlobalInvalid )
    {
    s_XPStatus.SetMessage( "Task corrupted : RAND: " + S );
    exi = new TConstant( 0 );
    }
  if( !Var_name.isEmpty() ) ExpStore::sm_pExpStore->Store_var( Var_name, exi );
  return exi;
  }

CalcPair::CalcPair( const QByteArray& Pair, CalcList *pOwner, XDescr_type Kind ) : m_pOwner( pOwner ), m_Kind( Kind )
  {
  int EqPos = Pair.indexOf( '=' );
  if( EqPos == -1 )
    {
    m_Expression = Pair.trimmed();
    return;
    }
  m_Variable = Pair.left( EqPos ).trimmed();
  m_Expression = Pair.mid( EqPos + 1 ).trimmed();
  }

void CalcPair::Calculate()
  {
  if( !CalcExpress() )
    m_pOwner->Random_gen( m_Variable + '=' + m_Expression );
  }

bool CalcPair::CalcExpress()
  {
  QByteArray Expression = m_Expression;
  MathExpr RandExpr;
  if( m_Kind == tXDrandom )
    {
    int OpBrackPos = Expression.indexOf( '[' );
    if( OpBrackPos <= 0) return false;
    RandExpr = m_pOwner->Random_gen(Expression.mid(OpBrackPos));
    Expression = Expression.left(OpBrackPos) + RandExpr.WriteE();
    }
  Expression.replace( "@S(", "@Simplify(");
  Expression.replace( "@PS(", "@PolinomSmplf(" );
  Expression.replace( "@SE(", "@SimplifyEquation(" );
  Expression.replace( "@SF(", "@SimplifyFull(" );
  s_iDogOption = !s_TaskEditorOn;
  MathExpr VarValue(Parser::PureStrToExpr( Expression ));
  if(!VarValue.IsEmpty()) VarValue = VarValue.Perform();
  if(!RandExpr.IsEmpty()) VarValue = VarValue.SimplifyFull();
  s_iDogOption = 0;
  if( s_GlobalInvalid || VarValue.IsEmpty() && !m_Variable.isEmpty() ) throw ErrParser( "Task corrupted " + m_Expression, ParserErr::peNewErr );
  if( m_Variable.isEmpty() ) return true;
  ExpStore::sm_pExpStore->Store_var( m_Variable, VarValue );
  return true;
  }

void CalcPair::Save( QByteStream& Stream )
  {
  if( m_Kind == tXDrandom )
    Stream << "RAND(";
  else 
    Stream << "EXPR(";
  if( !m_Variable.isEmpty() )
    Stream << m_Variable << "=";
  Stream << m_Expression << ")\r\n";
  }

void CalcList::LoadFromTaskFile( const QByteArray& LKeyWord )
  {
  clear();
  TXDescrList DList(*m_pTask);
  DList.LoadFromTaskFile( LKeyWord );
  for( PDescrMemb index = DList.m_pFirst; !index.isNull(); index = index->m_pNext )
    if( index->m_Kind == tXDexpress || index->m_Kind == tXDrandom )
      push_back( CalcPair( index->m_Content, this, index->m_Kind ) );
  Calculate();
  }

void CalcList::LoadFromQuestionVariables(const QByteArrayList& Variables)
  {
  clear();
  TXDescrList DList(*m_pTask);
  DList.LoadFromQuestionVariables( Variables );
  for( PDescrMemb index = DList.m_pFirst; !index.isNull(); index = index->m_pNext )
    push_back( CalcPair( index->m_Content, this, index->m_Kind ) );
  Calculate();
  }

void CalcList::Calculate()
  {
  try
    {
    ExpStore::sm_pExpStore->Clear();
    for( auto pPair = begin(); pPair != end(); pPair++ )
      pPair->Calculate();
    }
  catch( ErrParser& ErrMsg )
    {
    QMessageBox::critical( nullptr, "Error", ErrMsg.Message() );
    }
  }

void CalcList::Save( QByteStream& Stream)
  {
  Stream << "CALC\r\n";
  for( auto pPair = begin(); pPair != end(); pPair++ )
    pPair->Save(Stream);
  }

bool TXDescrList::ExtractValue( QByteArray& L, QByteArray& LO, int Start )
  {
  auto Replace_Smplf_Name = [&] ()
    {
    LO = LO.replace( "@S(", "@Simplify(" );
    LO = LO.replace( "@PS(", "@PolinomSmplf(" );
    LO = LO.replace( "@SE(", "@SimplifyEquation(" );
    LO = LO.replace( "@SF(", "@SimplifyFull(" );
    };

  int Opened = 0;
  for( int Ind = Start; Ind < L.length(); Ind++ )
    {
    if( Opened == 0 )
      {
      Start++;
      if( L[Ind] != '(' && L[Ind] != ' ' ) break;
      }
    if( L[Ind] == '(' || L[Ind] == '[' )
      {
      Opened++;
      continue;
      }
    if( L[Ind] == ')' || L[Ind] == ']' )
      {
      if( --Opened == 0 )
        {
        LO = L.mid( Start, Ind - Start );
        Replace_Smplf_Name();
        L = L.mid( Ind + 1 );
        return true;
        }
      }
    }
  return false;
  }

bool TXDescrList::GetPict( QByteArray& L, QByteArray& LO )
  {
  LO.clear();
  int PictPos = L.indexOf( "PICT" );
  if( PictPos == -1 ) return false;
  if( L.indexOf( "EXPR" ) > -1 && PictPos > L.indexOf( "EXPR" ) ) return false;
  if( L.indexOf( '\'' ) > -1 && PictPos > L.indexOf( '\'' ) ) return false;
  if( L.indexOf( "RAND" ) > -1 && PictPos > L.indexOf( "RAND" ) ) return false;
  if( ExtractValue( L, LO, PictPos + sizeof( "PICT" ) - 1 ) )
    {
    if(!s_TaskEditorOn || LO[0] != '@') return true;
    L = "EXPR(Picture(" + LO + "))" + L;
    LO = "";
    }
  return false;
  }

QByteArray TXDescrList::GetText()
  {
  QByteArray *TextPtr = GetTextPtr();
  if( TextPtr == nullptr ) return "";
  return *TextPtr;
  }

QByteArray* TXDescrList::GetTextPtr()
  {
  for( PDescrMemb pMemb = m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    if( pMemb->m_Kind == tXDtext && !pMemb->m_Content.isEmpty() ) return &pMemb->m_Content;
  return nullptr;
  }

void TXDescrList::Save( QByteStream& Stream, PDescrList pList)
  {
  if( pList->m_pDocument == nullptr )
    {
    if( pList->GetText().isEmpty() ) return;
    RichTextDocument RT;
    RT.SetContent( pList );
    RT.Save( Stream );
    pList->m_pDocument = nullptr;
    return;
    }
  pList->m_pDocument->Save( Stream );
  }

int TXDescrList::GetExpressCount()
  {
  int Count = 0;
  for( PDescrMemb pMemb = m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    if( pMemb->m_Kind == tXDexpress && !pMemb->m_Content.isEmpty() ) Count++;
  return Count;
  }

bool TXDescrList::GetExpr( QByteArray& L, QByteArray& LO )
  {
  LO.clear();
  int ExprPos = L.indexOf( "EXPR" );
  if( ExprPos == -1 ) return false;
  if( L.indexOf( "PICT" ) > -1 && ExprPos > L.indexOf( "PICT" ) ) return false;
  if( L.indexOf( '\'' ) > -1 && ExprPos > L.indexOf( '\'' ) ) return false;
  if( L.indexOf( "RAND" ) > -1 && ExprPos > L.indexOf( "RAND" ) ) return false;
  bool Result = ExtractValue( L, LO, ExprPos + sizeof( "EXPR" ) - 1 );
  if( !Result ) return false;
  int iSemicolon = LO.indexOf( ';' );
  if( iSemicolon == -1 ) return true;
  int iBrack = 0;
  for( int i = 0; i < iSemicolon; i++ )
    if( LO[i] == '(' ) 
      iBrack++;
    else
      if( LO[i] == ')' )
        iBrack--;
  if( iBrack != 0 ) return true;
  QByteArrayList Equations( LO.split( ';' ) );
  for( int i = 0; i < Equations.count(); i++ )
    if( Equations[i].indexOf( '=' ) == -1 ) return true;
  LO = "syst(" + LO + ')';
  return true;
  }

bool CalcList::GetRand( QByteArray& L, QByteArray& LO )
  {
  LO.clear();
  int RandPos = L.indexOf( "RAND" );
  if( RandPos == -1 ) return false;
  if( L.indexOf( "PICT" ) > -1 && RandPos > L.indexOf( "PICT" ) ) return false;
  if( L.indexOf( '\'' ) > -1 && RandPos > L.indexOf( '\'' ) ) return false;
  if( L.indexOf( "EXPR" ) > -1 && RandPos > L.indexOf( "EXPR" ) ) return false;
  if( !TXDescrList::ExtractValue( L, LO, RandPos + sizeof( "RAND" ) - 1 ) ) return false;
  return true;
  }

CalcList& CalcList::operator =( const CalcList& Arg )
  {
  clear();
  append( Arg );
  m_pTask = Arg.m_pTask;
  return *this;
  }


TXDescrMemb::TXDescrMemb( XDescr_type K, const QByteArray& C, const PDescrMemb& P, const PDescrMemb& N ) :
m_Kind( K ), m_Content( C ), m_pPrev( P ), m_pNext( N ), m_CompareType( AsExpression )
  {
  if( K == tXDpicture && C.indexOf( '@' ) != -1 )
    {
    try
      {
      MathExpr exi( Parser::StrToExpr( C ) );
      exi = exi.Perform();
      m_Content = exi.WriteE();
      if( m_Content.length() < 2 || m_Content.indexOf( '@' ) != -1 ) m_Content = C;
      }
    catch( ErrParser )
      {
      m_Content = C;
      }
    }

  char *CompareAs[] = { "AsExpression", "AsConstraint" };

  QByteArray CC( C );
  if( K == tXDexpress )
    {
    if( C.indexOf( "COMPARE(" ) != -1 )
      {
      for( int i = 0; i < 2; i++ )
        if( C.indexOf( CompareAs[i] ) > -1 )
          {
          m_CompareType = TCompareType( i );
          break;
          }
      CC = C.mid( C.indexOf( ')' ) + 1 );
      }
    m_Content = CC;
    }
  }

void TXDescrList::Add( XDescr_type Type_arg, const QByteArray& S )
  {
  if( Type_arg == tXDtext && S == "ENDLEFT" )
    m_bHasEndLeft = true;

  PDescrMemb Appending( new TXDescrMemb( Type_arg, S, m_pLast, PDescrMemb() ) );
  if( !m_pLast.isNull() )
    m_pLast->m_pNext = Appending;
  if( m_pFirst.isNull() )
    m_pFirst = Appending;
  m_pLast = Appending;
  }

void TXDescrList::Clear()
  {
  PDescrMemb pFirst;
  pFirst.swap( m_pFirst );
  for( PDescrMemb pNext; !pFirst.isNull(); pNext.swap( pFirst ) )
    {
    pFirst->m_pPrev.clear();
    pNext.swap( pFirst->m_pNext );
    pFirst.clear();
    }
  m_pLast.clear();
  m_pDocument = nullptr;
  }

void TXDescrList::Delete( PDescrMemb M )
  {
  if( M.isNull() ) return;
  if( M.data() == m_pFirst.data() )
    m_pFirst = M->m_pNext;
  if( M.data() == m_pLast.data() )
    m_pLast = M->m_pPrev;

  PDescrMemb Prev( M->m_pPrev );
  PDescrMemb Next( M->m_pNext );
  if( !Prev.isNull() )
    Prev->m_pNext = Next;
  if( !Next.isNull() )
    Next->m_pPrev = Prev;
  }

void TXDescrList::Update( QByteArray& S )
  {
  while( !S.isEmpty() && S[0] == ' ' )
    S = S.mid( 1 );

  while( !S.isEmpty() )
    {
    QByteArray PrevS( S ), LO;
    if( GetString( S, LO ) )
      {
      Add( tXDtext, LO );
      if( !LO.isEmpty() && m_QuestionCount >= 0 && LO[0] >= '0' && LO[0] <= '9' )
        {
        int iPos = LO.indexOf( '.' );
        if( iPos > 0 )
          {
          int iNumber = LO.left( iPos ).toInt();
          m_QuestionCount++;
          if( !s_TaskEditorOn && m_QuestionCount != iNumber )
            throw ErrParser( "Task: " + m_Task.GetFile().fileName() + " Questions numbering is incorrect!", ParserErr::peNewErr );
          }
        }
      }
    if( GetPict( S, LO ) )
      Add( tXDpicture, LO );
    if( GetExpr( S, LO ) )
      Add( tXDexpress, LO );
    if( m_Task.m_pCalc->GetRand( S, LO ) )
      Add( tXDrandom, LO );

    while( !S.isEmpty() && S[0] == ' ' )
      S = S.mid( 1 );

    if( PrevS == S ) break;
    }
  Add( tXDnewline, "" );
  }

void TXDescrList::Assign( PDescrList& A )
  {
  Clear();
  if( A.isNull() ) return;
  for( PDescrMemb index = A->m_pFirst; !index.isNull(); index = index->m_pNext )
    Add( index->m_Kind, index->m_Content );
  }

void TXDescrList::LoadFromTaskFile( const QByteArray& LKeyWord )
  {
  QByteArray line;
  TaskFile& File = m_Task.GetFile();
  Randomize();
  File.reset();
  File.SkipLinesUntilSelectedTrack( LKeyWord );
  do
    {
    line = File.readLine();
    if( line.indexOf( LKeyWord ) == 0 )
      {
      Update( line );
      while( !File.atEnd() )
        {
        line = File.readLine();
        if( line.indexOf( "TRACK" ) == 0 ) break;
        if( !XPkeyword( line ) )
          Update( line );
        else
          break;
        }
      break;
      }
    } while( !File.atEnd() );
  }

void TXDescrList::LoadFromQuestionVariables(const QByteArrayList& Variables)
  {
  QByteArrayList VarNames;
  for( auto pInstr = Variables.begin(); pInstr != Variables.end(); pInstr++ )
    {
    int SK = pInstr->indexOf(':');
    if( SK == -1 )
      throw ErrParser( "Line: " + *pInstr + " Error in STACK File", ParserErr::peNewErr );
    QByteArray VarName = pInstr->left(SK).trimmed();
    VarNames.append(VarName);
    QByteArray Instr = pInstr->mid(SK + 1).trimmed();
    int iRand = Instr.indexOf("rand");
    if(iRand != -1)
      {
      int iVal = iRand + 5;
      QByteArray Val = Instr.mid(iVal, Instr.lastIndexOf(')') - iVal);
      QByteArray Rand = VarName + '=' + Instr.left(iRand) + "[0.." + Val + "]1";
      Add( tXDrandom, Rand );
      continue;
      }
    bool bMustReduce = true;
    QByteArray NewName;
    QByteArray Expression;
    auto TestName = [&]()
      {
      if(NewName.isEmpty()) return;
      if(VarNames.indexOf(NewName) == -1)
        {
        if(NewName == "ev")
          {
          Expression += "@S";
          Instr.remove(Instr.indexOf(",simp"), 5);
          bMustReduce = false;
          NewName.clear();
          return;
          }
        Expression += NewName;
        int fnum;
        if(bMustReduce)
          bMustReduce = NewName == "i" || IsFunctName(NewName, fnum);
        }
      else
        Expression += '@' + NewName;
      NewName.clear();
      };
    for( int i = 0; i < Instr.length(); i++)
      {
      uchar C = Instr[i];
      if(isalpha(C))
        NewName += C;
      else
        if(NewName.isEmpty())
          { if(C != ';') Expression += C; }
        else
          if(isdigit(C))
            NewName += C;
          else
            {
            TestName();
            if(C != ';') Expression += C;
            }
      }
    TestName();
    QByteArray Expr = VarName + "=";
    if(bMustReduce)
      Add( tXDexpress, Expr + "@S(" + Expression + ")" );
    else
      Add( tXDexpress, Expr + Expression );
    }
  }

void TXStepList::Clear()
  {
  PStepMemb pFirst;
  pFirst.swap( m_pFirst );
  for( PStepMemb pNext; !pFirst.isNull(); pNext.swap( pFirst ) )
    {
    pNext.swap( pFirst->m_pNext );
    pFirst.clear();
    }
  m_pLast.clear();
  m_Counter = 0;
  m_SummMarksTotal = 0;
  }

void TXStepList::SetGlobalSWhowRad( bool Value )
  {
  for( PStepMemb pMemb = m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    pMemb->SetGlobalSWhowRad( Value );
  }

void TXStepList::Add( const QByteArray& StepID, BaseTask& Task )
  {
  PStepMemb pAppending( new TXStepMemb( m_pLast, Task ) );
  pAppending->LoadFromTaskFile( StepID );
  if( !m_pLast.isNull() )
    m_pLast->m_pNext = pAppending;
  if( m_pFirst.isNull() )
    m_pFirst = pAppending;
  m_pLast = pAppending;
  m_SummMarksTotal += pAppending->m_Mark;
  m_Counter++;
  }

void TXStepList::AddNewStep( const QByteArray& StepName, BaseTask& Task, PStepMemb pNext )
  {
  auto AddContent = [&] ( PStepMemb pNewStep )
    {
    pNewStep->m_Name = StepName;
    pNewStep->m_pMethodL->Add( tXDtext, "" );
    pNewStep->m_pResultE->Add( tXDexpress, "\"New Answer\"" );
    pNewStep->m_pF1->Add( tXDtext, "" );
    pNewStep->m_pF2->Add( tXDtext, "" );
    pNewStep->m_pF3->Add( tXDtext, "" );
    m_Counter++;
    };
  if( pNext.isNull() )
    {
    PStepMemb pAppending( new TXStepMemb( m_pLast, Task ) );
    if( !m_pLast.isNull() )
      m_pLast->m_pNext = pAppending;
    if( m_pFirst.isNull() )
      m_pFirst = pAppending;
    m_pLast = pAppending;
    AddContent( pAppending );
    return;
    }
  PStepMemb pMemb = m_pFirst;
  for( ; !pMemb.isNull() && pMemb != pNext; pMemb = pMemb->m_pNext );
  if( pMemb.isNull() )
    {
    AddNewStep( StepName, Task );
    return;
    }
  PStepMemb pNewStep( new TXStepMemb( pMemb->m_pPrev, Task, pMemb ) );
  if( pMemb->m_pPrev.isNull() )
    m_pFirst = pNewStep;
  else
    pMemb->m_pPrev->m_pNext = pNewStep;
  pMemb->m_pPrev = pNewStep;
  AddContent( pNewStep );
  }

void TXStepList::DeleteStep( PStepMemb pPstep )
  {
  PStepMemb pMemb = m_pFirst;
  for( ; !pMemb.isNull() && pMemb != pPstep; pMemb = pMemb->m_pNext );
  if( pMemb.isNull() ) return;
  if( pMemb.data() == m_pFirst.data() )
    m_pFirst = pMemb->m_pNext;
  if( pMemb.data() == m_pLast.data() )
    m_pLast = pMemb->m_pPrev;
  PStepMemb Prev( pMemb->m_pPrev );
  PStepMemb Next( pMemb->m_pNext );
  if( !Prev.isNull() )
    Prev->m_pNext = Next;
  if( !Next.isNull() )
    Next->m_pPrev = Prev;
  m_Counter--;
  }

bool TXStepList::ResetMarks()
  {
  m_SummMarksTotal = 0;
  if(m_pFirst.isNull() || !m_pFirst->m_Task.m_MultiTask) return true;
  for( PStepMemb pMemb = m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    m_SummMarksTotal += pMemb->m_Mark;
  if(m_SummMarksTotal == 100 ) return true;
  double K = 100.0 / m_SummMarksTotal;
  m_SummMarksTotal = 0;
  for( PStepMemb pMemb = m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    {
    int Mark = qRound(pMemb->m_Mark * K);
    pMemb->ResetMark(Mark);
    m_SummMarksTotal += Mark;
    }
  int dK = 100 - m_SummMarksTotal;
  if(dK == 0 ) return false;
  m_SummMarksTotal += dK;
  int Step = dK > 0 ? 1 : -1;
  for( PStepMemb pMemb = m_pFirst; dK && !pMemb.isNull(); pMemb = pMemb->m_pNext, dK -= Step )
    pMemb->ResetMark( pMemb->m_Mark + Step);
  return false;
  }

void TXStepList::Save( QByteStream& Stream )
  {
  if( !ResetMarks() )
    throw ErrParser( X_Str( "Bad Summ Steps", "Sum of steps marks is not 100" ), ParserErr::peNewErr );
  int iId = 1;
  for( PStepMemb pMemb = m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext, iId++ )
    pMemb->Save( Stream, iId );
  }

TXStepMemb::TXStepMemb( PStepMemb Prev, BaseTask& Task, PStepMemb Next ) : TXPStep( Task ), m_pPrev( Prev ),
m_pNext( Next ), m_pRichText( nullptr ), m_pPromptText(nullptr)
  {
  }

void TXPStep::LoadFromTaskFile( const QByteArray& StepID )
  {
  TaskFile& File = m_Task.GetFile();
  File.LoadNameFromTaskFile( "STEP" + StepID, m_Name );
  int iMarkPos = m_Name.indexOf( "MARKtoTEST" );
  if( iMarkPos != -1 )
    {
    int iStart = m_Name.indexOf('(', iMarkPos + 10 ) + 1;
    int iEnd = m_Name.indexOf( ')', iStart );
    bool OK;
    QByteArray Mark =  m_Name.mid(iStart, iEnd - iStart );
    m_Mark = m_Name.mid(iStart, iEnd - iStart ).toInt(&OK);
    if( !OK )
      throw ErrParser( X_Str( "ErrorWhileReadingStep", "Error  while(reading step of task" ), ParserErr::peNewErr );
    iStart = min(m_Name.indexOf('('), iMarkPos);
    m_Name.remove( iStart, iEnd - iStart + 1 );
    }
  else
    {
    int iPercPos = m_Name.indexOf( '%' );
    if(iPercPos != -1)
      {
      int iStart = m_Name.lastIndexOf('(', iPercPos) + 1;
      if( iStart == 0)
        throw ErrParser( X_Str( "ErrorWhileReadingStep", "Error  while(reading step of task" ), ParserErr::peNewErr );
      bool OK;
      m_Mark = m_Name.mid(iStart, iPercPos - iStart ).toInt(&OK);
      if( !OK )
        throw ErrParser( X_Str( "ErrorWhileReadingStep", "Error  while(reading step of task" ), ParserErr::peNewErr );
      }
    }
  LoadSubTaskFromTaskFile( "STEP" + StepID );
  m_pMethodL->LoadFromTaskFile( "METH" + StepID );
  m_pResultE->LoadFromTaskFile( "RESULT" + StepID );
  if(m_pResultE->m_pFirst == nullptr)
    throw ErrParser( "Step: " + StepID + " Result true expression not exists!", ParserErr::peNewErr );
  m_pAnswerPrompt->LoadFromTaskFile("PROMPT" + StepID);
  m_pAnswerTemplate->LoadFromTaskFile( "TEMPLATE" + StepID );
  m_pF1->LoadFromTaskFile( "FALSE" + StepID + ".1" );
  if(m_pF1->m_pFirst == nullptr)
    throw ErrParser( "Step: " + StepID + " Result expression False1 not exists!", ParserErr::peNewErr );
  m_pFComm1->Assign(m_pF1);
  m_pFComm1->Delete(m_pFComm1->m_pFirst);
  m_pF2->LoadFromTaskFile( "FALSE" + StepID + ".2" );
  if(m_pF2->m_pFirst == nullptr)
    throw ErrParser( "Step: " + StepID + " Result expression False2 not exists!", ParserErr::peNewErr );
  m_pFComm2->Assign(m_pF2);
  m_pFComm2->Delete(m_pFComm2->m_pFirst);
  m_pF3->LoadFromTaskFile( "FALSE" + StepID + ".3" );
  if(m_pF3->m_pFirst == nullptr)
    throw ErrParser( "Step: " + StepID + " Result expression False3 not exists!", ParserErr::peNewErr );
  m_pFComm3->Assign(m_pF3);
  m_pFComm3->Delete(m_pFComm3->m_pFirst);
  m_pComm->LoadFromTaskFile( "COMM" + StepID );
  m_ShowParms.m_HideUnarMinus = File.LoadBooleanFromTaskFile( "HIDEUNARMINUS" + StepID, m_ShowUnarm );
  bool HideUnarm = !m_ShowUnarm;
  m_ShowParms.m_ShowUnarMinus = File.LoadBooleanFromTaskFile( "SHOWUNARMINUS" + StepID, HideUnarm );
  m_ShowUnarm = !HideUnarm;
  m_ShowParms.m_HideMinusByAddition = File.LoadBooleanFromTaskFile("HIDEMINUSBYADDITION" + StepID, m_ShowMinusByAdd);
  bool HideMinus = !m_ShowMinusByAdd;
  m_ShowParms.m_ShowMinusByAddition = File.LoadBooleanFromTaskFile("SHOWMINUSBYADDITION" + StepID, HideMinus);
  m_ShowMinusByAdd = !HideMinus;
  m_ShowParms.m_HideMultSign = File.LoadBooleanFromTaskFile( "HIDEMULTSIGN" + StepID, m_ShowMSign );
  bool HideMSign = !m_ShowMSign;
  m_ShowParms.m_ShowMultSign = File.LoadBooleanFromTaskFile( "SHOWMULTSIGN" + StepID, HideMSign );
  m_ShowMSign = !HideMSign;
  m_ShowParms.m_ShowDeg = File.LoadBooleanFromTaskFile( "ANGLEDEG" + StepID, m_ShowRad );
  bool ShowDeg = !m_ShowRad;
  m_ShowParms.m_ShowRad = File.LoadBooleanFromTaskFile( "ANGLERAD" + StepID, ShowDeg );
  m_ShowRad = !ShowDeg;
  m_ShowParms.m_OnExactCompare = File.LoadBooleanFromTaskFile("EXACTCOMPARE" + StepID, m_Task.m_ExactCompare);
  m_Task.m_ExactCompare = m_ShowParms.m_OnExactCompare;
  m_ShowParms.m_NoHint = File.LoadBooleanFromTaskFile("NOHINT" + StepID, m_ShowParms.m_NoHint);
  File.LoadRValueFromTaskFile( "EDITORHEIGHT" + StepID + "=", m_ShowParms.m_HeightEditorWindow );
  }

QByteArray TXPStep::GetComment()
  {
  return *GetCommentPtr();
  }

QByteArray* TXPStep::GetCommentPtr()
  {
  QByteArray *pComment = m_pComm->GetTextPtr();
  if( pComment != nullptr ) return pComment;
  m_pComm->Add( tXDtext, X_BStr( "Step_Comment", "This step was evaluated" ) );
  return m_pComm->GetTextPtr();
  }

TXPSubTask::~TXPSubTask()
  {
  PSubStep pFirst;
  pFirst.swap( m_pSubStepsL );
  for( PSubStep pNext; !pFirst.isNull(); pNext.swap( pFirst ) )
    {
    pFirst->m_pPrev.clear();
    pNext.swap( pFirst->m_pNext );
    pFirst.clear();
    }
  }

void TXPSubTask::AddSubStep( PSubStep& NewSubStep )
  {
  if( m_pSubStepsL.isNull() )
    {
    m_pSubStepsL = NewSubStep;
    return;
    }
  PSubStep objSubStep( m_pSubStepsL );
  for( ; !objSubStep->m_pNext.isNull(); objSubStep = objSubStep->m_pNext );
  NewSubStep->m_pPrev = objSubStep;
  NewSubStep->m_pNext.clear();
  objSubStep->m_pNext = NewSubStep;
  }

void TXPSubTask::AssignTo( PSubTask& A )
  {
  BaseTask& Task = A->m_pSubMethodL->GetTask();
  if( A.isNull() ) A.reset( new TXPSubTask( Task ) );
  A->m_Name = m_Name;
  A->m_Level = m_Level;
  A->m_pHostSubStep = m_pHostSubStep;
  A->m_pHostSubTask = m_pHostSubTask;
  if( A->m_pSubMethodL.isNull() ) A->m_pSubMethodL.reset( new TXDescrList( Task ) );
  A->m_pSubMethodL->Assign( m_pSubMethodL );
  PSubStep objFromSubStep = m_pSubStepsL;
  A->m_pSubStepsL.reset( new TXPSubStep( Task ) );
  PSubStep objToSubStep = m_pSubStepsL;
  PSubStep oldSubStep;
  while( !objFromSubStep.isNull() )
    {
    if( objToSubStep.isNull() ) objToSubStep.reset( new TXPSubStep( Task ) );
    objFromSubStep->AssignTo( objToSubStep );
    objToSubStep->m_pPrev = oldSubStep;
    oldSubStep = objToSubStep;
    objFromSubStep = objFromSubStep->m_pNext;
    if( !objFromSubStep.isNull() )
      {
      oldSubStep->m_pNext.reset( new TXPSubStep( Task ) );
      objToSubStep = objToSubStep->m_pNext;
      }
    }
  }

void TXPSubTask::AdjustHosting( int iLevel, const PSubTask& ParentSubTask, const PSubStep& ParentSubStep )
  {
  if( iLevel != m_Level || iLevel < 1 ) return;
  if( iLevel == 1 )
    {
    m_pHostSubTask.clear();
    m_pHostSubStep.clear();
    }
  else
    {
    m_pHostSubTask = ParentSubTask;
    m_pHostSubStep = ParentSubStep;
    }
  for( PSubStep objSubStep( m_pSubStepsL ); !objSubStep.isNull(); objSubStep = objSubStep->m_pNext )
    objSubStep->m_pSubTask->AdjustHosting( iLevel + 1, PSubTask( this ), objSubStep );
  }

void TXPSubStep::AssignTo( PSubStep& A )
  {
  BaseTask& Task = m_pMethodL->GetTask();
  if( A.isNull() ) A.reset( new TXPSubStep( Task ) );
  A->m_Name = m_Name;

  if( !m_pMethodL.isNull() )
    {
    if( A->m_pMethodL.isNull() )
      A->m_pMethodL.reset( new TXDescrList( Task ) );
    A->m_pMethodL->Assign( m_pMethodL );
    }
  if( !m_pSubTask.isNull() )
    {
    if( A->m_pSubTask.isNull() )
      A->m_pSubTask.reset( new TXPSubTask( Task ) );
    m_pSubTask->AssignTo( A->m_pSubTask );
    }
  A->m_pPrev.clear();
  A->m_pNext.clear();
  }

void TXPStep::LoadSubTaskFromTaskFile( const QByteArray& KeyWord )
  {
  TaskFile& File = m_Task.GetFile();
  File.reset();
  File.SkipLinesUntilSelectedTrack( KeyWord );
  QByteArray Line;
  do
    Line = File.readLine();
  while( !File.atEnd() && Line.indexOf( KeyWord ) != 0 );
  do
    {
    Line = File.readLine();
    if( File.atEnd() || Line.indexOf( "TRACK" ) != -1 || Line.indexOf( "STEP" ) != -1 ) return;
    } while( Line.trimmed().indexOf( "START_SUB_TASK" ) == -1 );

    bool bFirstLine = true;
    int iCurrentLevel = 0;
    int bInsideSubStep = false;
    PSubTask objCurrentSubTask;
    PSubStep objCurrentSubStep, objNewSubStep;
    do
      {
      if( bFirstLine )
        bFirstLine = false;
      else
        Line = File.readLine();
      while( ( Line[0] == ';' || Line.trimmed().isEmpty() ) && !File.atEnd() )
        Line = File.readLine();
      Line = Line.trimmed();

      if( Line.indexOf( "START_SUB_TASK" ) == 0 )
        {
        if( iCurrentLevel == 0 )
          {
          m_pSubTask = PSubTask( new TXPSubTask( m_Task ) );
          objCurrentSubTask = m_pSubTask;
          }
        else
          {
          if( !bInsideSubStep || objCurrentSubStep.isNull() )
            throw ErrParser( X_Str( "ErrorWhileReadingSubTask", "Error  while(reading SubTask" ), ParserErr::peNewErr );
          objCurrentSubStep->m_pSubTask = PSubTask( new TXPSubTask( m_Task ) );
          objCurrentSubStep->m_pSubTask->m_pHostSubStep = objCurrentSubStep;
          objCurrentSubStep->m_pSubTask->m_pHostSubTask = objCurrentSubTask;
          objCurrentSubTask = objCurrentSubStep->m_pSubTask;
          }
        objCurrentSubStep.clear();
        bInsideSubStep = false;
        objCurrentSubTask->m_Name = BaseTask::ExtractNameFromString( Line );
        ++iCurrentLevel;
        objCurrentSubTask->m_Level = iCurrentLevel;
        continue;
        }

      if( Line.indexOf( "END_SUB_TASK" ) == 0 )
        {
        if( iCurrentLevel == 1 )
          --iCurrentLevel;
        else
          {
          objCurrentSubStep = objCurrentSubTask->m_pHostSubStep;
          objCurrentSubTask = objCurrentSubTask->m_pHostSubTask;
          --iCurrentLevel;
          }
        continue;
        }

      // This case holds,  if(step contains no subtasks at all
      if( iCurrentLevel == 0 )
        return;

      // Here we see some line inside SubTask of iCurrentLevel
      // if(we read new SUB_STEP of current subtask of iCurrentLevel...
      if( Line.indexOf( "SUB_STEP" ) == 0 )
        {
        // Create new SubStep, insert it in current SubTask&&regard it as current
        objNewSubStep = PSubStep( new TXPSubStep( m_Task ) );
        objNewSubStep->m_Name = BaseTask::ExtractNameFromString( Line );
        objCurrentSubTask->AddSubStep( objNewSubStep );
        objCurrentSubStep = objNewSubStep;
        objNewSubStep.clear();
        bInsideSubStep = true;
        continue;
        }
      // if(we read some line of subtask of iCurrentLevel
      // if(we read some line of subtask of iCurrentLevel, line from METHOD section of this SubTask...
      if( !bInsideSubStep )
        {
        objCurrentSubTask->m_pSubMethodL->Update( Line );
        continue;
        }
      if( bInsideSubStep )
        {
        objCurrentSubStep->m_pMethodL->Update( Line );
        continue;
        }
      } while( iCurrentLevel > 0 && !File.atEnd() );
  }

void TXPStep::ShowParms::Save( QByteStream & Stream, QByteArray& Id )
  {
  if( m_HideMultSign )
    Stream << "HIDEMULTSIGN" << Id << "\r\n";
  if( m_ShowMultSign )
    Stream << "SHOWEMULTSIGN" << Id << "\r\n";
  if( m_ShowUnarMinus )
    Stream << "SHOWUNARMINUS" << Id << "\r\n";
  if( m_HideUnarMinus )
    Stream << "HIDEUNARMINUS" << Id << "\r\n";
  if (m_ShowMinusByAddition)
    Stream << "SHOWMINUSBYADDITION" << Id << "\r\n";
  if (m_HideMinusByAddition)
    Stream << "HIDEMINUSBYADDITION" << Id << "\r\n";
  if( m_ShowRad )
    Stream << "ANGLERAD" << Id << "\r\n";
  if( m_ShowDeg )
    Stream << "ANGLEDEG" << Id << "\r\n";
  if (m_OnExactCompare)
    Stream << "EXACTCOMPARE" << Id << "\r\n";
  if (m_NoHint)
    Stream << "NOHINT" << Id << "\r\n";
  Stream << "EDITORHEIGHT" << Id << "=" << m_HeightEditorWindow << "\r\n";
  }

void TXPStep::ResetMark( int NewMark )
  {
  m_Mark = NewMark;
  int PercentPos = m_Name.lastIndexOf("%)");
  if(PercentPos != -1)
    {
    int iStart = m_Name.lastIndexOf('(', PercentPos);
    m_Name.remove(iStart, PercentPos - iStart + 2);
    }
  int iStart = m_Name.length() - 1;
  while(iStart > 0 && m_Name[iStart] == ' ') iStart--;
  m_Name.insert(iStart + 1, " (" + QByteArray::number(NewMark) + "%)");
  }

void TXPStep::Save( QByteStream& Stream, int iId )
  {
  QByteArray ID = QByteArray::number( iId );
  Stream << "STEP" << ID << "   '" << m_Name << "'\r\n";
  m_ShowParms.Save( Stream, ID );
  Stream << "METH" << ID << "\r\n";
  TXDescrList::Save( Stream, m_pMethodL );
  Stream << "PROMPT" << ID << "\r\n";
  TXDescrList::Save(Stream, m_pAnswerPrompt);
  Stream << "RESULT" << ID << "  ";
  for( PDescrMemb index = m_pResultE->m_pFirst; !index.isNull(); index = index->m_pNext )
    if( index->m_Kind == tXDexpress )
      {
      if(m_pResultE->m_pFirst == nullptr)
        throw ErrParser( "Step: " + ID + " Result expression not exists!", ParserErr::peNewErr );
      Stream << "EXPR(" << index->m_Content << ")\r\n";
      }
  if( !m_pAnswerTemplate->m_pFirst.isNull() )
    Stream << "TEMPLATE" << ID << "  EXPR(" << m_pAnswerTemplate->m_pFirst->m_Content << ")\r\n";
  if(m_pF1->m_pFirst == nullptr)
    throw ErrParser( "Step: " + ID + " Result expression False1 not exists!", ParserErr::peNewErr );
  QByteArray Content = m_pF1->m_pFirst->m_Content;
  if(Content.isEmpty()) Content = "Answer";
  Stream << "FALSE" << ID << ".1  EXPR(" << Content << ")\r\n";
  TXDescrList::Save(Stream, m_pFComm1);
  if(m_pF2->m_pFirst == nullptr)
    throw ErrParser( "Step: " + ID + " Result expression False2 not exists!", ParserErr::peNewErr );
  Content = m_pF2->m_pFirst->m_Content;
  if(Content.isEmpty()) Content = "Answer";
  Stream << "FALSE" << ID << ".2  EXPR(" << Content << ")\r\n";
  TXDescrList::Save(Stream, m_pFComm2);
  if(m_pF3->m_pFirst == nullptr)
    throw ErrParser( "Step: " + ID + " Result expression False3 not exists!", ParserErr::peNewErr );
  Content = m_pF3->m_pFirst->m_Content;
  if(Content.isEmpty()) Content = "Answer";
  Stream << "FALSE" << ID << ".3  EXPR(" << Content << ")\r\n";
  TXDescrList::Save(Stream, m_pFComm3);
  Stream << "COMM" << ID << "   '" << GetComment() << "'\r\n";
  }

void TXPStep::SetGlobalSWhowRad( bool Value )
  {
  m_ShowRad = Value;
  m_ShowParms.m_ShowRad = Value;
  m_ShowParms.m_ShowDeg = !Value;
  }

TTrack::TTrack( BaseTask& Task ) : m_MultyTrack( false ), m_TracksDescription( new TXDescrList( Task ) ), m_TrackSelected( false ),
m_SelectedTrack( 0 ), m_Task( Task )  {}

void TTrack::Clear()
  {
  m_MultyTrack = false;
  m_TracksDescription->Clear();
  m_TrackSelected = false;
  m_SelectedTrack = 0;
  m_NameOfTrack.clear();
  }

void TTrack::LoadTrackFromTaskFile()
  {
  TaskFile& File = m_Task.GetFile();
  File.reset();
  QByteArray Line;
  do
    {
    Line = File.readLine();
    if( Line.indexOf( "NAME_TRACK" ) == 0 )
      {
      m_MultyTrack = true;
      m_NameOfTrack.append( BaseTask::ExtractNameFromString( Line.trimmed() ) );
      }
    } while( !File.atEnd() && Line.indexOf( "DESCRIPTION_TRACKS" ) != 0 );
    if( !m_MultyTrack ) return;
    m_SelectedTrack = 1;
    m_TracksDescription->LoadFromTaskFile( "DESCRIPTION_TRACKS" );
  }

void TTrack::Save( QByteStream& Stream )
  {
  if( !m_MultyTrack ) return;
  for( int iTrack = 0; iTrack < m_NameOfTrack.count(); iTrack++ )
    Stream << "NAME_TRACK" << ( iTrack + 1 ) << " '" << m_NameOfTrack[iTrack] << "'\r\n";
  Stream << "DESCRIPTION_TRACKS\r\n";
  TXDescrList::Save( Stream, m_TracksDescription );
  }

BaseTask::BaseTask() :m_WorkMode(wrkTrain), m_WasConstraint(false), m_pFile(new TaskFile(this)), m_NewTask(false),
m_pStepsL(new TXStepList), m_OutTemplate(false), m_Exiting(false), m_PictureUpPos(true), m_MultiTask(false), m_ExactCompare(false)
  {
  m_pTrack = PTrack( new TTrack( *this ) );
  m_pCalc = new CalcList( this );
  m_pQuestion = PDescrList( new TXDescrList( *this ) );
  m_pMethodL = PDescrList( new TXDescrList( *this ) );
  m_pComment = PDescrList( new TXDescrList( *this ) );
  m_pPrompt = PDescrList( new TXDescrList( *this ) );
  sm_LastCreatedTrack = 0;
  }

BaseTask::~BaseTask()
  {
  delete m_pFile;
  delete m_pCalc;
  }

void BaseTask::Clear()
  {
  m_pTrack->Clear();
  m_Name = "";
  m_pCalc->clear();
  m_pQuestion->Clear();
  m_pMethodL->Clear();
  m_pStepsL->Clear();
  m_pComment->Clear();
  m_pPrompt->Clear();
  m_pCurr.clear();
  m_pMarker.clear();
  m_Something_skipped = false;
  m_Exiting = false;
  m_PictureUpPos = true;
  m_SummMarks = 0;
  m_Language = XPInEdit::sm_Language;
  sm_LastCreatedTrack = 0;
  sm_GlobalShowUnarMinus = true;
  sm_GlobalShowMinusByAddition = false;
  sm_GlobalShowMultSign = false;
  sm_GlobalShowRad = true;
  }

void BaseTask::ClearTrackDependent()
  {
  m_pMethodL->Clear();
  m_pStepsL->Clear();
  m_pComment->Clear();
  m_pCurr.clear();
  m_pMarker.clear();
  m_Something_skipped = false;
  m_Exiting = false;
  m_PictureUpPos = true;
  }

void BaseTask::SetFileName( const QString& FName )
  {
  qDebug() << "Name";
  qDebug() << FName;
  XPInEdit::sm_BasePath = FName.left(FName.lastIndexOf('/'));
  m_OutTemplate = false;
  m_Template = "";
  if( m_pFile->isOpen() ) m_pFile->close();
  m_pFile->setFileName( FName );
  m_pFile->open( QIODevice::ReadOnly );
  if( !m_pFile->isOpen() )
    throw ErrParser( "Can't open task file " + FName, ParserErr::peNewErr );
  LoadFromFile();
  m_Language = GetLangByFileName( FName );
  if( m_pQuestion->m_QuestionCount > 0 && m_pQuestion->m_QuestionCount != m_pStepsL->m_Counter )
    throw ErrParser( "Task: " + FName + "Questions number is not equal to steps number!", ParserErr::peNewErr );
  }

void BaseTask::SetWorkMode( TWorkMode WMode )
  {
  if( m_WorkMode == WMode ) return;
  m_WorkMode = WMode;
  SetFirstStep();
  }

void BaseTask::SetGlobalSWhowRad( bool Value)
  {
  m_Header.m_ShowDeg = !Value;
  m_Header.m_ShowRad = Value;
  m_pStepsL->SetGlobalSWhowRad(Value);
  sm_GlobalShowRad = Value;
  }

void BaseTask::SetWorkMode( const QString& Mode )
  {
  if( Mode == "wrkExam" )
    m_WorkMode = wrkExam;
  else
    if( Mode == "wrkLearn" )
      m_WorkMode = wrkLearn;
    else
      m_WorkMode = wrkTrain;
  }

void BaseTask::CalcRead()
  {
  if( m_pCalc->isEmpty() )
    {
    if( !m_pFile->isOpen() ) return;
    m_pCalc->LoadFromTaskFile( "CALC" );
    }
//  for( int iPair = 0; iPair < m_pCalc->count(); iPair++ )
//    ( *m_pCalc )[iPair].CalcExpress();
//    else
//  OutTemplate( m_Template );
//  m_OutTemplate = true;
  }

QByteArray BaseTask::GetTemplate( int Step )
  {
  PStepMemb pStep = m_pStepsL->m_pFirst;
  for( ; --Step > 0 && !pStep.isNull(); pStep = pStep->m_pNext );
  if( Step > 0 || pStep.isNull() || pStep->m_pAnswerTemplate.isNull() || pStep->m_pAnswerTemplate->m_pFirst.isNull() ||
    pStep->m_pAnswerTemplate->m_pFirst->m_Kind != tXDexpress ) return "";
  QByteArray Content = pStep->m_pAnswerTemplate->m_pFirst->m_Content;
  qDebug() << "Content" << Content;
  EdTable::sm_WasTemplate = true;
  MathExpr E( Parser::StrToExpr( Content ) );
  EdTable::sm_WasTemplate = false;
  if( s_GlobalInvalid || E.IsEmpty() ) 
    throw ErrParser( X_Str( "MTaskCorrupt", "Task corrupted " ) + Content, ParserErr::peNewErr );
  TStr::sm_Server = true;
  QByteArray Result( E.SWrite().replace( '\\', "\\\\" ).replace( '\n', "\\n" ) );
  TStr::sm_Server = false;
  qDebug() << "Content Result" << Result;
  return Result;
  }

QByteArray BaseTask::GetComment()
  {
  return *GetCommentPtr();
  }

QByteArray* BaseTask::GetCommentPtr()
  {
  QByteArray *pComment = m_pComment->GetTextPtr();
  if( pComment != nullptr ) return pComment;
  m_pComment->Add( tXDtext, EdStr::sm_pCodec->fromUnicode((X_Str( "MNoComment", "Task evaluated, no comment" ) )));
  return m_pComment->GetTextPtr();
  }

void BaseTask::LoadFromFile()
  {
  Clear();
  if( !m_pFile->isOpen() ) return;
  m_OutTemplate = true;
  m_pFile->reset();
  m_pTrack->LoadTrackFromTaskFile();
  sm_LastCreatedTrack = TrackCount();
  m_pFile->LoadNameFromTaskFile( "TASK", m_Name );
  if(m_Name.isEmpty()) m_Name = "Nameless";
//    throw ErrParser( "Error; Task without name", ParserErr::peName );
  m_pFile->LoadRValueFromTaskFile( "SIMULATION", s_XPSimNumber );
  m_pFile->LoadBooleanFromTaskFile( "DOWNPICT", m_PictureUpPos );
  sm_NewHebFormat = true;
  m_pFile->LoadBooleanFromTaskFile( "NEWHEBFORMAT", sm_NewHebFormat );
  sm_NewHebFormat = !sm_NewHebFormat;
  m_MultiTask = true;
  m_pFile->LoadBooleanFromTaskFile( "MULTITASK", m_MultiTask );
  m_MultiTask = !m_MultiTask;
  TExpr::sm_Accuracy = 0.01;
  if( m_pFile->LoadRValueFromTaskFile( "ACCURACY=", TExpr::sm_Accuracy ) )
    m_Header.m_Accuracy = TExpr::sm_Accuracy;
  m_pCalc->LoadFromTaskFile( "INIT" );
  if(m_pCalc->isEmpty())
    m_pCalc->LoadFromTaskFile( "CALC" );
  m_pQuestion->m_bHasEndLeft = false;
  m_pQuestion->LoadFromTaskFile( "QUESTION" );
  m_pMethodL->LoadFromTaskFile( "METHOD" );
  m_pComment->LoadFromTaskFile( "COMMENT" );
  m_pPrompt->LoadFromTaskFile( "PROMPT" );
  m_Header.m_HideUnarMinus = m_pFile->LoadBooleanFromTaskFile( "HIDEUNARMINUS", sm_GlobalShowUnarMinus );
  bool HideUnarm = !sm_GlobalShowUnarMinus;
  m_Header.m_ShowUnarMinus = m_pFile->LoadBooleanFromTaskFile( "SHOWUNARMINUS", HideUnarm );
  sm_GlobalShowUnarMinus = !HideUnarm;
  m_Header.m_HideMinusByAddition = m_pFile->LoadBooleanFromTaskFile("HIDEMINUSBYADDITION", sm_GlobalShowMinusByAddition);
  bool HideMinus = !sm_GlobalShowMinusByAddition;
  m_Header.m_ShowMinusByAddition = m_pFile->LoadBooleanFromTaskFile("SHOWMINUSBYADDITION", HideMinus);
  sm_GlobalShowMinusByAddition = !HideMinus;
  m_Header.m_HideMultSign = m_pFile->LoadBooleanFromTaskFile( "HIDEMULTSIGN", sm_GlobalShowMultSign );
  bool HideMSign = !sm_GlobalShowMultSign;
  m_Header.m_ShowMultSign = m_pFile->LoadBooleanFromTaskFile( "SHOWMULTSIGN", HideMSign );
  sm_GlobalShowMultSign = !HideMSign;
  m_Header.m_ShowDeg = m_pFile->LoadBooleanFromTaskFile( "ANGLEDEG", sm_GlobalShowRad );
  bool ShowDeg = !sm_GlobalShowRad;
  m_Header.m_ShowRad = m_pFile->LoadBooleanFromTaskFile( "ANGLERAD", ShowDeg );
  s_ShowMinute = m_pFile->LoadBooleanFromTaskFile("SHOWMINUTE", ShowDeg);
  sm_GlobalShowRad = !ShowDeg;
  TExpr::sm_TrigonomSystem = sm_GlobalShowRad ? TExpr::tsRad : TExpr::tsDeg;
  ResetParms();
  for( int Nstep = 1;; Nstep++ )
    {
    QByteArray StepId( QByteArray::number( Nstep ) );
    if( !m_pFile->Step_exist( StepId ) ) break;
    m_pStepsL->Add( StepId, *this );
    }
  SetFirstStep();
  m_pStepsL->ResetMarks();
  m_pMarker = m_pCurr;
  }

void BaseTask::LoadFromStack(QString& FileName)
  {
  QFile File(FileName);
  if( !File.open( QIODevice::ReadOnly ) ) throw ErrParser( "Error; Can't open file: " + FileName, ParserErr::peNewErr );
  QByteArrayList StackTest = File.readAll().split('\n');
  int iStart = 0, iCurrent;
  auto TextSearch = [&]( const QByteArray& Text )
    {
    iCurrent = iStart;
    int iText;
    for (; iCurrent < StackTest.count() && (iText = StackTest[iCurrent].indexOf(Text)) == -1; iCurrent++  );
    if(iText == -1) return -1;
    return iText + Text.count();
    };

//  if(TextSearch("<name>") == -1 ) throw ErrParser( "This File has no Stack", ParserErr::peNewErr );
//  m_OutTemplate = true;
//  sm_LastCreatedTrack = 0;
//  int iName = TextSearch("<text>");
//  if(iName == -1 ) throw ErrParser( "This File has no Stack", ParserErr::peNewErr );
//  m_Name = StackTest[iCurrent].mid(iName, iName - StackTest[iCurrent].lastIndexOf('<'));
//  m_Language = lngEnglish;
//  iStart = iCurrent;
//  if(TextSearch("ltr") != -1 ) m_Language = lngHebrew;
  if(TextSearch("<questionvariables>") == -1 ) throw ErrParser( "This File has no Stack", ParserErr::peNewErr );
  iStart = iCurrent;
  int iCalc = TextSearch("<text>");
  if(iCalc == -1 ) throw ErrParser( "This File has no Stack", ParserErr::peNewErr );
  iStart = iCurrent;
  iCalc = TextSearch("</questionvariables>");
  if(iCalc == -1 ) throw ErrParser( "This File has no Stack", ParserErr::peNewErr );
//  sm_NewHebFormat = true;
//  m_MultiTask = false;
//  TExpr::sm_Accuracy = 0.01;
  QByteArrayList CalcText = StackTest.mid(iStart, iCurrent - iStart);
  CalcText[0].replace("<text>", "");
  CalcText.last().replace("</text>", "");
  m_pCalc->LoadFromQuestionVariables(CalcText);

/*

  m_pQuestion->m_bHasEndLeft = false;
  m_pQuestion->LoadFromTaskFile( "QUESTION" );
  m_pMethodL->LoadFromTaskFile( "METHOD" );
  m_pComment->LoadFromTaskFile( "COMMENT" );
  m_pPrompt->LoadFromTaskFile( "PROMPT" );
  m_Header.m_HideUnarMinus = m_pFile->LoadBooleanFromTaskFile( "HIDEUNARMINUS", sm_GlobalShowUnarMinus );
  bool HideUnarm = !sm_GlobalShowUnarMinus;
  m_Header.m_ShowUnarMinus = m_pFile->LoadBooleanFromTaskFile( "SHOWUNARMINUS", HideUnarm );
  sm_GlobalShowUnarMinus = !HideUnarm;
  m_Header.m_HideMinusByAddition = m_pFile->LoadBooleanFromTaskFile("HIDEMINUSBYADDITION", sm_GlobalShowMinusByAddition);
  bool HideMinus = !sm_GlobalShowMinusByAddition;
  m_Header.m_ShowMinusByAddition = m_pFile->LoadBooleanFromTaskFile("SHOWMINUSBYADDITION", HideMinus);
  sm_GlobalShowMinusByAddition = !HideMinus;
  m_Header.m_HideMultSign = m_pFile->LoadBooleanFromTaskFile( "HIDEMULTSIGN", sm_GlobalShowMultSign );
  bool HideMSign = !sm_GlobalShowMultSign;
  m_Header.m_ShowMultSign = m_pFile->LoadBooleanFromTaskFile( "SHOWMULTSIGN", HideMSign );
  sm_GlobalShowMultSign = !HideMSign;
  m_Header.m_ShowDeg = m_pFile->LoadBooleanFromTaskFile( "ANGLEDEG", sm_GlobalShowRad );
  bool ShowDeg = !sm_GlobalShowRad;
  m_Header.m_ShowRad = m_pFile->LoadBooleanFromTaskFile( "ANGLERAD", ShowDeg );
  s_ShowMinute = m_pFile->LoadBooleanFromTaskFile("SHOWMINUTE", ShowDeg);
  sm_GlobalShowRad = !ShowDeg;
  TExpr::sm_TrigonomSystem = sm_GlobalShowRad ? TExpr::tsRad : TExpr::tsDeg;
  ResetParms();
  for( int Nstep = 1;; Nstep++ )
    {
    QByteArray StepId( QByteArray::number( Nstep ) );
    if( !m_pFile->Step_exist( StepId ) ) break;
    m_pStepsL->Add( StepId, *this );
    }
  SetFirstStep();
  m_pStepsL->ResetMarks();
  m_pMarker = m_pCurr;
*/
  }

void BaseTask::SetFirstStep()
  {
  PStepMemb Step = m_pStepsL->m_pFirst;
  if( m_WorkMode == wrkExam )
    for( ; Step != m_pStepsL->m_pLast && Step->m_Mark == 0; Step = Step->m_pNext );
  SetCurr( Step );
  }

void BaseTask::LoadTrackDependentFromFile()
  {
  m_pMethodL->LoadFromTaskFile( "METHOD" );
  m_pComment->LoadFromTaskFile( "COMMENT" );
  m_pPrompt->LoadFromTaskFile( "PROMPT" );
  for( int Nstep = 1;; Nstep++ )
    {
    QByteArray StepId( QByteArray::number( Nstep ) );
    if( !m_pFile->Step_exist( StepId ) ) break;
    m_pStepsL->Add( StepId, *this );
    }
  SetFirstStep();
  m_pStepsL->ResetMarks();
  m_pMarker = m_pCurr;
  }

QByteArray BaseTask::ExtractNameFromString( const QByteArray& Line )
  {
  //Argument string was previously processed by Trim function
  //In order to minimize damage of errors in task text, try to read name even if
  //apostrophes were omitted
  QByteArray sName( Line );
  //Remove keyword
  int i = Line.indexOf( ' ' );
  if( i > 0 )
    sName = sName.mid( i + 1 );
  sName = sName.trimmed();
  if( sName.indexOf( '\'' ) == 0 )
    sName = sName.mid( 1 );
  i = sName.indexOf( '\'' );
  if( i > 0 )
    sName = sName.left( i );
  return sName;
  }

PSubTask BaseTask::ExtractSubTask( int indStep )
  {
  PSubTask pObjSubTask( new TXPSubTask( *this ) );
  PStepMemb  pSindex = m_pStepsL->m_pFirst;
  for( int i = 1; !pSindex.isNull() && i < indStep; pSindex = pSindex->m_pNext );
  if( !pSindex.isNull() && !pSindex->m_pSubTask.isNull() )
    pSindex->m_pSubTask->AssignTo( pObjSubTask );
  return pObjSubTask;
  }

int BaseTask::TrackCount()
  {
  return m_pTrack->m_NameOfTrack.count();
  }

QString RichTextDocument::GetTempPath()
  {
  return sm_TempPath + '/' + "TestTmp" + QString::number( RichTextDocument::sm_NumTmp++)+ ".jpg";
  }

RichTextDocument::RichTextDocument(const QString& TempPath) : m_TempPath(TempPath), m_pTable(nullptr), m_FixedWidth(0)
  {
  m_ViewSettings.m_BkgrColor = "#dceff5";
  m_ViewSettings.m_TaskCmFont.setFamily( "Arial" );
  m_ViewSettings.m_TaskCmFont.setPointSize( 16 );
  m_ViewSettings.m_SimpCmFont.setFamily( "Arial" );
  m_ViewSettings.m_SimpCmFont.setPointSize( 16 );
  ResetLanguage();
  }

void RichTextDocument::ResetLanguage()
  {
  QTextOption option = defaultTextOption();
  if( XPInEdit::sm_Language == lngHebrew )
    option.setTextDirection( Qt::RightToLeft );
  else
    option.setTextDirection( Qt::LeftToRight );
  setDefaultTextOption( option );
  }

enum OutMode { omText, omTextLang, omTextComplex, omPict, omExpress, omEmpy };
class OutObject : public QByteArray
  {
  public:
    bool m_Space;
    OutMode m_Mode;
    OutObject( const QByteArray& Text, OutMode Mode, bool bSpace = true );
    OutObject() : QByteArray(), m_Mode( omEmpy ) {}
  };

OutObject::OutObject( const QByteArray& Text, OutMode Mode, bool bSpace ) : QByteArray( Text ), m_Mode( Mode ), m_Space( bSpace )
  {
  int iMinus = indexOf( '-' );
  if( Mode != omExpress && iMinus != -1 && iMinus < length() - 1 && ( uchar ) at( iMinus + 1 ) == 229 )
    {
    ( *this )[iMinus] = 229;
    ( *this )[iMinus + 1] = '-';
    }
  }

void RichTextDocument::AddRow()
  {
  if( m_pTable == nullptr ) return;
  int TextColumn = m_Language == lngHebrew ? 1 : 0;
  m_pTable->appendRows( 1 );
  int Row = m_pTable->rows() - 1;
  QTextCursor Cursor = m_pTable->cellAt( Row, TextColumn ).firstCursorPosition();
  QTextCursor PictCursor = m_pTable->cellAt( Row, 1 - TextColumn ).firstCursorPosition();
  QTextBlockFormat BlockFormat = Cursor.blockFormat();
  QTextBlockFormat PicBlocFormat = PictCursor.blockFormat();
  if( m_Language != lngHebrew )
    {
    BlockFormat.setAlignment( Qt::AlignLeft | Qt::AlignAbsolute );
    PicBlocFormat.setAlignment( Qt::AlignLeft | Qt::AlignHCenter );
    }
  else
    PicBlocFormat.setAlignment( Qt::AlignRight | Qt::AlignHCenter );
  }

void RichTextDocument::DeleteRow()
  {
  if( m_pTable == nullptr || m_pTable->rows() < 2 ) return;
  for( int iRow = 0, iCol; iRow < m_pTable->rows(); iRow++ )
    {
    for( iCol = 0; iCol < 2; iCol++ )
      {
      QTextTableCell Cell = m_pTable->cellAt( iRow, iCol );
      QTextFrame::iterator pText = Cell.begin();
      for( ; pText != Cell.end() && pText.currentBlock().text().isEmpty(); pText++ );
      if( pText != Cell.end() ) break;
      }
    if( iCol == 2 )
      {
      m_pTable->removeRows( iRow, 1 );
      return;
      }
    }
  }

void RichTextDocument::SetContent( PDescrList Content )
  {
  Content->m_pDocument = this;
  m_Language = Content->GetTask().GetLanguage();
  bool m_NewFormat = BaseTask::sm_NewHebFormat || m_Language != lngHebrew;
  clear();
  QTextCursor MainCursor( QTextCursor( this ) );
  m_pTable = MainCursor.insertTable( 1, 2 );
  QVector<QTextLength> Constraints;
  int LeftPercent = m_Language == lngHebrew ? 20 : 80;
  Constraints.push_back( QTextLength( QTextLength::PercentageLength, LeftPercent ) );
  Constraints.push_back( QTextLength( QTextLength::PercentageLength, 100 - LeftPercent ) );
  QTextTableFormat TableFormat;
  if( m_FixedWidth > 0 ) TableFormat.setWidth( QTextLength( QTextLength::FixedLength, m_FixedWidth ) );
  TableFormat.setColumnWidthConstraints( Constraints );
  if( s_TaskEditorOn )
    TableFormat.setBorder( 1 );
  else
    TableFormat.setBorder( 0 );
  m_pTable->setFormat( TableFormat );
  int TextColumn = m_Language == lngHebrew ? 1 : 0;
  QTextCursor Cursor( m_pTable->cellAt( 0, TextColumn ).firstCursorPosition() );
  QTextCursor PictCursor = m_pTable->cellAt( 0, 1 - TextColumn ).firstCursorPosition();
  QTextCursor *pActiveCursor = &Cursor;
  bool InsertRow = false;
  QTextCharFormat TextFormat;
  QTextBlockFormat BlockFormat = Cursor.blockFormat();
  QTextBlockFormat PicBlocFormat = PictCursor.blockFormat();
  if( m_Language != lngHebrew )
    {
    BlockFormat.setAlignment( Qt::AlignLeft | Qt::AlignAbsolute );
    PicBlocFormat.setAlignment( Qt::AlignLeft | Qt::AlignHCenter );
    }
  else
    {
    BlockFormat.setAlignment( Qt::AlignRight | Qt::AlignAbsolute );
    PicBlocFormat.setAlignment( Qt::AlignRight | Qt::AlignHCenter | Qt::AlignAbsolute );
    }
  TextFormat.setFont( m_ViewSettings.m_TaskCmFont );

  bool InsertSpace = false;
  bool bWasExspress = false;
  bool bWasHebrew = false;
  bool bWasBreak = false;
  bool bFirstBreak = true;

  auto AddRow = [&] ()
    {
    m_pTable->appendRows( 1 );
    int Row = m_pTable->rows() - 1;
    Cursor = m_pTable->cellAt( Row, TextColumn ).firstCursorPosition();
    PictCursor = m_pTable->cellAt( Row, 1 - TextColumn ).firstCursorPosition();
    BlockFormat = Cursor.blockFormat();
    if( m_Language != lngHebrew )
      {
      BlockFormat.setAlignment( Qt::AlignLeft | Qt::AlignAbsolute );
      PicBlocFormat.setAlignment( Qt::AlignLeft | Qt::AlignHCenter );
      }
    else
      PicBlocFormat.setAlignment( Qt::AlignRight | Qt::AlignHCenter );
    InsertSpace = false;
    bWasExspress = false;
    bWasHebrew = false;
    bWasBreak = false;
    bFirstBreak = true;
    };

  auto NoHebrew = [&] ( const QByteArray& Arr )
    {
    if( bWasHebrew ) return false;
    for( int iChar = 1; iChar < Arr.length(); iChar++ )
      {
      uchar Byte = Arr[iChar];
      bWasHebrew = Byte >= 224 && Byte <= 250;
      if( bWasHebrew ) return false;
      }
    return true;
    };

  auto HasColon = [] ( const QByteArray& Arr )
    {
    for( int iChar = 1; iChar < Arr.length(); iChar++ )
      if( Arr[iChar] == ':' ) return true;
    return false;
    };

  auto AddText = [&] ( const QByteArray& Text )
    {
    if( Text.left( 2 ) == "<a" )
      pActiveCursor->insertHtml( Text );
    else
      {
      BlockFormat.setAlignment( Text == "." || Text == "?" || Text == "," || Text == ":" ? Qt::AlignLeft : Qt::AlignRight | Qt::AlignAbsolute );
      pActiveCursor->setBlockFormat( BlockFormat );
      pActiveCursor->insertText( Text, TextFormat );
      }
    InsertSpace = false;
    };

  auto AddLangText = [&] ( const QByteArray& Text )
    {
    if( Text.left( 2 ) == "<a" )
      pActiveCursor->insertHtml( Text );
    else
      {
      if( m_Language == lngHebrew )
        if( m_NewFormat )
          BlockFormat.setAlignment( Qt::AlignRight | Qt::AlignAbsolute );
        else
          BlockFormat.setAlignment( bWasHebrew ? Qt::AlignLeft : Qt::AlignRight );
      pActiveCursor->setBlockFormat( BlockFormat );
      pActiveCursor->insertText( ToLanguage( Text, m_Language ), TextFormat );
      }
    InsertSpace = false;
    };

  auto AddComplexText = [&] ( const QByteArray& Text )
    {
    BlockFormat.setAlignment( Qt::AlignRight | Qt::AlignAbsolute );
    //    BlockFormat.setAlignment( Qt::AlignLeft );
    pActiveCursor->setBlockFormat( BlockFormat );
    pActiveCursor->insertText( FromQBA( Text, m_Language, InsertSpace ), TextFormat );
    InsertSpace = false;
    };

  auto AddExpress = [&] ( const QByteArray& Text, bool bSpace )
    {
    bool bErr = false;
    MathExpr Expr;
    s_iDogOption = !s_TaskEditorOn;
    Expr = MathExpr( Parser::StrToExpr( Text ) );
    s_iDogOption = 0;
    bErr = s_GlobalInvalid || Expr.IsEmpty();
    if( bErr )
      {
      Info_m( X_Str( "XPStatMess", "Task corrupted " ) + Text );
      return;
      }
//    bool bBrackets = Text[0] == '(' && Text.indexOf( "Matric" ) == -1 && Text.indexOf( "syst" ) == -1 && Text.indexOf( "Lim" ) == -1 &&
//      Text.indexOf( "Integral" ) == -1 && Expr.WriteE()[0] != '(' && Text.indexOf( "Der" ) == -1;
    BaseTask::sm_pEditSets->m_BkgrColor = m_ViewSettings.m_BkgrColor;
    TMult::sm_ShowMultSign = BaseTask::sm_GlobalShowMultSign;
    TMult::sm_ShowUnarMinus = BaseTask::sm_GlobalShowUnarMinus;
    TSumm::sm_ShowMinusByAddition = BaseTask::sm_GlobalShowMinusByAddition;
    //    TExpr::sm_TrigonomSystem = BaseTask::sm_GlobalShowRad ? TExpr::tsRad : TExpr::tsDeg;
    XPInEdit InEd( Expr.SWrite(), *BaseTask::sm_pEditSets, m_ViewSettings );
    QString Path( m_TempPath + '/' + "TestTmp" + QString::number( sm_NumTmp++ ) + ".jpg" );
    QImage *pImage = InEd.GetImage();
    pImage->setText( "F1", Parser::PackUnAscii( Text ) );
    pImage->save( Path );
    QTextImageFormat ExFormat;
    ExFormat.setName( Path );
    ExFormat.setVerticalAlignment( QTextCharFormat::AlignMiddle );
//    if( bSpace ) Cursor.insertText( " ", TextFormat );
//    if( bBrackets )
//      Cursor.insertText( "(", TextFormat );
    pActiveCursor->insertImage( ExFormat );
//    if( bBrackets )
//      Cursor.insertText( ")", TextFormat );
    bWasExspress = true;
    InsertSpace = bSpace;
    };

  qDebug() << "Start RichTextDocument";
  QQueue<OutObject> ToRight;
  QQueue<OutObject> ToLeft;

  bool bToLeft = true;
  bool bColon = false;
  bool bExprError = false;

  auto AddPict = [&] ( const QByteArray& Text )
    {
    QByteArray PictName( Text );
    if( Text[0] == msMetaSign )
      {
      MathExpr body;
      if( !ExpStore::sm_pExpStore->Known_var_expr( Text.mid( 1 ), body ) )
        {
        Info_m( X_Str( "XPStatMess", "Task corrupted " ) + Text );
        return;
        }
      PictName = body.WriteE();
      }
    if( InsertRow )  AddRow();
    InsertRow = true;
    pActiveCursor->setBlockFormat( PicBlocFormat );
    QString Fout = Content->GetTask().GetFile().fileName();
    Fout = Fout.left( Fout.lastIndexOf( '/' ) + 1 ) + PictName;
    QString PictFile = Fout + ".jpg";
    if(!QFile::exists(PictFile)) PictFile = Fout + ".bmp";
    QTextImageFormat ExFormat;
    ExFormat.setName( PictFile );
    pActiveCursor->insertImage( ExFormat );
    if( !Content->m_bHasEndLeft ) bToLeft = false;
  };

  auto PutObject = [&] ( QQueue<OutObject>* pQueue )
    {
    const OutObject &Out = pQueue->dequeue();
    if(pQueue == &ToLeft)
      pActiveCursor = &PictCursor;
    else
      pActiveCursor = &Cursor;

    switch( Out.m_Mode )
      {
      case omText:
        AddText( Out );
        return;
      case omTextLang:
        AddLangText( Out );
        return;
      case omTextComplex:
        AddComplexText( Out );
        return;
      case omExpress:
        AddExpress( Out, Out.m_Space );
        return;
      case omPict:
        AddPict( Out );
        bFirstBreak = true;
      }
    };

  auto CreateExpression = [&] ( const QByteArray Content )
    {
    return false;
    if( NoHebrew( Content ) ) return false;
    QList<QByteArray> Words;
    int iMinus = Content.indexOf( '-' );
    if( iMinus > 0 && Content[iMinus - 1] != ' ' )
      {
      QByteArray TmpContent( Content );
      TmpContent.insert( iMinus, ' ' );
      Words = TmpContent.split( ' ' );
      }
    else
      Words = Content.split( ' ' );
    auto pWord = Words.begin();
    auto pLeft = pWord;
    int StartCount = ToRight.count();;
    for( bWasHebrew = false; pWord != Words.end(); pWord++, bWasHebrew = false )
      {
      QByteArray& Word = *pWord;
      if( Word.isEmpty() || !NoHebrew( Word ) ) continue;
      int iStart = 0;
      for( ; iStart < Word.length() && !ISALPHA( Word[iStart] ); iStart++ );
      if( iStart < Word.length() )
        {
        if( iStart > 0 && Word[iStart - 1] == '(' ) iStart--;
        QByteArray sExpr( Word.mid( iStart ) );
        MathExpr Expr = MathExpr( Parser::StrToExpr( sExpr ) );
        bExprError = s_GlobalInvalid || Expr.IsEmpty();
        if( bExprError ) continue;
        ToRight.insert( StartCount, OutObject( sExpr, omExpress ) );
        bool bAdd = false;
        if( iStart > 0 )
          {
          QByteArray Left( Word.left( iStart ) );
          bAdd = pWord != Words.begin() && ( Left == "." || Left == "," );
          ToRight.insert( StartCount + 1, OutObject( Left, omText ) );
          }
        QByteArray Left;
        for( ; pLeft != pWord; pLeft++ )
          if( !pLeft->isEmpty() ) Left += *pLeft + ' ';
        if( !Left.isEmpty() )
          if( bAdd )
            ToRight.insert( StartCount + 2, OutObject( Left, omTextComplex ) );
          else
            ToRight.insert( StartCount + 1, OutObject( Left, omTextComplex ) );
        pLeft++;
        }
      }
    bWasHebrew = true;
    if( ToRight.count() > StartCount )
      {
      QByteArray Right;
      for( ; pLeft != Words.end(); pLeft++ )
        Right += ' ' + *pLeft;
      if( !Right.isEmpty() )
        ToRight.insert( StartCount, OutObject( Right, omTextComplex ) );
      return true;
      }
    return false;
    };

  auto FirstBracket = [&] ( PDescrMemb pIndex )
    {
    return pIndex;
    if( pIndex->m_Content.trimmed().isEmpty() ) return pIndex;
    int iFirstLen = -1;
    int iStartPos = ToRight.count();
    bool bInsert = false;
    for( PDescrMemb pCurrent = pIndex; !pCurrent->m_Content.isEmpty() && ( pCurrent->m_Kind != tXDtext ||
      ( bInsert = NoHebrew( pCurrent->m_Content ) ) ); pCurrent = pCurrent->m_pNext );
      for( PDescrMemb pCurrent = pIndex;; )
        {
        int iC = 0;
        for( ; iC < pCurrent->m_Content.length() && pCurrent->m_Content[iC] == ' '; iC++ );
        if( iC == pCurrent->m_Content.length() || pCurrent->m_Content[iC] != ')' ) return pCurrent;
        QByteArray Left = pCurrent->m_Content.mid( iC + 1, iFirstLen );
        if( !Left.isEmpty() )
          if( bInsert )
            ToRight.insert( iStartPos, OutObject( Left, NoHebrew( Left ) ? omTextLang : omTextComplex ) );
          else
            ToRight.enqueue( OutObject( Left, NoHebrew( Left ) ? omTextLang : omTextComplex ) );
        int iOpenPos = -1;
        for( pCurrent = pCurrent->m_pNext; !pCurrent->m_Content.isEmpty() &&
          ( pCurrent->m_Kind != tXDtext || ( iOpenPos = pCurrent->m_Content.indexOf( '(' ) ) == -1 ); pCurrent = pCurrent->m_pNext );
          if( iOpenPos == -1 ) return pCurrent->m_pPrev;
        int iStart = pCurrent->m_Content.lastIndexOf( ' ', iOpenPos ) + 1;
        if( NoHebrew( pCurrent->m_Content.left( iStart ) ) )
          while( iStart == iOpenPos )
            {
            pCurrent->m_Content.remove( iStart - 1, 1 );
            iStart = pCurrent->m_Content.lastIndexOf( ' ', --iOpenPos ) + 1;
            }
        bool bLastPoint = pCurrent->m_Content[iStart] == '.';
        if( bLastPoint ) iStart++;
        iC = pCurrent->m_Content.indexOf( ')' );
        if( bInsert )
//          ToRight.insert( iStartPos, OutObject( pCurrent->m_Content.mid( iStart ) + pCurrent->m_pPrev->m_Content + ')', omExpress, false ) );
          ToRight.insert( iStartPos, OutObject( pCurrent->m_Content.mid( iStart ) + pCurrent->m_pPrev->m_Content + ')', omText, false ) );
        else
          if( pCurrent->m_pPrev->m_Kind == tXDexpress )
            {
            ToRight.enqueue( OutObject( pCurrent->m_Content.mid( iStart ), omText, false ) );
            ToRight.enqueue( OutObject( pCurrent->m_pPrev->m_Content, omText, false ) );
//            ToRight.enqueue( OutObject( pCurrent->m_pPrev->m_Content, omExpress, false ) );
            ToRight.enqueue( OutObject( ")", omText, false ) );
            }
          else
            ToRight.enqueue( OutObject( pCurrent->m_Content.mid( iStart ) + pCurrent->m_pPrev->m_Content + ')', omText, false ) );
//        ToRight.enqueue( OutObject( pCurrent->m_Content.mid( iStart ) + pCurrent->m_pPrev->m_Content + ')', omExpress, false ) );
        if( iStart == 0 ) return pCurrent;
        iFirstLen = iStart - iC - 1;
        if( iC != -1 ) continue;
        if( bLastPoint )
          {
          iStart--;
          if( bInsert )
            ToRight.insert( iStartPos, OutObject( ".", omText ) );
          else
            ToRight.enqueue( OutObject( ".", omText ) );
          }
        Left = pCurrent->m_Content.left( iStart );
        ToRight.enqueue( OutObject( Left, NoHebrew( Left ) ? omTextLang : omTextComplex ) );
        return pCurrent;
        }
    };

  for( PDescrMemb pIndex = Content->m_pFirst; !pIndex.isNull(); pIndex = pIndex->m_pNext, bFirstBreak = false )
    {
    if( bWasBreak && !m_NewFormat )
      {
      PDescrMemb pLast, pCurr = pIndex;
      for( ; !pCurr.isNull() && !pCurr->m_Content.isEmpty(); pCurr = pCurr->m_pNext )
        pLast = pCurr;
      if( !pLast.isNull() && pLast->m_Content.trimmed() == "?" )
        {
        if( !CreateExpression( pLast->m_Content ) )
          ToRight.enqueue( OutObject( pLast->m_Content, NoHebrew( pLast->m_Content ) ? omText : omTextLang ) );
        pLast->m_Content = pIndex->m_Content;
        bWasBreak = false;
        continue;
        }
      }
    switch( pIndex->m_Kind )
      {
      case tXDtext:
        {
        if( Content->m_bHasEndLeft )
          {
          if(pIndex->m_Content == "ENDLEFT")
            {
            bToLeft = false;
            continue;
            }
          }
        else
         bToLeft = false;
        if( m_NewFormat )
          {
          if(bToLeft )
            ToLeft.enqueue( OutObject( pIndex->m_Content, omTextLang ) );
          else
            ToRight.enqueue( OutObject( pIndex->m_Content, omTextLang ) );
          continue;
          }
        PDescrMemb pNext = FirstBracket( pIndex );
        if( pNext != pIndex )
          {
          pIndex = pNext;
          continue;
          }
        if( CreateExpression( pIndex->m_Content ) ) continue;
        if (pIndex->m_Content.trimmed().size() < 2) continue;
        uchar C;
        int iLastChar = pIndex->m_Content.length();
        if( iLastChar == 0 ) continue;
        do
          C = pIndex->m_Content[--iLastChar];
        while( iLastChar > 0 && C == ' ' );
        if( !bExprError && ISALPHA( C ) && !NoHebrew( pIndex->m_Content ) )
          {
          int iFirstSpace = iLastChar;
          while( --iFirstSpace > 0 && ISALPHA( pIndex->m_Content[iFirstSpace] ) );
          int iLenSpace = pIndex->m_Content.length() - iLastChar - 1;
          if( iLenSpace > 0 )
            ToRight.enqueue( OutObject( QByteArray( iLenSpace, ' ' ), omText ) );
          ToRight.enqueue( OutObject( pIndex->m_Content.mid( iFirstSpace + 1, iLastChar - iFirstSpace ), omText ) );
//          ToRight.enqueue( OutObject( pIndex->m_Content.mid( iFirstSpace + 1, iLastChar - iFirstSpace ), omExpress ) );
          ToRight.enqueue( OutObject( pIndex->m_Content.left( iFirstSpace ), omTextComplex ) );
          continue;
          }
        int iC = 0;
        for( ; iC < pIndex->m_Content.length() && ( C = pIndex->m_Content[iC] ) != ':'; iC++ );
        if( C == ':' && !pIndex->m_pNext->m_Content.isEmpty() )
          {
          if( pIndex->m_pNext->m_Kind == tXDexpress || pIndex->m_pNext->m_Content[0] == ')' )
            {
            if( NoHebrew( pIndex->m_Content ) )
              {
              if( bToLeft )
                ToRight.enqueue( OutObject( ' ' + pIndex->m_Content, omTextLang ) );
              else
                ToLeft.enqueue( OutObject( ' ' + pIndex->m_Content, omTextLang ) );
              bToLeft = !bToLeft;
              }
            else
              {
              ToRight.enqueue( OutObject( ' ' + pIndex->m_Content, omTextComplex ) );
              bToLeft = true;
              }
            bColon = true;
            }
          else
            ToLeft.enqueue( OutObject( pIndex->m_Content, NoHebrew( pIndex->m_Content ) ? omTextLang : omTextComplex ) );
          continue;
          }
        QByteArray &Content = pIndex->m_Content;
        if( bToLeft )
          ToLeft.enqueue( OutObject( Content, NoHebrew( Content ) ? omTextLang : omTextComplex ) );
        else
          ToRight.enqueue( OutObject( Content, NoHebrew( Content ) ? omTextLang : omTextComplex ) );
        bWasBreak = false;
        bFirstBreak = false;
        break;
        }
      case tXDpicture:
        bWasBreak = false;
        if(bToLeft)
          ToLeft.enqueue( OutObject( pIndex->m_Content, omPict ) );
        else
          ToRight.enqueue( OutObject( pIndex->m_Content, omPict ) );
        bFirstBreak = false;
        break;
      case tXDexpress:
        {
        if( !Content->m_bHasEndLeft && pIndex->m_Content.indexOf("Picture") == -1) bToLeft = false;
        if( m_NewFormat )
          {
          if(bToLeft)
            ToLeft.enqueue( OutObject( pIndex->m_Content, omExpress ) );
          else
            ToRight.enqueue( OutObject( pIndex->m_Content, omExpress ) );
          continue;
          }
        QQueue<OutObject> &O = bToLeft ? ToLeft : ToRight;
        if( !bColon || O.isEmpty() || O.last().m_Mode == omTextComplex )
          O.enqueue( OutObject( pIndex->m_Content, omExpress ) );
        else
          O.insert( O.count() - 1, OutObject( pIndex->m_Content, omExpress ) );
        }
        bFirstBreak = false;
        bWasBreak = false;
        bColon = false;
        break;
      default:
        {
        if( ToRight.isEmpty() && ToLeft.isEmpty() ) continue;
        while( !ToRight.isEmpty() )
          PutObject( &ToRight );
        while( !ToLeft.isEmpty() )
          PutObject( &ToLeft );
        bWasBreak = true;
        if( !Content->m_bHasEndLeft ) bToLeft = false;
        if( bFirstBreak ) continue;
        pActiveCursor->insertBlock();
        bWasExspress = false;
        InsertSpace = false;
        bWasHebrew = false;
        }
      }
    }
  setModified( false );
  qDebug() << "End RichTextDocument";
  }

void RichTextDocument::Save( QByteStream& Stream )
  {
  QString Text( toPlainText() );
  if( Text.trimmed().isEmpty() ) return;
  bool bENDLEFTNotAdded = false;
  auto PutSection = [&] ( const QByteArray& Section )
    {
    for( int iNext, iTag = Section.indexOf( '<' ) + 1; iTag != -1;  )
      switch( Section[iTag] )
        {
        case 't': return -1;
        case 'p':
          iTag = Section.indexOf( '>', iTag ) + 1;
          if( Section[iTag] == '<' )
            {
            iTag++;
            continue;
            }
          iNext = Section.indexOf( '<', iTag );
          Stream << "'" << Section.mid( iTag, iNext - iTag ).replace( "&quot;", "\"" ) << "'";
          bENDLEFTNotAdded = true;
          iTag = iNext + 1;
          continue;
        case 's':
          iTag = Section.indexOf( '>', iTag ) + 1;
          iNext = Section.indexOf( "</span", iTag );
          Stream << "'" << Section.mid( iTag, iNext - iTag ).replace( "&quot;", "\"" ).replace( "&lt;", "<" ).replace( "&gt;", ">") << "'";
          bENDLEFTNotAdded = true;
          iTag = iNext + 8;
          continue;
        case 'a':
          iNext = Section.indexOf( "</a", iTag-- ) + 4;
          Stream << "'" << Section.mid( iTag, iNext - iTag ).replace( "&quot;", "\"" ).replace( "&lt;", "<" ).replace( "&gt;", ">" ) << "'";
          bENDLEFTNotAdded = true;
          iTag = iNext + 5;
          continue;
        case 'i':
          {
          bENDLEFTNotAdded = true;
          iTag = Section.indexOf( '"', iTag ) + 1;
          iNext = Section.indexOf( '"', iTag );
          QString Path = Section.mid( iTag, iNext - iTag );
          QImage Image( Path );
          QByteArray Formula = Parser::UnpackUnAscii( Image.text( "F1" ).toLatin1() );
          if( !Formula.isEmpty() )
            Stream << "EXPR(" << Formula << ")";
          else
           if(Path.indexOf("Picture(") > -1)
             Stream << "EXPR(" << Path.toLocal8Bit() << ")";
           else
            {
            Path = Path.mid( Path.lastIndexOf( '/' ) + 1 );
            Stream << "PICT(" << Path.left( Path.lastIndexOf( '.' ) ).toLatin1() << ")";
            }
          iTag = Section.indexOf( "><", iNext ) + 2;
          continue;
          }
        case '/':
          if( Section[iTag + 1] == 'p' ) return iTag + 4;
          return -iTag;
        default:
          return -1;
      }
    return -1;
    };
  
  auto NextTag = [&] ( QByteArray& Section, int& iTD  )
    {
    Section = Section.mid( abs( iTD ) );
    if( iTD < 0 )
      {
      iTD = Section.indexOf( "<td", -iTD );
      if( iTD == -1 ) return;
      iTD = Section.indexOf( "<td", iTD + 1 );
      if( iTD == -1 ) return;
      Section = Section.mid( iTD + 4 );
      }
    iTD = Section.indexOf( '<' ) + 1;
    };

  QByteArray  Left = EdStr::sm_pCodec->fromUnicode(toHtml());
  int iTD = Left.indexOf( "<td" );
  Left = Left.mid( Left.indexOf( '>', iTD ) + 1 );
  iTD = Left.indexOf( "<td" );
  QByteArray Right = Left.mid( Left.indexOf( '>', iTD ) + 1 );
  if( m_Language != lngHebrew ) Right.swap( Left );
  for( int iTDLeft = Left.indexOf( '<' ) + 1;  iTDLeft > -1; )
    {
    iTDLeft = PutSection( Left );
    NextTag( Left, iTDLeft );
    Stream << "\r\n";
    }
  if(bENDLEFTNotAdded)
    {
    Stream << "\r\nENDLEFT\r\n";
    bENDLEFTNotAdded = false;
    }
  for( int iTDRight = Right.indexOf( '<' ) + 1;  iTDRight > -1; )
    {
    iTDRight = PutSection( Right );
    NextTag( Right, iTDRight );
    Stream << "\r\n";
    }
  }

TLanguages BaseTask::GetLangByFileName( const QString& File )
  {
  return sm_FileExtensions.key( File.mid( File.lastIndexOf('.') ).toLower().trimmed() );
  }

QString BaseTask::GetFileExtByLang( TLanguages Lang )
  {
  return sm_FileExtensions[Lang];
  }

int BaseTask::Random( int MaxVal )
  {
  if( m_WorkMode == wrkExam )
    {
    int Val;
    if( m_Busy.isEmpty() ) 
      Val = ::Random( MaxVal );
    else
      {
      BusyValues Busy( m_Busy.dequeue() );
      if( Busy.count() == MaxVal )
        {
        QList<int> Selected( Busy.values() );
        if( Selected.count() == 1 )
          Val = Selected[0];
        else
          Val = Selected[::Random( MaxVal - 1 )];
        }
      else
        Val = ::Random( MaxVal );
      }
    if( !m_NewBusy.isEmpty() ) m_NewBusy += ',';
    m_NewBusy += QByteArray::number(Val);
    return Val;
    }
  if( m_Busy.isEmpty() ) return ::Random( MaxVal );
  BusyValues Busy( m_Busy.dequeue() );
  QVector<int> Values( ++MaxVal );
  for( int i = 0; i < MaxVal; i++ )
    Values[i] = i;
  for( auto pVal = Busy.begin(); pVal != Busy.end(); pVal++ )
    {
    int Val = Values.indexOf(*pVal);
    if( Val != -1) Values.remove( Val );
    }
  if( Values.size() == 1 ) return Values[0];
  int Indx = ::Random( Values.size() - 1 );
  return Values[Indx];
  }

void BaseTask::Save( QByteStream& Stream )
  {
  GetHeader();
  m_Header.Save( Stream );
  m_pTrack->Save( Stream );
  Stream << "QUESTION\r\n";
  m_pQuestion->m_bHasEndLeft = true;
  TXDescrList::Save( Stream, m_pQuestion );
  m_pCalc->Save( Stream );
  QByteArray line;
  if( m_pTrack->m_MultyTrack )
    {
    QByteArray SelectedTrack = "TRACK" + QByteArray::number( m_pTrack->m_SelectedTrack );
    if( m_pFile->isOpen() )
      {
      m_pFile->reset();
      do
        line = m_pFile->readLine();
      while( !m_pFile->atEnd() && line.indexOf( "TRACK1" ) != 0 && line.indexOf( "METHOD" ) != 0 );
      if( m_pFile->atEnd() ) throw ErrParser( "Bad task file " + m_pFile->fileName(), ParserErr::peNewErr );
      }
    if( line.indexOf( "TRACK1" ) != 0 )
      {
      while( !m_pFile->atEnd() && ( line = m_pFile->readLine() ).indexOf( SelectedTrack ) != 0 )
        Stream << line;
      }
    else
      while( !m_pFile->atEnd() && line.indexOf( SelectedTrack ) != 0 )
        {
        Stream << line;
        line = m_pFile->readLine();
        }
  Stream << SelectedTrack << "\r\n";
  }
  Stream << "METHOD\r\n";
  TXDescrList::Save( Stream, m_pMethodL );
  Stream << "COMMENT '" << GetComment() << "'\r\n";
  m_pStepsL->Save( Stream );
  if( !m_pTrack->m_MultyTrack || !m_pFile->isOpen() ) return;
  while( !m_pFile->atEnd() && ( line = m_pFile->readLine() ).indexOf( "TRACK" ) != 0 );
  if( m_pFile->atEnd() ) return; 
  for( Stream << line; !m_pFile->atEnd(); Stream << m_pFile->readLine() );
  }

BaseTask::TaskHeader& BaseTask::GetHeader()
  {
  if (m_Name.isEmpty())
    {
    QString FileExt = GetFileExtByLang(XPInEdit::sm_Language);
    QString FileName = QFileDialog::getSaveFileName(nullptr, X_Str("LangSwitch_Title", "Task Selecting Dialog"),"", "Task file (*" + FileExt + ')');
    if (FileName.isEmpty()) return m_Header;
    m_Name = FileName.mid(FileName.lastIndexOf('/') + 1).toLocal8Bit();
    m_Name = m_Name.left(m_Name.indexOf('.'));
    m_pFile->setFileName(FileName);
    QDir::setCurrent(FileName.left(FileName.lastIndexOf('/')));
    m_NewTask = true;
    }
  m_Header.m_pName = &m_Name;
  m_Header.m_pMultiTask = &m_MultiTask;
  return m_Header;
  }

void BaseTask::TaskHeader::Save( QByteStream & Stream )
  {
  Stream << "TASK '" << *m_pName << "'\r\nNEWHEBFORMAT\r\n";
  if( m_Accuracy != 0.0 )
    Stream << "ACCURACY=" << m_Accuracy << "\r\n";
  if( m_HideMultSign )
    Stream << "HIDEMULTSIGN" << "\r\n";
  if( m_ShowMultSign )
    Stream << "SHOWMULTSIGN" << "\r\n";
  if( m_ShowUnarMinus )
    Stream << "SHOWUNARMINUS" << "\r\n";
  if( m_HideUnarMinus )
    Stream << "HIDEUNARMINUS" << "\r\n";
  if (m_ShowMinusByAddition)
    Stream << "SHOWMINUSBYADDITION" << "\r\n";
  if (m_HideMinusByAddition)
    Stream << "HIDEMINUSBYADDITION" << "\r\n";
  if( m_ShowRad )
    Stream << "ANGLERAD" << "\r\n";
  if( m_ShowDeg )
    Stream << "ANGLEDEG" << "\r\n";
  if (s_ShowMinute)
    Stream << "SHOWMINUTE" << "\r\n";
  if( *m_pMultiTask )
    Stream << "MULTITASK" << "\r\n";
  }

QByteAStream::QByteAStream() : QByteArrayList(), m_CurrentLine(0) {}
QByteAStream::~QByteAStream() {}

QByteFStream::QByteFStream( const QString& Name ) : QFile( Name )
  {
  if(!open( QIODevice::WriteOnly ) )
    throw ErrParser( "Can't create file " + Name, ParserErr::peNewErr );
  }

QByteStream& QByteFStream::operator << ( const QByteArray& Arg )
  {
  write( Arg );
  return *this;
  }

QByteStream& QByteFStream::operator << ( const char* pArg )
  {
  write( pArg );
  return *this;
  }

QByteStream& QByteFStream::operator << ( double Arg )
  {
  write( QByteArray::number( Arg ) );
  return *this;
  }

QByteStream& QByteFStream::operator >> ( QByteArray& Result )
  {
  Result = readLine();
  return *this;
  }

bool QByteFStream::reset()
  {
  return QFile::reset();
  }

bool QByteFStream::AtEnd()
  {
  return atEnd();
  }


QByteStream& QByteAStream::operator << ( const QByteArray& Arg )
  {
  AppText( Arg );
  return *this;
  }

void QByteAStream::AppText( const QByteArray& Text )
  {
  if(Text == nullptr)
    {
//    throw ErrParser( "Null Text ptr", ParserErr::peNewErr );
    QMessageBox::critical(nullptr, "Error", "Null Text ptr");
    return;
    }
  int iEndPos = Text.indexOf( "\r\n" );
  if( iEndPos == -1 )
    {
    m_Line += Text;
    return;
    }
  m_Line += Text.left( iEndPos );
  append( m_Line.replace( "@Simplify(", "@S(" ).replace( "@PolinomSmplf(", "@PS(" ).replace( "@SimplifyEquation(", "@SE(" ).replace( "@SimplifyFull(", "@SF(") );
  m_Line = Text.mid( iEndPos + 2 );
  }

QByteStream& QByteAStream::operator << ( const char* pArg )
  {
  AppText( pArg );
  return *this;
  }

QByteStream& QByteAStream::operator << ( double Arg )
  {
  m_Line += QByteArray::number( Arg );
  return *this;
  }

QByteStream& QByteAStream::operator >> ( QByteArray& Result )
  {
  Result = at( m_CurrentLine++);
  return *this;
  }

bool QByteAStream::reset()
  {
  m_CurrentLine = 0;
  return true;
  }

bool QByteAStream::AtEnd()
  {
  return m_CurrentLine >= count();
  }
