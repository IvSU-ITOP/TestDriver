#include "Algebra.h"
#include "ExpOb.h"
#include "ExpObBin.h"
#include "MathTool.h"
#include <qalgorithms.h>
#include "SolChain.h"
#include "ExpStore.h"

int s_DegPoly = 8;
bool Solver::sm_TestMode = false;
Solver* Solver::sm_TestSolvers = nullptr;
MathExpr Solver::m_OldExpr;

bool GetAnswer( const MathExpr& expr, QByteArray VarName )
  {
  bool OLdPutAnswer = s_PutAnswer;
  s_PutAnswer = true;
  if( VarName.isEmpty() )
    VarName = expr.HasUnknown();
  bool Result = !VarName.isEmpty() && EquationsSolving( expr.WriteE(), VarName ) && !s_Answer.IsEmpty();
  if( Result )
    {
    if( IsType( TLexp, s_Answer ) )
      s_Answer = CastPtr(TLexp, s_Answer )->Last()->m_Memb;
    MathExpr exLeft, exRight;
    if( s_Answer.Binar( '=', exLeft, exRight ) )
      s_Answer = exRight;
    }
  s_PutAnswer = OLdPutAnswer;
  return Result;
  }

int Factorial( int N )
  {
  if( N == 1 ) return 1;
  return N * Factorial( N - 1 );
  }

double CalcC( int N, int M )
  {
  return ( double ) Factorial( N ) / ( Factorial( M )*Factorial( N - M ) );
  }

TSubstitution::TSubstitution( const MathExpr& exi )
  {
  if( !CheckInputOfEquationsSystem( exi, m_List, false ) )
    m_Status = liNotCorrect;
  else 
    m_Status = liOK;

  bool OldExpandPower = s_ExpandPower;
  s_ExpandPower = true;
  for( int I = 0; I < m_List.count(); I++ ) 
    {
    m_Multis.Add( m_List[I] );
    m_Multis.last()->Grouping();
    }

  CheckPossibleOfMethod();
  CalculateVarXGi();
  CalculateY();
  s_ExpandPower = OldExpandPower;
  }

bool TSubstitution::CheckPossibleOfMethod()
  {
  bool Result = m_Status == liOK;
  TMultiNominal *pMulti, *pMulti1, *pMulti2;
  int I = 0;
  if( Result )
    {
    for( I = 0; I < m_Multis.count(); m_Multis[I++]->GetAllVars( m_VarsList ) );

    int CurAttr = 0xFF;
    bool Sign = false;
    for( I = 0; I < m_List.count() && !Sign; I++ )
      {
      pMulti = m_Multis[I];
      for( int J = 0; J < pMulti->count() && !Sign; J++ )
        {
        MathExpr TempX;
        int AttrX;
        Sign = ( *pMulti )[J].IsOneVar( TempX, AttrX );
        Sign = Sign && pMulti->IsVarOnly1time( TempX );
        if( Sign )
          {
          if( AttrX < CurAttr )
            {
            CurAttr = AttrX;
            m_EqNo = I;
            m_TrNo = J;
            m_VarX = TempX;
            }
          Sign = CurAttr == 0;
          }
        }
      }
    Result = !m_VarX.IsEmpty();
    }
  if( Result ) return true;
  m_Status = liNotSUBST;
  if( m_Multis.count() != 2 || m_VarsList.count() != 2 ) return false;
  pMulti1 = m_Multis[0];
  pMulti2 = m_Multis[1];
  int Count1 = pMulti1->count();
  if( !pMulti1->HasVariable( "", 1 ).isEmpty() ) Count1++;
  int Count2 = pMulti2->count();
  if( !pMulti2->HasVariable( "", 1 ).isEmpty() ) Count2++;
  if( Count1 == Count2 )
    switch( Count1 )
    {
      case 3:
        if( !pMulti1->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti1->HasVariable( m_VarsList[1], 2 ).isEmpty() &&
          !pMulti2->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[1], 2 ).isEmpty() )
          m_Status = liSquareOnly;
        break;
      case 4:
        if( !pMulti1->HasVariable( m_VarsList[0], 1 ).isEmpty() &&
          !pMulti1->HasVariable( m_VarsList[1], 1 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[0], 1 ).isEmpty() &&
          !pMulti2->HasVariable( m_VarsList[1], 1 ).isEmpty() && !pMulti1->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() &&
          !pMulti2->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() )
          m_Status = liProductAndSimple;
        else
          if( !pMulti1->HasVariable( m_VarsList[0], 2 ).isEmpty() &&
            !pMulti1->HasVariable( m_VarsList[1], 2 ).isEmpty() &&
            !pMulti2->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[1], 2 ).isEmpty() &&
            !pMulti1->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() &&
            !pMulti2->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() )
            m_Status = liSquareAndProduct;
        break;
      case 5:
        if( !pMulti1->HasVariable( m_VarsList[0], 1 ).isEmpty() && !pMulti1->HasVariable( m_VarsList[1], 1 ).isEmpty() &&
          !pMulti2->HasVariable( m_VarsList[0], 1 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[1], 1 ).isEmpty() &&
          !pMulti1->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti1->HasVariable( m_VarsList[1], 2 ).isEmpty() &&
          !pMulti2->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[1], 2 ).isEmpty() )
          m_Status = liSquareAndSimple;
        break;
      case 6:
        if( !pMulti1->HasVariable( m_VarsList[0], 1 ).isEmpty() && !pMulti1->HasVariable( m_VarsList[1], 1 ).isEmpty() &&
          !pMulti2->HasVariable( m_VarsList[0], 1 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[1], 1 ).isEmpty() &&
          !pMulti1->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti1->HasVariable( m_VarsList[1], 2 ).isEmpty() &&
          !pMulti2->HasVariable( m_VarsList[0], 2 ).isEmpty() && !pMulti2->HasVariable( m_VarsList[1], 2 ).isEmpty() &&
          !pMulti1->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() &&
          !pMulti2->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() )
          m_Status = liSquareAndSimpleAndProduct;
    }
  else
    {
    if( !m_VarsList.indexOf( "x", I ) || !m_VarsList.indexOf( "y", I ) ) return false;
    m_EqNo = 1;
    if( Count1 < Count2 )
      {
      pMulti = pMulti1;
      pMulti1 = pMulti2;
      pMulti2 = pMulti;
      Count1 = Count2;
      m_EqNo = 0;
      }
    if( Count1 == 4 && pMulti2->count() == 2 && !pMulti1->HasVariable( m_VarsList[0], 2 ).isEmpty() &&
      !pMulti1->HasVariable( m_VarsList[1], 2 ).isEmpty() && !pMulti1->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() &&
      !pMulti2->HasProduct( m_VarsList[0], m_VarsList[1], 1, 1 ).isEmpty() )
      m_Status = liSquareAndProduct1;
    }
  return false;
  }

void TSubstitution::CalculateVarXGi()
  {
  TMultiNominal *pMulti;
  int J;
  MathExpr TempX;
  if( m_Status == liOK )
    {
    pMulti = m_Multis[m_EqNo];
    m_VarXGi = Constant( 0 );
    for( int J = 0; J < pMulti->count(); J++ )
      if( J != m_TrNo ) m_VarXGi -= ( *pMulti )[J].FullTerm();
    m_VarXGi = ReduceTExprs( m_VarXGi );
    TempX = ( *pMulti )[m_TrNo].FullTermWithout( m_VarX );
    m_VarXGi /= TempX;
    TempX = ReduceTExprs( m_VarXGi );
    m_VarXGi = ExpandExpr( TempX );
    }
  }

void TSubstitution::CalculateY()
  {
  if( m_Status != liOK ) return;
  for( int I = 0; I < m_List.count(); I++ )
    if( I != m_EqNo )
      {
      TMultiNominal *pMulti = m_Multis[I];
      pMulti->ChangeVar( m_VarX, m_VarXGi );
      MathExpr Temp1 = pMulti->FullExpr();
      Temp1 = ReduceTExprs( Temp1 );
      MathExpr Temp2 = ExpandExpr( Temp1 );
      Temp1 = m_List[I];
      m_List[I] = Temp2;
      }
  }

bool TSubstitution::Run()
  {
  bool Result = m_List.count() == 2 && m_VarsList.count() == 2;
  if( !Result ) 
    {
    m_Status = liNot1Decision;
    return false;
    }
  if( m_Status != liOK )
    {
    m_Status = liOK;
    return true;
    }
    m_Status = liNoFinish;
    for( int I = 0; I < m_List.count(); I++ )
      if( I != m_EqNo )
        {
        QByteArray VarName = m_List[I].HasUnknown();
        if( VarName.isEmpty() )
          {
          m_Roots.append( Variable( "x" ) );
          continue;
          }
        MathExpr Var = Variable( VarName );
        MathExpr VariableX = Variable( "x" );
        MathExpr TempX = m_List[I];
        TempX.Replace( Var, VariableX );
        TempX = new TBinar( '=', TempX, Constant( 0 ) );
        MathExpArray  _RootList;
        bool OldCalcOnly = s_CalcOnly;
        s_CalcOnly = true;
        TempX.MakeBiOrSq( _RootList );
        s_CalcOnly = OldCalcOnly;
        for( int J = 0; J < _RootList.count(); J++ ) MakeRoot( _RootList[J] );
        Result = _RootList.count()>0;
        if( !Result )
          {
          TempX = new TBinar( '=', m_List[I], Constant( 0 ) );
          if( GetAnswer( TempX, VarName ) )
            {
            m_Roots.append( s_Answer );
            m_Status = liEquationSolved;
            Result = true;
            if( m_VarsList[0] == m_VarX.WriteE() )
              m_VarsList[1] = VarName;
            else
            m_VarsList[0] = VarName;
            m_Status = liOK;
            }
          return Result;
          }
        }
  if( Result ) m_Status = liOK;
  return Result;
  }

void TSubstitution::MakeRoot( const MathExpr& root )
  {
  m_Roots.append( ExpandExpr( root.Reduce() ) );
  }

MathExpr TSubstitution::GetVarY()
  {
  if( m_VarsList.count() > 1 )
    {
    if( m_VarsList[0] == m_VarX.WriteE() ) return Variable( m_VarsList[1] );
    return Variable( m_VarsList[0] );
    }
  return MathExpr();
  }

MathExpr TSubstitution::RootX( int I )
  {
  try
    {
    QByteArray VarName = m_VarXGi.HasUnknown();
    MathExpr Result = m_VarXGi;
    if( VarName.isEmpty() ) return m_VarXGi;
    if( m_Roots[I].WriteE() == "x" ) return m_VarXGi;
    Result = m_VarXGi.Clone();
    Result.Replace( Variable( VarName ), m_Roots[I] );
    return ExpandExpr( ReduceTExprs( Result ) );
    }
  catch( ErrParser )
    {
    }
  return MathExpr();
  }

MathExpr TSimItem::MakeExchange()
  {
  MathExpr Result;
  if( EqDegrees() )
    {
    MathExpr TempC = Constant( m_DegreeX );
    MathExpr TempB = new TVariable( true, "v" );
    Result = MakePower( TempB, TempC );
    }
  else
    if( m_DegreeX == 1 )
      Result = new TVariable( true, "u" );
    else
      if( m_DegreeY > 0 )
        {
        TSimItem NewSItem1( m_DegreeY, m_DegreeY, Constant( 1 ) );
        TSimItem NewSItem2( m_DegreeX - m_DegreeY, 0, Constant( 1 ) );
        Result = NewSItem1.MakeExchange() * NewSItem2.MakeExchange();
        }
      else
        {
        MathExpr TempC = Constant( m_DegreeX );
        MathExpr TempB = new TVariable( true, "u" );
        Result = MakePower( TempB, TempC );
        for( int I = m_DegreeX - 1; I >= ( m_DegreeX + 1 ) / 2; I-- )
          {
          TSimItem NewSItem1( I, m_DegreeX - I, Constant( CalcC( m_DegreeX, I ) ) );
          Result -= NewSItem1.MakeExchange();
          Result = ReduceTExprs( Result );
          }
        }
  Result = m_Coeff * Result;
  MathExpr TempB = ReduceTExprs( Result );
  Result = ExpandExpr( TempB );
  return ReduceTExprs( Result );
  }

void TSimInfo::Duplicates()
  {
  for( int I = 0; I < count() - 1; I++ )
    for( int J = I + 1; J < count(); )
      if( (*this)[I].m_DegreeX == (*this)[J].m_DegreeX && (*this)[I].m_DegreeY == (*this)[J].m_DegreeY )
        {
        (*this)[I].m_Coeff += (*this)[J].m_Coeff;
        (*this)[I].m_Coeff = ReduceTExprs( (*this)[I].m_Coeff );
        remove( J );
        }
      else
        J++;
  }

bool TSimInfo::Check()
  {
  bool Result = true;
  for( int I = 0; I < count(); I++ )
    if( !( *this )[I].m_IsCheck )
      {
      bool Sign = false;
      int J = I + 1;
      for( ; J < count() && !Sign; J++ )
        Sign = Sign ||
          !(*this)[J].m_IsCheck && (*this)[I].m_DegreeX == (*this)[J].m_DegreeY && (*this)[I].m_DegreeY == (*this)[J].m_DegreeX &&
          IsEqualExprs( (*this)[I].m_Coeff, (*this)[J].m_Coeff );
      if( Sign )
        {
        (*this)[I].m_IsCheck = true;
        (*this)[J - 1].m_IsCheck = true;
        }
      Result = Result && Sign;
      }
  if( Result ) DeleteDuplicates();
  return Result;
  }

void TSimInfo::DeleteDuplicates()
  {
  for( int I = 0; I < count() - 1; I++ )
    if( ( *this )[I].m_DegreeX != ( *this )[I].m_DegreeY )
      {
      int J = I;
      bool Sign;
      do
        {
        J++;
        Sign = ( *this )[I].IsSim( ( *this )[J] );
        } while( J < count() && !Sign );
        if( Sign )
          remove( J );
      }

  for( int I = 0; I < count(); I++ )
    if( ( *this )[I].m_DegreeX < ( *this )[I].m_DegreeY )
      {
      int Degree = ( *this )[I].m_DegreeX;
      ( *this )[I].m_DegreeX = ( *this )[I].m_DegreeY;
      ( *this )[I].m_DegreeY = Degree;
      }
  }

MathExpr TSimInfo::MakeExchange()
  {
  MathExpr Result = Constant(0);
  for( int I = 0; I < count(); I++ )
    {
    Result += (*this)[I].MakeExchange();
    Result = ReduceTExprs( Result );
    }
  return Result;
  }

TExchange::TExchange( const MathExpr& exi )
  {
  m_DenomList = new TLexp();
  if( !CheckInputOfEquationsSystem( exi, m_List, false, m_DenomList ) ) 
    m_Status = liNotCorrect;
  else
    m_Status = liOK;
  for( int I = 0; I < m_List.count(); I++ ) m_Multis.append( new TMultiNominal( m_List[I] ) );
  CheckPossibleOfMethod();
  }

bool TExchange::CheckPossibleOfMethod()
  {
  if( m_Status != liOK )
    {
    m_Status = liNotEXCHANGE;
    return false;
    }
  bool Result = true;
  for( int I = 0; I < m_Multis.count(); I++ )
    m_Multis[I]->GetAllVars( m_VarsList );
  Result = Result && m_VarsList.count() == 2 && m_Multis.count() == 2;
    if( Result )
      {
      PascArray<MathExpr> exP( 1, 4 );
      m_VarX = m_VarsList[0];
      m_VarY = m_VarsList[1];
      PExMemb memMemb = m_DenomList.First();
      if( memMemb.isNull() )
        Result = CheckEquation( *m_Multis[0], m_SimInfo1 ) && CheckEquation( *m_Multis[1], m_SimInfo2 );
      else 
        {
        for( int I = 1; I <= 4; I++ ) 
          {
          exP[I] = memMemb->m_Memb;
          memMemb = memMemb->m_pNext;
          if( memMemb.isNull() ) 
            {
            Result = I == 4;
            break;
            }
          }
        Result = Result && ( memMemb.isNull() ) && !exP[1].Equal( exP[2] );
        MathExpr exQ, exQ2, exPower, exPower2;
        if( Result ) 
          {
          if( exP[1].Equal( exP[3] ) ) 
            {
            Result = exP[2].Equal( exP[4] );
            if( Result ) m_Status = liDenomsPQPQ;
            }
          else
            if( exP[1].Equal( exP[4] ) ) 
              {
              Result = exP[2].Equal( exP[3] );
              if( Result ) m_Status = liDenomsPQPQ;
              }
            else 
              {
              Result = false;
              if( exP[1].Power( exQ, exPower ) )
                Result = exP[2].Power( exQ2, exPower2 ) &&
                ( exQ.Equal( exP[3] ) && exQ2.Equal( exP[4] ) || exQ.Equal( exP[4] ) && exQ2.Equal( exP[3] ) );
              else
              if( exP[3].Power( exQ, exPower ) )
                Result = exP[4].Power( exQ2, exPower2 ) &&
                ( exQ.Equal( exP[1] ) && exQ2.Equal( exP[2] ) || exQ.Equal( exP[2] ) && exQ2.Equal( exP[1] ) );
              Result = Result && exPower2 == 2;
              if( Result ) m_Status = liDenomsPQP2Q2;
              }
            if( m_Status == liDenomsPQPQ && exP[1].Power( exQ, exPower ) )
              m_Status = liDenomsP2Q2P2Q2;
            Result = Result && ( exPower.IsEmpty() || exPower == 2 );
          }
        }
      }
  if( !Result ) m_Status = liNotEXCHANGE;
  return Result;
  }

bool TExchange::CheckEquation( const TMultiNominal& multi, TSimInfo& SimInfo )
  {
  for( int I = 0; I < multi.count(); I++ )
    {
    int DegX = 0;
    int DegY = 0;
    MathExpr Coeff = Constant( 1 );
    for( int J = 0; J < multi[I].count(); J++ )
      {
      if( multi[I][J].m_Base.WriteE() == m_VarX )
        DegX = multi[I][J].GetPwrInt();
      else
        {
        if( multi[I][J].m_Base.WriteE() == m_VarY )
          DegY = multi[I][J].GetPwrInt();
        else
          Coeff *= multi[I][J].RetMulti();
        }
      }
    Coeff = multi[I].m_Coefficient * Coeff;
    Coeff = ReduceTExprs( Coeff );
    Coeff = NegTExprs( Coeff, multi[I].m_Sign == -1 );
    Coeff = ReduceTExprs( Coeff );
    SimInfo.append( TSimItem( DegX, DegY, Coeff ) );
    }
  SimInfo.Duplicates();
  return SimInfo.Check();
  }

MathExpr TExchange::GetExprXpY()
  {
  return Variable( m_VarX, true ) *  Variable( m_VarY, true );
  }

MathExpr TExchange::GetExprXsY()
  {
  return Variable( m_VarX, true ) +  Variable( m_VarY, true );
  }

MathExpr TExchange::GetNewVarU()
  {
  return Variable( "u", true );
  }

MathExpr TExchange::GetNewVarV()
  {
  return Variable( "v", true );
  }

bool TExchange::Run()
  {
  if( m_Status != liOK ) return false;
  m_ExchEqu1 = m_SimInfo1.MakeExchange();
  m_ExchEqu2 = m_SimInfo2.MakeExchange();
  return true;
  }

MathExpr TExchange::GetExchEqu1()
  {
  return new TBinar( '=', m_ExchEqu1, Constant( 0 ) );
  }

MathExpr TExchange::GetExchEqu2()
  {
  return new TBinar( '=', m_ExchEqu2, Constant( 0 ) );
  }

void TExchange::InsertRoot( const QByteArray& Name1, const MathExpr& Root1, const MathExpr& Root2 )
  {
  if( Name1 == "u" )
    {
    m_RootsU.append( Root1 );
    m_RootsV.append( Root2 );
    return;
    }
  m_RootsU.append( Root2 );
  m_RootsV.append( Root1 );
  }

void TExchange::CalculateRoot( int I )
  {
  try
    {
    MathExpr RootU = m_RootsU[I];
    MathExpr RootV = m_RootsV[I];
    MathExpr x = Variable( "x", true );
    MathExpr SearchY = x ^ 2;
    SearchY -= RootU * x;
    SearchY += RootV;
    SearchY = new TBinar( '=', SearchY, Constant( 0 ) );
    SearchY = ReduceTExprs( SearchY );
    MathExpArray  _RootList;
    SearchY.MakeBiOrSq( _RootList );
    for( int J = 0; J < _RootList.count(); J++ )
      NewRoot( RootU, _RootList[J] );
    }
  catch( ErrParser )
    {
    }
  }

void TExchange::NewRoot( const MathExpr& RootU, const MathExpr& RootY )
  {
  MathExpr TempY = RootY.Reduce();
  MathExpr TempX = ReduceTExprs( RootU - TempY );
  m_RootsX.append( TempX );
  m_RootsY.append( TempY );
  }

void TExchange::CalculateFinish()
  {
  for( int I = 0; I < m_RootsU.count(); I++ ) CalculateRoot( I );
  }

MathExpr TExchange::GetPrintRoot( int I )
  {
  MathExpr TempX = new TBinar( '=', Variable( m_VarX + '_' + NumberToStr( I + 1 ), true ), m_RootsX[I] );
  MathExpr TempY = new TBinar( '=', Variable( m_VarY + '_' + NumberToStr( I + 1 ), true ), m_RootsY[I] );
  Lexp Result = new TLexp;
  Result.Addexp( TempX );
  Result.Addexp( TempY );
  return Result;
  }

bool IsThereThisName(PNode Expr, const QByteArray& Name )
  {
  bool Result = false;
  std::function<void(PNode )> PreOrder = [&](PNode p )
    {
    if(p== nullptr ) return;
    if(p->m_Info == Name )
      throw  1;
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };
  try
    {
    PreOrder( Expr );
    }
  catch( int )
    {
    Result = true;
    }
  return Result;
  }

PNode BinOp(PNode op1, PNode op2, char OpSgn, const QByteArray& Sign, uchar Prior )
  {
  PNode Result = new TNode( op1->m_pOwner );
  Result->m_pLeft = op1;
  op1->m_pParent = Result;
  Result->m_IsLeft = op1->m_IsLeft || op1->m_IsRight;
  Result->m_pRight = op2;
  op2->m_pParent = Result;
  Result->m_IsRight = op2->m_IsLeft || op2->m_IsRight;
  Result->m_OpSign = OpSgn;
  Result->m_Info = Sign;
  Result->m_pParent = nullptr;
  Result->m_Priority = Prior;
  return Result;
  }

PNode UnarOp( PNode op1, char OpSgn, const QByteArray& Sign, uchar Prior )
  {
  PNode Result = new TNode( op1->m_pOwner );
  Result->m_pRight = op1;
  op1->m_pParent = Result;
  Result->m_IsRight = op1->m_IsLeft || op1->m_IsRight;
  Result->m_pLeft = nullptr;
  Result->m_IsLeft = false;
  Result->m_OpSign = OpSgn;
  Result->m_Info = Sign;
  Result->m_pParent = nullptr;
  Result->m_Priority = Prior;
  return Result;
  }

PNode Add( PNode Addend1, PNode Addend2 )
  {
  return BinOp( Addend1, Addend2, '+', "+", 4 );
  }

PNode Sub( PNode Reduced, PNode Subtracted )
  {
  return BinOp( Reduced, Subtracted, '-', "-", 4 );
  }

PNode Mult( PNode Mult1, PNode Mult2 )
  {
  return BinOp( Mult1, Mult2, '*', "*", 3 );
  }

PNode Divis(PNode Dividend, PNode Divider )
  {
  return BinOp( Dividend, Divider, '/', "/", 3 );
  }

PNode Degree(PNode basis, PNode exponent )
  {
  return BinOp( basis, exponent, '^', "^", 1 );
  }

PNode UnMinus(PNode Expr )
  {
  return UnarOp( Expr, 'u', "-", 2 );
  }

PNode Func( const QByteArray& Name, PNode Expr )
  {
  return UnarOp( Expr, 'F', Name, 0 );
  }

PNode Clone(PNode P ) 
  { 
  PNode pResult = nullptr;
  if(P == nullptr ) return pResult;
  pResult = new TNode(P->m_pOwner );
  *pResult = *P;
  pResult->m_pLeft = Clone(P->m_pLeft );
  pResult->m_pRight = Clone(P->m_pRight );
  return pResult;
  }

PNode NewNode(Parser *P, const QByteArray& Name )
  {
  PNode pResult = new TNode(P);
  pResult->m_Info = Name;
  pResult->m_OpSign = 'v';
  return pResult;
  }

MathExpr FreeMember(PNode P, const QByteArray& v = "x" )
  {
  if(P == nullptr ) return nullptr;

  MathExpr op11, op22;
  int Degree;
  int Int, Nom, Den;
  double d;
  Lexp List;
  PExMemb f;

  MathExpr op1 = FreeMember(P->m_pLeft, v );
  MathExpr op2 = FreeMember(P->m_pRight, v );
  switch(P->m_OpSign )
    {
    case ',':
      if( op2.Listex( f ) )
        List = Lexp( op2 );
      else
        List.Addexp( op2 );
      List.Addexp( op1 );
      return List;
    case '+': return ( op1 + op2 ).Reduce();
    case '-': return ( op1 - op2 ).Reduce();
    case '*':
    case msMultSign2:
      return ( op1 * op2 ).Reduce();
    case '/':
      if( op2.Reduce().Constan( d ) && ( abs( d ) < 0.0000001 ) )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      return ( op1 / op2 ).Reduce();
    case '^': return ( op1^op2 ).Reduce();
    case '~':
      if( op2.Cons_int( Degree ) )
        {
        if( op1.Negative() )
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        return op1.Root( Degree ).Reduce();
        }
      return MathExpr( new TRoot1( op1, op2 ) ).Reduce();
    case '=': return MathExpr( new TBinar( '=', op1, op2 ) );
    case 'F':
      if(P->m_Info == "log" )
        {
        op2.Listex( f );
        return MathExpr( new TLog( f->m_Memb, f->m_pNext->m_Memb ) ).Reduce();
        }
      return Function(P->m_Info, op2 ).Reduce();
    case 'u': return ( -op2 ).Reduce();
    case 'i':
    case 'f':
      return Constant(P->m_Info.toDouble() );
    case 'd': return MathExpr( new TDegExpr(P->m_Info ) );
    case 'v':
      if(P->m_Info.toLower() == v.toLower() )
        return Constant( 0 );
      return Variable(P->m_Info );
    case  'M':
      Int = P->m_Info.toInt();
      if( op2.Divis( op11, op22 ) )
        {
        op11.Cons_int( Nom );
        op22.Cons_int( Den );
        }
      else
        op2.SimpleFrac_( Nom, Den );
      return MathExpr( new TMixedFrac( Int, Nom, Den ) ).Reduce();
    default:
      throw  ErrParser( "Wrong operation!", peOpertn );
    }
  }

MathExpr CreateExprPoly( MathExpArray &Arr, int n, const QByteArray& Name )
  {
  MathExpr Result;
  switch( n )
    {
    case 0:
      Result = Arr[0];
      break;
    case 1:
      Result = ( Arr[1] * Variable( Name ) ).Reduce();
      break;
    default:
      Result = ( Arr[n] * ( Variable( Name ) ^ n ) ).Reduce();
    }

  double r;
  MathExpr op1;
  for( int i = --n; i >= 2; i-- )
    if( !( Arr[i].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) )
      if( !( Arr[i].Unarminus( op1 ) || Arr[i].Constan( r ) && r < 0 ) )
        Result = Result + ( Arr[i] * ( Variable( Name ) ^ i ) ).Reduce();
      else
        {
        if( Arr[i].Constan( r ) ) op1 = Constant( abs( r ) );
        Result = Result - ( op1 * ( Variable( Name ) ^ i ) ).Reduce();
        }
  if( n >= 1 )
    if( !( Arr[1].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) )
      if( !( Arr[1].Unarminus( op1 ) || Arr[1].Constan( r ) && r < 0 ) )
        Result = Result + ( Arr[1] * Variable( Name ) ).Reduce();
      else
        {
        if( Arr[1].Constan( r ) ) op1 = Constant( abs( r ) );
        Result = Result - ( op1 * Variable( Name ) ).Reduce();
        }
  if( n >= 0 )
    if( !( Arr[0].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) )
      if( !( Arr[0].Unarminus( op1 ) || Arr[0].Constan( r ) && r < 0 ) )
        Result = Result + Arr[0];
      else
        {
        if( Arr[0].Constan( r ) ) op1 = Constant( abs( r ) );
        Result = Result - op1;
        }
  return Result;
  }

bool ReduceMeasure( MathExpr& ex )
  {
  PNode eq;
  Parser P;
  eq = P.GetMeasure( ex.WriteE() );
  bool Result = ReduceFactor( eq, ex );
  return Result;
  }

bool MultDiv(PNode eq )
  {
  bool Done;
  std::function<void(PNode )> PreOrder = [&] (PNode p )
    {
    if( p == nullptr ) return;
    PNode L, R;
    if( p->m_OpSign == '*' )
      {
      L = p->m_pLeft; R = p->m_pRight;
      if( L->m_OpSign == '/' && R->m_OpSign == '/' )
        {
        p->m_pLeft = Mult( L->m_pLeft, R->m_pLeft );
        p->m_pRight = Mult( L->m_pRight, R->m_pRight );
        p->m_pLeft->m_pParent = p;
        p->m_pRight->m_pParent = p;
        p->m_OpSign = '/';
        p->m_Info = "/";
        p->m_IsLeft = R->m_IsLeft || L->m_IsLeft;
        p->m_IsRight = R->m_IsRight || L->m_IsRight;
        Done = true;
        }
      else
        if( L->m_OpSign == '/' || R->m_OpSign == '/' )
          {
          if( L->m_OpSign == '/' )
            {
            p->m_pLeft = Mult( L->m_pLeft, R );
            p->m_pRight = L->m_pRight;
            p->m_IsLeft = R->m_IsLeft || R->m_IsRight || L->m_IsLeft;
            p->m_IsRight = L->m_IsRight;
            }
          else 
            {
            p->m_pLeft = Mult( L, R->m_pLeft );
            p->m_pRight = R->m_pRight;
            p->m_IsLeft = R->m_IsLeft || L->m_IsLeft || L->m_IsRight;
            p->m_IsRight = R->m_IsRight;
            }
          p->m_pLeft->m_pParent = p;
          p->m_pRight->m_pParent = p;
          p->m_OpSign = '/';
          p->m_Info = "/";
          Done = true;
          }
      }
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };

  bool Result = false;
  do
    {
    Done = false;
    PreOrder( eq );
    Result = Result || Done;
    } while( Done );
    return Result;
  }

bool Div2(PNode eq )
  {
  bool Done;
  std::function<void(PNode )> PreOrder = [&] (PNode p )
    {
    PNode D, R;
    if(p == nullptr ) return;
    if( p->m_OpSign == '/' ) 
      {
      R = p->m_pRight;
      if( R->m_OpSign == '/' )
        {
        D = R->m_pLeft;
        R->m_pLeft = R->m_pRight;
        R->m_pRight = D;
        p->m_OpSign = '*';
        p->m_Info = "*";
        Done = true;
        }
      }
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };
  
  bool Result = false;
  do
    {
    Done = false;
    PreOrder( eq );
    Result = Result || Done;
    } while( Done );
    return Result;
  }

