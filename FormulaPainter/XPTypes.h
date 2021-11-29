#ifndef MATH_XPTYPES
#define MATH_XPTYPES
#include <qpoint.h>
#include <qbytearray.h>
#include "formulapainter_global.h"

#define _IS_(Ref, Type) dynamic_cast<Type*>(Ref) != NULL

FORMULAPAINTER_EXPORT long long Round( double Val );
//long long Round( double Val );

enum TEdAction {	actPrintable,	actCtrlKey,	actIcon, actMouseButton, actNone,	actStep	};
enum TLanguages { lngHebrew, lngEnglish, lngRussian, lngBulgarian, lngAll };
enum memory_mode { SWtask, SWcalculator };
enum TWorkMode { wrkLearn, wrkTrain, wrkExam };

struct BasePoint
  {
  int X;
  int Y;
  };

struct TPoint : BasePoint
  {
  TPoint( int x, int y ) { X = x; Y = y; }
  TPoint() { X = Y = 0; }
  TPoint( BasePoint P ) { X = P.X; Y = P.Y; }
  };

class SimpleChar : public QByteArray
  {
  public:
    SimpleChar() : QByteArray() {}
    SimpleChar( const char* pc ) : QByteArray( pc ) {}
    SimpleChar( const char c ) : QByteArray( 1, c ) {}
    SimpleChar( const QByteRef c ) : QByteArray( 1, c ) {}
    SimpleChar( const QByteArray& A ) { append( A.isEmpty() ? QByteArray() : QByteArray( 1, A[0] ) ); }
    bool operator == ( const char c ) { return count() == 1 && at( 0 ) == c; }
 };

template<typename T>class PascArray : public QVector<T>
  {
  int m_Shift;
  public:
    PascArray( int Shift = 1, int iSize = 21 ) : QVector<T>( iSize ), m_Shift( Shift ) {}
    T& operator [] ( int i ) { return QVector<T>::operator[]( i - m_Shift ); }
  };

#endif
