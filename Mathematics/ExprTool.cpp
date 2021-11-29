#include "ExpOb.h"
#include "ExpObjOp.h"
#include "ExpObBin.h"
#include "Parser.h"
#include "MathTool.h"
#include "Factors.h"
#include "SolChain.h"

MathExpr TExpr::Lconcat( const MathExpr& B ) const
  {
  TLexp *pL = new TLexp;
  pL->Appendz( Ethis );
  pL->Appendz( B );
  return pL;
  }

MathExpr TExpr::Divisor()
  {
  MathExpr  Op1, Op2;
  if( Divis( Op1, Op2 ) ) return Op2;
  int N, D;
  if( SimpleFrac_( N, D ) ) return Constant( D );
  return new TFunc( true, "Divisor", Clone() );
  }

MathExpr TExpr::Dividend()
  {
  MathExpr  Op1, Op2;
  if( Divis( Op1, Op2 ) ) return Op1;
  int N, D;
  if( SimpleFrac_( N, D ) ) return Constant( N );
  return new TFunc( true, "Dividend", Clone() );
  }

MathExpr TExpr::GenChainInEq()
  {
  PExMemb  index;
  MathExpr retu;
  double Vprevious;
  bool undefined = !Listex( index );
  if( !undefined )
    {
    MathExpr reduced = index->m_Memb.Reduce();
    double V;
    int N, D;
    undefined = !( reduced.Constan( V ) || reduced.SimpleFrac_( N, D ) );
    if( !undefined )
      {
      if( reduced.SimpleFrac_( N, D ) )
        V = ( double ) N / D;
      retu = index->m_Memb;
      Vprevious = V;
      }
    }

  if( !index.isNull() ) index = index->m_pNext;
  for( ; !index.isNull() && !undefined; index = index->m_pNext )
    {
    MathExpr reduced = index->m_Memb.Reduce();
    double V;
    int N, D;
    if( reduced.Constan( V ) || reduced.SimpleFrac_( N, D ) )
      {
      if( reduced.SimpleFrac_( N, D ) )
        V = (double) N / D;
      if( abs( Vprevious - V ) < 0.00000001 )
        retu = new TBinar( '=', retu, index->m_Memb );
      else
        if( Vprevious < V )
          retu = new TBinar( '<', retu, index->m_Memb );
      else
      if( Vprevious > V )
        retu = new TBinar( '>', retu, index->m_Memb );
      Vprevious = V;
      }
    else
      {
      undefined = true;
      break;
      }
    }

  if( undefined )
    return new TFunc( true, "GenChainInEq", this );
  return retu;
  }

MathExpr TExpr::ChainToLCD()
  {
  uchar  N, Na, Nb;
  MathExpr a, b;
  if( !Binar_( N, a, b ) ) return this;
  MathExpr ret, al, ar, bl, br;
  if( a.Binar_( Na, al, ar ) && b.Binar_( Nb, bl, br ) )
    ret = al.Lconcat( ar ).Lconcat( bl.Lconcat( br ) );
  else
    if( a.Binar_( Na, al, ar ) )
      ret = al.Lconcat( ar ).Lconcat( b );
    else
      if( b.Binar_( Nb, bl, br ) )
        ret = a.Lconcat( bl.Lconcat( br ) );
      else
        ret = a.Lconcat( b );
  return ret.ToLeastCommDenom().GenChainInEq();
  }

MathExpr TExpr::ToLeastCommDenom()
  {
  PExMemb index;
  if( !Listex( index ) )
    {
    uchar N;
    MathExpr a, b;
    if( Binar_( N, a, b ) ) return ChainToLCD();
    return this;
    }

  MathExpr newexp = FindLeastCommDenom();
  int CommonDenom;
  bool undefined = false;
  newexp.Cons_int( CommonDenom );
  Lexp LISTex;
  if( List2ex( index ) )
    LISTex = new TL2exp;
  else
    LISTex = new TLexp;

  for( Listex( index ); !index.isNull(); index = index->m_pNext )
    {
    int Nomin, Denom;
    if( index->m_Memb.SimpleFrac_( Nomin, Denom ) )
      {
      if( ( Nomin == 0 ) )
        newexp = 0;
      else
        {
        int ResNom, ResVirt, Mult1, Mult2;
        LComDen( Nomin, Denom, 1, CommonDenom, ResNom, ResVirt, CommonDenom, Mult1, Mult2 );
        newexp = new TSimpleFrac( ResNom, CommonDenom );
        }
      LISTex.Addexp( newexp );
      LISTex.Last()->m_Visi = index->m_Visi;
      }
    else
      {
      MathExpr a, b;
      if( index->m_Memb.Divis( a, b ) && a.Cons_int( Nomin ) && b.Cons_int( Denom ) && ( abs( Nomin ) > INT_MAX ||
        abs( Denom ) > INT_MAX ) && ( s_MemorySwitch == SWcalculator ) )
        if( !s_CalcOnly ) Info_m( X_Str( "MNumDenGreat", "Too large" ) );
      undefined = true;
      }
    }

  if( !undefined ) return LISTex;
  if( s_MemorySwitch != SWcalculator ) return new TFunc( true, "ToLeastCommDenom", this );
  s_GlobalInvalid = true;
  return this;
  }

MathExpr TExpr::FindLeastCommDenom()
  { 
  PExMemb index;
  if( !Listex( index ) )
    {
    s_GlobalInvalid = true;
    return new TFunc( true, "FindLeastCommDenom", this );
    }

  int CommonDenom = 1;
  for( ; !index.isNull(); index = index->m_pNext )
    {
    int Nomin, Denom, ResNom, ResVirt, Mult1, Mult2;
    if( index->m_Memb.SimpleFrac_( Nomin, Denom ) )
      LComDen( Nomin, Denom, 1, CommonDenom, ResNom, ResVirt, CommonDenom, Mult1, Mult2 );
    }
  return Constant( CommonDenom );
  }

bool TLexp::AcceptMinus()
  {
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    double V;
    if( !index->m_Memb.Constan( V ) ) continue;
    index->m_Memb = Constant( -V );
#ifdef DEBUG_TASK 
    m_Contents = WriteE();
#endif
    return true;
    }
  return false;
  }

MathExpr TExpr::FactList() const
  {
  MathExpr Op1, Op2;
  if( Multp( Op1, Op2 ) )
    return Op1.FactList().Lconcat( Op2.FactList() );
  int N;
  MathExpr Op21, Op22;
  if( Divis( Op1, Op2 ) && Op1.Cons_int( N ) && N == 1 && Op2.Multp( Op21, Op22 ) )
    {
    Op1 = new TDivi( new TConstant( 1 ), Op21 );
    Op2 = new TDivi( new TConstant( 1 ), Op22 );
    return Op1.FactList().Lconcat( Op2.FactList() );
    }
  MathExpr Arg;
  if( Unarminus( Arg ) )
    {
    MathExpr AList = Arg.FactList();
    if( IsType( TMult, Arg ) && CastPtr( TLexp, AList )->AcceptMinus() ) return AList;
    return ( new TConstant( -1 ) )->Lconcat( AList );
    }
  if( Newline() )
    return new TLexp;
  if( Cons_int( N ) && N == 1 )
    return new TLexp;
  double v;
  if( Constan( v ) && v < 0.0 )
    return ( new TConstant( -1 ) )->Lconcat( new TConstant( -v ) );
  return Lconcat( nullptr );
  }

MathExpr TExpr::SimpleBinar() const
  {
  MathExpr  a, b, al, ar, bl, br;
  uchar N, Na, Nb;

  if( !Binar_( N, a, b ) )
    return Ethis;

  if( a.Binar_( Na, al, ar ) && ar.Newline() )
    {
    a = new TBinar( N, al.SimpleBinar(), b.SimpleBinar() );
    return  a.SimpleBinar();
    }

  if( b.Binar_( Nb, bl, br ) && bl.Newline() )
    {
    b = new TBinar( N, a.SimpleBinar(), br.SimpleBinar() );
    return b.SimpleBinar();
    }

  return Ethis;
  }

MathExpr TExpr::ExpandChain( MathExpr& L, MathExpr& R )  const
  {
  MathExpr a, b;
  uchar N;

  if( Binar_( N, a, b ) )
    {
    MathExpr al, ar, bl, br;
    uchar  Na, Nb;
    MathExpr ret;

    if( a.Binar_( Na, al, ar ) && b.Binar_( Nb, bl, br ) )
      {
      ret = a.ExpandChain( al, ar ).Lconcat( b.ExpandChain( bl, br ).Lconcat( new TBinar( N, ar, bl ) ) );
      L = al;
      R = br;
      }
    else
      {
      if( a.Binar_( Na, al, ar ) )
        {
        ret = a.ExpandChain( al, ar ).Lconcat( new TBinar( N, ar, b ) );
        L = al;
        R = b;
        }
      else if( b.Binar_( Nb, bl, br ) )
        {
        ret = b.ExpandChain( bl, br ).Lconcat( new TBinar( N, a, bl ) );
        L = a;
        R = br;
        }
      else
        {
        ret = Ethis;
        L = a;
        R = b;
        };
      }
    return ret;
    }
  return Ethis;
  }

MathExpr TExpr::UnarMin() const
  {
  double  V;
  MathExpr Op;
  int I, N, D;

  if( Unarminus( Op ) )
    return Op;
  if( Constan( V ) )
    return Constant( ( -V ) );
  if( SimpleFrac_( N, D ) )
    return new TSimpleFrac( -N, D );
  if( MixedFrac_( I, N, D ) )
    return new TMixedFrac( -I, -N, D );
  if( Unapm_( Op ) )
    return Ethis;
  return -MathExpr( Ethis );
  }

MathExpr TExpr::UnarPM() const
  {
  double  V;
  MathExpr Op;
  int I, N, D;

  if( Unarminus( Op ) )
    return new TUnapm( Op );

  if( Constan( V ) && ( V < 0 ) )
    return new TUnapm( Constant( -V ) );

  if( SimpleFrac_( N, D ) && Negative() )
    return new TUnapm( new TSimpleFrac( -N, D ) );

  if( MixedFrac_( I, N, D ) && Negative() )
    return new TUnapm( new TMixedFrac( -I, -N, D ) );

  if( Unapm_( Op ) )
    return Ethis;

  return new TUnapm( Ethis );
  }

MathExpr TExpr::SummList() const
  {
  MathExpr  Arg, Op1, Op2;
  MathExpr uOp1, uOp2;

  if( Summa( Op1, Op2 ) )
    return Op1.SummList().Lconcat( Op2.SummList() );

  if( Subtr( Op1, Op2 ) )
    return Op1.SummList().Lconcat( Op2.UnarMin().SummList() );

  if( Binar( msPlusMinus, Op1, Op2 ) )
    return Op1.SummList().Lconcat( Op2.UnarPM().SummList() );

  if( Unarminus( Arg ) )
    {
    if( Arg.Summa( Op1, Op2 ) )
      return Op1.UnarMin().SummList().Lconcat( Op2.UnarMin().SummList() );
    if( Arg.Subtr( Op1, Op2 ) )
      return Op1.UnarMin().SummList().Lconcat( Op2.SummList() );
    if( Arg.Newline() )
      return new TLexp;
    return Arg.UnarMin().Lconcat( nullptr );
    }
  if( Unapm_( Arg ) )
    {
    if( Arg.Newline() )
      return new TLexp;
    return Arg.UnarPM().Lconcat( nullptr );
    }
  if( Newline() )
    return new TLexp;
  return Lconcat( nullptr );
  }

MathExpr TExpr::Fcase()
  {
  PExMemb  pIndex;
  MathExpr C;
  s_GlobalInvalid = false;

  if( Listex( pIndex ) )
    {
    C = pIndex->m_Memb.Perform();
    pIndex = pIndex->m_pNext;

    while( !pIndex.isNull() )
      {
      if( C.Equal( pIndex->m_Memb) )
        {
        if( pIndex->m_pNext.isNull() )
          {
          Error_m( X_Str( "XPExprLowMess", "FCase error!" ) );
          s_GlobalInvalid = true;
          return new TFunc( true, "fcase", this );
          }
        else
          return pIndex->m_pNext->m_Memb;
        }
      pIndex = pIndex->m_pNext;
      if( !pIndex.isNull() )
        pIndex = pIndex->m_pNext;
      }
    }
  return new TFunc( true, "fcase", this );
  }

MathExpr TExpr::RtoDEC()
  {
 /*
  if( Cast( TConstant, this ) ) return MathExpr( this );
  int NOM, DEN;
  if( SimpleFrac_( NOM, DEN ) )
    {
    if( DEN == 0 )
      {
      if( !s_CalcOnly )
        throw ErrParser( X_Str( "MDenomIs0", "Denominator equals 0" ), peNewErr );
      return MathExpr( this );
      }
    return Constant( double( NOM ) / DEN );
    }
  return Reduce();
  */
  int NOM, DEN;
  MathExpr app;
  double V;
  if( SimpleFrac_( NOM, DEN ) )
    {
    if( DEN == 0 )
      {
      if( !s_CalcOnly )
        throw ErrParser( X_Str( "MDenomIs0", "Denominator equals 0" ), peNewErr );
      return MathExpr( this );
      }
    QByteArray StrDEC;
    ConvertComDec( 0, NOM, DEN, V, StrDEC );
    app = Constant( V );
    }
  else
    if( Constan( V ) )
      {
      int Intp, NomP, d, N;
      ConvertDecCom( V, Intp, N, d, NomP, DEN );
      if( Intp == 0 )
        app = new TSimpleFrac( NomP, DEN );
      else
      app = new TMixedFrac( Intp, NomP, DEN );
      }
    else
      return MathExpr(this);
  return app;
  }

MathExpr TExpr::AlgToGeometr( double Scale, bool PutExpr ) const
  {
  MathExpr  ex2 = Reduce();
  MathExpr op1, op2, exr, f, P2, ex1, Result;
  double  b = 0.0, a;
  if( ex2.IsNumericalValue( a ) || ( ex2.Complex( op1, op2 ) && op1.IsNumericalValue( a ) && op2.IsNumericalValue( b ) ) )
    {
    double a2b2 = a * a + b * b;
    double r = int( sqrt( a2b2 ) );
    if( abs( r * r ) - a2b2 < 0.0000001 )
      exr = Constant( r );
    else
      exr = Constant( a2b2 ).Root( 2 );
    if( a != 0.0 )
      if( a >= 0.0 )
        f = Function( "arctan", Constant( b / a ) ).Reduce();
      else
        {
        if( sm_TrigonomSystem == tsRad )
          P2 = Variable( msPi );
        else
          P2 = new TMeaExpr( Constant( 180 ), Variable( msDegree ) );
        f = ( Function( "arctan", Constant( b / a ) ) + P2 ).Reduce();
        }
    else
      {
      if( sm_TrigonomSystem == tsRad )
        f = Variable( msPi ) / Constant( 2 );
      else
        f = new TMeaExpr( Constant( 90 ), Variable( msDegree ) );
      if( b < 0 )
        f = -f;
      }
    if( exr.Constan( r ) && r == 1 )
      ex1 = Function( "cos", f ) + CreateComplex( Constant( 0 ), Constant( 1 ) ) * Function( "sin", f );
    else
      ex1 = exr * ( Function( "cos", f ) + CreateComplex( Constant( 0 ), Constant( 1 ) ) * Function( "sin", f ) );
    if( PutExpr )
      {
      Lexp exResult = new TL2exp;
      exResult.Addexp( new TBinar( '=', ( TExpr* ) this, ex1 ) );
      exResult.First()->m_Visi = false;
      exResult.Addexp( new TTrigoComplex( a, b, Scale ) );
      Result = exResult;
      }
    else
      Result = new TTrigoComplex( a, b, Scale );
    s_GlobalInvalid = false;
    }
  s_MemorySwitch = SWtask;
  return Result;
  }

bool MathExpr::IsNumericalValue( double& V )
  {
  TestPtr();
  auto GetValue = [&] ( TExpr *pExpr )
    {
    if( pExpr->Constan( V ) ) return true;
    int I, N, D;
    if( pExpr->SimpleFrac_( N, D ) )
      {
      V = (double) N / D;
      return true;
      }
    if( pExpr->MixedFrac_( I, N, D ) )
      {
      V = I + ( double ) N / D;
      return true;
      }
    double dN, dD;
    MathExpr OpN, OPD;
    if( !pExpr->DiviEvi( OpN, OPD ) || !OpN.IsNumericalValue( dN ) || !OPD.IsNumericalValue( dD ) ) return false;
    V = dN / dD;
    return true;
    };
  if( GetValue( m_pExpr ) ) return true;
  MathExpr op1;
  if( !m_pExpr->Unarminus( op1 ) || !GetValue( op1.m_pExpr ) ) return false;
  V = -V;
  return true;
  }

MathExpr TExpr::SimplBySummList()
  {
  MathExpr L( SummList() );
  TLexp *pL = dynamic_cast<TLexp*>( L.Ptr() );
  if( pL == nullptr )  throw  ErrParser( "Internal Error!", peSyntacs );
  bool Buildnew = false;
  bool Reduced = true;
  PExMemb pFirst, pIndex1, pIndex2;
  while( Reduced )
    {
    pL->Listex( pFirst );
    pIndex1 = pFirst;
    Reduced = false;
    while( !pIndex1.isNull() && !Reduced )
      {
      pIndex2 = pIndex1->m_pNext;
      if( pIndex1 == pFirst && ( pIndex2.isNull() || pIndex2->m_pNext.isNull() ) ) break;
      while( !pIndex2.isNull() && !Reduced )
        {
        MathExpr SumE = pIndex1->m_Memb + pIndex2->m_Memb;
        MathExpr SumER = SumE.Reduce();
        MathExpr exOp1, exOp2, exOp;
        uchar ch;
        if( !SumER.Summa( exOp1, exOp2 ) && !SumER.Subtr( exOp1, exOp2 ) && !SumER.Binar_( ch, exOp1, exOp2 ) && 
           !( SumER.Unarminus( exOp ) && ( exOp.Summa( exOp1, exOp2 ) || exOp.Subtr( exOp1, exOp2 ) ) ) )
          {
          pL->DeleteMemb( pIndex1 );
          pL->DeleteMemb( pIndex2 );
          double V;
          if( !( SumER.Constan( V ) && ( V == 0.0 ) ) )
            pL->Addexp( SumER );
          Reduced = true;
          Buildnew = true;
          }
        pIndex2 = pIndex2->m_pNext;
        }
      pIndex1 = pIndex1->m_pNext;
      }
    }
  if( !Buildnew ) return Clone().SetReduced();
  MathExpr SumER = pFirst->m_Memb;
  for( pIndex2 = pFirst->m_pNext; !pIndex2.isNull(); pIndex2 = pIndex2->m_pNext )
    SumER = ( SumER + pIndex2->m_Memb ).Reduce();
  return SumER;
  }

MathExpr TExpr::FindGreatestCommDivisor() const
  {
  if( s_MemorySwitch != SWcalculator ) return new TFunc( true, "FindGreatestCommDivisor", Clone() );
  s_GlobalInvalid = true;
  return nullptr;
  }

MathExpr TExpr::CalcFunc( const QByteArray& fname )
  {
  double SavePrec = ResetPrecision( 0.0000000001 );
  MathExpr exo = Parser::StrToExpr( PiVar2PiConst( WriteE() ) ).Reduce();
  if( IsTrigonom( fname ) )
    {
    MathExpr ex, exm;
    QByteArray Name;
    if( exo.Measur_( ex, exm ) && exm.Variab( Name ) && ( Name[0] == ( char ) msDegree ) )
      {
      sm_TrigonomSystem = tsDeg;
      exo = ex.Reduce();
      }
    exo = exo.SimplifyFull();
    }

  int Nom, Den;
  if( exo.SimpleFrac_( Nom, Den ) )
    exo = Constant( Nom / ( double ) Den );

  bool Neg = exo.Unarminus( exo );
  double V;
  MathExpr op1, op2;
  s_LastError = X_Str( "MEnterConst", "Enter constant expression!" );
  if( exo.Constan( V ) || ( exo.Measur_( op1, op2 ) && op1.Constan( V ) ) )
    {
    s_GlobalInvalid = false;
    if( Neg ) V = -V;
    MathExpr exii = this;
    if( IsTrigonom( fname ) && sm_TrigonomSystem == tsDeg && Cast( TMeaExpr, this ) != nullptr )
      exii = new TMeaExpr( this, Variable( msDegree ) );
    MathExpr exp = Parser::StrToExpr( fname + '(' + WriteE() + ')' );
    MathExpr expo = Parser::StrToExpr( fname + '(' + exii.WriteE() + ')' );
    ResetPrecision( SavePrec );
    try
      {
      exp = exp.Reduce();
      }
    catch( ErrParser E )
      {
      s_GlobalInvalid = true;
      s_LastError = E.Message();
      return this;
      }
    if( s_GlobalInvalid ) return exo;
    s_XPStatus.SetMessage( X_Str( "MCalculated", "Calculated!" ) );
    return new TBinar( '=', expo, exp );
    }
  ResetPrecision( SavePrec );
  s_GlobalInvalid = true;
  return this;
  }