bool OpenBrackets(PNode eq )
  {
  bool Done;
  std::function<void(PNode )> PreOrder = [&] (PNode p )
    {
    if(p == nullptr ) return;
    int Open;
    char sign;
    PNode L, R, U, D;
    bool IsName;
    if(p->m_OpSign == '*' )
      {
      L = p->m_pLeft;
      R = p->m_pRight;
      if( TestFrac( R ) || TestFrac( L ) )
        {
        if( R->m_OpSign == '+' || R->m_OpSign == '-' )
          {
          U = R; D = L; IsName = p->m_IsLeft; sign = R->m_OpSign; Open = 1;
          }
        else
          if( L->m_OpSign == '+' || L->m_OpSign == '-' )
            {
            U = L; D = R; IsName = p->m_IsRight; sign = L->m_OpSign; Open = 2;
            }
          else
            {
            U = nullptr; D = nullptr; IsName = false; sign = ' '; Open = 0;
            }
          if( Open > 0 )
            {
            switch( Open ) 
              {
              case 1: 
                p->m_pLeft = Mult( Clone( D ), U->m_pLeft );
                p->m_pRight = Mult( Clone( D ), U->m_pRight );
                break;
              case 2: 
                p->m_pLeft = Mult( U->m_pLeft, Clone( D ) );
                p->m_pRight = Mult( U->m_pRight, Clone( D ) );
              }
            p->m_pLeft->m_pParent = p;
            p->m_pRight->m_pParent = p;
            p->m_OpSign = sign;
            p->m_Info = QByteArray( 1, sign );
            p->m_IsLeft = IsName || U->m_IsLeft;
            p->m_IsRight = IsName || U->m_IsRight;
            p->m_Priority = 4;
            Done = true;
            }
        }
      }
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };
  bool Result = false;
  do
    {
    Done = false;
    PreOrder( eq );
    Result = Result || Done;
    } while( Done );
    return Result;
  }

void FindDivi(PNode Expr, TDivis& Divis )
  {
  auto AddDen = [] (PNode& p, PNode& q )
    {
    PNode  p1, p2;
    p1 = p; p2 = nullptr;
    while(p1 != nullptr && (p1->m_OpSign == '*' || p1->m_OpSign == '/' ) )
      {
      if(p1->m_OpSign == '/' )
        {
        q = Mult( q, p1->m_pRight );
        if(p2 == nullptr )
          p = p1->m_pLeft;
        else 
          {
          p2 = p1;
          p2->m_pLeft = p1->m_pLeft;
          }
        }
      p1 = p1->m_pLeft;
      }
    };

  auto Test = [&] (PNode& p )
    {
    if(p == nullptr ) return;
    PNode q, q1;
    if(p->m_OpSign == '/' )
      {
      Divis.append( TDiv(p->m_pLeft, p->m_pRight ) );
      AddDen(p->m_pLeft, p->m_pRight );
      Divis.last().m_DenDepX = p->m_IsRight;
      q1 = p;
      q = p->m_pParent;
      while( q != nullptr && q->m_OpSign == '*' )
        {
        if( q->m_pLeft == q1 )
          {
          Divis.last().m_Nom = Mult( Divis.last().m_Nom, q->m_pRight );
          q->m_pRight = nullptr;
          }
        else 
          {
          Divis.last().m_Nom = Mult( Divis.last().m_Nom, q->m_pLeft );
          q->m_pLeft = nullptr;
          }
        q1 = q;
        q = q->m_pParent;
        }
      while( q != nullptr && q->m_OpSign == '+' )
        {
        q1 = q;
        q = q->m_pParent;
        }
      if( q != nullptr )
        switch( q->m_OpSign )
          {
          case '+':
            Divis.last().m_Sign = '+';
            break;
          case 'u':
            Divis.last().m_Sign = '-';
            break;
          case '-':
            if( q->m_pLeft == q1 )
              Divis.last().m_Sign = '+';
            else
              Divis.last().m_Sign = '-';
          }
      p = nullptr;
      }
    };

  std::function<void(PNode )> PreOrder = [&](PNode p )
    {
    if(p== nullptr ) return;
    if(p->m_IsLeft ) Test(p->m_pLeft );
    if(p->m_IsRight ) Test(p->m_pRight );
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };

  std::function<PNode(PNode )> PostOrder = [&](PNode p )
    {
    PNode Result = nullptr;
    if(p== nullptr ) return Result;
    p->m_pLeft = PostOrder(p->m_pLeft );
    p->m_pRight = PostOrder(p->m_pRight );
    switch(p->m_OpSign )
      {
      case '+':
      case '*':
        if(p->m_pLeft == nullptr )
          Result = p->m_pRight;
        else
          if(p->m_pRight == nullptr )
            Result = p->m_pLeft;
      else
      Result = p;
        break;
      case '-':  
        if( (p->m_pLeft == nullptr ) )
          {
          if(p->m_pRight != nullptr )
            Result = UnMinus(p->m_pRight );
          else
            Result = nullptr;
          }
        else
          if(p->m_pRight == nullptr )
            Result = p->m_pLeft;
        else
        Result = p;
        break;
      case '/': 
        Result = p;
        break;
      case 'u':  
        if(p->m_pRight == nullptr )
          Result = nullptr;
      else
      Result = p;
        break;
      default:
      Result = p;
      }
    return Result;
    };

  Test( Expr );
  PreOrder( Expr );
  Divis[0].m_Nom = PostOrder( Expr );
  }

QByteArrayList GetFactors(PNode eq) 
  {
  QByteArrayList List;

  std::function<void(PNode )> PostOrder = [&](PNode p )
    {
    if(p == nullptr )
      return;
    PostOrder(p->m_pLeft );
    PostOrder(p->m_pRight );
    if(p->m_OpSign == 'v' )
      List.append(p->m_Info );
    else
      if( (p->m_OpSign == 'f' || p->m_OpSign == 'i') && p->m_Info != "1" )
        List.append(p->m_Info );
    };

  PostOrder( eq );
  qSort(List);
  return List;
  }

MathExpr SubtCube( const MathExpr& exi )
  {
  bool WasReduced = false;
  std::function<MathExpr( const MathExpr& )> Sqrt3 = [&] ( const MathExpr& ex )
    {
    MathExpr ex1, ex2;
    if( ex.Multp( ex1, ex2 ) ) return Sqrt3( ex1 ) * Sqrt3( ex2 );
    ex1 = ex.Root( 3 );
    MathExpr Result = ex1.Reduce();
    WasReduced = WasReduced || !Result.Eq( ex1 );
    return Result;
    };

  MathExpr Result;
  bool OldNoRootReduce = s_NoRootReduce;
  try
    {
    MathExpr exi1, exi2;
    if( exi.Subtr( exi1, exi2 ) )
      {
      s_NoRootReduce = true;
      MathExpr sq1 = Sqrt3( exi1 ), sq2;
      if( WasReduced )
        {
        WasReduced = false;
        sq2 = Sqrt3( exi2 );
        }
      if( WasReduced )
        {
        MathExpr exFirst = (sq1^Constant( 3 )) - (sq2^Constant( 3 ));
        MathExpr Mult2 = ( sq1 * sq2 ).Reduce();
        if( exi.Equal( exFirst ) )
          Result = new TBinar( '=', exi, ( sq1 - sq2 ) * ( ( sq1^Constant( 2 ) ).Reduce() + Mult2 + ( sq2^Constant( 2 ) ).Reduce() ) );
        else
          Result = new TBinar( '=', exi, new TBinar( '=', exFirst, ( sq1 - sq2 ) *
          ( ( sq1^Constant( 2 ) ).Reduce() + Mult2 + ( sq2^Constant( 2 ) ).Reduce() ) ) );
        s_XPStatus.SetMessage( X_Str( "MFactorized", "Factorized!" ) );
        }
      }
    }
  catch( ErrParser E )
    {
    }
  if( !WasReduced )
    {
    s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
    s_GlobalInvalid = true;
    Result = exi;
    }
  s_NoRootReduce = OldNoRootReduce;
  return Result;
  }

MathExpr SummCube( const MathExpr& exi )
  {
  bool WasReduced = false;
  std::function<MathExpr( const MathExpr& )> Sqrt3 = [&] ( const MathExpr& ex )
    {
    MathExpr ex1, ex2;
    if( ex.Multp( ex1, ex2 ) ) return Sqrt3( ex1 ) * Sqrt3( ex2 );
    ex1 = ex.Root( 3 );
    MathExpr Result = ex1.Reduce();
    WasReduced = WasReduced || !Result.Eq( ex1 );
    return Result;
    };

  bool OldNoRootReduce = s_NoRootReduce;
  bool OldRootToPower = s_RootToPower;
  MathExpr Result;
  try
    {
    MathExpr exi1, exi2;
    if( exi.Summa( exi1, exi2 ) )
      {
      s_NoRootReduce = true;
      s_RootToPower = false;
      MathExpr sq1 = Sqrt3( exi1 ), sq2;
      if( WasReduced )
        {
        WasReduced = false;
        sq2 = Sqrt3( exi2 );
        }
      if( WasReduced )
        {
        MathExpr exFirst = ( sq1^ 3 ) + ( sq2^ 3 );
        MathExpr Mult2 = ( sq1 * sq2 ).Reduce();
        if( exi.Equal( exFirst ) )
          Result = new TBinar( '=', exi, ( sq1 + sq2 ) *
          ( ( sq1^2 ).Reduce() - Mult2 + ( sq2 ^ 2 ).Reduce() ) );
        else
          Result = new TBinar( '=', exi, new TBinar( '=', exFirst, ( ( sq1 + sq2 ) *
          ( ( sq1 ^ 2 ).Reduce() - Mult2 + ( sq2 ^ 2 ).Reduce() ) ) ) );
        s_XPStatus.SetMessage( X_Str( "MFactorized", "Factorized!" ));
        }
      }
    }
  catch( ErrParser E )
    {
    }
  s_NoRootReduce = OldNoRootReduce;
  s_RootToPower = OldRootToPower;
  if( !WasReduced )
    {
    s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
    s_GlobalInvalid = true;
    Result = exi;
    }
  return Result;
  }

bool ReduceFactor(PNode eq, MathExpr& ex )
  {
  QByteArrayList FList;
  int FCount;
  struct Line
    {
    PNode Node; int Count;
    Line() {}
    };
  QVector<Line> F( 11 );

  QByteArray name;
  bool IsReduce;

  auto FracDegNode = [&] (PNode p, int& nom, int& den )
    {
    if(p == nullptr )
      return false;

    PNode q = p->m_pParent;
    if( q == nullptr)
      return false;

    if( q->m_OpSign == '^' )
      {
      if( q->m_pRight->m_OpSign == 'i' )
        {
        nom = q->m_pRight->m_Info.toInt();
        den = 1;
        return true;
        }
      else if( q->m_pRight->m_OpSign == '/' && q->m_pRight->m_pLeft->m_OpSign == 'i' && q->m_pRight->m_pRight->m_OpSign == 'i' )
        {
        nom = q->m_pRight->m_pLeft->m_Info.toInt();
        den = q->m_pRight->m_pRight->m_Info.toInt();
        return true;
        }
      else
        return false;
      }
    else
      if( QVector < char > {'*', '/', 'u'}.contains( q->m_OpSign ) )
        {
        nom = 1;
        den = 1;
        return true;
        }
    return false;
    };

  auto One = [&] (PNode p )
    {
    p->m_pLeft = nullptr;
    p->m_IsLeft = false;
    p->m_pRight = nullptr;
    p->m_IsRight = false;
    p->m_OpSign = 'i';
    p->m_Info = "1";
    p->m_Priority = 0;
    };

  auto DegToOne = [&] (PNode p, int Deg )
    {
    PNode q;
    if( Deg > 1 )
      {
      q = p->m_pParent;
      q->m_pRight = nullptr;
      One( q );
      }
    else
      One(p );
    };

  auto GenFrac = [&] ( int N, int D )
    {
    PNode pnom( new TNode( eq->m_pOwner ) );
    One(pnom );
    pnom->m_Info = QByteArray::number( N );

    PNode pden( new TNode( eq->m_pOwner ) );
    One(pden );
    pden->m_Info = QByteArray::number( D );

    PNode Result( new TNode( eq->m_pOwner ) );
    Result->m_pLeft = pnom;
    Result->m_IsLeft = false;
    Result->m_pRight = pden;
    Result->m_IsRight = false;
    Result->m_OpSign = '/';
    Result->m_Info = "/";
    Result->m_Priority = 3;
    return Result;
    };

  auto Reduce = [&] (PNode p1, PNode p2 )
    {
    int nom1, den1, nom2, den2, nom, den;
    PNode p;
    if( !( FracDegNode(p1, nom1, den1 ) && FracDegNode(p2, nom2, den2 ) ) )
      return;
    nom = nom1*den2 - nom2*den1;
    den = den1*den2;
    if( nom > 0 )
      {
      if( nom != den )
        p1->m_pParent->m_pRight = GenFrac( nom, den );
      else
        {
        p = p1->m_pParent->m_pRight;
        p1->m_pParent = p1;
        p1  = nullptr;
        p = nullptr;
        }
      DegToOne(p2, nom2*den2 );
      }
    else
      if( nom < 0 )
        {
        if( -nom != den )
          p2->m_pParent->m_pRight = GenFrac( -nom, den );
        else
          {
          p = p2->m_pParent->m_pRight;
          p2->m_pParent = p2;
          p2 = nullptr;
          p = nullptr;
          }
        DegToOne(p1, nom1*den1 );
        }
      else
        {
        DegToOne(p1, nom1*den1 );
        DegToOne(p2, nom2*den2 );
        }
    IsReduce = true;
    };

  auto Mult = [&] (PNode p1, PNode p2 )
    {
    int nom1, den1, nom2, den2;
    if( !( FracDegNode(p1, nom1, den1 ) && FracDegNode(p2, nom2, den2 ) ) )
      return;
    if(p1->m_pParent->m_OpSign == '^' )
      p1->m_pParent->m_pRight = GenFrac( nom1*den2 + nom2*den1, den1*den2 );
    else
      {
      p1->m_pLeft = Clone(p1 );
      p1->m_pLeft->m_pParent = p1;
      p1->m_pRight = GenFrac( nom1*den2 + nom2*den1, den1*den2 );
      p1->m_pRight->m_pParent = p1;
      p1->m_Info = "^";
      p1->m_OpSign = '^';
      p1->m_Priority = 1;
      }
    One(p2 );
    IsReduce = true;
    };

  std::function<void(PNode, int )> Test = [&] (PNode p, int n )
    {
    if(p == nullptr )
      return;
    switch(p->m_OpSign )
      {
      case '*':
        Test(p->m_pLeft, n );
        Test(p->m_pRight, n );
        break;
      case '^':
        Test(p->m_pLeft, n );
        break;
      case '/':
        Test(p->m_pLeft, n );
        Test(p->m_pRight, n + 1 );
        break;
      case 'v':
      case 'F':
      case 'i':
        if(p->m_Info == name )
          {
          FCount++;
          F[FCount].Node = p;
          F[FCount].Count = n;
          }
      }
    };

  std::function<void(PNode )> PostOrder = [&] (PNode p )
    {
    int j, m, k;
    bool odd1;
    if(p == nullptr )
      return;

    PostOrder(p->m_pLeft );
    PostOrder(p->m_pRight );

    if(p->m_OpSign == '*' || p->m_OpSign == '/' )
      {
      FCount = 0;
      Test(p, 0 );
      while( FCount > 1 )
        {
        odd1 = F[1].Count & 1;
        j = 2;
        if( j <= FCount )
          {
          if( odd1 == ( F[j].Count & 1 ) )
            Mult( F[1].Node, F[j].Node );
          else
            Reduce( F[1].Node, F[j].Node );
          FCount--;
          m = 1;
          for( k = 1; k <= FCount; k++ )
            {
            if( m == j )
              m++;
            F[k] = F[m];
            m++;
            }
          }
        else
          FCount = 0;
        }
      }
    };

  bool Result = false;

  FList = GetFactors( eq );
  for( int i = 0; i < FList.length(); i++ )
    {
    name = FList[i];
    do
      {
      IsReduce = false;
      PostOrder( eq );
      Result = Result || IsReduce;
      } while( IsReduce ); //until (!IsReduce)
    }
  ex = eq->m_pOwner->OutPut( eq ).Reduce();
  return Result;
  }

MathExpr TExpr::ReductionPoly( MathExpArray& Arr, const QByteArray& Name ) const
  {
  int PlusMinusCount = 0, PlusMinusOld = 0;

  auto OpenBrackets = [&] (PNode eq )
    {
    bool Done;
    std::function<void(PNode )> PreOrder = [&] (PNode p )
      {
      int Open;
      char sign;
      PNode L, R, U, D;
      bool IsName;
      if(p == nullptr ) return;
      if(p->m_OpSign == '*' )
        {
        L = p->m_pLeft;
        R = p->m_pRight;
        if( R->m_OpSign == '^' && R->m_pLeft->m_OpSign == 'v' && R->m_pLeft->m_Info == "x" &&
          R->m_pRight->m_OpSign == '/' ) throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        if( R->m_OpSign == '+' || R->m_OpSign == '-' )
          {
          U = R; D = L; IsName = p->m_IsLeft; sign = R->m_OpSign; Open = 1;
          PlusMinusCount++;
          }
        else
          if( L->m_OpSign == '+' || L->m_OpSign == '-' )
            {
            U = L; D = R; IsName = p->m_IsRight; sign = L->m_OpSign; Open = 2;
            PlusMinusCount++;
            }
          else
            {
            U = nullptr; D = nullptr; IsName = false; sign = ' '; Open = 0;
            }
        if( Open > 0 )
          {
          switch( Open )
            {
            case 1:
              p->m_pLeft = Mult( ::Clone( D ), U->m_pLeft );
              p->m_pRight = Mult( ::Clone( D ), U->m_pRight );
              break;
            case 2:
              p->m_pLeft = Mult( U->m_pLeft, ::Clone( D ) );
              p->m_pRight = Mult( U->m_pRight, ::Clone( D ) );
            }
          p->m_pLeft->m_pParent = p;
          p->m_pRight->m_pParent = p;
          p->m_OpSign = sign;
          p->m_Info = QByteArray( 1, sign );
          p->m_IsLeft = IsName || U->m_IsLeft;
          p->m_IsRight = IsName || U->m_IsRight;
          p->m_Priority = 4;
          Done = true;
          }
        }
      if(p->m_OpSign == '^' && p->m_pRight->m_OpSign == '/' )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      PreOrder(p->m_pLeft );
      PreOrder(p->m_pRight );
      };
    bool  Result = false;
    do
      {
      Done = false;
      PreOrder( eq );
      Result = Result || Done;
      } while( Done );
      return Result;
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  MathExpr Result;
  try
    {
    Parser P;
    MathExpr ex(Clone());
    PNode eq = P.GetExpression( WriteE() );
    OpenBrackets( eq );
    Arr.push_back( FreeMember( eq, Name ) );
    PlusMinusOld = 0;
    s_TruePolynom = true;
    for( int i = 1; i < s_DegPoly; i++ )
      {
      ex = Expand( ex.Diff( Name ).Reduce() / Constant( i ) ).Reduce();
      eq = P.GetExpression( ex.WriteE() );
      PlusMinusCount = 0;
      OpenBrackets( eq );
      if(PlusMinusOld == 0 )
        PlusMinusOld = PlusMinusCount;
      else
        if(PlusMinusCount > PlusMinusOld + 3 )
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      Arr.push_back( FreeMember( eq, Name ) );
      }
    Arr.push_back( ex.Diff( Name ).Reduce() );
    int n = s_DegPoly;
    double r;
//    for( int i = 0; i < Arr.count(); i++ )
//      qDebug() << Arr[i].WriteE();
    for( ; n > 0 && Arr[n].Reduce().Constan( r ) && abs( r ) < 0.0000001; n-- );
    Result = CreateExprPoly( Arr, n, Name ).Reduce(true);
    }
  catch( ErrParser Error )
    {
    s_Precision = OldPrecision;
    s_TruePolynom = false;
    throw Error;
    }
  s_Precision = OldPrecision;
  s_TruePolynom = false;
  return Result;
  }

bool ReduceExpr( MathExpr& ex )
  {
  Parser P;
  return ReduceFactor(P.GetExpression( ex.WriteE() ), ex );
  }

MathExpr RightExpr( const MathExpr& E )
  {
  MathExpr expLeft, expRight;
  if( E.Binar( '=', expLeft, expRight ) )
    return RightExpr( expRight );
  return E;
  }

MathExpr RemDenominator( MathExpr ex, Lexp& Cond )
  {
  const char SetBinaries[] = { '+', '-', '=', '<', '>', (char) msMaxequal, (char) msMinequal, (char) msNotequal, 0 };

  std::function<bool( MathExpr, Lexp& )> SearchFractions = [&] ( MathExpr exp, Lexp& List )
    {
    MathExpr exL, exR;
    char cSign;
    if( !exp.Oper_( cSign, exL, exR ) )
      {
      List.Addexp( exp );
      return false;
      }

    if( In( cSign, SetBinaries ) )
      {
      bool Result = SearchFractions( exL, List );
      bool bRes2 = SearchFractions( exR, List );
      return Result || bRes2;
      }

    bool Result = exp.Divis( exL, exR ) && exp.AsFraction();
    int N, D;
    if( Result )
      {
      if( !Cond.IsEmpty() && !exR.HasUnknown().isEmpty() ) Cond.Addexp( exR );
      }
    else
      if( exp.Multp( exL, exR ) )
        if( exL.SimpleFrac_( N, D ) )
          {
          if( N == 1 )
            exp = exR / Constant( D );
          else
            exp = Constant( N ) * exR / Constant( D );
          Result = true;
          }
        else  if( exR.SimpleFrac_( N, D ) )
          {
          if( N == 1 )
            exp = exL / Constant( D );
          else
            exp = Constant( N ) * exL / Constant( D );
          Result = true;
          }
    List.Addexp( exp );
    return Result;
    };

  MathExpr Result = ex.Clone();
  MathExpr *pLeft;
  MathExpr *pRight;
  std::function<MathExpr( MathExpr&, PExMemb& )> GetNumerators = [&] ( MathExpr& ex, PExMemb& exMemb )
    {
    MathExpr exL, exR;
    char cSign;
    int N, D;
    if( !ex.Oper_( cSign, exL, exR ) )
      {
      if( exMemb->m_Memb.SimpleFrac_( N, D ) ) return Constant( N );
      exMemb->m_Memb.Divis( exL, exR );
      exMemb = exMemb->m_pNext;
      return exL;
      }

    if( In( cSign, SetBinaries ) )
      {
      pLeft = &CastPtr( TOper, ex )->Left();
      MathExpr New = GetNumerators( exL, exMemb );
      if( pLeft == nullptr ) return ex;
      ( *pLeft ) = New;
      pLeft = nullptr;
      pRight = &CastPtr( TOper, ex )->Right();
      New = GetNumerators( exR, exMemb );
      ( *pRight ) = New;
      ex.SetReduced( false );
      return ex;
      }

    exMemb->m_Memb.Divis( exL, exR );
    exMemb = exMemb->m_pNext;
    return exL;
    };

  Lexp lstList( new TLexp ), lstL;
  if( !SearchFractions( ex, lstList ) ) return Result;
  lstL = lstList.LeastCommonDenominator( false );
  GetNumerators( ex, lstL.First() );
  return ex;
  }

MathExpr RemDenominator( MathExpr ex )
  {
  Lexp Cond = Lexp();
  return RemDenominator(ex, Cond);
  }

MathExpr TExpr::ReToMult()
  {
  MathExpr LCM;
  MathExpArray Frac;
  std::function<MathExpr( const MathExpr& )> ReplaceFrac = [&] ( const MathExpr& ex )
    {
    MathExpr ex1, ex2, op1, op2;
    int N, D;
    double v;
    char c;
    QByteArray name;
    if( ex.SimpleFrac_( N, D ) || ex.Constan( v ) || ex.Variab( name ) )
      return ( ex * LCM ).Reduce();
    if( ex.Oper_( c, op1, op2 ) )
      {
      if( In( c, "+-*^" ) )
        {
        ex1 = ReplaceFrac( op1 );
        ex2 = ReplaceFrac( op2 );
        switch( c )
          {
          case '+': return ex1 + ex2;
          case '-': return ex1 - ex2;
          case '*': return ( LCM * ex ).Reduce();
          case '^': return ( LCM * ex ).Reduce();
          }
        }
      else
        return MathExpr(ex);
      }
    else
      if( ex.Unarminus( op1 ) )
        return -ReplaceFrac( op1 );
    return MathExpr(ex);
    };

  auto FindFrac = [&] ( const MathExpr& ex )
    {
    std::function<void( const MathExpr& )> PreOrder = [&] ( const MathExpr& ex )
      {
      MathExpr op1, op2;
      int N, D;
      double v;
      char c;
      if( ex.SimpleFrac_( N, D ) || ( ex.Constan( v ) && !ex.Cons_int( N ) ) )
        Frac.append( ex );
      else
        if( ex.Oper_( c, op1, op2 ) && In( c, "+-*" ) )
          {
          PreOrder( op1 );
          PreOrder( op2 );
          }
        else
          if( ex.Unarminus( op1 ) )
            PreOrder( op1 );
      };
    Frac.clear();
    PreOrder( ex );
    return Frac.count() > 0;
    };

  auto DenList = [&] ()
    {
    int N, D, RP;
    double v = s_Precision;
    MathExpr ex;
    RP = 1;
    while( v < 1 )
      {
      v *= 10;
      RP *= 10;
      }
    TLexp *pResult = new TLexp;
    MathExpr Result(pResult );
    for( int i = 0; i < Frac.count(); i++ )
      if( Frac[i].SimpleFrac_( N, D ) )
        pResult->Addexp( Constant( D ) );
      else
        {
        Frac[i].Constan( v );
        ex = ( Constant( RP*v ) / Constant( RP ) ).Reduce();
        ex.SimpleFrac_( N, D );
        pResult->Addexp( Constant( D ) );
        }
    return Result;
    };

  std::function<MathExpr( const MathExpr&, bool )> GetExponent = [&] ( const MathExpr& exp, bool AllOper )
    {
    MathExpr exLeft, exRight, exArg;
    QByteArray sName;
    char cOper;
    MathExpr Result;
    if( exp.Funct( sName, exArg ) && sName == "exp" )
      return MathExpr( exp );
    if( exp.Oper_( cOper, exLeft, exRight ) )
      {
      if( ( cOper != '*' ) && !AllOper ) return Result;
      Result = GetExponent( exLeft, false );
      if( Result.IsEmpty() )
        Result = GetExponent( exRight, false );
      }
    return Result;
    };

  auto CreateMultExp = [&] ( const MathExpr& exExp )
    {
    MathExpr  exTmp;
    s_GlobalInvalid = false;
    MathExpr Arg;
    QByteArray Name;
    exExp.Funct( Name, Arg );
    exTmp = Function( "exp", -Arg ) * this;
    return exExp * ExpandExpr( exTmp );
    };

  MathExpr ex3;
  if( FindFrac( this ) )
    {
    LCM = FindLeastCommMult();
    ex3 = ReplaceFrac( this ).Reduce();
    }
  else
    {
    LCM.Clear();
    ex3 = this;
    }
  MathExpr exi_R = ex3.ReduceToMult();
  MathExpr op1, op2;
  if( exi_R.Multp( op1, op2 ) && ( IsConstType( TSumm, op2 ) || IsConstType( TSubt, op2 ) ) && !( IsConstType( TFunc, op1 ) ) &&
    !GetExponent( op2, true ).IsEmpty() )
    {
    MathExpr P = op2.ReToMult();
    if( IsConstType( TMult, P ) )
      return op1 * P;
    }
  MathExpr ex1 = ex3.Reduce();
  MathExpr ex2 = exi_R.Reduce();
  MathExpr Result;
  if( s_GlobalInvalid || exi_R.Eq( ex3 ) )
    {
    if( Subtr( op1, op2 ) )
      {
      s_GlobalInvalid = false;
      Result = SubtSq( this );
      if( !s_GlobalInvalid ) return Result;
      s_GlobalInvalid = false;
      Result = SubtCube( this );
      if( !s_GlobalInvalid ) return Result;
      ex1 = GetExponent( op1, false );
      if( !ex1.IsEmpty() )
        {
        ex2 = GetExponent( op2, false );
        if( !ex2.IsEmpty() )
          {
          s_GlobalInvalid = false;
          return ex1 * ( ( op1 / ex1 ).Reduce() - ( op2 / ex1 ).Reduce() );
          }
        else
          return CreateMultExp( ex1 );
        }
      else
        {
        ex2 = GetExponent( op2, false );
        if( !ex2.IsEmpty() )
          return CreateMultExp( ex2 );
        if( ( IsConstType( TDivi, op1 ) ) || ( IsConstType( TDivi, op2 ) ) )
          {
          MathExpr P = CalcMulti( 2, this, false );
          if( !P.IsEmpty() )
            {
            Result = new TBinar( '=', this, P.ReToMult() );
            s_GlobalInvalid = false;
            return Result;
            }
          }
        }
      }
    else
      if( Summa( op1, op2 ) )
        {
        s_GlobalInvalid = false;
        Result = SummCube( this );
        if( !s_GlobalInvalid ) return Result;
        ex1 = GetExponent( op1, false );
        if( !ex1.IsEmpty() )
          {
          ex2 = GetExponent( op2, false );
          if( !ex2.IsEmpty() )
            {
            s_GlobalInvalid = false;
            return ex1 * ( ( op1 / ex1 ).Reduce() + ( op2 / ex1 ).Reduce() );
            }
          else
            return CreateMultExp( ex1 );
          }
        else
          {
          ex2 = GetExponent( op2, false );
          if( !ex2.IsEmpty() )
            return CreateMultExp( ex2 );
          if( ( IsConstType( TDivi, op1 ) ) || ( IsConstType( TDivi, op2 ) ) )
            {
            MathExpr P = CalcMulti( 1, this, false );
            if( !P.IsEmpty() )
              {
              Result = new TBinar( '=', this, P.ReToMult() );
              s_GlobalInvalid = false;
              return Result;
              }
            }
          }
        }
    s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
    if( !LCM.IsEmpty() )
      {
      int n;
      LCM.Cons_int( n );
      exi_R *= MathExpr( new TSimpleFrac( 1, n ) );
      }
    s_GlobalInvalid = true;
    return exi_R;
    }

  MathExpr P = exi_R.ReduceToMult();
  for( int iCount = 0; !(P.Ptr()->Equal( exi_R ) || s_GlobalInvalid || iCount == 10 ); iCount++ )
    {
    exi_R = P;
    P = P.ReduceToMult();
    }

  s_GlobalInvalid = false;
  if( !LCM.IsEmpty() )
    {
    int n;
    LCM.Cons_int( n );
    exi_R *= MathExpr( new TSimpleFrac( 1, n ) );
    }
  s_XPStatus.SetMessage( X_Str( "MFactorized", "Factorized!" ) );
  return new TBinar( '=', this, exi_R );
  }

MathExpr Transfer( MathExpr exp )
  {
  MathExpr exLeft, exRight, exSumm, exL, exR, exLExpanded, exRExpanded;
  int iV, iRight;
  bool bHasUnknown, bExprChanged;
  int iPlusMinusCount, iPMCountLOld;
  uchar cRelSign;

  std::function<void( char, MathExpr& )> Form = [&] ( char cPrevSign, MathExpr& exTerm )
    {
    char cSign;
    MathExpr exL, exR, ex, exNew;
    int iVal;
    if( exTerm.Oper_( cSign, exL, exR ) )
      {
      if( cSign == '+' )
        {
        Form( cPrevSign, exL );
        Form( cPrevSign, exR );
        return;
        }
      if( cSign == '-' )
        {
        Form( cPrevSign, exL );
        if( cPrevSign == '+' )
          Form( '-', exR );
        else
          Form( '+', exR );
        return;
        }
      }

    bHasUnknown = !exTerm.HasUnknown().isEmpty();
    if( bHasUnknown )
      exNew = exLeft;
    else
      {
      if( cPrevSign == '+' )
        cPrevSign = '-';
      else
        cPrevSign = '+';
      exNew = exRight;
      }

    if( exNew.IsEmpty() )
      {
      if( cPrevSign == '+' )
        exNew = exTerm;
      else
        if( exTerm.Unarminus( ex ) )
          exNew = ex;
        else
          exNew = -exTerm;
      }
    else
      if( !exTerm.Cons_int( iVal ) || iVal != 0 )
        if( cPrevSign == '+' )
          if( exTerm.Unarminus( ex ) )
            exNew -= ex;
          else
            exNew += exTerm;
        else
          if( exTerm.Unarminus( ex ) )
            exNew += ex;
          else
            exNew -= exTerm;

    if( bHasUnknown )
      exLeft = exNew;
    else
      exRight = exNew;
    };

  std::function<bool( MathExpr&, int& )> ReduceMultipliers = [&] ( MathExpr& exp, int& PlusMinusCount )
    {
    MathExpr exL, exR, ex;
    char cSign;
    if( !exp.Oper_( cSign, exL, exR ) ) return false;
    if( cSign == '+' || cSign == '-' )
      {
      PlusMinusCount++;
      if( ReduceMultipliers( exL, PlusMinusCount ) )
        {
        MathExpr Left = CastPtr( TOper, exp )->Left();
        ex = Left.Reduce();
        exp.Replace( Left, ex );
        }
      if( ReduceMultipliers( exR, PlusMinusCount ) )
        {
        MathExpr Right = CastPtr( TOper, exp )->Right();
        ex = Right.Reduce();
        exp.Replace( Right, ex );
        }
      return false;
      }
    return cSign == '*';
    };

  if( !exp.Binar_( cRelSign, exL, exR ) || exp.HasUnknown().isEmpty() )
    throw  ErrParser( "Syntax error!", peSyntacs );
  if( !exR.Cons_int( iRight ) || iRight != 0 )
    {
    exL -= exR;
    exR = Constant( 0 );
    exp = new TBinar( cRelSign, exL, exR );
    }
  iPMCountLOld = 0;
  ReduceMultipliers( exL, iPMCountLOld );
  if( iPMCountLOld == 0 ) return exp;
  int iPMCountROld = 0;
  do
    {
    bExprChanged = false;
    exLExpanded = Expand( exL );
    iPlusMinusCount = 0;
    ReduceMultipliers( exLExpanded, iPlusMinusCount );
    if( !exLExpanded.Eq( exL ) && iPlusMinusCount >= iPMCountLOld )
      {
      bExprChanged = true;
      exL = exLExpanded;
      iPMCountLOld = iPlusMinusCount;
      }
    exRExpanded = Expand( exR );
    iPlusMinusCount = 0;
    ReduceMultipliers( exRExpanded, iPlusMinusCount );
    if( !exRExpanded.Eq( exR ) && iPlusMinusCount >= iPMCountROld )
      {
      bExprChanged = true;
      exR = exRExpanded;
      iPMCountROld = iPlusMinusCount;
      }
    if( bExprChanged )
      TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( cRelSign, exLExpanded, exRExpanded ) );
    } while( bExprChanged );
    exSumm = exL - exR;
    exLeft.Clear();
    exRight.Clear();
    Form( '+', exSumm );
    if( exLeft.IsEmpty() || exRight.IsEmpty() ) return exp;
    return new TBinar( cRelSign, exLeft, exRight );
  }
  
