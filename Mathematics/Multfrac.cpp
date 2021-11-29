#include "ExpObjOp.h"
#include "ResultReceiver.h"
#include "Parser.h"
#include "MathTool.h"
#include "Factors.h"
#include "ExpObBin.h"
#include "Algebra.h"
#include "SolChain.h"

MathExpr DiviExprs(const MathExpr& ExprX, const MathExpr& ExprY, int SignOfResult)
  {
  MathExpr Result = ExprX / ExprY;
  if (SignOfResult == -1) return -Result;
  return Result;
  }

MathExpr DiviExprsS(const MathExpr& ExprX, const MathExpr& ExprY, int SignOfResult)
  {
  if(ExprY == 1) 
    {
    if (SignOfResult == 1) return ExprX;
    return -ExprX;
    }
  return DiviExprs(ExprX, ExprY, SignOfResult);
  }

MathExpr ToFactorsDel(const MathExpr& exi) { return ToFactors(exi); }

MathExpr TExpr::SummSubtOper(int ssSign, const MathExpr& exi, const MathExpr& exi1, const MathExpr& exi2, bool IsDetails)
  {
  MathExpr dividend1, divisor1, dividend2, divisor2;
  int SignOfDivision1, SignOfDivision2;
  MathExpr exp, exp1, exp2;
  MathExpr CommExp;
  MathExpr exm1, exm3, exm4, exm5;
  int cSign;
  MathExpr Temp1, Temp2;
  bool OldAccumulate;
  CheckDivision(exi1, dividend1, divisor1, SignOfDivision1);
  CheckDivision(exi2, dividend2, divisor2, SignOfDivision2);

  dividend1 = ToFactorsDel(dividend1);
  divisor1 = ToFactorsDel(divisor1);
  dividend2 = ToFactorsDel(dividend2);
  divisor2 = ToFactorsDel(divisor2);

  exp1 = DiviExprs(dividend1, divisor1, SignOfDivision1);
  exp2 = DiviExprs(dividend2, divisor2, SignOfDivision2);
  if (ssSign == 1)
    exp = exp1 + exp2;
  else
    exp = exp1 - exp2;
  if (IsDetails && exi.WriteE() != exp.WriteE())
    {
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("Mfraction", "fraction"));
    }
  exm1 = exp;

  CommExp = GetCommon(divisor1, divisor2);
  Temp1 = dividend1;
  Temp2 = dividend2;
  dividend1 = MultExprs(Temp1, divisor2);
  dividend2 = MultExprs(Temp2, divisor1);
  if (CommExp != nullptr)
    {
    Temp1 = MultExprs(divisor1, divisor2);
    Temp2 = divisor2;
    divisor1 = MultExprs(CommExp, Temp1);
    divisor2 = divisor1;
    }
  else
    {
    Temp1 = divisor1;
    Temp2 = divisor2;
    divisor1 = MultExprs(divisor1, divisor2);
    divisor2 = divisor1;
    }
  exp1 = DiviExprs(dividend1, divisor1, SignOfDivision1);
  exp2 = DiviExprs(dividend2, divisor2, SignOfDivision2);
  if (ssSign == +1)
    exp = exp1 + exp2;
  else exp = exp1 - exp2;
  if (IsDetails && exm1.WriteE() != exp.WriteE() )
    {
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("Mcom_div", "common divisor"));
    }
  cSign = +1;
  if (SignOfDivision1 == -1 && SignOfDivision2 == -1 && ssSign == +1 || (SignOfDivision1 == -1 && SignOfDivision2 == +1 && ssSign == -1))
    {
    cSign = -1;
    SignOfDivision1 = +1;
    SignOfDivision2 = +1;
    ssSign = +1;
    }
  dividend1 = NegTExprs(dividend1, SignOfDivision1 == -1);
  dividend2 = NegTExprs(dividend2, SignOfDivision2 == -1);
  if (ssSign == +1)
    exp1 = dividend1 + dividend2;
  else
    exp1 = dividend1 - dividend2;
  exp = DiviExprs(exp1, divisor1, cSign);
  if (IsDetails)
    {
    sm_pResultReceiver->AddExp(exp);
    if (ssSign == +1)
      sm_pResultReceiver->AddComm(X_Str("Msum", "sum"));
    else
      sm_pResultReceiver->AddComm(X_Str("Mdifference", "difference"));
    }
  exm3 = exp;
  exp1 = ExpandExpr(exp1);
  exp = DiviExprs(exp1, ExpandExpr(divisor1), cSign);
  if (IsDetails && exm3.WriteE() != exp.WriteE())
    {
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("Mexpand", "expand"));
    }
  MathExpr Result;
  try
    {
    if (IsDetails)
      TSolutionChain::sm_SolutionChain.Clear();
    OldAccumulate = TSolutionChain::sm_SolutionChain.m_Accumulate;
    TSolutionChain::sm_SolutionChain.m_Accumulate = IsDetails;
    Result = exp.CancellationOfMultiNominals(exp1);
    }
  catch (ErrParser E)
    {
    TSolutionChain::sm_SolutionChain.m_Accumulate = OldAccumulate;
    }
  //  if (IsDetails && TSolutionChain::sm_SolutionChain.Count() > 0 )
  //    TSolutionChain::sm_SolutionChain.DrawCommentedExpressions;
  TSolutionChain::sm_SolutionChain.m_Accumulate = OldAccumulate;
  return Result;
  }

