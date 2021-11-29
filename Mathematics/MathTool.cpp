#include <qbytearray.h>
#include "MathTool.h"
//#include "StrHebEng.h"
#include "Globalvars.h"
#include "../FormulaPainter/XPSymbol.h"
#include "ExpObjOp.h"
#include "ExpObBin.h"
#include "Parser.h"
#include "SolChain.h"
#include <functional>

enum Param { arVect, arLim };

TListOfInt::~TListOfInt()
  {
  TElement *pFree;
  while( m_pFirst )
    {
    pFree = m_pFirst;
    m_pFirst = m_pFirst->m_pNext;
    delete pFree;
    }
  }

void TListOfInt::Add_element( int A, int B )
  {
  TElement  *pIndex = m_pLast;
  m_pLast = new TElement( m_pLast, A, B );
  if( m_pFirst == nullptr )
    m_pFirst = m_pLast;
  if( pIndex )
    pIndex->m_pNext = m_pLast;
  }

void CancFrac( int &n, int &d )
  {
  if( d == 0 )
    {
    if( !s_CalcOnly )
    s_LastError="INFVAL";
    s_GlobalInvalid = true;
    s_CheckError = true;
    n = sc_NumError;
    d = sc_NumError;
    return;
    };
  int m = GrComDiv( n, d );
  if( m == 0 ) return;
  n /= m;
  d /= m;
  }

int GrComDiv( int x, int y )
  {
  int  r, r1, r2;
  if( x == 0 || y == 0 )
    return 1;
  r = abs( x );
  r1 = abs( y );
  while( r1 != 0 )
    {
    r2 = r % r1;
    r = r1;
    r1 = r2;
    }
  return abs( r );
  }

void LComDen( int n1, int d1, int n2, int d2, int& n1r, int& n2r, int& cd, int& m1, int& m2 )
  {
  int  n;
  n1r = sc_NumError;
  n2r = sc_NumError;
  cd = sc_NumError;
  m1 = sc_NumError;
  m2 = sc_NumError;

  if( d1 == 0 || d2 == 0 )
    {
    if( !s_CalcOnly )
      Error_m( X_Str( "MDenomIs0", "Denominator equals 0" ) );
    s_CheckError = true;
    s_GlobalInvalid = true;
    return;
    }

  if( n1 == 0 )
    {
    cd = d2;
    n1r = 0;
    m1 = 1;
    m2 = 1;
    n2r = n2;
    return;
    }

  if( n2 == 0 )
    {
    cd = d1;
    n1r = n1;
    m1 = 1;
    m2 = 1;
    n2r = n2;
    return;
    }
  n = GrComDiv( d1, d2 );
  if( n == 0 ) return;

  m1 = d2 / n;
  m2 = d1 / n;

  cd = m1 * m2 * n;
  n1r = n1 * m1;
  n2r = n2 * m2;
  }

void SumFrac( int n1, int d1, int n2, int d2, int& nr, int& dr )
  {
  int  m1, m2;
  if( d1 == 0 || d2 == 0 )
    {
    if( !s_CalcOnly )
      Error_m( X_Str( "MDenomIs0", "Denominator equals 0" ) );
    s_CheckError = true;
    s_GlobalInvalid = true;
    nr = sc_NumError;
    dr = sc_NumError;
    return;
    }

  LComDen( n1, d1, n2, d2, n1, n2, dr, m1, m2 );
  nr = n1 + n2;
  CancFrac( nr, dr );
  }

void MultFractions( int n1, int d1, int n2, int d2, int& nr, int& dr )
  {
  if( d1 == 0 || d2 == 0 )
    {
    if( !s_CalcOnly )
      Error_m( X_Str( "MDenomIs0", "Denominator equals 0" ) );
    s_GlobalInvalid = true;
    nr = sc_NumError;
    dr = sc_NumError;
    return;
    }
  nr = n1 * n2;
  dr = d1 * d2;
  if( nr / n1 != n2 || dr / d1 != d2 )
    {
    s_GlobalInvalid = true;
    nr = 0;
    dr = sc_NumError;
    return;
    }
  CancFrac( nr, dr );
  }

void DivFrac( int n1, int d1, int n2, int d2, int& nr, int& dr )
  {
  if( d1 == 0 || d2 == 0 )
    {
    if( !s_CalcOnly )
      Error_m( X_Str( "MDenomIs0", "Denominator equals 0" ) );
    s_GlobalInvalid = true;
    nr = sc_NumError;
    dr = sc_NumError;
    return;
    }
  if( n2 == 0 )
    {
    Error_m( X_Str( "MDivisBy0", "Division by 0!" ) );
    s_GlobalInvalid = true;
    nr = sc_NumError;
    dr = sc_NumError;
    return;
    }

  if( n1 * 1.0 * d2 > INT_MAX * 0.1 || d1 * 1.0 * n2 > INT_MAX * 0.1 )
    {
    Error_m( X_Str( "MFracToReal", "Numerator || Denominator >40000. Result -> real" ) );
    s_GlobalInvalid = true;
    nr = 1;
    dr = sc_NumError;
    return;
    }

  nr = n1 * d2;
  dr = d1 * n2;
  CancFrac( nr, dr );
  }

QByteArray PiVar2PiConst( const QByteArray& ExprStr )
  {
  QByteArray Result(ExprStr);
  for( int P = Result.indexOf( msPi ); P > -1; P = Result.indexOf( msPi ) )
    {
    Result.remove( P, 1 );
    Result.insert( P, QByteArray::number( M_PI ) );
    }
  return Result;
  }

QByteArray PiVar2Pi180( const QByteArray& ExprStr )
  {
  QByteArray Result( ExprStr );
  for( int P = Result.indexOf( msPi ); P > -1; P = Result.indexOf( msPi ) )
    {
    Result.remove( P, 1 );
    Result.insert( P, "180" );
    }
  return Result;
  }

bool IsTrigonom( const QByteArray& FuncName )
  {
  return FuncName == "sin" || FuncName == "cos" || FuncName == "tan" || FuncName == "cot";
  }