bool IsFuncEqu(PNode Expr, const QByteArray& Excepted )
  {
  std::function<void(PNode )> PreOrder = [&] (PNode p )
    {
    if(p == nullptr ) return;
    if(p->m_OpSign == 'F' &&  p->m_Info != Excepted  && p->m_IsRight ) throw 0;
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };

  try
    {
    PreOrder( Expr );
    }
  catch( int ix )
    {
    return true;
    }
  return false;
  }

bool TestFrac(PNode Expr )
  {
  std::function<void(PNode )> PreOrder = [&] (PNode p )
    {
    if(p == nullptr ) return;
    if(p->m_OpSign == '/' && p->m_IsRight ) throw  1;
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };
  try
    {
    PreOrder( Expr );
    }
  catch( int ix )
    {
    return true;
    }
  return false;
  }

bool IsExpEqu(PNode Expr )
  {
  std::function<void(PNode )> PreOrder = [&] (PNode p )
    {
    if(p == nullptr ) return;
    if(p->m_OpSign == '^' && p->m_IsRight ) throw 1;
    PreOrder(p->m_pLeft );
    PreOrder(p->m_pRight );
    };
  try
    {
    PreOrder( Expr );
    }
  catch( int ix )
    {
    return true;
    }
  return false;
  }

MathExpr DelSplitted( const MathExpr& ex )
  {
  char c;
  MathExpr op1, op2, ex1, ex2;
  if( ex.Oper_( c, op1, op2 ) )
    {
    if( op1.Newline() ) return DelSplitted( op2 );
    if( op2.Newline() ) return DelSplitted( op1 );
    ex1 = DelSplitted( op1 );
    ex2 = DelSplitted( op2 );
    switch( c )
      {
      case  '+': return ex1 + ex2;
      case  '-': return ex1 - ex2;
      case  '*':
      case msMultSign2: return ex1 * ex2;
      case  '/':
      case ':': return ex1 / ex2;
      case  '^':
      case  '~': return ex1 ^ ex2;
      default:
        return new TBinar( c, ex1, ex2 );
      }
    }
  if( ex.Unarminus( op1 ) ) return DelSplitted( op1 );
  return ex;
  }

Lexp SolutionSimpleEquaion(PNode equation, const QByteArray& SelectName )
  {
  if( equation->m_IsLeft && equation->m_IsRight )
    throw  ErrParser( "Too many unknowns!", peMultipl );

  enum TUnknownSide { UnkLeft, UnkRight } UnknownSide;
  PNode LeftPart, RightPart;
  if( equation->m_IsLeft )
    {
    LeftPart = equation->m_pLeft;
    RightPart = equation->m_pRight;
    }
  else
    {
    LeftPart = equation->m_pRight;
    RightPart = equation->m_pLeft;
    }

  int IterCount = 0;
  PNode Addition;
  PNode Sol = new TNode( equation->m_pOwner );
  Sol->m_Info = "=";
  Sol->m_OpSign = '=';
  Sol->m_Priority = 5;
  MathExpr Expr1;
  Lexp Result = new TL2exp;
  Parser &P = *equation->m_pOwner;
  while( LeftPart->m_Info.toUpper() != SelectName.toUpper() )
    {
    IterCount++;
    if( LeftPart->m_IsLeft && LeftPart->m_IsRight )
      throw  ErrParser( "Too many unknowns!", peMultipl );

    if( LeftPart->m_IsLeft )
      {
      UnknownSide = UnkLeft;
      Addition = LeftPart->m_pRight;
      }
    else
      {
      UnknownSide = UnkRight;
      Addition = LeftPart->m_pLeft;
      }

    switch( LeftPart->m_OpSign )
      {
      case '+': RightPart = Sub( RightPart, Addition ); break;
      case '*':
      case msMultSign2: RightPart = Divis( RightPart, Addition );  break;
      case '^':
        switch( UnknownSide )
          {
          case UnkLeft: RightPart = BinOp( RightPart, Addition, '~', "~", 1 );  break;
          case UnkRight: throw  ErrParser( "Wrong operation!", peOpertn );
          }
        break;
      case '~':
        switch( UnknownSide )
          {
          case UnkLeft: RightPart = BinOp( RightPart, Addition, '^', "^", 1 ); break;
          case UnkRight: throw  ErrParser( "Wrong operation!", peOpertn );
          }
        break;
      case '-':
        switch( UnknownSide )
          {
          case UnkLeft: RightPart = Add( RightPart, Addition );  break;
          case UnkRight: RightPart = Sub( Addition, RightPart ); break;
          }
        break;
      case '/': switch( UnknownSide )
        {
        case UnkLeft: RightPart = Mult( RightPart, Addition ); break;
        case UnkRight: RightPart = Divis( Addition, RightPart ); break;
        }
                break;
      case 'u': RightPart = UnMinus( RightPart ); break;
      case 'F':
        if( LeftPart->m_Info == "sin" )
          {
          RightPart = Func( "arcsinM", RightPart ); 
          break;
          }
        if( LeftPart->m_Info == "cos" )
          {
          RightPart = Func( "arccosM", RightPart ); 
          break;
          }
        if( LeftPart->m_Info == "tan" || LeftPart->m_Info == "tg" )
          {
          RightPart = Func( "arctanM", RightPart ); 
          break;
          }
        if( LeftPart->m_Info == "cot" || LeftPart->m_Info == "ctg" )
          {
          RightPart = Func( "arccotM", RightPart ); 
          break;
          }
        if( LeftPart->m_Info == "log" )
          {
          LeftPart = LeftPart->m_pRight;
          RightPart = Degree( LeftPart->m_pRight, RightPart );
          UnknownSide = UnkLeft;
          break;
          }
        else
          throw  ErrParser( "Wrong operation!", peOpertn );
      default:
        throw  ErrParser( "Wrong operation!", peOpertn );
      }

    PNode DelNode = LeftPart;
    switch( UnknownSide )
      {
      case UnkLeft: LeftPart = LeftPart->m_pLeft; break;
      case UnkRight: LeftPart = LeftPart->m_pRight;
      }
    Sol->m_pLeft = LeftPart;
    Sol->m_pRight = RightPart;
    if( IterCount == 1 )
      Expr1 = P.OutPut( Sol );
    else
      Expr1 = P.OutPut( Sol ).Reduce();
    Result.Addexp( Expr1 );
    }

  if( equation->m_IsLeft )
    equation->m_pLeft = nullptr;
  else
    equation->m_pRight = nullptr;

  if( !Expr1.IsEmpty() )
    {
    MathExpr Expr2 = Expr1.Reduce();
    if( !Expr1.Eq( Expr2 ) )
      Result.Addexp( Expr2 );
    }
  else
    {
    Sol->m_pLeft = LeftPart;
    Sol->m_pRight = RightPart;
    Result.Addexp(P.OutPut( Sol ).Reduce() );
    }
  return Result;
  }

Lexp CalcDetLinEqu( const QByteArray& Source, const QByteArray& VarName )
  {
  if( Source.isEmpty() ) return new TL2exp();

  auto PutConditions = [&] ( Lexp& Conditions, const MathExpr& exResult )
    {
    for(PExMemb Condition = Conditions.First(); !Condition.isNull(); Condition = Condition->m_pNext )
      {
      MathExpr ex = Condition->m_Memb.Substitute( VarName, exResult );
      if( ex.Eq( Condition->m_Memb ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
      else
        {
        MathExpr exOut = new TBinar( '=', Condition->m_Memb, ex );
        MathExpr ex1 = ex.Reduce();
        if( !( IsConstType( TBool, ex1 ) ) )
          if( ex1 == 0 )
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', exOut, ex1 ), X_Str( "MRootStranger", "false root!" ) );
          else
            {
            TSolutionChain::sm_SolutionChain.AddExpr( exOut );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, ex1, Constant( 0 ) ) );
            }
        }
      }
    };

  auto PutCondForInfiniteSol = [&] ( Lexp& Conditions )
    {
    for(PExMemb Condition = Conditions.First(); !Condition.isNull(); Condition = Condition->m_pNext )
      {
      MathExpr ex = Transfer( new TBinar( '=', Condition->m_Memb.Reduce(), Constant( 0 ) ) );
      MathExpr exL, exR, exLeft, exRight, exCoeff;
      ex.Binar( '=', exLeft, exRight );
      if( exLeft.Multp( exL, exR ) )
        {
        if( !exL.HasUnknown().isEmpty() )
          exCoeff = exR;
        else
          exCoeff = exL;
        exRight /= exCoeff.Reduce();
        exLeft /= exCoeff.Reduce();
        ex = new TBinar( msNotequal, exLeft, exRight );
        }
      else
        CastPtr( TBinar, ex )->SetName( msNotequal );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  int OldDegPoly = s_DegPoly;
  s_DegPoly = 3;
  Lexp Result;

  auto Final = [&] ()
    {
    if( s_FinalComment && !Result.IsEmpty() )
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsE", "Roots are found" ) );
    s_Precision = OldPrecision;
    s_DegPoly = OldDegPoly;
    if( s_PutAnswer && s_Answer.IsEmpty() && !Result.IsEmpty() )
      s_Answer = Result;
    return Result;
    };

  try
    {
    bool IsName, MultUnknown, Ratio;
    TExprs p;
    Lexp Cond = new TLexp;
    Parser P( &Cond );
    PNode eq = P.Equation( Source, VarName, IsName, MultUnknown, true ), q;
    if( !( IsFuncEqu( eq ) || IsExpEqu( eq ) ) ) //{ Whether contains the equation }
      {
      MathExpr expr = Parser::StrToExpr( Source ), ex1, ex2;
      if( !expr.Binar( '=', ex1, ex2 ) || !ex2.ConstExpr() && !ex2.IsLinear() || !ex1.ConstExpr() && !ex1.IsLinear() )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      if( IsType( TVariable, ex1 ) && IsType( TConstant, ex2 ) )
        {
        Result = new TL2exp;
        Result.Addexp( ex2 );
        return Final();
        }
      TSolutionChain::sm_SolutionChain.AddExpr( expr );
      s_PowerToFraction = true;
      MathExpr ex = expr.Reduce();
      s_PowerToFraction = false;
      if( !ex.Eq( expr ) )
        {
        expr = ex;
        TSolutionChain::sm_SolutionChain.AddExpr( expr );
        }
      ex = RemDenominator( expr, Cond );
      if( !ex.Eq( expr ) )
        {
        expr = ex;
        TSolutionChain::sm_SolutionChain.AddExpr( expr );
        }

      expr.Binar( '=', ex1, ex2 );
      QByteArray Name;
      if( ex1.Variab( Name ) && Name == VarName && ex2.HasUnknown( VarName ).isEmpty() )
        return Final();
      ex = Transfer( expr );
      if( !ex.Eq( expr ) ) {
        expr = ex;
        TSolutionChain::sm_SolutionChain.AddExpr( expr );
        }
      if( expr.Splitted() )
        {
        ex = DelSplitted( expr );
        expr = ex;
        }
      ex = expr.Reduce();
      MathExpr lp, rp;
      if( !ex.Equal( expr ) && !( ex.Binar( '=', lp, rp ) && lp.Variab( Name ) && Name == VarName ) )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        eq = P.Equation( ex.WriteE(), VarName, IsName, MultUnknown );
        }
      if( IsName )
        {
        Ratio = false;
        try
          {
          expr.Binar( '=', lp, rp );
          ( lp - rp ).ReductionPoly(p, VarName );
          }
        catch( ErrParser )
          {
          Ratio = true;
          }
        if( Ratio ) throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        double d;
        if( MultUnknown || !Ratio && p[1].Constan( d ) && abs( d ) < 0.0000001 )
          {
          if( !Ratio )
            for( int i = s_DegPoly; i >= 2; i-- )
              if( !( p[i].Reduce().Constan( d ) && ( abs( d ) < 0.0000001 ) ) )
                throw  ErrParser( "Wrong type of equation!", peNoSolvType );
          if( !ex.Equal( expr ) )
            TSolutionChain::sm_SolutionChain.AddExpr( expr );
           MathExpr a = p[1].Reduce(true);
          MathExpr b = (-p[0]).Reduce(true);
          ex = new TBinar( '=', ( a * Variable( VarName ) ).Reduce(), b );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          if( Cast( TVariable, CastPtr( TBinar, ex )->Left().Ptr() ) != nullptr )
            {
            Result = new TL2exp;
            Result.Addexp( ex );
            return Final();
            }
          if( !( a.Constan( d ) && abs( d ) < 0.0000001 ) )
            {
            ex = b / a;
            ex1 = ex.Reduce();
            ex = ex1.CancellationOfMultiNominals( ex2 );
            Result = new TL2exp;
            Result.Addexp( ex );
            if( !ex1.Eq( ex ) ) ex = new TBinar( '=', ex1, ex );
            else
              ex = new TBinar( '=', Variable( VarName ), ex );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            q = P.GetExpression( a.WriteE() );
            if( !NIsConst( q ) )
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a, Constant( 0 ) ) );
            PutConditions( Cond, Result.Last()->m_Memb);
            if( s_FinalComment )
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Equation is solved" ) );
            }
          else
            if( b.Reduce().Constan( d ) && ( abs( d ) < 0.0000001 ) )
              {
              PutCondForInfiniteSol( Cond );
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfiniteSetSol", "Infinite set of solutions!" ) );
              }
            else
              {
              q = P.GetExpression( b.WriteE() );
              if( !NIsConst( q ) )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfOrNotSolRight1",
                " if an expression at the right side is equal to zero, " ) +
                X_Str( "MInfOrNotSolRight2", "the equation has infinite set of solutions " ) +
                X_Str( "MInfOrNotSolRight3", "otherwise equation has no solutions." ) );
              else
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
              }
          }
        else
          {
          try
            {
            ex = SolutionSimpleEquaion( eq, VarName );
            }
          catch( ErrParser )
            {
            throw  ErrParser( "No Solutions!", peNoSolv );
            }
          CastPtr( TL2exp, ex )->Last()->m_Memb.Binar( '=', lp, rp );
          ex1 = rp.CancellationOfMultiNominals( ex2 ).Reduce();
          ex2 = new TBinar( '=', lp, rp );
          if( !ex1.Eq( rp ) ) ex2 = new TBinar( '=', ex2, ex1 );
          Result = new TL2exp;
          Result.Addexp( ex1 );
          PExMemb  F;
          if( Cond.FindEqual( new TBinar( msNotequal, lp, rp ), F ) )
            throw  ErrParser( "No Solutions!", peNoSolv );
          if( Cond.FindEqual( new TBinar( msNotequal, lp, ex1 ), F ) )
            throw  ErrParser( "No Solutions!", peNoSolv );

          if( s_Calculations || !ex2.Equal( expr ) )
            TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
          PutConditions( Cond, rp );
          return Final();
          }
        }
      else
        {
        expr.Binar( '=', lp, rp );
        if( lp.Equal( rp ) )
          TSolutionChain::sm_SolutionChain.AddExpr( expr, X_Str( "MInfOrNotSolRight2", "Equation has infinite set of solutions!" ) );
        else
          {
          expr = ( rp - lp ).Reduce();
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Constant( 0 ), expr ) );
          double d;
          if( expr.Constan( d ) )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
          else
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfOrNotSolRight1",
            " if an expression at the right side is equal to zero, " ) +
            X_Str( "MInfOrNotSolRight2", "the equation has infinite set of solutions " ) +
            X_Str( "MInfOrNotSolRight3", "otherwise equation has no solutions." ) );
          }
        }
      }
    else
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinNo", "Must be linear equation!" ) );
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Final();
  }

void FracToMult(PNode Expr )
    {
    PNode e, q;
    std::function<void(PNode )> PreOrder = [&](PNode p)
      {
      if(p == nullptr ) return;
      PreOrder(p->m_pLeft );
      PreOrder(p->m_pRight );
      if(p->m_OpSign == '/' && !In(p->m_pRight->m_OpSign, "fidM" ) )
        {
        e = new TNode( Expr->m_pOwner );
        e->m_pLeft = nullptr;
        e->m_IsLeft = false;
        e->m_pRight = nullptr;
        e->m_IsRight = false;
        e->m_OpSign = 'f';
        e->m_Info = "1";
        e->m_Priority = 0;
        q = Divis( e, p->m_pRight );
        e->m_pParent = q;
        p->m_pRight->m_pParent = q;
        p->m_OpSign = '*';
        p->m_Info = "*";
        p->m_pRight = q;
        }
      };
    PreOrder( Expr );
    }

Lexp CalcDetQuEqu( const QByteArray& Source, QByteArray VarName )
  {
  Lexp Result = new TL2exp;
  auto ErrResult = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "?" ), X_Str( "MEnterQuadrEquat", "Enter quadratic equation!" ) );
    return nullptr;
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  bool OldNoRootReduce = s_NoRootReduce;

  auto Final = [&] ()
    {
    s_Precision = OldPrecision;
    if( s_PutAnswer && s_Answer.IsEmpty() && Result.Count() != 0 )
      s_Answer = Result;
    s_NoRootReduce = OldNoRootReduce;
    return Result;
    };

  MathExpr ex;
  try
    {
    if( Source.isEmpty() ) return Result;
    s_NoRootReduce = true;
    if( VarName == "" )
      {
      ex = Parser::StrToExpr( Source );
      VarName = ex.HasUnknown();
      if( VarName.isEmpty() ) return ErrResult();
      }
    bool IsName, Mult;
    Parser P;
    PNode eq = P.Equation( Source, VarName, IsName, Mult );
    if( IsFuncEqu( eq ) || TestFrac( eq ) || IsExpEqu( eq ) ) return ErrResult();
    ex = Parser().OutPut( eq );
    MathExpr op1, op2;
    if( !ex.Binar( '=', op1, op2 ) || !op1.IsLinear() && !op1.ConstExpr() || !op2.IsLinear() && !op2.ConstExpr() )
      throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    if( GetFactorCount( ex, VarName ) > 1 )
      {
      s_FinalComment = true;
      do
        {
        int i = s_RootCount;
        bool OldAccumulate = TSolutionChain::sm_SolutionChain.m_Accumulate;
        TSolutionChain::sm_SolutionChain.m_Accumulate = false;
        ex = GetFactorEquation( VarName );
        TSolutionChain::sm_SolutionChain.m_Accumulate = OldAccumulate;
        if( ex.IsEmpty() )
          {
          if( s_FinalComment )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Equation is solved" ) );
          return Final();
          }
        if( s_RootCount > i )
          Result.Addexp( ex );
        else
          {
          MathExpr ex1 = CalcDetQuEqu( ex.WriteE(), VarName );
          s_FactorizedSolving = !ex1.IsEmpty() && CastPtr( TL2exp, ex1 )->Count() != 0;
          if( !s_FactorizedSolving ) return ErrResult();
          s_FinalComment = false;
          for(PExMemb ResMemb = CastPtr( TL2exp, ex1 )->First(); !ResMemb.isNull(); ResMemb = ResMemb->m_pNext )
            Result.Addexp( ResMemb->m_Memb );
          }
        } while( true );
      }
    FracToMult( eq );
    ex = P.OutPut( eq );
    MathExpr ex1;
    if( ex.Splitted() )
      {
      ex1 = DelSplitted( ex );
      ex = ex1;
      }
    ex1 = ExpandExpr( ex );
    if( !ex.Eq( ex1 ) )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      ex = ex1;
      }
    if( ex.Binar( '=', op1, op2 ) )
      {
      QByteArray sName;
      if( !op1.Variab( sName ) || sName != VarName || op2.HasUnknown() == VarName )
        {
        ex1 = ( op1 - op2 ).Reduce();
        TExprs a;
        MathExpr ex2 = ex1.ReductionPoly( a, VarName );
        double r;
        for( int i = s_DegPoly; i >= 3; i-- )
          if( !( a[i].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) ) return ErrResult();
        if( !( op2.Constan( r ) && abs( r ) < 0.0000001 ) )
          {
          ex = new TBinar( '=', ex1, Constant( 0 ) );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        ex1 = new TBinar( '=', ex2, Constant( 0 ) );
        if( !ex.Equal( ex1 ) )
          TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
        if( ( a[2].Constan( r ) && abs( r ) < 0.0000001 ) )
          {
          if( a[1].Reduce().Constan( r ) && abs( r ) < 0.0000001 )
            if( a[0].Reduce().Constan( r ) && ( abs( r ) < 0.0000001 ) )
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfiniteSetSol", "Infinite set of solutions!" ) );
            else
              {
              PNode q = P.GetExpression( a[0].WriteE() );
              if( !NIsConst( q ) )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfOrNotSolLeft1",
                " if an expression at the left side is equal to zero, " ) +
                X_Str( "MInfOrNotSolLeft2", "the equation has infinite set of solutions, " ) +
                X_Str( "MInfOrNotSolLeft3", "otherwise equation has no solutions." ) );
              else
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
              }
          else
            {
            ex1 = ( -a[0] / a[1] ).Reduce().Reduce(true);
            if( s_FactorizedSolving )
              ex = GetPutRoot( ex1, VarName );
            else
              {
              ex = new TBinar( '=', Variable( VarName ), ex1 );
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              }
            PNode q = P.GetExpression( a[1].WriteE() );
            if( !NIsConst( q ) )
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[1], Constant( 0 ) ) );
            if( s_FinalComment )
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Equation is solved" ) );
            Result.Addexp( ex );
            }
          }
        else
          {
          MathExpr op;
          if( a[2].Divis( op1, op2 ) || ( a[2].Unarminus( op ) && op.Divis( op1, op2 ) ) )
            {
            ex = ( a[1] * op2 ).Reduce();
            a[1] = ex;
            ex = ( a[0] * op2 ).Reduce();
            a[0] = ex;
            if( a[2].Unarminus( op ) )
              ex = -op1;
            else
              ex = op1;
            a[2] = ex;
            ex = Variable( VarName ) ^ 2;
            if( !( a[2].Constan( r ) && ( r == 1 ) ) )
              ex = a[2] * ex;
            if( !( a[1].Constan( r ) && abs( r ) < 0.0000001 ) )
              if( !( a[1].Constan( r ) && ( r == 1 ) ) )
                ex += a[1] * Variable( VarName );
              else
                ex += Variable( VarName );
            if( !( a[0].Constan( r ) && abs( r ) < 0.0000001 ) )
              ex += a[0];
            ex = new TBinar( '=', ex, Constant( 0 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            }

          if( a[0].Constan( r ) && ( abs( r ) < 0.0000001 ) )
            {
            ex = new TBinar( '=', ( Variable( VarName, true ) * ( a[2] * Variable( VarName, true ) + a[1] ) ).Reduce(), Constant( 0 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            ex = GetPutRoot( Constant( 0 ), VarName );
            Result.Addexp( Constant( 0 ) );
            ex = GetPutRoot( (-( a[1] / a[2] ) ).Reduce(), VarName );
            if( s_FinalComment )
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
            Result.Addexp( (-( a[1] / a[2] ) ).Reduce() );
            }
          else
            if( a[1].Constan( r ) && abs( r ) < 0.0000001 )
              {
              ex1 = -( a[0] / a[2] ).Reduce();
              int nom, den;
              if( !( ex1.Constan( r ) && r < 0 || ex1.SimpleFrac_( nom, den ) && nom < 0 ) )
                ex1 = ex1.Root( 2 ).Reduce();
              else
                ex1 = CreateComplex( Constant( 0 ), ex1.RetNeg().Reduce().Root( 2 ) ).Reduce();
              ex = GetPutRoot( ex1, VarName );
              Result.Addexp( ex );
              ex = GetPutRoot( -ex1, VarName );
              if( s_FinalComment )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
              Result.Addexp( ex );
              }
            else
              {
              MathExpr D1 = ( a[1] ^ 2 ) - Constant( 4 ) * a[2] * a[0];
              MathExpr D = D1.Reduce(true);
              if( D.Equal( D1 ) )
                ex = new TBinar( '=', Variable( 'D' ), D );
              else
                {
                MathExpr D2 = ExpandExpr( D );
                if( D2.Equal( D ) )
                  ex = new TBinar( '=', Variable( 'D' ), new TBinar( '=', D1, D ) );
                else
                  ex = new TBinar( '=', Variable( 'D' ), new TBinar( '=', D1, new TBinar( '=', D, D2 ) ) );
                D = D2;
                }
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              if( D.Constan( r ) && abs( r ) < 0.0000001 )
                {
                ex1 = ( -a[1] / ( Constant( 2 ) * a[2] ) ).Reduce().Reduce();
                ex = new TBinar( '=', Variable( VarName + QByteArray::number( s_RootCount + 1 ), true ),
                  new TBinar( '=', Variable( VarName + QByteArray::number( s_RootCount + 2 ), true ), ex1 ) );
                s_RootCount += 2;
                TSolutionChain::sm_SolutionChain.AddExpr( ex );
                Result.Addexp( ex1.Reduce() );
                PNode q = P.GetExpression( a[2].WriteE() );
                if( !NIsConst( q ) )
                  TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[2], Constant( 0 ) ) );
                if( s_FinalComment )
                  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
                }
              else
                {
                int nom, den;
                if( !( D.Constan( r ) && r < 0 || D.SimpleFrac_( nom, den ) && nom < 0 ) )
                  ex = D.Root( 2 );
                else
                  ex = CreateComplex( Constant( 0 ), D.RetNeg().Reduce().Root( 2 ) );
                ex1 = Constant( 2 ) * a[2];
                op1 = ( -a[1] - ex ) / ex1;
                MathExpr x1 = op1.Reduce().Reduce();
                ReduceExpr( x1 );
                x1 = ExpandExpr( x1 );
                Result.Addexp( x1 );
                op2 = ( -a[1] + ex ) / ex1;
                MathExpr x2 = op2.Reduce().Reduce();
                ReduceExpr( x2 );
                x2 = ExpandExpr( x2 );
                Result.Addexp( x2 );
                ex = GetPutRoot( new TBinar( '=', op1, x1 ), VarName );
                ex = GetPutRoot( new TBinar( '=', op2, x2 ), VarName );
                PNode q = P.GetExpression( a[2].WriteE() );
                if( !NIsConst( q ) )
                  TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[2], Constant( 0 ) ) );
                if( s_FinalComment )
                  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
                }
              }
          }
        }
      else
        Result.Addexp( ex );
      }
    else
      return ErrResult();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Final();
  }

bool ReduceEquation( MathExpr& ex, const QByteArray& SelName )
    {
    bool IsName, Mult;
    Parser P;
    PNode eq = P.Equation( ex.WriteE(), SelName, IsName, Mult );
    return ReduceFactor( eq, ex );
    }

int DegOfPoly( const MathExpr& ex, const QByteArray& Name )
  {
  int k;
  try
    {
    TExprs a;
    double v;
    ex.ReductionPoly( a, Name );
    k = s_DegPoly;
    for( ; a[k].Constan( v ) && abs( v ) < 0.0000001; k-- );
    }
  catch(ErrParser )
    {
    return -1;
    }
  return k;
  }

