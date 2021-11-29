#include "Factors.h"
#include "MathTool.h"
#include "Parser.h"
#include "ExpObBin.h"
#include "Algebra.h"
#include "SolChain.h"

const QByteArray TTerm::sm_ParamNames = "abcd";
const int TTerm::sm_TermNotExist = -100;
extern int s_DegPoly;
TExprs s_FactorArray;
int s_Factor;
double s_Multiplier;

bool CompareExpr( const MathExpr& ExprI, const MathExpr& ExprJ )
  {
  ExprI.TestPtr();
  ExprJ.TestPtr();
  double V;
  if( ExprI.Constan( V ) ) return false;
  if( ExprJ.Constan( V ) ) return true;
  if( IsConstType( TRoot, ExprI ) ) return true;
  if( IsConstType( TRoot, ExprJ ) ) return false;
  return ExprI.WriteE() > ExprJ.WriteE();
  }

MathExpr TExpr::SortMult() const
  {
  MathExpr Op1, Op2;
  if( Binar( '=', Op1, Op2 ) ) return Ethis;
  QList<MathExpr> List;
  DetMults( List );
  for( int i = 0; i < List.count() - 1; i++ )
    for( int j = i + 1; j < List.count(); j++ )
      if( CompareExpr( List[i], List[j] ) ) List.swap( i, j );
  MathExpr Result(List[0]), vars;
  if( List.count()>1) 
    {
    MathExpr vars = List[1];
    for( int i = 2; i < List.count(); vars *= List[i++] );
    return (Result *= vars);
    }
  return Result;
  }

void TExpr::DetMults( QList<MathExpr>& List ) const
  {
  MathExpr arg1, arg2;
  if( Multp( arg1, arg2 ) )
    {
    arg1.Ptr()->DetMults( List );
    arg2.Ptr()->DetMults( List );
    }
  else
    List.append(Ethis );
  }

MathExpr MakePower( const MathExpr& Base, const MathExpr& Power )
  {
  Base.TestPtr();
  Power.TestPtr();
  int PwrInt;
  if( !Power.Cons_int( PwrInt ) ) return Base ^ Power;
  switch( PwrInt )
    {
    case 0: return Constant( 1 );
    case 1: return Base;
    default: return Base ^ Power;
    }
  }

bool IsEqualExprs( const MathExpr& Expr1, const MathExpr& Expr2 )
  {
  Expr1.TestPtr();
  Expr2.TestPtr();
  return ReduceTExprs( Expr1 - Expr2 ) == 0;
  }

int CheckPower( int const Number, int NPower )
  {
  int BaseInt = Round( pow( Number, 1.0 / NPower ) );
  int TempInt = 1;
  for( int I = 1; I <= NPower; TempInt *= BaseInt, I++ );
  if( TempInt == Number ) return BaseInt;
  return 0;
  }

MathExpr NegTExprs( const MathExpr& ExprX, bool IsNeg )
  {
  ExprX.TestPtr();
  if( IsNeg ) return ExprX.RetNeg();
  return ExprX;
  }

MathExpr DeletBR( const MathExpr& exp )
  {
  exp.TestPtr();
  TMultiNominal MultiNominal( exp );
  if( MultiNominal.m_IsCorrect ) return MultiNominal.FullExpr();
  return exp;
  }

bool SearchMulti( MathExpr exi, TTerm& Term, bool& ResultInt )
  {
  exi.TestPtr();
  MathExpr arg1, arg2;
  MathExpr narg1, narg2;
  bool Result = true;
  if( exi.Unarminus( exi ) ) Term.m_Sign = -Term.m_Sign;
  if( exi.Multp( arg1, arg2 ) )
    Result = SearchMulti( arg1, Term, ResultInt ) && SearchMulti( arg2, Term, ResultInt );
  else
    {
    double V;
    if( exi.Constan( V ) )
      {
      MathExpr nexi;
      ResultInt = ResultInt && Round( V ) == V;
      if( V < 0 )
        {
        nexi = Constant( -V );
        Term.m_Sign = -Term.m_Sign;
        }
      else
        nexi = exi;
      Term.MultCoeff( nexi );
      }
    else
      {
      if( exi.Power( arg1, arg2 ) )
        {
        int N, D;
        if( arg2.SimpleFrac_( N, D ) )
          {
          Result = exi.IsNumerical();
          if( Result )
            Term.MultCoeff( exi );
          else
            Term.append( TMultiplier( arg1, arg2 ) );
          }
        else
          {
          int argB, argP;
          arg2 = ExpandExpr( arg2 );
          Result = Result && arg2.Cons_int( argP );
          if( Result )
            {
            if( arg1.Cons_int( argB ) )
              {
              arg1.Constan( V );
              Result = Result && V == argB;
              narg2 = arg2.Reduce();
              narg2.Cons_int( argP );
              Term.MultCoeff( Constant( pow( argB, argP ) ) );
              }
            else Term.append( TMultiplier( arg1, arg2 ) );
            }
          }
        }
      else
        {
        narg1.Clear();
        if( exi.Unarminus( narg1 ) ) Term.m_Sign = -Term.m_Sign;
        if( narg1.IsEmpty() ) narg1 = exi;
        Result = Result && !narg1.IsEmpty();
        if( Result )
          {
          int N, D;
          if( narg1.SimpleFrac_( N, D ) )
            {
            if( N < 0 && D >= 0 || D < 0 && N >= 0 ) Term.m_Sign = -Term.m_Sign;
            narg1 = new TSimpleFrac( abs( N ), abs( D ) );
            Term.MultCoeff( narg1 );
            ResultInt = false;
            }
          else
            {
            Result = Result && !narg1.Divis( arg1, arg2 );
            if( Result )
              Term.append( TMultiplier( narg1, Constant( 1 ) ) );
            }
          }
        }
      }
    }
  return Result;
  }

MathExpr CallExpand( const MathExpr& exp )
  {
  exp.TestPtr();
  bool Sign = s_GlobalInvalid;
  MathExpr texp = Expand( exp );
  texp = ReduceTExprs( texp );
  MathExpr Result = ReduceTExprs( Expand( texp ) );
  s_GlobalInvalid = Sign;
  return Result;
  }

int FindPower( int const Number )
  {
  int Result = 1;
  MathExpr cxpr = Constant( ( Number ) );
  MathExpr expr = cxpr.ReduceToMultiplicators();
  MathExpr sxpr = expr;
  QList<MathExpr> SimpleList;
  MathExpr oper1, oper2;
  while( expr.Multp( oper1, oper2 ) )
    {
    SimpleList.append( oper2 );
    expr = oper1;
    }
  SimpleList.append( expr );
  if( SimpleList.count() > 1 )
    {
    TLexp ListExprs;
    for( int I = 0; I < SimpleList.count(); )
      {
      expr = SimpleList[I];
      int CntExpr = 0;
      int J = I;
      for( ; J < SimpleList.count() && ( expr.Eq( SimpleList[J] ) ); J++, CntExpr++ );
      ListExprs.Addexp( Constant( CntExpr ) );
      I = J;
      }
    ListExprs.FindGreatestCommDivisor().Cons_int( Result );
    }
  return Result;
  }

bool CheckThreeTerms( TTerm& TermXs2, TTerm& TermYs2, TTerm& Term2xy )
  {
  bool Result = TermXs2.m_Sign == TermYs2.m_Sign;
  Result = Result &&
    ( 2 * CheckPower( TermXs2.GetCoeffInt(), 2 )*CheckPower( TermYs2.GetCoeffInt(), 2 ) == Term2xy.GetCoeffInt() );
  if( Result )
    {
    MathExpr texp = ReduceTExprs( TermXs2.TermSQRT( 2 ) * TermYs2.TermSQRT( 2 ) );
    MathExpr cexp = Term2xy.Term_SC();
    Result = IsEqualExprs( texp, cexp );
    }
  return Result;
  }

bool CheckFourTerms( TTerm& TermXs3, TTerm& TermYs3, TTerm& Term3x2y, TTerm& Term3xy2 )
  {
  MathExpr texp, texp1, texp2;
  bool Result = TermXs3.m_Sign == Term3xy2.m_Sign && TermYs3.m_Sign == Term3x2y.m_Sign;
  if( Result )
    {
    texp1 = TermXs3.TermSQRT( 3 );
    texp2 = Constant( 2 );
    texp = MakePower( texp1, texp2 ) * TermYs3.TermSQRT( 3 );
    texp = ReduceTExprs( texp );
    texp1 = Term3x2y.Term_SC();
    Result = IsEqualExprs( texp, texp1 );
    }
  if( Result )
    {
    texp1 = TermYs3.TermSQRT( 3 );
    texp2 = Constant( ( 2 ) );
    texp = TermXs3.TermSQRT( 3 ) * MakePower( texp1, texp2 );
    texp = ReduceTExprs( texp );
    texp1 = Term3xy2.Term_SC();
    Result = IsEqualExprs( texp, texp1 );
    }
  Result = Result &&
    3 * CheckPower( TermXs3.GetCoeffInt(), 3 )*CheckPower( TermXs3.GetCoeffInt(), 3 )*CheckPower( TermYs3.GetCoeffInt(), 3 ) ==
    Term3x2y.GetCoeffInt() &&
    3 * CheckPower( TermXs3.GetCoeffInt(), 3 )*CheckPower( TermYs3.GetCoeffInt(), 3 )*CheckPower( TermYs3.GetCoeffInt(), 3 ) ==
    Term3xy2.GetCoeffInt();
  return Result;
  }

bool CheckFourTerms1( TTerm& TermXs2, TTerm& TermYs2, TTerm& TermZs2, TTerm& Term2xy )
  {
  bool Result = TermXs2.m_Sign != TermZs2.m_Sign;
  return Result && CheckThreeTerms( TermXs2, TermYs2, Term2xy );
  }

void SwapExpr( MathExpr& ExprX, MathExpr& ExprY )
  {
  MathExpr exp( ExprX );
  ExprX = ExprY;
  ExprY = exp;
  }

MathExpr CommonTerm( MathExpr& TermX, MathExpr& TermY )
  {
  MathExpr Result;
  bool SignInvert = false;
  auto ctExit = [&] ()
    {
    if( SignInvert && !Result.IsEmpty() ) SwapExpr( TermX, TermY );
    return Result;
    };

  if( TermX.IsEmpty() || TermY.IsEmpty() ) return ctExit();
  TMultiNominal MultiX( TermX );
  TMultiNominal MultiY( TermY );

  if( ( MultiX.count() > 1 ) && ( MultiY.count() > 1 ) && IsEqualExprs( TermX, TermY ) )
    {
    Result = TermX;
    TermX = Constant( 1 );
    TermY = Constant( 1 );
    return ctExit();
    }
  if( MultiX.count() > 1 && MultiY.count() == 1 )
    {
    SignInvert = true;
    TMultiNominal temp = MultiX;
    MultiX = MultiY;
    MultiY = temp;
    }

  if( MultiX.count() == 1 && MultiY.count() > 1 )
    {
    if( !MultiX.m_IsCorrect || !MultiY.m_IsCorrect ) return ctExit();
    for( int I = 0; I < MultiX[0].count(); I++ )
      {
      MathExpr txp = MultiX[0][I].m_Base - MultiY.FullExpr();
      MathExpr exp = CallExpand( txp );
      bool Sign = exp == 0;
      if( Sign )
        {
        Result = MultiX[0][I].m_Base;
        MultiX[0][I].DecPwrInt();
        TermX = MultiX.FullExpr();
        TermY = Constant( 1 );
        return ctExit();
        }
      txp = MultiX[0][I].m_Base + MultiY.FullExpr();
      exp = CallExpand( txp );
      Sign = exp == 0;
      if( Sign )
        {
        Result = MultiX[0][I].m_Base;
        MultiX[0][I].DecPwrInt();
        TermX = MultiX.FullExpr();
        TermY = Constant( -1 );
        return ctExit();
        }
      }
    }
  int FGCDint;
  if( MultiX.count() == 1 && MultiY.count() == 1 )
    {
    if( !MultiX[0].m_Coefficient.IsEmpty() && !MultiY[0].m_Coefficient.IsEmpty() && MultiX[0].GetCoeffInt() != 1 &&
      MultiY[0].GetCoeffInt() != 1 )
      {
      TLexp ListExprs;
      ListExprs.Addexp( MultiX[0].m_Coefficient );
      ListExprs.Addexp( MultiY[0].m_Coefficient );
      int V;
      MathExpr FGCD;
      if( !MultiX[0].m_Coefficient.Cons_int( V ) || !MultiY[0].m_Coefficient.Cons_int( V ) )
        {
        MathExpr txp = MultiX[0].m_Coefficient / MultiY[0].m_Coefficient;
        MathExpr exp = txp.Reduce();
        if( !txp.Eq( exp ) )
          {
          double RV;
          if( exp.Constan( RV ) && !exp.Cons_int( V ) )
            {
            Result.Clear();
            return ctExit();
            }
          Result = MultiY[0].m_Coefficient;
          MultiX[0].m_Coefficient = exp;
          MultiY[0].m_Coefficient = Constant( 1 );
          MultiX[0].PutCoeffInt( 1 );
          MultiY[0].PutCoeffInt( 1 );
          TermX = MultiX.FullExpr();
          TermY = MultiY.FullExpr();
          return ctExit();
          }
        else
          FGCDint = 1;
        }
      else
        {
        FGCD = ListExprs.FindGreatestCommDivisor();
        if( !FGCD.Cons_int( FGCDint ) )
          {
          Result.Clear();
          return ctExit();
          }
        }
      if( FGCDint != 1 )
        {
        Result = FGCD;
        MultiX[0].PutCoeffInt( MultiX[0].GetCoeffInt() / FGCDint );
        MultiY[0].PutCoeffInt( MultiY[0].GetCoeffInt() / FGCDint );
        TermX = MultiX.FullExpr();
        TermY = MultiY.FullExpr();
        return ctExit();
        }
      }
    for( int I = 0; I < MultiX[0].count(); I++ )
      for( int J = 0; J < MultiY[0].count(); J++ )
        {
        MathExpr exp = CallExpand( MultiX[0][I].m_Base - MultiY[0][J].m_Base );
        MathExpr expp = CallExpand( MultiX[0][I].m_Base + MultiY[0][J].m_Base );
        bool Sign = exp == 0 || expp == 0;
        bool SignP = expp == 0;
        if( Sign )
          {
          if( ( IsConstType( TSimpleFrac, MultiX[0][I].m_Power ) ) || ( IsConstType( TSimpleFrac, MultiY[0][J].m_Power ) ) )
            {
            int Cmp = MultiX[0][I].m_Power.Compare( MultiY[0][J].m_Power );
            if( Cmp == 0 )
              {
              Result = MultiX[0][I].m_Base;
              Result = ReduceTExprs( Result );
              MultiX[0][I].m_Base = Constant( 1 );
              MultiY[0][J].m_Base = Constant( 1 );
              MultiX[0][I].m_Power = Constant( 1 );
              MultiY[0][J].m_Power = Constant( 1 );
              }
            else
              if( SignInvert )
                {
                if( Cmp == 1 && !s_RootToPower ) continue;
                Result = MultiX[0][I].m_Base;
                Result = ReduceTExprs( Result );
                exp = ReduceTExprs( MultiX[0][J].m_Power - MultiX[0][I].m_Power );
                MultiX[0][I].m_Base = Constant( 1 );
                MultiX[0][I].m_Power = Constant( 1 );
                MultiY[0][J].m_Power = exp;
                }
              else
                {
                if( ( Cmp == -1 ) && !s_RootToPower ) continue;
                Result = MultiX[0][J].m_Base;
                Result = ReduceTExprs( Result );
                exp = ReduceTExprs( MultiX[0][I].m_Power - MultiY[0][J].m_Power );
                MultiY[0][J].m_Base = Constant( 1 );
                MultiY[0][J].m_Power = Constant( 1 );
                MultiX[0][I].m_Power = exp;
                }
            }
          else
            {
            Result = MultiX[0][I].m_Base;
            Result = ReduceTExprs( Result );
            MultiX[0][I].DecPwrInt();
            MultiY[0][J].DecPwrInt();
            }
          TermX = MultiX.FullExpr();
          TermY = MultiY.FullExpr();
          TermY = NegTExprs( TermY, SignP );
          return ctExit();
          }
        }
    }
  return ctExit();
  }