bool PiProcess( const MathExpr& exi, double& Coeff1, double& Coeff2 )
  {
  MathExpr expo, modul, op1, op2, op11, op22;
  bool SignNeg = false;
  QByteArray VarName;
  bool Result = exi.WriteE().indexOf( msPi ) > -1;
  if( Result ) 
    {
    SignNeg = exi.Unarminus( modul );
    if( !SignNeg) modul = exi;
    Coeff2 = 1;
    if( modul.Variab( VarName ) && (char) VarName[0] == msPi ) Coeff1 = 1;
    if( modul.Multp( op1, op2 ) )
      {
      if( op1.Variab( VarName ) && ( char ) VarName[0] == msPi ) 
        {
        expo = op2.Reduce();
        Result = expo.Constan( Coeff1 ) || expo.SimpleFrac_( Coeff1, Coeff2 );
        }
      if( op2.Variab( VarName ) && ( char ) VarName[0] == msPi )
         {
        expo = op1.Reduce();
        Result = expo.Constan( Coeff1 ) || expo.SimpleFrac_( Coeff1, Coeff2 );
        }
      }
    else 
      if( modul.Divis( op1, op2 ) )
        {
        if( op1.Multp( op11, op22 ) )
          {
          Result = true;
          if( op11.Variab( VarName ) && ( char ) VarName[0] == msPi )
            Result = op22.Constan( Coeff1 ) && Result;
          if( op22.Variab( VarName ) && ( char ) VarName[0] == msPi )
            Result = op11.Constan( Coeff1 ) && Result;
          Result = op2.Constan( Coeff2 ) && Result;
          }
        else
          if( op1.Variab( VarName ) && ( char ) VarName[0] == msPi )
            {
            Result = op2.Constan( Coeff2 );
            Coeff1 = 1;
            }
        }
    }
  if( SignNeg ) Coeff1 = -Coeff1;
  return Result;
  }

bool PiSinStandard( double Coeff1, double Coeff2, MathExpr& PiRes )
  {
  PiRes = MathExpr(nullptr);

  bool SignNeg = ( Coeff1 < 0 && Coeff2 > 0 ) || ( Coeff1 > 0 && Coeff2 < 0 );
  Coeff1 = abs( Coeff1 );
  Coeff2 = abs( Coeff2 );
  while( abs( Coeff1 / Coeff2 ) > 2 ) Coeff1 = Coeff1 - 2 * Coeff2;

  if( ( ( Coeff1 == 1 ) && ( Coeff2 == 6 ) ) )               // ---- Pi/6 ----
    PiRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
  if( ( Coeff1 == 0.25 ) || ( ( Coeff1 == 1 ) && ( Coeff2 == 4 ) ) ) // ---- Pi/4 ----
    PiRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
  if( ( ( Coeff1 == 1 ) && ( Coeff2 == 3 ) ) )               // ---- Pi/3 ----
    PiRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
  if( ( Coeff1 == 0.5 ) || ( ( Coeff1 == 1 ) && ( Coeff2 == 2 ) ) ) // ---- Pi/2 ----
    PiRes = new TConstant( 1 );
  if( ( ( Coeff1 == 2 ) && ( Coeff2 == 3 ) ) )               // ---- 2*Pi/3 ----
    PiRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
  if( ( Coeff1 == 0.75 ) || ( ( Coeff1 == 3 ) && ( Coeff2 == 4 ) ) ) // ---- 3*Pi/4 ----
    PiRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
  if( ( ( Coeff1 == 5 ) && ( Coeff2 == 6 ) ) )               // ---- 5*Pi/6 ----
    PiRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
  if( ( Coeff1 == 1 ) && ( Coeff2 == 1 ) )                 // ---- Pi ----
    PiRes = new TConstant( 0 );
  if( ( ( Coeff1 == 7 ) && ( Coeff2 == 6 ) ) )               // ---- 7*Pi/6 ----
    PiRes = new TUnar( new TDivi( new TConstant( 1 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 1.25 ) || ( ( Coeff1 == 5 ) && ( Coeff2 == 4 ) ) ) // ---- 5*Pi/4 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
  if( ( ( Coeff1 == 4 ) && ( Coeff2 == 3 ) ) )               // ---- 4*Pi/3 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 1.5 ) || ( ( Coeff1 == 3 ) && ( Coeff2 == 2 ) ) ) // ---- 3*Pi/2 ----
    PiRes = new TUnar( new TConstant( 1 ) );
  if( ( ( Coeff1 == 5 ) && ( Coeff2 == 3 ) ) )               // ---- 5*Pi/3 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 1.75 ) || ( ( Coeff1 == 7 ) && ( Coeff2 == 4 ) ) ) // ---- 7*Pi/4 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
  if( ( ( Coeff1 == 11 ) && ( Coeff2 == 6 ) ) )              // ---- 11*Pi/6 ----
    PiRes = new TUnar( new TDivi( new TConstant( 1 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 2 ) && ( Coeff2 == 1 )  )                // ---- 2*Pi ----
    PiRes = new TConstant( 0 );

  if( !PiRes.IsEmpty() && SignNeg && !PiRes.Unarminus( PiRes ) )
    PiRes = TUnar( PiRes ).Reduce();
  return !PiRes.IsEmpty();
  }

bool PiTanStandard( double Coeff1, double Coeff2, MathExpr& PiRes )
  {
  PiRes = MathExpr( nullptr );

  bool SignNeg = ( ( Coeff1 < 0 ) && ( Coeff2>0 ) ) || ( ( Coeff1 > 0 ) && ( Coeff2 < 0 ) );
  Coeff1 = abs( Coeff1 );
  Coeff2 = abs( Coeff2 );
  while( abs( Coeff1 / Coeff2 ) > 2 ) Coeff1 = Coeff1 - 2 * Coeff2;

  if( ( ( Coeff1 == 1 ) && ( Coeff2 == 6 ) ) )               // ---- Pi/6 ----
    PiRes = new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) );
  if( ( Coeff1 == 0.25 ) || ( ( Coeff1 == 1 ) && ( Coeff2 == 4 ) ) )// ---- Pi/4 ----
    PiRes = new TConstant( 1 );
  if( ( ( Coeff1 == 1 ) && ( Coeff2 == 3 ) ) )               // ---- Pi/3 ----
     PiRes = new TRoot( new TConstant( 3 ), 2 );
  if( ( Coeff1 == 0.5 ) || ( ( Coeff1 == 1 ) && ( Coeff2 == 2 ) ) )  // ---- Pi/2 ----
    s_GlobalInvalid = s_TanCotError = true; //PiRes= new TVariable(false,msInfinity);
  if( ( ( Coeff1 == 2 ) && ( Coeff2 == 3 ) ) )                // ---- 2*Pi/3 ----
     PiRes = new TUnar( new TRoot( new TConstant( 3 ), 2 ) );
  if( ( Coeff1 == 0.75 ) || ( ( Coeff1 == 3 ) && ( Coeff2 == 4 ) ) )// ---- 3*Pi/4 ----
     PiRes = new TUnar( new TConstant( 1 ) );
  if( ( ( Coeff1 == 5 ) && ( Coeff2 == 6 ) )  )              // ---- 5*Pi/6 ----
     PiRes = new TUnar( new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) ) );
  if( ( Coeff1 == 1 ) && ( Coeff2 == 1 ) )                 // ---- Pi ----
     PiRes = new TConstant( 0 );
  if( ( ( Coeff1 == 7 ) && ( Coeff2 == 6 ) )                // ---- 7*Pi/6 ----
    ) PiRes = new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) );
  if( ( Coeff1 == 1.25 ) || ( ( Coeff1 == 5 ) && ( Coeff2 == 4 ) ) )// ---- 5*Pi/4 ----
     PiRes = new TConstant( 1 );
  if( ( ( Coeff1 == 4 ) && ( Coeff2 == 3 ) ) )               // ---- 4*Pi/3 ----
     PiRes = new TRoot( new TConstant( 3 ), 2 );
  if( ( Coeff1 == 1.5 ) || ( ( Coeff1 == 3 ) && ( Coeff2 == 2 ) ) )// ---- 3*Pi/2 ----
    s_GlobalInvalid = s_TanCotError = true;
  if( ( ( Coeff1 == 5 ) && ( Coeff2 == 3 ) )  )              // ---- 5*Pi/3 ----
     PiRes = new TUnar( new TRoot( new TConstant( 3 ), 2 ) );
  if( ( Coeff1 == 1.75 ) || ( ( Coeff1 == 7 ) && ( Coeff2 == 4 ) ) )// ---- 7*Pi/4 ----
     PiRes = new TUnar( new TConstant( 1 ) );
  if( ( ( Coeff1 == 11 ) && ( Coeff2 == 6 ) ) )              // ---- 11*Pi/6 ----
     PiRes = new TUnar( new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) ) );
  if( ( Coeff1 == 2 ) && ( Coeff2 == 1 )  )                // ---- 2*Pi ----
     PiRes = new TConstant( 0 );

  if( !PiRes.IsEmpty() && SignNeg && !PiRes.Unarminus( PiRes ) )
      PiRes = TUnar( PiRes ).Reduce();
  return !PiRes.IsEmpty();
  }