Lexp CalcDetBiQuEqu( const QByteArray& Source, const QByteArray& VarName )
  {
  if( Source.isEmpty() ) return nullptr;
  bool bComplex = false;
  MathExpr exRoot;
  Lexp Result = new TL2exp;
  bool IsName, Mult, bFinalComment;
  auto OutLexp = [&] ( const MathExpr& ex, int Num )
    {
    Lexp Cond, Solv;
    Parser P( &Cond );
    try
      {
      Solv = SolutionSimpleEquaion(P.Equation( ex.WriteE(), "x", IsName, Mult ), "x" );
      }
    catch( ErrParser )
      {
      throw  ErrParser( "No Solutions!", peNoSolv );
      }
    PExMemb f = Solv.First();
    for( ; f != Solv.Last() && !bComplex; f = f->m_pNext )
      TSolutionChain::sm_SolutionChain.AddExpr( f->m_Memb );
    MathExpr op1, op2;
    f->m_Memb.Binar( '=', op1, op2 );
    if( bComplex ) op2 = op2.Substitute( "b", exRoot ).Reduce();
    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName + '_' + NumberToStr( Num ) ), op2 ) );
    Result.Addexp( op2 );
    };

  auto ComplexRoot = [&] ( MathExpr& CEx )
    {
    MathExpr R = CEx.Root( 2 ).Reduce();
    if( !( IsType( TLexp, R ) ) )
      {
      bool OldFullReduce = TExpr::sm_FullReduce;
      TExpr::sm_FullReduce = true;
      CEx.SetReduced( false );
      R = CEx.Reduce().Root( 2 ).Reduce();
      TExpr::sm_FullReduce = OldFullReduce;
      if( !( IsType( TLexp, R ) ) ) throw  ErrParser( "No Solutions!", peNoSolv );
      }
    TConstant::sm_ConstToFraction = false;
    return CastPtr( TLexp, R )->First();
    };

  double OldPrecision = s_Precision;
  bool OldRootReduce = s_NoRootReduce;
  s_Precision = 0.0000001;

  auto Final = [&] ()
    {
    s_Precision = OldPrecision;
    s_NoRootReduce = OldRootReduce;
    return Result;
    };

  auto ErrResult = [&] ()
    {
    if( s_FinalComment )
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MEnterBiQuEquat", "Enter biquadratic equation" ) );
    return Final();
    };

  try
    {
    s_NoRootReduce = true;
    s_DegPoly = 5;
    if( s_PutAnswer )
      s_Answer = new TL2exp;
    QByteArray VarName1;
    if( VarName == "y" )
      VarName1 = "z";
    else
      VarName1 = "y";
    Parser P;
    PNode eq = P.Equation(P.FullPreProcessor( Source, VarName ), VarName, IsName, Mult );
    if( IsFuncEqu( eq ) || TestFrac( eq ) || IsExpEqu( eq ) ) return ErrResult();
    MathExpr ex = P.OutPut( eq ), ex1, ex2;
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    FracToMult( eq );
    ex = P.OutPut( eq );
    if( ex.Splitted() )
      {
      ex1 = DelSplitted( ex );
      ex = ex1;
      }
    if( GetFactorCount( ex, VarName ) > 1 )
      {
      bFinalComment = true;
      do
        {
        int i = s_RootCount;
        bool OldAccumulate = TSolutionChain::sm_SolutionChain.m_Accumulate;
        TSolutionChain::sm_SolutionChain.m_Accumulate = false;
        ex = GetFactorEquation( VarName );
        TSolutionChain::sm_SolutionChain.m_Accumulate = OldAccumulate;
        if( ex.IsEmpty() )
          {
          if( bFinalComment )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Equation is solved" ) );
          return Final();
          }
        if( s_RootCount > i )
          Result.Addexp( ex );
        else
          {
          bool InterRes = TSolutionChain::sm_InterimResult;
          TSolutionChain::sm_InterimResult = true;
          if( !s_CalcOnly )
            {
            s_CalcOnly = true;
            i = s_RootCount;
            TSolutionChain::sm_SolutionChain.m_Accumulate = false;
            ex1 = CalcDetQuEqu( ex.WriteE(), VarName );
            TSolutionChain::sm_SolutionChain.m_Accumulate = true;
            s_RootCount = i;
            s_CalcOnly = false;
            }
          if( s_CalcOnly || !ex1.IsEmpty() )
            ex1 = CalcDetQuEqu( ex.WriteE(), VarName );
          if( ex1.IsEmpty() )
            ex1 = CalcDetBiQuEqu( ex.WriteE(), VarName );
          TSolutionChain::sm_InterimResult = InterRes;
          s_FactorizedSolving = !ex1.IsEmpty() && !CastPtr( TL2exp, ex1 )->First().isNull();
          if( !s_FactorizedSolving ) return ErrResult();
          bFinalComment = false;
          for(PExMemb ResMemb = CastPtr( TL2exp, ex1 )->First(); !ResMemb.isNull(); ResMemb = ResMemb->m_pNext )
            Result.Addexp( ResMemb->m_Memb );
          }
        } while( true );
      }
    ex1 = ex;
    if( ReduceEquation( ex ) || !ex.Eq( ex1 ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr op1, op2, op11, op22, op;
    int iPow;
    double r;
    bool Simple, Solve;
    if( ex.Binar( '=', op1, op2 ) )
      {
      if( op1.Power( op11, op22 ) && op22.Cons_int( iPow ) && iPow == 4 && op2.Constan( r ) )
        {
        if( r == 0 ) return ErrResult();
        ex1 = op11 ^ 2;
        ex2 = op2.Root( 2 );
        Simple = true;
        }
      else
        if( op2.Power( op11, op22 ) && op22.Cons_int( iPow ) && iPow == 4 && op1.Constan( r ) )
          {
          if( r == 0 ) return ErrResult();
          ex1 = op11 ^ 2;
          ex2 = op1.Root( 2 );
          Simple = true;
          }
        else
          {
          ex2.Clear();
          Simple = false;
          }
      if( Simple )
        {
        if( r < 0 )
          {
          Result = RootPolinom( ex );
          TSolutionChain::sm_SolutionChain.AddExpr( Result );
          if( s_FinalComment )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
          return Final();
          }
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, ex2 ) );
        exRoot = ex2.Reduce().Root( 2 );
        op = new TBinar( '=', op11, exRoot );
        OutLexp( op, 1 );
        op = new TBinar( '=', op11, -exRoot );
        OutLexp( op, 2 );
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, -ex2 ) );
        exRoot = CreateComplex( Constant( 0 ), exRoot );
        ex1 = Variable( ( "b" ) );
        ex2 = new TBinar( '=', op11, ex1 );
        bComplex = true;
        OutLexp( ex2, 3 );
        ex2 = new TBinar( '=', op11, -ex1 );
        OutLexp( ex2, 4 );
        if( s_FinalComment )
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
        return Final();
        }
      ex1 = ( op1 - op2 ).Reduce();
      if( !( op2.Constan( r ) && abs( r ) < 0.0000001 ) )
        {
        ex = new TBinar( '=', ex1, Constant( 0 ) );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        }
      MathExpArray a;
      ex2 = ex1.ReductionPoly( a, VarName );
      int n = 0;
      for( int i = 1; i <= s_DegPoly; i++ )
        if( i == 4 || i == 2 )
          {
          if( !( a[i].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) ) n = i;
          continue;
          }
        else
          if( !( a[i].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) ) return ErrResult();

      if( a[4].IsEmpty() ) return Final();
      if( !( a[4].Constan( r ) && r == 0 ) && a[2].Constan( r ) && r == 0 )
        {
        ex1 = ( -a[0] / a[4] ).Reduce();
        ex2 = Variable( VarName ) ^ 4;
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex2, ex1 ) );
        if( ex1.Negative() )
          {
          Result = RootPolinom( ex );
          TSolutionChain::sm_SolutionChain.AddExpr(Result );
          if( s_FinalComment )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
          return Final();
          }
        ex2 = ex1.Root( 2 ).Reduce();
        ex1 = Variable( VarName ) ^ 2;
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, ex2 ) );
        ex = ex2.Root( 2 ).Reduce();
        GetPutRoot( ex, VarName );
        Result.Addexp( ex );
        GetPutRoot( -ex, VarName );
        Result.Addexp( -ex );
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, -ex2 ) );
        ex = CreateComplex( Constant( 0 ), ex2 );
        GetPutRoot( -ex, VarName );
        Result.Addexp( ex );
        GetPutRoot( -ex, VarName );
        Result.Addexp( -ex );
        if( s_FinalComment )
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
        return Final();
        }
      ex1 = new TBinar( '=', ex2, Constant( 0 ) );
      if( !ex.Eq( ex1 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      MathExpr exFirst = ex;
      MathExpr x1, x2;
      ex = new TBinar( '=', Variable( VarName1 ), Variable( VarName ) ^ 2 );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      switch( n )
        {
        case 0:
          ex = a[0];
          break;
        case 2:
          ex = ( a[2] * Variable( VarName1 ) ).Reduce();
          break;
        case 4:
          ex = ( a[4] * ( Variable( VarName1 ) ^ 2 ) ).Reduce();
        }
      n -= 2;
      if( n >= 2 )
        if( !( a[2].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) )
          ex += ( a[2] * Variable( VarName1 ) ).Reduce();
      if( n >= 0 )
        if( !( a[0].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) )
          ex = ( ex + a[0] ).Reduce();
      ex = new TBinar( '=', ex, Constant( 0 ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      if( ( a[4].Constan( r ) && abs( r ) < 0.0000001 ) )
        {
        if( a[2].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) return ErrResult();
        MathExpr y1 = ( -a[0] / a[2] ).Reduce().Reduce();
        ex = new TBinar( '=', Variable( VarName1 ), y1 );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        ex = new TBinar( '=', Variable( VarName ) ^ 2, y1 );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        int nom, den;
        if( !( y1.Constan( r ) && r < 0 || y1.SimpleFrac_( nom, den ) && nom < 0 ) )
          {
          x1 = -( y1.Root( 2 ) );
          x2 = y1.Root( 2 );
          }
        else
          {
          x2 = CreateComplex( Constant( 0 ), y1.RetNeg().Reduce().Root( 2 ) );
          x1 = -x2;
          }
        ex = GetPutRoot( new TBinar( '=', x1, x1.Reduce() ), VarName );
        Result.Addexp( x1.Reduce() );
        if( s_PutAnswer )
          CastPtr( TL2exp, s_Answer )->Addexp( ex );
        ex = GetPutRoot( new TBinar( '=', x2, x2.Reduce() ), VarName );
        Result.Addexp( x2.Reduce() );
        if( s_PutAnswer )
          CastPtr( TL2exp, s_Answer )->Addexp( ex );
        PNode q = P.GetExpression( a[2].WriteE() );
        if( !NIsConst( q ) )
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[2], Constant( 0 ) ) );
        if( s_FinalComment )
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
        }
      else
        {
        if( a[4].Divis( op1, op2 ) || ( a[4].Unarminus( op ) && op.Divis( op1, op2 ) ) )
          {
          ex = ( a[2] * op2 ).Reduce();
          a[2] = ex;
          ex = ( a[0] * op2 ).Reduce();
          a[0] = ex;
          if( a[4].Unarminus( op ) )
            ex = -op1;
          else
            ex = op1;
          a[4] = ex;
          ex = Variable( VarName1 ) ^ 2;
          if( !( a[4].Constan( r ) && r == 1 ) )
            ex = a[4] * ex;
          if( !( a[2].Constan( r ) && abs( r ) < 0.0000001 ) )
            if( !( a[2].Constan( r ) && r == 1 ) )
              ex += a[2] * Variable( VarName1 );
            else
              ex += Variable( VarName1 );
          if( !( a[0].Constan( r ) && abs( r ) < 0.0000001 ) )
            ex += a[0];
          ex = new TBinar( '=', ex, Constant( 0 ) );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        MathExpr D1 = ( a[2] ^ 2 ) - Constant( 4 ) * a[4] * a[0];
        MathExpr D = D1.Reduce();
        if( D.Equal( D1 ) )
          ex = new TBinar( '=', Variable( "D" ), D );
        else
          {
          MathExpr D2 = D.Reduce();
          if( D2.Equal( D ) )
            ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, D ) );
          else
            ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, new TBinar( '=', D, D2 ) ) );
          D = D2;
          }
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        if( D.Constan( r ) && abs( r ) < 0.0000001 )
          {
          MathExpr y1 = ( -a[2] / ( Constant( 2 ) * a[4] ) ).Reduce().Reduce();
          ex = new TBinar( '=', new TVariable( true, VarName1 + "_1" ), new TBinar( '=', new TVariable( true, VarName1 + "_2" ), y1 ) );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          ex = new TBinar( '=', Variable( VarName ) ^ 2, y1 );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          if( !( y1.Constan( r ) && r < 0 ) )
            {
            x1 = -( y1.Root( 2 ) ).Reduce();
            x2 = y1.Root( 2 ).Reduce();
            }
          else
            {
            x2 = CreateComplex( Constant( 0 ), Constant( -r ).Root( 2 ) ).Reduce();
            x1 = -( x2 ).Reduce();
            }
          ex = new TBinar( '=', new TVariable( true, VarName + '_' + NumberToStr( s_RootCount + 1 ) ), new TBinar( '=', new TVariable( true, VarName + '_' + NumberToStr( s_RootCount + 2 ) ), x1 ) );
          s_RootCount = s_RootCount + 2;
          Result.Addexp( x1.Reduce() );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          if( s_PutAnswer )
            CastPtr( TL2exp, s_Answer )->Addexp( ex );
          ex = new TBinar( '=', new TVariable( true, VarName + '_' + NumberToStr( s_RootCount + 1 ) ), new TBinar( '=', new TVariable( true, VarName + '_' + NumberToStr( s_RootCount + 2 ) ), x2 ) );
          s_RootCount = s_RootCount + 2;
          Result.Addexp( x2.Reduce() );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          if( s_PutAnswer )
            CastPtr( TL2exp, s_Answer )->Addexp( ex );
          PNode q = P.GetExpression( a[4].WriteE() );
          if( !NIsConst( q ) )
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[4], Constant( 0 ) ) );
          if( s_FinalComment )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
          }
        else
          {
          int nom, den;
          if( D.Constan( r ) && r < 0 || D.SimpleFrac_( nom, den ) && nom < 0 )
            ex = CreateComplex( Constant( 0 ), ( -D ).Root( 2 ) ).Reduce();
          else
            ex = D.Root( 2 );
//          if( !( D.Constan( r ) && r < 0 || D.SimpleFrac_( nom, den ) && nom < 0 ) )
//            {
            ex1 = Constant( 2 ) * a[4];
            op1 = ( -a[2] - ex ) / ex1;
            MathExpr y1 = ExpandExpr( op1 ).Reduce();
            op2 = ( -a[2] + ex ) / ex1;
            MathExpr y2 = ExpandExpr( op2 ).Reduce();
            ex = new TBinar( '=', new TVariable( true, VarName1 + "_1" ), new TBinar( '=', op1, y1 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            ex = new TBinar( '=', new TVariable( true, VarName1 + "_2" ), new TBinar( '=', op2, y2 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            ex = new TBinar( '=', Variable( VarName ) ^ 2, y1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex.Reduce() );
            Solve = false;
            if( IsType( TComplexExpr, y1 ) )
              {
              PExMemb pFirst = ComplexRoot(y1);
              x1 = pFirst->m_Memb;
              x2 = pFirst->m_pNext->m_Memb;
              }
            else
              if( !( y1.Constan( r ) && r < 0 || y1.SimpleFrac_( nom, den ) && nom < 0 ) )
                {
                x1 = -( y1.Root( 2 ) ).Reduce();
                x2 = y1.Root( 2 ).Reduce();
                }
              else
                {
                x2 = CreateComplex( Constant( 0 ), y1.RetNeg().Root( 2 ) ).Reduce();
                x1 = -( x2 ).Reduce();
                }
            Result.Addexp( x1.Reduce() );
            ex = GetPutRoot( x1, VarName );
            if( s_PutAnswer )
              CastPtr( TL2exp, s_Answer )->Addexp( ex );
            Result.Addexp( x2.Reduce() );
            ex = GetPutRoot( x2, VarName );
            if( s_PutAnswer )
              CastPtr( TL2exp, s_Answer )->Addexp( ex );
            Solve = true;
            ex = new TBinar( '=', Variable( VarName ) ^ 2, y2 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( IsType( TComplexExpr, y2 ) )
              {
              PExMemb pFirst = ComplexRoot( y2 );
              x1 = pFirst->m_Memb;
              x2 = pFirst->m_pNext->m_Memb;
              }
            else
              if( !( y2.Constan( r ) && r < 0 || y2.SimpleFrac_( nom, den ) && nom < 0 ) )
                {
                x1 = -( y2.Root( 2 ) ).Reduce();
                x2 = y2.Root( 2 ).Reduce();
                }
              else
                {
                x2 = CreateComplex( Constant( 0 ), y2.RetNeg().Root( 2 ) ).Reduce();
                x1 = -( x2 ).Reduce();
                }
            Result.Addexp( x1.Reduce() );
            ex = GetPutRoot( x1, VarName );
            if( s_PutAnswer )
              CastPtr( TL2exp, s_Answer )->Addexp( ex );
            ex = GetPutRoot( x2, VarName );
            Result.Addexp( x2.Reduce() );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( s_PutAnswer )
              CastPtr( TL2exp, s_Answer )->Addexp( ex );
            Solve = true;
            if( Solve )
              {
              PNode q = P.GetExpression( a[4].WriteE() );
              if( !NIsConst( q ) )
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[4], Constant( 0 ) ) );
              if( s_FinalComment )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
              }
            /*
            }
          else
            {
            pResult = RootPolinom( exFirst );
            TSolutionChain::sm_SolutionChain.AddExpr(pResult );
            if( s_FinalComment )
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MBiQuadEqSolved", "Biquadratic equation is solved" ) );
            if( s_PutAnswer )
              s_Answer = pResult;
            }
*/
          }
        }
      }
    else
      return ErrResult();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Final();
  }

bool EquationsSolving( const QByteArray& UserTask, const QByteArray& VarName )
  {
  bool CalcOnlyOld = s_CalcOnly;
  s_CalcOnly = true;
  TSolutionChain::sm_SolutionChain.m_Accumulate = false;

  try
    {
    if( CalcDetLinEqu( UserTask, VarName ) != nullptr )
      {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      s_Answer = CalcDetLinEqu( UserTask, VarName );
      return true;
      }

    if( CalcDetQuEqu( UserTask, VarName ) != nullptr )
      {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      s_Answer = CalcDetQuEqu( UserTask, VarName );
      return true;
      }

    if( CalcDetBiQuEqu( UserTask, VarName ) != nullptr ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      s_Answer = CalcDetBiQuEqu( UserTask, VarName );
      return true;
      }

    if( FractRatEq( UserTask, VarName ) != nullptr ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      s_Answer = FractRatEq( UserTask, VarName );
      return true;
      }

    if( !CalcIrratEq( UserTask ).IsEmpty() ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      s_Answer = CalcIrratEq( UserTask );
      return true;
      }

    if( CalcExpEq( UserTask ) ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      CalcExpEq( UserTask );
      return true;
      }

    if( CalcLog1Eq( UserTask, VarName ) ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      CalcLog1Eq( UserTask, VarName );
      return true;
      }

    if( CalcSimpleTrigo( UserTask ) ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      CalcSimpleTrigo( UserTask );
      return true;
      }

    if( CalcTrigoEqu( UserTask ) ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      CalcTrigoEqu( UserTask );
      return true;
      }

    if( CalcHomogenTrigoEqu( UserTask ) ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      CalcHomogenTrigoEqu( UserTask );
      return true;
      }

    if( CalcSinCosEqu( UserTask ) ) {
      s_CalcOnly = CalcOnlyOld;
      TSolutionChain::sm_SolutionChain.m_Accumulate = true;
      CalcSinCosEqu( UserTask );
      return true;
      }
    }
  catch( ErrParser )
    {
    s_CalcOnly = CalcOnlyOld;
    TSolutionChain::sm_SolutionChain.m_Accumulate = true;
    }
  return false;
  }

Lexp FractRatEq( const QByteArray& Source, const QByteArray& VarName, bool CalcBiQ )
  {
  auto Gorner = [] ( const MathExpr& x, const TExprs& p, int Deg )
    {
    MathExpr Res = p[Deg];
    for( int i = Deg - 1; i >= 0; i-- )
      Res = ( ( Res * x ).Reduce() + p[i] ).Reduce();
    return Res;
    };
  MathExpr CommDen, ex, ex1, ex2;
  TExprs q;
  int DegDen;
  double d;
  Lexp Result = new TL2exp;

  auto CheckRoot = [&] ( const MathExpr& Root, int n )
    {
    MathExpr CommDenSubst, DenVal, DenValue;
    if( n > 0 )
      CommDenSubst = CommDen.Substitute( VarName, new TVariable( true, VarName + '_' + QByteArray::number( n ) ) );
    else
      CommDenSubst = CommDen;
    DenVal = Gorner( Root, q, DegDen );
    DenValue = new TBinar( '=', CommDenSubst, DenVal );
    if( DenVal.Constan( d ) && abs( d ) < 0.0000001 )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( DenValue, X_Str( "MRootStranger", "false Root!" ) );
      return;
      }
    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, DenValue, Constant( 0 ) ) );
    if( !Result.IsEmpty()) Result.Addexp( Root );
    };

  int RootNum;
  std::function<void( MathExpr& )> FindRoots = [&] ( const MathExpr& op )
    {
    MathExpr op1, op2;
    if( op.Multp( op1, op2 ) )
      {
      FindRoots( op1 );
      FindRoots( op2 );
      return;
      }
    TExprs p;
    op.ReductionPoly( p, VarName );
    RootNum++;
    MathExpr  x = ( -p[0] / p[1] ).Reduce();
    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName + '_' + QByteArray::number( RootNum ) ), x ) );
    CheckRoot( x, RootNum );
    };

  bool OldNoRootReduce = s_NoRootReduce;
  s_NoRootReduce = true;
  if( CalcBiQ )
    {
    bool CalcOnlyOld = s_CalcOnly;
    s_CalcOnly = true;
    TSolutionChain::sm_SolutionChain.m_Accumulate = false;
    Result = CalcDetQuEqu( Source, VarName );
    TSolutionChain::sm_SolutionChain.m_Accumulate = !CalcOnlyOld;
    if( !Result.IsEmpty() )
      {
      s_CalcOnly = CalcOnlyOld;
      bool InterRes = TSolutionChain::sm_InterimResult;
      TSolutionChain::sm_InterimResult = true;
      Result = CalcDetQuEqu( Source, VarName );
      TSolutionChain::sm_InterimResult = InterRes;
      for( PExMemb memMemb = Result.First(); !memMemb.isNull(); memMemb = memMemb->m_pNext )
        if( memMemb->m_Memb.Binar( '=', ex1, ex2 ) )
          {
          ex = ExpandExpr( ex2 );
          memMemb->m_Memb = ex;
          }
      return Result;
      }
    Lexp R = CalcIrratEq( Source );
    s_CalcOnly = CalcOnlyOld;
    if( !R.IsEmpty() )
      {
      R = CalcIrratEq( Source );
      for( PExMemb memMemb = Result.First(); !memMemb.isNull(); memMemb = memMemb->m_pNext )
        if( memMemb->m_Memb.Binar( '=', ex1, ex2 ) )
          {
          ex = ExpandExpr( ex2 );
          Result.Addexp( ex );
          }
        else
          Result.Addexp( ExpandExpr( memMemb->m_Memb ) );
      return Result;;
      }
    }
  if( Source.isEmpty() )
    {
    s_NoRootReduce = OldNoRootReduce;
    return Result;
    }

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  QByteArray OldGlobalVarName = s_GlobalVarName;

  auto Final = [&] ()
    {
    s_EquationSolving = false;
    s_GlobalVarName = OldGlobalVarName;
    s_Precision = OldPrecision;
    if( s_PutAnswer && Result.Count() != 0 ) s_Answer = Result;
    s_NoRootReduce = OldNoRootReduce;
    return Result;
    };

  try
    {
    Parser P;
    bool IsName, MultUnknown, IsFractions;
    PNode eq = P.Equation( P.FullPreProcessor( Source, VarName ), VarName, IsName, MultUnknown );
    if( IsFuncEqu( eq ) || IsExpEqu( eq ) )
      throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    ex = P.OutPut( eq );
    s_PowerToFraction = true;
    s_EquationSolving = true;
    s_GlobalVarName = VarName;
    ex1 = ex.Reduce();
    s_EquationSolving = false;
    s_PowerToFraction = false;
    if( !ex.Eq( ex1 ) )
      {
      PNode q1 = P.Equation( ex1.WriteE(), VarName, IsName, MultUnknown );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      ex = ex1;
      eq = q1;
      }
    IsFractions = TestFrac( eq );
    if( !IsFractions )
      {
      if( ex.Splitted() )
        ex1 = DelSplitted( ex );
      else
        ex1 = ex;
      MathExpr op1, op2;
      ex1.Binar( '=', op1, op2 );
      MathExpArray p;
      ex2 = ( op1 - op2 ).Reduce().ReductionPoly( p, VarName );
      int Deg = s_DegPoly;
      for( ; Deg > 0 && p[Deg].Reduce().Constan( d ) && abs( d ) < 0.0000001; Deg-- );
      if( Deg < 5 )
        {
        QByteArray NewSource = ex.WriteE();
        switch( Deg )
          {
          case 0:
          case 1:
            Result = CalcDetLinEqu( NewSource, VarName );
            break;
          case 2:
            {
            bool InterRes = TSolutionChain::sm_InterimResult;
            TSolutionChain::sm_InterimResult = true;
            Result = CalcDetQuEqu( NewSource, VarName );
            TSolutionChain::sm_InterimResult = InterRes;
            }
            break;
          case 3:
            {
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( p[2].Constan( d ) && abs( d ) < 0.0000001 && p[1].Constan( d ) && abs( d ) < 0.0000001 )
              {
              ex1 = ( -p[0] / p[3] ).Root( 3 ).Reduce();
              Result.Addexp( ex1 );
              if( ex1.Negative() )
                {
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName + "_1" ), ex1 ) );
                MathExpr x1 = -( ex1 / Constant( 2 ) );
                ex2 = x1.Reduce();
                x1 = CreateComplex( ex2, ex2 * Constant( 3 ).Root( 2 ) );
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName + "_2" ), x1 ) );
                Result.Addexp( x1 );
                MathExpr x2 = CreateComplex( ex2, -( ex2 * Constant( 3 ).Root( 2 ) ) );
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName + "_3" ), x2 ) );
                Result.Addexp( x2 );
                }
              else
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName ), ex1 ) );
              }
            else
              if( p[0] == 0 )
                {
                ex1 = new TBinar( '=', ex2, Constant( 0 ) );
                if( !ex1.Equal( ex ) )
                  {
                  TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
                  ex = ex1;
                  }
                ex.Binar( '=', op1, op2 );
                MathExpr x1 = Variable( "x" );
                ex1 = ( ( p[3] * ( x1 ^ 2 ) + p[2] * x1 ) + p[1] ).Reduce();
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( "x" ) * ex1, op2 ) );
                bool InterRes = TSolutionChain::sm_InterimResult;
                TSolutionChain::sm_InterimResult = true;
                Result = CalcDetQuEqu( TBinar( '=', ex1, op2 ).WriteE(), VarName );
                TSolutionChain::sm_InterimResult = InterRes;
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( "x_3" ), op2 ) );
                Result.Addexp( op2 );
                }
              else
                throw  ErrParser( "Wrong type of equation!", peNoSolvType );
            }
            break;
          case 4: Result = CalcDetBiQuEqu( NewSource, VarName );
          }
        return Final();
        }
      }
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    if( Div2( eq ) )
      {
      ex = P.OutPut( eq );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }
    if( OpenBrackets( eq ) )
      {
      ex = P.OutPut( eq );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }

    if( MultDiv( eq ) )
      {
      ex = P.OutPut( eq );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      };

    ex = P.OutPut( eq );
    if( ex.Splitted() )
      {
      ex1 = DelSplitted( ex );
      ex = ex1;
      }
    ex1 = ex.Reduce();
    bool IsSolv;
    if( ex1.Boolean_( IsSolv ) && !IsSolv )
      throw  ErrParser( "No Solutions!", peNoSolv );
    if( ex1.Boolean_( IsSolv ) ) ex1 = ex;
    if( !ex.Equal( ex1 ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
    MathExpr op1, op2;
    ex1.Binar( '=', op1, op2 );
    int i;
    if( !op2.Cons_int( i ) || i != 0 )
      ex = new TBinar( '=', ( op1 - op2 ).Reduce(), Constant( 0 ) );
    eq = P.Equation( ex.WriteE(), VarName, IsName, MultUnknown );
    TDivis Divis(1);
    FindDivi( Sub( eq->m_pLeft, eq->m_pRight ), Divis );
    int iDivi = 1;
    for( ; iDivi < Divis.count() && !Divis[iDivi].m_DenDepX; iDivi++ );
    if( iDivi == Divis.count() )
      {
      ex.Binar( '=', op1, op2 );
      MathExpArray p;
      op1.ReductionPoly( p, VarName );
      int Deg = s_DegPoly;
      for( ; Deg > 0 && p[Deg].Reduce().Constan( d ) && abs( d ) < 0.0000001; Deg-- );
      if( Deg < 5 )
        {
        QByteArray NewSource = ex.WriteE();
        switch( Deg )
          {
          case 0:
          case 1:
            Result = CalcDetLinEqu( NewSource, VarName );
            break;
          case 2:
            {
            bool InterRes = TSolutionChain::sm_InterimResult;
            TSolutionChain::sm_InterimResult = true;
            Result = CalcDetQuEqu( NewSource, VarName );
            TSolutionChain::sm_InterimResult = InterRes;
            }
            break;
          case 3:
            if( p[2].Constan( d ) && abs( d ) < 0.0000001 && p[1].Constan( d ) && abs( d ) < 0.0000001 )
              {
              ex1 = ( -p[0] / p[3] ).Reduce().Root( 3 );
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( VarName ), ex1 ) );
              Result.Addexp( ex1 );
              }
            else
              throw  ErrParser( "Wrong type of equation!", peNoSolvType );
            break;
          case 4:
            Result = CalcDetBiQuEqu( NewSource, VarName );
          }
        return Final();
        }
      else
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      }
    if( !ex1.Equal( ex ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
    ex1 = ex;
    TDivEx DivEx;
    MathExpArray p;
    if( Divis[0].m_Nom != nullptr )
      DivEx.append( TDivExpr( P.OutPut( Divis[0].m_Nom ).ReductionPoly( p, VarName ).Reduce() ) );
    else
      DivEx.append( Constant( 0 ) );
    MathExpr Nom;
    if( p.count() != 1 || !( DivEx[0].m_NomEx.Constan( d ) && abs( d ) < 0.0000001 ) )
      {
      p.clear();
      DivEx.append( TDivExpr( P.OutPut( Divis[1].m_Nom ).ReductionPoly( p, VarName ) ) );
      if( Divis[1].m_Sign == '-' )
        DivEx[1].m_NomEx = -DivEx[1].m_NomEx;
      MathExpr Den = P.OutPut( Divis[1].m_Den );
      p.clear();
      DivEx[1].m_DenEx = Den.ReductionPoly( p, VarName );
      ex = DivEx[1].m_NomEx / Den;
      for( int i = 2; i < Divis.count(); i++ )
        {
        p.clear();
        DivEx.append( TDivExpr( P.OutPut( Divis[i].m_Nom ).ReductionPoly( p, VarName ) ) );
        if( Divis[i].m_Sign == '-' )
          DivEx[i].m_NomEx = -DivEx[i].m_NomEx;
        Den = P.OutPut( Divis[i].m_Den );
        p.clear();
        DivEx[i].m_DenEx = Den.ReductionPoly( p, VarName );
        ex = DivEx[i].m_NomEx / Den + ex;
        }
      if( !( DivEx[0].m_NomEx.Reduce().Constan( d ) && abs( d ) < 0.0000001 ) )
        ex += DivEx[0].m_NomEx;
      ex = MathExpr( new TBinar( '=', ex, Constant( 0 ) ) ).Reduce();
      if( !ex.Equal( ex1 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
      TExprs AddMult;
      AddMult.append( Constant( 1 ) );
      CommDen = ToFactors( DivEx[1].m_DenEx );
      MathExpr Mult;
      for( int i = 2; i < DivEx.count(); i++ )
        {
        Mult = ToFactors( DivEx[i].m_DenEx );
        MathExpr CommExp = GetCommon( CommDen, Mult );
        for( int j = 0; j < i-1; j++ )
          AddMult[j] = ( AddMult[j] * Mult ).Reduce();
        AddMult.append( CommDen.Reduce() );
        CommDen = MultExprs( CommExp, MultExprs( CommDen, Mult ) ).Reduce();
        }
      Mult = ExpandExpr( DivEx[1].m_NomEx * AddMult[0] );
      Nom = Mult;
      ex = Mult / CommDen;
      for( int i = 2; i < DivEx.count(); i++ )
        {
        Mult = ExpandExpr( DivEx[i].m_NomEx * AddMult[i - 1] );
        Nom += Mult;
        ex = Mult / CommDen + ex;
        }
      if( !( DivEx[0].m_NomEx.Reduce().Constan( d ) && abs( d ) < 0.0000001 ) )
        {
        Mult = ExpandExpr( DivEx[0].m_NomEx * CommDen );
        Nom += Mult;
        ex += Mult / CommDen;
        }
      ex = new TBinar( '=', ex, Constant( 0 ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }
    else
      {
      Nom = P.OutPut( Divis[0].m_Nom );
      CommDen = P.OutPut( Divis[0].m_Den );
      }

    p.clear();
    if( GetFactorCount( Nom, VarName ) < 2 )
      Nom = Nom.ReductionPoly( p, VarName );
    ex = new TBinar( '=', ( ( Nom ) / ( CommDen ) ), Constant( 0 ) );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    int DegNom = 0;
    if( s_FactorCount < 2 )
      {
      DegNom = s_DegPoly;
      for( ; DegNom > 0 && p[DegNom].Reduce().Constan( d ) && abs( d ) < 0.0000001; DegNom-- );
      if( DegNom > 4 )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      }
    ex = new TBinar( '=', Nom, Constant( 0 ) );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    ex = new TBinar( msNotequal, CommDen, Constant( 0 ) );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    q.clear();
    CommDen.ReductionPoly( q, VarName );
    DegDen = s_DegPoly;
    for( ; DegDen > 0 && p[DegDen].Reduce().Constan( d ) && abs( d ) < 0.0000001; DegDen-- );
    if( s_FactorCount >= 2 )
      do
        {
        i = s_RootCount;
        bool OldAccumulate = TSolutionChain::sm_SolutionChain.m_Accumulate;
        TSolutionChain::sm_SolutionChain.m_Accumulate = false;
        ex = GetFactorEquation( VarName );
        TSolutionChain::sm_SolutionChain.m_Accumulate = OldAccumulate;
        if( ex.IsEmpty() )
          {
          s_FinalComment = true;
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found" ) );
          return Final();
          }
        if( s_RootCount > i )
          {
          MathExpr x1;
          ex.Binar( '=', ex1, x1 );
          CheckRoot( x1, s_RootCount );
          }
        else
          {
          i = s_RootCount;
          s_FinalComment = false;
          bool InterRes = TSolutionChain::sm_InterimResult;
          TSolutionChain::sm_InterimResult = true;
          if( !s_CalcOnly )
            {
            s_CalcOnly = true;
            ex1 = CalcDetQuEqu( ex.WriteE(), VarName );
            s_RootCount = i;
            s_CalcOnly = false;
            }
          if( s_CalcOnly || !ex1.IsEmpty() )
            ex1 = CalcDetQuEqu( ex.WriteE(), VarName );
          if( ex1.IsEmpty() )
            ex1 = CalcDetBiQuEqu( ex.WriteE(), VarName );
          TSolutionChain::sm_InterimResult = InterRes;
          s_FactorizedSolving = !ex1.IsEmpty() && !CastPtr( TL2exp, ex1 )->First().isNull();
          if( !s_FactorizedSolving )
            throw  ErrParser( "Wrong type of equation!", peNoSolvType );
          for( PExMemb memMemb = CastPtr( TL2exp, ex1 )->First(); !memMemb.isNull(); memMemb = memMemb->m_pNext )
            CheckRoot( memMemb->m_Memb, ++i );
          }
        } while( true );
        RootNum = 0;
        if( DegNom == 4 )
          {
          ex = Nom.ReduceToMult();
          if( ex.Equal( Nom ) )
            throw  ErrParser( "Wrong type of equation!", peNoSolvType );
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex, Constant( 0 ) ) );
          FindRoots( ex );
          return Final();
          }

        if( DegNom == 3 )
          if( ( p[0].Constan( d ) && abs( d ) < 0.0000001 ) )
            {
            RootNum++;
            MathExpr x1 = Constant( 0 );
            ex = new TBinar( '=', Variable( VarName + "_" + NumberToStr( RootNum ) ), x1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            CheckRoot( x1, RootNum );
            MathExpr pp = p[0];
            for( i = 0; i <= 2; i++ )
              p[i] = p[i + 1];
            p[3] = pp;
            x1 = Variable( VarName );
            ex = p[2] * ( x1 ^ 2 );
            if( !( p[1].Constan( d ) && abs( d ) < 0.0000001 ) )
              if( d > 0 )
                ex += p[1] * x1;
              else
                ex -= Constant( abs( d ) ) * x1;
            if( !( p[0].Constan( d ) && abs( d ) < 0.0000001 ) )
              if( d > 0 )
                ex += p[0];
              else
                ex -= Constant( abs( d ) );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex, Constant( 0 ) ) );
            }
          else
            if( p[1].Constan( d ) && abs( d ) < 0.0000001 && p[2].Constan( d ) && abs( d ) < 0.0000001 )
              {
              RootNum++;
              MathExpr x1 = ( -p[0] ).Root( 3 ).Reduce();
              ex = new TBinar( '=', Variable( VarName + "_" + NumberToStr( RootNum ) ), x1 );
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              CheckRoot( x1, RootNum );
              if( s_FinalComment )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found" ) );
              return Final();
              }
            else
              throw  ErrParser( "Wrong type of equation!", peNoSolvType );

        if( p[2].Constan( d ) && abs( d ) < 0.0000001 )
          {
          if( p[1].Reduce().Constan( d ) && abs( d ) < 0.0000001 )
            if( p[0].Reduce().Constan( d ) && abs( d ) < 0.0000001 )
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfiniteSetSol", "Infinite set of solutions" ) );
            else
              {
              PNode q1 = P.GetExpression( p[0].WriteE() );
              if( !NIsConst( q1 ) )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfOrNotSolLeft1", " if( an expression at the left side is equal to zero,\n" ) +
                X_Str( "MInfOrNotSolLeft2", "the equation has infinite set of solutions,\n" ) +
                X_Str( "MInfOrNotSolLeft3", "otherwise equation has no solutions." ) );
              else
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
              }
          else
            {
            RootNum++;
            MathExpr x1 = ( -p[0] / p[1] ).Reduce().Reduce();
            if( RootNum == 1 )
              ex = new TBinar( '=', Variable( VarName ), x1 );
            else
              ex = new TBinar( '=', Variable( VarName + "_" + NumberToStr( RootNum ) ), x1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex, "", true );
            PNode q1 = P.GetExpression( p[1].WriteE() );
            if( !NIsConst( q1 ) )
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, p[1], Constant( 0 ) ) );
            if( RootNum == 1 )
              CheckRoot( x1, 0 );
            else
              CheckRoot( x1, RootNum );
            }
          }
        else
          {
          MathExpr D1 = ( p[1] ^ 2 ) - Constant( 4 ) * p[2] * p[0];
          MathExpr D2 = D1.Reduce();
          if( D2.Eq( D1 ) )
            ex = new TBinar( '=', Variable( "d" ), D1 );
          else
            {
            MathExpr D3 = D2.Reduce();
            if( D2.Eq( D3 ) )
              ex = new TBinar( '=', Variable( "d" ), new TBinar( '=', D1, D2 ) );
            else
              ex = new TBinar( '=', Variable( "d" ), new TBinar( '=', D1, new TBinar( '=', D2, D3 ) ) );
            D2 = D3;
            }
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          D1 = D2;
          if( D1.Constan( d ) && ( abs( d ) < 0.0000001 ) )
            {
            RootNum++;
            MathExpr x1 = ( -p[1] / ( Constant( 2 ) * p[2] ) ).Reduce().Reduce();
            ex = new TBinar( '=', new TVariable( true, VarName + "_" + NumberToStr( RootNum ) ),
              new TBinar( '=', new TVariable( true, VarName + "_" + NumberToStr( RootNum + 1 ) ), x1 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            PNode q1 = P.GetExpression( p[2].WriteE() );
            if( !NIsConst( q1 ) )
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, p[2], Constant( 0 ) ) );
            CheckRoot( x1, RootNum );
            }
          else
            {
            if( D1.Unarminus( ex ) )
              ex = CreateComplex( Constant( 0 ), ex.Root( 2 ) );
            else
              ex = D1.Root( 2 );
            ex1 = Constant( 2 ) * p[2];
            RootNum++;
            op1 = ( -p[1] - ex ) / ex1;
            MathExpr x1 = op1.Reduce().Reduce().Reduce();
            ReduceExpr( x1 );
            x1 = ExpandExpr( x1 );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', new TVariable( true, VarName + "_" + NumberToStr( RootNum ) ),
              new TBinar( '=', op1, x1 ) ), "", true );
            CheckRoot( x1, RootNum );
            RootNum++;
            op2 = ( -p[1] + ex ) / ex1;
            MathExpr x2 = op2.Reduce().Reduce();
            ReduceExpr( x2 );
            x2 = ExpandExpr( x2 );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', new TVariable( true, VarName + "_" + NumberToStr( RootNum ) ),
              new TBinar( '=', op2, x2 ) ), "", true );
            CheckRoot( x2, RootNum );
            if( RootNum > 0 )
              {
              PNode q1 = P.GetExpression( p[2].WriteE() );
              if( !NIsConst( q1 ) )
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, p[2], Constant( 0 ) ) );
              if( s_FinalComment )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found" ) );
              }
            }
          }
        return Final();
    }
  catch( ErrParser E )
    {
    if( !s_FinalComment )
      throw E;
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Final();
  }

  void Solver::SimplifyExpand()
  {
  MathExpr Simple;
  std::function<void( const MathExpr& )> SimplifySumm = [&] ( const MathExpr& exp )
    {
    MathExpr Left = exp, Right, Result;
    bool Next = exp.Multp( Left, Right );
    if( IsType( TSumm, Left ) )
       Result = Left.Reduce();
     else
       Result = Left;
    if( Simple.IsEmpty() )
      Simple = Result;
    else
      Simple *= Result;
    if( Next ) SimplifySumm( Right );
    };

  bool OldNoRootReduce = s_NoRootReduce;
  s_NoRootReduce = false;
  MathExpr Right = RightExpr( m_Expr );
  SimplifySumm( Right );
  if( !Simple.Eq(Right ) )
    m_Expr = new TBinar( '=', m_Expr, Simple );
  MathExpr NewSimple = Simple.Reduce();
  if( !Simple.Eq( NewSimple ) )
    m_Expr = new TBinar( '=', m_Expr, NewSimple );
  s_NoRootReduce = OldNoRootReduce;
  }