bool ReducePWR( int& NPwr, int& MinPwr )
  {
  MathExpr expr = Constant( NPwr ).ReduceToMultiplicators();
  MathExpr oper1, oper2;
  bool Result = !expr.Multp( oper1, oper2 );
  while( expr.Multp( oper1, oper2 ) ) expr = oper1;
  expr.Cons_int( MinPwr );
  NPwr = NPwr / MinPwr;
  return Result;
  }

MathExpr ExpandOpRes( const MathExpr& exi )
  {
  exi.TestPtr();
  MathExpr exi_E = Expand( exi );
  if( exi_E.Eq( exi ) )
    {
    if( !s_CalcOnly )
      throw  ErrParser( "I can`t expand it!", peCanNotExpand );
    return exi_E;
    }

  MathExpr Result;
  if( s_ExpandDetailed )
    Result = new TBinar( '=', new TBinar( '=', exi, new TNewLin ), exi_E );

  MathExpr op1, op2;
  int d;
  MathExpr P;
  if( !( exi.Power( op1, op2 ) && op2.Cons_int( d ) && ( d>3 ) ) )
    {
    P = Expand( exi_E );
    while( !P.Eq( exi_E ) || !s_GlobalInvalid )
      {
      if( s_ExpandDetailed )
        if( !P.Equal( exi_E ) )
          Result = new TBinar( '=', new TBinar( '=', Result, new TNewLin ), P );
      exi_E = P;
      P = Expand( exi_E );
      }
    P = DeletBR( exi_E.Reduce() );
    ReduceExpr( P );
    }
  else
    P = exi;

  if( s_ExpandDetailed )
    {
    if( !P.Eq( exi_E ) )
      Result = new TBinar( '=', new TBinar( '=', Result, new TNewLin ), P.Reduce() );
    }
  else
    if( P.Eq( exi ) )
      Result = new TBinar( '=', new TBinar( '=', exi, new TNewLin ), exi_E );
    else
    Result = new TBinar( '=', new TBinar( '=', exi, new TNewLin ), P.Reduce() );
    s_GlobalInvalid = false;

  if( !s_CalcOnly )
    s_XPStatus.SetMessage( X_Str( "MExpanded", "Expanded!" ) );
  return Result;
  }

MathExpr InsertY( const MathExpr& exi, const MathExpr& TermY )
  {
  exi.TestPtr();
  TermY.TestPtr();
  MathExpr Result;
  TMultiNominal multi( exi );
  if( multi.count() == 1 )
    for( int I = 0; I < multi[0].count(); I++ )
      {
      TMultiNominal multic( multi[0][I].m_Base );
      multic.InsertExpr( TermY );
      MathExpr texp = multic.FullExpr();
      if( Result.IsEmpty() )
        Result = CallExpand( texp );
      else
        Result *= CallExpand( texp );
      }

  if( multi.count() > 1 )
    {
    multi.InsertExpr( TermY );
    Result = multi.FullExpr();
    }

  if( Result.IsEmpty() ) Result = exi;
  return Result;
  }

MathExpr IToFactors( const MathExpr& exi )
  {
  exi.TestPtr();
  MathExpr tempX, tempY, tempC, tempC1;
  MathExpr texp;
  MathExpr tpexp;
  MathExpr TermX;
  MathExpr TermY;
  s_GlobalInvalid = true;
  MathExpr Result;

  TMultiNominal MultiNominal( exi );

  if( !( MultiNominal.m_IsCorrect && MultiNominal.m_ResultInt ) || ( MultiNominal.count() < 2 ) )
    return exi;

  if( MultiNominal.IsFullNegative() )
    {
    MultiNominal.SetFullPositive();
    texp = MultiNominal.FullExpr();
    Result = NegTExprs( IToFactors( texp ) );
    s_GlobalInvalid = false;
    return Result;
    }

  MathExpr CommonCoeff = MultiNominal.SearchCommonCoeff();
  int CommonCoeffInt;
  CommonCoeff.Cons_int( CommonCoeffInt );

  if( CommonCoeffInt != 1 )
    {
    MultiNominal.DivideCoeff( CommonCoeffInt );
    MathExpr FExpr = MultiNominal.FullExpr();
    Result = Constant( CommonCoeffInt ) * IToFactors( FExpr );
    s_GlobalInvalid = false;
    return Result;
    }

  MathExpr CommonTerm = MultiNominal.SearchCommonTerm();
  if( !CommonTerm.IsEmpty() )
    {
    MathExpr FExpr = MultiNominal.FullExpr();
    Result = CommonTerm * IToFactors( FExpr );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 2 )
    {
    int NPwr = MultiNominal.StandardCase( TermX, TermY );

    if( NPwr > 1 && MultiNominal[0].m_Sign*MultiNominal[1].m_Sign < 0 )
      {
      if( MultiNominal[0].m_Sign < 0 )
        {
        texp = TermY;
        TermY = TermX;
        TermX = texp;
        }

      int MinPwr;
      ReducePWR( NPwr, MinPwr );
      tempX = TermX;
      tempY = TermY;
      tempC = Constant( NPwr );
      TermX = MakePower( tempX, tempC );
      TermY = MakePower( tempY, tempC );
      tempC = Constant( MinPwr - 1 );
      Result = MakePower( TermX, tempC );

      for( int I = ( MinPwr - 1 ) - 1; I > 0; I-- )
        {
        tempC = Constant( I );
        tempC1 = Constant( MinPwr - 1 - I );
        Result += MakePower( TermX, tempC ) * MakePower( TermY, tempC1 );
        }

      Result += MakePower( TermY, Constant( MinPwr - 1 ) );
      Result = IToFactors( CallExpand( Result ) );
      Result *= IToFactors( CallExpand( TermX - TermY ) );
      s_GlobalInvalid = false;
      return Result;
      }

    if( NPwr > 1 && ( NPwr & 1 ) == 1 && MultiNominal[0].m_Sign*MultiNominal[1].m_Sign > 0 )
      {
      Result = MakePower( TermX, Constant( NPwr - 1 ) );
      Result = NegTExprs( Result, ( NPwr - 1 ) % 2 == 1 );
      for( int I = ( NPwr - 1 ) - 1; I > 0; I-- )
        {
        texp = MakePower( TermX, Constant( I ) ) * MakePower( TermY, Constant( NPwr - 1 - I ) );
        texp = NegTExprs( texp, ( I % 2 ) == 1 );
        Result += texp;
        }
      Result += MakePower( TermY, Constant( ( NPwr - 1 ) ) );
      Result = IToFactors( CallExpand( Result ) );
      Result *= IToFactors( CallExpand( TermX + TermY ) );
      Result = NegTExprs( Result, MultiNominal[0].m_Sign < 0 );
      s_GlobalInvalid = false;
      return Result;
      }
    }

  int SignDEG2;
  if( Result.IsEmpty() && MultiNominal.count() == 3 && MultiNominal.StandardCase_DEG2( TermX, TermY, SignDEG2 ) )
    {
    Result = MakePower( IToFactors( TermX + TermY ), Constant( 2 ) );
    Result = NegTExprs( Result, SignDEG2 == -1 );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 4 && MultiNominal.StandardCase_DEG3( TermX, TermY, SignDEG2 ) )
    {
    Result = MakePower( IToFactors( TermX + TermY ), Constant( 3 ) );
    Result = NegTExprs( Result, SignDEG2 == -1 );
    s_GlobalInvalid = false;
    return Result;
    }

  MathExpr TermXY, TermZ;
  if( Result.IsEmpty() && MultiNominal.count() == 4 && MultiNominal.StandardCase_DEG2p2( TermXY, TermZ, SignDEG2 ) )
    {
    if( SignDEG2 == 1 )
      Result = IToFactors( TermXY + TermZ ) * IToFactors( TermXY - TermZ );
    else
      Result = IToFactors( TermZ + TermXY ) * IToFactors( TermZ - TermXY );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 3 && MultiNominal.StandardCaseGroupS3( TermX, TermY, TermXY, SignDEG2 ) )
    {
    Result = IToFactors( CallExpand( TermXY ) );
    Result *= IToFactors( CallExpand( TermX + TermY ) );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 4 && MultiNominal.StandardCaseGroupS( TermX, TermY, TermXY, SignDEG2 ) )
    {
    Result = IToFactors( CallExpand( TermXY ) );
    Result *= IToFactors( CallExpand( TermX + TermY ) );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 4 && MultiNominal.StandardCaseGroupS4( TermX, TermY, TermXY, SignDEG2 ) )
    {
    Result = IToFactors( CallExpand( TermXY ) );
    Result *= IToFactors( CallExpand( TermX + TermY ) );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 5 && MultiNominal.StandardCaseGroupS5( TermX, TermY, TermXY, SignDEG2 ) )
    {
    texp = CallExpand( TermXY );
    bool Calc = s_CalcOnly;
    bool PA = s_PutAnswer;
    s_CalcOnly = true;
    MathExpr ee2 = ExpandOpRes( texp * ( TermX + TermY ) );
    bool Res = !ee2.IsEmpty() && !ee2.Equal( exi );
    s_PutAnswer = PA;
    s_CalcOnly = Calc;
    Result = IToFactors( texp );
    if( Res )
      tpexp = TermY - TermX;
    else
      tpexp = TermX + TermY;
    Result *= IToFactors( CallExpand( tpexp ) );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() == 6 && MultiNominal.StandardCaseGroupS6( TermX, TermY, TermXY, SignDEG2 ) )
    {
    Result = IToFactors( CallExpand( TermXY ) );
    Result *= IToFactors( CallExpand( TermX + TermY ) );
    s_GlobalInvalid = false;
    return Result;
    }

  if( Result.IsEmpty() && MultiNominal.count() > 1 && MultiNominal.IsForm( TermX, TermY ) )
    return InsertY( IToFactors( CallExpand( MultiNominal.FullExpr() ) ), TermY );

  if( Result.IsEmpty() && MultiNominal.IsClassic() )
    {
    MathExpr ExprX, ExprX1, ExprX2;
    int Part_a1, Part_a2;
    if( MultiNominal.count() == 3 && MultiNominal[0].PowerGCD() % 2 == 0 && MultiNominal[1].PowerGCD() * 2 ==
      MultiNominal[0].PowerGCD() && MultiNominal.ClassicSquare( ExprX, ExprX1, ExprX2, Part_a1, Part_a2 ) )
      {
      s_ClassicTrinom = true;
      MathExpr Expr1( ExprX );
      if( Part_a1 != 1 ) Expr1 *= Constant( Part_a1 );
      MathExpr Expr2( ExprX );
      if( Part_a2 != 1 ) Expr2 *= Constant( Part_a2 );
      Result = IToFactors( CallExpand( Expr1 - ExprX1 ) * IToFactors( CallExpand( Expr2 - ExprX2 )));
      Result = NegTExprs( Result, MultiNominal[0].m_Sign == -1 );
      s_GlobalInvalid = false;
      return Result;
      }

    if( MultiNominal.count() == 3 && MultiNominal[0].PowerInt() % 2 == 0 && MultiNominal[1].PowerInt() * 2 ==
      MultiNominal[0].PowerInt() && MultiNominal[1].PowerInt() % 2 == 0 && MultiNominal[1].GetCoeffInt() == 1 &&
      MultiNominal[1].m_Sign == 1 )
      {
      tpexp = MultiNominal[0].FullTerm();
      tpexp += Constant( 2 ) * MultiNominal[1].FullTerm();
      tpexp += MultiNominal[2].FullTerm();
      TMultiNominal tmulti( tpexp );
      tmulti.IsClassic();
      if( tmulti.StandardCase_DEG2( TermX, TermY, SignDEG2 ) )
        {
        Result = MakePower( TermX + TermY, Constant( 2 ) );
        Result = NegTExprs( Result, SignDEG2 == -1 );
        Result = IToFactors( Result - MultiNominal[1].FullTerm() );
        s_GlobalInvalid = false;
        return Result;
        }
      }

    if( MultiNominal[0].PowerGCD() > 2 && abs( MultiNominal[0].GetCoeffInt() ) == 1 &&
      MultiNominal.IsIntegerRootInFreeTerm( ExprX, ExprX1 ) )
      {
      TMultiNominal tmulti( TMultiNominal( TMultiNominal( MultiNominal, Variable( "x" ) ).Divide( ExprX1 ) ), ExprX );
      MathExpr FE( tmulti.FullExpr() );
      Result = IToFactors( ExprX - ExprX1 ) *  IToFactors( FE );
      s_GlobalInvalid = false;
      return Result;
      }

    int X1, X2;
    if( MultiNominal[0].PowerGCD() > 2 && abs( MultiNominal[0].GetCoeffInt() ) != 1 &&
      MultiNominal.IsFractionRootInFreeTerm( X1, X2 ) )
      {
      TermY = Constant( X2 ) * ( MultiNominal[0].TermBase() - Constant( X1 ) );
      MathExpr tempX;
      MathExpr tempY;
      Divide2Polinoms( MultiNominal.FullExpr(), TermY, tempX, tempY );
      Result = IToFactors( TermY ) * IToFactors( tempX );
      s_GlobalInvalid = false;
      return Result;
      }

    if( MultiNominal[0].PowerInt() > 2 && MultiNominal.IsRootInInterval( ExprX, ExprX1 ) )
      {
      Result = CallExpand( IToFactors( CallExpand( ExprX - ExprX1 ) ) ) *
        CallExpand( IToFactors( CallExpand( MultiNominal.Divide( ExprX1 ) ) ) );
      s_GlobalInvalid = false;
      return Result;
      }
    }
  s_GlobalInvalid = true;
  return exi;
  }