bool PiCotStandard( double Coeff1, double Coeff2, MathExpr& PiRes )
  {
  auto Inverse = [] ( MathExpr& ex ) -> MathExpr
    {
    MathExpr exNom, exDenom, exTmp;
    if( ex.Divis( exNom, exDenom ) )
      {
      if( exNom == 1 ) return exDenom;
      exTmp = exNom;
      return exDenom / exTmp;
      }
    else
      return Constant( 1 ) / ex;
    };

  bool Result = PiTanStandard( Coeff1, Coeff2, PiRes );
  MathExpr exArg;
  if( Result )
    {
    if( PiRes.Unarminus( exArg ) )
      {
      if( exArg != 1 )
        PiRes = new TUnar( Inverse( exArg ) );
      }
    else
      if( PiRes != 1 ) PiRes = Inverse( PiRes );
    }
  else
    {
    Result = true;
    s_GlobalInvalid = false;
    PiRes = new TConstant( 0 );
    }
  return Result;
  }

bool DegSinStandard( double V, MathExpr& DegRes )
  {
  DegRes = MathExpr( nullptr );
  while( ( V > 2 * 180 ) ) V = V - 2 * 180;
  while( ( V < 0 ) ) V = V + 2 * 180;
  bool Result = Round( V ) == V;
  if( Result )
    switch( Round( V ) )
    {
      case 0:
        DegRes = new TConstant( 0 );
        break;
      case 30:
        DegRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
        break;
      case 45:
        DegRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
        break;
      case 60:
        DegRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
        break;
      case 90:
        DegRes = new TConstant( 1 );
        break;
      case 120:
        DegRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
        break;
      case 135:
        DegRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
        break;
      case 150:
        DegRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
        break;
      case 180:
        DegRes = new TConstant( 0 );
        break;
      case 210:
        DegRes = new TDivi( new TConstant( -1 ), new TConstant( 2 ) );
        break;
      case 225:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 240:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 270:
        DegRes = new TConstant( -1 );
        break;
      case 300:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 315:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 330:
        DegRes = new TDivi( new TConstant( -1 ), new TConstant( 2 ) );
        break;
      case 360:
        DegRes = new TConstant( 0 );
    }
  return !DegRes.IsEmpty();
  }

bool DegCosStandard( double V, MathExpr& DegRes )
  {
  DegRes = MathExpr( nullptr );
  while( ( V > 2 * 180 ) ) V = V - 2 * 180;
  while( ( V < 0 ) ) V = V + 2 * 180;
  bool Result = Round( V ) == V;
  if( Result )
    switch( Round( V ) )
    {
      case 0:
        DegRes = new TConstant( 1 );
        break;
      case 30:
        DegRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
        break;
      case 45:
        DegRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
        break;
      case 60:
        DegRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
        break;
      case 90:
        DegRes = new TConstant( 0 );
        break;
      case 120:
        DegRes = new TDivi( new TConstant( -1 ), new TConstant( 2 ) );
        break;
      case 135:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 150:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 180:
        DegRes = new TConstant( -1 );
        break;
      case 210:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 225:
        DegRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
        break;
      case 240:
        DegRes = new TDivi( new TConstant( -1 ), new TConstant( 2 ) );
        break;
      case 270:
        DegRes = new TConstant( 0 );
        break;
      case 300:
        DegRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
        break;
      case 315:
        DegRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
        break;
      case 330:
        DegRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
        break;
      case 360:
        DegRes = new TConstant( 1 );
    }
  return !DegRes.IsEmpty();
  }