void TSolvReToMult::Solve()
  {
  bool OldNoRootReduce = s_NoRootReduce;
  s_NoRootReduce = true;
  try
    {
    m_Expr = m_Expr.ReToMult();
    }
  catch( ErrParser Err )
    {
    s_NoRootReduce = OldNoRootReduce;
    throw Err;
    }
  s_NoRootReduce = OldNoRootReduce;
  SimplifyExpand();
  s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
  }

void TSolvSubSqr::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  try
    {
    m_Expr = SubtSq( m_Expr );
    }
  catch( ErrParser Err )
    {
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  SimplifyExpand();
  s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
  }

MathExpr SqSbSm( const MathExpr& exi )
  {
  MathExpr pow = Constant( 2 ), a, b, SbSm, pw, exi1, Result;
  int st;
  if( ( exi.Subtr( a, b ) || exi.Summa( a, b ) ) || ( exi.Power( SbSm, pw ) && ( SbSm.Subtr( a, b ) || SbSm.Summa( a, b ) ) && 
    pw.Cons_int( st ) && st == 2 ) )
    {
    if( exi.Subtr( a, b ) || exi.Summa( a, b ) )
      {
      SbSm = exi;
      exi1 = exi^pow;
      }
    else
      exi1 = exi;
    MathExpr a_2 = a^pow;
    MathExpr b_2 = b^pow;
    MathExpr ab2 = a * b;
    MathExpr ab2r = (pow * ab2.Reduce());
    ab2 *= pow;
    MathExpr P, Pr;
    if( SbSm.Subtr( a, b ) )
      {
      P = a_2 - ab2;
      Pr = a_2.Reduce() - ab2r.Reduce();
      }

    if( SbSm.Summa( a, b ) )
      {
      P = a_2 + ab2;
      Pr = a_2.Reduce() + ab2r.Reduce();
      }

    P += b_2;
    Pr += b_2.Reduce();
    MathExpr sq = new TBinar( '=', exi1, P );
    if(Pr.Eq(P ) )
      Result = sq;
    else
      {
      MathExpr sqr = new TBinar( '=', sq, Pr );

      MathExpr ex12 = Pr.Reduce(true);
      if(Pr.Eq( ex12 ) )
        Result = sqr;
      else
        Result = new TBinar( '=', sqr, ex12 );
      }
    }
  else
    {
    if( !s_CalcOnly )
      s_LastError = X_Str( "MCannotCalculate", "Cannot calculate!" );
    s_GlobalInvalid = true;
    return exi;
    }
  s_XPStatus.SetMessage(X_Str( "MCalculated", "Calculated!" ) );
  return Result;
  }

void TSolvSqrSubSum::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  try
    {
    m_Expr = SqSbSm( m_Expr );
    }
  catch( ErrParser Err )
    {
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    s_LastError = X_Str("MCannotCalculate", "Cannot calculate!");
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  SimplifyExpand();
  }

void TSolvSumCub::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  TPowr::sm_Factorize = false;
  MathExpr ExpOld = m_Expr;
 // TExpr::sm_FullReduce = true;
  try
    {
    m_Expr = SummCube( ExpOld );
    }
  catch( ErrParser Err )
    {
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    TExpr::sm_FullReduce = false;
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  TPowr::sm_Factorize = true;
  if( !ExpOld.Eq( m_Expr ) ) SimplifyExpand();
  TExpr::sm_FullReduce = false;
  s_LastError = X_Str( "MCannotCalculate", "Cannot calculate!" );
  }

void TSolvExpand::Solve()
  {
  double OldAcc = s_Precision;
  s_Precision = 0.0000000000001;
  bool OldRootToPower = s_RootToPower;
  bool OldExpandPower = s_ExpandPower;
  s_ExpandPower = true;
  s_NoRootReduce = true;
  s_ExpandDetailed = !sm_TestMode;
  try
    {
    m_Expr = ExpandOpRes( m_Expr );
    if( !m_OldExpr.Eq( m_Expr ) )
      {
      MathExpr Op1, Op2, Op3;
      if( !m_Expr.Binar( '=', Op1, Op2 ) ) Op2 = m_Expr;
      if( sm_TestMode )
        {
        s_ExpandPower = false;
        TSolvReToMult ReToMult;
        ReToMult.SetExpression( Op2 );
        MathExpr Result( ReToMult.Result() );
        if( Result.Binar( '=', Op1, Op3 ) ) Result = Op3;
        m_Expr = new TBinar( '=', m_OldExpr, new TBinar( '=', Op2, Result ) );
        if( Result.Equal( m_OldExpr ) )
          s_XPStatus.SetMessage( QString("true") );
        else
          s_XPStatus.SetMessage( QString( "false" ) );
        }
      else
        {
        s_NoRootReduce = false;
        s_RootToPower = true;
        MathExpr exTmp = Op2.Reduce();
        if( !exTmp.Eq( Op2 ) )
          m_Expr = new TBinar( '=', m_Expr, exTmp );
        }
      }
    }
  catch( ErrParser Err )
    {
    s_NoRootReduce = true;
    s_ExpandDetailed = true;
    s_RootToPower = OldRootToPower;
    s_ExpandPower = OldExpandPower;
    s_Precision = OldAcc;
    throw Err;
    }
  s_NoRootReduce = true;
  s_ExpandDetailed = true;
  s_RootToPower = OldRootToPower;
  s_ExpandPower = OldExpandPower;
  s_Precision = OldAcc;
  s_LastError = X_Str( "MCanNotExpand", "I can`t expand it!" );
  SimplifyExpand();
  }

void TTrinom::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  try
    {
    m_Expr = MakeTrinom( m_Expr );
    }
  catch( ErrParser Err )
    {
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  SimplifyExpand();
  s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
  }

void TSolvCubSubSum::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  bool OnlyOld = s_CalcOnly;
  s_NoRootReduce = true;
  try
    {
    m_Expr = CuSbSm( m_Expr );
    if( !s_GlobalInvalid )
      {
      MathExpr exp = RightExpr( m_Expr );
      s_CalcOnly = true;
      MathExpr exp2 = ExpandOpRes( exp );
      if( !exp2.Eq( exp ) )
        m_Expr = new TBinar( '=', new TBinar( '=', m_Expr, new TNewLin ), RightExpr( exp2 ) );
      }
    }
  catch( ErrParser Err )
    {
    s_NoRootReduce = true;
    s_CalcOnly = OnlyOld;
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  s_NoRootReduce = true;
  s_GlobalInvalid = false;
  s_CalcOnly = OnlyOld;
  SimplifyExpand();
  s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
  }

void TSolvSubCub::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  try
    {
    m_Expr = SubtCube( m_Expr );
    }
  catch( ErrParser Err )
    {
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  SimplifyExpand();
  s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
  }

void Alg1Calculator::Solve()
  {
  bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
  MathExpr::sm_NoReduceByCompare = true;
  s_GlobalInvalid = false;

  auto TestSolve = [&]( Solver &Solv )
    {
    s_GlobalInvalid = false;
    Solv.SetExpression( m_Expr );
    MathExpr exr = Solv.Result();
    bool Result = !s_GlobalInvalid && !s_CheckError && !m_Expr.Eq( exr );
    s_NoRootReduce = true;
    MathExpr ex1, ex2;
    int N;
    if( Result )
      if( m_Expr.Power( ex1, ex2 ) && ex2.Cons_int( N ) && ( N == 2 || N == 3 ) )
        m_Expr = new TBinar( '=', m_Expr, ExpandExpr( exr.Binar()->Right() ) );
      else
        m_Expr = exr;
    return Result;
    };

  try
    {
    if( m_Expr->HasComplex() )
      SimplifyExpand();
    else
      {
      TSolvReToMult Retomult;
      TSolvExpand Expand;
      TSolvSqrSubSum SSS;
      if( s_GlobalInvalid || s_CheckError || ( !TestSolve( Retomult ) && !TestSolve( Expand ) && !TestSolve( SSS ) ) )
        s_LastError = X_Str( "MCannotCalculate", "Cannot calculate!" );
      }
    }
  catch( ErrParser Err )
    {
    MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
    throw Err;
    }
  MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
  }

void TLg::Solve()
  {
  m_Expr = m_Expr.CalcFunc( "lg" );
  }

void TSin::Solve()
  {
  m_Expr = m_Expr.CalcFunc( "sin" );
  }

void TCos::Solve()
  {
  m_Expr = m_Expr.CalcFunc( "cos" );
  }

void TTan::Solve()
  {
  m_Expr = m_Expr.CalcFunc( "tan" );
  }

void TLn::Solve()
  {
  m_Expr = m_Expr.CalcFunc( "ln" );
  }

void TDegRad::Solve()
  {
  m_Expr = CalcDeg2Rad( m_Expr );
  }

void TRadDeg::Solve()
  {
  m_Expr = CalcRad2Deg( m_Expr );
  }

void TSciCalc::Solve()
  {
 /*
  PNode eq;
  MathExpr ex1, ex2, arg, ex, ex3, arg1, arg2 ;
  MathExpr op1,op2;
  QByteArray name, name1, name2 ;
  double v,OldAcc;
  bool RootToPowerOld ;
  memory_switch=SWcalculator;
  TanCotError = false;
  OldAcc=Precision;
  Precision=0.0000000000001;
  Result = false;
  RootToPowerOld = RootToPower;
   if( Source!="" )
  {
  Result = true;
  try{
  eq=AnyExpr(FullPreProcessor(Source,"x"));

  ex1=OutPut(eq);
   if( TrigonomSystem == tsDeg )
  {
  name = ex1.WriteE;
  Source = PiVar2Pi180( name );
   if( name != Source )
  {
  FreeTree( eq );
  eq = AnyExpr( Source );
  ex1 = OutPut( eq );
  };
  };

  FreeTree(eq);
  try{
   if( ex1.Binar("==",op1,op2) )
  {
   if( ! op1.Variab(name) )
  throw  ErrParser( "Syntax error!", peSyntacs);
   if( Pos(msPi,op2.WriteE)>0 )
  ex=OutPut(AnyExpr(PiVar2PiConst(op2.WriteE))).Reduce()
  else
  ex=op2.Reduce();

   if( PutAnswer )
  Answer = ex1.Clone();

   if( ex.Constan(v) )
  {
   if( ! PutAnswer )
  store_var(name,ex);

  OutWin.Addexp(ex1.Clone());
  OutWin.Addcomm( X_str( "XPStatMess","MVarDefined", "Variable defined"));
  }
  else
  OutWin.Addexp(ex1.Clone());
  }
  else
  {
  RecursionDepth = 2;
  ReductionMustBeContinued = false;
  ex2=ex1.Reduce();
   if( ReductionMustBeContinued )
  {
   if( IsConstType( TLexp, ex1 ) )
  ex = ex2.Reduce()
  else
  ex =  new TBinar( "==", ex2.Clone(), ex2.Reduce() );
  ex2 = ex;
  };
   if( Pos(msPi,ex2.WriteE)>0 )
  {
  ex=OutPut(AnyExpr(PiVar2PiConst(ex2.WriteE))).Reduce();
  ex2=ex
  };

  RootToPower = true;
   if( ex1.Eq( ex2 ) )
  {
  ex2 = ex1.Reduce();
  };
   if( ex1.Eq(ex2) && ex1.Divis( op1, op2 ) && op1.Funct( name1, arg1 ) && op2.Funct( name2, arg2 ) && arg1.Eq( arg2 ) )
   if( ( name1 == "sin" ) && ( name2 == "cos" ) )
  {
  ex2 = Function( ( "tan" ), ( arg1.Clone() ) );
  }
  else
   if( ( name1 == "cos" ) && ( name2 == "sin" ) )
  {
  ex2 = Function( ( "cot" ), ( arg1.Clone() ) );
  };
   if( ex1.Eq(ex2) && ex1.Subtr( op1, op2 ) && IsConst( op1, 1 ) && op2.Power( op1, op2 ) && IsConst( op2, 2 ) && op1.Funct( name, arg ) )
   if( name == "sin" )
  {
  ex2 = (( Function( ( "cos" ), ( arg.Clone() ) ) ) ^ ( 2 ));
  }
  else
   if( name == "cos" )
  {
  ex2 = (( Function( ( "sin" ), ( arg.Clone() ) ) ) ^ ( 2 ));
  };
   if( ex1.Eq( ex2 ) )
  {
  Result = false;
  OutWin.Addexp(ex1.Clone());
   if( PutAnswer )
  Answer = ex2.Clone()
  }
  else
   if( (IsConstType( TLexp, ex1 )) || (IsConstType( TSyst, ex1 )) )
  {
  OutWin.Addexp(ex1.Clone());
  OutWin.Addexp(ex2.Clone());
   if( PutAnswer )
  Answer = ex2.Clone();
  }
  else
  {
   if( Pos( msBigFi, ex1.WriteE ) > 0 )
  ex = ex2.Clone()
  else
  {
   if( ( IsConstType( TConstant, ex2 ) ) ) TConstant( ex2 ).IsE = false;
  ex= new TBinar("==",ex1.Clone(),ex2.Clone());
  ex3 = ex2.Reduce();
   if( ex3.Eq(ex2) )
  else
  ex =  new TBinar( "==", ex, ex3 );
  };
  OutWin.Addexp(ex.Clone());
   if( PutAnswer )
  Answer = ex.Clone();
  };

  };
   if( ! CalcOnly ) WEditor.XPEditor1.Clear;
  except
  Result = false;
   if( TanCotError && ex1.Funct(name,arg) && ((name=="tan") || (name=="cot")))
  {
  Result = true;
  ex= new TBinar("==",ex1.Clone(),Variable( ( msInfinity ) ));
  OutWin.Addexp( ex.Clone() );
   if( PutAnswer )
  Answer = ex.Clone();
  OutWin.Addcomm(X_Str("XPSimpleCalcMess","MCalced","Calculated!"));
   if( ! CalcOnly ) WEditor.XPEditor1.Clear;
  }
  else {
  OutWin.Addexp( ex1.Clone() );
  OutWin.Addcomm( LastError );

   if( PutAnswer )
  Answer =  new TStr( LastError );

  }
  };
  except
  on E: ErrParser )
  {
  OutWin.AddExp( new TStr(""));
  OutWin.Addcomm(X_Str("XPSimpleCalcMess", E.MsgName, E.Message ));
  }
  };
  };
  RootToPower = RootToPowerOld;
  memory_switch=SWtask;
  Precision=OldAcc;
  OutWin.Show;
  WEditor.XPEditor1.RefreshXPE;
  };


*/

  MathExpr Old = m_Expr;
  m_Expr = m_Expr.Simplify();
  if( m_Expr.IsEmpty() || m_Expr.Eq( Old ) || (!m_Expr.ConstExpr() && !Old.HasMatrix() ) )
    {
    s_LastError = X_Str( "MCanNotReduced", "can not be reduced!" );
    m_Expr.Clear();
    return;
    }
  m_Expr = new TBinar( '=', Old, m_Expr );
  }

void Log1Eq::Solve()
  {
  m_Expr = new TBool( CalcLog1Eq( m_Expr.WriteE() ) );
  }

void SysInEq::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcSysInEq( m_Expr.WriteE() ) );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void SysInEqXY::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcSysInEqXY( m_Expr.WriteE() ) );
  }

