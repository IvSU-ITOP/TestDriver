#ifndef OUT_WINDOW
#define OUT_WINDOW

#include "ExpOb.h"

class ResultReceiver
  {
  public:
    ResultReceiver() {}
    virtual void AddComm( const QString& ) = 0;
    virtual void AddExp( const MathExpr& ) = 0;
    virtual void Clear() = 0;
  };

#endif