bool PiCosStandard( double Coeff1, double Coeff2, MathExpr& PiRes )
  {
  PiRes = MathExpr( nullptr );
  Coeff1 = abs( Coeff1 );
  Coeff2 = abs( Coeff2 );
  while( abs( Coeff1 / Coeff2 ) > 2 ) Coeff1 = Coeff1 - 2 * Coeff2;

  if( ( ( Coeff1 == 1 ) && ( Coeff2 == 6 ) ) )               // ---- Pi/6 ----
    PiRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
  if( ( Coeff1 == 0.25 ) || ( ( Coeff1 == 1 ) && ( Coeff2 == 4 ) ) ) // ---- Pi/4 ----
    PiRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
  if( ( ( Coeff1 == 1 ) && ( Coeff2 == 3 ) ) )               // ---- Pi/3 ----
    PiRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
  if( ( Coeff1 == 0.5 ) || ( ( Coeff1 == 1 ) && ( Coeff2 == 2 ) ) ) // ---- Pi/2 ----
    PiRes = new TConstant( 0 );
  if( ( ( Coeff1 == 2 ) && ( Coeff2 == 3 ) ) )               // ---- 2*Pi/3 ----
    PiRes = new TUnar( new TDivi( new TConstant( 1 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 0.75 ) || ( ( Coeff1 == 3 ) && ( Coeff2 == 4 ) ) )// ---- 3*Pi/4 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
  if( ( ( Coeff1 == 5 ) && ( Coeff2 == 6 ) ) )               // ---- 5*Pi/6 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 1 ) && ( Coeff2 == 1 ) )                 // ---- Pi ----
    PiRes = new TUnar( new TConstant( 1 ) );
  if( ( ( Coeff1 == 7 ) && ( Coeff2 == 6 ) ) )               // ---- 7*Pi/6 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 1.25 ) || ( ( Coeff1 == 5 ) && ( Coeff2 == 4 ) ) )// ---- 5*Pi/4 ----
    PiRes = new TUnar( new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) ) );
  if( ( ( Coeff1 == 4 ) && ( Coeff2 == 3 ) ) )               // ---- 4*Pi/3 ----
    PiRes = new TUnar( new TDivi( new TConstant( 1 ), new TConstant( 2 ) ) );
  if( ( Coeff1 == 1.5 ) || ( ( Coeff1 == 3 ) && ( Coeff2 == 2 ) ) )// ---- 3*Pi/2 ----
    PiRes = new TConstant( 0 );
  if( ( ( Coeff1 == 5 ) && ( Coeff2 == 3 ) ) )               // ---- 5*Pi/3 ----
    PiRes = new TDivi( new TConstant( 1 ), new TConstant( 2 ) );
  if( ( Coeff1 == 1.75 ) || ( ( Coeff1 == 7 ) && ( Coeff2 == 4 ) ) )// ---- 7*Pi/4 ----
    PiRes = new TDivi( new TRoot( new TConstant( 2 ), 2 ), new TConstant( 2 ) );
  if( ( ( Coeff1 == 11 ) && ( Coeff2 == 6 ) ) )             // ---- 11*Pi/6 ----
    PiRes = new TDivi( new TRoot( new TConstant( 3 ), 2 ), new TConstant( 2 ) );
  if( ( Coeff1 == 2 ) && ( Coeff2 == 1 ) )                 // ---- 2*Pi ----
    PiRes = new TConstant( 1 );
  return !PiRes.IsEmpty();
  }

bool DegTanStandard( double V, MathExpr& DegRes )
  {
  DegRes = MathExpr( nullptr );
  while( ( V > 2 * 180 ) ) V = V - 2 * 180;
  while( ( V < 0 ) ) V = V + 2 * 180;
  bool Result = Round( V ) == V;
  if( Result )
    switch( Round( V ) )
    {
      case 0:
        DegRes = new TConstant( 0 );
        break;
      case 30:
        DegRes = new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) );
        break;
      case 45:
        DegRes = new TConstant( 1 );
        break;
      case 60:
        DegRes = new TRoot( new TConstant( 3 ), 2 );
        break;
      case 90:
        s_GlobalInvalid = s_TanCotError = true;
        break;
      case 120:
        DegRes = new TUnar( new TRoot( new TConstant( 3 ), 2 ) );
        break;
      case 135:
        DegRes = new TConstant( -1 );
        break;
      case 150:
        DegRes = new TDivi( new TConstant( -1 ), new TRoot( new TConstant( 3 ), 2 ) );
        break;
      case 180:
        DegRes = new TConstant( 0 );
        break;
      case 210:
        DegRes = new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) );
        break;
      case 225:
        DegRes = new TConstant( 1 );
        break;
      case 240:
        DegRes = new TRoot( new TConstant( 3 ), 2 );
        break;
      case 270:
        s_GlobalInvalid = s_TanCotError = true;
        break;
      case 300:
        DegRes = new TUnar( new TRoot( new TConstant( 3 ), 2 ) );
        break;
      case 315:
        DegRes = new TConstant( -1 );
        break;
      case 330:
        DegRes = new TDivi( new TConstant( -1 ), new TRoot( new TConstant( 3 ), 2 ) );
        break;
      case 360:
        DegRes = new TConstant( 0 );
    }
  return !DegRes.IsEmpty();
  }

bool DegCotStandard( double V, MathExpr& DegRes )
  {
  DegRes = MathExpr( nullptr );
  while( ( V > 2 * 180 ) ) V = V - 2 * 180;
  while( ( V < 0 ) ) V = V + 2 * 180;
  bool Result = Round( V ) == V;
  if( Result )
    switch( Round( V ) )
    {
    case 0:
      s_GlobalInvalid = s_TanCotError = true;
      break;
    case 30:
      DegRes = new TRoot( new TConstant( 3 ), 2 );
      break;
    case 45:
      DegRes = new TConstant( 1 );
      break;
    case 60:
      DegRes = new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) );
      break;
    case 90:
      DegRes = new TConstant( 0 );
      break;
    case 120:
      DegRes = new TDivi( new TConstant( -1 ), new TRoot( new TConstant( 3 ), 2 ) );
      break;
    case 135:
      DegRes = new TConstant( -1 );
      break;
    case 150:
      DegRes = new TUnar( new TRoot( new TConstant( 3 ), 2 ) );
      break;
    case 180:
      s_GlobalInvalid = s_TanCotError = true;
      break;
    case 210:
      DegRes = new TRoot( new TConstant( 3 ), 2 );
      break;
    case 225:
      DegRes = new TConstant( 1 );
      break;
    case 240:
      DegRes = new TDivi( new TConstant( 1 ), new TRoot( new TConstant( 3 ), 2 ) );
      break;
    case 270:
      DegRes = new TConstant( 0 );
      break;
    case 300:
      DegRes = new TDivi( new TConstant( -1 ), new TRoot( new TConstant( 3 ), 2 ) );
      break;
    case 315:
      DegRes = new TConstant( -1 );
      break;
    case 330:
      DegRes = new TUnar( new TRoot( new TConstant( 3 ), 2 ) );
      break;
    case 360:
      s_GlobalInvalid = s_TanCotError = true;
    }
  return !DegRes.IsEmpty();
  }

double PowerI( double x, int y )
  {
  int  i;
  double pow;

  if( x == 0 )
    if( y > 0 )
      return 0;
    else
      {
      s_GlobalInvalid = true;
      Error_m( X_Str( "MBaseIs0", "Base of{ power is 0!" ) );
      return 71;
      }

  if( x == 1 )
    return 1;

  if( x == -1 )
    if( ( y % 2 ) == 0 )
      return 1;
    else
      return -1;

  if( log( abs( x ) )*y > 320 * log( 10 ) )
    {
    s_GlobalInvalid = true;
    if( !s_CalcOnly )
      Error_m( X_Str( "MTooBigNumber", "Number too big!" ) );
    return 68;
    }

  if( y == 0 && x != 0 )
    return 1;

  if( log( abs( x ) )*y < -35 * log( 10 ) )
    return 0;
  pow = 1;

  for( i = 1; i <= abs( y ); i++ )
    pow *= x;

  if( y < 0 )
    return 1 / pow;
  return pow;
  }

double PowerR( double x, double y )
  {
  if( x == 0 )
    {
    s_GlobalInvalid = true;
    Error_m( X_Str( "MBaseIs0", "Base of{ power is 0!" ) );
    return 71;
    }

  if( x < 0 )
    {
    s_GlobalInvalid = true;
    Error_m( X_Str( "MBaseLess0", "Negative base of{ power!" ) );
    return 70;
    }

  if( x == 1 )
    return 1;

  if( log( abs( x ) )*y > 320 * log( 10 ) )
    {
    s_GlobalInvalid = true;
    if( !s_CalcOnly )
      Error_m( X_Str( "MTooBigNumber", "Number too big!" ) );
    return 68;
    }

  if( log( abs( x ) )*y < -35 * log( 10 ) )
    return 0;

  return exp( y*log( abs( x ) ) );
  }

double PowerF( double x, int n, int d )
  {
  double y, pow;
  int i;

  y = ( double ) n / d;
  if( x == 0 )
    {
    if( y > 0 )
      return 0;
    s_GlobalInvalid = true;
    Error_m( X_Str( "MBaseIs0", "Base of{ power is 0!" ) );
    return 71;
    }

  if( x == 1 )
    return 1;

  if( log( abs( x ) )*y > 320 * log( 10 ) )
    {
    s_GlobalInvalid = true;
    if( !s_CalcOnly )
      Error_m( X_Str( "MTooBigNumber", "Number too big!" ) );
    return 68;
    }

  if( log( abs( x ) )*y < -35 * log( 10 ) )
    return 0;

  pow = 1;

  for( i = 1; i <= abs( n ); i++ )
    pow *= x;

  if( abs( d ) % 2 == 0 && pow < 0 )
  {
      s_LastError="MArgRootLess0";
      s_GlobalInvalid=true;
      return 0;
  }

  if( n*d < 0 )
    pow = 1 / pow;
  d = abs( d );
  if( pow>0 )
    return exp( log( pow ) / d );
  return -exp( log( -pow ) / d );
  }

MathExpr Expand( MathExpr Exi0 )
  {
  Exi0.TestPtr();
  auto BreakInfiniteLoop = [&] ( MathExpr Exi )
    {
    if( Exi.Equal( Exi0 ) )
      {
      s_InvalidExpand = true;
      return Exi0;
      }
    return Exi;
    };

  MathExpr Op11, Op12, Op21, Op22, Exi, P;
  int i;
  bool OldSummExpFactorize;
  double Rconst;

  if( !( Exi0.Power( Op11, Op12 ) && Op12.Cons_int( i ) && ( i > 3 ) ) )
    {
    if( IsConstType( TIntegral, Exi0 ) ) return Exi0;
    OldSummExpFactorize = s_SummExpFactorize;
    s_SummExpFactorize = false;
    Exi = Exi0.Reduce();
    s_SummExpFactorize = OldSummExpFactorize;
    if( Exi.Unarminus( Op11 ) && !Exi0.Unarminus( Op11 ) ) Exi = Exi0;
    }
  else
    Exi = Exi0;

  if( Exi.Constan( Rconst ) )
    {
    s_InvalidExpand = Exi.Eq( Exi0 );
    return Exi;
    }

  if( Exi.Unarminus( P ) )
    {
    if( P.Summa( Op11, Op12 ) )
      {
      s_InvalidExpand = false;
      return (-Op11 - Op12);
      }

    if( P.Subtr( Op11, Op12 ) )
      {
      s_InvalidExpand = false;
      return (Op12 - Op11);
      }

    if( P.Unarminus( Op11 ) )
      {
      s_InvalidExpand = false;
      return Op11;
      }
    return -Expand( P );
    }

  int St = 0;
  MathExpr SbSm, Pw, A, B;

  if( Exi.Power( SbSm, Pw ) && Pw.Cons_int( St ) && St > 0 && ( SbSm.Subtr( A, B ) || SbSm.Summa( A, B ) ) && s_ExpandPower )
    {
    MathExpr Ex = A ^ St, Mult1, Mult2;
    int c = 1;
    for( int i = 1; i <= St; i++ )
      {
      c = c * ( St - i + 1 ) / i;
      if( St - i == 0 )
        Mult1 = 1;
      else
        if( St - i == 1 )
          Mult1 = A;
        else
          Mult1 = A ^ ( St - i );
      if( i != 1 )
        Mult2 = B ^ i;
      else
        Mult2 = B;
      MathExpr Ex1 = ( Constant( c ) * Mult1 * Mult2 ).Reduce();
      if( SbSm.Summa( A, B ) || i % 2 == 0 )
        Ex += Ex1;
      else
        Ex -= Ex1;
      }
    return Ex;
    }

  MathExpr Arg1, Arg2;

  if( St > 0 )
    {
    if( SbSm.Multp( Arg1, Arg2 ) )
      return ( ( Arg1 ^ Pw )  * ( Arg2 ^ Pw ) ).Reduce();
    if( SbSm.Divis( Arg1, Arg2 ) )
      return ( Expand( Arg1 ^ Pw ) / Expand( Arg2 ^ Pw ) );
    }

  if( Exi.Summa( Arg1, Arg2 ) )
    {
    int Flag1 = 0, Flag2 = 0;
    MathExpr Opr1 = Expand( Arg1 );
    if( !s_InvalidExpand || !Opr1.Eq( Arg1 ) )
      Flag1 = 1;
    MathExpr Opr2 = Expand( Arg2 );
    if( !s_InvalidExpand || !Opr2.Equal( Arg2 ) )
      Flag2 = 1;
    if( ( Flag1 + Flag2 ) == 0 )
      {
      s_InvalidExpand = true;
      return Exi;
      }
    s_InvalidExpand = false;
    return (Opr1 + Opr2);
    }

  if( Exi.Subtr( Arg1, Arg2 ) )
    {
    int Flag1 = 0, Flag2 = 0;
    MathExpr Opr1 = Expand( Arg1 );
    if( !s_InvalidExpand || !Opr1.Eq( Arg1 ) )
      Flag1 = 1;
    MathExpr Opr2 = Expand( Arg2 );
    if( !s_InvalidExpand || !Opr2.Equal( Arg2 ) )
      Flag2 = 1;

    if( Opr2.Summa( Op21, Op22 ) )
      return (Opr1 - Op21 - Op22);

    if( Opr2.Subtr( Op21, Op22 ) )
      return (Opr1 - Op21 + Op22);

    if( Flag1 + Flag2 == 0 )
      {
      s_InvalidExpand = true;
      return Exi;
      }
    s_InvalidExpand = false;
    return BreakInfiniteLoop( Opr1 - Opr2 );
    }

  if( Exi.Multp( Arg1, Arg2 ) )
    {
    MathExpr Opr1 = Expand( Arg1 );
    MathExpr Opr2 = Expand( Arg2 );
    if( Opr1.Summa( Op11, Op12 ) )
      {
      s_InvalidExpand = false;
      return BreakInfiniteLoop( Op11.Reduce() * Opr2.Reduce() + Op12.Reduce() *  Opr2.Reduce() );
      }
    if( Opr2.Summa( Op21, Op22 ) )
      {
      s_InvalidExpand = false;
      return BreakInfiniteLoop( Opr1.Reduce() * Op21.Reduce() + Opr1.Reduce() *  Op22.Reduce() );
      }
    if( Opr1.Subtr( Op11, Op12 ) )
      {
      s_InvalidExpand = false;
      return BreakInfiniteLoop( Op11.Reduce() * Opr2.Reduce() - Op12.Reduce() *  Opr2.Reduce() );
      }
    if( Opr2.Subtr( Op21, Op22 ) )
      {
      s_InvalidExpand = false;
      return BreakInfiniteLoop( Opr1.Reduce() * Op21.Reduce() - Opr1.Reduce() *  Op22.Reduce() );
      }

    uchar BinSign;
    if( Opr1.Binar_( BinSign, Op11, Op12 ) && ( BinSign == msPlusMinus || BinSign == msMinusPlus ) )
      {
      s_InvalidExpand = false;
      return BreakInfiniteLoop( new TBinar( BinSign, Op11.Reduce() * Opr2.Reduce(), Op12.Reduce() * Opr2.Reduce() ) );
      }
    if( Opr2.Binar_( BinSign, Op21, Op22 ) && BinSign == msPlusMinus )
      {
      s_InvalidExpand = false;
      return BreakInfiniteLoop( new TBinar( BinSign, Op21.Reduce() * Opr1.Reduce(), Op22.Reduce() * Opr1.Reduce() ) );
      }
    return ( Opr1 * Opr2 );
    }

  if( Exi.Binar( '=', Op11, Op12 ) )
    return new TBinar( '=', Expand( Op11 ), Expand( Op12 ) );

  if( s_ExpandLog && Exi.Log( Op11, Op12 ) && Op12.Power( Op21, Op22 ) )
    return Expand( Op11.Log( Op21 ) * Op22 );

  if( Exi.Root_( Op11, Op12, St ) )
    {
    MathExpr P = Expand( Op11 );
    if( !P.Eq( Op11 ) )
      Exi = P.Root( St );
    }

  if( Exi.Divis( Op11, Op12 ) )
    {
    s_GlobalInvalid = false;
    MathExpr P = SubtSq( Op11 );
    int ReduceDivi = 0;
    if( !s_GlobalInvalid )
      {
      ReduceDivi = 1;
      Op11 = P;
      if( IsConstType( TBinar, P ) )
        while( Op11.Binar( '=', A, Op11 ) );
      }
    s_GlobalInvalid = false;
    P = SubtSq( Op12 );
    if( !s_GlobalInvalid )
      {
      ReduceDivi++;
      Op12 = P;
      if( IsConstType( TBinar, P ) )
        while( Op12.Binar( '=', A, Op12 ) );
      }
    if( ReduceDivi == 1 )
      {
      A = Op11 / Op12;
      P = A.Reduce();
      if( !P.Equal( A ) )
        return P;
      }
    if( ReduceDivi == 2 )
      {
      Exi.Divis( Op21, Op22 );
      A = Op21 / Op12;
      P = A.Reduce();
      if( !P.Equal( A ) )
        return P;
      A = Op11 / Op22;
      P = A.Reduce();
      if( !P.Equal( A ) )
        return P;
      }
    }

  if( Exi.Equal( Exi0 ) )
    {
    s_InvalidExpand = true;
    return Exi0;
    }
  s_GlobalInvalid = false;
  s_InvalidExpand = false;
  return Exi;
  }

MathExpr SubtSq( MathExpr Exi )
  {
  Exi.TestPtr();
  bool WasReduced = false, OldNoRootReduce;

  std::function< MathExpr( const MathExpr& ex )> Sqrt = [&] ( const MathExpr& Ex )
    {
    MathExpr Ex1, Ex2, Ex3, Ex4, Result;
    int N;
    if( Ex.Multp( Ex1, Ex2 ) )
      Result = Sqrt( Ex1 ) * Sqrt( Ex2 );
    else
      {
      Ex1 = Ex.Root( 2 );
      Result = Ex1.Reduce();
      int N;
      WasReduced = WasReduced || ( IsConstType( TVariable, Ex ) ) || !Result.Eq( Ex1 ) && (!Ex.Power( Ex3, Ex4 ) || !Ex4.Cons_int( N ) || N != 3);
      }
    return Result;
    };

  try
    {
    MathExpr Exi1, Exi2, exSqr1, exSqr2, exFirst;
    if( Exi.Subtr( Exi1, Exi2 ) )
      {
      OldNoRootReduce = s_NoRootReduce;
      s_NoRootReduce = true;
      exSqr1 = Sqrt( Exi1 );
      if( WasReduced )
        {
        WasReduced = false;
        exSqr2 = Sqrt( Exi2 );
        }
      s_NoRootReduce = OldNoRootReduce;
      if( WasReduced )
        {
        exFirst = ( exSqr1 ^ 2 ) - ( exSqr2 ^ 2 );
        s_XPStatus.SetMessage( X_Str( "MFactorized", "Factorized!" ) );
        if( Exi.Equal( exFirst ) )
          return new TBinar( '=', Exi, ( exSqr1 + exSqr2 ).Reduce() * ( exSqr1 - exSqr2 ).Reduce() );
        return new TBinar( '=', Exi, new TBinar( '=', exFirst, ( exSqr1 + exSqr2 ).Reduce() * ( exSqr1 - exSqr2 ).Reduce() ) );
        }
      }
    }
  catch( char * ){}
  s_LastError = X_Str( "MCanNotFactor", "I can`t factor it!" );
  s_GlobalInvalid = true;
  return Exi;
  }

int LComMul( int x, int y )
  {
  int n = GrComDiv( x, y );
  if( n == 0 ) return 0;
  int m1 = y / n;
  int m2 = x / n;
  return abs( m1 * m2 * n );
  }

MathExpr ExpandExpr( MathExpr exi )
  {
  MathExpr exi_E = Expand( exi );
  MathExpr P = Expand( exi_E );
  int Count = 0;
  while( !P.Eq( exi_E ) || !s_GlobalInvalid )
    {
    if( P.Eq( exi_E ) )
      Count = Count + 1;
    else
    Count = 0;
    if( Count == 2 ) break;
    exi_E = P;
    P = Expand( exi_E );
    }
  s_GlobalInvalid = false;
  P = exi_E.Reduce();
//  P = DeletBR( P );
  return P;
  }

void FactNumber( int NumOrg, int& NumFact, TListOfInt& NumLst )
  {
  NumLst = TListOfInt();
  if( NumOrg == 0 )
    {
    s_GlobalInvalid = true;
    NumLst.Add_element( 1, 0 );
    NumFact = 0;
    return;
    }

  if( abs( NumOrg ) == 1 )
    {
    NumLst.Add_element( 1, 1 );
    NumFact = 1;
    return;
    }

  QVector<int> PrFact, PowFact;
  int j = 2, a = abs( NumOrg );
  do
    {
    int k = 0;
    for( ; a % j == 0; k++, a /= j );
    if( k != 0 )
      {
      PrFact.append( j );
      PowFact.append( k );
      }

    if( j > 2 )
      j += 2;
    else
      j = 3;
    } while( a != 1 );

    NumFact = PrFact.count();
    for( int j = 0; j < NumFact; j++ )
      NumLst.Add_element( PrFact[j], PowFact[j] );
  }

void ConvertDecCom( double x, int& i, int& n, int& d, int& nr, int& dr )
  {
  int y = x;
  i = Round( y );

  if( abs( x - y ) < 0.00001 )
    {
    n = i;
    d = 1;
    nr = n;
    dr = d;
    i = 0;
    return;
    }

  if( y > 1000000 )
    {
    if( !s_CalcOnly )
      throw ErrParser( X_Str( "MTooBigNumber", "Number too big!" ), peNewErr );
    n = i; d = 1; i = 0;
    nr = n; dr = d;
    return;
    }

  int sign = 1;
  if( x < 0 ) sign = -1;
  QByteArray DecNum = QByteArray::number( x, 'f', 9 );
  int poz = DecNum.indexOf( '.' );
  QByteArray FrPart = DecNum.mid( poz + 1);
  int l = FrPart.count() - 1;
  for( ; l >= 0 && FrPart[l] == '0'; l-- );
  if( l > 0 && l == FrPart.count() - 1 ) l--;
  double prec = TExpr::sm_Accuracy;
//  double prec = TExpr::Precision();
  if( pow( 10, -l ) - prec < prec )
    {
    int nPeriod = 0;
    bool bPeriodical = false;
    QByteArray sPeriod;
    int nLeft;
    while( !bPeriodical && ( nPeriod < l / 2 ) )
      {
      nPeriod++;
      nLeft = -1;
      while( !bPeriodical && nLeft < l - 2 * nPeriod )
        {
        nLeft++;
        sPeriod = FrPart.mid( nLeft, nPeriod );
        int k = nLeft + nPeriod + 1;
        while( k <= l )
          {
          if( sPeriod != FrPart.mid(k - 1, nPeriod ) ) break;
          k = k + nPeriod;
          }
        bPeriodical = k > l;
        }
      }
    if( bPeriodical )
      {
      n = sPeriod.toInt();
      QByteArray sDenom = "";
      for( int k = 1; k <= nPeriod; k++ )
        sDenom += '9';
      d = sDenom.toInt();
      if( nLeft > 0 )
        {
        n += FrPart.left( nLeft ).toInt() * d;
        for( int k = 1; k <= nLeft; k++ )
          d *= 10;
        }
      nr = n;
      dr = d;
      CancFrac( nr, dr );
      nr *= sign;
      return;
      }
    }
  l = FrPart.count();
  int a = FrPart.toInt();
  while( a % 10 == 0 )
    {
    a /= 10;
    l--;
    }

  int b = 1;
  for( int k = 1; k <= l; b *= 10, k++ );
  n = a; d = b;
  if( b > 10000 )
    {
    b /= 10000;
    CancFrac( a, b );
    b *= 10000;
    }
  else
    CancFrac( a, b );
  nr = a * sign; dr = b;
  }