void RatInEq::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcRatInEq( m_Expr.WriteE() ) );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void ExpEq::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcExpEq( m_Expr.WriteE() ) );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void MakeSubstitution::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcSubstitution( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void SolveLinear::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcLinear( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void MakeExchange::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcExchange( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcSimpleTrigoEq::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = false;
  m_Expr = new TBool( CalcSimpleTrigo( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcSinCosEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = false;
  m_Expr = new TBool( CalcSinCosEqu( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcTrigoEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = false;
  m_Expr = new TBool( CalcTrigoEqu( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcHomogenTrigoEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = false;
  m_Expr = new TBool( CalcHomogenTrigoEqu( m_Expr.WriteE() ) );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvQuaEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcRootsQuEqu( m_Expr->WriteE() ) );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvDetQuaEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  bool Result = true;
  try
    {
    Lexp LResult = CalcDetQuEqu( m_Expr->WriteE() );
    Result = !LResult.IsEmpty()  && LResult.Count() > 0;
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  m_Expr = new TBool(Result);
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcDetBiQuEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  bool Result = true;
  try
    {
    Lexp LResult = CalcDetBiQuEqu( m_Expr->WriteE() );
    Result = !LResult.IsEmpty()  && LResult.Count() > 0;
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  m_Expr = new TBool( Result );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvFractRatEq::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  bool Result = true;
  try
    {
    Lexp LResult = FractRatEq( m_Expr->WriteE() );
    Result = !LResult.IsEmpty()  && LResult.Count() > 0;
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  m_Expr = new TBool( Result );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcIrratEq::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  bool Result = true;
  try
    {
    Lexp LResult = CalcIrratEq( m_Expr->WriteE() );
    Result = !LResult.IsEmpty()  && LResult.Count() > 0;
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  m_Expr = new TBool( Result );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcPolinomEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  bool Result = true;
  try
    {
    Lexp LResult = CalcPolinomEqu( m_Expr->WriteE() );
    Result = !LResult.IsEmpty()  && LResult.Count() > 0;
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  m_Expr = new TBool( Result );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvDisQuaEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcDiscrim( m_Expr->WriteE() ) );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvCalcEquation::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = new TBool( CalcEquation( m_Expr->WriteE() ) );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvDetVieEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  m_Expr = DetVieEqu( m_Expr );
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

void TSolvDetLinEqu::Solve()
  {
  bool OldConstToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = true;
  bool Result = true;
  try
    {
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr );
    Lexp Result = CalcDetLinEqu( m_Expr.WriteE() );
    if( Result.IsEmpty() ) throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    PExMemb pMemb = Result.First();
    for( ; !pMemb->m_pNext.isNull(); pMemb = pMemb->m_pNext )
      TSolutionChain::sm_SolutionChain.AddExpr( pMemb->m_Memb );
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Linear Equation is Solved!" ) );
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  m_Expr = new TBool( Result );
  TExpr::sm_FullReduce = false;
  TConstant::sm_ConstToFraction = OldConstToFraction;
  }

bool CalcSimpleTrigo( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;
  double OldPrecision = s_Precision;
  bool OldToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = false;
  s_Precision = 0.0000001;
  try
    {
    QByteArray  VarName = "x";
    Lexp Cond = new TLexp;
    Parser P( &Cond );
    bool IsName, MultUnknown;
    PNode eq;
    while( true )
      {
      eq = P.Equation(P.FullPreProcessor( Source, VarName ), VarName, IsName, MultUnknown );
      if( IsName || VarName == "y" ) break;
      VarName = "y";
      }
    MathExpr ex = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    if( IsName && !MultUnknown && IsFuncEqu( eq ) )
      {
      P.OutPut( eq );
      try
        {
        ex = SolutionSimpleEquaion( eq, VarName );
        }
      catch( ErrParser E )
        {
        switch( E.ErrStatus() )
          {
          case peFunctn:
          case peOpertn: throw  ErrParser( "Wrong type of equation!", peNoSolvType );
          default:
            throw  ErrParser( "No Solutions!", peNoSolv );
          }
        }
      MathExpr ex0 = CastPtr( TLexp, ex )->Last()->m_Memb;
      MathExpr op1, op2;
      ex0.Binar( '=', op1, op2 );
      MathExpr ex1 = new TBinar( '=', op1, ExpandExpr( op2 ).Reduce() );
      if( !ex0.Equal( ex1 ) ) CastPtr( TLexp, ex )->Addexp( ex1 );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      if( s_PutAnswer )
        {
        s_Answer = CastPtr( TLexp, ex )->Last()->m_Memb;
        MathExpr ZeroEx = Constant( 0 );
        MathExpr s_AnswerTrigo = s_Answer.Substitute( "k", ZeroEx );
        s_AnswerTrigo = s_AnswerTrigo.Reduce();
        }
      Cond.Addexp( new TBinar( msAddition, Variable( "k" ), Variable( "Z" ) ) );
      TSolutionChain::sm_SolutionChain.AddExpr( Cond, X_Str( "MTrigoSolved", "Trigonometrical equation is solved." ) );
      }
    else
      throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    return false;
    }
  s_Precision = OldPrecision;
  TConstant::sm_ConstToFraction = OldToFraction;
  return true;
  }

bool NotOnlyInFunc(PNode p )
  {
  if(p == nullptr ) return false;
  switch(p->m_OpSign )
    {
    case 'F': return false;
    case 'v':
      if(p->m_Info.toUpper() == "X" ) return true;
      return false;
    }
  return NotOnlyInFunc(p->m_pLeft ) || NotOnlyInFunc(p->m_pRight );
  }

PNode Replace(PNode Expr, const QByteArray& OldName, PNode Term, TNodes& Nodes, int& SubstCount )
  {
  std::function<PNode(PNode )> SubstInExpr = [&](PNode Expr )
    {
    PNode Result = nullptr;
    if( Expr == nullptr ) return Result;
    if( Expr->m_Info == OldName && ( Expr->m_OpSign != 'F' || Expr->m_IsRight ) )
      {
      PNode Result = Clone( Term );
      if( OldName == "log" )
        {
        Nodes[++SubstCount] = Expr->m_pRight->m_pLeft;
        Nodes[++SubstCount] = Expr->m_pRight->m_pRight;
        }
      else 
        Nodes[++SubstCount] = Expr->m_pRight;
      return Result;
      }
    Expr->m_pLeft = SubstInExpr( Expr->m_pLeft );
    Expr->m_pRight = SubstInExpr( Expr->m_pRight );
    return Expr;
    };

  SubstCount = 0;
  return SubstInExpr( Expr );
  }

bool Error( const MathExpr& arg, TNodes& p, int n )
  {
  for( int i = 1; i <= n; i++ )
    if( !arg.Equal(p[i]->m_pOwner->OutPut(p[i] ) ) ) return true;
  return false;
  }

bool CalcTrigoEqu( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;
  QByteArrayList FuncName;
  FuncName << "sin" << "cos" << "tan" << "cot";
  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  int OldDegPoly = s_DegPoly;
  s_DegPoly = 3;
  bool Result = true;

  auto Final = [&] ()
    {
    s_DegPoly = OldDegPoly;
    s_Precision = OldPrecision;
    return Result;
    };

  auto ErrResult = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MEnterTrigoQuEq", "Enter a*Trigo(x)^2+b*Trigo(x)+c==0 equation" ) );
    Result = false;
    return Final();
    };

  try
    {
    QByteArray  VarName = "x";
    Parser P;
    PNode eq;
    while( true )
      {
      bool IsName, Mult;
      eq = P.Equation(P.FullPreProcessor( Source, VarName ), VarName, IsName, Mult );
      if( IsName || ( VarName == "y" ) ) break;
      VarName = "y";
      }
    if( TestFrac( eq ) ) return ErrResult();
    MathExpr ex = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr exs = ex;
    if( eq->m_OpSign == '=' )
      {
      eq = Sub( eq->m_pLeft, eq->m_pRight );
      if( NotOnlyInFunc( eq ) ) return ErrResult();
      int Count = 0;
      int Num = -1;
      for( int i = 0; i < 4; i++ )
        {
        PNode eq1 = Clone( eq );
        TNodes p;
        int n;
        Replace( eq1, FuncName[i], nullptr, p, n );
        if( n > 0 )
          {
          Count++;
          Num = i;
          }
        }
      if( Count != 1 ) return ErrResult();
      QByteArray FName = FuncName[Num];
      PNode eq1 = Clone( eq );
      TNodes p;
      int n;
      eq1 = Replace( eq1, FName, NewNode( &P, "y" ), p, n );
      MathExpr arg = P.OutPut(p[1] );
      if( Error( arg, p, n ) ) return ErrResult();
      ex = P.OutPut( eq1 );
      if( ex.Splitted() ) ex = DelSplitted( ex );
      MathExpArray q;
      MathExpr ex1 = ex.ReductionPoly( q, "y" );
      MathExpr a = q[2];
      MathExpr b = q[1];
      MathExpr c = q[0];
      double r;
      if( !( q[3].Constan( r ) && abs( r ) < 0.0000001 ) ) return ErrResult();
      if( a.Constan( r ) && abs( r ) < 0.0000001 ) return ErrResult();
      ex = ex1.Substitute( "y", Function( FName, arg ) );
      ex = new TBinar( '=', ex, Constant( 0 ) );
      if( !exs.Eq( ex ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
      ex = new TBinar( '=', Function( FName, arg ), Variable( "y" ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      ex = new TBinar( '=', ex1, Constant( 0 ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      MathExpr D1 = ( b ^ 2 ) - Constant( 4 ) * a * c;
      MathExpr D = D1.Reduce();
      if( D.Equal( D1 ) )
        ex = new TBinar( '=', Variable( "D" ), D );
      else
        {
        MathExpr D2 = D.Reduce();
        if( D2.Equal( D ) )
          ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, D ) );
        else
          ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, new TBinar( '=', D, D2 ) ) );
        D = D2;
        }
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      Lexp Cond = new TLexp;
      Parser PCond( &Cond );
      if( s_PutAnswer )
        {
        s_Answer = new TL2exp;
        s_AnswerTrigo = new TL2exp;
        }
      int nom, den;
      bool IsName, Mult;
      if( D.Constan( r ) && abs( r ) < 0.0000001 )
        {
        MathExpr y1 = ( -b / ( Constant( 2 ) * a ) ).Reduce().Reduce();
        ex = new TBinar( '=', new TVariable( true, "y_1" ), new TBinar( '=', new TVariable( true, "y_2" ), y1 ) );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        eq1 = PCond.GetExpression( a.WriteE() );
        if( !NIsConst( eq1 ) )
          Cond.Addexp( new TBinar( msNotequal, a, Constant( 0 ) ) );
        ex = new TBinar( '=', Function( FName, arg ), y1 );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        eq = PCond.Equation( ex.WriteE(), "x", IsName, Mult );
        try
          {
          ex = SolutionSimpleEquaion( eq, "x" );
          }
        catch( ErrParser )
          {
          throw  ErrParser( "No Solutions!", peNoSolv );
          }
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        if( s_PutAnswer )
          {
          CastPtr( TL2exp, s_Answer )->Addexp( CastPtr( TL2exp, ex )->Last()->m_Memb );
          MathExpr ZeroEx = Constant( 0 );
          MathExpr at2 = CastPtr( TL2exp, ex )->Last()->m_Memb.Substitute( "k", ZeroEx );
          CastPtr( TL2exp, s_AnswerTrigo )->Addexp( at2.Reduce() );
          }
        Cond.Addexp( new TBinar( msAddition, Variable( "k" ), Variable( "Z" ) ) );
        TSolutionChain::sm_SolutionChain.AddExpr( Cond, X_Str( "MRootsFound", "Roots are found" ) );
        }
      else
        if( !( D.Constan( r ) && r < 0 || ( D.SimpleFrac_( nom, den ) && nom < 0 ) ) )
          {
          ex = D.Root( 2 );
          ex1 = Constant( 2 ) * a;
          MathExpr op1 = ( -b - ex ) / ex1;
          MathExpr y1 = op1.Reduce().Reduce();
          MathExpr op2 = ( -b + ex ) / ex1;
          MathExpr y2 = op2.Reduce().Reduce();
          ex = new TBinar( '=', new TVariable( true, "y_1" ), new TBinar( '=', op1, y1 ) );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          ex = new TBinar( '=', new TVariable( true, "y_2" ), new TBinar( '=', op2, y2 ) );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          eq1 = PCond.GetExpression( a.WriteE() );
          if( !NIsConst( eq1 ) )
            Cond.Addexp( new TBinar( msNotequal, a, Constant( 0 ) ) );
          ex = new TBinar( '=', Function( FName, arg ), y1 );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          bool Solv = false;
          eq = PCond.Equation( ex.WriteE(), "x", IsName, Mult );
          try
            {
            ex = SolutionSimpleEquaion( eq, "x" );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( s_PutAnswer )
              {
              CastPtr( TL2exp, s_Answer )->Addexp( CastPtr( TL2exp, ex )->Last()->m_Memb );
              MathExpr ZeroEx = Constant( 0 );
              MathExpr at2 = CastPtr( TL2exp, ex )->Last()->m_Memb.Substitute( "k", ZeroEx );
              CastPtr( TL2exp, s_AnswerTrigo )->Addexp( at2.Reduce() );
              }
            Solv = true;
            }
          catch( ErrParser )
            {
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
            }
          ex = new TBinar( '=', Function( FName, arg ), y2 );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          eq = PCond.Equation( ex.WriteE(), "x", IsName, Mult );
          try
            {
            ex = SolutionSimpleEquaion( eq, "x" );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( s_PutAnswer )
              {
              CastPtr( TL2exp, s_Answer )->Addexp( CastPtr( TL2exp, ex )->Last()->m_Memb );
              MathExpr ZeroEx = Constant( 0 );
              MathExpr at2 = CastPtr( TL2exp, ex )->Last()->m_Memb.Substitute( "k", ZeroEx );
              CastPtr( TL2exp, s_AnswerTrigo )->Addexp( at2.Reduce() );
              }
            Solv = true;
            }
          catch( ErrParser )
            {
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
            }
          Cond.Addexp( new TBinar( msAddition, Variable( "k" ), Variable( "Z" ) ) );
          if( Solv )
            TSolutionChain::sm_SolutionChain.AddExpr( Cond, X_Str( "MRootsFound", "Roots are found" ) );
          }
        else
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions" ) );
      }
    else
      return ErrResult();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  return Final();
  }

bool CalcHomogenTrigoEqu( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;
  MathExpr op1, op2;
  auto Trinom = [&] ( const MathExpr& a, const MathExpr& ax, const MathExpr& b, const MathExpr& bx, const MathExpr& c, const MathExpr& cx )
    {
    double  r;
    MathExpr Result;
    if( !( a.Constan( r ) && abs( r ) < 0.0000001 ) )
      Result = ( a * ax ).Reduce();
    else
      Result = ( b  * bx ).Reduce();
    if( !( b.Constan( r ) && abs( r ) < 0.0000001 ) && !( a.Constan( r ) && abs( r ) < 0.0000001 ) )
      if( !( b.Unarminus( op1 ) || b.Constan( r ) && r < 0 ) )
        Result += ( b * bx ).Reduce();
      else
        {
        if( b.Constan( r ) )
          op1 = Constant( abs( r ) );
        Result -= ( op1 * bx ).Reduce();
        }
    if( !( c.Constan( r ) && abs( r ) < 0.0000001 ) )
      if( !( c.Unarminus( op1 ) || c.Constan( r ) && r < 0 ) )
        Result += ( c *  cx ).Reduce();
      else
        {
        if( c.Constan( r ) )
          op1 = Constant( abs( r ) );
        Result -= ( op1 * cx ).Reduce();
        }
    return Result;
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  int OldDegPoly = s_DegPoly;
  s_DegPoly = 3;
  bool Result = true;

  auto Final = [&] ()
    {
    s_DegPoly = OldDegPoly;
    s_Precision = OldPrecision;
    return Result;
    };

  auto Error1 = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MEnterHomogenEq", "Enter equation of a type: a*sin(x)^2+b*sin(x)*cos(x)+c*cos(x)^2==0 " ) );
    Result = false;
    return Final();
    };

  auto Error2 = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions" ) );
    Result = false;
    return Final();
    };

  try
    {
    Parser P;
    bool IsName, Mult, Solv;
    int Nom, Den;
    MathExpr y1, y2;
    PNode eq = P.Equation(P.FullPreProcessor( Source, "x" ), "x", IsName, Mult );
    if( TestFrac( eq ) ) return Error1();
    MathExpr first = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( first );
    if( eq->m_OpSign == '=' )
      {
      eq = Sub( eq->m_pLeft, eq->m_pRight );
      if( NotOnlyInFunc( eq ) ) return Error1();
      PNode eq1 = Clone( eq );
      TNodes p;
      int n;
      eq1 = Replace( eq1, "sin", NewNode( &P, "x" ), p, n );
      if( n == 0 ) return Error1();
      MathExpr arg = P.OutPut(p[1] );
      if( Error( arg, p, n ) ) return Error1();
      eq1 = Replace( eq1, "cos", NewNode( &P, "x" ), p, n );
      if( n == 0 ) return Error1();
      if( Error( arg, p, n ) ) return Error1();
      MathExpr ex = P.OutPut( eq1 );
      if( ex.Splitted() ) ex = DelSplitted( ex );
      MathExpArray q;
      ex.ReductionPoly( q, "x" );
      double r;
      for( int i = 0; i <= s_DegPoly; i++ )
        if( i != 2 & !( q[i].Constan( r ) && abs( r ) < 0.0000001 ) )
          return Error1();
      eq1 = Clone( eq );
      eq1 = Replace( eq1, "sin", NewNode( &P, "x" ), p, n );
      eq1 = Replace( eq1, "cos", NewNode( &P, "qwert" ), p, n );
      ex = P.OutPut( eq1 );
      if( ex.Splitted() ) ex = DelSplitted( ex );
      q.clear();
      ex.ReductionPoly( q, "x" );
      MathExpr a = q[2];
      MathExpr b = q[1];
      MathExpr c = q[0];
      ex = b.Substitute( "qwert", Variable( "x" ) );
      q.clear();
      ex.ReductionPoly( q, "x" );
      b = q[1];
      ex = c.Substitute( "qwert", Variable( "x" ) );
      q.clear();
      ex.ReductionPoly( q, "x" );
      c = q[2];
      q.clear();
      if( a.Constan( r ) && abs( r ) < 0.0000001 && c.Constan( r ) && abs( r ) < 0.0000001 ) return Error1();
      MathExpr ex1 = Function( "sin", arg );
      MathExpr ex2 = Function( "cos", arg );
      ex = Trinom( a, ex1 ^ 2, b, ex1 * ex2, c, ex2 ^ 2 );
      MathExpr second = new TBinar( '=', ex, Constant( 0 ) );
      if( !first.Eq( second ) )
        TSolutionChain::sm_SolutionChain.AddExpr( second );
      MathExpr dd;
      QByteArray FName;
      if( !( a.Constan( r ) && abs( r ) < 0.0000001 ) )
        {
        dd = ex2;
        FName = "tan";
        }
      else
        {
        dd = ex1;
        FName = "cot";
        op1 = a;
        a = c;
        c = op1;
        }
      ex = new TBinar( '=', ex / ( dd ^ 2 ), Constant( 0 ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );

      ex1 = Function( FName, arg );
      ex = Trinom( a, ex1 ^ 2, b, ex1, c, Constant( 1 ) );
      ex = new TBinar( '=', ex, Constant( 0 ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );

      ex = new TBinar( '=', Function( FName, arg ), Variable( "y" ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );

      ex1 = Variable( "y" );
      ex = Trinom( a, ex1 ^ 2, b, ex1, c, Constant( 1 ) );
      ex = new TBinar( '=', ex, Constant( 0 ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );

      MathExpr D1 = ( b ^ 2 ) - Constant( 4 ) * a * c;
      MathExpr D = D1.Reduce();
      if( D.Equal( D1 ) )
        ex = new TBinar( '=', Variable( "D" ), D );
      else
        {
        MathExpr D2 = D.Reduce();
        if( D2.Equal( D ) )
          ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, D ) );
        else
          ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, new TBinar( '=', D, D2 ) ) );
        D = D2;
        }
      if( !( c.Constan( r ) && abs( r ) < 0.0000001 || b.Constan( r ) && abs( r ) < 0.0000001 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex );

      Lexp Cond = new TLexp;
      Parser PCond( &Cond );

      if( s_PutAnswer )
        {
        s_Answer = new TL2exp;
        s_AnswerTrigo = new TL2exp;
        }
      if( D.Constan( r ) && abs( r ) < 0.0000001 )
        {
        MathExpr y1 = ( -b / ( Constant( 2 ) * a ) ).Reduce().Reduce();
        ex = new TBinar( '=', new TVariable( true, "y_1" ), new TBinar( '=', new TVariable( true, "y_2" ), y1 ) );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        eq1 = PCond.GetExpression( a.WriteE() );
        if( !NIsConst( eq1 ) )
          Cond.Addexp( new TBinar( msNotequal, a, Constant( 0 ) ) );
        ex = new TBinar( '=', Function( FName, arg ), y1 );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );

        eq = PCond.Equation( ex.WriteE(), "x", IsName, Mult );
        try
          {
          ex = SolutionSimpleEquaion( eq, "x" );
          }
        catch( ErrParser )
          {
          throw  ErrParser( "No Solutions!", peNoSolv );
          }
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        if( s_PutAnswer )
          {
          CastPtr( TL2exp, s_Answer )->Addexp( CastPtr( TL2exp, ex )->Last()->m_Memb );
          MathExpr ZeroEx = Constant( 0 );
          MathExpr at2 = CastPtr( TL2exp, ex )->Last()->m_Memb.Substitute( "k", ZeroEx );
          CastPtr( TL2exp, s_AnswerTrigo )->Addexp( at2.Reduce() );
          }

        Cond.Addexp( new TBinar( msAddition, Variable( "k" ), Variable( "Z" ) ) );
        TSolutionChain::sm_SolutionChain.AddExpr( Cond, X_Str( "MRootsFound", "Roots are found" ) );
        }
      else
        if( !( D.Constan( r ) && r < 0 || D.SimpleFrac_( Nom, Den ) && Nom < 0 ) )
          {
          if( c.Constan( r ) && abs( r ) < 0.0000001 )
            {
            y1 = Constant( 0 );
            y2 = ( -( b / a ) ).Reduce();
            ex = new TBinar( '=', new TVariable( true, "y_1" ), y1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            ex = new TBinar( '=', new TVariable( true, "y_2" ), y2 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            }
          else
            if( b.Constan( r ) && abs( r ) < 0.0000001 )
              {
              ex1 = ( -( c / a ) ).Reduce();
              if( !( ex1.Constan( r ) && r < 0 || ex1.SimpleFrac_( Nom, Den ) && Nom < 0 ) )
                {
                y1 = ( ex1.Root( 2 ) ).Reduce();
                y2 = -y1;
                ex = new TBinar( '=', new TVariable( true, "y_1" ), y1 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex );
                ex = new TBinar( '=', new TVariable( true, "y_2" ), y2 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex );
                }
              else
                return Error2();
              }
            else
              {
              ex = D.Root( 2 );
              ex1 = Constant( 2 ) * a;
              op1 = ( -b - ex ) / ex1;
              y1 = op1.Reduce().Reduce();
              op2 = ( -b + ex ) / ex1;
              y2 = op2.Reduce().Reduce();
              ex = new TBinar( '=', new TVariable( true, "y_1" ), new TBinar( '=', op1, y1 ) );
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              ex = new TBinar( '=', new TVariable( true, "y_2" ), new TBinar( '=', op2, y2 ) );
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              eq1 = PCond.GetExpression( a.WriteE() );
              if( !NIsConst( eq1 ) )
                Cond.Addexp( new TBinar( msNotequal, a, Constant( 0 ) ) );
              }
          ex = new TBinar( '=', Function( FName, arg ), y1 );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          Solv = false;
          eq = PCond.Equation( ex.WriteE(), "x", IsName, Mult );
          try
            {
            ex = SolutionSimpleEquaion( eq, "x" );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( s_PutAnswer )
              {
              CastPtr( TL2exp, s_Answer )->Addexp( CastPtr( TL2exp, ex )->Last()->m_Memb );
              MathExpr ZeroEx = Constant( 0 );
              MathExpr at2 = CastPtr( TL2exp, ex )->Last()->m_Memb.Substitute( "k", ZeroEx );
              CastPtr( TL2exp, s_AnswerTrigo )->Addexp( at2.Reduce() );
              }
            Solv = true;
            }
          catch( ErrParser )
            {
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
            }
          ex = new TBinar( '=', Function( FName, arg ), y2 );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          eq = PCond.Equation( ex.WriteE(), "x", IsName, Mult );
          try
            {
            ex = SolutionSimpleEquaion( eq, "x" );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            if( s_PutAnswer )
              {
              CastPtr( TL2exp, s_Answer )->Addexp( CastPtr( TL2exp, ex )->Last()->m_Memb );
              MathExpr ZeroEx = Constant( 0 );
              MathExpr at2 = CastPtr( TL2exp, ex )->Last()->m_Memb.Substitute( "k", ZeroEx );
              CastPtr( TL2exp, s_AnswerTrigo )->Addexp( at2.Reduce() );
              }
            Solv = true;
            }
          catch( ErrParser )
            {
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solution" ) );
            }

          Cond.Addexp( new TBinar( msAddition, Variable( "k" ), Variable( "Z" ) ) );
          if( Solv )
            TSolutionChain::sm_SolutionChain.AddExpr( Cond, X_Str( "MRootsFound", "Roots are found" ) );
          }
        else
          return Error2();
      }
    else
      return Error1();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  return Final();
  }

bool CalcSinCosEqu( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;
  double OldPrecision = s_Precision;
  bool OldToFraction = TConstant::sm_ConstToFraction;
  TConstant::sm_ConstToFraction = false;
  s_Precision = 0.0000001;
  int OldDegPoly = s_DegPoly;
  s_DegPoly = 2;
  bool Result = true;

  auto Final = [&] ()
    {
    s_DegPoly = OldDegPoly;
    s_Precision = OldPrecision;
    TConstant::sm_ConstToFraction = OldToFraction;
    return Result;
    };

  auto ErrResult = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MEnterSinCosEquat", "Enter equation of a type: a*sinx+b*cosx==c" ) );
    Result = false;
    return Final();
    };

  try
    {
    QByteArray  VarName = "x";
    bool IsName, Mult;
    Parser P;
    PNode eq;
    while( true )
      {
      eq = P.Equation(P.FullPreProcessor( Source, VarName ), VarName, IsName, Mult );
      if( IsName || VarName == "y" ) break;
      VarName = "y";
      }
    if( TestFrac( eq ) ) return ErrResult();
    MathExpr ex = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr SourceEq = ex;
    if( eq->m_OpSign != '=' )return ErrResult();
    eq = Sub( eq->m_pLeft, eq->m_pRight );
    if( NotOnlyInFunc( eq ) ) return ErrResult();

    PNode eq1 = Clone( eq );
    TNodes p;
    int n;
    eq1 = Replace( eq1, "sin", NewNode( &P, VarName ), p, n );
    if( n == 0 ) return ErrResult();
    MathExpr arg = P.OutPut(p[1] );
    if( Error( arg, p, n ) ) return ErrResult();
    eq1 = Replace( eq1, "cos", NewNode( &P, "zwqpuydgtk" ), p, n );
    if( n == 0 ) return ErrResult();
    if( Error( arg, p, n ) ) return ErrResult();
    ex = P.OutPut( eq1 );
    if( ex.Splitted() ) ex = DelSplitted( ex );
    MathExpArray q;
    ex.ReductionPoly( q, VarName );
    double r;
    if( !( q[2].Constan( r ) && abs( r ) < 0.0000001 ) ) return ErrResult();
    MathExpr a = q[1];
    eq1 = P.GetExpression( a.WriteE() );
    if( IsThereThisName( eq1, "zwqpuydgtk" ) ) return ErrResult();
    PNode eq2 = Clone( eq );
    eq2 = Replace( eq2, "sin", NewNode( &P, "y" ), p, n );
    eq2 = Replace( eq2, "cos", NewNode( &P, "x" ), p, n );
    ex = P.OutPut( eq2 );
    if( ex.Splitted() ) ex = DelSplitted( ex );
    q.clear();
    ex.ReductionPoly( q, VarName );
    if( !( q[2].Constan( r ) && abs( r ) < 0.0000001 ) ) return ErrResult();
    MathExpr b = q[1];
    eq = Replace( eq, "sin", NewNode( &P, VarName ), p, n );
    eq = Replace( eq, "cos", NewNode( &P, VarName ), p, n );
    ex = P.OutPut( eq );
    if( ex.Splitted() ) ex = DelSplitted( ex );
    q.clear();
    ex.ReductionPoly( q, VarName );
    MathExpr c = -( q[0] ).Reduce();
    ex = new TBinar( '=', ( a * Function( "sin", arg ) ).Reduce() + ( b * Function( "cos", arg ) ).Reduce(), c );
    if( !SourceEq.Equal( ex ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr a2b2 = ( ( a ^ 2 ) + ( b ^ 2 ) ).Reduce();
    MathExpr root = a2b2.Root( 2 );
    a /= root;
    b /= root;
    c /= root;
    ex = new TBinar( '=', a * Function( "sin", arg ) + b * Function( "cos", arg ), c );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    if( c.Reduce().Constan( r ) && abs( r ) > 1 )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( new TCommStr( X_Str( "MRightPartGE1", "The right part of the normalized equation is greater than 1" ) ) );
      throw  ErrParser( "No Solution", peNoSolv );
      }
    ex = new TBinar( '=', a, Function( "cos", Variable( msAlpha ) ) );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    ex = new TBinar( '=', b, Function( "sin", Variable( msAlpha ) ) );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );

    MathExpr alfa1 = Function( "arccos", a );
    MathExpr alfa = alfa1.Reduce();
    if( alfa.Eq( alfa1 ) )
      ex = new TBinar( '=', Variable( msAlpha ), alfa );
    else
      ex = new TBinar( '=', new TBinar( '=', Variable( msAlpha ), alfa1 ), alfa );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    ex = new TBinar( '=', ( ( ( ( Function( ( "cos" ), ( Variable( ( msAlpha ) ) ) ) ) * ( Function( ( "sin" ), ( arg ) ) ) ) ) + ( ( ( Function( ( "sin" ), ( Variable( ( msAlpha ) ) ) ) ) * ( Function( ( "cos" ), ( arg ) ) ) ) ) ), c );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    ex = new TBinar( '=', Function( "sin", Variable( msAlpha ) + arg ), c );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr af;
    switch( TExpr::sm_TrigonomSystem )
      {
      case TExpr::tsDeg:
        af = ( -Constant( 1 ) ^ Variable( "k" ) ) * Function( "arcsin", c ) + MathExpr( new TDegExpr( "180;" ) ) * Variable( "k" );
        break;
      default:
        af = ( -Constant( 1 ) ^ Variable( "k" ) ) * Function( "arcsin", c ) + Variable( msPi ) * Variable( "k" );
      }
    ex = new TBinar( '=', Variable( msAlpha ) + arg, af );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr u = af.Reduce().Reduce() - Variable( msAlpha );
    MathExpr exu = new TBinar( '=', arg, Variable( ( "u" ) ) );
    QByteArray Name;
    if( !arg.Variab( Name ) )
      TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', arg, u ) );
    Lexp Cond = new TLexp;
    Parser PCond( &Cond );
    eq = PCond.Equation( exu.WriteE(), "x", IsName, Mult );
    try
      {
      exu = SolutionSimpleEquaion( eq, "x" );
      }
    catch( ErrParser )
      {
      throw  ErrParser( "No Solution", peNoSolv );
      }
    ex = CastPtr( TLexp, exu )->Last()->m_Memb.Substitute( "u", u );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr ex1 = ex.Substitute( QByteArray( 1, msAlpha ), alfa );
    TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
    MathExpr op1, op2;
    ex = ex1.Reduce();
    if( !ex.Equal( ex1 ) )
      {
      ex1 = ex;
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }
    else
      ex1.Binar( '=', op1, op2 );
    ex = new TBinar( '=', op1, Expand( op2 ).Reduce() );
    if( !ex.Equal( ex1 ) )
      {
      ex1 = ex;
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }
    if( s_PutAnswer )
      {
      s_Answer = ex1;
      MathExpr ZeroEx = Constant( 0 );
      s_AnswerTrigo = s_Answer.Substitute( "k", ZeroEx );
      MathExpr at2 = s_AnswerTrigo;
      s_AnswerTrigo = s_AnswerTrigo.Reduce();
      }
    Cond.Addexp( new TBinar( msAddition, Variable( ( "k" ) ), Variable( ( "Z" ) ) ) );
    TSolutionChain::sm_SolutionChain.AddExpr( Cond, X_Str( "MRootsFound", "Roots are found" ) );
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  return Final();
  }

void AssignVar( const MathExpr& exi )
  {
  if( exi.IsEmpty() ) return;
  MathExpr op1, op2, op3;
  QByteArray Name;
  if( exi.Binar( '=', op1, op2 ) && op1.Variab( Name ) )
    {
    op3 = op2.Reduce();
    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', op1, op3 ), X_Str( "MCalcAssValue", "Calculator assigned the value" ) );
    ExpStore::sm_pExpStore->Store_var( Name, op2 );
    return;
    }
  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MWhatThis", "(?)" ) );
  }

bool CalcSubstitution( const QByteArray& Equation, MathExpr BaseSubst )
  {
  auto CheckSubstRoots = [&] ( MathExpr Subs, const QByteArray& xs, const MathExpr& X, const QByteArray& ys, const MathExpr& Y )
    {
    Lexp Cond = new TLexp;

    std::function<void( const MathExpr& )> SearchFractions = [&] ( const MathExpr& exp )
      {
      MathExpr exL, exR;
      if( exp.Divis( exL, exR ) && exp.AsFraction() )
        {
        PExMemb Memb;
        if( !exR.HasUnknown().isEmpty() && !Cond.FindEq( exR, Memb ) )
          Cond.Addexp( exR );
        return;
        }
      char cSign;
      if( !exp.Oper_( cSign, exL, exR ) ) return;
      SearchFractions( exL );
      SearchFractions( exR );
      };

    bool Result = true;
    try
      {
      Subs.Syst_( Subs );
      PExMemb R = CastPtr( TLexp, Subs )->First();
      for( ; !R.isNull(); R = R->m_pNext )
        SearchFractions( R->m_Memb );
      for( R = Cond.First(); !R.isNull(); R = R->m_pNext )
        {
        MathExpr S1 = R->m_Memb.Substitute( xs, X );
        MathExpr S2 = S1.Substitute( ys, Y );
        S1 = S2.Reduce();
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', R->m_Memb, S1 ) );
        if( S1 == 0 )
          Result = false;
        else
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, S1, Constant( 0 ) ) );
        }
      if( !Result ) return false;
      MathExpr S1 = Subs.Substitute( xs, X );
      MathExpr S2 = S1.Substitute( ys, Y );
      S1 = S2.Reduce();
      if( IsConstType( TLexp, S1 ) )
        for( R = CastPtr( TLexp, S1 )->First(); !R.isNull() && Result; R = R->m_pNext )
          R->m_Memb.Boolean_( Result );
      else
        S1.Boolean_( Result );
      }
    catch( ErrParser )
      {
      Result = false;
      }
    return Result;
    };

  MathExpr exi = Parser().StrToExpr( Equation );
  if( BaseSubst.IsEmpty() ) BaseSubst = exi;
  bool Result = false;
  bool NoRootReduceOld = s_NoRootReduce;
  s_NoRootReduce = true;
  MathExpArray lstRoots, List;

  auto Final = [&] ()
    {
    s_NoRootReduce = NoRootReduceOld;
    if( lstRoots.isEmpty() ) return Result;
    s_Answer = new TLexp;
    for( int I = 0; I < lstRoots.count(); I += 2 )
      {
      MathExpr exD1, exD2, exRight;
      lstRoots[I].Binar( '=', exD1, exRight );
      lstRoots[I + 1].Binar( '=', exD2, exRight );
      QByteArray x, y;
      exD1.Variab( x );
      exD2.Variab( y );
      if( x < y )
        {
        CastPtr( TLexp, s_Answer )->Addexp( lstRoots[I] );
        CastPtr( TLexp, s_Answer )->Addexp( lstRoots[I + 1] );
        }
      else
        {
        CastPtr( TLexp, s_Answer )->Addexp( lstRoots[I + 1] );
        CastPtr( TLexp, s_Answer )->Addexp( lstRoots[I] );
        }
      }
    return Result;
    };

  if( s_GlobalInvalid || s_CheckError ) return Final();
  TSolutionChain::sm_SolutionChain.AddExpr( exi.Clone(), X_Str( "MInputSystem", "input system!" ) );
  exi.Syst_( exi );
  MathExpr exn, limit;
  int Check = exi.CheckAndCheckDivision( exn, limit, Lexp() );
  if( Check == 1 )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( exn, X_Str( "MEqSystem", "equivalent system" ) );
    TSolutionChain::sm_SolutionChain.AddExpr( limit, X_Str( "MLimitSystem", "limit of start systems!" ) );
    exi = exn;
    }
  if( Check != 2 )
    {
    CheckInputOfEquationsSystem( exi, List, false );
    TLinear Linear( List );
 //   if( Linear.Status() == liOK ) Linear.PutToWindow();
    TSubstitution Substitution( exi );
    switch( Substitution.Status() )
      {
      case liOK:
        {
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Substitution.GetVarX(), Substitution.GetVarXGi() ),
          X_Str( "MSetSubst", "set of Substitution!" ) );
        PExMemb memMemb = CastPtr( TLexp, exi )->First();
        for( int I = 0; I < Substitution.MultisCount(); I++, memMemb = memMemb->m_pNext )
          {
          if( I != Substitution.EqNo() )
            {
            exn = memMemb->m_Memb;
            exn.Replace( Substitution.GetVarX(), Substitution.GetVarXGi() );
            TSolutionChain::sm_SolutionChain.AddExpr( exn );
            MathExpr exf = new TBinar( '=', Substitution.List( I ), Constant( 0 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( exf, X_Str( "MSubst", "Substitution!" ) );
            exf.SetReduced( false );
            exn = exf.Reduce();
            bool bVal;
            if( exn.Boolean_( bVal ) && !bVal )
              {
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinDec", "No solutions!" ) );
              Result = true;
              return Final();
              }
            }
          }
        Substitution.Run();
        switch( Substitution.Status() )
          {
          case liOK:
            {
            QByteArray y = Substitution.GetVarY().WriteE();
            for( int I = 0; I < Substitution.RootsCount(); I++ )
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( y + "_" + NumberToStr( I + 1 ) ),
              Substitution[I] ) );
            for( int I = 0; I < Substitution.RootsCount(); I++ )
              {
              MathExpr RootX = Substitution.RootX( I );
              QByteArray x;
              if( !Substitution.GetVarX().Variab( x ) )
                {
                x = Substitution.GetVarX().HasUnknown();
                exn = new TBinar( '=', Substitution.GetVarX(), RootX );
                if( GetAnswer( exn ) )
                  RootX = s_Answer;
                else
                  continue;
                }
              MathExpr RootY = Substitution[I];
              bool RightRoot = CheckSubstRoots( BaseSubst, x, RootX, y, RootY );
              if( RootY.WriteE() == "x" )
                RootY = new TStr( "any number" );
              if( Substitution.RootsCount() == 1 )
                {
                RootX = new TBinar( '=', Variable( x ), RootX );
                RootY = new TBinar( '=', Variable( y ), RootY );
                }
              else
                {
                RootX = new TBinar( '=', Variable( ( x + "_" + NumberToStr( I + 1 ) ) ), RootX );
                RootY = new TBinar( '=', Variable( ( y + "_" + NumberToStr( I + 1 ) ) ), RootY );
                }
              Lexp exo = new TLexp;
              if( s_PutAnswer )
                {
                lstRoots.append( new TBinar( '=', Substitution.GetVarX(), Substitution.RootX( I ) ) );
                lstRoots.append( new TBinar( '=', Substitution.GetVarY(), Substitution[I] ) );
                }
              if( x < y )
                {
                exo.Addexp( RootX );
                exo.Addexp( RootY );
                }
              else
                {
                exo.Addexp( RootY );
                exo.Addexp( RootX );
                }
              TSolutionChain::sm_SolutionChain.AddExpr( exo, "", true );
              if( RightRoot )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRoot", "root!" ) );
              else
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootStranger", "false root!" ) );
              }
            Result = true;
            }
            break;
          case liNoFinish:
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotFinish", "! finish!" ) );
            break;
          case liNot1Decision:
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinDec", "it is ! one decision!" ) );
            break;
          case liNotFoundRoots:
            TSolutionChain::sm_SolutionChain.AddComment( s_XPStatus.GetCurrentMessage() );
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotFoundRoots", "! found roots!" ) );
          }
        }
        break;
      case liSquareOnly: //a1x^2+b1y^2=c1; a2x^2+b2y^2=c2
        {
        char *NewVars[] = { "u", "v" };
        for( int I = 0; I <= 1; I++ )
          {
          MathExpr  varNewVar = Variable( NewVars[I] );
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( Substitution.VarName( I ) ) ^ 2, varNewVar ) );
          exn = exi.Substitute( Substitution.VarName( I ), varNewVar.Root( 2 ) );
          exi = exn;
          }
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MSubst", "Substitution!" ) );
        exi.SetReduced( false );
        exn = exi.Reduce();
        MathExpArray List;
        CheckInputOfEquationsSystem( exn, List, false );
        TLinear Linear( List );
        Linear.Run();
        Result = true;
        if( Linear.Status() == liOK )
          for( int I = 0; I <= 1; I++ )
            {
            MathExpr varNewVar, exRoot;
            exi = Linear.GetSolution( I );
            exi.Binar( '=', varNewVar, exRoot );
            int iName = I;
            if( NewVars[I] != varNewVar.WriteE() ) iName = 1 - iName;
//            if( exRoot.Negative() )
//              TSolutionChain::sm_SolutionChain.AddComment( Substitution.VarName( iName ) + ": " +
//              X_Str( "MNoRealSolutions", "No real solutions!" ) );
//            else
//              {
              exn = exRoot.Root( 2 ).Reduce();
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( Substitution.VarName( iName ) + "_1" ), exn ), "", !s_PutAnswer );
              TSolutionChain::sm_SolutionChain.AddExpr(
                MathExpr( new TBinar( '=', Variable( Substitution.VarName( iName ) + "_2" ), -exn ) ).Reduce(), "", !s_PutAnswer );
              if( s_PutAnswer )
                {
                lstRoots.append( new TBinar( '=', Variable( Substitution.VarName( iName ) ), exn ) );
                lstRoots.append( MathExpr( new TBinar( '=', Variable( Substitution.VarName( iName ) ), -exn ) ).Reduce() );
                }
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRoot", "root!" ) );
//              }
            }
        }
        break;
      case liSquareAndProduct1: //a1x^2+b1y^2+c1xy=d1;c2xy=d2
        {
        PExMemb memMemb = CastPtr( TLexp, exi )->First();
        MathExpr exBig, exMult, exRoot, exf;
        if( Substitution.EqNo() == 0 )
          exn = memMemb->m_Memb;
        else
          exBig = memMemb->m_Memb;
        memMemb = memMemb->m_pNext;
        if( Substitution.EqNo() == 1 )
          exn = memMemb->m_Memb;
        else
          exBig = memMemb->m_Memb;
        exn.Binar( '=', exMult, exRoot );
        MathExpr varNewVar = Variable( "x" );
        exMult = ( exRoot / (exMult / varNewVar) ).Reduce().CancellationOfMultiNominals( exf );
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', varNewVar, exMult ), X_Str( "MSubst", "Substitution!" ) );
        exn = exBig.Substitute( "x", exMult );
        TSolutionChain::sm_SolutionChain.AddExpr( exn );
        exBig = ( exn * ( Variable( "y" ) ^ 2 ) ).Reduce();
        do
          {
          exn = exBig;
          exBig = Expand( exBig ).Reduce();
          } while( !exn.Eq( exBig ) );
          exn = CalcDetBiQuEqu( exBig.WriteE(), "y" );
          if( !exn.IsEmpty() )
            {
            memMemb = CastPtr( TLexp, exn )->First();
            for( int I = 1; I <= 4 && !memMemb.isNull(); I++, memMemb = memMemb->m_pNext )
              {
              exf = exMult.Substitute( "y", memMemb->m_Memb );
              varNewVar = Variable( ( "x_" + NumberToStr( I ) ) );
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', varNewVar, exf ) );
              exf.SetReduced( false );
              exRoot = exf.Reduce();
              if( !exRoot.Eq( exf ) )
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', varNewVar, exRoot ), X_Str( "MRoot", "root!" ), true );
              }
            Result = true;
            }
        }
        break;
      case liProductAndSimple: //a1x+b1y+c1xy=d1;a2x+b2y+c2xy=d2
        {
        Lexp liNewSystem = new TL2exp;
        PExMemb memMemb = CastPtr( TLexp, exi )->First();
        liNewSystem.Addexp( ( Substitution( 1 ).HasProduct( Substitution.VarName( 0 ), Substitution.VarName( 1 ), 1, 1 ).CoeffAndSign() *
          memMemb->m_Memb).Reduce() );
        liNewSystem.Addexp( ( Substitution( 0 ).HasProduct( Substitution.VarName( 0 ), Substitution.VarName( 1 ), 1, 1 ).CoeffAndSign() *
          memMemb->m_pNext->m_Memb).Reduce() );
        TSolutionChain::sm_SolutionChain.AddExpr( liNewSystem );
        memMemb->m_Memb = ( liNewSystem.First()->m_Memb - liNewSystem.First()->m_pNext->m_Memb ).Reduce();
        Result = CalcSubstitution( exi.WriteE(), BaseSubst );
        }
        break;
      case liSquareAndProduct: //a1x^2+b1y^2+c1xy=d1;a2x^2+b2y^2+c2xy=d2
        {
        TTerm trmTerm1 = Substitution( 0 ).HasVariable( "", 1 );
        TTerm trmTerm2 = Substitution( 1 ).HasVariable( "", 1 );
        PExMemb memMemb = CastPtr( TLexp, exi )->First();
        MathExpr exHomo, exForSubstitution;
        if( trmTerm1.m_Sign == TTerm::sm_TermNotExist )
          {
          exHomo = memMemb->m_Memb;
          exForSubstitution = memMemb->m_pNext->m_Memb;
          }
        else
          if( trmTerm2.m_Sign == TTerm::sm_TermNotExist )
            {
            exHomo = memMemb->m_pNext->m_Memb;
            exForSubstitution = memMemb->m_Memb;
            }
          else
            {
            exForSubstitution = memMemb->m_Memb;
            Lexp liNewSystem = new TL2exp;
            MathExpr exD1 = -trmTerm1.CoeffAndSign();
            MathExpr exD2 = -trmTerm2.CoeffAndSign();
            liNewSystem.Addexp( ( exD2 * memMemb->m_Memb ).Reduce() );
            liNewSystem.Addexp( ( exD1 * memMemb->m_pNext->m_Memb ).Reduce() );
            TSolutionChain::sm_SolutionChain.AddExpr( liNewSystem );
            exHomo = ( liNewSystem.First()->m_Memb - liNewSystem.First()->m_pNext->m_Memb ).Reduce();
            TSolutionChain::sm_SolutionChain.AddExpr( exHomo );
            exHomo.Binar( '=', exD1, exD2 );
            int I;
            if( exD1.Cons_int( I ) && exD2.Cons_int( I ) )
              exHomo.Clear();
            else
              memMemb->m_pNext->m_Memb = exHomo;
            }
        if( !exHomo.IsEmpty() )
          {
          MathExpr exLeft, exRoot;
          exHomo.Binar( '=', exLeft, exRoot );
          TMultiNominal mulMulti( exLeft );
          trmTerm1 = mulMulti.HasVariable( Substitution.VarName( 0 ), 2 );
          trmTerm2 = mulMulti.HasVariable( Substitution.VarName( 1 ), 2 );
          TTerm trmProduct = mulMulti.HasProduct( Substitution.VarName( 0 ), Substitution.VarName( 1 ), 1, 1 );
          QByteArray x, y;
          if( trmProduct.isEmpty() )
            {
            if( trmTerm1.isEmpty() )
              x = trmTerm2[0].m_Base.WriteE();
            else
              if( trmTerm2.isEmpty() ) x = trmTerm1[0].m_Base.WriteE();
            if( !x.isEmpty() )
              {
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( x ), Constant( 0 ) ) );
              exn = exForSubstitution.Substitute( x, Constant( 0 ) );
              exn.SetReduced( false );
              exForSubstitution = exn.Reduce();
              if( x == Substitution.VarName( 0 ) )
                y = Substitution.VarName( 1 );
              else
                y = Substitution.VarName( 0 );
              bool InterRes = TSolutionChain::sm_InterimResult;
              TSolutionChain::sm_InterimResult = true;
              CalcDetQuEqu( exForSubstitution.WriteE(), y );
              TSolutionChain::sm_InterimResult = InterRes;
              }
            else
              {
              MathExpr exD1 = trmTerm1.FullTerm();
              MathExpr exD2 = trmTerm2.FullTerm();
              if( exD1.Negative() && exD2.Negative() || !exD1.Negative() && !exD2.Negative() )
                TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions!" ) );
              else
                {
                exn = new TBinar( '=', trmTerm1.AbsFullTerm().Root( 2 ), trmTerm2.AbsFullTerm().Root( 2 ) );
                Lexp liNewSystem = new TL2exp;
                liNewSystem.Addexp( exForSubstitution );
                exn.SetReduced( false );
                exHomo = exn.Reduce();
                liNewSystem.Addexp( exHomo.Reduce() );
                CalcSubstitution( liNewSystem.WriteE(), BaseSubst );
                }
              }
            }
          else
            if( trmTerm1.isEmpty() && trmTerm2.isEmpty() )
              for( int I = 0; I <= 1; I++ )
                {
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( Substitution.VarName( I ) ), Constant( 0 ) ) );
                exn = exForSubstitution.Substitute( Substitution.VarName( I ), Constant( 0 ) );
                exn.SetReduced( false );
                bool InterRes = TSolutionChain::sm_InterimResult;
                TSolutionChain::sm_InterimResult = true;
                CalcDetQuEqu( exn.Reduce().WriteE(), Substitution.VarName( 1 - I ) );
                TSolutionChain::sm_InterimResult = InterRes;
                }
            else
              {
              TTerm trmTerm;
              if( trmTerm1.isEmpty() )
                trmTerm = trmTerm2;
              else
                if( trmTerm2.isEmpty() )
                  trmTerm = trmTerm1;
              if( !trmTerm.isEmpty() )
                {
                x = trmTerm[0].m_Base.WriteE();
                if( x == Substitution.VarName( 0 ) )
                  y = Substitution.VarName( 1 );
                else
                  y = Substitution.VarName( 0 );
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( x ), Constant( 0 ) ) );
                exn = exForSubstitution.Substitute( x, Constant( 0 ) );
                exn.SetReduced( false );
                bool InterRes = TSolutionChain::sm_InterimResult;
                TSolutionChain::sm_InterimResult = true;
                CalcDetQuEqu( exn.Reduce().WriteE(), y );
                exn = trmProduct.CoeffAndSign() * Variable( y ) / trmTerm.CoeffAndSign();
                MathExpr exf = -( exn ).Reduce();
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( x ), exf ),
                  X_Str( "MSetSubst", "set of Substitution!" ) );
                exn = exForSubstitution.Substitute( x, exf );
                TSolutionChain::sm_SolutionChain.AddExpr( exn );
                exLeft = exn.Reduce();
                Lexp liRoots = CalcDetQuEqu( exLeft.WriteE(), y );
                TSolutionChain::sm_InterimResult = InterRes;
                if( !liRoots.IsEmpty() )
                  {
                  memMemb = liRoots.First();
                  for( int I = 1; I <= 2; I++, memMemb = memMemb->m_pNext )
                    {
                    if( memMemb.isNull() ) return Final();
                    MathExpr RootX = exf.Substitute( y, memMemb->m_Memb );
                    exn = RootX.Reduce().CancellationOfMultiNominals( exLeft );
                    TSolutionChain::sm_SolutionChain.AddExpr(
                      new TBinar( '=', Variable( x + "_" + NumberToStr( I ) ), exn.Reduce() ) );
                    }
                  }
                }
              else
                {
                exHomo = new TBinar( '=', trmTerm1.CoeffAndSign() + trmTerm2.FullTerm() / trmTerm1.Term_SC() +
                  trmProduct.CoeffAndSign() * trmTerm2.TermBase() / trmTerm1.TermBase(), Constant( 0 ) );
                TSolutionChain::sm_SolutionChain.AddExpr( exHomo.Reduce() );
                MathExpr varNewVar = Variable( "u" );
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', varNewVar, trmTerm2.TermBase() / trmTerm1.TermBase() ),
                  X_Str( "MSetSubst", "set of Substitution!" ) );
                exHomo = new TBinar( '=', trmTerm2.CoeffAndSign() * ( varNewVar ^ 2 ) +
                  trmProduct.CoeffAndSign() * varNewVar + trmTerm1.CoeffAndSign(), Constant( 0 ) );
                exn = exHomo.Reduce();
                bool OldInterim = TSolutionChain::sm_InterimResult;
                TSolutionChain::sm_InterimResult = true;
                Lexp liRootsu = CalcDetQuEqu( exn.WriteE(), "u" );
                TSolutionChain::sm_InterimResult = OldInterim;
                if( !liRootsu.IsEmpty() )
                  {
                  for( PExMemb memMembu = liRootsu.First(); !memMembu.isNull(); memMembu = memMembu->m_pNext )
                    {
                    exn = memMembu->m_Memb * trmTerm1.TermBase();
                    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', trmTerm2.TermBase(), exn ),
                      X_Str( "MSetSubst", "set of Substitution!" ) );
                    MathExpr exf = exForSubstitution.Substitute( trmTerm2.VarName(), exn );
                    TSolutionChain::sm_InterimResult = true;
                    Lexp liRoots = CalcDetQuEqu( exf.WriteE(), trmTerm1.VarName() );
                    TSolutionChain::sm_InterimResult = OldInterim;
                    if( !liRoots.IsEmpty() )
                      {
                      memMemb = liRoots.First();
                      for( int I = 1; I <= 2; I++, memMemb = memMemb->m_pNext )
                        {
                        if( memMemb.isNull() ) return Final();
                        MathExpr RootY = memMembu->m_Memb * memMemb->m_Memb;
                        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=',
                          Variable( trmTerm2.VarName() + "_" + NumberToStr( I ) ), RootY.Reduce() ), "", true );
                        }
                      }
                    }
                  }
                }
              }
          }
        Result = true;
        }
        break;
      case liSquareAndSimple:
      case liSquareAndSimpleAndProduct:
        {
        //a1x^2+b1y^2+c1x+d1y=e1;a2x^2+b2y^2+c2x+d2y=e2
        //a1x^2+b1y^2+c1x+d1y+f1xy=e1;a2x^2+b2y^2+c2x+d2y+f2xy=e3
        Result = true;
        QByteArray x = Substitution.VarName( 0 );
        TTerm trmTerm1 = Substitution( 0 ).HasVariable( x, 2 );
        TTerm trmTerm2 = Substitution( 1 ).HasVariable( x, 2 );
        PExMemb  memMemb = CastPtr( TLexp, exi )->First();
        Lexp liNewSystem = new TL2exp;
        liNewSystem.Addexp( ExpandExpr( trmTerm2.CoeffAndSign() * memMemb->m_Memb ) );
        liNewSystem.Addexp( ExpandExpr( trmTerm1.CoeffAndSign() * memMemb->m_pNext->m_Memb ) );
        TSolutionChain::sm_SolutionChain.AddExpr( liNewSystem );
        exn = ( liNewSystem.First()->m_Memb - liNewSystem.First()->m_pNext->m_Memb ).Reduce();
        MathExpr exD1, exD2;
        exn.Binar( '=', exD1, exD2 );
        int I;
        if( exD1.Cons_int( I ) && exD2.Cons_int( I ) )
          {
          TSolutionChain::sm_SolutionChain.AddExpr( exn, X_Str( "MNotCIES", "! correct input of equations system!" ) );
          Result = false;
          }
        else
          {
          TMultiNominal mulMulti1( ( exD1 - exD2 ).Reduce() );
          QByteArray y = Substitution.VarName( 1 );
          TTerm trmTerm = mulMulti1.HasVariable( y, 2 );
          if( trmTerm.isEmpty() )
            {
            TSolutionChain::sm_SolutionChain.AddExpr( exn );
            CastPtr( TLexp, exi )->First()->m_pNext->m_Memb = exn;
            CalcSubstitution( exi.WriteE(), BaseSubst );
            }
          else
            {
            TSolutionChain::sm_SolutionChain.AddExpr( exn );
            trmTerm1 = Substitution( 0 ).HasVariable( y, 2 );
            trmTerm2 = Substitution( 1 ).HasVariable( y, 2 );
            liNewSystem = new TL2exp;
            liNewSystem.Addexp( ( trmTerm2.CoeffAndSign() * memMemb->m_Memb ).Reduce() );
            liNewSystem.Addexp( ( trmTerm1.CoeffAndSign() * memMemb->m_pNext->m_Memb ).Reduce() );
            TSolutionChain::sm_SolutionChain.AddExpr( liNewSystem );
            MathExpr exForSubstitution = ( liNewSystem.First()->m_Memb - liNewSystem.First()->m_pNext->m_Memb ).Reduce();
            TSolutionChain::sm_SolutionChain.AddExpr( exForSubstitution );
            trmTerm = mulMulti1.HasVariable( x, 1 );
            if( !trmTerm.isEmpty() )
              {
              exForSubstitution.Binar( '=', exD1, exD2 );
              TMultiNominal mulMulti2( ( exD1 - exD2 ).Reduce() );
              trmTerm = mulMulti2.HasVariable( y, 1 );
              if( trmTerm.isEmpty() )
                {
                QByteArray sTmp = x;
                x = y;
                y = sTmp;
                exD1 = exn;
                exn = exForSubstitution;
                exForSubstitution = exD1;
                }
              }
            if( trmTerm.isEmpty() )
              {
              bool InterRes = TSolutionChain::sm_InterimResult;
              TSolutionChain::sm_InterimResult = true;
              Lexp liRoots = CalcDetQuEqu( exn.WriteE(), y );
              if( !liRoots.IsEmpty() )
                {
                for( memMemb = liRoots.First(); !memMemb.isNull(); memMemb = memMemb->m_pNext )
                  {
                  TSolutionChain::sm_SolutionChain.AddExpr( memMemb->m_Memb, X_Str( "MSetSubst", "set of Substitution!" ) );
                  MathExpr exf = exForSubstitution.Substitute( y, memMemb->m_Memb );
                  CalcDetQuEqu( exf.Reduce().Reduce().WriteE(), x );
                  }
                }
              TSolutionChain::sm_InterimResult = InterRes;
              }
            else
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotPSMS", "not possibility to solve by method Substitution!" ) );
            }
          }
        }
        break;
      case liNotCorrect:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotCIES", "not correct input of equations system!" ) );
        break;
      case liNotSUBST:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotPSMS", "not possibility to solve by method Substitution!" ) );
      }
    }
  else
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotCIES", "not correct input of equations system!" ) );
    }
  return Final();
  }

