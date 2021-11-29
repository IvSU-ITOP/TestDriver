#ifndef MATH_EXPOBJBIN
#define MATH_EXPOBJBIN
#include "ExpOb.h"

class TBinar : public TOper
  {
  friend MathExpr;
  public:
    static bool sm_NoBoolReduce;
    MATHEMATICS_EXPORT TBinar( uchar, const MathExpr& ex1, const MathExpr& ex2 );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform();
    virtual MathExpr Diff( const QByteArray& d = "x" );
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    bool Binar( char N, MathExpr& op1, MathExpr& op2 ) const;
    bool Binar_( uchar& N, MathExpr& op1, MathExpr& op2 ) const;
    bool Infinity( bool& ANeg );
    virtual bool operator < ( const MathExpr& );
    virtual QByteArray SWrite() const;
  };

inline TBinar* MathExpr::Binar() { return reinterpret_cast< TBinar* >( m_pExpr ); }
#endif