void ConvertComDec( int i, int n, int d, double& x, QByteArray& StDecNum )
  {
  if( i >= 0 )
    x = i + ( double ) n / d;
  else
    x = i - ( double ) n / d;
  int a = x;
  QByteArray StInPart = QByteArray::number( a );
  if( abs( a - x ) < 0.00001 )
    StDecNum = QByteArray::number( x, 'f', 1 );
  else
    StDecNum = QByteArray::number( x, 'f', 9 );
  int b = n - ( n / d )*d;
  if( b == 0 ) return;
  int l = StDecNum.count();
  int poz = StDecNum.indexOf( '.' );
  StInPart = StDecNum.left( poz );
  double x1 = abs( ( double ) b / d );
  QByteArray StFrPart = QByteArray::number( x1, 'f', 12 );
  StFrPart = StFrPart.mid( 2, 9 );
  int m = StFrPart.count();
  a = StFrPart.toInt();
  l = m;
  while( a % 10 == 0 )
    {
    a /= 10;
    l--;
    }
  if( l != m )
    {
    StFrPart = StFrPart.left( l );
    StDecNum = StInPart + '.' + StFrPart;
    return;
    }
  else
    {
    StDecNum = StInPart + '.' + StFrPart;
    const int LenD = 22;
    int rmd[LenD], frd[LenD];
    rmd[1] = b; b = b * 10;
    bool Break = false;
    for( int k = 2; k < LenD; k++ )
      {
      rmd[k] = b % d;
      frd[k - 1] = b / d;
      for( int j = k - 1; k > 0; k-- )
        if( rmd[k] == rmd[j] )
          {
          l = j; m = k;
          Break = true;
          break;
          }
      if( Break ) break;
      b = ( b - ( b / d )*d ) * 10;
      }
    if( m > l )
      {
      QByteArray StPrPart = StFrPart.mid( l - 1, m - l );
      StFrPart = StFrPart.left( l - 1 );
      StDecNum = StInPart + '.' + StFrPart + '(' + StPrPart + ')';
      }
    }
  }

void ConvertRoot( int n, int arg, int& Extern, int& Intern )
  {
  auto Errata = [&] ()
    {
    s_CheckError = true;
    Intern = sc_NumError;
    Extern = sc_NumError;
    };

  auto Root = [&]( int n, double ag )
    {
    ag = abs( ag );
    if( ag == 0 )
      return 0.0;
    return exp( log( ag ) / n );
    };

  if( n == 0 )
    {
    Errata();
    return;
    }

  if( arg == 0 )
    {
    Intern = 1;
    Extern = 0;
    return;
    }

  if( arg == 1 )
    {
    Intern = 1;
    Extern = 1;
    return;
    }

  int  a, b;
  if( arg < 0 )
    if( n % 2 == 0 )
      {
      Errata();
      return;
      }
    else
      a = abs( arg );
  else
  a = arg;

  b = 1;
  double res = Root( n, a );
  while( b < arg && abs( res - (int) res ) > 0.0000001 )
    {
    b++;
    if( ( arg % b ) != 0 )
      continue;
    res = Root( n, ( double ) arg / b );
    }
  a = Round( res );
  if( arg < 0 )
    Extern = -a;
  else
  Extern = a;
  Intern = b;
  }

MathExpr CalcDeg2Rad( const MathExpr& exi )
  {
  double SavePrec = s_Precision;
  s_Precision = 0.0000000001;
  bool FullReduce = TExpr::sm_FullReduce;
  TExpr::sm_FullReduce = true;
  MathExpr exo = exi.Reduce();
  s_Precision = SavePrec;
  TExpr::sm_FullReduce = FullReduce;
  double V;
  MathExpr op1, op2;
  MathExpr Result;
  if( exo.Constan( V ) )
    Result = new TBinar( '=', new TMeaExpr( exo, Variable( msDegree ) ), Constant( ( V / 180 )*M_PI ) );
  else
    if( exo.Measur_( op1, op2 ) )
      {
      op1.Constan( V );
      Result = new TBinar( '=', exo, Constant( ( V / 180 )*M_PI ) );
      }
    else
      {
      s_LastError = X_Str( "MEnterConst", "Enter constant expression!" );
      return Result;
      }
  if( !exo.Eq( exi ) )
    Result = new TBinar( '=', exi, Result );
  s_XPStatus.SetMessage(X_Str( "MConverting", "Conversion" ));
  return Result;
  }

MathExpr CalcRad2Deg( const MathExpr& exi )
  {
  double SavePrec = s_Precision;
  s_Precision = 0.0000000001;
  bool FullReduce = TExpr::sm_FullReduce;
  TExpr::sm_FullReduce = true;
  MathExpr exo = Parser::StrToExpr( PiVar2PiConst( exi.WriteE() ) ).ReduceTExprs();
  s_Precision = SavePrec;
  TExpr::sm_FullReduce = FullReduce;
  double V;
  MathExpr Result;
  if( !exo.Constan( V ) )
    {
    s_LastError = X_Str( "MEnterConst", "Enter constant expression!" );
    return Result;
    }
  Result = new TBinar( '=', exo, new TMeaExpr( Constant( ( V / M_PI ) * 180 ), Variable( msDegree ) ) );
  if( !exo.Eq( exi ) )
    Result = new TBinar( '=', exi, Result );
  s_XPStatus.SetMessage( X_Str( "MConverting", "Conversion" ) );
  return Result;
  }
