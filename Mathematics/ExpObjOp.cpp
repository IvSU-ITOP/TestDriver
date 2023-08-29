#include "ExpObjOp.h"
#include "Factors.h"
#include "ExpStore.h"
#include "Algebra.h"
#include "MathTool.h"
#include "ResultReceiver.h"
#include "SolChain.h"
#include <functional>

extern ExpStore s_ExpStore;
bool TMult::sm_ShowUnarMinus = true;
bool TMult::sm_ShowMultSign = false;
bool TSumm::sm_ShowMinusByAddition = false;

bool s_ShowDiviMessages = false;

bool ConstExpression( const MathExpr& E )
  {
  double Value;
  int I, N, D;
  MathExpr Op1, Op2;
  return E.Constan( Value )	|| E.SimpleFrac_( N, D )	|| E.MixedFrac_( I, N, D )	|| 
    E.Divis( Op1, Op2 )	&& ConstExpression( Op1 ) && ConstExpression( Op2 );
  }

bool NegatConst( const MathExpr& E )
  {
  double Var, Value;
  int I, N, D;
  MathExpr Op1, Op2;
  return E.Constan( Value ) && ( Value < 0 ) || E.SimpleFrac_( N, D ) && ( ( N < 0 || D < 0 ) && !( N < 0 && D > 0 ) ) ||
    E.MixedFrac_( I, N, D ) && I < 0 || ( E.Divis( Op1, Op2 ) && NegatConst( Op1 ) ^ NegatConst( Op2 ) );
  }

TSumm::TSumm( const MathExpr& ex1, const MathExpr& ex2 ) :TOper( ex1, ex2, '+' )
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TSumm::Clone() const
  {
  return new TSumm( m_Operand1.Clone(), m_Operand2.Clone() );
  }

MathExpr TSumm::Reduce() const
  {
  double Value1, Value2, Value3;
  MathExpr P, Op11, Op12, Op21, Op22, opr1, opr2, opr1r, opr2r;
  MathExpr result;
  int N1, N2, NR, D1, D2, DR, Int;
  PExMemb ind;
  QByteArray VarName;

  opr1r = m_Operand1.Reduce();
  if( IsConstType( TIntegral, m_Operand1 ) )
    opr1r = opr1r.Calculate();
  opr2r = m_Operand2.Reduce();
  if( IsConstType( TIntegral, m_Operand2 ) )
    opr2r = opr2r.Calculate();

  if( s_IsLogEquation )
    {
    if( ( IsConstType( TConstant, opr1r ) || IsConstType( TSimpleFrac, opr1r ) ) && opr2r.Log( Op21, Op22 ) && opr2r.ConstExpr() )
      {
      P = ( Op21 ^ opr1r ) * Op22;
      return Op21.Log( P.Reduce() );
      }
    if( ( IsConstType( TConstant, opr2r ) || IsConstType( TSimpleFrac, opr2r ) ) && opr2r.Log( Op11, Op12 ) && opr1r.ConstExpr() )
      {
      P = ( Op11 ^ opr2r ) * Op12;
      return Op11.Log( P.Reduce() );
      }
    }

  if( opr1r.Variab( VarName ) && VarName == "i" )
    return new TComplexExpr( opr2, new TConstant( 2 ) );

  if( opr2r.Variab( VarName ) && VarName == "i" )
    return new TComplexExpr( opr1, new TConstant( 2 ) );

  if( IsConstType( TMatr, opr1r ) )
    return CastPtr( TMatr, opr1r )->Add( opr2r );

  opr1 = opr1r.SortMult();
  opr2 = opr2r.SortMult();

  if( opr1.Complex( Op11, Op12 ) && opr2.Complex( Op21, Op22 ) )
    return TComplexExpr( Op11 + Op21, Op12 + Op22 ).Reduce();

  if( opr1.Complex( Op11, Op12 ) && !opr2.Complex( Op21, Op22 ) )
    return TComplexExpr( Op11 + opr2, Op12 ).Reduce();

  if( !opr1.Complex( Op11, Op12 ) && opr2.Complex( Op21, Op22 ) )
    return TComplexExpr( opr1 + Op21, Op22 ).Reduce();

  if( opr1.Measur_( Op11, Op12 ) && opr2.Measur_( Op21, Op22 ) )
    {
    opr1r = Op12 / Op22;
    ReduceMeasure( opr1r );
    if( opr1r.Constan( Value3 ) && Value3 == 1 )
      return new TMeaExpr( ( Op11 + Op21 ).Reduce(), Op22 );
    else
      {
      s_LastError = "Inadmissible operation.";
      throw "Inadmissible operation.";
      }
    }

  if( opr1.Listex( ind ) || opr2.Listex( ind ) )
    return opr1.Lconcat( opr2 );

  if( opr1.Constan( Value1 ) && opr2.Constan( Value2 ) )
    {
    if( opr1.Measur_( opr1r, opr2r ) )
      return new TMeaExpr( new TConstant( Value1 + Value2 ), new TVariable( false, msDegree ) );
    else
      return new TConstant( Value1 + Value2 );
    }

  if( opr1.Subtr( Op11, Op12 ) && Op12.Equal( opr2 ) )
    return Op11.Reduce();

  if( opr2.Subtr( Op21, Op22 ) && Op22.Equal( opr1 ) )
    return Op21.Reduce();

  if( opr2.Unapm_( Op21 ) )
    {
    P = new TBinar( msPlusMinus, opr1, Op21 );
    return P.Reduce();
    }

  if( ( opr1.Constan( Value1 ) && Value1 == 0 ) || opr1.Newline() )
    return opr2;

  if( ( opr2.Constan( Value2 ) && Value2 == 0 ) || opr2.Newline() )
    return opr1;

  if( opr2.Constan( Value2 ) && Value2 < 0 )
    return ( opr1 - ( -Value2 ) ).Reduce();

  if( opr1.Eq( opr2 ) )
    return ( Constant( 2 ) * opr1 ).Reduce();

  if( opr2.Unarminus( Op21 ) )
    return ( opr1 - Op21 ).Reduce();

  if( opr1.Unarminus( Op11 ) && s_CanExchange )
    return ( opr2 - Op11 ).Reduce();

  if( opr1.Subtr( Op11, Op12 ) && opr2.Summa( Op21, Op22 ) )
    {
    if( Op12.Equal( Op22 ) )
      return ( Op11.Reduce() + Op21.Reduce() ).Reduce();

    if( Op12.Equal( Op21 ) )
      return ( Op11.Reduce() + Op22.Reduce() ).Reduce();
    }

  if( opr1.Summa( Op11, Op12 ) && opr2.Subtr( Op21, Op22 ) )
    {
    if( Op12.Equal( Op22 ) )
      return( Op11.Reduce() + Op21.Reduce() ).Reduce();

    if( Op11.Equal( Op22 ) )
      return( Op12.Reduce() + Op21.Reduce() ).Reduce();
    }

  if( opr1.Multp( Op11, Op12 ) && opr2.Multp( Op21, Op22 ) )
    {
    if( Op11.Eq( Op21 ) && Op12.ConstExpr() && Op22.ConstExpr() && !ConstExpression( Op21 ) )
      return ( ( Op12 + Op22 ) * Op11 ).Reduce();

    if( Op11.Eq( Op22 ) && Op12.ConstExpr() && Op21.ConstExpr() && !ConstExpression( Op22 ) )
      return ( ( Op11 + Op21 ) * Op12 ).Reduce();

    if( Op12.Eq( Op22 ) && Op11.ConstExpr() && Op21.ConstExpr() && !ConstExpression( Op22 ) )
      return ( ( Op11 + Op21 ) * Op12 ).Reduce();

    if( Op12.Eq( Op21 ) && Op11.ConstExpr() && Op22.ConstExpr() && !ConstExpression( Op21 ) )
      return ( ( Op11 + Op22 ) * Op12 ).Reduce();
    }

  if( opr1.Multp( Op11, Op12 ) )
    {
    if( Op12.Eq( opr2 ) && Op11.ConstExpr() && !ConstExpression( Op12 ) )
      return ( ( Op11 + 1) * opr2).Reduce();

    if( Op11.Eq( opr2 ) && Op12.ConstExpr() && !ConstExpression( Op11 ) )
      return ( ( Op12 + 1 ) * opr2 ).Reduce();
    }

  if( opr2.Multp( Op21, Op22 ) )
    {
    if( Op22.Eq( opr1 ) && Op21.ConstExpr() && !ConstExpression( Op22 ) )
      return ( ( Op21 + 1 ) * opr1 ).Reduce();

    if( Op21.Eq( opr1 ) && Op22.ConstExpr() && !ConstExpression( Op21 ) )
      return ( ( Op22 + 1 ) * opr1 ).Reduce();
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.SimpleFrac_( N2, D2 ) )
    {
    SumFrac( N1, D1, N2, D2, NR, DR );
    if( s_GlobalInvalid )
      return Ethis;
    return  GenerateFraction( NR, DR ).Reduce();
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Cons_int( Int ) || opr2.SimpleFrac_( N1, D1 ) && opr1.Cons_int( Int ) )
    return GenerateFraction( N1 + Int * D1, D1 ).Reduce();

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Constan( Value2 ) || opr2.SimpleFrac_( N1, D1 ) && opr1.Constan( Value2 ) )
    return new TConstant( Value2 + ( double ) N1 / D1 );

  if( opr1.SimpleFrac_( N2, D2 ) && N2 * D2 < 0 )
    {
    P = new TSubt( opr1, new TSimpleFrac( abs( N2 ), abs( D2 ) ) );
    return P.Reduce();
    }
  if( opr1.SimpleFrac_( N1, D1 ) && N1 * D1 < 0 )
    {
    P = new TSubt( opr2, new TSimpleFrac( abs( N1 ), abs( D1 ) ) );
    return P.Reduce();
    }

  if( ( opr1.Divis( Op11, Op12 ) && opr2.Constan( Value3 ) && Op11.Constan( Value1 ) && Op12.Constan( Value2 ) ) ||
    ( opr2.Divis( Op11, Op12 ) && opr1.Constan( Value3 ) && Op11.Constan( Value1 ) && Op12.Constan( Value2 ) ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "XPCalcMess", "Too large" ) );
    if(Value2 == sm_Precision ) return Ethis;
    return new TConstant( Value3 + Value1 / Value2 );
    }

  return ( opr1 + opr2 ).SimplBySummList();
  }

MathExpr TSumm::Perform()
  {
  return m_Operand1.Perform() + m_Operand2.Perform();
  }

MathExpr TSumm::Diff( const QByteArray& d )
  {
  return new TSumm( m_Operand1.Diff( d ), m_Operand2.Diff( d ) );
  }

MathExpr TSumm::Integral( const QByteArray& d )
  {
  return new TSumm( m_Operand1.Integral( d ), m_Operand2.Integral( d ) );
  }

MathExpr TSumm::Lim( const QByteArray& v, const MathExpr& lm ) const
  {
  MathExpr op1, op2;
  bool ng1, ng2;

  if( IsConstType( TDivi, m_Operand1 ) && IsConstType( TDivi, m_Operand2 ) )
    {
//    sm_pResultReceiver->Clear();
    op1 = TExpr::CalcMulti( 1, MathExpr(this->Clone()), false );
    if( !op1.IsEmpty() && !( IsConstType ( TSumm, m_Operand1  ) ) ) return op1->Lim( v, lm );
    }

  op1 = m_Operand1->Lim( v, lm );
  op2 = m_Operand2->Lim( v, lm );
  if( !op1.IsEmpty() && op1.Infinity( ng1 ) )
    if( !op2.IsEmpty() && op2.Infinity( ng2 ) )
      if( ng1 == ng2 )
        return new  TInfinity( ng1 );
      else
        {
        s_GlobalInvalid = true;
        return nullptr;
        }
    else
      return op1;
  else
    if( !op2.IsEmpty() && op2.Infinity( ng2 ) )
      return op2;
    else
      if( op1.IsEmpty() || op2.IsEmpty() )
        return nullptr;
      else
        return new TSumm( op1, op2 );
  return nullptr;
  }

bool TSumm::Eq( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;
  if( E2.Summa( op21, op22 ) )
    return m_Operand1.Eq( op21 ) && m_Operand2.Eq( op22 ) || m_Operand1.Eq( op22 ) && m_Operand2.Eq( op21 );
  return false;
  }

bool TSumm::Equal( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;
  if( E2.Summa( op21, op22 ) && (m_Operand1.Equal( op21 ) && m_Operand2.Equal( op22 ) || m_Operand1.Equal( op22 ) && m_Operand2.Equal( op21 ) ) )
      return true;

  if( E2.Summa( op21, op22 ) || E2.Subtr( op21, op22 ) || E2.Unarminus( op21 ) || E2.Binar( msPlusMinus, op21, op22 ) || IsConstType(TComplexExpr, E2) )
    {
    MathExpr A1( SummList() ), A2( E2.SummList() );
    if( A1.Equal( A2 ) )
      return true;
    if( CastPtr( TLexp, A1 )->Count() == CastPtr( TLexp, A2 )->Count() ) return false;
    MathExpr R( Reduce() );
    MathExpr RE( E2.Reduce() );
    if( Eq( R ) )
      if( RE.Eq( E2 ) )
        return false;
      else
        return RE.Equal( Ethis );
    return R.Equal( RE );
    }

  QByteArray _eqName;
  MathExpr Ar2;
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );

  return false;
  }