MathExpr TExpr::MultOper(const MathExpr& exi, const MathExpr& exi1, const MathExpr& exi2, bool IsDetails )
  {
  MathExpr dividend1, divisor1, dividend2, divisor2;
  int SignOfDivision1, SignOfDivision2;
  MathExpr texp, exp, exp1, exp2;
  MathExpr exm1, exm2;
  bool Sign;
  CheckDivision(exi1, dividend1, divisor1, SignOfDivision1);
  CheckDivision(exi2, dividend2, divisor2, SignOfDivision2);
  exp1 = ToFactorsDel(dividend1) * ToFactorsDel(dividend2);
  exp2 = ToFactorsDel(divisor1) * ToFactorsDel(divisor2);
  exp = DiviExprs(exp1, exp2, SignOfDivision1*SignOfDivision2);
  exm1 = exp;
  if (IsDetails)
    {
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("Mproduct", "product"));
    }
  exp = GetCommon(exp1, exp2);
  exp = DiviExprs(exp1, exp2, SignOfDivision1*SignOfDivision2);
  exm2 = exp;
  Sign = exm1.WriteE() != exp.WriteE();
  if (IsDetails && Sign)
    {
    exp = DiviExprsS(exp1, exp2, SignOfDivision1*SignOfDivision2);
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("MCancelInfo", "Canceling algebraic fraction"));
    }
  texp = exp1;
  exp1 = ExpandExpr(texp);
  exp = DiviExprs(exp1, exp2, SignOfDivision1*SignOfDivision2).Reduce();
  Sign = exm2.WriteE() != exp.WriteE();
  if (IsDetails && Sign)
    {
    exp = DiviExprsS(exp1, ExpandExpr(exp2), SignOfDivision1*SignOfDivision2).Reduce();
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("Mexpand", "expand"));
    }
  return exp;
  }

MathExpr TExpr::DivvOper(const MathExpr& exi, const MathExpr& exi1, const MathExpr& exi2, bool IsDetails)
  {
  MathExpr dividend1, divisor1, dividend2, divisor2;
  int SignOfDivision1, SignOfDivision2;
  MathExpr exp, exp1, exp2;
  CheckDivision(exi1, dividend1, divisor1, SignOfDivision1);
  CheckDivision(exi2, dividend2, divisor2, SignOfDivision2);

  exp1 = DiviExprs(dividend1, divisor1, SignOfDivision1);
  exp2 = DiviExprs(divisor2, dividend2, SignOfDivision2);
  exp = exp1 * exp2;
  if (IsDetails)
    {
    sm_pResultReceiver->AddExp(exp);
    sm_pResultReceiver->AddComm(X_Str("M2product", "to product"));
    }
  SwapExpr(dividend2, divisor2);
  exp1 = dividend1 / divisor1;
  exp2 = dividend2 / divisor2;
  if (SignOfDivision1 == -1) exp1 = -exp1;
  if (SignOfDivision2 == -1) exp2 = -exp2;
  return MultOper(exp, exp1, exp2, IsDetails);
  }