bool IsSubtSq( MathExpr& TermX, MathExpr& TermY )
  {
  TermX.TestPtr();
  TermY.TestPtr();
  MathExpr ExprX = TermX.Reduce();
  MathExpr ExprY = TermY.Reduce();
  MathExpr ModulX, ModulY;
  bool SignU1 = ExprX.Unarminus( ModulX );
  if( !SignU1 ) ModulX = ExprX;
  bool SignU2 = ExprY.Unarminus( ModulY );
  if( !SignU2 ) ModulY = ExprY;
  MathExpr p1, p2, b1, b2;
  bool Result = SignU1 != SignU2 && ModulX.Power( b1, p1 ) && ModulY.Power( b2, p2 ) && p1 == 2 && p2 == 2;
  if( Result )
    {
    if( !SignU1 )
      {
      TermX = b1;
      TermY = b2;
      }
    else
      {
      TermX = b2;
      TermY = b1;
      }
    }
  return Result;
  }

TCounter::TCounter( int AQBits, int AQBins ) : m_Limit( 0 ), m_Value( 0 ), m_QBits( AQBits ), m_QBins( AQBins )
  {
  for( int I = 0; I < AQBits; I++ ) m_Limit = ( m_Limit << 1 ) + 1;
  for( int I = 0; I < AQBins; I++ ) m_Value = ( m_Value << 1 ) + 1;
  SearchBits();
  }

void TCounter::Next()
  {
  if( m_Value == m_Limit ) m_Value++;
  if( m_Value < m_Limit )
    {
    do
      m_Value++;
    while( !BinsOk() );
    SearchBits();
    }
  }

void TCounter::SearchBits()
  {
  m_Cbin = 0;
  m_Czer = 0;
  for( int I = 0; I < m_QBits; I++ )
    if( ( ( m_Value >> I ) & 1 ) == 1 )
      m_Bins[m_Cbin++] = I;
    else
      m_Zers[m_Czer++] = I;
  }

bool TCounter::IsFinish()
  {
  return m_Value > m_Limit;
  }

void TCounter::SetFinish()
  {
  m_Value = m_Limit + 1;
  }

bool TCounter::BinsOk()
  {
  int BinCounter = 0;
  for( int I = 0; I < m_QBits; I++ )
    if( ( ( m_Value >> I ) & 1 ) == 1 ) BinCounter++;
  return BinCounter == m_QBins;
  }

int TCounter::GetB( int I )
  {
  return m_Bins[I];
  }

int TCounter::GetZ( int I )
  {
  return m_Zers[I];
  }

TMultiplier::TMultiplier( const MathExpr& ABase, const MathExpr& APower ) : m_Base(ABase), m_Power(APower)
  {}

MathExpr TMultiplier::RetMulti() const
  {
  return MakePower( m_Base, m_Power );
  }

bool TMultiplier::Eq( const MathExpr& exi, int& NewSign )
  {
  exi.TestPtr();
  MathExpr op1, op2;
  bool Result = IsEqualExprs( m_Base, exi );
  if( Result ) 
    NewSign = +1;
  else 
    NewSign = -1;
  return Result || m_Base.Subtr( op1, op2 ) && IsEqualExprs( m_Base, exi.RetNeg() );
  }

bool TMultiplier::IsDegree( int const NDeg )
  {
  return GetPwrInt() % NDeg == 0;
  }

MathExpr TMultiplier::SQRT( int degree )
  {
  return MakePower( m_Base, Constant(  GetPwrInt() / degree ) );
  }

void TMultiplier::DecPwrInt()
  {
  PutPwrInt( GetPwrInt() - 1 );
  }

int TMultiplier::GetPwrInt() const
  {
  int Result;
  if( m_Power.Cons_int( Result ) ) return Result;
  return 0;
  }

void TMultiplier::PutPwrInt( int Value )
  {
  m_Power = Constant( Value );
  }

TTerm::TTerm( int Sign ) : m_Sign( Sign ), m_Coefficient( Constant( 1 ) )
  {}

void TTerm::MultCoeff( const MathExpr& MCoeff )
  {
  MCoeff.TestPtr();
  if( !m_Coefficient.IsEmpty() )
    m_Coefficient = m_Coefficient * MCoeff;
  else
    m_Coefficient = MCoeff;
  m_Coefficient = ReduceTExprs( m_Coefficient );
  }

bool TTerm::IsDegree( int const NDeg )
  {
  if( CheckPower( GetCoeffInt(), NDeg ) == 0 ) return false;
  for( int I = 0; I < count(); I++ )
    if( !( *this )[I].IsDegree( NDeg ) ) return false;
  return true;
  }

MathExpr TTerm::TermSQRT( int degree )
  {
  MathExpr Result( Constant( 1 ) );
  for( int I = 0; I < count(); I++ ) Result *= ( *this )[I].SQRT( degree );
  return ReduceTExprs( Result );
  }

MathExpr TTerm::FullTerm()
  {
  return NegTExprs( AbsFullTerm(), m_Sign == -1 );
  }

MathExpr TTerm::FullTermWithout( MathExpr& VarX )
  {
  VarX.TestPtr();
  MathExpr Result( Constant( 1 ) );
  for( int I = 0; I < count(); I++ )
    if( !( *this )[I].m_Base.Eq( VarX ) ) Result *= (*this)[I].RetMulti();
  Result = ReduceTExprs( Result );
  return NegTExprs( m_Coefficient * Result, m_Sign == -1 );
  }

MathExpr TTerm::GetVarsOnly( QList<TMultiplier> *pVarsList )
  {
  MathExpr Result( Constant( 1 ) );
  MathExpr exBase;
  for( int I = 0; I < count(); I++ )
    {
    exBase = ( *this )[I].m_Base;
    if( sm_ParamNames.indexOf(exBase.WriteE() ) == -1 && !exBase.ConstExpr() )
      {
      Result *= ( *this )[I].RetMulti();
      if( pVarsList != nullptr ) pVarsList->append( ( *this )[I] );
      }
    }
  return ReduceTExprs( Result );
  }

MathExpr TTerm::GetTermWithoutVars()
  {
  MathExpr Result( Constant( 1 ) );
  MathExpr exBase;
  for( int I = 0; I < count(); I++ )
    {
    exBase = ( *this )[I].m_Base;
    if( sm_ParamNames.indexOf(exBase.WriteE() ) != -1 || exBase.ConstExpr() )
      Result *= ( *this )[I].RetMulti();
    }
  return NegTExprs( m_Coefficient * ReduceTExprs( Result ), m_Sign == -1 );
  }

MathExpr TTerm::AbsFullTerm()
  {
  return m_Coefficient * Term_SC();
  }

MathExpr TTerm::Term_SC()
  {
  MathExpr Result( Constant( 1 ) );
  for( int I = 0; I < count(); I++ )
    Result *= ( *this )[I].RetMulti();
  return ReduceTExprs( Result );
  }

MathExpr TTerm::CoeffAndSign()
  {
  MathExpr exCoeff( NegTExprs( m_Coefficient, m_Sign == -1 ) );
  for( int I = 0; I < count(); I++ )
    {
    MathExpr exConstExpr = ( *this )[I].m_Base;
    if( exConstExpr.ConstExpr() )
      exCoeff = exCoeff * exConstExpr;
    }
  return exCoeff;
  }

MathExpr TTerm::TermBase() const
  {
  MathExpr Result;
  if( count() > 0 ) Result = ( *this )[0].m_Base;
  for( int I = 1; I < count(); I++ ) Result *= (*this)[I].m_Base;
  return Result;
  }

MathExpr TTerm::TermBaseGCD()
  {
  int GCD = PowerGCD();
  MathExpr Result;
  for( int I = 0; I < count(); I++ )
    {
    MathExpr NextItem = MakePower( ( *this )[I].m_Base, Constant( ( *this )[I].GetPwrInt() / GCD ) );
    if( Result.IsEmpty() )
      Result = NextItem;
    else 
      Result *= NextItem;
    }
  return Result;
  }

bool TTerm::IsOneVar( MathExpr& VarX, int& AttrX )
  {
  double V;
  VarX.Clear();
  bool Result = count() > 0;
  int VarCnt = 0;
  for( int I = 0; I < count(); I++ )
    {
    MathExpr exVar = ( *this )[I].m_Base;
    bool VarI = sm_ParamNames.indexOf(exVar.WriteE() ) == -1 && !exVar.ConstExpr();
    bool PwrI = ( *this )[I].GetPwrInt() == 1;
    if( VarI )
      {
      if( !PwrI ) Result = false;
      VarCnt++;
      VarX = ( *this )[I].m_Base;
      }
    }
  Result = Result && VarCnt == 1;
  if( !Result )
    VarX.Clear();
  else
    {
    AttrX = 0;
    if( abs( GetCoeffInt() ) != 1 ) AttrX = 1;
    if( count() > 1 ) AttrX |= 2;
    }
  return Result;
  }

int TTerm::PowerInt()
  {
  if( count() > 0 ) return ( *this )[0].GetPwrInt();
  return 0;
  }

int TTerm::PowerGCD() const
  {
  int Result = 0;
  if( count()>0 )
    {
    TLexp ListExprs;
    for( int I = 0; I < count(); I++ )
      ListExprs.Addexp( Constant( ( *this )[I].GetPwrInt() ) );
    MathExpr TermGCD = ListExprs.FindGreatestCommDivisor();
    if( TermGCD.IsEmpty() )
      Result = 1;
    else
      TermGCD.Cons_int( Result );
    }
  return Result;
  }

int TTerm::Pwr()
  {
  int Result = 0;
  for( int I = 0; I < count(); I++ ) Result += ( *this )[I].GetPwrInt();
  return Result;
  }

bool TTerm::IsLinear()
  {
  bool Result = count() == 0;
  if( !Result )
    {
    int VarCount = 0, VarNo = 0;
    for( int I = 0; I < count(); I++ ) 
      {
      MathExpr exBase = ( *this )[I].m_Base;
      if( sm_ParamNames.indexOf(exBase.WriteE() ) == -1  && !exBase.ConstExpr() )
        {
        VarCount++;
        VarNo = I;
        }
      }
    Result = VarCount == 0 || VarCount == 1 && (*this)[VarNo].GetPwrInt() == 1;
    }
  return Result;
  }

QByteArray TTerm::VarName()
  {
  QByteArray Result("0");
  for( int I = 0; I < count(); I++ )
    {
    MathExpr exBase = ( *this )[I].m_Base;
    if( sm_ParamNames.indexOf(exBase.WriteE() ) == -1 && !exBase.ConstExpr() )
      Result = (*this)[I].m_Base.WriteE();
    }
  return Result;
  }

MathExpr TTerm::Params()
  {
  MathExpr Result;
  for( int I = 0; I < count(); I++ )
    if( sm_ParamNames.indexOf( (*this) [I].m_Base.WriteE()) != -1 )
      {
      if( Result.IsEmpty() )
        Result = ( *this )[I].RetMulti();
      else
        Result *= ( *this )[I].RetMulti();
      }
  return Result;
  }

void TTerm::MakeCommonBase()
  {
  for( int I = 0; I < count() - 1; I++ )
    {   
    for( int J = I + 1; J < count(); )
      {
      if( IsEqualExprs( ( *this )[I].m_Base, ( *this )[J].m_Base ) )
        {
        ( *this )[I].PutPwrInt( ( *this )[I].GetPwrInt() + ( *this )[J].GetPwrInt() );
        removeAt( J );
        }
      else
        J++;
      }
    }
  }

bool TTerm::Compare( const MathExpr& exp1, const MathExpr& exp2 )
  {
  bool Result = count() > 0;
  for( int I = 0; I < count(); I++ )
    Result = Result &&
    ( IsEqualExprs( ( *this )[I].m_Base, exp1 ) || IsEqualExprs( ( *this )[I].m_Base, exp2 ) );
  return Result;
  }

void TTerm::ClearExpr( const MathExpr& exp )
  { 
  exp.TestPtr();
  for( int I = 0; I < count(); )
    if( IsEqualExprs( (*this)[I].m_Base, exp ) ) 
      removeAt( I );
    else 
      I++;
  }

void TTerm::InsertExpr( int MaxPwr, const MathExpr& exp )
  {
  exp.TestPtr();
  int cPwr = MaxPwr - PowerInt();
  if( cPwr > 0 ) append( TMultiplier( exp, Constant( cPwr ) ) );
  }

int TTerm::GetCoeffInt()
  {
  int Result;
  m_Coefficient.Cons_int( Result );
  return Result;
  }

void TTerm::PutCoeffInt( int const NewCoeff )
  {
  m_Coefficient = Constant( NewCoeff );
  }

double TTerm::GetCoeffReal() const
  {
  double Result;
  m_Coefficient.Constan( Result );
  return Result;
  }

void TTerm::PutCoeffReal( double const NewCoeff )
  {
  m_Coefficient = Constant( NewCoeff );
  }

bool TTerm::HasVariable( const QByteArray& VarName, int Power )
  {
  if( VarName == "" ) return count() == 0;
  bool Result = false;
  for( int I = 0; I < count(); I++ )
    {
    QByteArray sName = ( *this )[I].m_Base.WriteE();
    if( sm_ParamNames.indexOf(sName ) > -1 ) continue;
    if( sName == VarName )
      Result = ( *this )[I].GetPwrInt() == Power;
    else
      return Result;
    }
  return Result;
  }

bool TTerm::HasProduct( const QByteArray& Var1, const QByteArray& Var2, int Power1, int Power2 )
  {
  bool Result = false;
  int N = 0;
  for( int I = 0; I < count(); I++ )
    {
    QByteArray sName = ( *this )[I].m_Base.WriteE();
    if( sm_ParamNames.indexOf(sName ) > -1 ) continue;
    if( sName == Var1 )
      {
      Result = ( *this )[I].GetPwrInt() == Power1;
      if( !Result ) return false;
      N++;
      }
    else
      if( sName == Var2 )
        {
        Result = ( *this )[I].GetPwrInt() == Power2;
        if( !Result ) return false;
        N++;
        }
      else
        return false;
    }
  return Result && N == 2;
  }

TMultiNominal::TMultiNominal( const MathExpr& exi ) : m_IsCorrect( exi.IsLinear() ), m_ResultInt( true )
  {
  exi.TestPtr();
  m_IsCorrect = SearchSumma( exi, m_ResultInt ) && m_IsCorrect;
  if( m_IsCorrect )
    {
    DefaultCoefficient();
    MakeCommonBase();
    int value;
    if( m_IsCorrect && !CallExpand( FullExpr() ).Cons_int( value ) ) IsPolinom();
    }
  }

TMultiNominal::TMultiNominal( const TMultiNominal& MN, const MathExpr& VarsName ) : m_IsCorrect( true )
  {
  for( int I = 0; I < MN.count(); I++ )
    {
    TTerm Term;
    Term.m_Sign = MN[I].m_Sign;
    Term.PutCoeffReal( MN[I].GetCoeffReal() );
    if( !MN[I].TermBase().IsEmpty() ) Term.append( TMultiplier( VarsName, Constant( MN[I].PowerGCD() ) ) );
    append( Term );
    }
  }

bool TMultiNominal::SearchSumma( const MathExpr& exi, bool& ResultInt )
  {
  exi.TestPtr();
  MathExpr args, arg1, arg2;
  MathExpr narg1, narg2;
  bool Result = true;
  MathExpr exp = exi.Reduce();
  if( exp.Summa( arg1, arg2 ) )
    {
    Result = SearchSumma( arg1, ResultInt );
    Result = SearchSumma( arg2, ResultInt ) && Result;
    }
  else
    if( exp.Subtr( arg1, arg2 ) )
      {
      narg2 = arg2.RetNeg();
      Result = SearchSumma( arg1, ResultInt );
      Result = SearchSumma( narg2, ResultInt ) && Result;
      }
    else
      if( exp.Unarminus( args ) && args.Summa( arg1, arg2 ) )
        {
        narg1 = ReduceTExprs( arg1.RetNeg() );
        Result = Result && SearchSumma( narg1, ResultInt );
        narg2 = ReduceTExprs( arg2.RetNeg() );
        Result = Result && SearchSumma( narg2, ResultInt );
        }
      else
        {
        TTerm Term;
        Result = Result && SearchMulti( exp, Term, ResultInt );
        append( Term );
        }
  return Result;
  }

void TMultiNominal::MakeCommonBase()
  {
  for( int I = 0; I < count(); I++ ) (*this)[I].MakeCommonBase();
  }

bool TMultiNominal::IsVarOnly1time( MathExpr& VarX )
  {
  VarX.TestPtr();
  int CntVarX = 0;
  QByteArray X = VarX.WriteE();
  for( int I = 0; I < count(); I++ )
    if( ( *this )[I].Term_SC().WriteE().indexOf(X) != -1 ) CntVarX++;
  return CntVarX == 1;
  }

void TMultiNominal::ChangeVar( MathExpr& VarX, MathExpr& VarY )
  {
  VarX.TestPtr();
  VarY.TestPtr();
  for( int I = 0; I < count(); I++ )
    for( int J = 0; J < ( *this )[I].count(); J++ )
      if( ( *this )[I][J].m_Base.Eq( VarX ) )
        ( *this )[I][J].m_Base = VarY;
  }

void TMultiNominal::GetAllVars( QByteArrayList& VarsList )
  {
  for( int I = 0; I < count(); I++ )
    for( int J = 0; J < ( *this )[I].count(); J++ )
      {
      MathExpr exBase = ( *this )[I][J].m_Base;
      if( TTerm::sm_ParamNames.indexOf(exBase.WriteE() ) == -1 && !exBase.ConstExpr() )
        {
        QByteArray Var = ( *this )[I][J].m_Base.WriteE();
        if( VarsList.isEmpty() )
          {
          VarsList.append(Var);
          continue;
          }
        int iMax = VarsList.count() - 1;
        for( ; iMax >= 0 && VarsList[iMax] > Var; iMax-- );
        if( iMax == -1 )
          {
          VarsList.prepend( Var );
          continue;
          }
        if( VarsList[iMax] != Var ) VarsList.insert( ++iMax, Var );
        }
      }
  }

bool TMultiNominal::IsFullNegative()
  {
  bool Result = count() > 0;
  for( int I = 0; I < count(); I++ ) Result = Result && ( *this )[I].m_Sign == -1;
  return Result;
  }

void TMultiNominal::SetFullPositive()
  {
  for( int I = 0; I < count(); I++ ) ( *this )[I].m_Sign = 1;
  }

MathExpr TMultiNominal::SearchCommonCoeff()
  {
  TLexp ListExprs;
  for( int I = 0; I < count(); I++ ) ListExprs.Addexp( ( *this )[I].m_Coefficient );
  return ListExprs.FindGreatestCommDivisor();
  }

void TMultiNominal::DivideCoeff( int CoeffInt )
  {
  for( int I = 0; I < count(); I++ )
    ( *this )[I].PutCoeffInt( ( *this )[I].GetCoeffInt() / CoeffInt );
  }

void TMultiNominal::DefaultCoefficient()
  {
  for( int I = 0; I < count(); I++ )
    if( ( *this )[I].m_Coefficient.IsEmpty() ) ( *this )[I].PutCoeffInt( 1 );
  for( int I = 0; I < count() && count() > 1; )
    if( ( ( *this )[I].GetCoeffReal() == 0 ) )
      removeAt( I );
    else
      I++;
  }

MathExpr TMultiNominal::FullExpr()
  {
  MathExpr Result;
  if( count() > 0 ) Result = ReduceTExprs( ( *this )[0].FullTerm() );
  for( int I = 1; I < count(); I++ )
    if( ( *this )[I].m_Sign == +1 ) Result += ReduceTExprs( ( *this )[I].AbsFullTerm() );
    else
      Result -= ReduceTExprs( ( *this )[I].AbsFullTerm() );
  return Result;
  }

MathExpr TMultiNominal::SearchCommonTerm()
  {
  int MinPwr;
  int NewSign;
  MathExpr Result( Constant( 1 ) );
  for( int K = 0; K < ( *this )[0].count(); K++ )
    {
    MathExpr sexpr = ( *this )[0][K].m_Base;
    MinPwr = ( *this )[0][K].GetPwrInt();
    bool SignOfMulti = true;
    for( int I = 0; I < count(); I++ )
      {
      bool SignOfTerm = false;
      if( ( *this )[I].count()>0 )
        {
        for( int J = 0; J < ( *this )[I].count(); J++ )
          if( ( *this )[I][J].Eq( sexpr, NewSign ) )
            {
            int CurPwr = ( *this )[I][J].GetPwrInt();
            if( MinPwr > CurPwr ) MinPwr = CurPwr;
            SignOfTerm = true;
            break;
            }
        }
      SignOfMulti = SignOfMulti && SignOfTerm;
      if( !SignOfMulti ) break;
      }

    if( SignOfMulti )
      {
      Result *= MakePower( sexpr, Constant( MinPwr ) );
      for( int I = 0; I < count(); I++ )
        for( int J = 0; J < ( *this )[I].count(); J++ )
          if( ( *this )[I][J].Eq( sexpr, NewSign ) )
            {
            ( *this )[I].m_Sign = ( *this )[I].m_Sign*NewSign;
            int CurPwr = ( *this )[I][J].GetPwrInt();
            ( *this )[I][J].PutPwrInt( CurPwr - MinPwr );
            break;
            }
      }
    }

  Result = ReduceTExprs( Result );
  if( Result == 1 ) Result.Clear();
  return Result;
  }

bool TMultiNominal::IsLinear()
  {
  bool Result = true;
  for( int I = 0; I < count(); I++ )
    Result = Result && ( *this )[I].IsLinear();
  return Result;
  }

int TMultiNominal::StandardCase( MathExpr& TermX, MathExpr& TermY )
  {
  int Result = -1;
  int CoeffX, CoeffY;
  if( count() == 2 )
    {
    TLexp ListExprs;
    for( int I = 0; I < ( *this )[0].count(); I++ ) ListExprs.Addexp( ( *this )[0][I].m_Power );
    for( int I = 0; I < ( *this )[1].count(); I++ ) ListExprs.Addexp( ( *this )[1][I].m_Power );

    if( ( *this )[0].GetCoeffInt()>1 ) ListExprs.Addexp( Constant( FindPower( ( *this )[0].GetCoeffInt() ) ) );
    if( ( *this )[1].GetCoeffInt()>1 ) ListExprs.Addexp( Constant( FindPower( ( *this )[1].GetCoeffInt() ) ) );

    if( !ListExprs.FindGreatestCommDivisor().Cons_int( Result ) ) Result = -1;
    if( Result > 1 )
      {
      CoeffX = CheckPower( ( *this )[0].GetCoeffInt(), Result );
      if( CoeffX < 1 )
        Result = -1;
      else
        {
        CoeffY = CheckPower( ( *this )[1].GetCoeffInt(), Result );
        if( CoeffY < 1 ) Result = -1;
        }
      }

    if( Result > 1 )
      {
      TermX = Constant( 1 );
      for( int I = 0; I < ( *this )[0].count(); I++ )
        {
        MathExpr expt = ( *this )[0][I].m_Base;
        MathExpr texp = Constant( ( *this )[0][I].GetPwrInt() / Result );
        expt = MakePower( expt, texp );
        TermX = TermX * expt;
        }
      TermX = TermX * Constant( CoeffX );
      TermX = ReduceTExprs( TermX );

      TermY = Constant( 1 );
      for( int I = 0; I < ( *this )[1].count(); I++ )
        TermY *= MakePower( ( *this )[1][I].m_Base, Constant( ( *this )[1][I].GetPwrInt() / Result ) );
      TermY *= Constant( CoeffY );
      TermY = ReduceTExprs( TermY );
      }
    }
  return Result;
  }

bool TMultiNominal::StandardCase_DEG2( MathExpr& TermX, MathExpr& TermY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = +1;
  if( count() == 3 )
    {
    TCounter Counter3( 3, 2 );
    int I = Counter3.GetB( 0 );
    int J = Counter3.GetB( 1 );
    int K = Counter3.GetZ( 0 );
    while( !Counter3.IsFinish() )
      {
      if( ( *this )[I].IsDegree( 2 ) && ( *this )[J].IsDegree( 2 ) && CheckThreeTerms( ( *this )[I], ( *this )[J], ( *this )[K] ) )
        {
        TermX = Constant( CheckPower( ( *this )[I].GetCoeffInt(), 2 ) ) *  ( *this )[I].TermSQRT( 2 );
        TermY = Constant( CheckPower( ( *this )[J].GetCoeffInt(), 2 ) ) *  ( *this )[J].TermSQRT( 2 );
        if( ( *this )[I].m_Sign == -1 ) SignDEG2 = -1;
        TermY = NegTExprs( TermY, ( *this )[K].m_Sign == -1 || ( *this )[I].m_Sign == -1 );
        Result = true;
        Counter3.SetFinish();
        }
      Counter3.Next();
      I = Counter3.GetB( 0 );
      J = Counter3.GetB( 1 );
      K = Counter3.GetZ( 0 );
      }
    }
  return Result;
  }

bool TMultiNominal::StandardCase_DEG3( MathExpr& TermX, MathExpr& TermY, int& SignDEG3 )
  {
  bool Result = false;
  SignDEG3 = 1;
  if( count() == 4 )
    {
    TCounter Counter4( 4, 2 );
    int I = Counter4.GetB( 0 );
    int J = Counter4.GetB( 1 );
    int K = Counter4.GetZ( 0 );
    int L = Counter4.GetZ( 1 );
    while( !Counter4.IsFinish() )
      {
      if( ( *this )[I].IsDegree( 3 ) && ( *this )[J].IsDegree( 3 ) && CheckFourTerms( ( *this )[I], 
        ( *this )[J], ( *this )[K], ( *this )[L] ) || CheckFourTerms( ( *this )[I], ( *this )[J], ( *this )[L], ( *this )[K] ) )
        {
        TermX = Constant( CheckPower( ( *this )[I].GetCoeffInt(), 3 ) ) * ( *this )[I].TermSQRT( 3 );
        TermX = NegTExprs( TermX, ( *this )[I].m_Sign == -1 );
        TermY = Constant( CheckPower( ( *this )[J].GetCoeffInt(), 3 ) ) * ( *this )[J].TermSQRT( 3 );
        TermY = NegTExprs( TermY, ( *this )[J].m_Sign == -1 );
        Result = true;
        Counter4.SetFinish();
        };
      Counter4.Next();
      I = Counter4.GetB( 0 );
      J = Counter4.GetB( 1 );
      K = Counter4.GetZ( 0 );
      L = Counter4.GetZ( 1 );
      }
    }
  return Result;
  }

bool TMultiNominal::StandardCase_DEG2p2( MathExpr& TermX, MathExpr& TermY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = 1;
  if( count() == 4 )
    {
    TCounter Counter4( 4, 2 );
    int I = Counter4.GetB( 0 );
    int J = Counter4.GetB( 1 );
    int K = Counter4.GetZ( 0 );
    int L = Counter4.GetZ( 1 );
    while( !Counter4.IsFinish() )
      {
      if( ( *this )[I].IsDegree( 2 ) && ( *this )[J].IsDegree( 2 ) )
        {
        if( ( *this )[K].IsDegree( 2 ) && CheckFourTerms1( ( *this )[I], ( *this )[J], ( *this )[K], ( *this )[L] ) )
          {
          TermX = Constant( CheckPower( ( *this )[I].GetCoeffInt(), 2 ) ) * ( *this )[I].TermSQRT( 2 );
          TermY = Constant( CheckPower( ( *this )[J].GetCoeffInt(), 2 ) ) * ( *this )[J].TermSQRT( 2 );
          if( ( *this )[I].m_Sign == ( *this )[K].m_Sign )
            TermX = TermX + TermY;
          else
            TermX = TermX - TermY;
          if( ( *this )[I].m_Sign == -1 ) SignDEG2 = -1;
          TermY = Constant( CheckPower( ( *this )[K].GetCoeffInt(), 2 ) ) * ( *this )[K].TermSQRT( 2 );
          Result = true;
          Counter4.SetFinish();
          }
        else
          if( ( *this )[L].IsDegree( 2 ) && CheckFourTerms1( ( *this )[I], ( *this )[J], ( *this )[L], ( *this )[K] ) )
            {
            TermX = Constant( CheckPower( ( *this )[I].GetCoeffInt(), 2 ) ) * ( *this )[I].TermSQRT( 2 );
            TermY = Constant( CheckPower( ( *this )[J].GetCoeffInt(), 2 ) ) * ( *this )[J].TermSQRT( 2 );
            if( ( *this )[I].m_Sign == ( *this )[K].m_Sign )
              TermX = TermX + TermY;
            else
              TermX = TermX - TermY;
            if( ( *this )[I].m_Sign == -1 ) SignDEG2 = -1;
            TermY = Constant( CheckPower( ( *this )[L].GetCoeffInt(), 2 ) ) * ( *this )[L].TermSQRT( 2 );
            Result = true;
            Counter4.SetFinish();
            }
        }
      Counter4.Next();
      I = Counter4.GetB( 0 );
      J = Counter4.GetB( 1 );
      K = Counter4.GetZ( 0 );
      L = Counter4.GetZ( 1 );
      }
    }
  return Result;
  }