MathExpr TSumm::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TSumm( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

bool TSumm::Summa( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TSumm::IsLinear() const
  {
  return m_Operand1.IsLinear() && m_Operand2.IsLinear();
  }

bool  TSumm::Negative() const
  {
  return m_Operand1.Negative() && m_Operand2.Negative();
  }

bool TSumm::Positive() const
  {
  return m_Operand1.Positive() && m_Operand2.Positive();
  }

MathExpr TSumm::TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower )
  {
  QByteArray Name;
  MathExpr Arg;
  if( sName != "cos" ) return nullptr;
  if( exPower.IsEmpty() || exPower != 2 || ( m_Operand1 != 1 && m_Operand2 != 1 ) ||
    (!m_Operand1.Funct( Name, Arg ) && !m_Operand2.Funct( Name, Arg )) || ( Name != "cos" )) return nullptr;
  MathExpr ex1 = new TDivi( Arg, exArg );
  MathExpr ex2 = ex1.Reduce();
  if( ex2 == 2  )
     return new TConstant( 2 );
  return nullptr;
  }

QByteArray TSumm::SWrite() const
  {
  MathExpr Op1, Op2;
  double Val;
  QByteArray Left = m_Operand1.SWrite();
  if (m_Operand2.Negative() && !sm_ShowMinusByAddition)
    {
    if (m_Operand2.Unarminus(Op1))
      return Left + "\\longminus\n" + Op1.SWrite();
    if (m_Operand2.Constan(Val) && Val < 0 )
      return Left + "\\longminus\n" + TConstant(qAbs(Val)).SWrite();
    if (m_Operand2.Multp(Op1, Op2) && Op1.Constan(Val) && Val < 0)
      return Left + "\\longminus\n" + (Constant(-Val) * Op2).SWrite();
    }
  QByteArray Name("\\longplus\n");
  uchar Op;
  if (!(IsConstType(TSumm, m_Operand2)) &&
    (IsConstType(TUnar, m_Operand2) || IsConstType(TUnapm, m_Operand2) ||
    (m_Operand2.Constan(Val) && Val < 0.0) ||
    m_Operand2.MustBracketed() == brOperation || (m_Operand2.Oper_(Op, Op1, Op2) &&
    (IsConstType(TUnapm, Op1) || Op1.Negative()))))
    return Left + Name + '(' + m_Operand2.SWrite() + ')';
  return Left + Name + m_Operand2.SWrite();
  }

TSubt::TSubt( const MathExpr& ex1, const MathExpr& ex2 ) :TOper( ex1, ex2, '-' )
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TSubt::Clone() const
  {
  return new TSubt( m_Operand1.Clone(), m_Operand2.Clone() );
  }

MathExpr  TSubt::Perform()
  {
  return new TSubt( m_Operand1.Perform(), m_Operand2.Perform() );
  }

MathExpr TSubt::Reduce() const
  {
  MathExpr P, A, opr1, opr2, Op11, Op12, Op21, Op22, opr1r, opr2r;
  double Value1, Value2, Value3;
  int Int, N1, D1, N2, D2, NR, DR;
  QByteArray VarName;

  if( m_Operand1.Binar( '=', Op11, Op12 ) && m_Operand2.Binar( '=', Op21, Op22 ) )
    return TBinar( '=', Op11 - Op21, Op12 - Op22 ).Reduce();

  if( m_Operand1.Variab( VarName ) && VarName == "i" )
    {
    Op11 = new TComplexExpr( -m_Operand1, new TConstant( 1 ) );
    return Op11.Reduce();
    }

  if( m_Operand2.Variab( VarName ) && VarName == "i" )
    {
    return new TComplexExpr( m_Operand1.Reduce(), new TConstant( -1 ) );
    }

  s_GlobalInvalid = false;
  opr1r = m_Operand1.Reduce();
  if( IsConstType( TIntegral, m_Operand1 ) )
    opr1r = opr1r.Calculate();
  if(s_GlobalInvalid)return Ethis;
  opr2r = m_Operand2.Reduce(true);
  if( IsConstType( TIntegral, m_Operand2 ) )
    opr2r = opr2r.Calculate();
  else
  if(s_GlobalInvalid)return Ethis;

  if( s_IsLogEquation )
    {
    if( ( IsConstType( TConstant, opr1r ) || IsConstType( TSimpleFrac, opr1r ) ) && opr2r.Log( Op21, Op22 ) && opr2r.ConstExpr() )
      return  Op21.Log( ( ( Op21 ^ opr1r ) / Op22 ).Reduce() );

    if( ( IsConstType( TConstant, opr2r ) || IsConstType( TSimpleFrac, opr2r ) ) && opr1r.Log( Op11, Op12 ) && opr1r.ConstExpr() )
      return  Op11.Log( ( Op12 / ( Op11 ^ opr2r ) ).Reduce() );
    }

  if( IsConstType( TMatr, opr1r ) )
    return CastPtr( TMatr, opr1r )->Subt( opr2r );

  opr1 = opr1r.SortMult();
  opr2 = opr2r.SortMult();

  if( opr1.Complex( Op11, Op12 ) && opr2.Complex( Op21, Op22 ) )
    return CreateComplex( Op11 - Op21, Op12 - Op22 ).Reduce();

  if( opr1.Complex( Op11, Op12 ) && !opr2.Complex( Op21, Op22 ) )
    return CreateComplex( Op11 - opr2, Op12 ).Reduce();

  if( !opr1.Complex( Op11, Op12 ) && opr2.Complex( Op21, Op22 ) )
    return CreateComplex( opr1 - Op21, -Op22 ).Reduce();

  if( ( opr1.Measur_( Op11, Op12 ) && ( opr2.Constan( Value1 ) || opr2.Variab( VarName ) && VarName != "x" ) ) ||
    opr2.Measur_( Op21, Op22 ) && ( opr1.Constan( Value1 ) || opr1.Variab( VarName ) && VarName != "x" ) )
    return opr1 - opr2;

  if( opr1.Measur_( Op11, Op12 ) && opr2.Measur_( Op21, Op22 ) )
    {
    opr1r = Op12 / Op22;
    ReduceMeasure( opr1r );
    if( opr1r.Constan( Value3 ) && Value3 == 1 )
      return new TMeaExpr( ( Op11 - Op21 ).Reduce(), Op22 );
    else
      {
      s_LastError = "Inadmissible operation.";
      throw "Inadmissible operation.";
      }
    }

  if( opr1.Equal( opr2 ) )
    return new TConstant( 0 );

  if( opr1.Summa( Op11, Op12 ) )
    {
    if( Op11.Equal( opr2 ) )
      return Op12.Reduce();

    if( Op12.Equal( opr2 ) )
      return Op11.Reduce();
    }

  if( opr1.Subtr( Op11, Op12 ) && Op11.Equal( opr2 ) )
    return ( -Op12.Reduce() ).Reduce();

  if( opr2.Summa( Op21, Op22 ) )
    {
    if( Op21.Equal( opr1 ) )
      return( -Op22.Reduce() ).Reduce();

    if( Op22.Equal( opr1 ) )
      return ( -Op21.Reduce() ).Reduce();
    }

  if( opr2.Subtr( Op21, Op22 ) && Op21.Equal( opr1 ) )
    return Op22.Reduce();

  if( ( opr1.Subtr( Op11, Op12 ) && opr2.Subtr( Op21, Op22 ) || opr1.Summa( Op11, Op12 ) && opr2.Summa( Op21, Op22 ) ) && Op12.Equal( Op22 ) )
    P = ( Op11.Reduce() - Op21.Reduce() ).Reduce();

  if( opr2.Multp( Op21, Op22 ) )
    {
    if( Op22.Eq( opr1 ) && Op21.ConstExpr() && !ConstExpression( Op22 ) )
      return ( ( Constant( 1 ) - Op21 ) * opr1 ).Reduce();

    if( Op21.Eq( opr1 ) && Op22.ConstExpr() && !ConstExpression( Op21 ) )
      return ( ( Constant( 1 ) - Op22 ) + opr1).Reduce();
    }

  if( opr1.Multp( Op11, Op12 ) && opr2.Multp( Op21, Op22 ) )
    {
    if( Op11.Eq( Op21 ) && Op12.ConstExpr() && Op22.ConstExpr() && !Op21.ConstExpr() )
      return ( ( Op12 - Op22 ) * Op11 ).Reduce();

    if( Op11.Eq( Op22 ) && Op12.ConstExpr() && Op21.ConstExpr() && !ConstExpression( Op22 ) )
      return ( ( Op12 - Op21 ) * Op11 ).Reduce();

    if( Op12.Eq( Op22 ) && Op11.ConstExpr() && Op21.ConstExpr() && !ConstExpression( Op22 ) )
      return ( ( Op11 - Op21 ) * Op12 ).Reduce();

    if( Op12.Eq( Op21 ) && Op11.ConstExpr() && Op22.ConstExpr() && !ConstExpression( Op21 ) )
      return ( ( Op11 - Op22 ) * Op12).Reduce();
    }

  if( opr1.Multp( Op11, Op12 ) )
    {
    if( Op12.Eq( opr2 ) && Op11.ConstExpr() && !ConstExpression( Op12 ) )
      return ( ( Op11 - 1 ) * opr2 ).Reduce();

    if( Op11.Eq( opr2 ) && Op12.ConstExpr() && !ConstExpression( Op11 ) )
      return ( ( Op12 - 1 ) * opr2 ).Reduce();
    }

  if( opr1.Unarminus( Op11 ) && s_CanExchange )
    return ( -( Op11 + opr2 ) ).Reduce();

  if( !opr2.ConstExpr() || NegatConst( opr2 ) )
    {
    Op11 = -opr2;
    Op12 = Op11.Reduce();
    if( !Op12.Unarminus( Op11 ) )
      return ( opr1 + Op12 ).Reduce();
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Cons_int( Int ) )
    return GenerateFraction( N1 - Int * D1, D1 ).Reduce();

  if( opr2.SimpleFrac_( N1, D1 ) && opr1.Cons_int( Int ) )
    return GenerateFraction( Int * D1 - N1, D1 ).Reduce();

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Constan( Value2 ) )
    return new TConstant( ( N1 - Value2 * D1 ) / D1 );

  if( opr2.SimpleFrac_( N1, D1 ) && opr1.Constan( Value2 ) )
    {
    return new TConstant( ( Value2 * D1 - N1 ) / D1 );
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.SimpleFrac_( N2, D2 ) )
    {
    N2 = -N2;
    SumFrac( N1, D1, N2, D2, NR, DR );
    return GenerateFraction( NR, DR ).Reduce();
    }

  if( opr1.Constan( Value1 ) && opr2.Constan( Value2 ) )
    return new TConstant( Value1 - Value2 );

  if( opr2.Constan( Value2 ) && Value2 == 0 || opr2.Newline() )
    return opr1;

  if( opr2.Unarminus( A ) && s_CanExchange )
    return opr1 + A;
  
  if( opr2.SimpleFrac_( N1, D1 ) && opr2.MustBracketed() == brOperation && s_CanExchange )
    return opr1 + GenerateFraction( abs( N1 ), abs( D1 ) );

  if( opr1.Constan( Value1 ) && Value1 == 0 || opr1.Newline() )
    return -opr2;

  if( opr1.Divis( Op11, Op12 ) && opr2.Constan( Value3 ) && Op11.Constan( Value1 ) && Op12.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return new TConstant( Value1 / Value2 - Value3 );
    }

  if( opr2.Divis( Op11, Op12 ) && opr1.Constan( Value3 ) && Op11.Constan( Value1 ) && Op12.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return new TConstant( Value3 - Value1 / Value2 );
    }

  if( opr1.Divis( Op11, Op12 ) && opr2.SimpleFrac_( N1, D1 ) && Op11.Constan( Value1 ) && Op12.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return new TConstant( -N1 / ( double ) D1 + Value1 / Value2 );
    }

  if( opr2.Divis( Op11, Op12 ) && opr1.SimpleFrac_( N1, D1 ) && Op11.Constan( Value1 ) && Op12.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return new TConstant( ( double ) N1 / D1 - Value1 / Value2 );
    }

  return ( opr1 - opr2 ).SimplBySummList();
  }

MathExpr TSubt::Diff( const QByteArray& d )
  {
  return new TSubt( m_Operand1.Diff( d ), m_Operand2.Diff( d ) );
  }

MathExpr TSubt::Integral( const QByteArray& d )
  {
  return new TSubt( m_Operand1.Integral( d ), m_Operand2.Integral( d ) );
  }

MathExpr TSubt::Lim( const QByteArray& v, const MathExpr& lm ) const
  {
  MathExpr result;
  MathExpr  ex, op1, op2, op3, op4, op11, op12, op21, op22, exBase, exExp, ex1, exL;
  bool ng1, ng2;
  int root;
  QByteArray Name;

  MathExpr opr1 = m_Operand1.Lim( v, lm );
  MathExpr opr2 = m_Operand2.Lim( v, lm );

  if( !opr1.IsEmpty() && opr1.Infinity( ng1 ) )
    if( !opr2.IsEmpty() && opr2.Infinity( ng2 ) )
      if( ng1 != ng2 )
        return new TInfinity( ng1 );
      else
        {
        MathExpr This(this->Clone());
        ex = MathExpr( This ).ReduceToMult();
        if( !ex.Equal( This ) )
          return ex.Lim( v, lm );
        if( m_Operand1.Divis( op1, op2 ) )
          {
          if( m_Operand2.Divis( op3, op4 ) && op2.Subtr( op11, op12 ) && op12.Root_( exBase, exExp, root ) &&
            ( root == 2 ) && exBase.Variab( Name ) && ( Name == v ) && op4.Subtr( op21, op22 ) && op22.Root_( exBase, exExp, root ) &&
            ( root == 3 ) && exBase.Variab( Name ) && ( Name == v ) )
            {
            if( v == "t" )
              Name = "x";
            else
              Name = "t";
            ex1 = op1 / ( op11 - ::Power( Name, 3 ) ) - op3 / ( op21 - ::Power( Name, 2 ) );
            ex = new TRoot( lm, 6 );
            exL = ex.Reduce();
            ex = new TLimit( false, ex1, new TVariable( false, Name ), exL );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', new TLimit( false, Ethis, new TVariable( false, v ), lm ), ex ) );
            ex1 = CalcMulti( 2, ex1, true );
            result = ex1.Lim( Name, exL );
            s_GlobalInvalid = result.IsEmpty();
            if( !s_GlobalInvalid )
              return new TBinar( '=', ex, result );
            }
          else
            {
            TSolutionChain::sm_SolutionChain.AddExpr( new TLimit( false, Ethis, new TVariable( false, v ), lm ) );
            ex = CalcMulti( 2, ex, true );
            return ex.Lim( v, lm );
            }
          }
        else
          if( ( m_Operand1.Root_( op1, op2, root ) || m_Operand2.Root_( op1, op2, root ) ) && root == 2 )
            {
            ex = ( op1 - ( m_Operand2 ^ 2 ) ).Reduce() / ( m_Operand1 + m_Operand2 );
            result = ex.Lim( v, lm );
            s_GlobalInvalid = result.IsEmpty();
            if( !s_GlobalInvalid )
              return new TBinar( '=', new TLimit( false, ex, new TVariable( false, v ), lm ), result );
            }
          else
            {
            s_GlobalInvalid = true;
            return nullptr;
            }
        }
    else
      return opr1;
  else
    if( !opr2.IsEmpty() && opr2.Infinity( ng2 ) )
      return new TInfinity( !ng2 );
    else
      if( opr1.IsEmpty() || opr2.IsEmpty() )
        return nullptr;
      else
        result = opr1 - opr2;
  return result;
  }

bool TSubt::Eq( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;
  if( E2.Subtr( op21, op22 ) )
    return m_Operand1.Eq( op21 ) && m_Operand2.Eq( op22 );
  return false;
  }

bool TSubt::Equal( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;

  if( E2.Subtr( op21, op22 ) && m_Operand1.Equal( op21 ) && m_Operand2.Equal( op22 ) )
    return true;

  MathExpr Ar2;

  if( IsConstType( TSubt, E2 ) || IsConstType( TSumm, E2 ) || IsConstType( TUnar, E2 ) || E2.Binar( msPlusMinus, op21, op22 ) 
    || IsConstType( TComplexExpr, E2 ) )
    {
    MathExpr Ar1 = SummList();
    Ar2 = E2.SummList();
    if( Ar1.Equal( Ar2 ) ) return true;
    if( CastPtr( TLexp, Ar1 )->Count() == CastPtr( TLexp, Ar2 )->Count() ) return false;
    MathExpr R( Reduce() );
    MathExpr RE( E2.Reduce() );
    if( Eq( R ) )
      if( RE.Eq( E2 ) )
        return false;
      else
        return RE.Equal( Ethis );
    return R.Equal( RE );
    }

  QByteArray _eqName;

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );

  return false;
  }