bool CalcLinear( const QByteArray& Equation )
  {
  MathExpr exi = Parser().StrToExpr( Equation );
  if( s_GlobalInvalid || s_CheckError && !( IsConstType( TL2exp, exi ) ) && !( IsConstType( TSyst, exi ) ) ) return false;
  bool NoRootReduceOld = s_NoRootReduce;
  bool s_NoRootReduce = true;
  bool Result = false;
  if( IsConstType( TL2exp, exi ) ) exi = new TSyst( exi );
  TSolutionChain::sm_SolutionChain.AddExpr( exi, X_Str( "MInputSystem", "input system!" ) );
  MathExpr exold = exi;
  exi.Syst_( exi );
  MathExpr exn, limit;
  if( exi.CheckAndCheckDivision( exn, limit, Lexp() ) == 1 )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( exn, X_Str( "MEqSystem", "equivalent system" ) );
    TSolutionChain::sm_SolutionChain.AddExpr( limit, X_Str( "MLimitSystem", "limit of start systems!" ) );
    exi = exn;
    }
  MathExpArray List;
  bool SignOk = CheckInputOfEquationsSystem( exi, List, false );
  TLinear Linear( List );
  if( !Linear.m_Matrix2.isEmpty() )
    Linear.m_Matrix2.m_OldStep = exold;
  if( SignOk && Linear.Run() )
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsE", "roots of equations!" ) );
    Result = true;
    }
  else
    {
    Result = false;
    switch( Linear.Status() )
      {
      case liNotLinear:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinNo", "it is ! Linear equations!" ) );
        break;
      case liNot1Decision:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinDec", "it is ! one decision!" ) );
        Result = true;
        break;
      case liInfinityDecision:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfDec", "Infinite set of solutions!" ) );
        Result = true;
        break;
      case liSystemOverdetermined:
        Linear.m_Matrix2.PutToWindow();
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinOwerDet", "System has too many equations!" ) );
        Result = true;
        break;
      default:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinInv", "invalid input data!" ) );
      }
    }
  s_NoRootReduce = NoRootReduceOld;
  return Result;
  }

MathExpArray SolveLinear( const MathExpr& exi )
  {
  if( exi.IsEmpty() ) return MathExpArray();
  MathExpArray List;
  bool SignOk = CheckInputOfEquationsSystem( exi, List, false );
  TLinear Linear( List );
  if( SignOk && Linear.Run() )
    {
    MathExpArray Result;
    for( int I = 0; I < Linear.m_Matrix2.count(); I++ )
      Result.append( Linear.GetSolution( I ) );
    qSort( Result.begin(), Result.end() );
    return Result;
    }
  return MathExpArray();
  }

bool CalcExchange( const QByteArray& Equation )
  {
  MathExpr exi = Parser().StrToExpr( Equation );
  if( s_GlobalInvalid || s_CheckError ) return false;
  MathExpr* ptrOperand;
  std::function<MathExpr( MathExpr&, MathExpr& )> VarChange = [&] ( MathExpr& exSource, MathExpr& exTarget )
    {
    MathExpr exOp1, exOp2, exOp, exPower;
    char cOper;
    MathExpr Result;
    if( exTarget.Oper_( cOper, exOp1, exOp2 ) )
      {
      if( cOper == '+' || cOper == '-' || cOper == '=' )
        {
        Result = VarChange( exSource, exOp1 );
        if( !Result.IsEmpty() )
          {
          if( ptrOperand == nullptr ) ptrOperand = &CastPtr( TOper, exTarget )->Left();
          }
        else
          {
          Result = VarChange( exSource, exOp2 );
          if( !Result.IsEmpty() && ptrOperand == nullptr ) ptrOperand = &CastPtr( TOper, exTarget )->Right();
          }
        if( ptrOperand != nullptr )
          {
          ptrOperand->SetReduced( false );
          exTarget.SetReduced( false );
          }
        return Result;
        }
      if( cOper == '/' && exOp2.Equal( exSource ) || exOp2.Power( exOp, exPower ) && exOp.Equal( exSource ) )
        Result = exOp1;
      }
    return Result;
    };

  bool Result = false;
  bool NoRootReduceOld = s_NoRootReduce;
  s_NoRootReduce = true;
  TSolutionChain::sm_SolutionChain.AddExpr( exi.Clone(), X_Str( "MInputSystem", "input system!" ) );
  exi.Syst_( exi );
  MathExpr ex = exi.Clone();
  int DetailDegreeOld = s_DetailDegree;
  s_DetailDegree = 0;
  MathExpr exn, limit;
  if( ex.CheckAndCheckDivision( exn, limit, Lexp() ) == 1 )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( exn, X_Str( "MEqSystem", "equivalent system" ) );
    TSolutionChain::sm_SolutionChain.AddExpr( limit, X_Str( "MLimitSystem", "limit of start systems!" ) );
    exi = exn;
    }
  ex = exi.Clone();
  TExchange  Exchange( ex );
  const char* SubstVars[] = { "u", "v" };
  s_DetailDegree = DetailDegreeOld;
  switch( Exchange.Status() )
    {
    case liOK:
      {
      Lexp exo = new TLexp;
      exo.Addexp( new TBinar( '=', Exchange.GetNewVarU(), Exchange.GetExprXsY() ) );
      exo.Addexp( new TBinar( '=', Exchange.GetNewVarV(), Exchange.GetExprXpY() ) );
      TSolutionChain::sm_SolutionChain.AddExpr( exo, X_Str( "MExchange", "Exchange" ) );
      Exchange.Run();
      if( Exchange.Status() == liOK )
        {
        exo = new TL2exp;
        exo.Addexp( Exchange.GetExchEqu1() );
        exo.Addexp( Exchange.GetExchEqu2() );
        TSolutionChain::sm_SolutionChain.AddExpr( exo, X_Str( "MNewSystem", "new system!" ) );
        exo = new TL2exp;
        exo.Addexp( Exchange.GetExchEqu1() );
        exo.Addexp( Exchange.GetExchEqu2() );
        TSubstitution Substitution( exo );
        if( Substitution.Status() == liOK && Substitution.Run() )
          {
          for( int I = 0; I < Substitution.RootsCount(); I++ )
            {
            MathExpr RootX = new TBinar( '=', Substitution.GetVarX(), Substitution.RootX( I ) );
            MathExpr RootY = new TBinar( '=', Substitution.GetVarY(), Substitution[I] );
            Exchange.InsertRoot( Substitution.GetVarX().WriteE(), Substitution.RootX( I ), Substitution[I] );
            exo = new TLexp;
            exo.Addexp( RootX );
            exo.Addexp( RootY );
            TSolutionChain::sm_SolutionChain.AddExpr( exo, X_Str( "MRoot", "root!" ) );
            Result = true;
            }
          Exchange.CalculateFinish();

          for( int I = 0; I < Exchange.m_RootsX.count(); I++ )
            TSolutionChain::sm_SolutionChain.AddExpr( Exchange.GetPrintRoot( I ), X_Str( "MFinishRoot", "finish root!" ), true );
          if( Exchange.m_RootsX.count() == 0 ) TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotFoundRoots", "not found roots!" ) );
          }
        else
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotPSMS", "not possibility to solve by method Substitution!" ) );
        }
      }
      break;
    case liDenomsPQPQ: //a1/P+b1/Q=c1; a2/P+b2/Q=c2;
    case liDenomsP2Q2P2Q2: //a1/P^2+b1/Q^2=c1; a2/P^2+b2/Q^2=c2;
      {
      ex = exi;
      PExMemb memMemb = Exchange.m_DenomList.First();
      Result = true;
      for( int I = 0; I < 2; I++, memMemb = memMemb->m_pNext )
        {
        MathExpr exNewVar;
        for( PExMemb memEquation = CastPtr( TLexp, ex )->First(); !memEquation.isNull() && Result; memEquation = memEquation->m_pNext )
          {
          ptrOperand = nullptr;
          MathExpr exCoeff = VarChange( memMemb->m_Memb, memEquation->m_Memb );
          Result = exCoeff != nullptr;
          if( !Result ) break;
          exNewVar = Variable( SubstVars[I] );
          *ptrOperand = exCoeff *exNewVar;
          }
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Constant( 1 ) / memMemb->m_Memb, exNewVar ) );
        }
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MExchange", "Exchange" ) );
      if( Result )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( new TNewLin, X_Str( "MNewSystem", "new system!" ) );
        DetailDegreeOld = s_DetailDegree;
        s_DetailDegree = liPutFirstStage;
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        bool InterResult = TSolutionChain::sm_InterimResult;
        TSolutionChain::sm_InterimResult = true;
        MathExpArray lstRoots = SolveLinear( ex );
        TSolutionChain::sm_InterimResult = InterResult;
        s_DetailDegree = DetailDegreeOld;
        Result = false;
        if( !lstRoots.isEmpty() )
          {
          Result = true;
          Lexp lstNewSystem = new TL2exp;
          memMemb = Exchange.m_DenomList.First();
          for( int I = 0; I < 2; I++, memMemb = memMemb->m_pNext )
            {
            MathExpr exNewVar, exCoeff;
            lstRoots[I].Binar( '=', exNewVar, exCoeff );
            if( Exchange.Status() == liDenomsP2Q2P2Q2 )
              {
              MathExpr exSource;
              memMemb->m_Memb.Power( exSource, exn );
              lstNewSystem.Addexp( new TBinar( '=', exCoeff.Root( 2 ) * exSource, Constant( 1 ) ) );
              }
            else
              lstNewSystem.Addexp( new TBinar( '=', exCoeff * memMemb->m_Memb, Constant( 1 ) ) );
            }
          Result = CalcSubstitution( lstNewSystem.WriteE() );
          }
        }
      if( !Result )
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotPSME", "not possibility to solve by method Exchange!" ) );
      }
      break;
    case liDenomsPQP2Q2: //a1/P+b1/Q=c1; a2/P^2+b2/Q^2=c2;
      {
      ex = exi;
      PExMemb memMemb = Exchange.m_DenomList.First();
      Result = true;
      for( int I = 0; I < 2; I++, memMemb = memMemb->m_pNext )
        {
        MathExpr exNewVar;
        for( PExMemb memEquation = CastPtr( TLexp, ex )->First(); !memEquation.isNull() && Result; memEquation = memEquation->m_pNext )
          {
          MathExpr exSource = memMemb->m_Memb;
          exSource.Power( exSource, exn );
          ptrOperand = nullptr;
          MathExpr exCoeff = VarChange( exSource, memEquation->m_Memb );
          Result = !exCoeff.IsEmpty();
          if( !Result ) break;
          exNewVar = Variable( SubstVars[I] );
          MathExpr exNom, exDenom;
          ptrOperand->Divis( exNom, exDenom );
          if( exDenom.Power( exn, exn ) )
            *ptrOperand = exCoeff * ( exNewVar ^ 2 );
          else
            *ptrOperand = exCoeff * exNewVar;
          }
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Constant( 1 ) / memMemb->m_Memb, exNewVar ) );
        }
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MExchange", "Exchange" ) );
      if( Result )
        {
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNewSystem", "new system!" ) );
        bool bPutAnswerOld = s_PutAnswer;
        s_PutAnswer = true;
        bool Interim = TSolutionChain::sm_InterimResult;
        TSolutionChain::sm_InterimResult = true;
        Result = CalcSubstitution( ex.WriteE() );
        TSolutionChain::sm_InterimResult = Interim;
        s_PutAnswer = bPutAnswerOld;
        if( Result )
          {
          for( PExMemb memEquation = CastPtr( TLexp, s_Answer )->First(); !memEquation.isNull();  )
            {
            Lexp lstNewSystem = new TL2exp;
            Lexp SimpleResult = new TL2exp;
            memMemb = Exchange.m_DenomList.First();
            for( int I = 0; I < 2; I++, memMemb = memMemb->m_pNext, memEquation = memEquation->m_pNext )
              {
              MathExpr exSource = memMemb->m_Memb;
              exSource.Power( exSource, exn );
              MathExpr exNewVar, exCoeff;
              memEquation->m_Memb.Binar( '=', exNewVar, exCoeff );
              if( IsType( TVariable, memMemb->m_Memb ) && exCoeff.IsNumerical() )
                SimpleResult.Addexp( new TBinar( '=', memMemb->m_Memb, ( Constant( 1 ) / exCoeff ).Reduce() ) );
              lstNewSystem.Addexp( new TBinar( '=', exCoeff * memMemb->m_Memb, Constant( 1 ) ) );
              }
            if( SimpleResult.Count() == 2 )
              TSolutionChain::sm_SolutionChain.AddExpr( SimpleResult, X_Str( "MRootsE", "Roots are found" ), true );
            else
              Result = Result && CalcSubstitution( lstNewSystem.WriteE() );
            }
          }
        }
      if( !Result )
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotPSME", "! possibility to solve by method Exchange!" ) );
      }
      break;
    case liNotCorrect:
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotCIES", "! correct input of equations system!" ) );
      break;
    case liNotEXCHANGE: TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotPSME", "! possibility to solve by method Exchange!" ) );
    }
  s_NoRootReduce = NoRootReduceOld;
  return Result;
  }