bool TMultiNominal::StandardCaseGroupS3( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = 1;
  if( count() == 3 )
    {
    TCounter Counter3( 3, 2 );
    int I = Counter3.GetB( 0 );
    int J = Counter3.GetB( 1 );
    int K = Counter3.GetZ( 0 );
    while( !Counter3.IsFinish() )
      {
      TermX = ( *this )[I].FullTerm() + ( *this )[J].FullTerm();
      TermY = ( *this )[K].FullTerm();
      TermXY = CommonTerm( TermX, TermY );
      if( !TermXY.IsEmpty() )
        {
        Result = true;
        Counter3.SetFinish();
        }
      Counter3.Next();
      I = Counter3.GetB( 0 );
      J = Counter3.GetB( 1 );
      K = Counter3.GetZ( 0 );
      }
    }
  if( !Result ) s_GlobalInvalid = true;
  return Result;
  }

bool TMultiNominal::StandardCaseGroupS( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = 1;
  if( count() == 4 )
    {
    TCounter Counter4( 4, 2 );
    int I = Counter4.GetB( 0 );
    int J = Counter4.GetB( 1 );
    int K = Counter4.GetZ( 0 );
    int L = Counter4.GetZ( 1 );
    while( !Counter4.IsFinish() )
      {
      TermX = IToFactors( ( *this )[I].FullTerm() + ( *this )[J].FullTerm() );
      bool Sign1 = s_GlobalInvalid;
      TermY = IToFactors( ( *this )[K].FullTerm() + ( *this )[L].FullTerm() );
      bool Sign2 = s_GlobalInvalid;
      if( !( Sign1 && Sign2 ) )
        {
        TermXY = CommonTerm( TermX, TermY );
        if( !TermXY.IsEmpty() )
          {
          Result = true;
          Counter4.SetFinish();
          }
        }
      Counter4.Next();
      I = Counter4.GetB( 0 );
      J = Counter4.GetB( 1 );
      K = Counter4.GetZ( 0 );
      L = Counter4.GetZ( 1 );
      }
    }
  if( !Result ) s_GlobalInvalid = true;
  return Result;
  }

bool TMultiNominal::StandardCaseGroupS4( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = 1;
  if( count() == 4 )
    {
    TCounter Counter4( 4, 3 );
    int I = Counter4.GetB( 0 );
    int J = Counter4.GetB( 1 );
    int K = Counter4.GetB( 2 );
    int L = Counter4.GetZ( 0 );
    while( !Counter4.IsFinish() )
      {
      TermX = IToFactors( ( *this )[I].FullTerm() + ( *this )[J].FullTerm() + ( *this )[K].FullTerm() );
      TermY = ( *this )[L].FullTerm();
      TermXY = CommonTerm( TermX, TermY );
      if( !TermXY.IsEmpty() )
        {
        Result = true;
        Counter4.SetFinish();
        }
      Counter4.Next();
      I = Counter4.GetB( 0 );
      J = Counter4.GetB( 1 );
      K = Counter4.GetB( 2 );
      L = Counter4.GetZ( 0 );
      }
    }
  if( !Result ) s_GlobalInvalid = true;
  return Result;
  }

bool TMultiNominal::StandardCaseGroupS5( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = 1;
  if( count() == 5 )
    {
    TCounter Counter5( 5, 3 );
    int I = Counter5.GetB( 0 );
    int J = Counter5.GetB( 1 );
    int K = Counter5.GetB( 2 );
    int L = Counter5.GetZ( 0 );
    int M = Counter5.GetZ( 1 );
    while( !Counter5.IsFinish() )
      {
      TermX = IToFactors( ( *this )[I].FullTerm() + ( *this )[J].FullTerm() + ( *this )[K].FullTerm() );
      TermY = IToFactors( ( *this )[L].FullTerm() + ( *this )[M].FullTerm() );
      TermXY = CommonTerm( TermX, TermY );
      if( !TermXY.IsEmpty() )
        {
        Result = true;
        Counter5.SetFinish();
        }
      Counter5.Next();
      I = Counter5.GetB( 0 );
      J = Counter5.GetB( 1 );
      K = Counter5.GetB( 2 );
      L = Counter5.GetZ( 0 );
      M = Counter5.GetZ( 1 );
      }
    }
  if( !Result ) s_GlobalInvalid = true;
  return Result;
  }

bool TMultiNominal::StandardCaseGroupS6( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 )
  {
  bool Result = false;
  SignDEG2 = 1;
  if( count() == 6 )
    {
    TCounter Counter6( 6, 3 );
    int I1 = Counter6.GetB( 0 );
    int I2 = Counter6.GetB( 1 );
    int I3 = Counter6.GetB( 2 );
    int I4 = Counter6.GetZ( 0 );
    int I5 = Counter6.GetZ( 1 );
    int I6 = Counter6.GetZ( 2 );
    while( !Counter6.IsFinish() )
      {
      TermX = IToFactors( ( *this )[I1].FullTerm() + ( *this )[I2].FullTerm() + ( *this )[I3].FullTerm() );
      bool Sign1 = s_GlobalInvalid;
      TermY = IToFactors( ( *this )[I4].FullTerm() + ( *this )[I5].FullTerm() + ( *this )[I6].FullTerm() );
      bool Sign2 = s_GlobalInvalid;
      TermXY.Clear();
      if( !( Sign1 && Sign2 ) )
        {
        TermXY = CommonTerm( TermX, TermY );
        if( !TermXY.IsEmpty() )
          {
          Result = true;
          Counter6.SetFinish();
          }
        else
          {
          Result = IsSubtSq( TermX, TermY );
          if( Result )
            {
            Counter6.SetFinish();
            TermXY = TermX - TermY;
            }
          }
        }
      Counter6.Next();
      I1 = Counter6.GetB( 0 );
      I2 = Counter6.GetB( 1 );
      I3 = Counter6.GetB( 2 );
      I4 = Counter6.GetZ( 0 );
      I5 = Counter6.GetZ( 1 );
      I6 = Counter6.GetZ( 2 );
      }
    }
  if( !Result ) s_GlobalInvalid = true;
  return Result;
  }

bool TMultiNominal::IsForm( MathExpr& TermX, MathExpr& TermY )
  {
  int SingleArray[2];
  bool Result = count() > 1;
  if( Result )
    {
    int PwrTerm = ( *this )[0].Pwr();
    for( int I = 1; I < count(); I++ )
      Result = Result && ( *this )[I].Pwr() == PwrTerm;
    Result = Result && PwrTerm > 1;
    if( Result )
      {
      int saCnt = 0;
      for( int I = 0; I < count(); I++ )
        if( ( *this )[I].count() == 1 )
          {
          Result = saCnt < 2;
          if( Result )
            SingleArray[saCnt++] = I;
          }
      Result = Result && saCnt == 2;
      if( Result )
        {
        MathExpr TermBase0 = ( *this )[SingleArray[0]].TermBase();
        MathExpr TermBase1 = ( *this )[SingleArray[1]].TermBase();
        for( int I = 0; I < count(); I++ ) Result = Result && ( *this )[I].Compare( TermBase0, TermBase1 );
        }
      if( Result )
        {
        TermX = ( *this )[SingleArray[0]].TermBase();
        TermY = ( *this )[SingleArray[1]].TermBase();
        ClearExpr( TermY );
        }
      }
    }
  return Result;
  }

void TMultiNominal::ClearExpr( const MathExpr& exp )
  {
  for( int I = 0; I < count(); I++ ) ( *this )[I].ClearExpr( exp );
  }

void TMultiNominal::InsertExpr( const MathExpr& exp )
  {
  exp.TestPtr();
  int MaxPwr = 0;
  for( int I = 0; I < count(); I++ )
    if( MaxPwr < ( *this )[I].PowerInt() ) MaxPwr = ( *this )[I].PowerInt();
  if( MaxPwr > 0 )
    for( int I = 0; I < count(); I++ ) ( *this )[I].InsertExpr( MaxPwr, exp );
  }

bool TMultiNominal::IsClassic()
  {
  return count() > 1 && IsPolinom();
  }

bool TMultiNominal::IsMultiTerm( MathExpr& TermX )
  {
  bool Result = count() > 1 && ( *this )[0].count()>1;
  if( Result ) TermX = ( *this )[0].TermBase();
  return Result;
  }

bool TMultiNominal::IsPolinom()
  {
  bool Result = count() == 1 && ( *this )[0].count() <= 1;
  if( !Result && count() > 1 )
    {
    int I;
    if( ( *this )[0].TermBase().IsEmpty() )
      I = 1;
    else
      I = 0;
    MathExpr ExprX = ( *this )[I].TermBase();
    MathExpr BaseX = ( *this )[I].TermBaseGCD();
    QSet<uchar> degres;
    for( int I = 0; I < count(); I++ )
      {
      MathExpr texp = ( *this )[I].TermBase();
      MathExpr bexp = ( *this )[I].TermBaseGCD();
      Result = texp.IsEmpty() || IsEqualExprs( bexp, BaseX ) && !degres.contains( ( *this )[I].PowerGCD() );
      if( !bexp.IsEmpty() ) degres += ( *this )[I].PowerGCD();
      if( !Result ) break;
      }
    if( Result && ( count() > 1 ) )
      for( int I = 0; I < count() - 1; I++ )
        for( int J = I + 1; J < count(); J++ )
          if( ( *this )[I].PowerGCD() < ( *this )[J].PowerGCD() )
            {
            TTerm TermX = ( *this )[I];
            ( *this )[I] = ( *this )[J];
            ( *this )[J] = TermX;
            }
    }
  return Result;
  }

bool TMultiNominal::ClassicSquare( MathExpr& TermX, MathExpr& TermX1, MathExpr& TermX2, int& Part_a1, int& Part_a2 )
  {
  Part_a1 = 1;
  Part_a2 = 1;
  bool Result = false;
  int a = ( *this )[0].m_Sign*( *this )[0].GetCoeffInt(), b = 0, c = 0;
  switch( count() )
    {
    case 2:
      if( ( *this )[1].PowerInt() == 0 )
        c = ( *this )[1].m_Sign*( *this )[1].GetCoeffInt();
      else
        b = ( *this )[1].m_Sign*( *this )[1].GetCoeffInt();
      break;
    case 3:
      b = ( *this )[1].m_Sign*( *this )[1].GetCoeffInt();
      c = ( *this )[2].m_Sign*( *this )[2].GetCoeffInt();
    }

  int D = b*b - 4 * a*c;
  if( ( D > 0 ) && ( CheckPower( D, 2 ) > 0 ) )
    {
    int x11, x12, x21, x22;
    double x1 = ( -b - CheckPower( D, 2 ) ) / ( 2 * a );
    double x2 = ( -b + CheckPower( D, 2 ) ) / ( 2 * a );
    if( Frac( x1 ) == 0 && Frac( x2 ) == 0 )
      {
      TermX1 = Constant( Round( x1 ) );
      TermX2 = Constant( Round( x2 ) );
      TermX = ( *this )[1].Term_SC();
      Result = true;
      }
    else
      {
      if( Frac( x1 ) != 0 )
        {
        x11 = -b - CheckPower( D, 2 );
        x12 = 2 * a;
        TLexp ListExprs;
        ListExprs.Addexp( Constant( abs( x11 ) ) );
        ListExprs.Addexp( Constant( abs( x12 ) ) );
        MathExpr CommonXE = ListExprs.FindGreatestCommDivisor();
        int CommonX;
        CommonXE.Cons_int( CommonX );
        x11 = x11 / CommonX;
        x12 = x12 / CommonX;
        }
      else
        {
        x11 = Round( x1 );
        x12 = 1;
        }
      if( Frac( x2 ) != 0 )
        {
        x21 = -b + CheckPower( D, 2 );
        x22 = 2 * a;
        TLexp ListExprs;
        ListExprs.Addexp( Constant( abs( x21 ) ) );
        ListExprs.Addexp( Constant( abs( x22 ) ) );
        MathExpr CommonXE = ListExprs.FindGreatestCommDivisor();
        int CommonX;
        CommonXE.Cons_int( CommonX );
        x21 = x21 / CommonX;
        x22 = x22 / CommonX;
        }
      else
        {
        x21 = Round( x2 );
        x22 = 1;
        }

      if( x12 < 0 )
        {
        x11 = -x11;
        x12 = -x12;
        }
      if( x22 < 0 )
        {
        x21 = -x21;
        x22 = -x22;
        }

      TermX1 = Constant( x11 );
      TermX2 = Constant( x21 );
      Part_a1 = x12;
      Part_a2 = x22;
      TermX = ( *this )[1].Term_SC();
      Result = true;
      }
    }
  return Result;
  }

bool TMultiNominal::IsRoot( int X1, MathExpr& ExprX1 )
  {
  int Summa = 0;
  for( int I = 0; I < count(); I++ )
    Summa += ( *this )[I].m_Sign * ( *this )[I].GetCoeffInt() * Round( pow( X1, ( *this )[I].PowerInt() ) );
  bool Result = Summa == 0;
  if( Result ) ExprX1 = Constant( X1 );
  return Result;
  }

bool TMultiNominal::IsIntegerRootInFreeTerm( MathExpr& ExprX, MathExpr& ExprX1 )
  {
  if( ( *this )[count() - 1].PowerInt() != 0 ) return false;
  bool Result = false;
  int FreeTerm = ( *this )[count() - 1].GetCoeffInt();
  for( int X1 = 1; X1 <= FreeTerm && !Result; X1++ )
    Result = FreeTerm % X1 == 0 && ( IsRoot( X1, ExprX1 ) || IsRoot( -X1, ExprX1 ) );
  if( Result ) ExprX = ( *this )[0].TermBaseGCD();
  return Result;
  }

bool TMultiNominal::IsFractionRoot( int& X1, int& X2 )
  {
  MathExpr x = ( *this )[0].TermBase();
  MathExpr Dividend = FullExpr();
  MathExpr Divisor = Constant( X2 ) * x - Constant( X1 );
  MathExpr Quotient, Rest;
  return Divide2Polinoms( Dividend, Divisor, Quotient, Rest ) == dpOk && Rest == 0;
  }

bool TMultiNominal::IsFractionRootInFreeTerm( int& X1, int& X2 )
  {
  if( ( *this )[count() - 1].PowerInt() != 0 ) return false;
  bool Result = false;
  int FreeTerm = ( *this )[count() - 1].GetCoeffInt();
  int A0 = ( *this )[0].GetCoeffInt();
  X1 = 1;
  int Counter = 0;
  while( X1 <= FreeTerm && !Result && Counter <= sm_MaxFractionRootIters )
    {
    if( FreeTerm % X1 == 0 )
      {
      X2 = 1;
      while( X2 <= A0 && !Result && Counter <= sm_MaxFractionRootIters )
        {
        if( A0 % X2 == 0 )
          {
          Counter++;
          Result = IsFractionRoot( X1, X2 );
          if( !Result )
            {
            X1 = -X1;
            Counter++;
            Result = IsFractionRoot( X1, X2 );
            if( !Result ) X1 = abs( X1 );
            }
          }
        if( !Result ) X2++;
        }
      }
    if( !Result ) X1++;
    }
  return Result;
  }