int TExpr::WhatIsIt( const MathExpr& exi, MathExpr& arg1, MathExpr& arg2 ) // Determine type( +, -, *, / ) of expression. 
  {
  if( exi.Summa( arg1, arg2 ) ) return 1;
  if( exi.Subtr( arg1, arg2 ) ) return 2;
  if( exi.Multp( arg1, arg2 ) ) return 3;
  double V;
  if( exi.Divis( arg1, arg2 ) && arg2.Constan( V ) && V != 0 ) return 4;
  return 0;
  }

bool TExpr::CheckDivision( MathExpr Exp, MathExpr& Dividend, MathExpr& Divisor, int& SignOfDivision )
  {
  SignOfDivision = 1;
  MathExpr ext;
  if( Exp.Unarminus( ext ) )
    {
    Exp = ext;
    SignOfDivision = -1;
    }
  if( Exp.Divis( Dividend, Divisor ) )
    {
    Dividend = Dividend;
    Divisor = Divisor;
    return true;
    }
  int N, D;
  if( Exp.SimpleFrac_( N, D ) )
    {
    Dividend = new TConstant( N );
    Divisor = new TConstant( D );
    return true;
    }
  Dividend = Exp;
  Divisor = new TConstant( 1 );
  return true;
  }

MathExpr TExpr::CalcMulti( int OperDef, const MathExpr& exi, bool IsDetails )
  {
  MathExpr arg1, arg2;
  int Operation = WhatIsIt( exi, arg1, arg2 );
  bool SignOkIn = OperDef == 0 || OperDef == Operation;
  if( SignOkIn && Operation == 4 && !( IsType( TDivi, arg1 ) ) && !( IsType( TDivi, arg2 ) ) )
    {
    arg1 = CalcMulti( 0, arg1, false );
    arg2 = CalcMulti( 0, arg2, false );
    SignOkIn = ( ( IsType( TDivi, arg1 ) || IsType( TDivi, arg2 ) ) && !arg1.IsEmpty() && !arg2.IsEmpty() );
    if( SignOkIn && IsDetails )
      sm_pResultReceiver->AddExp( arg1 / arg2 );
    }
  if( !SignOkIn )
    {
    if( sm_pResultReceiver != nullptr )
      sm_pResultReceiver->AddComm( X_Str( "XPAlgCalcMess", "Different operations!" ) );
    }
  else
    {
    if( IsDetails && sm_pResultReceiver != nullptr )
      sm_pResultReceiver->AddComm( X_Str( "XPAlgCalcMess", "Show operation in details." ) );
    SignOkIn = Operation != 0;
    // first item it is division
    MathExpr dividend1, dividend2, divisor1, divisor2;

    int SignOfDivision1, SignOfDivision2;
    SignOkIn = SignOkIn && CheckDivision( arg1, dividend1, divisor1, SignOfDivision1 );
    // second item it is division
    SignOkIn = SignOkIn && CheckDivision( arg2, dividend2, divisor2, SignOfDivision2 );
    if( !SignOkIn )
      {
      if( IsDetails && sm_pResultReceiver != nullptr )
        sm_pResultReceiver->AddComm( X_Str( "XPAlgCalcMess", "Expression it is!correct operation of two divisions!" ) );
      return nullptr;
      }
    MathExpr exs( exi );
    try
      {
      switch( Operation )
        {
        case 1:
          return SummSubtOper( +1, exs, arg1, arg2, IsDetails );
        case 2:
          return SummSubtOper( -1, exs, arg1, arg2, IsDetails ); // subt
        case 3:
          return MultOper( exs, arg1, arg2, IsDetails );        // mult
        case 4:
          return DivvOper( exs, arg1, arg2, IsDetails );        // divv
        }
      }
    catch( ErrParser E )
      {
      if( sm_pResultReceiver != nullptr )
        sm_pResultReceiver->AddComm( X_Str( "XPAlgCalcMess", "Expression it is not correct operation of two divisions! " ) + E.Name() );
      }    
    }
  return nullptr;
  }