bool CalcRootsQuEqu( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;
  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  bool OldNoRootReduce = s_NoRootReduce;
  bool Result = true;

  auto Fin = [&] ()
    {
    s_Precision = OldPrecision;
    s_NoRootReduce = OldNoRootReduce;
    return Result;
    };

  auto BadEquation = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MEnterQuadrEquat", "Enter quadratic equation!" ) );
    Result = false;
    return Fin();
    };

  try
    {
    s_NoRootReduce = true;
    QByteArray VarName = "x";
    PNode eq;
    Parser P;
    bool IsName, Mult;
    while( true )
      {
      eq = P.Equation( P.FullPreProcessor( Source, VarName ), VarName, IsName, Mult );
      if( IsName || VarName == "y" ) break;
      VarName = "y";
      }
    if( IsFuncEqu( eq ) || TestFrac( eq ) || IsExpEqu( eq ) ) return BadEquation();
    MathExpr ex = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    FracToMult( eq );
    ex = P.OutPut( eq );
    if( ex.Splitted() ) ex = DelSplitted( ex );
    if( GetFactorCount( ex, VarName ) > 1 )
      {
      bool bFinalComment = true;
      do
        {
        int i = s_RootCount;
        bool OldAccumulate = TSolutionChain::sm_SolutionChain.m_Accumulate;
        TSolutionChain::sm_SolutionChain.m_Accumulate = false;
        ex = GetFactorEquation( VarName );
        TSolutionChain::sm_SolutionChain.m_Accumulate = OldAccumulate;
        if( ex.IsEmpty() )
          {
          if( bFinalComment )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Equation is solved" ) );
          return Fin();
          }
        if( s_RootCount == i )
          {
          CalcRootsQuEqu( ex.WriteE() );
          bFinalComment = false;
          }
        } while( true );
        s_FactorizedSolving = false;
      }
    if( ReduceEquation( ex ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex.Clone() );
    MathExpr op1, op2;
    if( ex.Binar( '=', op1, op2 ) )
      {
      MathExpr ex1 = ( op1 - op2 ).Reduce();
      TExprs a;
      MathExpr ex2 = ex1.ReductionPoly( a, "x" );
      double r;
      for( int i = s_DegPoly; i >= 3; i-- )
        if( !( a[i].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) ) return BadEquation();
      ex1 = new TBinar( '=', ex2, Constant( 0 ) );
      if( !ex.Eq( ex1 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      if( a[2].Constan( r ) && abs( r ) < 0.0000001 )
        {
        if( a[1].Reduce().Constan( r ) && abs( r ) < 0.0000001 )
          if( a[0].Reduce().Constan( r ) && ( abs( r ) < 0.0000001 ) )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfiniteSetSol", "Infinite set of solutions" ) );
          else
            {
            PNode q = P.GetExpression( a[0].WriteE() );
            if( !NIsConst( q ) )
              {
              TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MInfOrNotSolLeft1", " if( an expression at the left side is equal to zero," ) );
              TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MInfOrNotSolLeft2", "the equation has infinite set of solutions," ) );
              TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MInfOrNotSolLeft3", "otherwise equation has no solutions." ) );
              }
            else
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
            }
        else
          {
          ex1 = ( -a[0] / a[1] ).Reduce().Reduce();
          if( s_FactorizedSolving )
            ex = GetPutRoot( ex1, VarName );
          else
            {
            ex = new TBinar( '=', Variable( ( VarName ) ), ex1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            }
          PNode q = P.GetExpression( a[1].WriteE() );
          if( !NIsConst( q ) )
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[1], Constant( 0 ) ), X_Str( "MLinSolved", "Equation is solved" ), true );
          }
        }
      else
        {
        MathExpr op;
        if( a[2].Divis( op1, op2 ) || a[2].Unarminus( op ) && op.Divis( op1, op2 ) )
          {
          ex = ( a[1] * op2 ).Reduce();
          a[1] = ex;
          ex = ( a[0] * op2 ).Reduce();
          a[0] = ex;
          if( a[2].Unarminus( op ) )
            ex = -op1;
          else
            ex = op1;
          a[2] = ex;
          ex = Variable( VarName ) ^ 2;
          if( !( a[2].Constan( r ) && r == 1 ) )
            ex = a[2] * ex;
          if( !( a[1].Constan( r ) && ( abs( r ) < 0.0000001 ) ) )
            if( !( a[1].Constan( r ) && r == 1 ) )
              ex += a[1] * Variable( VarName );
            else
              ex += Variable( VarName );
          if( !( a[0].Constan( r ) && abs( r ) < 0.0000001 ) )
            ex += a[0];
          ex = new TBinar( '=', ex, Constant( 0 ) );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        MathExpr D1 = ( ( a[1] ^ 2 ) - Constant( 4 ) * a[2] * a[0] ).Reduce();
        MathExpr D = D1.Reduce();
        int nom, den;
        if( D.Constan( r ) && abs( r ) < 0.0000001 )
          {
          ex1 = ( -a[1] / ( Constant( 2 ) * a[2] ) ).Reduce().Reduce();
          ex = new TBinar( '=', new TVariable( true, VarName + "_" + NumberToStr( s_RootCount + 1 ) ),
            new TBinar( '=', Variable( VarName + "_" + NumberToStr( s_RootCount + 2 ), true ), ex1 ) );
          s_RootCount += 2;
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          PNode q = P.GetExpression( a[2].WriteE() );
          if( !NIsConst( q ) )
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[2], Constant( 0 ) ), X_Str( "MRootsFound", "Roots are found." ), true );
          }
        else

          if( !( D.Constan( r ) && r < 0 || D.SimpleFrac_( nom, den ) && nom < 0 ) )
            {
            ex = D.Root( 2 );
            ex1 = Constant( 2 ) * a[2];
            op1 = ( -a[1] - ex ) / ex1;
            MathExpr x1 = ExpandExpr( op1 );
            op2 = ( -a[1] + ex ) / ex1;
            MathExpr x2 = ExpandExpr( op2 );
            ex = GetPutRoot( x1, VarName );
            ex = GetPutRoot( x2, VarName );
            PNode q = P.GetExpression( a[2].WriteE() );
            if( !NIsConst( q ) )
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, a[2], Constant( 0 ) ), X_Str( "MRootsFound", "Roots are found" ), true );
            }
          else
            {
            ex1 = Constant( 2 ) * a[2];
            ex = -a[1] / ex1;
            op1 = ex;
            ex = Constant( -r ).Root( 2 ) / ex1;
            op2 = ex.Reduce();
            ex = GetPutRoot( CreateComplex( op1, op2 ), VarName );
            ex = GetPutRoot( CreateComplex( op1, -op2 ), VarName );
            }
        }
      }
    else
      return BadEquation();
    return Fin();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Fin();
  }

bool CalcDiscrim( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  bool NoRootReduceOld = s_NoRootReduce;
  s_NoRootReduce = true;

  auto BadEq = [&] ()
    {
    s_Precision = OldPrecision;
    s_NoRootReduce = NoRootReduceOld;
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MEnterQuadrEquat", "Enter quadratic equation!" ) );
    return false;
    };

  bool Result = true;
  try
    {
    Parser P;
    bool IsName, Mult;
    PNode eq = P.Equation( P.FullPreProcessor( Source, "x" ), "x", IsName, Mult );
    if( IsFuncEqu( eq ) || TestFrac( eq ) ) return BadEq();
    MathExpr  ex = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    FracToMult( eq );
    ex = P.OutPut( eq );
    if( ex.Splitted() )
      ex = DelSplitted( ex );
    if( ReduceEquation( ex ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr op1, op2;
    if( ex.Binar( '=', op1, op2 ) )
      {
      MathExpr ex1 = ( op1 - op2 ).Reduce();
      double r;
      if( !( op2.Constan( r ) && abs( r ) < 0.0000001 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, Constant( 0 ) ) );
      MathExpArray a;
      MathExpr ex2 = ex1.ReductionPoly( a, "x" );
      for( int i = s_DegPoly; i >= 3; i-- )
        if( !( a[i].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) ) return BadEq();
      if( a[2].Reduce().Constan( r ) && abs( r ) < 0.0000001 ) return BadEq();
      ex1 = new TBinar( '=', ex2, Constant( 0 ) );
      if( !ex.Equal( ex1 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      MathExpr op;
      if( a[2].Divis( op1, op2 ) || a[2].Unarminus( op ) && op.Divis( op1, op2 ) )
        {
        ex = ( a[1] * op2 ).Reduce();
        a[1] = ex;
        ex = ( a[0] * op2 ).Reduce();
        a[0] = ex;
        if( a[2].Unarminus( op ) )
          ex = -op1;
        else
          ex = op1;
        a[2] = ex;
        ex = Variable( "x" ) ^ 2;
        if( !( a[2].Constan( r ) && r == 1 ) )
          ex = a[2] * ex;
        if( !( a[1].Constan( r ) && abs( r ) < 0.0000001 ) )
          if( !( a[1].Constan( r ) && r == 1 ) )
            ex += a[1] * Variable( "x" );
          else
            ex += Variable( "x" );
        if( !( a[0].Constan( r ) && abs( r ) < 0.0000001 ) )
          ex += a[0];
        ex = new TBinar( '=', ex, Constant( 0 ) );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        }
      MathExpr D1 = ( a[1] ^ 2 ) - Constant( 4 ) * a[2] * a[0];
      MathExpr D = D1.Reduce();
      if( D.Equal( D1 ) )
        ex = new TBinar( '=', Variable( "D" ), D );
      else
        {
        MathExpr D2 = ExpandExpr( D );
        if( D2.Equal( D ) )
          ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, D ) );
        else
          ex = new TBinar( '=', Variable( "D" ), new TBinar( '=', D1, new TBinar( '=', D, D2 ) ) );
        D = D2;
        }
      TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MDiscrimFound", "Discriminant is found" ), true );
      }
    else
      return BadEq();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result = false;
    }
  s_Precision = OldPrecision;
  s_NoRootReduce = NoRootReduceOld;
  return Result;
  }

MathExpr DetVieEqu( const MathExpr& exi )
  {
  auto BadEq = [&] ( const QString& Msg )
    {
    TSolutionChain::sm_SolutionChain.AddComment( Msg );
    s_GlobalInvalid = true;
    return exi;
    };

  TSolutionChain::sm_SolutionChain.AddExpr( exi );

  if( !exi.EquaCh() )
    return BadEq( X_Str( "MEnterEquat", "Enter Equation!" ) );

  MathExpr a, b, c;
  bool check;
  exi.QuaEquCh( a, b, c, check );
  s_GlobalInvalid = false;
  if( !check )
    return BadEq( X_Str( "MEnterQuadrEquat", "Enter Quadratic Equation!" ) );

  int ia, ib, ic;
  if( !( a.Cons_int( ia ) && b.Cons_int( ib ) && c.Cons_int( ic ) ) || ia == 0 )
    return BadEq( X_Str( "MNotProcDiscrim", "not accessible for( these a,b,c!" ) );

  Lexp P = new TL2exp;
  int id = ib*ib - 4 * ia*ic;
  if( id < 0 )
    {
    MathExpr P1 = new TBinar( '=', Variable( "D", true ), Constant( id ) );
    P1 = new TBinar( '<', P1, Constant( 0 ) );
    P.Addexp( P1.Reduce() );
    TSolutionChain::sm_SolutionChain.AddExpr( P, X_Str( "MNoRealSolutions", "No Real Solutions!" ) );
    return P;
    }

  double rx1 = ( -ib - sqrt( id ) ) / ( 2 * ia );
  double rx2 = ( -ib + sqrt( id ) ) / ( 2 * ia );

  if( abs( round( rx1 ) - rx1 ) > 0.00000001 || abs( round( rx2 ) - rx2 ) > 0.00000001 )
    return BadEq( X_Str( "MNoIntSol", "No integer solution!" ) );

  MathExpr Temp = Variable( "x", true ) ^ 2;
  Temp -= Constant( Round( rx1 + rx2 ) ) * Variable( "x", true );
  Temp += Constant( Round( rx1 * rx2 ) );
  MathExpr P1 = new TBinar( '=', Temp.Reduce(), Constant( 0 ) );

  if( !P1.Eq( exi ) )
    P.Addexp( P1 );

  Temp = Variable( "x", true ) ^ 2;
  P1 = Constant( Round( rx1 ) ) + Constant( Round( rx2 ) );
  P1 *= Variable( "x", true );

  P1 = Temp - P1;
  Temp = Constant( Round( rx1 ) ) * Constant( Round( rx2 ) );
  P1 += Temp;
  P1 = new TBinar( '=', P1, Constant( 0 ) );
  P.Addexp( P1 );

  if( abs( rx1 - rx2 ) > 0.00000001 )
    {
    MathExpr x1 = new TBinar( '=', Variable( "x" + QByteArray( 1, msLowReg ) + "1", true ), Constant( rx1 ) );
    P.Addexp( x1.Reduce() );
    MathExpr x2 = new TBinar( '=', Variable( "x" + QByteArray( 1, msLowReg ) + "2", true ), Constant( rx2 ) );
    P.Addexp( x2.Reduce() );
    }
  else
    {
    MathExpr x1 = new TBinar( '=', Variable( "x" + QByteArray( 1, msLowReg ) + "1", true ), Variable( "x" + QByteArray( 1, msLowReg ) + "2", true ) );
    x1 = new TBinar( '=', x1, Constant( rx1 ) );
    P.Addexp( x1.Reduce() );
    }

  TSolutionChain::sm_SolutionChain.AddExpr( P, X_Str( "MViettSolved", "Viett is Solved!" ));
  s_GlobalInvalid = false;
  return P;
  }

Lexp CalcPolinomEqu( const QByteArray& Source, const QByteArray&& VarName )
  {
  if( Source.isEmpty() ) return nullptr;

  class TPoly : public PascArray<double>
    {
    public:
      TPoly() : PascArray<double>( -10, 23 ) {}
    };

  auto PolyDiv = [] ( const TPoly& a, TPoly& b, int n, int M, TPoly& d, TPoly& r, int& k1, int& k )
    {
    r = a;
    k = n;
    k1 = n - M;
    int j = k1;
    do
      {
      for( ; k > 0 && abs( r[k] ) < 0.0000001; k--, d[--j] = 0 );
      if( k < M || k == M && M == 0 ) break;
      d[j] = r[k] / b[M];
      r[k] = 0;
      for( int t = M - 1; t >= 0; t-- )
        {
        int t1 = t - M + k;
        r[t1] -= b[t] * d[j];
        }
      } while( true );
    };

  QVector<TPoly> g( 11 );
  int Degs[11];
  TPoly dd;
  int nd, FuncCount;

  auto ShturmFunc = [&] ()
    {
    int k = 0;
    do
      {
      PolyDiv( g[k], g[k + 1], Degs[k], Degs[k + 1], dd, g[k + 2], nd, Degs[k + 2] );
      for( int i = 0; i <= Degs[k + 2]; i++ ) g[k + 2][i] = -g[k + 2][i];
      } while( Degs[++k + 1] != 0 );
      FuncCount = k + 2;
    };

  auto NOD = [&] ()
    {
    int k = 0;
    do
      PolyDiv( g[k], g[k + 1], Degs[k], Degs[k + 1], dd, g[k + 2], nd, Degs[k + 2] );
    while( Degs[++k + 1] != 0 );
    if( abs( g[k + 1][0] ) < 0.0000001 )
      {
      PolyDiv( g[0], g[k], Degs[0], Degs[k], g[0], dd, Degs[0], nd );
      Degs[1] = Degs[0] - 1;
      for( int i = 0; i <= Degs[1]; i++ )
        g[1][i] = ( i + 1 )*g[0][i + 1];
      }
    };

  auto Gorner = [] ( TPoly& p, int n, double x )
    {
    double Result = p[n];
    for( int i = n - 1; i >= 0; i-- )
      Result = Result*x + p[i];
    return Result;
    };

  auto SignChangeCount = [&] ( double x )
    {
    int Result = 0;
    double s1 = Gorner( g[0], Degs[0], x );
    for( int i = 1; i < FuncCount; i++ )
      {
      double s2 = Gorner( g[i], Degs[i], x );
      if( s1*s2 < 0 ) Result++;
      s1 = s2;
      }
    return Result;
    };

  double Roots[12];
  std::function<void( double, double )> FindRoots = [&] ( double a, double b )
    {
    int RootsCount = SignChangeCount( a ) - SignChangeCount( b );
    if( RootsCount > 1 )
      {
      double c = ( a + b ) / 2;
      FindRoots( a, c );
      FindRoots( c, b );
      }
    else
      if( RootsCount == 1 )
        {
        double ga = Gorner( g[0], Degs[0], a );
        do
          {
          double c = ( a + b ) / 2;
          double gc = Gorner( g[0], Degs[0], c );
          if( abs( gc ) < 0.1*s_Precision ) break;
          if( ga*gc < 0 )
            b = c;
          else
            a = c;
          } while( abs( a - b ) >= 0.1*s_Precision );
          Roots[++s_RootCount] = ( a + b ) / 2;
        }
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  Lexp Result;

  auto BadEq = [&] ()
    {
    s_Precision = OldPrecision;
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotAcceptEquat", "Wrong type of equation!" ) );
    return Result;
    };

  TExprs a;
  try
    {
    Parser P;
    bool IsName, Mult;
    PNode eq = P.Equation( P.FullPreProcessor( Source, VarName ), VarName, IsName, Mult );
    if( IsFuncEqu( eq ) || TestFrac( eq ) ) return BadEq();
    MathExpr ex = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    FracToMult( eq );
    ex = P.OutPut( eq );
    if( ex.Splitted() ) ex = DelSplitted( ex );
    MathExpr ex1 = ex;
    if( ReduceEquation( ex, VarName ) || !ex.Eq( ex1 ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
    MathExpr op1, op2;
    if( ex.Binar( '=', op1, op2 ) )
      {
      ex1 = ( op1 - op2 ).Reduce();
      MathExpr ex2 = ex1.ReductionPoly( a, VarName );
      ex2 = new TBinar( '=', ex2, Constant( 0 ) );
      if( !ex.Eq( ex2 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
      for( int i = 0; i <= s_DegPoly; i++ )
        {
        op1 = a[i].Reduce();
        if( !op1.IsNumericalValue( g[0][i] ) ) return BadEq();
        }
      int Deg = s_DegPoly;
      for( ; Deg > 0 && g[0][Deg] == 0; Deg-- );
      if( Deg >= 9 ) return BadEq();
      Result = new TL2exp;
      Degs[0] = Deg;
      Degs[1] = Degs[0] - 1;
      for( int i = 0; i <= Degs[1]; i++ )
        g[1][i] = ( i + 1 )*g[0][i + 1];
      TPoly g0 = g[0];
      int degs0 = Degs[0];
      NOD();
      ShturmFunc();
      double M = abs( g[0][0] );
      for( int i = 1; i < Degs[0]; i++ )
        if( abs( g[0][i] )>M ) M = abs( g[0][i] );
      M = M / abs( g[0][Degs[0]] ) + 1;
      s_RootCount = 0;
      FindRoots( -M, M + 0.1 );
      dd[1] = 1;
      int jj = 0;
      for( int j = 1; j <= s_RootCount; j++ )
        {
        dd[0] = -Roots[j];
        g[0] = g0;
        Degs[0] = degs0;
        MathExpr x = Constant( Roots[j] );
        Result.Addexp( x );
        do
          {
          ex1 = new TBinar( '=', Variable( "x_" + NumberToStr( ++jj ) ), x );
          TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
          PolyDiv( g[0], dd, Degs[0], 1, g[0], g[10], Degs[0], Degs[10] );
          } while( abs( Gorner( g[0], Degs[0], Roots[j] ) ) < 0.000001 );
        }
      if( s_RootCount == 0 )
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions!" ) );
      }
    else
      return BadEq();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  s_Precision = OldPrecision;
  return Result;
  }

bool CalcEquation( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return false;
  auto ExprValue = [] ( const MathExpr& ex1, double& x )
    {
    MathExpr ex = ex1.Reduce();
    bool Result = true;
    bool sign = ex.Unarminus( ex );
    int N, D;
    if( ex.SimpleFrac_( N, D ) )
      x = ( double ) N / D;
    else
      if( !ex.Constan( x ) )
        Result = false;
    if( sign )
      x = -x;
    return Result;
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;

  auto InvalidInput = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MLinInv", "Invalid input data!" ) );
    s_Precision = OldPrecision;
    return false;
    };

  try
    {
    Parser P;
    PNode eq = P.AnyExpr( PiVar2PiConst( P.FullPreProcessor( Source, "x" ) ) );
    MathExpr ex = P.OutPut( eq );
    PExMemb f;
    if( ex.Listex( f ) )
      {
      MathExpr ex1 = f->m_Memb;
      MathExpr op1, op2;
      if( !ex1.Binar( '=', op1, op2 ) ) return InvalidInput();
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      ex1 = op1 - op2;
      f = f->m_pNext;
      if( f.isNull() ) return InvalidInput();
      PExMemb f1;
      if( f->m_Memb.Listex( f1 ) ) f = f1;
      double a, b, c, fa, fb, fc;
      if( !ExprValue( f->m_Memb, a ) ) return InvalidInput();
      f = f->m_pNext;
      if( f.isNull() ) return InvalidInput();
      if( !ExprValue( f->m_Memb, b ) ) return InvalidInput();
      f = f->m_pNext;
      if( !f.isNull() ) return InvalidInput();
      MathExpr ex2 = ex1.Substitute( "x", Constant( a ) ).Reduce();
      if( !ExprValue( ex2, fa ) ) return InvalidInput();
      ex2 = ex1.Substitute( "x", Constant( b ) ).Reduce();
      if( !ExprValue( ex2, fb ) ) return InvalidInput();
      if( fa*fb <= 0 )
        {
        if( ( fa != 0 ) && ( fb != 0 ) )
          {
          do
            {
            c = 0.5*( a + b );
            ex2 = ex1.Substitute( "x", Constant( c ) ).Reduce();
            if( !ExprValue( ex2, fc ) ) return InvalidInput();
            if( fc == 0 ) break;
            if( fa*fc < 0 )
              b = c;
            else
              a = c;
            } while( abs( a - b ) > s_Precision );
            c = 0.5*( a + b );
          }
        else
          if( fa == 0 )
            c = a;
          else
            c = b;
        ex2 = new TBinar( '=', Variable( "x" ), Constant( c ) );
        TSolutionChain::sm_SolutionChain.AddExpr( ex2, X_Str( "MRoot", "Roots are found!" ) );
        }
      else
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotFoundRoots", "Roots are ! found." ) );
      }
    else
      return InvalidInput();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    s_Precision = OldPrecision;
    return false;
    }
  s_Precision = OldPrecision;
  return true;
  }

 TSolvReToMult::TSolvReToMult( const MathExpr Expr ) : Solver( Expr ) {}
 TSolvReToMult::TSolvReToMult() : Solver() {}

TSolvExpand::TSolvExpand( const MathExpr Expr ) : Solver( Expr ) {}
TSolvExpand::TSolvExpand() : Solver() { m_Code = 0; m_Name = "MExpandAlHint"; m_DefaultName = "Opening Brackets"; }

TSolvSubSqr::TSolvSubSqr( const MathExpr Expr ) : Solver( Expr ) {}
TSolvSubSqr::TSolvSubSqr() : Solver() { m_Code = 0; m_Name = "MSubtSqrAlHint"; m_DefaultName = "Difference of Squares"; }

TSolvSqrSubSum::TSolvSqrSubSum( const MathExpr Expr ) : Solver( Expr ) {}
TSolvSqrSubSum::TSolvSqrSubSum() : Solver() { m_Code = 0; m_Name = "MSqrSubSumAlHint"; m_DefaultName = "Square of a Sum or a Difference"; }

TSolvSumCub::TSolvSumCub( const MathExpr Expr ) : Solver( Expr ) {}
TSolvSumCub::TSolvSumCub() : Solver() { m_Code = 0; m_Name = "MSumCubAlHint"; m_DefaultName = "Sum of Cubes"; }

TTrinom::TTrinom( const MathExpr Expr ) : Solver( Expr ) {}
TTrinom::TTrinom() : Solver() { m_Code = 0; m_Name = "MTrinomAlHint"; m_DefaultName = "Factoring Trinomial"; }

TSolvCubSubSum::TSolvCubSubSum( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCubSubSum::TSolvCubSubSum() : Solver() { m_Code = 0; m_Name = "MCubSumAlHint"; m_DefaultName = "Cube of Sum or Difference"; }

TSolvSubCub::TSolvSubCub( const MathExpr Expr ) : Solver( Expr ) {}
TSolvSubCub::TSolvSubCub() : Solver() { m_Code = 0; m_Name = "MSubCubAlHint"; m_DefaultName = "Difference of Cubes"; }

Alg1Calculator::Alg1Calculator( const MathExpr Expr ) : Solver( Expr ) {}
Alg1Calculator::Alg1Calculator() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TLg::TLg( const MathExpr Expr ) : Solver( Expr ) {}
TLg::TLg() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

Log1Eq::Log1Eq( const MathExpr Expr ) : Solver( Expr ) {}
Log1Eq::Log1Eq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

SysInEq::SysInEq( const MathExpr Expr ) : Solver( Expr ) {}
SysInEq::SysInEq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

SysInEqXY::SysInEqXY( const MathExpr Expr ) : Solver( Expr ) {}
SysInEqXY::SysInEqXY() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

RatInEq::RatInEq( const MathExpr Expr ) : Solver( Expr ) {}
RatInEq::RatInEq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

ExpEq::ExpEq( const MathExpr Expr ) : Solver( Expr ) {}
ExpEq::ExpEq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

MakeSubstitution::MakeSubstitution( const MathExpr Expr ) : Solver( Expr ) {}
MakeSubstitution::MakeSubstitution() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

SolveLinear::SolveLinear( const MathExpr Expr ) : Solver( Expr ) {}
SolveLinear::SolveLinear() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

MakeExchange::MakeExchange( const MathExpr Expr ) : Solver( Expr ) {}
MakeExchange::MakeExchange() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvDetLinEqu::TSolvDetLinEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvDetLinEqu::TSolvDetLinEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvQuaEqu::TSolvQuaEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvQuaEqu::TSolvQuaEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvDetQuaEqu::TSolvDetQuaEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvDetQuaEqu::TSolvDetQuaEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvDisQuaEqu::TSolvDisQuaEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvDisQuaEqu::TSolvDisQuaEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvDetVieEqu::TSolvDetVieEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvDetVieEqu::TSolvDetVieEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcDetBiQuEqu::TSolvCalcDetBiQuEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcDetBiQuEqu::TSolvCalcDetBiQuEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvFractRatEq::TSolvFractRatEq( const MathExpr Expr ) : Solver( Expr ) {}
TSolvFractRatEq::TSolvFractRatEq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcIrratEq::TSolvCalcIrratEq( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcIrratEq::TSolvCalcIrratEq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcPolinomEqu::TSolvCalcPolinomEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcPolinomEqu::TSolvCalcPolinomEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcSimpleTrigoEq::TSolvCalcSimpleTrigoEq( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcSimpleTrigoEq::TSolvCalcSimpleTrigoEq() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcSinCosEqu::TSolvCalcSinCosEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcSinCosEqu::TSolvCalcSinCosEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcTrigoEqu::TSolvCalcTrigoEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcTrigoEqu::TSolvCalcTrigoEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcHomogenTrigoEqu::TSolvCalcHomogenTrigoEqu( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcHomogenTrigoEqu::TSolvCalcHomogenTrigoEqu() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSolvCalcEquation::TSolvCalcEquation( const MathExpr Expr ) : Solver( Expr ) {}
TSolvCalcEquation::TSolvCalcEquation() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSin::TSin( const MathExpr Expr ) : Solver( Expr ) {}
TSin::TSin() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TCos::TCos( const MathExpr Expr ) : Solver( Expr ) {}
TCos::TCos() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TTan::TTan( const MathExpr Expr ) : Solver( Expr ) {}
TTan::TTan() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TLn::TLn( const MathExpr Expr ) : Solver( Expr ) {}
TLn::TLn() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TDegRad::TDegRad( const MathExpr Expr ) : Solver( Expr ) {}
TDegRad::TDegRad() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TRadDeg::TRadDeg( const MathExpr Expr ) : Solver( Expr ) {}
TRadDeg::TRadDeg() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSciCalc::TSciCalc( const MathExpr Expr ) : Solver( Expr ) {}
TSciCalc::TSciCalc() : Solver() { m_Code = 0; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }
