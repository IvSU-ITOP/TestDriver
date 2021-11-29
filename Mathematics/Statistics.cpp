#include "Statistics.h"
#include "Parser.h"

uLong  TRandom::m_A, TRandom::m_C;
double TRandom::m_Scale;
uLong TRandom::m_Next;
TRandom s_Random;

MathExpr GaussProbability( const MathExpr& Ex )
  { 
  double Value;
  if( !Ex.Constan( Value ) && !Ex.Reduce().Constan( Value ) )
    throw( ErrParser( "Invalid Argument!", peSyntacs ) );
  return Constant( erf( Value ) );
  }

void Randomize() 
  { 
  s_Random.Randomize();
  }

int Random( int Max ) 
  {
  return s_Random.GetNumber(Max);
  }

void TRandom::Initiate()
  {
  if( m_A > 0 ) return;
  uLong M = 1, M2;
  do
    {
    M2 = M;
    M *= 2;
    } while( M > M2 );
    double HalfM = M2;
    uLong Tmp = floor( HalfM * atan( 1.0 ) / 8.0 );
    m_A = 8 * Tmp + 5;
    Tmp = floor( HalfM * ( 0.5 - sqrt( 3.0 ) / 6.0 ) );
    m_C = 2 * Tmp + 1;
    m_Scale = 0.5 / HalfM;
  }

TRandom::TRandom() { Initiate(); }