bool DetFracExpr( const MathExpr& exi, TExprs& List )
  {
  MathExpr arg1, arg2;
  bool Result = true;
  if( exi.Summa( arg1, arg2 ) )
    Result = DetFracExpr( arg1, List ) && DetFracExpr( arg2, List );
  else
    if( exi.Subtr( arg1, arg2 ) )
      Result = DetFracExpr( arg1, List ) && DetFracExpr( -arg2, List );
    else
      List.append( exi );
  return Result;
  }

bool IsMulti( const MathExpr& exi )
  {
  return exi.WriteE().indexOf( '/' ) == -1;
  }

MathExpr CalculateMulti( const MathExpr& exp )
  {
  int SignOfDivision;
  MathExpr dividend, divisor;
  bool Sign = TExpr::CheckDivision( exp, dividend, divisor, SignOfDivision );
  Sign = Sign && IsMulti( dividend ) && IsMulti( divisor );
  if( Sign ) return exp;
  MathExpr arg1, arg2;
  if( exp.IsEmpty() ) return MathExpr();
  switch( TExpr::WhatIsIt( exp, arg1, arg2 ) )
    {
    case 3:
      return TExpr::CalcMulti( 0, MultExprs( CalculateMulti( arg1 ), CalculateMulti( arg2 ) ), false );
    case 4:
      return TExpr::CalcMulti( 0, DiviExprs( CalculateMulti( arg1 ), CalculateMulti( arg2 ), 1 ), false );
    }
  return exp;
  }

MathExpr MakeExpr( int Oper, const MathExpr& arg1, const MathExpr& arg2 )
  {
  switch( Oper )
    {
    case 1: return arg1 + arg2;
    case 2: return arg1 - arg2;
    case 3: return arg1 * arg2;
    case 4: return arg1 / arg2;
    }
  return MathExpr();
  }

bool TExpr::SetOfFractions( MathExpr& exr )
  {
  bool OldPowerToFraction = s_PowerToFraction;
  s_PowerToFraction = true;
  MathExpr exi = ReduceTExprs( this );
  s_PowerToFraction = OldPowerToFraction;
  TExprs List;
  bool Result = DetFracExpr( exi, List );
  for( int I = 0; I < List.count(); I++ )
    {
    int Sign = 1;
    MathExpr exp = List[I];
    MathExpr exs;
    if( exp.Unarminus( exs ) )
      {
      Sign = -1;
      exp = exs;
      }
    exp = CalculateMulti( exp );
    if( Sign == -1 ) exp = -exp;
    List[I] = exp;
    }
  Result = Result && List.count() > 1;
  if( Result )
    {
    exr = CalcMulti( 0, MakeExpr( 1, List[0], List[1] ), false );
    for( int I = 2; I < List.count(); I++ ) exr = CalcMulti( 0, MakeExpr( 1, exr, List[I] ), false );
    MathExpr dividend, divisor;
    int SignOfDivision;
    if( CheckDivision( exr, dividend, divisor, SignOfDivision ) )
      {
      MathExpr Temp = dividend;
      dividend = DeletBR( Temp );
      Temp = divisor;
      divisor = DeletBR( Temp );
      exr = DiviExprs( dividend, divisor, SignOfDivision ).SortMult();
      }
    }
  return Result;
  }

