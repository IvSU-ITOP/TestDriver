#ifndef MATH_LOGEXP
#define MATH_LOGEXP
#include "ExpObBin.h"

bool CalcLog1Eq( const QByteArray& Source, QByteArray Name = "x", int StartIndex = 0 );
bool CalcExpEq( const QByteArray& Source, bool Recurs = true );
Lexp CalcIrratEq( const QByteArray& Source, bool PutSource = true, bool Recurs = true );
void LogPropertiesEx( MathExpr Ex, int Prop );
bool CalcSysInEq( const QByteArray& InEq );
bool CalcRatInEq( const QByteArray& InSource );

#endif