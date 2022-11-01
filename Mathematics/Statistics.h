#ifndef MATH_STATISTICS
#define MATH_STATISTICS
#include "ExpOb.h"
//#include <QGlobal>
#include <QString>
#include <math.h>
#include <time.h>
#include <QVector>

MathExpr GaussProbability( const MathExpr& Ex );
MATHEMATICS_EXPORT int Random( int Max );
MATHEMATICS_EXPORT void Randomize();
void CalcStatistics(MathExpr&);
void CalcCorrelation(MathExpr&);
using namespace std;

typedef unsigned long long uLong;
int const SecLength = 10;

class TRandom
  {
  static uLong  m_A, m_C;
  static double m_Scale;
  static void Initiate();
  static uLong m_Next;
  protected:
    double m_Expection;
    double GetStandard() { m_Next *= m_A; m_Next += m_C; return m_Scale * m_Next; }
  public:
    TRandom();
    void Randomize() { m_Next = time( NULL ); }
    int GetNumber( int Val ) { return round( Val * GetStandard() ); }
  };

#endif
