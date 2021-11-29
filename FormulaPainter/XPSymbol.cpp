#include "XPSymbol.h"
//#include <mbstring.h>
#include <ctype.h>
#include "XPTypes.h"
#include "StrHebEng.h"

TexNames TexNames::Symbols;

bool In( uchar C, uchar *S )
  {
  return strchr((const char*) S, C ) != NULL;
  }

bool _operation( uchar C )
  {
  uchar S[] = {  msMultSign1,  msMultSign2, '/', '^', '+', '-', ':', '|', '!',  msPlusMinus,  msMinusPlus, '~', '%', msDegree, 0 };
  return In( C, S ); 
  }

bool _relation( uchar C)
  {
  uchar S[] = {  msNotequal,  msMinequal,  msAround,  msParallel, '=', '>', '<',  msMaxequal,  msRound,  msCross,  msAddition, 0 };
  return In( C, S ); 
  }

bool _litera( uchar C )
  {
  uchar S[] = {  msMean,  msGenQuantifier,  msExQuantifier,  msHighReg,  msBird,  msTriangle,  ms3Points, msPartialDiff, 0 };
  return isalpha( C ) || C >=  msBigAlpha && C <=  msOmega || In( C, S ) || IsHebChar(C);
  }

bool _bracket( uchar C )
  {
	return In( C, "()[]{}`'\"" );
  }

bool _separator( uchar C )
  {
	return In( C, ",;?!:." );
  }

bool _printable( uchar C )
  {
  uchar S[] = { ' ', '.',  msMetaSign,  msCharNewLine, msIdle, 0 };
  return _litera( C ) || _infinity( C ) || isdigit( C ) || _operation( C ) || _relation( C ) || _bracket( C ) ||
    _separator( C ) || In( C, S );
  }

bool _start_ident( uchar C )
  {
  return _litera( C ) || C ==  msMetaSign || C ==  msInvisible;
  }

bool _inside_ident( uchar C )
  {
  return _start_ident( C ) || isdigit( C ) || C ==  msLowReg || C ==  msHighReg;
  }

SetOfChars::SetOfChars( const uchar *Set )
  {
  for( const uchar *pC = Set; *pC != 0; pC++ )
    {
    insert( *pC );
    if( pC[1] == '.' && pC[2] == '.' )
      {
      for( uchar Char = *pC + 1; Char <= pC[3]; insert( Char++ ) );
      pC += 3;
      }
    }
  }

SetOfChars::SetOfChars( const std::initializer_list<uchar> &Set )
  {
  for( auto pC = Set.begin(); pC != Set.end(); pC++ )
    {
    insert( *pC );
    if( pC[1] == '.' && pC[2] == '.' )
      {
      for( uchar Char = *pC + 1; Char <= pC[3]; insert( Char++ ) );
      pC += 3;
      }
    }
  }

QByteArray charToTex( uchar Ch )
  {
  QByteArray Result = TexNames::Symbols.key( Ch );
  if( Result.isEmpty() ) return SimpleChar( Ch );
  return '\\' + Result;
  }

bool SplitOperSign( uchar SignChar )
  {
  uchar S[] = { ',', '+', '-',  msMultSign1,  msMultSign2, '|',  msPlusMinus,  msMinusPlus };
  return In( SignChar, S ) || _relation( SignChar );
  }