bool TMultiNominal::IsRootInInterval( MathExpr& ExprX, MathExpr& ExprX1 )
  {
  int maxValueOfRoot = LimitOfRoot();
  bool Result = false;
  if( maxValueOfRoot > 20 ) maxValueOfRoot = 20;
  for( int I = -maxValueOfRoot; I <= maxValueOfRoot && !Result; I++ )
    Result = IsRoot( I, ExprX1 );
  if( Result ) ExprX = ( *this )[0].TermBase();
  return Result;
  }

MathExpr TMultiNominal::Divide( const MathExpr& ExprX1 )
  {
  ExprX1.TestPtr();
  MathExpr Result = Constant( 0 );
  TMultiNominal Delimoe( FullExpr() );
  Delimoe.IsClassic();
  MathExpr Chastnoe;
  do
    {
    MathExpr TermA = MakePower( Delimoe[0].TermBase(), Constant( Delimoe[0].PowerInt() - 1 ) );
    TermA *= Delimoe[0].m_Coefficient;
    TermA = ReduceTExprs( NegTExprs( TermA, Delimoe[0].m_Sign == -1 ) );
    Result = ReduceTExprs( Result + TermA );
    MathExpr Delitel = ReduceTExprs( TermA * (*this )[0].TermBase() - TermA * ExprX1 );
    Chastnoe = ReduceTExprs( Delimoe.FullExpr() - Delitel );
    Delimoe = TMultiNominal( Chastnoe );
    Delimoe.IsClassic();
    } while( Chastnoe != 0 );
  return Result;
  }

int TMultiNominal::LimitOfRoot()
  {
  int Result = ( *this )[0].GetCoeffInt();
  for( int I = 0; I < count(); I++ )
    if( Result < ( *this )[I].GetCoeffInt() ) Result = ( *this )[I].GetCoeffInt();
  Result = Round( 2 + ( *this )[0].GetCoeffInt() / ( double ) Result );
  return Result;
  }

void TMultiNominal::Grouping()
  {
  QList<TTerm> lstNewList;
  QList<TMultiplier> lstVars;
  bool bWasHanged = false;
  for( int I = 0; I < count(); I++ )
    {
    if( ( *this )[I].isEmpty() ) continue;
    MathExpr exCoeff;
    if( ( *this )[I].VarName() == "0" )
      lstNewList.append( ( *this )[I] );
    else
      {
      lstVars.clear();
      MathExpr exVars = ( *this )[I].GetVarsOnly( &lstVars );
      exCoeff = ( *this )[I].GetTermWithoutVars();
      for( int J = I + 1; J < count(); J++ )
        {
        if( ( *this )[J].isEmpty() ) continue;
        if( exVars.Equal( ( *this )[J].GetVarsOnly() ) )
          {
          exCoeff += ( *this )[J].GetTermWithoutVars();
          bWasHanged = true;
          ( *this )[J].clear();
          }
        }
      TTerm trmNewTerm;
      for( int J = 0; J < lstVars.count(); J++ )
        trmNewTerm.append( lstVars[J] );
      trmNewTerm.append( TMultiplier( exCoeff, Constant( 1 ) ) );
      lstNewList.append( trmNewTerm );
      }
    }
  if( bWasHanged )
    {
    clear();
    for( int I = 0; I < lstNewList.count(); I++ )
      append( lstNewList[I] );
    }
  }

TTerm TMultiNominal::HasVariable( const QByteArray& VarName, int Power )
  {
  for( int I = 0; I < count(); I++ )
    if( ( *this )[I].HasVariable( VarName, Power ) )
      return ( *this )[I];
  return TTerm( TTerm::sm_TermNotExist );
  }

TTerm TMultiNominal::HasProduct( const QByteArray& Var1, const QByteArray& Var2, int Power1, int Power2 )
  {
  for( int I = 0; I < count(); I++ )
    if( ( *this )[I].HasProduct( Var1, Var2, Power1, Power2 ) )
      return ( *this )[I];
  return TTerm();
  }


MultNomVector::~MultNomVector()
  {
  for( auto it = begin(); it != end(); it++ )
    delete *it;
  }

DivPolResult Divide2Polinoms( const MathExpr& Dividend, const MathExpr& Divisor, MathExpr& Quotient, MathExpr& Rest )
  {
  /*
  {   Dividend - input polionom-dividend                           }
  {   Divisor  - input polinom-divisor                             }
  {   Quotient - output polinom-quotient                           }
  {   Rest     - output polinom-rest                               }
  {                                                                }
  {   Dividend:= Quotient *  Divisor + Rest                        }
  {                                                                }
  {   Function returns longint value:                              }
  {    dpOk   = 0   - ok                                           }
  {    dpNPdd = -1  - dividend it is not polinom                   }
  {    dpNPdr = -2  - divisor it is not polinom                    }
  {    dpIMPS = -4  - divisor have degree more high than dividend  }
  */
  Dividend.TestPtr();
  Divisor.TestPtr();
  QByteArray Name, Name1, Name2;
  auto IsExprPolynom = [&] ( const MathExpr& ex, int& n, MathExpArray&  a )
    {
    MathExpArray q;
    double r;
    int i;
    bool Result = false;
    try
      {
      ex.ReductionPoly( q, Name );
      }
    catch( ErrParser Err )
      {
      return Result;
      }

    for( n = q.count() - 1; n >= 0 && q[n].Constan( r ) && r == 0; n-- );
    if( n == q.count() - 1 ) return Result;
    for( int i = 0; i <= n; i++ ) a[i] = q[i];
    return true;
    };

  auto DivExprPoly = [&] ( MathExpArray& a, MathExpArray& b, int n, int m, MathExpArray& d, MathExpArray& r, int& L, int& k )
    {
    if( n > m )
      L = n - m;
    else
      {
      L = 0;
      d[0] = Constant(0);
      }
    for( int i = 0; i <= n; i++ )
      r[i] = a[i];
    double v;
    for( k = n; k >= m; )
      {
      d[k - m] = ( r[k] / b[m] ).Reduce();
      for( int i = m; i >= 0; i-- )
        r[k - m + i] = ( r[k - m + i] - d[k - m] * b[i] ).Reduce();
      k--;
      while( k > m && r[k].Constan( v ) && abs( v ) < 1e-9 )
        d[k-- - m] = Constant( 0 );
      };
    if( k < 0 ) k = 0;
    while( k > 0 && r[k].Constan( v ) && ( abs( v ) < 1e-9 ) )
      k--;
    };

  auto MultyVar = [&] ( const MathExpr& ex, QByteArray& Name )
    {
    QByteArray VName;

    std::function<void( const MathExpr& )> PreOrder = [&] ( const MathExpr& ex )
      {
      MathExpr op1, op2;
      char c;
      if( ex.Variab( VName ) )
        {
        if( Name.isEmpty() )
          Name = VName;
        else
          if( Name != VName )
            throw  1;
        }
      else
        if( ex.Oper_( c, op1, op2 ) )
          {
          PreOrder( op1 );
          PreOrder( op2 );
          }
        else
          if( ex.Unarminus( op1 ) )
            PreOrder( op1 );
      };

    Name = "";
    bool Result = false;
    try
      {
      PreOrder( ex );
      }
    catch( int )
      {
      Result = true;
      }
    return Result;
    };

  s_DegPoly = 20;
  if( MultyVar( Dividend, Name1 ) || MultyVar( Divisor, Name2 ) || ( ( Name1 != Name2 ) && ( Name1 != "" ) && ( Name2 != "" ) ) )
    Name = "x";
  else
    if( Name1 != "" )
      Name = Name1;
    else
      if( Name2 != "" )
        Name = Name2;
      else
        Name = "x";
  int nDividend, nDivisor, nQuotient, nRest;
  MathExpArray exa(20), exb(20), exd(20), exr(20);
  if( !IsExprPolynom( Dividend, nDividend, exa ) )
    {
    s_DegPoly = 8;
    return dpNPdd;
    }
  if( !IsExprPolynom( Divisor, nDivisor, exb ) )
    {
    s_DegPoly = 8;
    return dpNPdr;
    }
  if( nDivisor > nDividend )
    {
    s_DegPoly = 8;
    return dpIMPS;
    }
  DivExprPoly( exa, exb, nDividend, nDivisor, exd, exr, nQuotient, nRest );
  Quotient = CreateExprPoly( exd, nQuotient, Name );
  Rest = CreateExprPoly( exr, nRest, Name );
  s_DegPoly = 8;
  return dpOk;
  }
  
MathExpr ToFactors( const MathExpr& exi )
  {
  MathExpr Result = IToFactors( exi );
  QByteArray VarName = exi.HasUnknown();
  if( VarName == "" ) VarName = "x";
  if( !s_GlobalInvalid )
    {
    TMultiNominal MultiNominal( Result );
    if( MultiNominal.m_IsCorrect && MultiNominal.m_ResultInt )
      return MultiNominal.FullExpr();
    }
  MathExpr op1, op2;
  if( !s_GlobalInvalid && Result.Multp( op1, op2 ) )
    {
    Result = ToFactors( op1 ) * ToFactors( op2 );
    s_GlobalInvalid = false;
    return Result;
    }
  return exi;
  }
  
MathExpr MakeTrinom( const MathExpr& exp )
  {
  MathExpr trinom, zero, Result;
  bool Sign = !( exp.Binar( '=', trinom, zero ) && zero == 0 );
  if( Sign ) 
    {
    trinom = exp;
    Result = ToFactors( trinom );
    if( s_GlobalInvalid || Result.Eq( exp ) )
      s_XPStatus.SetMessage( X_Str( "MCanNotFactor", "I can`t factor it!" ) );
    else 
      {
      Result = new TBinar( '=', exp, Result );
      s_XPStatus.SetMessage( X_Str( "MFactorized", "Factorized!" ) );
      }
    }
  else 
    {
    Result = exp;
    s_XPStatus.SetMessage( X_Str( "MAnswMistake", "Wrong!" ) );
    }
  return Result;
  }

