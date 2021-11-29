#ifndef MATH_TOOL
#define MATH_TOOL
#include <qbytearray.h>
#include "ExpOb.h"
#define _USE_MATH_DEFINES
#include <math.h>

void CancFrac( int &n, int &d );
int GrComDiv( int x, int y );
inline double Frac( double V ) { return V - ( int ) V; }
inline double Trunc( double V ) { return ( int ) V; }
inline double DegToRad( double V ) { return V * M_PI / 180.0; }
inline double RadToDeg( double V ) { return V * 180.0 / M_PI; }
QByteArray PiVar2PiConst( const QByteArray& );
QByteArray PiVar2Pi180( const QByteArray& );
bool IsTrigonom( const QByteArray& );
bool PiProcess( const MathExpr& exi, double& Coeff1, double& Coeff2 );
bool PiSinStandard( double Coeff1, double Coeff2, MathExpr& PiRes );
bool PiCosStandard( double Coeff1, double Coeff2, MathExpr& PiRes );
bool PiTanStandard( double Coeff1, double Coeff2, MathExpr& PiRes );
bool PiCotStandard( double Coeff1, double Coeff2, MathExpr& PiRes );

bool DegSinStandard( double V, MathExpr& DegRes );
bool DegCosStandard( double V, MathExpr& DegRes );
bool DegTanStandard( double V, MathExpr& DegRes );
bool DegCotStandard( double V, MathExpr& DegRes );

inline bool MakeTrigonometric( const MathExpr& exi, MathExpr& exo, bool const IsPriv = false ) { return false; }
inline bool MakeSummaTrigonometric( const MathExpr& exi, MathExpr& exo ) { return false; }
inline bool MakeMultTrigonometric( const MathExpr& exi, MathExpr& exo ) { return false; }

inline TExpr* OutPutTrigonom( MathExpr& exi, MathExpr& exo ) { return nullptr; }

inline void Trigo2Str( MathExpr& Exo, MathExpr& Exi ) {}

inline TExpr* DegOutput( const MathExpr& exo ) { return nullptr; }
void SumFrac( int n1, int d1, int n2, int d2, int &nr, int &dr );
void MultFractions( int n1, int d1, int n2, int d2, int& nr, int& dr );
void DivFrac( int N1, int D1, int N2, int D2, int& NR, int& DR );
double PowerF( double x, int n, int d );
double PowerI( double x, int y );
double PowerR( double x, double y );
void LComDen( int n1, int d1, int n2, int d2, int& n1r, int& n2r, int& cd, int& m1, int& m2 );
int GrComDiv( int x, int y );
MathExpr Expand( MathExpr );
MathExpr ExpandExpr( MathExpr );
MathExpr SubtSq( MathExpr );	// Formula aэ - bэ = ( ma ^ ( 2k ) - nb ^ ( 2l ) ) processing
int LComMul( int x, int y );
MathExpr DeletBR( const MathExpr& exp );
void ConvertComDec( int i, int n, int d, double& x, QByteArray& StDecNum );
void ConvertDecCom( double x, int& i, int& n, int& d, int& nr, int& dr );
void ConvertRoot( int n, int arg, int& Extern, int& Intern );
MathExpr CalcDeg2Rad( const MathExpr& exi );
MathExpr CalcRad2Deg( const MathExpr& exi );

struct TElement
  {
  int m_N1, m_N2;
  TElement *m_pPrev;
  TElement *m_pNext;
  TElement( TElement *p, int M1, int M2 ) : m_N1( M1 ), m_N2( M2 ), m_pPrev( p ), m_pNext( nullptr ) {}
  };

struct TListOfInt
  {
  TElement *m_pFirst;
  TElement *m_pLast;
  TListOfInt() : m_pFirst( nullptr ), m_pLast( nullptr ) {}
  void Add_element( int A, int B );
  ~TListOfInt();
  };

void FactNumber( int NumOrg, int& NumFact, TListOfInt& NumLst );

#endif