MathExpr TSubt::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TSubt( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

bool TSubt::Subtr( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TSubt::IsLinear() const
  {
  return m_Operand1.IsLinear() && m_Operand2.IsLinear();
  }

MathExpr TSubt::TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower )
  {
  QByteArray Name;
  MathExpr Arg;
  if( sName == "sin" )
    if( exPower.IsEmpty() || exPower == 2 || !( m_Operand1.Funct( Name, Arg ) ) || ( Name != "cos" ) )
      return nullptr;
  MathExpr ex1 = new TDivi( Arg, exArg );
  MathExpr ex2 = ex1.Reduce();
  if( ex2 == 2 )
    if( m_Operand1 == 1 )
      return new TConstant( 2 );
    else
      return new TConstant( -2 );
  return nullptr;
  }

QByteArray TSubt::SWrite() const
  {
  MathExpr Op1, Op2;
  QByteArray Left = m_Operand1.SWrite();
  double Val;
  if (m_Operand2.Negative() && !TSumm::sm_ShowMinusByAddition)
    {
    if (m_Operand2.Unarminus(Op1))
      return Left + "\\longplus\n" + Op1.SWrite();
    if (m_Operand2.Constan(Val) && Val < 0 )
      return Left + "\\longplus\n" + TConstant(qAbs(Val)).SWrite();
    if (m_Operand2.Multp(Op1, Op2) && Op1.Constan(Val) && Val < 0)
      return Left + "\\longplus\n" + (Constant(-Val) * Op2).SWrite();
    }
  QByteArray Name(charToTex(m_Name));
  if (Name[0] == '\\') Name += '\n';
  if ( !TOper::sm_InsideChart) Name = "\\longminus\n";
  uchar Op;
  if ((IsConstType(TUnar, m_Operand2) || IsConstType(TUnapm, m_Operand2) ||
    (m_Operand2.Constan(Val) && Val < 0.0) ||
    m_Operand2.MustBracketed() == brOperation || (m_Operand2.Oper_(Op, Op1, Op2) &&
    (IsConstType(TUnapm, Op1) || Op1.Negative()))))
    return Left + Name + '(' + m_Operand2.SWrite() + ')';
  return Left + Name + m_Operand2.SWrite();
  }

TMult::TMult( const MathExpr& ex1, const MathExpr& ex2, uchar sign ) :TOper( ex1, ex2, sign )
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TMult::Clone() const
  {
  return new TMult( m_Operand1.Clone(), m_Operand2.Clone(), m_Name );
  }

MathExpr TMult::Reduce() const
  {
  MathExpr P, Op11, Op12, Op21, Op22, op3, op4, opr1, opr2;
  QList<MathExpr> Multiplicators;
  double Value1, Value2, Value3;
  int Int, N1, D1, N2, D2, NR, DR;
  QByteArray VarName;
  bool GlobInvOld;

  auto Append = [&](MathExpr& Exp)
    {
    if (IsType(TComplexExpr, Exp))
      Multiplicators.prepend(Exp);
    else
      Multiplicators.append(Exp);
    };

  std::function<void( const MathExpr& )> CreateMultList = [&] ( const MathExpr& ex )  
    {
    MathExpr exLeft, exRight;
    if( ex.Multp( exLeft, exRight ) )
      {
      CreateMultList( exLeft );
      CreateMultList( exRight );
      return;
      }
    MathExpr exp = ex.Reduce();
    if( exp.Multp( exLeft, exRight ))
      {
      Append( exLeft );
      Append( exRight );
      }
    else
      Append(exp);
    };

  if(sm_ReduceOperands)
    {
    if(m_Operand1.Constan(Value1))
      if( m_Operand2.Constan(Value2))
         return Constant(Value1 * Value2);
      else
        if(Value1 == 1)
          return m_Operand2;
        else
          if(Value1 == -1)
            return -m_Operand2;
    uchar C;
    if( m_Operand1.Oper_(C, opr1, opr2) && (C == '+' || C == '-') )
      Op11 = m_Operand1.Reduce();
    if( m_Operand2.Oper_(C, opr1, opr2) && (C == '+' || C == '-') )
      if(Op11.IsEmpty())
        return m_Operand1 * m_Operand2.Reduce();
      else
        return Op11 * m_Operand2.Reduce();
     if(Op11.IsEmpty())
       return Ethis;
     return Op11 * m_Operand2;
     }

  if( IsConstType( TMult, m_Operand1 ) || IsConstType( TMult, m_Operand2 ) )
    {
    CreateMultList( Ethis );
    for( int i = 0; i < Multiplicators.count() - 1; i++ )
      for( int k = i + 1; k < Multiplicators.count();)
        {
        opr1 = ( Multiplicators[i] * Multiplicators[k] );
//        if( Eq( opr1 ) ) return opr1;
        opr2 = opr1.Reduce();
        if(!opr2.Multp(op3, op4) || !op3.Eq(Multiplicators[k]))
          opr1 = opr2;
        if( opr1.Multp( op3, op4 ) && !op4.ImUnit() )
          {
          k++;
          continue;
          }
        Multiplicators[k] = opr1;
        Multiplicators.removeAt( i );
        k = i + 1;
        }
    MathExpr result = Multiplicators[0];
    for( int i = 1; i < Multiplicators.count(); i++ )
      result *= Multiplicators[i];
    if( Multiplicators.count() > 1 )
      return result.SortMult();
    return result;
    }

  opr1 = m_Operand1.Reduce(true);
  opr2 = m_Operand2.Reduce(true);

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.SimpleFrac_( N2, D2 ) )
    {
    GlobInvOld = s_GlobalInvalid;
    s_GlobalInvalid = false;
    MultFractions( N1, D1, N2, D2, NR, DR );
    if( s_GlobalInvalid )
      {
      if( NR == 0 )
        return Constant( ( ( double ) N1 / D1 ) * ( ( double ) N2 / D2 ) );
      return Ethis;
      }
    s_GlobalInvalid = GlobInvOld;
    return GenerateFraction( NR, DR ).Reduce();
    }

  if( sm_FullReduce && m_Operand1.Constan( Value1 ) && m_Operand2.Constan( Value2 ) )
    return Constant( Value1 * Value2 );
  
  if( s_SmartReduceLogArg && opr2.Log( Op21, Op22 ) && opr1.ConstExpr() )
    {
    if( IsConstType( TSimpleFrac, opr1 ) && Op22.Cons_int( Int ) )
      {
      op3 = ( Op22 ^ opr1 ).Reduce();
      if( op3.Cons_int( Int ) )
        return new TLog( opr1, op3 );
      }
    return opr1 * opr2;
    }

  if( s_IsLogEquation && ( IsConstType( TConstant, opr1 ) || IsConstType( TSimpleFrac, opr1 ) ) && opr2.Log( Op21, Op22 ) && opr2.ConstExpr() )
    return new TLog( Op21, ( Op22 ^ opr1 ).Reduce() );

  if( opr2.Variab( VarName ) && VarName == "i" )
    return new TComplexExpr( Constant( 0 ), opr1 );

  if( IsConstType( TMatr, opr1 ) )
    return CastPtr(TMatr, opr1 )->Mult( opr2 );

  if( IsConstType( TMatr, opr2 ) )
    return CastPtr( TMatr, opr2 )->Mult( opr1 );

  if( opr1.Complex( Op11, Op12 ) && opr2.Complex( Op21, Op22 ) )
    return CreateComplex( (( Op11 * Op21 ) - ( Op12 * Op22 )).Reduce(), (( Op11 * Op22 ) + ( Op12 * Op21 )).Reduce() ).Reduce();

  if( opr1.Complex( Op11, Op12 ) && !opr2.Complex( Op21, Op22 ) )
    return CreateComplex( (Op11 * opr2).Reduce(), (Op12 * opr2).Reduce() ).Reduce();

  if (!opr1.Complex(Op11, Op12) && opr2.Complex(Op21, Op22))
    return CreateComplex((Op21 * opr1).Reduce(), (Op22 * opr1).Reduce()).Reduce();;

  if( opr1.Measur_( Op11, Op12 ) && ( opr2.Constan( Value1 ) || opr2.SimpleFrac_( N1, D1 ) ) )
    return new TMeaExpr( ( Op11 * opr2 ).Reduce(), Op12 );

  if( opr2.Measur_( Op21, Op22 ) && ( opr1.Constan( Value1 ) || opr1.SimpleFrac_( N1, D1 ) ) )
    return new TMeaExpr( ( opr1 * Op21 ).Reduce(), Op22 );

  if( opr1.Measur_( Op11, Op12 ) && opr2.Measur_( Op21, Op22 ) )
    {
    P = Op12 * Op22;
    ReduceMeasure( P );
    if( P.Constan( Value3 ) && Value3 == 1 )
      return ( Op11 * Op21 ).Reduce();
    return new TMeaExpr( ( Op11 * Op21 ).Reduce(), P );
    }

  if( opr1.Measur_( Op11, Op12 ) && Op11.Constan( Value1 ) && Value1 == 0 )
    return new TMeaExpr( Constant( 0 ), Op12 );

  if( opr2.Measur_( Op21, Op22 ) && Op21.Constan( Value2 ) && Value2 == 0 )
    return new TMeaExpr( Constant( 0 ), Op22 );

  if( opr2.ConstExpr() && !opr1.ConstExpr() )
    return ( opr2 * opr1 ).Reduce();

  if( ConstExpression( opr1 ) && opr2.Multp( Op21, Op22 ) && ConstExpression( Op21 ) )
    return ( opr1 * Op21.Reduce() * Op22.Reduce() ).Reduce();
 
  if( opr1.Multp( Op11, Op12 ) && ConstExpression( Op11 ) )
    {
    P = Op11.Reduce() * Op12.Reduce() * opr2;
    if( CastPtr( TMult, P )->m_Operand1.Eq( opr1 ) && CastPtr( TMult, P )->m_Operand2.Eq( opr2 ) )
      return P;
    return P.Reduce();
    }

  if( opr2.Multp( Op21, Op22 ) && ConstExpression( Op21 ) )
    return ( Op21.Reduce() * opr1 * Op22.Reduce() ).Reduce();

  if( opr1.Divis( Op11, Op12 ) )
    {
    if( Op12.Equal( opr2 ) )
      return Op11.Reduce();

    if( Op11.Cons_int( NR ) && NR == 1 )
      return opr2 / Op12;

    if( opr2.Divis( Op21, Op22 ) )
      {
      if( Op11.Equal( Op22 ) )
        return ( Op21.Reduce() / Op12.Reduce() ).Reduce();
      if( Op12.Equal( Op21 ) )
        return ( Op11.Reduce() / Op22.Reduce() ).Reduce();
      return ( Op11 * Op21 / ( Op12 * Op22 ) ).Reduce();
      }
    return Op11 * opr2 / Op12;
    }
  if( opr2.Divis( Op21, Op22 ) )
    {
    if( Op22.Equal( opr1 ) )
      return Op21.Reduce();
    if( Op21.Cons_int( NR ) && NR == 1 )
      return opr1 / Op22;
    }

  if( opr2.Power( Op21, Op22 ) && ( !( IsConstType( TRoot, opr2 ) ) || s_RootToPower ) && !s_PowerToMult && opr1.Equal( Op21 ) && ( !s_IsIntegral || Op22.ConstExpr() ) )
    return ( opr1 ^ ( ( Constant( 1 ) + Op22 ).Reduce() ) ).Reduce();

  if( opr1.Power( Op11, Op12 ) && opr2.Equal( Op11 ) && ( !( IsConstType( TRoot, opr1 ) ) || s_RootToPower ) )
    return ( opr2 ^ ( Constant( 1 ) + Op12.Reduce() ) ).Reduce();

  if( opr1.Root_( Op11, Op12, N1 ) && opr2.Root_( Op21, Op22, N2 ) )
    if( N1 == N2 )
      return ( Op11 * Op21 ).Root(N1 ).Reduce();
    else
      if( N1 % N2 == 0 )
        return new TRoot( ( Op11 * ( Op21 ^ ( N1 / N2 ) ) ).Reduce(), N1 );
      else
        if( N2 % N1 == 0 )
          return new TRoot( ( Op21 * ( Op11 ^ ( N2 / N1 ) ) ).Reduce(), N2 );

  if( opr1.Power( Op11, Op12 ) && opr2.Power( Op21, Op22 ) && ( s_RootToPower || ( !( IsConstType( TRoot, opr1 ) ) && !( IsConstType( TRoot, opr2 ) ) ) ) )
    {
    if( Op11.Equal( Op21 ) )
      return ( Op11 ^ ( Op12 + Op22 ) ).Reduce();
    if( Op12.Equal( Op22 ) )
      {
      Op22 = ( Op11 * Op21 ).Reduce();
      if( sm_FullReduce || !( IsConstType( TMult, Op22 ) ) )
        return (Op22 ^ Op12).Reduce();
      }
    }

  if( ( opr1.Divis( Op11, Op12 ) && opr2.Constan( Value3 ) ) || ( opr2.Divis( Op11, Op12 ) && opr1.Constan( Value3 ) ) )
    if( Op11.Constan( Value1 ) && Op12.Constan( Value2 ) )
      return Constant( Value1 / Value2 * Value3 );
    else
      return ( Constant( Value3 ) * Op11 / Op12 ).Reduce();

  if( ( opr1.Divis( Op11, Op12 ) && opr2.SimpleFrac_( N1, D1 ) ) || ( opr2.Divis( Op11, Op12 ) && opr1.SimpleFrac_( N1, D1 ) ) )
    if( Op11.Constan( Value1 ) && Op12.Constan( Value2 ) )
      {
      if( s_ShowDiviMessages && !s_CalcOnly )
        Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
      return Constant( ( ( double ) N1 / D1 ) * ( Value1 / Value2 ) );
      }
    else
      return ( Op11 * Constant( N1 ) / ( Op12 * Constant( D1 ) ) ).Reduce();

  if( opr1.Newline() )
    return opr2;

  if( opr2.Newline() )
    return opr1;

  if( opr1.Constan( Value1 ) && opr2.Constan( Value2 ) )
    {
    if( Value1 != 0 && Value2 != 0 )
      if( ( log( abs( Value1 ) ) + log( abs( Value2 ) ) ) > 320 * log( 10 ) )
        {
        if( !s_CalcOnly )
          throw ErrParser( X_Str( "INFVAL", "Result is infinity" ), peInfinity );
        s_GlobalInvalid = true;
        return Ethis;
        }
    return Constant( Value1 * Value2 );
    }

  if( ( opr1.SimpleFrac_( N1, D1 ) && opr2.Cons_int( Int ) ) || ( opr2.SimpleFrac_( N1, D1 ) && opr1.Cons_int( Int ) ) )
    {
    if( ( N1 * 1.0 * Int ) > ( 1.0*INT_MAX ) )
      return Constant( ( double ) N1 / D1 * Int );
    return GenerateFraction( N1 * Int, D1 ).Reduce();
    }

  if( ( opr1.SimpleFrac_( N1, D1 ) && opr2.Constan( Value2 ) ) || ( opr2.SimpleFrac_( N1, D1 ) && opr1.Constan( Value2 ) ) )
    return Constant( Value2 * N1 / D1 ).Reduce();

  if( opr1.Constan( Value1 ) )
    {
    if( Value1 == 0 )
      return Constant( 0 );
    if( Value1 == 1 )
      return opr2.Reduce();
    if( Value1 < 0 )
      return ( -( Constant( -Value1 ) * opr2.Reduce() ) ).Reduce();
    }

  if( opr2.Constan( Value2 ) )
    {
    if( Value2 == 0 )
      return Constant( 0 );
    if( Value2 == 1 )
      return opr1.Reduce();
    if( Value2 < 0 )
      return ( -Constant( -Value2 ) * opr1.Reduce() ).Reduce();
    }

  if( opr1.Eq( opr2 ) )
    return ( opr1 ^ 2 ).Reduce();

  if( opr2.Divis( Op21, Op22 ) )
    if( opr1.Eq( Op22 ) )
      return Op21.Reduce();

  if( opr1.Divis( Op11, Op12 ) )
    if( opr2.Eq( Op12 ) )
      return Op11.Reduce();

  if( opr1.Unarminus( Op11 ) )
    {
    Op22 = ( -opr2 ).Reduce();
    if( !Op22.Unarminus( Op12 ) )
      return ( Op11 * Op22 ).Reduce();
    return ( -( Op11 * opr2 ) ).Reduce();
    }

  if( opr2.Unarminus( Op21 ) )
    {
    Op12 = ( -opr1 ).Reduce();
    if( !Op12.Unarminus( Op22 ) )
      return ( Op12 * Op21 ).Reduce();
    return ( -( opr1 * Op21 ) ).Reduce();
    }

  if( opr1.Unapm_( Op11 ) )
    return ( Op11 * opr2 ).Unapm().Reduce();

  if( opr2.Unapm_( Op21 ) )
    return ( opr1 * Op21 ).Unapm().Reduce();

  if( opr1.Multp( Op11, Op12 ) && opr1.Eq( m_Operand1 ) )
    {
    op3 = Op12 * opr2;
    op4 = op3.Reduce();
    if( !op3.Equal( op4 ) )
      return ( Op11 * op4 ).Reduce();

    op3 = Op11 * opr2;
    op4 = op3.Reduce();
    if( !op3.Equal( op4 ) )
      return ( Op12 * op4 ).Reduce();
    }

  if( opr2.Multp( Op11, Op12 ) && opr2.Eq( m_Operand2 ) )
    {
    op3 = opr1 * Op11;
    op4 = op3.Reduce();
    if( !op3.Equal( op4 ) )
      return ( op4 * Op12 );

    op3 = opr1 * Op12;
    op4 = op3.Reduce();
    if( !op3.Equal( op4 ) )
      return ( op4 * Op11 );
    }

  if( opr1.Funct( VarName, Op11 ) && ( VarName == "tan" ) )
    {
    op3 = opr2.TrigTerm( "cos", Op11 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TFunc( false, "sin", Op11 );
      else
        return new TMult( op3, new TFunc( false, "sin", Op11 ), m_Name );
    }

  if( opr2.Funct( VarName, Op11 ) && VarName == "tan" )
    {
    op3 = opr1.TrigTerm( "cos", Op11 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TFunc( false, "sin", Op11 );
      else
        return new TMult( op3, new TFunc( false, "sin", Op11 ), m_Name );
    }

  if( opr1.Funct( VarName, Op11 ) && ( VarName == "cot" ) )
    {
    op3 = opr2.TrigTerm( "sin", Op11 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TFunc( false, "cos", Op11 );
      else
        return new TMult( op3, new TFunc( false, "cos", Op11 ), m_Name );
    }

  if( opr2.Funct( VarName, Op11 ) && VarName == "cot" )
    {
    op3 = opr1.TrigTerm( "sin", Op11 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TFunc( false, "cos", Op11 );
      else
        return new TMult( op3, new TFunc( false, "cos", Op11 ), m_Name );
    }

  if( opr1.Funct( VarName, Op11 ) && VarName == "exp" && opr2.Funct( VarName, Op21 ) && VarName == "exp" )
    {
    op3 = ( Op11 + Op21 ).Reduce();
    if( op3 == 0 )
      return Constant( 1 );
    return new TFunc( false, "exp", op3 );
    }

  if( opr1.Power( Op11, Op12 ) && Op11.Funct( VarName, Op21 ) && VarName == "tan" )
    {
    op3 = opr2.TrigTerm( "cos", Op21, Op12 );
    if( !op3.IsEmpty() )
      if(op3 == 1 )
        return new TPowr( new TFunc( false, "sin", Op21 ), Op12 );
      else
        return new TMult( op3, new TPowr( new TFunc( false, "sin", Op21 ), Op12 ), m_Name );
    }

  if( opr2.Power( Op11, Op12 ) && Op11.Funct( VarName, Op21 ) && VarName == "tan" )
    {
    op3 = opr1.TrigTerm( "cos", Op21, Op12 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TPowr( new TFunc( false, "sin", Op21 ), Op12 );
      else
        return new TMult( op3, new TPowr( new TFunc( false, "sin", Op21 ), Op12 ), m_Name );
    }

  if( opr1.Power( Op11, Op12 ) && Op11.Funct( VarName, Op21 ) && VarName == "cot" )
    {
    op3 = opr2.TrigTerm( "sin", Op21, Op12 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TPowr( new TFunc( false, "cos", Op21 ), Op12 );
      else
        return new TMult( op3, new TPowr( new TFunc( false, "cos", Op21 ), Op12 ), m_Name );
    }

  if( opr2.Power( Op11, Op12 ) && Op11.Funct( VarName, Op21 ) && VarName == "cot" )
    {
    op3 = opr1.TrigTerm( "sin", Op21, Op12 );
    if( !op3.IsEmpty() )
      if( op3 == 1 )
        return new TPowr( new TFunc( false, "cos", Op21 ), Op12 );
      else
        return new TMult( op3, new TPowr( new TFunc( false, "cos", Op21 ), Op12 ), m_Name );
    }

  if( s_NoLogReduce && opr2.Funct( VarName, Op21 ) && VarName == "ln" && Op21.ConstExpr() && IsConstType( TConstant, opr1 ) )
    return new TFunc( false, "ln", ( Op21 ^ opr1 ).Reduce() );

  if( opr1.Binar( '=', Op11, Op12 ) )
    return new TBinar( '=', ExpandExpr( Op11 * opr2 ), ExpandExpr( Op12 * opr2 ) );

  if( opr2.Binar( '=', Op11, Op12 ) )
    return new TBinar( '=', ExpandExpr( Op11 * opr1 ), ExpandExpr( Op12 * opr1 ) );

  if( opr1.Summa(Op11, Op12 ) && !(IsType(TRoot, Op11)) && !(IsType(TRoot, Op12)))
    {
    P = new TSumm( ExpandExpr( Op11 * opr2 ), ExpandExpr( Op12 * opr2 ) );
    return P.Reduce();
    }
  if( opr1.Subtr(Op11, Op12 ) && !(IsType(TRoot, Op11)) && !(IsType(TRoot, Op12)))
    {
    P = new TSubt( ExpandExpr( Op11 * opr2 ), ExpandExpr( Op12 * opr2 ) );
    return P.Reduce();
    }
  if( opr2.Summa( Op11, Op12 ) && !(IsType(TRoot, Op11)) && !(IsType(TRoot, Op12)) )
    {
    P = new TSumm( ExpandExpr( Op11 * opr1 ), ExpandExpr( Op12 * opr1 ) );
    return P.Reduce();
    }
  if( opr2.Subtr( Op11, Op12 ) && !(IsType(TRoot, Op11)) && !(IsType(TRoot, Op12)) )
    {
    P = new TSubt( ExpandExpr( Op11 * opr1 ), ExpandExpr( Op12 * opr1 ) );
    return P.Reduce();
    }
  return new TMult( opr1, opr2, m_Name );
  }

MathExpr TMult::Perform()
  {
  return new TMult( m_Operand1.Perform(), m_Operand2.Perform(), m_Name );
  }

MathExpr TMult::Diff( const QByteArray& d )
  {
  MathExpr Diff1 = m_Operand1.Diff( d ).Reduce();
  MathExpr Diff2 = m_Operand2.Diff( d ).Reduce();
  if( Diff1 == 0 )
    if( Diff2 == 0 )
      return Diff2;
    else
      return m_Operand1 * Diff2;
  if( Diff2 == 0 )
    if( Diff1 == 0 )
      return Diff1;
    else
      return Diff1 * m_Operand2;
  return Diff1 * m_Operand2 + m_Operand1 * Diff2;
  }

MathExpr TMult::Integral( const QByteArray& d )
  {
  MathExpr df, rd, arg, u, du, dv, v, vdu, uv, it;
  int n;
  QByteArray name;

  df = m_Operand1.Diff( d );
  rd = df.Reduce();

  if( rd.Cons_int( n ) && n == 0 )
    return m_Operand1 * m_Operand2.Integral( d );

  df = m_Operand2.Diff( d );
  rd = df.Reduce();

  if( rd.Cons_int( n ) && n == 0 )
    return m_Operand1.Integral( d ) * m_Operand2;

  df = Expand(this);
  s_GlobalInvalid = false;

  if( df.Eq( this ) )
    {
    if( ++s_IntegralCount > 3 )
      {
      TExpr::sm_IntegralError = true;
      return Ethis;
      }
    if( ( !m_Operand1.Funct( name, arg ) || name == "ln" ) && !( m_Operand2.Funct( name, arg ) && name == "ln" ) )
      {
      u = m_Operand1;
      dv = m_Operand2;
      }
    else
      {
      u = m_Operand2;
      dv = m_Operand1;
      }
    v = dv.Integral( d );
    uv = u * v;
    du = u.Diff( d );
    vdu = ( v * du ).Reduce();
    it = new TIntegral( false, vdu, new TVariable( false, d ) );
    return (uv - it).Clone();
    }
  else
    {
    rd = df.Reduce();
    return rd.Integral( d );
    }
  }

MathExpr TMult::Lim( const QByteArray& v, const MathExpr& lm ) const
  {
  auto OpenAmbiguity = [=] ( const MathExpr& operand1, const MathExpr& operand2 )
    {
    s_OpenMultAmbiguity = true;
    QByteArray Name;

    MathExpr op1 = operand1.Reduce();
    MathExpr op2 = operand2.Reduce();

    MathExpr op11, op12, op21, op22, rd;
    if( op1.Divis( op11, op12 ) )
      if( op2.Divis( op21, op22 ) )
        {
        rd = ( op11 * op21 ) / ( op12 * op22 );
        return rd.Lim( v, lm );
        }
      else
        {
        rd = ( op11 * op2 ) / op12;
        return rd.Lim( v, lm );
        }
    else
      if( operand2.Divis( op21, op22 ) )
        {
        rd = ( op1 * op21 ) / op22;
        return rd.Lim( v, lm );
        }

    if( operand1.Funct( Name, op1 ) )
      rd = operand1 / ( Constant( 1 ) / operand2 );
    else
      rd = operand2 / ( Constant( 1 ) / operand1 );
    return rd.Lim( v, lm );
    };

  MathExpr op1 = m_Operand1.Lim( v, lm );
  bool nl1 = false;
  if( !op1.IsEmpty() )
    nl1 = op1.Reduce() == 0;

  MathExpr op2 = m_Operand2.Lim( v, lm );
  bool nl2 = false;
  if( !op2.IsEmpty() )
    nl2 = op2.Reduce() == 0;

  if( op1.IsEmpty() )
    if( nl2 )
      return new TConstant( 0 );
    else
      return nullptr;
  if( op2.IsEmpty() )
    if( nl1 )
      return new TConstant( 0 );
    else
      return nullptr;

  bool ng1, ng2;

  if( op1.Infinity( ng1 ) )
    if( op2.Infinity( ng2 ) )
      return new TInfinity( ng1 ^ ng2 );
    else
      if( nl2 )
        return OpenAmbiguity( m_Operand1, m_Operand2 );
      else
        return new TInfinity( ng1 ^ op2.Negative() );
  else
    if( op2.Infinity( ng2 ) )
      if( nl1 )
        return OpenAmbiguity( m_Operand1, m_Operand2 );
      else
        return new TInfinity( ng2 ^ op1.Negative() );
    else
      return op1 * op2;
  }

bool TMult::Eq( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;
  if( E2.Multp( op21, op22 ) )
    {
    QVector<MathExpr> Opers = GetOperands();
    QVector<MathExpr> EOpers = CastConstPtr(TMult, E2)->GetOperands();
    if(Opers.length() != EOpers.length()) return false;
    for(int i = 0; i < Opers.length(); i++)
      {
      bool Found = false;
      for(int j = 0; !Found && (j < EOpers.length()); j++)
        {
        Found = Opers[i].Eq(EOpers[j]);
        if(Found)
          EOpers.removeAt(j);
        }
      if(!Found) return false;
      }
    return true;
//    return m_Operand1.Eq( op21 ) && m_Operand2.Eq( op22 ) || m_Operand1.Eq( op22 ) && m_Operand2.Eq( op21 );
    }
  if( s_EqualPicture && E2.Unarminus( op21 ) && IsConstType( TMult, op21 ) )
      {
      bool OldEqualPicture = s_EqualPicture;
      s_EqualPicture = false;
      op22 = Reduce();
      bool result = op22.Eq( E2 );
      s_EqualPicture = OldEqualPicture;
      return result;
      }
  return false;
  }

bool TMult::Equal( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;
  if( E2.Multp( op21, op22 ) )
    if( m_Operand1.Equal( op21 ) && m_Operand2.Equal( op22 ) || m_Operand1.Equal( op22 ) && m_Operand2.Equal( op21 ) )
      return true;

  int N, N1, D1;
  bool TestReduce = m_Operand1.SimpleFrac_( N1, D1 ) || m_Operand2.SimpleFrac_( N1, D1 ) || m_Operand1.MixedFrac_( N, N1, D1 )
    || m_Operand2.MixedFrac_( N, N1, D1 );
  if( !TestReduce && !op21.IsEmpty() )
    {
    TestReduce = op21.SimpleFrac_( N1, D1 ) || op22.SimpleFrac_( N1, D1 )
      || op21.MixedFrac_( N, N1, D1 ) || op22.MixedFrac_( N, N1, D1 );
    if( !TestReduce )
      {
      MathExpr Base, Powr, BaseE, PowrE;
      TestReduce = (m_Operand1.Power(Base, Powr) || m_Operand2.Power(Base, Powr)) && (op21.Power(BaseE, PowrE) || op22.Power(BaseE, PowrE))
        && ((!(IsConstType(TMult, m_Operand1)) && !((IsConstType(TMult, m_Operand2)))) || (!(IsConstType(TMult, op21)) && !((IsConstType(TMult, op22)))));
      if( TestReduce && !Base.Equal( BaseE ) ) return false;
      }
    }
  if( TestReduce )
    {
    static bool IsRecursion = false;
    bool bResult = false;
    if( !IsRecursion )
      {
      IsRecursion = true;
      bResult = Reduce().Equal( E2.Reduce() );
      }
    if( bResult )
      {
      IsRecursion = false;
      return true;
      }
    }

  QByteArray _eqName;
  if( E2.Multp( op21, op22 ) || E2.Divis( op21, op22 ) && op21.Cons_int( N ) && N == 1 || E2.Unarminus( op21 ) || E2.Variab( _eqName ) )
    {
    static bool IsListRecursion = false;
    if( IsListRecursion )
      {
      IsListRecursion = false;
      return false;
      }
    MathExpr Ar1 = FactList(); 
    MathExpr Ar2 = E2.FactList(); 
    IsListRecursion = true;
    bool bResult = Ar1.Equal( Ar2 );
    IsListRecursion = false;
    return bResult;
    }

  MathExpr Ar2;
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );

  return false;
  }

bool TMult::Multp( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TMult::Multiplication( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TMult::IsFactorized( const QByteArray& Name )
  {
  return m_Operand1.IsFactorized( Name ) && m_Operand2.IsFactorized( Name );
  }

QByteArray TMult::WriteE() const
  {
  QByteArray S;
  if( !m_Operand1.IsEmpty() )
    S = "((" + m_Operand1.WriteE() + ")" + m_Name;
  else
    S = "(???)" + m_Name;
  if( !m_Operand2.IsEmpty() )
    S += "(" + m_Operand2.WriteE() + "))";
  else
    S += "(???)";
  return S;
  }

QByteArray TMult::SWrite() const
  {
  QByteArray Left( m_Operand1.SWrite() );
  if( m_Operand1.MustBracketed() == brOperation )
    Left = '(' + Left + ')';
  QByteArray Right( m_Operand2.SWrite() );
  if( Right.left( 1 ) == "-" && sm_ShowUnarMinus || m_Operand2.MustBracketed() == brOperation )
    Right = '(' + Right + ')';
  if(IsConstType(TFunc, m_Operand1) && IsConstType(TFunc, m_Operand2))
    return Left + charToTex(m_Name) + Right;
  bool Unvisible = !sm_ShowMultSign && !( IsConstType( TConstant, m_Operand2 ) );
  if( Unvisible )
    {
    Unvisible = Right[0] == '(';
    if( !Unvisible )
      {
      Unvisible = !( m_Operand2.Negative() || IsConstType( TUnapm, m_Operand2 ) );
      if( Unvisible )
        {
        Unvisible = Left.endsWith( ')' ) || IsConstType( TPowr, m_Operand1 ) || m_Operand1.IsIndexed();
        if( !Unvisible )
          {
          MathExpr Op1, Op2;
          uchar Sign;
          if( IsConstType( TConstant, m_Operand1 ) || m_Operand1.Oper_( Sign, Op1, Op2 ) && IsConstType( TConstant, Op2 ) )
            Unvisible = !( m_Operand2.Multp( Op1, Op2 ) && IsConstType( TConstant, Op1 ) ||
            m_Operand2.Power( Op1, Op2 ) && IsConstType( TConstant, Op1 ) || m_Operand2.Measur_(Op1, Op2 ) );
          else
            if( IsConstType( TVariable, m_Operand1 ) || m_Operand1.Oper_( Sign, Op1, Op2 ) && IsConstType( TVariable, Op2 ) )
              Unvisible = !( IsConstType( TVariable, m_Operand2 ) || IsConstType( TFunc, m_Operand2 ) || 
              ( m_Operand2.Oper_( Sign, Op1, Op2 ) && ( IsConstType( TVariable, Op1 ) || IsConstType( TFunc, Op1 ) ) ) || 
              m_Operand2.Multp( Op1, Op2 ) && IsConstType( TConstant, Op1 ) ||
              m_Operand2.Power( Op1, Op2 ) && IsConstType( TConstant, Op1 ) );
          }
        }
      }
    }
  if( Right[0] == 'd' && Right.length() > 1 && _litera( Right[1] ) || Unvisible )
//  if( Right[0] == 'd' && Right.length() > 1 && _litera( Right[1] ) )
    return Left += "\\setunvisible\n*\\setvisible\n" + Right;
  return Left + charToTex(m_Name) + Right;
//  return '(' + Left + charToTex(m_Name) + Right +')';
  }

MathExpr TMult::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TMult( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ), m_Name );
  }

bool TMult::Negative() const
  {
  return m_Operand1.Negative() && !m_Operand2.Negative() || m_Operand2.Negative() && !m_Operand1.Negative();
  }

bool TMult::IsLinear() const
  {
  return true;
  }

MathExpr TMult::TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower )
  {
  MathExpr result = m_Operand1.TrigTerm( sName, exArg, exPower );
  MathExpr ex;
  if( !result.IsEmpty() )
    ex = new TMult( m_Operand2, result, m_Name );
  else
    {
    result = m_Operand2.TrigTerm( sName, exArg, exPower );
    if( !result.IsEmpty() )
      ex = new TMult( m_Operand1, result, m_Name );
    }
  if( !ex.IsEmpty() ) return ex.Reduce();
  return nullptr;
  }

bool TMult::Positive() const
  {
  return m_Operand1.Positive() && m_Operand2.Positive();
  }

QVector<MathExpr> TMult::GetOperands() const
  {
  QVector<MathExpr> Result;
  const TMult *pOper = CastConst(TMult, m_Operand1);
  if(pOper == nullptr)
    Result.append(m_Operand1);
  else
    Result.append(pOper->GetOperands());
  pOper = CastConst(TMult, m_Operand2);
  if(pOper == nullptr)
    Result.append(m_Operand2);
  else
    Result.append(pOper->GetOperands());
  return Result;
  }

TDivi::TDivi( const MathExpr& ex1, const MathExpr& ex2, bool AsFrac, char Name ) : TOper( ex1, ex2, Name ), m_AsFrac( AsFrac )
  {
  if( ex1.AsFraction() || ex2.AsFraction() )
    m_AsFrac = false;
  double Value2;
  if( ex2.Constan( Value2 ) && abs( Value2 ) < 1.0E-10 )
    {
    s_CheckError = true;
    m_Operand2 = new TConstant( 1.0E-10 );
    }
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TDivi::Clone() const
  {
  int	linom, liden;
  if( m_Operand1.Cons_int( linom ) && m_Operand2.Cons_int( liden ) )
    return GenerateFraction( linom, liden );
  return new TDivi( m_Operand1.Clone(), m_Operand2.Clone(), m_AsFrac );
  }

MathExpr TDivi::Reduce() const
  {
  enum ExpressionType{ constant, power, expression };

  MathExpr opr1, opr2, exNom, pNom, exDenom, pDenom;
  QByteArray VarName;
  bool bResult;
  static MathExpr OldDiviExpr;
  static bool IsCancellation;

  auto ReplaceNom = [&] ( MathExpr ex )
    {
    if( exNom == pNom )
      {
      if( exNom == opr1 )
        opr1 = ex;
      exNom = ex;
      pNom = exNom;
      }
    else
      if( IsConstType( TMult, opr1 ) )
        if( CastPtr( TMult, opr1 )->m_Operand1 == pNom )
          CastPtr( TMult, opr1 )->m_Operand1 = ex;
        else
          CastPtr( TMult, opr1 )->m_Operand2 = ex;
      else
        opr1 = ex;
    };

  auto ReplaceDenom = [&] ( MathExpr ex )
    {
    if( exDenom == pDenom )
      {
      if( exDenom == opr2 )
        opr2 = ex;
      exDenom = ex;
      pDenom = exDenom;
      }
    else
      if( IsConstType( TMult, opr2 ) )
        if( CastPtr( TMult, opr2 )->m_Operand1 == pDenom )
          CastPtr( TMult, opr2 )->m_Operand1 = ex;
        else
          CastPtr( TMult, opr2 )->m_Operand2 = ex;
      else
        opr2 = ex;
    };

  std::function<MathExpr( MathExpr& )> ExpToPolynom = [&] ( MathExpr ex )
    {
    MathExpr  exLeft, exRight, exArg, result;
    uchar cOper;
    QByteArray sName;
    TOper *pExOper;
    if( ex.Funct( sName, exArg ) && sName == "exp" )
      {
      if( exArg.Variab( sName ) && sName == s_GlobalVarName )
        return Variable( VarName );
      if( exArg.Multp( exLeft, exRight ) && IsConstType( TConstant, exLeft ) && exRight.Variab( sName ) && sName == s_GlobalVarName )
        return Variable( VarName ) ^ exLeft;
      bResult = false;
      }
    result = ex;
    if( !bResult )
      return result;
    if( result.Oper_( cOper, exLeft, exRight ) )
      {
      pExOper = CastPtr( TOper, result );
      if( cOper == '+' || cOper == '-' )
        {
        exArg = pExOper->m_Operand1;
        pExOper->m_Operand1 = ExpToPolynom( exArg );
        exArg = pExOper->m_Operand2;
        pExOper->m_Operand2 = ExpToPolynom( exArg );
        return result;
        }
      if( cOper == '*' && IsConstType( TConstant, exLeft ) )
        {
        exArg = pExOper->m_Operand2;
        pExOper->m_Operand2 = ExpToPolynom( exArg );
        return result;
        }
      bResult = false;
      return result;
      }
    if( !( IsConstType( TConstant, ex ) ) )
      bResult = false;
    return  result;
    };

  std::function<MathExpr( MathExpr& )> PolynomToExp = [&] ( MathExpr ex )
    {
    MathExpr  exLeft, exRight, exArg;
    uchar cOper;
    TOper *pExOper;

    if( ex.Oper_( cOper, exLeft, exRight ) )
      {
      if( cOper == '^' )
        return Function( "exp", exRight * Variable( s_GlobalVarName ) );
      pExOper = CastPtr( TOper, ex );
      exArg = pExOper->m_Operand1;
      pExOper->m_Operand1 = PolynomToExp( exArg );
      exArg = pExOper->m_Operand2;
      pExOper->m_Operand2 = PolynomToExp( exArg );
      return MathExpr( pExOper );
      }
    if( IsConstType( TVariable, ex ) )
      return Function( "exp", Variable( s_GlobalVarName ) );
    return ex;
    };

  double Value1 = 1, Value2 = 1, Value3 = 1;

  if( Eq( OldDiviExpr ) )
    {
    OldDiviExpr.Clear();
    return Ethis;
    }

  int N2, D2;
  int N1, D1, NR, DR;
  if( m_Operand1.SimpleFrac_( N1, D1 ) && m_Operand2.SimpleFrac_( N2, D2 ) )
    {
    DivFrac( N1, D1, N2, D2, NR, DR );
    if( s_GlobalInvalid )
      if( NR == 1 )
        return Constant( ( double ) N1 * D2 / ( D1 * N2 ) );
      else
        return Ethis;
    else
      return GenerateFraction( NR, DR ).Reduce();
    }

  MathExpr Op11, Op12, Op21, Op22;

  if( m_Operand2.Root_( Op11, Op22, NR ) )
    if( Op11.SimpleFrac_( N1, D1 ) )
      return ( m_Operand1 *  Expand( GenerateFraction( D1, N1 ).Reduce().Root( NR ) ) );

  if( m_Operand2.Multp( Op11, Op12 ) )
    if( Op11.Root_( Op21, Op22, NR ) && Op21.SimpleFrac_( N1, D1 ) )
      return Expand( m_Operand1 * GenerateFraction( D1, N1 ).Reduce().Root( NR ) / Op12 );
    else if( Op12.Root_( Op21, Op22, NR ) && Op21.SimpleFrac_( N1, D1 ) )
      return Expand( m_Operand1 * GenerateFraction( D1, N1 ).Reduce().Root( NR ) / Op11 );

  opr1 = m_Operand1.Reduce(true);
  opr2 = m_Operand2.Reduce(true);

  if( sm_FullReduce && opr1.Constan( Value1 ) && opr2.Constan( Value2 ) )
    {
    s_GlobalInvalid = false;
    double A2 = fabs(Value2);
    if( A2 > sm_Precision )
      return Constant( Value1 / Value2 );
    double A1 = fabs(Value1);
    s_GlobalInvalid = true;
    s_LastError="INFVAL";
    if(A1 < sm_Precision)
       return MathExpr( new TConstant(1, true) );
    return Ethis;
    }

  if( s_FractionToPower )
    return opr1 * ( opr2 ^ -1 );

  if( opr1.Complex( Op11, Op12 ) && opr2.Complex( Op21, Op22 ) )
    {
    MathExpr Sq = ( Op21 ^ 2 ) + ( Op22 ^ 2 );
    return CreateComplex( ( Op11 * Op21 + Op12 * Op22 ) / Sq, ( Op12 * Op21 - Op11 * Op22 ) / Sq ).Reduce();
    }

  if( opr1.Complex( Op11, Op12 ) && !opr2.HasComplex() )
    return CreateComplex( Op11 / opr2, Op12 / opr2 ).Reduce();

  if( !opr1.HasComplex() && opr2.Complex( Op21, Op22 ) )
    {
    MathExpr Sq = ( Op21 ^ 2 ) + ( Op22 ^ 2 );
    return CreateComplex( opr1 * Op21 / Sq, opr1 * ( -Op22 ) / Sq ).Reduce();
    }

  if( opr1.Unarminus( Op11 ) && opr2.Unarminus( Op12 ) )
    {
    opr1 = Op11;
    opr2 = Op12;
    }

  if( opr1.Measur_( Op11, Op12 ) && ( opr2.Constan( Value1 ) || opr2.Variab( VarName ) ) )
    return new TMeaExpr( ( Op11 / opr2 ).Reduce(), Op12 );

  if( opr2.Measur_( Op21, Op22 ) && ( opr1.Constan( Value1 ) || opr1.Variab( VarName ) ) )
    return new TMeaExpr( ( opr1 / Op21 ).Reduce(), ( Constant( 1 ) / Op22 ).Reduce() );

  if( opr1.Measur_( Op11, Op12 ) && opr2.Measur_( Op21, Op22 ) )
    {
    MathExpr P = Op12 / Op22;
    ReduceMeasure( P );
    if( P.Constan( Value3 ) && Value3 == 1 )
      return ( Op11 / Op21 ).Reduce();
    return new TMeaExpr( ( Op11 / Op21 ).Reduce(), P );
    }

  if( opr1 == 0 )
    return Constant( 0 );

  if( opr1 == 1 && opr2.Power( Op21, Op22 ) && s_RootToPower && !s_PowerToFraction )
    return ( Op21 ^ ( -Op22 ) ).Reduce();

  if( opr1.Equal( opr2 ) )
    return Constant( 1 );

  if( opr1.Unarminus( Op11 ) || NegatConst( opr1 ) )
    {
    OldDiviExpr = Ethis;
    MathExpr P = ( -( (-opr1).Reduce() / opr2 ) ).Reduce();
    OldDiviExpr.Clear();
    return P;
    }

  if( opr2.Unarminus( Op21 ) || NegatConst( opr2 ) )
    if( opr2 == -1 )
      return -opr1;
    else
      return ( -( opr1 / (-opr2).Reduce() ) ).Reduce();

  if( opr1.Unapm_( Op11 ) )
    return ( -( Op11 / opr2 ) ).Reduce();

  if( opr2.Unapm_( Op21 ) )
    return ( -( opr1 / Op21 ) ).Reduce();

  if( opr1.Cons_int( N1 ) && opr2.Cons_int( D1 ) )
    {
    if( D1 == 0 )
      {
      Error_m( X_Str( "MDivisBy0", "Division by 0!" ) );
      s_GlobalInvalid = true;
      return Ethis;
      }
    CancFrac( N1, D1 );
    if( abs( D1 ) == 1 )
      return Constant( N1 * D1 );
    if( N1 == 0 )
      return Constant( 0 );
//    if( TConstant::sm_ConstToFraction && abs( D1 ) < 40001 )
      if( !TConstant::sm_FullReduce && abs( D1 ) < 40001 )
      return GenerateFraction( N1, D1 );
    if( Frac( N1 / ( double ) D1 ) > 0.000001 && s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MFracToReal", "Convert to real" ) );
    return Constant( N1 / ( double ) D1 );
    }

  if( opr2.Constan( Value2 ) )
    {
    MathExpr P;
    if( Value2 == 0 )
      {
      Error_m( X_Str( "MDivisBy0", "Division by 0!" ) );
      s_GlobalInvalid = true;
      return Ethis;
      }
    else
      if( Value2 == 1 )
        P = opr1.Reduce();
      else if( Value2 == -1 )
        P = -( opr1.Reduce() );
      else if( opr1.Constan( Value1 ) )
        P = Constant( Value1 / Value2 );
      if( !P.IsEmpty() )
        {
        if( ( Value1 > 10000000 || Value2 > 40000 ) && s_ShowDiviMessages && !s_CalcOnly )
          Info_m( X_Str( "MFracToReal", "Convert to real" ) );
        return P.Reduce();
        }
    }

  int Int;
  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Cons_int( Int ) )
    return GenerateFraction( N1, D1 * Int ).Reduce();

  if( opr2.SimpleFrac_( N1, D1 ) && opr1.Cons_int( Int ) )
    {
    Value1 = D1 * Int;
    if( Value1 > INT_MAX )
      return Constant( Value1 / N1 );
    return GenerateFraction( D1 * Int, N1 ).Reduce();
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Constan( Value2 ) )
    return Constant( N1 / ( D1 * Value2 ) ).Reduce();

  if( opr2.SimpleFrac_( N1, D1 ) && opr1.Constan( Value2 ) )
    return Constant( ( Value2 * D1 ) / N1 ).Reduce();

  if( opr2.Divis( Op21, Op22 ) )
    return ( opr1 * ( Op22 / Op21 ) ).Reduce();

  int  Nomin2, Denom2;

  if( opr2.SimpleFrac_( Nomin2, Denom2 ) )
    return  ( opr1 * GenerateFraction( Denom2, Nomin2 ) ).Reduce();

  if( opr1.Divis( Op21, Op22 ) && opr2.Constan( Value3 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return Constant( Value1 / Value2 / Value3 );
    }

  if( opr2.Divis( Op21, Op22 ) && opr1.Constan( Value3 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return Constant( Value3 / Value1 / Value2 );
    }

  if( opr1.Divis( Op21, Op22 ) && opr2.SimpleFrac_( N1, D1 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return Constant( ( D1 / ( double ) N1 ) * ( Value1 / Value2 ) );
    }

  if( opr2.Divis( Op21, Op22 ) && opr1.SimpleFrac_( N1, D1 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    {
    if( s_ShowDiviMessages && !s_CalcOnly )
      Info_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    return Constant( ( N1 / ( double ) D1 ) * ( Value2 / Value1 ) );
    }

  if( opr2.Cons_int( D1 ) )
    return ( GenerateFraction( 1, D1 ) * opr1 ).Reduce();

  if( opr2.Constan( Value1 ) )
    return ( Constant( 1 / Value1 ) * opr1 ).Reduce();

  if( opr1.Power( Op11, Op12 ) )
    if( CastPtr( TPowr, opr1 )->EqualBase( opr2 ) )
      return ( opr2 ^ ( Op12.Reduce() - 1 ) ).Reduce();

  if( opr2.Power( Op21, Op22 ) )
    {
    if( Op21.Equal( opr1 ) )
      return ( Constant( 1 ) / (opr1 ^ ( Op22.Reduce() - 1 ) ) ).Reduce();

    if( Op22.Cons_int( D1 ) && D1 < 0 )
      return ( opr1 *  ( Op21 ^ abs( D1 ) ) ).Reduce();
    }

  if( opr1.Power( Op11, Op12 ) && opr2.Power( Op21, Op22 ) )
    {
    if( Op11.Equal( Op21 ) )
      return ( Op11 ^ ( Op12 - Op22 ) ).Reduce();

    if( Op12.Equal( Op22 ) )
      if( opr1.Root_( Op11, Op12, N1 ) )
        {
        MathExpr d = Op11 / Op21;
        MathExpr r = d.Root(N1);
        return r.Reduce();
//        return ( Op11 / Op21 ).Root( N1 ).Reduce();
        }
      else
        return ( ( Op11 / Op21 ) ^ Op12 ).Reduce();

    MathExpr exLeft, exRight;
    uchar cOper;
    if( Op11.Cons_int( N1 ) && Op21.Cons_int( N2 ) && N2 % N1 == 0 && Op12.Oper_( cOper, exLeft, exRight ) )
      {
      N1 = N2 / N1;
      if( cOper == '+' )
        {
        if( exLeft.Eq( Op22 ) )
          if( N1 == 1 )
            return Op11 ^ exRight;
          else
            return ( Op11 ^ exRight ) / ( Constant( N1 ) ^ exLeft );
        if( exRight.Eq( Op22 ) )
          if( N1 == 1 )
            return Op11 ^ exLeft;
          else
            return ( Op11 ^ exLeft ) / ( Constant( N1 ) ^ exRight );
        }
      else if( cOper == '-' && exLeft.Eq( Op22 ) )
        if( N1 == 1 )
          return Op11 ^ ( -exRight );
        else
          return ( Op11 ^ ( -exRight ) ) / ( Constant( N1 ) ^ exLeft );
      }
    }

  if( opr1.Multp( Op11, Op12 ) )
    {
    if( Op11.Equal( opr2 ) )
      if( s_EquationSolving && !Op11.HasUnknown( s_GlobalVarName ).isEmpty() )
        return opr1 / opr2;
      else
        return Op12.Reduce();

    if( Op12.Equal( opr2 ) )
      if( s_EquationSolving && !Op12.HasUnknown( s_GlobalVarName ).isEmpty() )
        return opr1 / opr2;
      else
        return Op11.Reduce();

    if( opr2.Multp( Op21, Op22 ) )
      {
      if( Op11.Equal( Op21 ) )
        if( s_EquationSolving && !Op11.HasUnknown( s_GlobalVarName ).isEmpty() )
          return opr1 / opr2;
        else
          return ( Op12.Reduce() / Op22.Reduce() ).Reduce();

      if( Op11.Equal( Op22 ) )
        if( s_EquationSolving && !Op11.HasUnknown( s_GlobalVarName ).isEmpty() )
          return opr1 / opr2;
        else
          return ( Op12.Reduce() / Op21.Reduce() ).Reduce();

      if( Op12.Equal( Op21 ) )
        if( s_EquationSolving && !Op12.HasUnknown( s_GlobalVarName ).isEmpty() )
          return opr1 / opr2;
        else
          return ( Op11.Reduce() / Op22.Reduce() ).Reduce();

      if( Op12.Equal( Op22 ) )
        if( s_EquationSolving && !Op12.HasUnknown( s_GlobalVarName ).isEmpty() )
          return opr1 / opr2;
        else
          return ( Op11.Reduce() / Op21.Reduce() ).Reduce();
      }
    }

  if( opr2.Multp( Op21, Op22 ) )
    {
    if( Op21.Equal( opr1 ) )
      if( s_EquationSolving && !Op21.HasUnknown( s_GlobalVarName ).isEmpty() )
        return opr1 / opr2;
      else
        return ( Constant( 1 ) / Op22.Reduce() ).Reduce();

    if( Op22.Equal( opr1 ) )
      if( s_EquationSolving && !Op22.HasUnknown( s_GlobalVarName ).isEmpty() )
        return opr1 / opr2;
      else
        return ( Constant( 1 ) / Op21.Reduce() ).Reduce();
    }

  if( opr1.Constan( Value1 ) && opr2.Variab( VarName ) && VarName.at(0) == msInfinity )
    return Constant( 0 );

  if( opr1.Divis( Op11, Op12 ) )
    if( opr2.Divis( Op21, Op22 ) )
      return ( ( Op11 * Op22 ) / ( Op12 * Op21 ) ).Reduce();
    else
      return ( Op11 / ( Op12 * opr2 ) ).Reduce();

  QByteArray FunctName;

  if( opr1.Funct( FunctName, Op11 ) && FunctName == "exp" && opr2.Funct( FunctName, Op12 ) && FunctName == "exp" )
    return new TFunc( false, "exp", ( Op11 - Op12 ).Reduce() );

  MathExpr exTmp, P;

  if( !( IsConstType( TMult, opr1 ) ) && !( IsConstType( TMult, opr2 ) ) || IsCancellation )
    {
    MathExpr result = opr1 / opr2;
    if( s_IsIntegral && !s_IsTrinom )
      {
      bResult = true;
      if( s_GlobalVarName[0] == 'z' )
        VarName = "x";
      else
        VarName.fill( s_GlobalVarName[0] + 1, 1 );
      exTmp = ExpToPolynom( opr1 ) / ExpToPolynom( opr2 );
      TSolutionChain::sm_SolutionChain.m_Accumulate = false;
      if( bResult )
        {
        P = exTmp.CancellationOfMultiNominals( opr1 );
        TSolutionChain::sm_SolutionChain.m_Accumulate = !s_CalcOnly;
        if( !P.Eq( exTmp ) )
          return PolynomToExp( P );
        return result;
        }
      P = result.CancellationOfMultiNominals( opr1 );
      TSolutionChain::sm_SolutionChain.m_Accumulate = !s_CalcOnly;
      if( !P.Eq( result ) )
        if( P.Divis( opr1, opr2 ) )
          return Expand( opr1 ) / Expand(opr2);
        else
          return Expand(P);
      }
    return result;
    }

  exNom = opr1;
  exDenom = opr2;
  ExpressionType ExprType;
  do
    {
    if( IsConstType( TMult, opr1 ) )
      if( pNom == CastPtr( TMult, opr1 )->m_Operand1 )
        if( CastPtr( TMult, opr1 )->m_Operand2.Multp( pNom, Op12 ) )
          opr1 = CastPtr( TMult, opr1 )->m_Operand2;
        else
          pNom = CastPtr( TMult, opr1 )->m_Operand2;
      else
        if( pNom == CastPtr( TMult, opr1 )->m_Operand2 )
          break;
        else
          pNom = CastPtr( TMult, opr1 )->m_Operand1;
    else
      if( pNom == opr1 )
        break;
      else
        pNom = opr1;

    if( pNom.Cons_int( NR ) )
      ExprType = constant;
    else if( pNom.Power( Op11, Op12 ) )
      ExprType = power;
    else
      ExprType = expression;

    opr2 = exDenom;
    pDenom.Clear();
    bool EndLoop = false;
    do
      {
      if( IsConstType( TMult, opr2 ) )
        if( pDenom == CastPtr( TMult, opr2 )->m_Operand1 )
          if( CastPtr( TMult, opr2 )->m_Operand2.Multp( pDenom, Op22 ) )
            opr2 = CastPtr( TMult, opr2 )->m_Operand2;
          else
            pDenom = CastPtr( TMult, opr2 )->m_Operand2;
        else
          if( pDenom == CastPtr( TMult, opr2 )->m_Operand2 )
            break;
          else
            pDenom = CastPtr( TMult, opr2 )->m_Operand1;
      else
        if( pDenom == opr2 )
          break;
        else
          pDenom = opr2;
      switch( ExprType )
        {
        case constant:
          if( pDenom.Cons_int( DR ) )
            {
            N1 = NR;
            CancFrac( NR, DR );
            if( N1 != NR )
              {
              ReplaceNom( Constant( NR ) );
              ReplaceDenom( Constant( DR ) );
              }
            EndLoop = true;
            continue;
            }
          break;
        case power:
          if( pDenom.Power( Op21, Op22 ) )
            {
            if( Op11.Equal( Op21 ) )
              {
              if( Op12.Equal( Op22 ) )
                {
                ReplaceNom( Constant( 1 ) );
                ReplaceDenom( Constant( 1 ) );
                }
              else
                {
                P = ReduceTExprs( Op12 - Op22 );
                if( P.Negative() )
                  {
                  ReplaceDenom( Op11 ^ ReduceTExprs( -P ) );
                  ReplaceNom( Constant( 1 ) );
                  }
                else
                  {
                  ReplaceDenom( Constant( 1 ) );
                  ReplaceNom( Op11 ^ P );
                  }
                }
              EndLoop = true;
              continue;
              }
            }
          else
            if( Op11.Equal( pDenom ) )
              {
              if( Op12.Cons_int( N2 ) )
                CastPtr( TConstant, CastPtr( TPowr, pNom )->m_Operand2 )->m_Value = N2 - 1; 
              else
                ReplaceNom( ReduceTExprs( Op12 - 1 ) );
              ReplaceDenom( Constant( 1 ) );
              EndLoop = true;
              continue;
              }
          break;
        case expression:
          if( pNom.Equal( pDenom ) )
            {
            if( s_EquationSolving && !pNom.HasUnknown( s_GlobalVarName ).isEmpty() )
              {
              EndLoop = true;
              continue;
              }
            ReplaceNom( Constant( 1 ) );
            ReplaceDenom( Constant( 1 ) );
            EndLoop = true;
            continue;
            }
          else
            if( pDenom.Power( Op21, Op22 ) && Op21.Equal( pNom ) && Op22.Cons_int( D2 ) )
              {
              CastPtr( TConstant, CastPtr( TPowr, pDenom )->m_Operand2 )->m_Value = D2 - 1;
              ReplaceNom( Constant( 1 ) );
              EndLoop = true;
              continue;
              }
            else
              if( pNom.ConstExpr() && pDenom.ConstExpr() )
                {
                if( pDenom == 1 )
                  {
                  EndLoop = true;
                  continue;
                  }
                P = ( pNom / pDenom ).Reduce();
                if( !( IsConstType( TDivi, P ) ) )
                  {
                  ReplaceNom( P );
                  ReplaceDenom( Constant( 1 ) );
                  EndLoop = true;
                  continue;
                  }
                }
              else
                if( pNom.Funct( FunctName, Op11 ) && FunctName == "exp" && pDenom.Funct( FunctName, Op12 ) && FunctName == "exp" )
                  {
                  P = Op11 - Op12;
                  ReplaceNom( new TFunc( false, "exp", P.Reduce() ) );
                  ReplaceDenom( Constant( 1 ) );
                  EndLoop = true;
                  continue;
                  }
        }
      } while( EndLoop );
    } while( true );

  MathExpr Result = exNom / exDenom;
  IsCancellation = true;
  bool OldSummExpFactorize = s_SummExpFactorize;
  s_SummExpFactorize = false;
  try
    {
    if(!exNom.Eq(m_Operand1) && !exDenom.Eq(m_Operand2))
      Result = Result.Reduce();
    }
  catch( char * )
    {
    }
  s_SummExpFactorize = OldSummExpFactorize;
  IsCancellation = false;
  return Result;
  }

MathExpr TDivi::Perform()
  {
  return new TDivi( m_Operand1.Perform(), m_Operand2.Perform() );
  }


MathExpr TDivi::Diff( const QByteArray& d )
  {
  return new TDivi( new TSubt( new TMult( m_Operand1.Diff( d ), m_Operand2 ),
    new TMult( m_Operand1, m_Operand2.Diff( d ) ) ),
    new TPowr( m_Operand2, new TConstant( 2 ) ) );
  }

MathExpr TDivi::Integral( const QByteArray& d )
  {
  MathExpr rd = m_Operand2.Diff( d ).ReduceTExprs();

  if( rd == 0 )
    return m_Operand1.Integral( d ) / m_Operand2;

  MathExpr op1, op2, opr1;
  int n;
  QByteArray sName;
  rd = m_Operand1.Diff( d ).ReduceTExprs();

  if( rd == 0 )
    {
    rd.Clear();
    if( m_Operand2.Summa( op1, op2 ) )
      {
      if( op2 == 1 )
        m_Operand2.Summa( op2, op1 );

      if( op1 == 1 && op2.Funct( sName, op1 ) && op1.HasUnknown( d ) == d )
        {
        if( sName == "sin" )
          rd = m_Operand1 / ( Constant( 2 ) * ( Function( "cos", Variable( msPi ) / 4 - op1 / 2 ) ^ 2 ) );
        else
          if( sName == "cos" )
            rd = m_Operand1 / (Constant( 2 ) * ( Function( "cos", op1 /  2 ) ^ 2 ) );
        }
      }
    if( m_Operand2.Subtr( op1, op2 ) )
      {
      opr1 = m_Operand1;
      if( op2 == 1 )
        {
        opr1 = -opr1;
        m_Operand2.Subtr( op2, op1 );
        }

      if( op1 == 1 && op2.Funct( sName, op1 ) && sName == "cos" && op1.HasUnknown( d ) == d )
        rd = opr1 / ( Constant( 2 ) * ( Function( "sin", op1 / 2 ) ^ 2 ) );
      }
    if( !rd.IsEmpty() )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( new TIntegral( false, rd, new TVariable( false, d ) ) );
      return rd.Integral( d );
      }
    rd = ( m_Operand2 ^ -1 ).ReduceTExprs();
    return m_Operand1 * rd.Integral( d );
    }

  if( m_Operand2.Multp( op1, op2 ) )
    {
    rd = op1.Diff( d ).ReduceTExprs();
    if( rd == 0 )
      {
      rd = ( m_Operand1 / op2 ).ReduceTExprs();
      return ( Constant( 1 ) / op1 ) * rd.Integral( d );
      }

    rd = op2.Diff( d ).ReduceTExprs();
    if( rd == 0 )
      {
      rd = ( m_Operand1 / op1 ).ReduceTExprs();
      return ( Constant( 1 ) / op2 ) * rd.Integral( d );
      }
    }

  n = Divide2Polinoms( m_Operand1, m_Operand2, op1, op2 );

  if( n == dpOk )
    {
//    TExpr::sm_IntegralError = true;
//    return Ethis;
    rd = ( op1 + op2 / m_Operand2 ).ReduceTExprs();
    return rd.Integral( d );
    }

  opr1 = Expand(m_Operand1);

  if( opr1.Summa( op1, op2 ) )
    {
    op1 = op1 / m_Operand2;
    op2 = op2 / m_Operand2;
    rd = ( op1 + op2 ).ReduceTExprs();
    return rd.Integral( d );
    }

  if( opr1.Subtr( op1, op2 ) )
    {
    op1 = op1 / m_Operand2;
    op2 = op2 / m_Operand2;
    rd = ( op1 - op2 ).ReduceTExprs();
    return rd.Integral( d );
    }

  TSolutionChain::sm_SolutionChain.m_Accumulate = false;
  rd = CancellationOfMultiNominals( op1 );
  TSolutionChain::sm_SolutionChain.m_Accumulate = !s_CalcOnly;
  if( !Eq( rd ) )
    return rd.Integral( d );

  TExpr::sm_IntegralError = true;
  return Ethis;
  }

MathExpr TDivi::Lim(const QByteArray& v, const MathExpr& lm) const
  {
  MathExpr  op1,op2,op11,op12,op21,op22,rd, Result, This(this->Clone());
  bool nl1,nl2,ng1,ng2;
  int iNomPower, iDenomPower, Deep = 0;
  QByteArray sName ;
  uchar cOper ;

  auto OpenAmbiguity = [&] ( MathExpr operand1, MathExpr operand2 )
    {
    const int MaxDeep = 10;
    MathExpr  df, R;
    s_OpenMultAmbiguity = false;
    Deep++;
    df = ReduceTExprs(operand1.Diff(v)) / ReduceTExprs(operand2.Diff(v));
    if( df.Equal(This) || Deep > MaxDeep )
      if( operand1.Equal(operand2) )
        R = Constant( 1 );
      else
        {
        s_GlobalInvalid = true;
        R = Clone();
        }
    else
      R = ReduceTExprs(df.Reduce().Lim(v,lm));
    Deep--;
    return R;
    };

  auto  GetMaxExponent = [&] (const MathExpr& ex)
    {
    int iMaxExp, iMinExp, iRoot, R;

    std::function<void(const MathExpr& )> GetExponent = [&] (const MathExpr& exp)
      {
      MathExpr exLeft, exRight, exArg ;
      uchar cOper ;
      QByteArray Name ;
      int iVal ;
      if( exp.Unarminus( exArg ) )
        {
        GetExponent( exArg );
        return;
        }
      if( exp.Oper_( cOper, exLeft, exRight ) )
         {
        switch (cOper)
          {
          case '+':
          case '-':
            GetExponent( exLeft );
            GetExponent( exRight );
            break;
          case '*':
            if( IsConstType( TConstant, exLeft ) && ( IsConstType( TVariable, exRight ) || IsConstType( TPowr, exRight ) ) )
              GetExponent( exRight );
            else
              iMinExp = -1000;
            break;
          case '^':
            if( exLeft.Variab( Name ) && Name == v && exRight.Cons_int( iVal ) )
              if( iVal % iRoot != 0 )
                iMinExp = -1000;
              else
                {
                iVal /= iRoot;
                iMaxExp = max( iMaxExp, iVal );
                iMinExp = min( iMinExp, iVal );
                }
            else
              iMinExp = -1000;
            break;
          case '~':
            exp.Root_( exLeft, exRight, iRoot );
            GetExponent( exLeft );
            iRoot = 1;
            break;
          default:
            iMinExp = -1000;
          }
        return;
        }
     if( exp.Variab( Name ) && Name == v )
       {
       if( iRoot == 1 )
         {
         iMaxExp = max( iMaxExp, 1 );
         iMinExp = min( iMinExp, 1 );
         }
       return;
       }
     if( IsConstType( TConstant, exp ) ) return;
     iMinExp = -1000;
    };
   iMaxExp = 0;
   iMinExp = 0;
   iRoot = 1;
   GetExponent( ex );
   R = iMaxExp;
   if( iMinExp < 0 )
     R = 0;
   return R;
   };

 std::function<MathExpr(const MathExpr&, int )> TermwiseDivide = [&] (const MathExpr& ex, int iPower)
  {
  TMultiNominal multi(ex) ;
  int i, iCoeff, iTermPower, Nom, Denom ;
  TTerm Term ;
  MathExpr exTerm, exBase, R ;
  for( i = 0; i <= multi.count() - 1; i++ )
    {
    Term = multi[i];
    iCoeff = Term.GetCoeffInt();
    exBase = Term.TermBase();
    if( exBase.IsEmpty() )
      exBase = Variable( v );
    if( !Term.TermBase().IsEmpty() && Term[0].m_Power.SimpleFrac_( Nom, Denom ) )
      if( iCoeff == 1 )
        exTerm = TermwiseDivide( exBase, iPower * Denom ).Root( Denom );
      else
        exTerm = ExpandExpr( Constant( iCoeff ^ Denom ) * TermwiseDivide( exBase, iPower * Denom ) ).Root( Denom );
    else
      {
      iTermPower = iPower - Term.PowerInt();
      if( iTermPower == 0 )
        exTerm =  Constant( iCoeff );
      else
        if( iTermPower == 1 )
          exTerm = Constant( iCoeff ) / exBase;
        else
          exTerm = Constant( iCoeff ) / (exBase ^ Constant( iTermPower ));
     }
   if( i == 0 )
   if( Term.m_Sign < 0 )
     R = -exTerm;
   else
     R = exTerm;
  else
    if( Term.m_Sign < 0 )
     R -= exTerm;
    else
      R += exTerm;
   }
  return R;
  };

  op1 = m_Operand1.Reduce();
  op2 = m_Operand2.Reduce();
  if( op1.Funct( sName, op11 ) && sName == "log" )
    {
    if( IsConst( lm, 0 ) && op11.Oper_( cOper, op21, op22 ) && IsConst( op21, 1 ) && ( cOper == '+' || cOper == '-' ) )
      {
      rd = op11.Lim( v, lm ).Reduce();
      if( !rd.IsEmpty() && IsConst( rd, 1 ) )
        {
        if( cOper == '-' ) op22 = -op22;
        op21 = op22 / op2;
        rd = op21.Divisor2Polinoms();
        if( rd.Binar( '=', op21, rd ) && rd.ConstExpr() )
          {
          Result =  new TBinar( '=',  new TLimit( false, rd * op1 / op22, Variable(v), lm ), rd );
          return Result;
          }
        }
      }
    rd = op11 ^ ( Constant(1) / m_Operand2);
    Result = rd.Lim( v, lm );
    if( !Result.IsEmpty() )
      {
      while( Result.Binar( '=', op1, Result ) );
        if( Result.Funct( sName, op1 ) && sName == "exp" )
          Result = new TBinar( '=', new TLimit( false, Function( "log", rd ), Variable(v), lm ), op1);
        else
          Result = new TBinar( '=', new TLimit( false, Function( "log", rd ), Variable(v), lm ),
            Function("log", Result ) );
      }
    return Result;
   }
 if( !s_OpenMultAmbiguity )
   if( op1.Divis(op11,op12) )
     if( op2.Divis(op21,op22) )
       {
       rd = op11 * op22 / ( op12 * op21 );
       Result = rd.Lim(v,lm);
       return Result;
       }
     else
      {
      rd = op11 / ( op12 * op2 );
      Result = rd.Lim(v,lm);
      return Result;
      }
   else
     if( m_Operand2.Divis(op21,op22) )
       {
       rd = op1 * op22 / op21;
       Result = rd.Lim(v,lm);
       return Result;
       }
 iNomPower = 0;
 if( IsConstType( TInfinity, lm ) )
   iNomPower = GetMaxExponent( m_Operand1 );
 if( iNomPower > 0 )
   {
   iDenomPower = GetMaxExponent( m_Operand2 );
   if( iDenomPower > 0 )
     {
     op1 = TermwiseDivide( m_Operand1, iNomPower );
     op2 = TermwiseDivide( m_Operand2, iDenomPower );
     rd = op1/op2;
     if( iNomPower > iDenomPower )
       {
       iNomPower -= iDenomPower;
       if( iNomPower == 1 )
         rd = Variable(v) * rd;
       else
         rd = (Variable(v) ^ Constant( iNomPower )) * rd;
       }
     else
       if( iNomPower < iDenomPower )
         {
         iDenomPower -= iNomPower;
         if( iDenomPower == 1 )
           rd = (Constant(1) / Variable(v)) * rd;
         else
           rd = (Constant(1) / (Variable(v)^Constant(iNomPower))) * rd;
         }
     Result = rd.Lim( v, lm );
     if( !Result.IsEmpty() )
       Result =  new TBinar( '=',  new TLimit( false, rd, Variable(v), lm ), Result );
     return Result;
     }
  }
  op1 = m_Operand1.Lim(v,lm);
  if( !op1.IsEmpty() )
    {
    rd = op1.Reduce();
    nl1 = IsConst(rd,0);
    }
  else
    nl1 = false;
  op2 = m_Operand2.Lim(v,lm);
  if( !op2.IsEmpty() )
    {
    rd = op2.Reduce();
    nl2 = IsConst(rd, 0);
    }
  else
    nl2 = false;
  if( op1.IsEmpty() )
    {
    if( op2.Infinity(ng2) )
      Result = Constant(0);
    else
      Result.Clear();
    return Result;
    }
  if( op2.IsEmpty() )
    {
    s_GlobalInvalid = true;
    Result = Clone();
    return Result;
    }
  if( op1.Infinity(ng1) )
    if( op2.Infinity(ng2) )
      Result = OpenAmbiguity(m_Operand1,m_Operand2);
    else
      Result= new TInfinity(ng1 ^ op2.Negative());
  else
    if( op2.Infinity(ng2) )
      Result = Constant( 0 );
    else
      if( nl2 )
        if( nl1 )
          Result = OpenAmbiguity(m_Operand1, m_Operand2);
        else
          Result = new TInfinity(op1.Negative());
      else
        Result = op1 / op2;
  return Result;
  }

  bool TDivi::Eq( const MathExpr& E2 ) const
  {
  MathExpr	op21, op22;
  if( ( !E2.IsEmpty() ) && E2.Divis( op21, op22 ) )
    return ( m_Operand1.Eq( op21 ) && m_Operand2.Eq( op22 ) );
  return false;
  }

  bool TDivi::Equal( const MathExpr& E2 ) const
    {
    MathExpr  Ar1, Ar2, op21, op22, op11, op12;
    int N, N1, N2, D1, D2;
    int rt1, rt2;
    QByteArray _eqName;

    if( m_Operand1.Unapm_( op11 ) )
      {
      if( m_Operand2.Unapm_( op12 ) )
        Ar1 = new TUnapm( op11 / op12 );
      else
        Ar1 = new TUnapm( op11 / m_Operand2 );
      return Ar1.Equal( E2 );
      }

    if( m_Operand2.Unapm_( op12 ) )
      {
      if( m_Operand1.Unapm_( op11 ) )
        Ar1 = new TUnapm( op11 / op12 );
      else
        Ar1 = new TUnapm( m_Operand1 / op12 );
      return Ar1.Equal( E2 );
      }

    if( E2.Divis( op21, op22 ) )
      {
      if( m_Operand1.Equal( op21 ) && m_Operand2.Equal( op22 ) )
        return true;
      if( m_Operand1.SimpleFrac_( N1, D1 ) || m_Operand2.SimpleFrac_( N1, D1 ) || op21.SimpleFrac_( N1, D1 ) || op22.SimpleFrac_( N1, D1 ) ||
        m_Operand1.MixedFrac_( N, N1, D1 ) || m_Operand2.MixedFrac_( N, N1, D1 ) || op21.MixedFrac_( N, N1, D1 ) || op22.MixedFrac_( N, N1, D1 ) )
        {
        static bool Recursion = false;
        bool Result = false;
        if( !Recursion )
          {
          Recursion = true;
          Result = Reduce().Equal( E2.Reduce() );
          Recursion = false;
          }
        return Result;
        }
      Ar1 = m_Operand1.FactList();
      Ar2 = op21.FactList();

      if( Ar1.Equal( Ar2 ) )
        {
        Ar1 = m_Operand2.FactList();
        Ar2 = op22.FactList();
        return Ar1.Equal( Ar2 );
        }
      return false;
      }

    if( ( m_Operand1.Cons_int( N ) && N == 1 && E2.Multp( op21, op22 ) ) )
      {
      Ar1 = FactList();
      Ar2 = E2.FactList();
      return Ar1.Equal( Ar2 );
      }

    if( E2.Unarminus( op11 ) && op11.SimpleFrac_( N1, D1 ) && m_Operand1.Cons_int( N2 ) && m_Operand2.Cons_int( D2 ) )
      return N2*D1 == -N1*D2;

    if( E2.SimpleFrac_( N1, D1 ) && m_Operand1.Cons_int( N2 ) && m_Operand2.Cons_int( D2 ) )
      return N1 == N2 && D1 == D2;

    if( m_Operand1.Root_( op11, op12, rt1 ) && m_Operand2.Root_( op21, op22, rt2 ) && rt1 == rt2 )
      {
      if( op11.Cons_int( N1 ) && op21.Cons_int( N2 ) )
        Ar1 = GenerateFraction( N1, N2 ).Root( rt1 );
      else
        Ar1 = ( op11 / op21 ).Root( rt1 );
      return Ar1.Equal( E2 );
      }

    if( m_Operand1.Power( op11, op12 ) && m_Operand2.Power( op21, op22 ) && op12.Eq( op22 ) )
      {
      if( op11.Cons_int( N1 ) && op21.Cons_int( N2 ) )
        Ar1 = GenerateFraction( N1, N2 ) ^ op12;
      else
        Ar1 = ( op11 / op21 ) ^ op12;
      return Ar1.Equal( E2 );
      }

    if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
      return Equal( Ar2 );

    if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
      return Equal( Ar2 );

    if( E2.Power( op11, op12 ) && op12.Negative() && m_Operand1 == 1 )
      {
      bool Result = op12 == -1 && m_Operand2.Equal( op11 );
      if( Result || !m_Operand2.Power( op21, op22 ) || !op21.Equal( op11 ) ) return Result;
      Ar1 = m_Operand2.RetNeg();
      return Ar1.Equal( op12 );
      }
    return false;
    }

QByteArray TDivi::WriteE() const
  {
  QByteArray S;
  if( !m_Operand1.IsEmpty() )
    S = "((" + m_Operand1.WriteE() + ")" + m_Name;
  else
    S = "(???)" + m_Name;
  if( !m_Operand2.IsEmpty() )
    S += "(" + m_Operand2.WriteE() + "))";
  else
    S += "(???)";
  return S;
  }

QByteArray TDivi::SWrite() const
  {
  if( m_Name != ':' ) 
     return "\\frac{" + m_Operand1.SWrite() + "}{" + m_Operand2.SWrite() + '}';
  MathExpr Op1, Op2;
  QByteArray Name(charToTex(m_Name));
  if (Name[0] == '\\') Name += '\n';
  uchar Op;
  double Val;
  QByteArray Left = m_Operand1.SWrite();
  if (m_Operand1.MustBracketed() == brOperation) Left = '(' + Left + ')';
  if ((IsConstType(TUnar, m_Operand2) || IsConstType(TUnapm, m_Operand2) ||
    (m_Operand2.Constan(Val) && Val < 0.0) ||
    m_Operand2.MustBracketed() == brOperation || (m_Operand2.Oper_(Op, Op1, Op2) &&
    (IsConstType(TUnapm, Op1) || Op1.Negative()))))
    return Left + Name + '(' + m_Operand2.SWrite() + ')';
  return '(' + Left + Name + m_Operand2.SWrite() +')';
  }

MathExpr TDivi::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TDivi( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

bool TDivi::AsFraction()
  {
  return m_AsFrac;
  }

bool TDivi::IsLinear() const
  {
  return m_Operand1.IsLinear() && m_Operand2.IsNumerical();
  }

MathExpr TDivi::TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower )
  {
  MathExpr exNom, exDenom, ex;
  if( sName == "tan" )
    {
    exNom = m_Operand1.TrigTerm( QByteArray( "sin" ), exArg, exPower );
    if( !exNom.IsEmpty() )
      {
      exDenom = m_Operand2.TrigTerm( QByteArray( "cos" ), exArg, exPower );
      if( !exDenom.IsEmpty() )
        {
        ex = new TDivi( exNom, exDenom );
        return ex.Reduce();
        }
      }
    return nullptr;
    }
  if( sName == "cot" )
    {
    exNom = m_Operand1.TrigTerm( QByteArray( "cos" ), exArg, exPower );
    if( !exNom.IsEmpty() )
      {
      exDenom = m_Operand2.TrigTerm( QByteArray( "sin" ), exArg, exPower );
      if( !exDenom.IsEmpty() )
        {
        ex = new TDivi( exNom, exDenom );
        return ex.Reduce();
        }
      }
    }
  return nullptr;
  }

bool TDivi::Positive() const
  {
  return m_Operand1.Positive() && m_Operand2.Positive();
  }

bool TDivi::Negative() const
  {
  return m_Operand1.Negative() && !m_Operand2.Negative() || m_Operand2.Negative() && !m_Operand1.Negative();
  }

MathExpr MathExpr::operator + ( double V ) const 
  {
  TComplexExpr *pCE = dynamic_cast< TComplexExpr* >( m_pExpr );
  double Val;
  if( pCE == nullptr || !pCE->m_Re.Constan(Val) || Val != 0 ) return new TSumm( m_pExpr, new TConstant( V ) );
  return new TComplexExpr( new TConstant( V ), pCE->m_Im );
  }

MathExpr MathExpr::operator - ( const MathExpr& E ) const 
  { 
  TComplexExpr *pCE = dynamic_cast< TComplexExpr* >( E.m_pExpr );
  TComplexExpr *pC = dynamic_cast< TComplexExpr* >( m_pExpr );
  double Val;
  auto CanRe = [] ( TExpr *pE )
    {
    return dynamic_cast< TConstant* >( pE ) != nullptr || dynamic_cast< TVariable* >( pE ) != nullptr ||
      dynamic_cast< TSimpleFrac* >( pE ) != nullptr || dynamic_cast< TMixedFrac* >( pE ) != nullptr;
    };

  if( pC == nullptr )
    {
    if( pCE == nullptr || !pCE->m_Re.Constan( Val ) || Val != 0 || !CanRe( m_pExpr ) ) return new TSubt( m_pExpr, E );
    return new TComplexExpr( *this, (-pCE->m_Im).Reduce() );
    }

  if( pCE == nullptr )
    {
    if( !pC->m_Re.Constan( Val ) || Val != 0 || !CanRe( E.m_pExpr ) ) return new TSubt( m_pExpr, E );
    return new TComplexExpr( (-E).Reduce(), pC->m_Im );
    }

  if( pC->m_Re.Constan( Val ) && Val == 0 ) return new TComplexExpr( ( -pCE->m_Re ).Reduce(), new TSubt( pC->m_Im, pCE->m_Im ) );
  if( pCE->m_Re.Constan( Val ) && Val == 0 ) return new TComplexExpr( pC->m_Re, new TSubt( pC->m_Im, pCE->m_Im ) );

  return new TSubt( m_pExpr, E ); 
  }

MathExpr MathExpr::operator - ( double V ) const 
  { 
  TComplexExpr *pCE = dynamic_cast< TComplexExpr* >( m_pExpr );
  double Val;
  if( pCE == nullptr || !pCE->m_Re.Constan( Val ) || Val != 0 ) return new TSubt( m_pExpr, new TConstant( V ) );
  return new TComplexExpr( new TConstant( -V ), pCE->m_Im );
  }

MathExpr& MathExpr::operator += ( const MathExpr& E )
  {
  return *this = ( *this + E );
  }

MathExpr& MathExpr::operator -= ( const MathExpr& E )
  {
  return *this = ( *this - E );
  }

MathExpr MathExpr::operator + ( const MathExpr& E ) const 
  { 
  TComplexExpr *pCE = dynamic_cast< TComplexExpr* >( E.m_pExpr );
  TComplexExpr *pC = dynamic_cast< TComplexExpr* >( m_pExpr );
  double Val;
  auto CanRe = [] ( TExpr *pE )
    {
    return dynamic_cast< TConstant* >( pE ) != nullptr || dynamic_cast< TVariable* >( pE ) != nullptr || 
      dynamic_cast< TSimpleFrac* >( pE ) != nullptr || dynamic_cast< TMixedFrac* >( pE ) != nullptr;
    };
  
  if( pC == nullptr )
    {
    if( pCE == nullptr || !pCE->m_Re.Constan( Val ) || Val != 0 || !CanRe( m_pExpr) ) return new TSumm( m_pExpr, E );
    return new TComplexExpr( *this, pCE->m_Im );
    }

  if( pCE == nullptr )
    {
    if( !pC->m_Re.Constan( Val ) || Val != 0 || !CanRe( E.m_pExpr ) ) return new TSumm( m_pExpr, E );
    return new TComplexExpr( E, pC->m_Im );
    }

  if( pC->m_Re.Constan( Val ) && Val == 0 ) return new TComplexExpr( pCE->m_Re, new TSumm( pC->m_Im, pCE->m_Im ) );
  if( pCE->m_Re.Constan( Val ) && Val == 0 ) return new TComplexExpr( pC->m_Re, new TSumm( pC->m_Im, pCE->m_Im ) );

  return new TSumm( m_pExpr, E );
  }

MathExpr MathExpr::operator * ( const MathExpr& E )  const
  {
  if (*this == 0 || E == 0) return Constant(0);
/*
  TComplexExpr *pCE = dynamic_cast< TComplexExpr* >( E.m_pExpr );
  TComplexExpr *pC = dynamic_cast< TComplexExpr* >( m_pExpr );
  double Val;

  if( pC == nullptr )
    {
    if( pCE == nullptr || !pCE->m_Re.Constan( Val ) || Val != 0 ) return new TMult( m_pExpr, E );
    if( pCE->m_Im == 1 )  return new TMult( m_pExpr, CreateComplex( 0, 1 ) );
    return new TMult( new TMult( m_pExpr, pCE->m_Im ), CreateComplex( 0, 1 ) );
    }

  if( pCE == nullptr )
    {
    if( !pC->m_Re.Constan( Val ) || Val != 0 ) return new TMult( m_pExpr, E );
    if( pC->m_Im == 1 )  return new TMult( E.m_pExpr, CreateComplex( 0, 1 ) );
    return new TMult( new TMult( pC->m_Im, E.m_pExpr ), CreateComplex( 0, 1 ) );
    }
    */
  return new TMult( m_pExpr, E );
  }

MathExpr& MathExpr::operator *= ( const MathExpr& E )
  {
  return *this = ( *this * E );
  }

MathExpr& MathExpr::operator /= ( const MathExpr& E )
  {
  TExpr *pNew = new TDivi( m_pExpr, E );
  if( --m_pExpr->m_Counter == 0 ) delete m_pExpr;
  m_pExpr = pNew;
  m_pExpr->m_Counter++;
  return *this;
  }

/*
MathExpr TDiviEv::Clone() const
  {
  return new TDiviEv( m_Operand1.Clone(), m_Operand2.Clone() );
  }

MathExpr TDiviEv::Reduce() const
  {
  MathExpr opr1 = m_Operand1.Reduce();
  MathExpr opr2 = m_Operand2.Reduce();

  if( opr1.Equal( opr2 ) )
    return Constant( 1 );

  if( opr2.Newline() )
    return opr1;

  if( opr1.Newline() )
    return (Constant( 1 ) | opr2 ).Reduce();

  double Value2;
  int N2, D2;
  if( (opr2.Constan( Value2 ) && Value2 == 0) || (opr2.SimpleFrac_( N2, D2 ) && N2 == 0) )
    {
    Error_m( X_Str( "MDivisBy0", "Division by 0!" ) );
    s_GlobalInvalid = true;
    return Ethis;
    }

  double Value1;
  if( opr1.Constan( Value1 ) && opr2.Constan( Value2 ) )
    return Constant( Value1 / Value2 );

  if( opr2.Constan( Value2 ) && (abs( Value2 ) == 1) )
    if( Value2 == 1 )
      return opr1;
    else
      return (-opr1).Reduce();

  MathExpr Op11;
  if( opr1.Unarminus( Op11 ) || NegatConst( opr1 ) )
    return (-(-opr1 / opr2)).Reduce();

  MathExpr Op21;
  if( opr2.Unarminus( Op21 ) || NegatConst( opr2 ) )
    return (-(opr1 / -opr2)).Reduce();

  if( opr1.Unapm_( Op11 ) )
    return ( Op11 | opr2 ).Unapm().Reduce();

  if( opr2.Unapm_( Op21 ) )
    return ( opr1 | Op21 ).Unapm().Reduce();

  int N1, D1, NR, DR;
  if( opr1.SimpleFrac_( N1, D1 ) && opr2.SimpleFrac_( N2, D2 ) )
    {
    DivFrac( N1, D1, N2, D2, NR, DR );
    if( s_GlobalInvalid )
      if( NR == 1 )
        return Constant( ( double ) N1 * D2 / ( D1 * N2 ) );
      else
        return Ethis;
    else
      return Constant( NR / (double) DR );
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Constan( Value2 ) )
    return Constant( N1 / (D1 * Value2) );

  if( opr1.Constan( Value1 ) && opr2.SimpleFrac_( N2, D2 ) )
    return Constant( (Value1 * D2) / N2 );

  MathExpr Op22;
  if( opr2.Divis( Op21, Op22 ) )
    if( opr2.DiviEvi( Op21, Op22 ) )
      return (opr1 * ( Op22 | Op21 ) ).Reduce();
    else
      return (opr1 * (Op22 / Op21)).Reduce();

  int Nomin2, Denom2;
  if( opr2.SimpleFrac_( Nomin2, Denom2 ) )
    return (opr1 * GenerateFraction( Denom2, Nomin2 )).Reduce();

  double Value3;
  if( opr1.Divis( Op21, Op22 ) && opr2.Constan( Value3 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    return Constant( Value1 / Value2 / Value3 );

  if( opr2.Divis( Op21, Op22 ) && opr1.Constan( Value3 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    return Constant( Value3 / Value1 / Value2 );

  if( opr1.Divis( Op21, Op22 ) && opr2.SimpleFrac_( N1, D1 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    return Constant( (D1 / (double) N1) * (Value1 / Value2) );

  if( opr2.Divis( Op21, Op22 ) && opr1.SimpleFrac_( N1, D1 ) && Op21.Constan( Value1 ) && Op22.Constan( Value2 ) )
    return Constant( (N1 / (double) D1) * (Value2 / Value1) );

  return new TDiviEv( opr1, opr2 );
  }

MathExpr TDiviEv::Perform()
  {
  return new TDiviEv( m_Operand1.Perform(), m_Operand1.Perform() );
  }

MathExpr TDiviEv::Diff( const QByteArray& d )
  {
  return new TDiviEv( new TSubt( new TMult( m_Operand1.Diff( d ), m_Operand2 ), 
    new TMult( m_Operand1, m_Operand2.Diff( d ) ) ), new TPowr( m_Operand2, new TConstant( 2 ) ) );
  }

bool TDiviEv::Eq( const MathExpr& E2 ) const
  {
  MathExpr op21, op22;
  if( E2.DiviEvi( op21, op22 ) )
    return ( m_Operand1.Eq( op21 ) && m_Operand2.Eq( op22 ) );
  return false;
  }

MathExpr TDiviEv::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TDiviEv( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

bool TDiviEv::AsFraction()
  {
  return false;
  }

bool TDiviEv::DiviEvi( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }
  */
