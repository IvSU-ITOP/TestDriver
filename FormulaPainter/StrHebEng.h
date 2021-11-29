#ifndef MATH_STRHEBENG
#define MATH_STRHEBENG

#include <QByteArray>
#include <qmap.h>
#include "formulapainter_global.h"

void InvertDigitSequences(QByteArray &s, int iStart, int iEnd );
QByteArray WInvertHebrewEnglishString(QByteArray &ss); 
QByteArray RInvertHebrewEnglishString(QByteArray &ss); 
bool IsEngChar(uchar c);
FORMULAPAINTER_EXPORT bool IsHebChar( uchar c );
bool IsNoHebrew( uchar c);
//QString X_Str( const uchar* Key, const uchar* Default );

#endif