MathExpr TLexp::FindGreatestCommDivisor() const
  {
  TLexp *pNewList = new TLexp;
  MathExpr NList( pNewList );
  int iConst;
  for( PExMemb pInd = m_pFirst; !pInd.isNull(); pInd = pInd->m_pNext )
    if( !( pInd->m_Memb.Cons_int( iConst ) && iConst == 0 ) ) pNewList->Addexp( pInd->m_Memb );
  int iCommDivisor;
  PExMemb pIndex = pNewList->m_pFirst;
  if( pIndex.isNull() || !pIndex->m_Memb.Cons_int( iCommDivisor ) ) return TExpr::FindGreatestCommDivisor();
  for( ; !pIndex.isNull(); pIndex = pIndex->m_pNext )
    if( !pIndex->m_Memb.Cons_int( iConst ) || ( ( iConst <= 0 || iConst > 10000000 ) && s_MemorySwitch == SWcalculator ) )
      return TExpr::FindGreatestCommDivisor();
    else
      iCommDivisor = GrComDiv( iConst, iCommDivisor );
  return Constant( iCommDivisor );
  }

MathExpr TLexp::FindLeastCommMult()
  {
  int Nomin, CommonMult = 1;
  for( PExMemb pIndex = m_pFirst; !pIndex.isNull(); pIndex = pIndex->m_pNext )
    if( pIndex->m_Memb.Cons_int( Nomin ) )
      CommonMult = LComMul( Nomin, CommonMult );
    else
      {
      s_GlobalInvalid = true;
      return this;
      }
  return Constant( CommonMult );
  }

QByteArray TExpr::HasUnknown( const QByteArray& Name ) const
  {
  SimpleChar Result;
  double V;
  int N, D;
  if( Constan( V ) || SimpleFrac_( N, D ) ) return Result;
  QByteArray sV;
  SimpleChar VarName( Name );
  if( Variab( sV ) )
    {
    if( VarName == "" )
      {
      if( sV[0] >= 'e' && sV[0] <= 'z' ) Result = sV[0];
      }
    else
      if( VarName == sV[0] ) Result = VarName;
    return Result;
    }
  MathExpr ex1, ex2;
  if( Unarminus( ex1 ) )
    return ex1.HasUnknown( VarName );
  if( Log( ex1, ex2 ) )
    {
    Result = ex2.HasUnknown( VarName );
    if( !Result.isEmpty() ) return Result;
    return ex1.HasUnknown( VarName );
    }
  if( Funct( sV, ex1 ) )
    return ex1.HasUnknown( VarName );
  char cV;
  if( !Oper_( cV, ex1, ex2 ) )
    {
    Syst_( ex1 );
    if( IsConstType( TLexp, ex1 ) )
      {
      PExMemb memb = CastPtr( TLexp, ex1 )->First();
      while( !memb.isNull() )
        {
        Result = memb->m_Memb.HasUnknown( VarName );
        if( !Result.isEmpty() ) return Result;
        memb = memb->m_pNext;
        }
      return Result;
      }
    else
      throw  ErrParser( "Wrong expression!", peNoSolvType );
    }
  Result = ex1.HasUnknown( VarName );
  if( !Result.isEmpty() ) return Result;
  return ex2.HasUnknown( VarName );
  }

MathExpr TExpr::CancellationOfMultiNominals( MathExpr& exf )
  {
  int SignOfDivision;
  MathExpr dividend, divisor, Result2, Result3;
  auto Canc = [&] ()
    {
    Result2 = NegTExprs( dividend / divisor, SignOfDivision == -1 );
    do{} while( !CommonTerm( dividend, divisor ).IsEmpty() );
    if( divisor == 1 )
      Result3 = dividend;
    else
      Result3 = NegTExprs( dividend / divisor, SignOfDivision == -1 );
    };
  exf.Clear();
  MathExpr Result = this;

  if( CheckDivision( this, dividend, divisor, SignOfDivision ) )
    {
    if( s_IsIntegral && divisor.WriteE().length() > 20 ) return Result;
    dividend = Expand( dividend );
    divisor = Expand( divisor );

    dividend = ToFactors( dividend );
    divisor = ToFactors( divisor );
    Canc();

    if( Result2.Eq( Result3 ) )
      {
      dividend = dividend.ReToMult();
      divisor = divisor.ReToMult();
      Canc();
      }
    if( !Result2.Eq( Result3 ) )
      {
      exf = Result2;
      Result = Result3;
      TSolutionChain::sm_SolutionChain.AddExpr( exf, X_Str( "Mfactors", "factors" ) );
      TSolutionChain::sm_SolutionChain.AddExpr( Result, X_Str( "Mcancel", "cancellation!" ) );
      }
    }
  return Result;
  }

MathExpr MathExpr::ReduceToMult() const
  {
  TestPtr();
  MathExpr Arg1, Arg2, exi, exi1, exi2, exi3;
  double ar, br;
  if( Summa( Arg1, Arg2 ) || Subtr( Arg1, Arg2 ) || ( Multp( Arg1, Arg2 ) ) && Arg1.Constan( ar ) && Arg2.Constan( br ) )
    exi = Reduce();
  else
    exi = *this;

  int  n;
  if( exi.Power( exi1, exi2 ) && exi2.Cons_int( n ) && n > 1 )
    {
    MathExpr P = exi1.Reduce().ReduceToMult();
    if( n > 1 ) s_GlobalInvalid = false;
    for( int i = 2; i <= n; i++ )
      P *= exi1.Reduce();
    return P;
    }

  if( exi.Multp( exi1, exi2 ) )
    {
    Arg1 = exi1.ReduceToMult();
    Arg2 = exi2.ReduceToMult();
    if( exi1.Eq( Arg1 ) && exi2.Eq( Arg2 ) )
      {
      s_GlobalInvalid = true;
      return exi;
      }
    s_GlobalInvalid = false;
    return Arg1 * Arg2;
    }

  if( exi.Divis( exi2, exi3 ) )
    {
    s_GlobalInvalid = false;
    MathExpr exi_d = exi2.ReduceToMult();
    bool SaveGlobalInvalid = s_GlobalInvalid;
    s_GlobalInvalid = false;
    MathExpr exi_n = exi3.ReduceToMult();
    s_GlobalInvalid = s_GlobalInvalid && SaveGlobalInvalid;
    return exi_d / exi_n;
    }

  int  num, den;
  if( exi.SimpleFrac_( num, den ) )
    return Constant( num ).ReduceToMultiplicators() / Constant( den ).ReduceToMultiplicators();

  s_GlobalInvalid = false;

  if( exi.Summa( Arg1, Arg2 ) || exi.Subtr( Arg1, Arg2 ) )
    {
    MathExpr P = ToFactors( exi );
    bool SaveGlobalInvalid = s_GlobalInvalid;
    s_GlobalInvalid = false;
    P = P.Reduce();
    s_GlobalInvalid = SaveGlobalInvalid;
    return P;
    }

  s_GlobalInvalid = true;
  return exi;
  }

MathExpr MathExpr::ReduceToMultiplicators()
  {
  TestPtr();
  int iconst;
  if( Cons_int( iconst ) )
    {
    TListOfInt NumLst;
    int NumFact;
    FactNumber( iconst, NumFact, NumLst );
    TElement *pIndexOfInt = NumLst.m_pFirst;
    MathExpr newexp = Constant( pIndexOfInt->m_N1 );
    for( ; pIndexOfInt->m_N2 > 1; pIndexOfInt->m_N2-- )
      newexp *= Constant( pIndexOfInt->m_N1 );

    for( pIndexOfInt = pIndexOfInt->m_pNext; pIndexOfInt != nullptr; pIndexOfInt = pIndexOfInt->m_pNext )
      for( ; pIndexOfInt->m_N2 > 0; pIndexOfInt->m_N2-- )
        newexp *= Constant( pIndexOfInt->m_N1 );
    return newexp;
    }
  MathExpr Op1, Op2;
  if( Multp( Op1, Op2 ) )
    return Op1.ReduceToMultiplicators() * Op2.ReduceToMultiplicators();
  return new TFunc( true, "ReduceToMultiplicators", *this );
  }

MathExpr TLexp::SortList( bool Order ) const
  {
  auto IsValue = [&] ( const MathExpr& E, double& Val )
    {
    MathExpr  reduced = E.Reduce();
    int N, D;
    if( reduced.SimpleFrac_( N, D ) )
      {
      Val = N / ( double ) D;
      return true;
      }
    return reduced.Constan( Val );
    };

  auto SwapEl = [&] ( TLexp  *pRlist, const PExMemb& p1, const PExMemb& p2 )
    {
    p1->m_pNext = p2->m_pNext;
    p2->m_pNext = p1;
    p2->m_pPrev = p1->m_pPrev;
    p1->m_pPrev = p2;

    if( !p1->m_pNext.isNull() )
      p1->m_pNext->m_pPrev = p1;
    if( !p2->m_pPrev.isNull() )
      p2->m_pPrev->m_pNext = p2;

    if( pRlist->m_pFirst == p1 )
      pRlist->m_pFirst = p2;
    if( pRlist->m_pLast == p2 )
      pRlist->m_pLast = p1;
    };

  PExMemb ibeg, iend;
  MathExpr retu(Clone());
  retu.Listex( ibeg );
  bool defined = true;
  PExMemb ilast = ibeg;

  while( defined && ibeg != iend )
    {
    PExMemb p1 = ibeg;
    PExMemb p2 = p1->m_pNext;
    bool isval1 = false;
    while( ( p2 != iend ) )
      {
      double V1, V2;
      if( !isval1 )
        isval1 = IsValue( p1->m_Memb, V1 );
      if( !isval1 || !IsValue( p2->m_Memb, V2 ) )
        {
        defined = false;
        break;
        }

      if( Order && V1 > V2 || !Order && V1 < V2 )
        {
        SwapEl( CastPtr( TLexp, retu ), p1, p2 );
        ilast = p1;
        }
      else
        {
        p1 = p2;
        V1 = V2;
        }

      p2 = p1->m_pNext;
      }

    iend = ilast;
    retu.Listex( ibeg );
    ilast = ibeg;
    }

  if( defined )
    return retu;
  if( Order )
    return new TFunc( true, "SortListA", Clone() );
  return new TFunc( true, "SortListD", Clone() );
  }

Lexp Lexp::LeastCommonDenominator( bool bReduce )
  {
  MathExpArray AddMult, Nom, Den;
  int N, D;
  for( PExMemb Item = First(); !Item.isNull(); Item = Item->m_pNext )
    {
    MathExpr op1, op2;
    if( Item->m_Memb.Divis( op1, op2 ) )
      {
      Nom.append( op1 );
      Den.append( op2 );
      }
    else
      if( Item->m_Memb.SimpleFrac_( N, D ) )
        {
        Nom.append( Constant( N ) );
        Den.append( Constant( D ) );
        }
      else
        {
        if( IsConstType( TBinar, Item->m_Memb ) ) return Lexp();
        Nom.append( Item->m_Memb );
        Den.append( Constant( 1 ) );
        }
    }

  AddMult.append( Constant( 1 ) );
  MathExpr CommDen = ToFactors( Den[0] ), CommExp, Mult;
  for( int i = 1; i < Den.count(); i++ )
    {
    Mult = ToFactors( Den[i] );
    try
      {
      CommExp = GetCommon( CommDen, Mult );
      }
    catch( ErrParser )
      {
      CommExp = Constant( 1 );
      }
    for( int j = 0; j < i; j++ )
      AddMult[j] = ( AddMult[j] * Mult ).Reduce();
    AddMult.append( CommDen.Reduce() );
    CommDen = MultExprs( CommExp, MultExprs( CommDen, Mult ) ).Reduce();
    }

  Lexp ex( new TLexp );
  for( int i = 0; i < Nom.count(); i++ )
    {
    Mult = Nom[i] * AddMult[i];
    if( bReduce ) Mult = ExpandExpr( Mult );
    ex.Addexp( Mult / CommDen );
    }
  return ex;
  }

bool MathExpr::EquaCh() const
  {
  MathExpr  ex1, ex2;
  if( !Binar( '=', ex1, ex2 ) ) return false;
  MathExpr arg1 = ex1.Diff();
  MathExpr arg2 = ex2.Diff();
  double a1, a2;
  return !( arg1.Constan( a1 ) && arg2.Constan( a2 ) && abs( a1 + a2 ) <= 0.1E-7 );
  }

void MathExpr::BinomX( MathExpr& a, MathExpr& b, bool& Check, const QByteArray& v ) const
  {
  Check = true;
  b = Reduce();
  MathExpr p1 = b.Diff( v );
  a = p1.Reduce();
  double c;
  if( a.Constan( c ) && c == 0 ) return;
  p1 = a.Diff( v );
  MathExpr dif2 = p1.Reduce();
  if( !( dif2.Constan( c ) && abs( c ) <= 0.0000001 ) )
    {
    Check = false;
    return;
    }
  p1 = a * ( new TVariable( true, v ) );
  MathExpr p = *this - Expand( p1 );
  p = ExpandExpr( p.ReduceTExprs() );
  s_GlobalInvalid = false;
  b = p.Reduce();
  }

bool MathExpr::LinearCh( int& a ) const
  {
  MathExpr  ap, bp;
  double c;
  bool Check;
  BinomX( ap, bp, Check );
  if( ap.Constan( c ) && c == 0 )
    a = 0;
  else
    a = 1;
  return Check;
  }

void MathExpr::LinEquCh( MathExpr& a, MathExpr& b, bool& check ) const
  {
  a = b = Constant( 1 );
  check = false;
  MathExpr  ex1, ex2;
  double ca, c0;
  int cod1, cod2;
  if( Binar( '=', ex1, ex2 ) && !( ex2.Constan( c0 ) && c0 == 0 ) )
    {
    if( !( ex1.LinearCh( cod1 ) && ex2.LinearCh( cod2 ) ) ) return;
    MathExpr p1 = ex1 - ex2;
    MathExpr arg1 = Expand( p1 );
    if( arg1.LinearCh( cod1 ) && cod1 != 0 )
      {
      MathExpr p = new TBinar( '=', p1.Reduce(), Constant( 0 ) );
      p.LinEquCh( a, b, check );
      }
    else
      {
      b = arg1;
      a = Constant( 0 );
      check = true;
      }
    return;
    }

  if( Binar( '=', ex1, ex2 ) && ex2.Constan( c0 ) && c0 == 0 )
    {
    MathExpr arg1 = ex1;
    if( !arg1.LinearCh( cod1 ) || cod1 == 0 ) return;
    MathExpr p = ex1.Reduce();
    MathExpr p1 = p.Diff();
    MathExpr dif1 = p1.Reduce();
    if( dif1.Constan( ca ) && abs( ca ) <= 0.0000001 )
      {
      check = true;
      return;
      }
    p1 = dif1.Diff();
    MathExpr dif2 = p1.Reduce();
    if( dif2.Constan( ca ) && abs( ca ) <= 0.0000001 )
      {
      check = true;
      a = dif1.Reduce();
      p = ex1 - a * ( new TVariable( true, "x" ) );
      b = Expand( p );
      b = ExpandExpr( b );
      }
    }
  }

MathExpr MathExpr::LinEqu( MathExpArray& List ) const
  {
  Lexp P;
  auto Solved = [&]()
    {
    s_GlobalInvalid = false;
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MLinSolved", "Linear Equation is Solved!" ) );
    return P; 
    };

  MathExpr a, b;
  bool check;
  LinEquCh( a, b, check );
  P = new TL2exp;
  P.Addexp( *this );
  double ca, cb;
  if( a.Constan( ca ) && abs( ca ) <= 0.0000001 )
    {
    if( !b.Constan( cb ) || ( b.Constan( cb ) && abs( cb ) >= 0.0000001 ) )
      {
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolutions", "No Solutions!" ) );
      Info_m( X_Str( "MNoSolutions", "No Solutions!" ) );
      }
    else
      {
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfinSolutions", "Infinit Number of Solutions!" ) );
      Info_m( X_Str( "MInfinSolutions", "Infinit Number of Solutions!" ) );
      }
    s_GlobalInvalid = false;
    return P;
    }

  Lexp limits = new TLexp;
  MathExpr Temp = CalcR( ( -( b.Reduce() ) ).ReduceTExprs(), a.Reduce(), limits );
  P.Addexp( new TBinar( '=', new TVariable( true, "x" ), Temp ) );
  if( limits.Count() > 0 ) P.Addexp( limits );
  List.append( -b / a );
  return Solved();
  }

void MathExpr::TrinomCh( MathExpr& ea, MathExpr& eb, MathExpr& ec, bool& check, const QByteArray& v ) const
  {
  s_IsTrinom = true;
  ea = Constant( 0 );
  MathExpr Arg = Reduce();
  Arg.BinomX( eb, ec, check, v );
  if( check )
    {
    s_IsTrinom = false;
    return;
    }
  MathExpr Arg1 = Arg.Diff( v );
  MathExpr Darg = Arg1.Reduce();
  Darg.BinomX( ea, eb, check, v );
  if( !check )
    {
    s_IsTrinom = false;
    return;
    }
  MathExpr P =  ea / 2;
  MathExpr P1 = P.Reduce();
  MathExpr expa = Expand( P1 );

  s_GlobalInvalid = false;
  ea = expa.Reduce();
  P = ea * ( Variable( v, true ) ^ 2 );
  P += eb * Variable( v, true );
  P1 = Expand( P );
  s_GlobalInvalid = false;
  P = Arg - P1.Reduce();
  P1 = P.Reduce();
  Arg1 = Expand( P1 );
  ec = Arg1.ReduceToMult();
  if( Arg1.Negative() && !ec.Negative() )
    ec = -ec;
  check = true;
  s_IsTrinom = false;
  }

void MathExpr::QuaEquCh( MathExpr& ea, MathExpr& eb, MathExpr& ec, bool& check ) const
  {
  ea = Constant( 1 );
  eb = Constant( 1 );
  ec = Constant( 1 );
  check = false;

  MathExpr  ex1, ex2, P, P1, Op1, Op2;
  if( !Binar( '=', ex1, ex2 ) ) return;
  if(  ex2 != 0 )
    {
    if( ex2.Summa( Op1, Op2 ) )
      P1 = ex1 - Op1 - Op2;
    else
      P1 = ex1 - ex2;
    P = new TBinar( '=', P1.Reduce(), Constant( 0 ) );
    P.QuaEquCh(ea, eb, ec, check );
    return;
    }
  ex1.TrinomCh( ea, eb, ec, check );
  s_GlobalInvalid = false;
  }

MathExpr MathExpr::SqRootSm() const
  {
  MathExpr  op1, op2;
  int a, b, r, c;
  if( Root_( op1, op2, r ) && op1.Cons_int( c ) && r == 2 && c >= 0 )
    {
    ConvertRoot( 2, c, a, b );
    s_GlobalInvalid = false;
    if( b == 1 ) return Constant( a );
    if( a != 1 ) return Constant( a ) * Constant( b ).Root( 2 );
    return *this;
    }
  s_GlobalInvalid = true;
  return *this;
  }

MathExpr MathExpr::DetQuaEqu( MathExpArray& List ) const
  {
  MathExpr a;
  double cd;
  Lexp P;
  auto Solved = [&] ()
    {
    if( !a.Constan( cd ) )
      P.Addexp( new TBinar( msNotequal, a, Constant( 0 ) ) );
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MQuadEqSolved", "Quadratic Equation is Solved!" ) );
    s_GlobalInvalid = false;
    return P;
    };

  if( !EquaCh() )
    {
    if( !s_CalcOnly ) Info_m( X_Str( "MEnterEquat", "Enter Equation!" ) );
    s_GlobalInvalid = true;
    return *this;
    }

  MathExpr b, c;
  bool check;
  LinEquCh( a, b, check );
  if( check ) return LinEqu( List );

  QuaEquCh( a, b, c, check );
  if( !check )
    {
    s_GlobalInvalid = true;
    if( !s_CalcOnly )
      Info_m( X_Str( "MEnterQuadrEquat", "Enter Quadratic Equation!" ) );
    return *this;
    }

  P = new TL2exp;
  P.Addexp( *this );
  double ca, cb, cc;
  MathExpr P1;
  if( a.Constan( ca ) && abs( ca ) <= 0.0000001 )
    {
    if( b.Constan( cb ) && abs( cb ) <= 0.0000001 )
      {
      if( !c.Constan( cc ) || c.Constan( cc ) && abs( cc ) >= 0.0000001 )
        {
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolutions", "No Solutions!" ) );
        Info_m( X_Str( "MNoSolutions", "No Solutions!" ) );
        P1 = new TBinar( '=', b * Variable( "x", true ), c );
        }
      else
        {
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInfinSolutions", "Infinit Number of Solutions!" ) );
        Info_m( X_Str( "MInfinSolutions", "Infinit Number of Solutions!" ) );
        P1 = new TBinar( '=', b * Variable( "x", true ), c );
        }
      P.Addexp( P1 );
      s_GlobalInvalid = false;
      return P;
      }

    MathExpr Temp = b * Variable( "x", true ) + c;
    P1 = new TBinar( '=', Temp.Reduce(), Constant( 0 ) );
    P.Appendz( P1.LinEqu( List ).Reduce() );
    return Solved();
    }

  if( a.Negative() )
    {
    a = ( -a ).Reduce();
    b = ( -b ).Reduce();
    c = ( -c ).Reduce();
    }

  MathExpr Temp = a * ( Variable( "x", true ) ^ 2 );
  Temp += b * Variable( "x", true );
  Temp += c;
  P1 = new TBinar( '=', Temp.Reduce(), Constant( 0 ) );
  if( !Eq( P1 ) )
    P.Addexp( P1 );
  int CoeffInt = 1;
  int ValA, ValB, ValC;
  if( a.Cons_int( ValA ) && b.Cons_int( ValB ) && c.Cons_int( ValC ) )
    {
    Lexp CoeffList = new TLexp;
    CoeffList.Addexp( Constant( abs( ValA ) ) );
    CoeffList.Addexp( Constant( abs( ValB ) ) );
    CoeffList.Addexp( Constant( abs( ValC ) ) );
    MathExpr CoeffGCD = CoeffList.FindGreatestCommDivisor();
    if( !CoeffGCD.IsEmpty() ) CoeffGCD.Cons_int( CoeffInt );
    if( CoeffInt > 1 )
      {
      a = Constant( ValA / CoeffInt );
      b = Constant( ValB / CoeffInt );
      c = Constant( ValC / CoeffInt );
      }
    }

  P1 = a * c;
  P1 = Constant( 4 ) * P1;
  P1 = b * b - P1;
  MathExpr D = Expand( P1 ).Reduce();
  if( D.Negative() )
    {
    Temp = CreateComplex( ( Constant( ( 0 ) ) ), ( ( ( -( ( D ) ).Reduce() ).Root( 2 ) ) ) );
    MathExpr x1 = ( -b - Temp ) / ( Constant( 2 ) * a );
    P1 = x1.Reduce();
    List.append( P1 );
    if( !P1.Eq( x1 ) )
      x1 = new TBinar( '=', x1, P1 );
    P.Addexp( x1 );
    MathExpr x2 = ( -b + Temp ) / ( Constant( 2 ) * a );
    P1 = x2.Reduce();
    List.append( P1 );
    if( !P1.Eq( x2 ) )
      x2 = new TBinar( '=', x2, P1 );
    P.Addexp( x2 );
    return Solved();
    }

  if( D.Constan( cd ) )
    {
    if( abs( cd ) <= 1E-7 )
      {
      P1 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "1", true ),
        Variable( QByteArray( "x" ) + ( char ) msLowReg + "2", true ) );
      P1 = new TBinar( '=', P1, -b / ( Constant( 2 ) * a ) );
      List.append( CastPtr( TBinar, P1 )->Right() );
      P.Addexp( P1 );
      return Solved();
      }
    }

  int id, ib;
  MathExpr P2;
  if( D.Cons_int( id ) && b.Cons_int( ib ) )
    {
    Temp = D.Root( 2 );
    P1 = Temp.SqRootSm();
    MathExpr x1 = ( -b - Temp ) / ( Constant( 2 ) * a );
    MathExpr x2 = ( -b + Temp ) / ( Constant( 2 ) * a );
    List.append( x1.Reduce() );
    List.append( x2.Reduce() );
    if( b.Negative() )
      P2 = ( Constant( abs( ib ) ) - P1 ) / ( Constant( 2 ) * a );
    else
      P2 = ( -b - P1 ) / ( Constant( 2 ) * a );
    x1 = new TBinar( '=', x1, P2 );
    Temp = P2.Reduce();
    if( !P2.Eq( Temp ) )
      x1 = new TBinar( '=', x1, Temp );
    x1 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "1", true ), x1 );
    P.Addexp( x1 );
    Temp = D.Root( 2 );
    if( b.Negative() )
      P2 = ( Constant( abs( ib ) ) + P1 ) / ( Constant( 2 ) * a );
    else
      P2 = ( -b + P1 ) / ( Constant( 2 ) * a );
    x2 = new TBinar( '=', x2, P2 );
    Temp = P2.Reduce();
    if( !P2.Eq( Temp ) )
      x2 = new TBinar( '=', x2, Temp );
    x2 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "2", true ), x2 );
    P.Addexp( x2 );
    return Solved();
    }

  if( D.Constan( cd ) && b.Constan( cb ) )
    {
    Temp = D.Root( 2 );
    P1 = Temp.Reduce();
    MathExpr x1 = ( -b - Temp ) / ( Constant( 2 ) * a );
    MathExpr x2 = ( -b + Temp ) / ( Constant( 2 ) * a );
    List.append( x1.Reduce() );
    List.append( x2.Reduce() );
    if( !P1.Eq( Temp ) )
      {
      if( b.Negative() )
        P2 = ( Constant( abs( cb ) ) - P1 ) / ( Constant( 2 ) * a );
      else
        P2 = ( -b - P1 ) / ( Constant( 2 ) * a );
      x1 = new TBinar( '=', x1, P2 );
      Temp = P2.Reduce();
      if( !P2.Eq( Temp ) )
        x1 = new TBinar( '=', x1, Temp );
      }
    x1 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "1", true ), x1 );
    P.Addexp( x1 );
    Temp = D.Root( 2 );
    if( !P1.Eq( Temp ) )
      {
      if( b.Negative() )
        P2 = ( Constant( abs( cb ) ) + P1 ) / ( Constant( 2 ) * a );
      else
        P2 = ( -b + P1 ) / ( Constant( 2 ) * a );
      x2 = new TBinar( '=', x2, P2 );
      Temp = P2.Reduce();
      if( !P2.Eq( Temp ) )
        x2 = new TBinar( '=', x2, Temp );
      }
    x2 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "2", true ), x2 );
    P.Addexp( x2 );
    return Solved();
    }

  if( D.Constan( cd ) )
    {
    P1 = Constant( sqrt( cd ) );
    MathExpr x1 = ( -b - P1 ) / ( Constant( 2 ) * a );
    Temp = x1.Reduce();
    List.append( Temp );
    if( !Temp.Eq( x1 ) )
      x1 = new TBinar( '=', x1, Temp );
    x1 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "1", true ), x1 );
    P.Addexp( x1 );
    MathExpr x2 = ( -b + P1 ) / ( Constant( 2 ) * a );
    List.append( x2.Reduce() );
    Temp = x2.Reduce();
    if( !x2.Eq( Temp ) )
      x2 = new TBinar( '=', x2, Temp );
    x2 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "2", true ), x2 );
    P.Addexp( x2 );
    return Solved();
    }

  P1 = D.Root( 2 );
  MathExpr x1 = ( -b - P1 ) / ( Constant( 2 ) * a );
  List.append( x1.Reduce() );
  Temp = x1.Reduce();
  if( !Temp.Eq( x1 ) )
    x1 = new TBinar( '=', x1, Temp );
  x1 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "1", true ), x1 );
  P.Addexp( x1 );
  MathExpr x2 = ( -b + P1 ) / ( Constant( 2 ) * a );
  List.append( x2.Reduce() );
  Temp = x2.Reduce();
  if( !x2.Eq( Temp ) )
    x2 = new TBinar( '=', x2, Temp );
  x2 = new TBinar( '=', Variable( QByteArray( "x" ) + ( char ) msLowReg + "2", true ), x2 );
  P.Addexp( x2 );
  return Solved();
  }