TexNames::TexNames()
  {
  ( *this )["alpha"] = msAlpha;
  ( *this )["beta"] = msBetta;
  ( *this )["gamma"] = msGamma;
  ( *this )["delta"] = msDelta;
  (*this)["epsilon"] = msEpsilon;
  ( *this )["dzeta"] = msDZeta;
  ( *this )["eta"] = msEta;
  ( *this )["theta"] = msTeta;
  ( *this )["iota"] = msIota;
  ( *this )["kappa"] = msKappa;
  ( *this )["lambda"] = msLambda;
  ( *this )["mu"] = msMu;
  ( *this )["nu"] = msNu;
  ( *this )["ksi"] = msKsi;
  ( *this )["omicron"] = msOmicron;
  ( *this )["pi"] = msPi;
  ( *this )["rho"] = msRo;
  ( *this )["sigmax"] = msSigmaX;
  ( *this )["sigma"] = msSigma;
  ( *this )["tau"] = msTau;
  ( *this )["ypsilon"] = msYpsilon;
  ( *this )["phi"] = msFi;
  ( *this )["hi"] = msHi;
  ( *this )["psi"] = msPsi;
  ( *this )["omega"] = msOmega;
  ( *this )["Alpha"] = msBigAlpha;
  ( *this )["Beta"] = msBigBetta;
  ( *this )["Gamma"] = msBigGamma;
  ( *this )["Delta"] = msBigDelta;
  (*this)["Epsilon"] = msBigEpsilon;
  ( *this )["Dzeta"] = msBigDZeta;
  ( *this )["Eta"] = msBigEta;
  ( *this )["Theta"] = msBigTeta;
  ( *this )["Iota"] = msBigIota;
  ( *this )["Kappa"] = msBigKappa;
  ( *this )["Lambda"] = msBigLambda;
  ( *this )["Mu"] = msBigMu;
  ( *this )["Nu"] = msBigNu;
  ( *this )["Ksi"] = msBigKsi;
  ( *this )["Omicron"] = msBigOmicron;
  ( *this )["Pi"] = msBigPi;
  ( *this )["Rho"] = msBigRo;
  ( *this )["Sigma"] = msBigSigma;
  ( *this )["Tau"] = msBigTau;
  ( *this )["Ypsilon"] = msBigYpsilon;
  ( *this )["Phi"] = msBigFi;
  ( *this )["Hi"] = msBigHi;
  ( *this )["Psi"] = msBigPsi;
  ( *this )["Omega"] = msBigOmega;
  ( *this )["deg"] = msDegree;
  ( *this )["min"] = msMinute;
  ( *this )["Im"] = msImUnit;
  ( *this )["sim"] = msRound;
  ( *this )["simeq"] = msAround;
  ( *this )["leq"] = msMinequal;
  ( *this )["geq"] = msMaxequal;
  ( *this )["infty"] = msInfinity;
  ( *this )["parallel"] = msParallel;
  ( *this )["perp"] = msCross;
  ( *this )["in"] = msAddition;
  ( *this )["neq"] = msNotequal;
  ( *this )["pm"] = msPlusMinus;
  ( *this )["mp"] = msMinusPlus;
  ( *this )["ldots"] = ms3Points;
  ( *this )["triangle"] = msTriangle;
  ( *this )["bird"] = msBird;
  ( *this )["invisible"] = msInvisible;
  ( *this )["arrowr"] = msArrowToRight;
  ( *this )["genquant"] = msGenQuantifier;
  ( *this )["exquant"] = msExQuantifier;
  ( *this )["union"] = msUnion;
  ( *this )["intersect"] = msIntersection;
  ( *this )["strictincl"] = msStrictInclusion;
  ( *this )["unstrictincl"] = msUnstrictInclusion;
  ( *this )["notbelong"] = msNotBelong;
  ( *this )["approx"] = msApprox;
  ( *this )["ident"] = msIdentity;
  ( *this )["arrowsup"] = msArrowsUp;
  ( *this )["arrowsupdown"] = msArrowsUpDown;
  ( *this )["dblarrow"] = msDoubleArrow;
  ( *this )["conjugate"] = msCConjugation;
  ( *this )["longminus"] = msLongMinus;
  ( *this )["longplus"] = msLongPlus;
  (*this)["cross"] = msMultSign2;
  }

QByteArray Encode( const QByteArray& Formula )
  {
  QByteArray Result;
  for( int i = 0; i < Formula.size(); i++ )
    {
    uchar C = Formula.at( i );
    if( C > 127 )
      {
      if( Result.isEmpty() )
        Result = Formula.left( i );
      Result += "!0x" + QByteArray::number( C, 16 ) + '!';
      continue;
      }
    else
      if( !Result.isEmpty() )
        Result += C;
    }
  if( Result.isEmpty() ) return Formula;
  return Result;
  }