MathExpr CuSbSm( const MathExpr& exi )
  {
  MathExpr pow3 = Constant( 3 );
  MathExpr pow2 = Constant( 2 );
  MathExpr a, b, SbSm, pw, exi1, Result;
  int st;
  if( ( exi.Subtr( a, b ) || exi.Summa( a, b ) ) || ( exi.Power( SbSm, pw ) && ( SbSm.Subtr( a, b ) ||
    SbSm.Summa( a, b ) ) && ( pw.Cons_int( st ) && ( st == 3 ) ) ) )
    {
    if( exi.Subtr( a, b ) || exi.Summa( a, b ) )
      {
      SbSm = exi;
      exi1 = exi ^ pow3;
      }
    else
      exi1 = exi;

    MathExpr a_3 = a ^ pow3;
    MathExpr b_3 = b ^ pow3;
    MathExpr a_2b3 = ( a ^ pow2 ) * b;
    MathExpr a_2b3r = pow3 * a_2b3.Reduce();
    a_2b3 = pow3 * a_2b3;
    MathExpr ab_23 = a * ( b ^ pow2 );
    MathExpr ab_23r = pow3 * ab_23.Reduce();
    ab_23 = pow3 * ab_23;
    MathExpr P, Pr;

    if( SbSm.Subtr( a, b ) )
      {
      P = a_3 - a_2b3;
      Pr = a_3.Reduce() - a_2b3r.Reduce();
      }

    if( SbSm.Summa( a, b ) )
      {
      P = a_3 + a_2b3;
      Pr = a_3.Reduce() + a_2b3r.Reduce();
      }

    P += ab_23;
    Pr += ab_23r.Reduce();

    if( SbSm.Subtr( a, b ) )
      {
      P -= b_3;
      Pr -= b_3.Reduce();
      }

    if( SbSm.Summa( a, b ) )
      {
      P += b_3;
      Pr += b_3.Reduce(); //Pr == Reduced( a3 + 3a ^ 2b + 3ab ^ 2 + b ^ 3 )
      }

    MathExpr sq = new TBinar( '=', exi1, P );
    if( Pr.Eq( P ) )
      Result = sq;
    else
      {
      MathExpr sqr = new TBinar( '=', sq, Pr );
      MathExpr ex12 = Pr.Reduce();
      if( Pr.Eq( ex12 ) )
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
  s_XPStatus.SetMessage( X_Str( "MCalculated", "Calculated!" ) );
  return Result;
  }

MathExpr Cancellation( MathExpr& ExprX, MathExpr& ExprY )
  {
  return CommonTerm( ExprX, ExprY );
  }

MathExpr GetCommon( MathExpr& ExprX, MathExpr& ExprY )
  {
  MathExpr exp, Result;
  do
    {
    exp = Cancellation( ExprX, ExprY );
    if( !exp.IsEmpty() )
      {
      if( !Result.IsEmpty() )
        Result *= exp;
      else
        Result = exp;
      }
    } while( !exp.IsEmpty() );
    return Result;
  }

MathExpr MultExprs( const MathExpr& ExprX, const MathExpr& ExprY )
  {
  if( ExprX.IsEmpty() && !ExprY.IsEmpty() ) return ExprY;
  if( !ExprX.IsEmpty() && ExprY.IsEmpty() ) return ExprX;
  if( ExprX.IsEmpty() && ExprY.IsEmpty() ) return MathExpr();
  if( ExprX == 1 ) return ExprY;
  if( ExprY == 1 ) return ExprX;
  return ExprX * ExprY;
  }

int GetFactorCount( const MathExpr& ex, const QByteArray& VarName )
  {
  std::function<void( MathExpr )> SearchFactors = [&] ( MathExpr exp )
    {
    MathExpr exLeft, exRight;
    if( exp.Multp( exLeft, exRight ) )
      {
      SearchFactors( exLeft );
      SearchFactors( exRight );
      return;
      }
    if( exp.HasUnknown( VarName ).isEmpty() ) return;
    if( exp.Power( exLeft, exRight ) && !exLeft.HasUnknown( VarName ).isEmpty() && exRight.HasUnknown( VarName ).isEmpty() ) exp = exLeft;
    for( int i = 0; i < s_FactorArray.count(); i++ )
      if( s_FactorArray[i].Eq( exp ) ) return;
    s_FactorArray.append( exp );
    };

  if( s_FactorizedSolving ) return 0;
  s_FactorArray.clear();
  s_RootCount = 0;
  s_Factor = -1;
  MathExpr exLeft, exRight;
  int i;
  if( ex.Binar( '=', exLeft, exRight ) )
    {
    if( !exRight.Cons_int( i ) || i != 0 ) return 0;
    SearchFactors( exLeft );
    }
  else
    SearchFactors( ex );
  return s_FactorArray.count();
  }

MathExpr GetPutRoot( const MathExpr& ex, const QByteArray& VarName )
  {
  s_RootCount++;
  MathExpr Result = new TBinar( '=', Variable( VarName + '_' + QByteArray::number( s_RootCount ) ), ex );
  TSolutionChain::sm_SolutionChain.AddExpr( Result, "", true );
  return Result;
  }

TL2exp* RootPolinom( MathExpr ex )
  {
  const int ItMax = 50, NDigit = 10;
  int N, TermPower;
  char Name;
  QByteArray Unknown;
  PascArray<double> h( -2, 103 ), b( -2, 103 ), c( -2, 103 ), d( -2, 103 ), e( -2, 103 ), acc( 1, 99 );

  std::function<void( MathExpr )> MaxPower = [&] ( MathExpr ex )
    {
    if( N == -1 ) return;
    MathExpr Left, Right;
    double V;
    int Nom, Denom;
    QByteArray sName;
    if( !ex.Oper_( Name, Left, Right ) )
      {
      ex.Unarminus( ex );
      if( ex.Constan( V ) || ex.SimpleFrac_( Nom, Denom ) ) return;
      if( ex.Variab( sName ) )
        if( Unknown.isEmpty() )
          {
          Unknown = sName;
          if( N == 0 ) N = 1;
          return;
          }
        else
          if( sName == Unknown ) return;
      if( !ex.Oper_( Name, Left, Right ) )
        {
        N = -1;
        return;
        }
      }
    if( Name == '-' || Name == '+' || Name == '*' )
      {
      MaxPower( Left );
      if( N != -1 ) MaxPower( Right );
      return;
      }
    if( Name == '^' && Right.Cons_int( TermPower ) )
      {
      Left.Variab( sName );
      if( Unknown.isEmpty() )
        Unknown = sName;
      else
        if( sName != Unknown )
          {
          N = -1;
          return;
          }
      N = std::max( N, TermPower );
      }
    else
      N = -1;
    };

  std::function<void( const MathExpr&, double )> GetCoeff = [&] ( const MathExpr& ex, double Sign )
    {
    MathExpr Left, Right, exTmp;
    double V;
    int pwr, Nom, Denom;
    char Name;
    if( ex.Oper_( Name, Left, Right ) )
      {
      if( Name == '-' || Name == '+' )
        {
        GetCoeff( Left, Sign );
        if( Name == '-' ) Sign *= -1;
        GetCoeff( Right, Sign );
        return;
        }
      if( Name == '^' )
        {
        Right.Cons_int( pwr );
        h[N - pwr] = Sign;
        return;
        }
      if( IsConstType( TConstant, Right ) )
        {
        exTmp = Right;
        Right = Left;
        Left = exTmp;
        }
      if( IsConstType( TVariable, Right ) )
        pwr = 1;
      else
        {
        Right.Power( exTmp, Right );
        Right.Cons_int( pwr );
        }
      if( !Left.Constan( V ) )
        {
        Left.SimpleFrac_( Nom, Denom );
        V = ( double ) Nom / Denom;
        }
      h[N - pwr] = V * Sign;
      return;
      }
    if( ex.Unarminus( Left ) )
      {
      GetCoeff( Left, Sign * -1 );
      return;
      }
    if( ex.Constan( V ) )
      {
      h[N] = V * Sign;
      return;
      }
    else
      if( ex.SimpleFrac_( Nom, Denom ) )
        {
        h[N] = ( double ) Nom / Denom * Sign;
        return;
        };
    h[N - 1] = Sign;
    };

  double t, kk, ps, qs, pt, qt, s, rev, r, p, q;
  MathExpr Root1, Root2, exTmp;
  auto GetRoots = [&] ()
    {
    if( t < 0 )
      {
      p = p / q;
      q = 1 / q;
      }
    s = q - std::pow( p / 2, 2 );
    if( abs( s ) < 10 / kk ) s = 0;
    if( s > 0 )
      {
      s = sqrt( s );
      Root1 = CreateComplex( Constant( -p / 2 ), Constant( s ) );
      Root2 = CreateComplex( Constant( -p / 2 ), Constant( -s ) );
      return;
      };
    s = sqrt( -s );
    if( p < 0 )
      r = -p / 2 + s;
    else
      r = -p / 2 - s;
    Root1 = Constant( r );
    Root2 = Constant( q / r );
    };

  TL2exp* pResult = new TL2exp;
  MathExpr Left, Right;
  if( !ex.Binar( '=', Left, Right ) ) return pResult;
  Left -= Right;
  ex = ExpandExpr( Left );
  N = 0;
  MaxPower( ex );
  if( N < 1 ) return pResult;
  int iOut;
  if( !ex.Oper_( Name, Left, Right ) )
    {
    for( int i = 1; i <= N; i++ )
      {
      pResult->Addexp( new TBinar( '=', new TIndx( Variable( Unknown ), Constant( i ) ), Constant( 0 ) ) );
      acc[i] = NDigit;
      }
    return pResult;
    }
  for( int j = -2; j <= N; j++ )
    h[j] = 0;
  GetCoeff( ex, 1 );
  s_Multiplier = h[0];
  for( int i = -2; i <= -1; i++ )
    {
    b[i] = 0;
    c[i] = 0;
    d[i] = 0;
    e[i] = 0;
    }
  t = 1;
  iOut = 0;
  while( h[N] == 0 && N > 0 )
    {
    iOut++;
    pResult->Addexp( new TBinar( '=', new TIndx( Variable( Unknown ), Constant( iOut ) ), Constant( 0 ) ) );
    acc[iOut] = NDigit;
    N--;
    }
  while( N > 0 )
    {
    ps = 0; qs = 0; pt = 0; qt = 0; s = 0;
    rev = 1; kk = std::pow( 10, NDigit );
    if( N == 1 )
      {
      r = -h[1] / h[0];
      goto lin;
      }
    for( int j = 0; j <= N; j++ )
      if( h[j] != 0 ) s += log( abs( h[j] ) );
    s = exp( s / ( N + 1 ) );
    for( int j = 0; j <= N; j++ ) h[j] /= s;
    if( abs( h[N - 1] / h[N] ) > abs( h[1] / h[0] ) )
    revers:
      {
      t = -t;
      int m = ( N - 1 ) / 2;
      for( int j = 0; j <= m; j++ )
        {
        s = h[j];
        h[j] = h[N - j];
        h[N - j] = s;
        }
      }
      if( qs != 0 )
        {
        p = ps;
        q = qs;
        goto iter;
        }
      if( h[N - 2] == 0 )
        {
        q = 1;
        p = -2;
        }
      else
        {
        q = h[N] / h[N - 2];
        p = ( h[N - 1] - q*h[N - 3] ) / h[N - 2];
        }
      if( N == 2 ) goto quadr;
      r = 0;
    iter:
      for( int i = 1; i <= ItMax; i++ )
        {
      bairstow:
        for( int j = 0; j <= N; j++ )
          {
          b[j] = h[j] - p*b[j - 1] - q*b[j - 2];
          c[j] = b[j] - p*c[j - 1] - q*c[j - 2];
          }
        if( h[N - 1] == 0 || b[N - 1] == 0 ) goto testbn;
        if( abs( h[N - 1] / b[N - 1] ) < kk ) goto newton;
        b[N] = h[N] - q * b[N - 2];
      testbn:
        if( b[N] == 0 ) goto quadr;
        if( abs( h[N] / b[N] ) > kk ) goto quadr;
      newton:
        for( int j = 0; j <= N; j++ )
          {
          d[j] = h[j] + r * d[j - 1];
          e[j] = d[j] + r * e[j - 1];
          }
        if( d[N] == 0 ) goto lin;
        if( abs( h[N] / d[N] ) > kk ) goto lin;
        c[N - 1] = -p*c[N - 2] - q * c[N - 3];
        s = std::pow( c[N - 2], 2 ) - c[N - 1] * c[N - 3];
        if( s == 0 )
          {
          p = p - 2;
          q = q * ( q + 1 );
          }
        else
          {
          p = p + ( b[N - 1] * c[N - 2] - b[N] * c[N - 3] ) / s;
          q = q + ( -b[N - 1] * c[N - 1] + b[N] * c[N - 2] ) / s;
          }
        if( e[N - 1] == 0 )
          r = r - 1;
        else
          r = r - d[N] / e[N - 1];
        }
      if( ps == p && qs == q )
        {
        GetRoots();
        exTmp = ReduceTExprs( ex.Substitute( Unknown, Root1 ) );
        s_Precision = 1e-4;
        if( exTmp == 0 ) goto QuadrOut;
        pResult->Clear();
        return pResult;
        }
      ps = pt;
      qs = qt;
      pt = p;
      qt = q;
      if( rev < 0 ) kk = kk / 10;
      rev = -rev;
      goto revers;
    lin:
      if( t < 0 ) r = 1 / r;
      iOut++;
      pResult->Addexp( new TBinar( '=', new TIndx( Variable( Unknown ), Constant( iOut ) ), Constant( r ) ) );
      acc[iOut] = kk;
      N--;
      if( N == 0 ) break;
      for( int j = 0; j <= N; j++ )
        if( abs( h[j] / d[j] ) < kk )
          h[j] = d[j];
        else
          h[j] = 0;
      goto iter;
    quadr:
      GetRoots;
    QuadrOut:
      exTmp.Clear();
      iOut++;
      pResult->Addexp( new TBinar( '=', new TIndx( Variable( Unknown ), Constant( iOut ) ), Root1 ) );
      iOut++;
      pResult->Addexp( new TBinar( '=', new TIndx( Variable( Unknown ), Constant( iOut ) ), Root2 ) );
      acc[iOut] = kk;
      acc[iOut - 1] = kk;
      N -= 2;
      if( N == 0 ) break;
      for( int j = 0; j <= N; j++ )
        if( abs( h[j] / b[j] ) < kk )
          h[j] = b[j];
        else
          h[j] = 0;
    }
    return pResult;
  }

MathExpr GetFactorEquation( const QByteArray& VarName )
  {
  MathExpr Result, Root;
  s_Factor++;
  s_FactorizedSolving = s_Factor < s_FactorArray.count();
  if( !s_FactorizedSolving ) return Result;
  QByteArray Name;
  if( s_FactorArray[s_Factor].Variab( Name ) && Name == VarName )
    Root = Constant( 0 );
  else
    {
    Result = new TBinar( '=', s_FactorArray[s_Factor], Constant( 0 ) );
    bool OldCalcOnly = s_CalcOnly;
    s_CalcOnly = true;
    bool OldPutAnswer = s_PutAnswer;
    s_PutAnswer = false;
    MathExpr MLR = CalcDetLinEqu( Result.WriteE(), VarName );
    if( MLR.IsEmpty() ) return Result;
    TL2exp& LinRoot = *CastPtr( TL2exp, MLR );
    s_CalcOnly = OldCalcOnly;
    s_PutAnswer = OldPutAnswer;
    if( LinRoot.Count() == 0 ) return Result;
    Root = LinRoot.Last()->m_Memb;
    TSolutionChain::sm_SolutionChain.AddExpr( Result );
    }
  return GetPutRoot( Root, VarName );
  }

bool IsMultiNominal( const MathExpr& exi )
  {
  TMultiNominal multinominal( exi );
  return multinominal.m_IsCorrect;
  }

TMatrix2::TMatrix2( int Size )
  {
  InitMatrix( Size );
  }

TMatrix2::TMatrix2( TMatrix2& AMatrix2 ) : TMatrix2( AMatrix2.count() )
  {
  m_Status = AMatrix2.m_Status;
  for( int I = 0; I < count(); I++ )
    {
    Variable(I) = AMatrix2.Variable(I);
    m_RowsOrder[I] = AMatrix2.m_RowsOrder[I];
    for( int J = 0; J <= count(); J++ )
      ( *this )( I, J ) = AMatrix2( I, J );
    }
  }

void TMatrix2::InitMatrix( int ASize )
  {
  m_Status = liOK;
  clear();
  resize( ASize );
  m_Variables.clear();
  int iRow = 0;
  m_RowsOrder.resize( ASize );
  for( auto pRow = begin(); pRow != end(); pRow++, iRow++ )
    {
    m_Variables.append( "" );
    m_RowsOrder[iRow] = iRow;
    for( int J = 0; J <= ASize; J++ )
      pRow->append( Constant( 0 ) );
    }
  }

MathExpr TMatrix2::Det()
  {
  if( count() == 1 ) return ( *this )( 0, 1 );
  MathExpr  Result = Constant( 0 );
  for( int I = 1; I <= count(); I++ )
    {
    TMatrix2 Matrix2( count() - 1 );
    for( int iRow = 1; iRow < count(); iRow++ )
      for( int iCol = 1; iCol < count(); iCol++ )
        {
        int iColNew = iCol;
        if( iCol >= I ) iColNew++;
        Matrix2( iRow - 1, iCol ) = ( *this )( iRow, iColNew );
        }
    MathExpr Item1I = ( *this )( 0, I );
    MathExpr Det1I = Matrix2.Det();
    if( ( I + 1 ) % 2 == 0 )
      Result += MultExprs( Item1I, Det1I );
    else
      Result -= MultExprs( Item1I, Det1I );
    }
  return ExpandExpr( Result );
  }

MathExpr TMatrix2::DetI( int I )
  {
  TMatrix2 Matrix2I(*this);
  Matrix2I.InsertFreeToI( I );
  return Matrix2I.Det();
  }

void TMatrix2::InsertFreeToI( int No )
  {
  for( int I = 0; I < count(); I++ ) 
    ( *this )(I, No) = FreeTerms(I);
  }
/*
int TMatrix2::IndexOfName( const QByteArray& VarName )
  {
  for( int I = 0; I < VariableCount(); I++ )
    if( Variable( I ) == VarName ) return I;
  return -1;
  }
  */

int TMatrix2::IndexOfName( const QByteArray& VarName )
  {
  int IndexOfFreePlace = -1;
  for( int I = 0; I < count(); I++ )
    {
    if( Variable( I ) == VarName ) return I;
    if( IndexOfFreePlace == -1 && Variable( I ).isEmpty() ) IndexOfFreePlace = I;
    }
  if( IndexOfFreePlace == -1 )
    {
    m_Variables.append( VarName );
    for( int I = 0; I < count(); I++ )
      ( *this )[I].append( Constant( 0 ) );
    return m_Variables.count() - 1;
    }
  Variable( IndexOfFreePlace ) = VarName;
  return IndexOfFreePlace;
  }

void TMatrix2::CountOfDet()
  {
  m_CountedDet = Det();
  }

MathExpr& TMatrix2::operator ()( int iRow, int iCol )
  {
  return ( *this )[m_RowsOrder[iRow]][iCol+1];
  }

MathExpr& TMatrix2::FreeTerms( int I )
  {
  return ( *this )( I, -1 );
  }

MathExpr CreateVar( const QByteArray& Name, int index )
  {
  if( Name.isEmpty() )
    return new TIndx( Variable( "x" ), Constant( index ) );
  if( Name.indexOf( '[' ) == -1 )
    return Variable( Name );
  return Parser().StrToExpr( Name );
  }

MathExpr TMatrix2::GetEquation( int I )
  {
  if( isEmpty() || I >= count() ) return MathExpr();
  MathExpr Result = ( *this )( I, 0 ) * CreateVar( Variable( 0 ), 1 );
  int N, D;
  for( int iItem = 1; iItem < VariableCount(); iItem++ )
    if( ( *this )( I, iItem ).SimpleFrac_( N, D ) && ( N < 0 || D < 0 ) )
      Result -= MathExpr( new TSimpleFrac( abs( N ), abs( D ) ) ) * CreateVar( Variable( iItem ), iItem );
    else
      Result += ( *this )( I, iItem ) * CreateVar( Variable( iItem ), iItem );
  s_CanExchange = false;
  Result = new TBinar( '=', Result.Reduce(), FreeTerms( I ) );
  s_CanExchange = true;
  return Result;
  }

void TMatrix2::PutToWindow()
  {
  if( isEmpty() ) return;
  Lexp SysEq = new TL2exp;
  bool bSolved = true;
  for( int iRow = 0; iRow < count(); iRow++ )
    {
    MathExpr exEquation = GetEquation( iRow );
    if( bSolved )
      {
      MathExpr exLeft, exRight;
      exEquation.Binar( '=', exLeft, exRight );
      QByteArray sVarName;
      bSolved = exLeft.Variab( sVarName ) && IndexOfName( sVarName ) != -1;
      double V;
      int t;
      if( bSolved )
        bSolved = exRight.Constan( V ) || exRight.SimpleFrac_( t, t ) ||
        exRight.Variab( sVarName ) && IndexOfName( sVarName ) == -1;
      }
    SysEq.Addexp( exEquation );
    SysEq.Last()->m_Visi = false;
    }
  MathExpr exSystem = new TSyst( SysEq );
  if( m_OldStep.IsEmpty() || !exSystem.Equal( m_OldStep ) )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( exSystem );
    m_OldStep = exSystem;
    }
  if( bSolved )
    {
    m_Status = liSystemSolved;
    if( s_PutAnswer && s_Answer.IsEmpty() )
      {
      s_Answer = new TL2exp;
      for( int iRow = 0; iRow < count(); iRow++ )
        CastPtr( TL2exp, s_Answer )->Addexp( GetEquation( iRow ) );
      }
    }
  }

void TMatrix2::ChangeOrder( int I, int J )
  {
  int V = m_RowsOrder[I];
  m_RowsOrder[I] = m_RowsOrder[J];
  m_RowsOrder[J] = V;
  }

void TMatrix2::Sort()
  {
  for( int k = 0; k < count(); k++ )
    for( int j = 1; j < count() - k; j++ )
      if( Variable( j - 1 ) > Variable( j ) )
        {
        QByteArray sTmp = Variable( j - 1 );
        Variable( j - 1 ) = Variable( j );
        Variable( j ) = sTmp;
        for( int i = 0; i < count(); i++ )
          {
          MathExpr exTmp = ( *this )( i, j );
          ( *this )( i, j ) = ( *this )( i, j + 1 );
          ( *this )( i, j + 1 ) = exTmp;
          }
        }
  }

TLinear::TLinear( const MathExpArray& List ) : m_Status( liEmpty ), m_InputList( List )
  {
  if( m_InputList.count() == 0 )
    m_Status = liNotLinear;
  else
    ResetMatrix();
  }

void TLinear::ResetMatrix()
  {
  m_Matrix2.InitMatrix( m_InputList.count() );
  QVector<TMultiNominal> MultiList;
  bool bResult = true;
  m_Status = liOK;
  for( int I = 0; I < m_InputList.count(); I++ )
    MultiList.append( TMultiNominal( m_InputList[I] + Constant( 0  ) ) );
  for( int I = 0; I < MultiList.count(); I++ )
    bResult = bResult && MultiList[I].IsLinear();
  if( bResult )
    {
    for( int I = 0; I < MultiList.count(); I++ ) 
      {
      TMultiNominal &M = MultiList[I];
      MathExpr Oper1, Oper2;
      for( int J = 0; J < M.count(); J++ )
        {
        TTerm &T = M[J];
        MathExpr CoeffExpr = T.CoeffAndSign();
        MathExpr ParamExpr = T.Params();
        MathExpr Aij = MultExprs( CoeffExpr, ParamExpr );
        QByteArray VarName = T.VarName();
        if( VarName == "0" )
          {
          Oper1 = m_Matrix2.FreeTerms(I);
          Oper2 = Aij.RetNeg();
          m_Matrix2.FreeTerms(I) = ExpandExpr( Oper1 + Oper2 );
          }
        else 
          {
          int IndexName = m_Matrix2.IndexOfName( VarName );
          bResult = bResult && IndexName != -1;
          if( bResult )
            {
            Oper1 = m_Matrix2(I, IndexName);
            Oper2 = Aij;
            m_Matrix2(I, IndexName) = ExpandExpr( Oper1 + Oper2 );
            }
          }
        }
      }
    if( bResult )
      if( MultiList.count() < MultiList[0].count() - 1 )
        m_Status = liInfinityDecision;
      else
        m_Status = liOK;
    else
    m_Status = liNot1Decision;
    }
  else 
    m_Status = liNotLinear;
  if( m_Status == liNot1Decision || m_Status == liNotLinear ) return;
  m_Matrix2.Sort();
  m_Solution.resize( m_Matrix2.VariableCount() );
  for( int I = 0; I < m_Matrix2.VariableCount(); I++ )
    {
    if( m_Matrix2.Variable( I ).isEmpty() )
      m_Status = liWasEmptyName;
    else
    if( m_Status == liWasEmptyName )
      m_Status = liSystemOverdetermined;
    m_Solution[I] = CreateVar( m_Matrix2.Variable(I), I + 1 );
    }
  }

MathExpr TLinear::GetSolution( int i )
  {
  MathExpr Result = new TBinar( '=', CreateVar( m_Matrix2.Variable( i ), i + 1 ), m_Solution[i] );
  MathExpr ex = Parser().StrToExpr( m_Matrix2.Variable( i ) );
  if( IsConstType( TVariable, ex ) ) return Result;
  QByteArray VarName = ex.HasUnknown();
  if( VarName.isEmpty() ) return Result;
  MathExpr ex1 = new TBinar( '=', ex.Clone(), m_Solution[i] );
  if( !GetAnswer( ex1, VarName ) ) return Result;
  m_Status = liEquationSolved;
  m_Matrix2.Variable( i ) = VarName;
  m_Solution[i] = s_Answer;
  return Result;
  }

bool TLinear::Run()
  {
  const double Zero = 1e-5;
  if( m_Status == liInfinityDecision || m_Status == liWasEmptyName ) m_Status = liOK;
  if( m_Status != liOK ) return false;
  m_Homogeneous = true;
  for( int I = 0; I < m_Matrix2.count(); I++ )
    {
    int V;
    m_Homogeneous = !m_Matrix2.FreeTerms(I).Cons_int( V ) || V == 0;
    if( !m_Homogeneous ) break;
    }
  if( s_DetailDegree > 0 )
    {
    m_Matrix2.PutToWindow();
    if( m_Matrix2.Status() == liSystemSolved ) return true;
    }

  double PrecOld = s_Precision;
  s_Precision = 1e-14;
  int K = -1;
  double Val;
  int EquationCount;
  for( int K1 = 0; K1 < m_Matrix2.count(); K1++)
    {
    EquationCount = K1 + 1;
    bool AllWasZero;
    do
      {
      K++;
      AllWasZero = false;
      if( K == m_Matrix2.count() || m_Matrix2( K1, K ).Constan( Val ) && abs( Val ) < Zero )
        {
        AllWasZero = true;
        for( int J = K + 1; J < m_Matrix2.VariableCount(); J++ )
          if( !m_Matrix2( K1, J ).Constan( Val ) || abs( Val ) > Zero )
            {
            AllWasZero = false;
            break;
            }
        if( AllWasZero )
          {
          if( m_Matrix2.FreeTerms(K1).Constan( Val ) && abs( Val ) < Zero )
            {
            m_Status = liInfinityDecision;
            EquationCount = K1;
            break;
            }
          else
            m_Status = liNot1Decision;
          s_Precision = PrecOld;
          return false;
          }

        AllWasZero = true;
        for( int I = K1 + 1; I < m_Matrix2.count(); I++ )
          if( !m_Matrix2( I, K ).Constan( Val ) || abs( Val ) > Zero )
            {
            m_Matrix2.ChangeOrder( K1, I );
            AllWasZero = false;
            break;
            }
        }
      } while( AllWasZero );
      if( K1 == m_Matrix2.count() - 1 || m_Status == liInfinityDecision ) break;
      MathExpr exMul1 = m_Matrix2( K1, K );
      for( int I = K1 + 1; I < m_Matrix2.count(); I++ )
        {
        if( m_Matrix2( I, K ).Constan( Val ) && abs( Val ) < Zero )
          continue;
        MathExpr exMul2 = m_Matrix2( I, K );
        for( int J = K; J < m_Matrix2.VariableCount(); J++ )
          m_Matrix2( I, J ) = ( exMul1 * m_Matrix2( I, J ) - exMul2 * m_Matrix2( K1, J ) ).Reduce();
        m_Matrix2.FreeTerms(I) = ( exMul1 * m_Matrix2.FreeTerms(I) - exMul2 * m_Matrix2.FreeTerms(K1) ).Reduce();
        }

      if( s_DetailDegree > 0 )
        {
        m_Matrix2.PutToWindow();
        if( m_Matrix2.Status() == liSystemSolved )
          {
          s_Precision = PrecOld;
          return true;
          }
        }
    }
  for( int I = EquationCount - 1; I >= 0; I-- )
    {
    int J = 0;
    for( int K = 0; K < m_Matrix2.VariableCount(); K++ )
      {
      J++;
      if( !m_Matrix2( I, K ).Constan( Val ) || abs( Val ) > Zero ) break;
      }
    MathExpr exTmp = m_Matrix2.FreeTerms(I);
    for( int K = J; K < m_Matrix2.VariableCount(); K++ )
      if( !m_Matrix2( I, K ).Constan( Val ) || abs( Val ) > Zero )
        exTmp -= m_Matrix2( I, K ) * m_Solution[K];
    m_Solution[J - 1] = ( exTmp / m_Matrix2( I, J - 1 ) ).Reduce();
    }

  int iDim = 0;
  for( int K = 0; K < m_Solution.count(); K++ )
    {
    QByteArray VarName = m_Matrix2.Variable( K );
    if( VarName.isEmpty() ) VarName = CreateVar( m_Matrix2.Variable( K ), K + 1 ).WriteE();
    if( m_Solution[K].WriteE() == VarName )
      {
      m_Solution[K] = new TStr( "any number" );
      iDim++;
      }
    else
      m_Solution[K] = ExpandExpr( m_Solution[K] );
    }

  Lexp ResultOut = new TL2exp;
  for( int I = 0; I < m_Solution.count(); I++ )
    {
    ResultOut.Addexp( GetSolution( I ) );
    ResultOut.Last()->m_Visi = false;
    }
  TSolutionChain::sm_SolutionChain.AddExpr( new TSyst( ResultOut ), "", true );

  if( m_Status == liEquationSolved )
    {
    ResultOut = new TL2exp;
    for( int I = 0; I < m_Solution.count(); I++ )
      {
      ResultOut.Addexp( GetSolution( I ) );
      ResultOut.Last()->m_Visi = false;
      }
    TSolutionChain::sm_SolutionChain.AddExpr( new TSyst( ResultOut ) );
    m_Status = liOK;
    }

  if( s_PutAnswer && s_Answer.IsEmpty() )
    {
    s_Answer = new TL2exp;
    for( int I = 1; I < m_Solution.count(); I++ )
      CastPtr( TL2exp, s_Answer )->Addexp( GetSolution( I ) );
    }

  if( m_Status == liOK && m_Matrix2.VariableCount() > m_Matrix2.count() )
    m_Status = liInfinityDecision;

  if( !m_Homogeneous || iDim == 0 )
    {
    s_Precision = PrecOld;
    return true;
    }
  TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', new TStr( "Dim W" ), Constant( iDim ) ) );
  QVector<int> Undefined( iDim );
  K = 0;
  for( int I = 0; I < m_Solution.count(); I++ )
    if( IsConstType( TStr, m_Solution[I] ) )
      {
      Undefined[K] = I;
      m_Solution[I] = Constant( 0 );
      K++;
      }
  for( int I = 0; I < iDim; I++ )
    {
    Lexp BasisRow = new TLexp;
    Lexp BasisEl = new TLexp;
    for( int J = 0; J < iDim; J++ )
      {
      if( I == J )
        m_Solution[Undefined[J]] = 1;
      else
        m_Solution[Undefined[J]] = 0;
      BasisEl.Addexp( new TBinar( '=', CreateVar( m_Matrix2.Variable( Undefined[J] ), Undefined[J] + 1 ), m_Solution[Undefined[J]] ) );
      }
    BasisRow.Addexp( BasisEl );
    BasisEl = new TLexp;
    for( int J = 0; J < m_Solution.count(); J++ )
      if( IsConstType( TConstant, m_Solution[J] ) )
        BasisEl.Addexp( m_Solution[J] );
      else
        {
        MathExpr exFree = m_Solution[J];
        for( int K = 0; K < iDim; K++ )
          exFree = exFree.Substitute( m_Matrix2.Variable( Undefined[K] ), m_Solution[Undefined[K]] );
        BasisEl.Addexp( exFree.Reduce() );
        }
    BasisRow.Addexp( BasisEl );
    TSolutionChain::sm_SolutionChain.AddExpr( BasisRow );
    }
  s_Precision = PrecOld;
  return true;
  }

MathExpr TLinear::GetRoot( int I )
  {
  return m_Matrix2.DetI( I ) / m_Matrix2.CountedDet();
  }

/*
      UAnaliza.Multiplier = 1;
      res = CalcAnyEquation( Result.WriteE + "==0", nullptr );
      if( ( res != nullptr ) && res.Listex( mem ) && ( mem != nullptr ) && mem->m_Memb.Binar( '=', op1, op2 ) )
        {
        R = ( ( Variable( ( VarName ) ) ) - ( op2 ) ).Reduce();
        mem = mem.Next;
        iCount = 1;
        while( mem != nullptr )
          {
          mem->m_Memb.Binar( '=', op1, op2 );
          R = ( ( R ) * ( ( ( Variable( ( VarName ) ) ) - ( op2 ) ).Reduce() ) );
          mem = mem.Next;
          iCount = iCount + 1;
          };
        if( iCount > 2 )
          {
          Result = R;
          if( abs( UAnaliza.Multiplier - 1 ) > 0.0001 )
            Result = ( ( Constant( ( UAnaliza.Multiplier ) ) ) * ( Result ) );
          };
        };
      };
    };
*/