MathExpr MathExpr::Divisor2Polinoms() const
  {
  TestPtr();
  MathExpr Result( *this );
  MathExpr exi3, exi4;
  if( Unarminus( exi4 ) ) exi3 = exi4;
  else 
    exi3 = *this;
  MathExpr dividend, divisor, quotient, rest;
  if( exi3.Divis( dividend, divisor ) )
    {
    if( !exi4.IsEmpty() ) dividend = dividend.RetNeg();
    dividend = Expand( dividend );
    divisor = Expand( divisor );
    s_GlobalInvalid = false;
    switch( Divide2Polinoms( dividend, divisor, quotient, rest ) ) 
      {
    case dpOk:
      exi4 = quotient;
      if( rest != 0  )
        {
        rest = ToFactors( rest );
        exi3 = ToFactors( divisor );
        while( !Cancellation( rest, exi3 ).IsEmpty() );
        exi4 += rest / exi3;
        }
      Result = dividend / divisor;
      if( Result.Equal( exi4 ) )
        s_XPStatus.SetMessage( X_Str( "MNoChange", "This action does ! change initial expression!" ) );
      else
        {
        Result = new TBinar( '=', Result, exi4 );
        s_XPStatus.SetMessage(X_Str( "MDivided", "Divided!" ));
        }
      break;
    case dpNPdd:
      s_XPStatus.SetMessage( X_Str( "MDividedDD_NP", "dividend it is ! polinom!" ));
      break;
    case dpNPdr:
      s_XPStatus.SetMessage( X_Str( "MDividedDR_NP", "divisor it is ! polinom!" ));
      break;
    case dpIMPS:
      s_XPStatus.SetMessage( X_Str( "MDividedDR_DP", "divisor have degree more high than dividend!" ));
      break;
    case dpDB:
      s_XPStatus.SetMessage( X_Str( "MDividedDB", "different bases!" ));
      break;
    case dpRCR:s_XPStatus.SetMessage( X_Str( "MDividedRCR", "real coefficients in result!" ));
      }
    }
  else s_XPStatus.SetMessage(X_Str( "MPolErr", "It is ! division of two polinoms!" ));
  return Result;
  }

bool CheckInputOfEquationsSystem( const MathExpr& ex, MathExpArray& List, bool MakeFull, Lexp DenomList )
  {
  PExMemb item;
  bool Result = true;
  bool Sign;
  bool SignF = true;
  MathExpr Temp, NewItem, arg1, arg2;
  if( ex.Listex( item ) )
    {
    bool bPutExp = false;
    while( !item.isNull() && !item->m_Memb.IsEmpty() )
      {
      MathExpr OldItem = item->m_Memb.Clone();
      MathExpr NewItem = RemDenominator( item->m_Memb, DenomList );
      bPutExp = bPutExp || !NewItem.Eq( OldItem );
      item = item->m_pNext;
      }
    if( bPutExp && s_DetailDegree > liPutNone )
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
    for( ex.Listex( item ); !item.isNull() && !item->m_Memb.IsEmpty() && ( Result || MakeFull ); item = item->m_pNext )
      {
      Sign = item->m_Memb.Binar( '=', arg1, arg2 );
      SignF = SignF && Sign;
      if( item->m_Memb.Binar( '=', arg1, arg2 ) )
        Temp = arg1 - arg2;
      else
        Temp = item->m_Memb;
      NewItem = ExpandExpr( Temp );
      Sign = Sign && IsMultiNominal( NewItem );
      Result = Result && Sign;
      if( Result || MakeFull ) List.append( NewItem );
      }
    }
  else
    {
    Sign = ex.Binar( '=', arg1, arg2 );
    SignF = SignF && Sign;
    if( ex.Binar( '=', arg1, arg2 ) )
      Temp = arg1 - arg2;
    else
      Temp = ex;
    NewItem = ExpandExpr( Temp );
    Result = Result && Sign && IsMultiNominal( NewItem );
    if( ( Result || MakeFull ) ) List.append( NewItem );
    }
  if( !SignF ) List.clear();
  return Result;
  }

void FormLimits( Lexp& limits, MathExpr CExpr )
  {
  if( !CExpr.IsEmpty() ) CExpr = ReduceTExprs( CExpr );
  double V;
  if( !CExpr.IsEmpty() && !CExpr.Constan( V ) )
    {
    if( limits.IsEmpty() ) limits = new TLexp;
    MathExpr LimitItem = new TBinar( msNotequal, CExpr, Constant( 0 ) );
    QByteArray Limits = limits.WriteE();
    int P = ( ',' + Limits.mid( 1, Limits.length() - 2 ) + ',' ).indexOf( ',' + LimitItem.WriteE() + ',' );
    if( P == -1 ) limits.Addexp( LimitItem );
    }
  CExpr.Clear();
  }