MathExpr MathExpr::DetLinEqu() const
  {
  Lexp  P;
  auto Solved = [&] ()
    {
    s_GlobalInvalid = false;
    return P;
    };

  if( !EquaCh() )
    {
    s_GlobalInvalid = true;
    throw  ErrParser( X_Str( "MEnterEquat", "Enter Equation!" ), peNoSolvType );
    }
  bool check;
  MathExpr a, b;
  LinEquCh( a, b, check );
  if( !check )
    {
    s_GlobalInvalid = true;
    throw  ErrParser( X_Str( "MEnterLinEquat", "Enter Linear Equation!" ), peNoSolvType );
    }
  P = new TL2exp;
  P.Addexp( *this );
  MathExpr Temp = ( -b ).ReduceTExprs();
  MathExpr P1 = new TBinar( '=', a.Reduce() * Variable( "x", true ), Temp.Reduce() );
  if( !Eq( P1 ) ) P.Addexp( P1 );
  double ca, cb;
  if( a.Constan( ca ) && abs( ca ) <= 0.0000001 )
    {
    s_GlobalInvalid = false;
    if( !b.Constan( cb ) || b.Constan( cb ) && abs( cb ) >= 0.0000001 )
      throw  ErrParser( X_Str( "MNoSolutions", "No Solutions!" ), peNoSolvType );
    throw  ErrParser( X_Str( "MInfinSolutions", "Infinit Number of Solutions!" ), peNoSolvType );
    }
  if( a.Constan( ca ) && abs( ca - 1 ) <= 0.0000001 )
    {
    P1 = new TBinar( '=', Variable( "x", true ), Temp.Reduce() );
    P.Addexp( P1 );
    return Solved();
    }
  if( b.Constan( cb ) && cb == 0 )
    {
    if( !a.Constan( ca ) || a.Constan( ca ) && abs( ca - 1 ) >= 0.0000001 )
      {
      P1 = new TBinar( '=', Variable( "x", true ), b.Reduce() );
      P.Addexp( P1.Clone() );
      }
    return Solved();
    }
  Lexp limits;
  Temp = CalcR( ( -b ).Reduce().ReduceTExprs(), a.Reduce(), limits );
  P1 = new TBinar( '=', Variable( "x", true ), Temp );
  P.Addexp( P1 );
  if( !limits.IsEmpty() ) P.Addexp( limits );
  return Solved();
  }

MathExpr TExpr::Mixer()
  {
  PExMemb  index;
  MathExpr app;
  if( Listex( index ) )
    {
    Lexp newexp;
    if( List2ex( index ) )
      newexp = new TL2exp;
    else
      newexp = new TLexp;
    for( Listex( index ); !index.isNull(); index = index->m_pNext )
      {
      int N1, D1;
      if( index->m_Memb.SimpleFrac_( N1, D1 ) )
        {
        if( N1 == D1 )
          app = 1;
        else
          if( -N1 == D1 )
            app = -1;
          else
            if( abs( N1 ) > abs( D1 ) )
              if( N1 % D1 == 0 )
                app = N1 / D1;
              else
                app = new TMixedFrac( 0, N1, D1 );
            else
              app = new TSimpleFrac( N1, D1 );
        }
      else
        {
        MathExpr op1, op2;
        if( index->m_Memb.Divis( op1, op2 ) && op1.Cons_int( N1 ) && op2.Cons_int( D1 ) && D1 < 40001 )
          {
          if( N1 == D1 )
            app = 1;
          else
            if( -N1 == D1 )
              app = -1;
            else
              if( abs( N1 ) > abs( D1 ) )
                if( N1 % D1 == 0 )
                  app = N1 / D1;
                else
                  app = new TMixedFrac( 0, N1, D1 );
              else
                app = new TSimpleFrac( N1, D1 );
          }
        else
          app = index->m_Memb;
        }
      newexp.Addexp( app );
      newexp.Last()->m_Visi = index->m_Visi;
      }
    return newexp;
    }
  int N1, D1;
  if( SimpleFrac_( N1, D1 ) && Cast( TMixedFrac, this ) == nullptr )
    {
    if( N1 == D1 ) return app = 1;
    if( -N1 == D1 ) return app = -1;
    if( abs( N1 ) > abs( D1 ) )
      {
      if( N1 % D1 == 0 ) return app = N1 / D1;
      return new TMixedFrac( 0, N1, D1 );
      }
    return new TSimpleFrac( N1, D1 );
    }
  MathExpr op1, op2;
  if( !( Divis( op1, op2 ) && op1.Cons_int( N1 ) && op2.Cons_int( D1 ) && D1 < 40001 ) ) return this;
  if( N1 == D1 ) return app = 1;
  if( -N1 == D1 ) return  app = -1;
  if( abs( N1 ) > abs( D1 ) ) return new TMixedFrac( 0, N1, D1 );
  return new TSimpleFrac( N1, D1 );
  }