MathExpr CalcR( MathExpr divisor, MathExpr dividend, Lexp& limits )
  {
  divisor = ToFactors( divisor );
  dividend = ToFactors( dividend );
  MathExpr divisorF = divisor;
  MathExpr dividendF = dividend;
  MathExpr CExpr = Cancellation( divisor, dividend );
  FormLimits( limits, CExpr );
  FormLimits( limits, dividend );
  MathExpr ModDivisor;
  if( divisor == 0 || divisor.Unarminus( ModDivisor ) && ModDivisor == 0 )
    {
    FormLimits( limits, dividend );
    return Constant( 0 );
    }
  MathExpr  op1 = divisorF / dividendF, op2;
  double v1, v2;
  int D;
  if( divisor.Constan( v1 ) && dividend.Constan( v2 ) && !( divisor.Cons_int( D ) && dividend.Cons_int( D ) ) )
    op2 = Constant( v1 / v2 );
  else
    op2 = ExpandExpr( divisor / dividend );
  if( !op1.Eq( op2 ) ) return new TBinar( '=', op1, op2 );
  return op1;
  }

void MathExpr::MakeBiOrSq( MathExpArray& _RootList, bool IsDuplicate )
  {
  int CounterX = 0;
  QByteArray ExiStr = WriteE();
  for( int I = 0; I < ExiStr.length(); I++ )
    if( ExiStr[I] == 'x' ) CounterX++;

  bool SignB4 = CounterX == 2 && ExiStr.indexOf( "x^(4)" ) != -1 && ExiStr.indexOf( "x^(2)" ) != -1 ||
    CounterX == 1 && ExiStr.indexOf( "x^(4)" ) != -1;
  MathExpr exn;
  if( SignB4 )
    {
    int P = ExiStr.indexOf( "x^(2)" );
    if( P > -1 ) ExiStr[P + 3] = '1';
    P = ExiStr.indexOf( "x^(4)" );
    ExiStr[P + 3] = '2';
    exn = Parser().StrToExpr( ExiStr );
    }
  else
    exn = *this;
  exn.DetQuaEqu( _RootList );
  if( SignB4 )
    {
    if( IsDuplicate &&  _RootList.count() == 1 ) _RootList.append( _RootList[0] );
    MathExpArray _TempList;
    for( int I = 0; I < _RootList.count(); I++ )
      {
      MathExpr Temp = _RootList[I].Reduce();
      if( Temp == 0 )
        {
        _TempList.append( Temp );
        _TempList.append( Temp );
        }

      double V;
      if( !( Temp.Constan( V ) && V <= 0 ) )
        {
        _TempList.append( Temp.Root( 2 ).ReduceTExprs() );
        _TempList.append( -( Temp.Root( 2 ).ReduceTExprs() ) );
        }
      }
    _RootList.clear();
    _RootList.append( _TempList );
    }
  }

int MathExpr::CheckAndCheckDivision( MathExpr& exo, MathExpr& limit, Lexp DenomList )
  {
  int Result = 0;
  exo.Clear();
  limit.Clear();
  MathExpArray List;
  bool  Sign = CheckInputOfEquationsSystem( *this, List, true );
  if( List.count() != 2 ) return 2;
  if( !Sign && List.count() > 0 )
    {
    Result = 1;
    limit = new TLexp;
    bool Sign1 = true;
    for( int I = 0; I < List.count() && Sign1; I++ )
      {
      MathExpr exr, dividend, divisor;
      bool Sign0 = IsMultiNominal( List[I] );
      Sign1 = Sign1 &&
        ( Sign0 || List[I].SetOfFractions( exr ) && exr.Divis( dividend, divisor ) && IsMultiNominal( dividend ) );
      if( !Sign0 && Sign1 )
        {
        List[I] = dividend;
        CastPtr( TLexp, limit )->Addexp( new TBinar( msNotequal, divisor, Constant( 0 ) ) );
        }
      }
    if( Sign1 )
      {
      exo = new TL2exp;
      for( int I = 0; I < List.count(); I++ )
        CastPtr( TL2exp, exo )->Addexp( new TBinar( '=', List[I], Constant( 0 ) ) );
      }
    else
      Result = 2;
    }
  return Result;
  }