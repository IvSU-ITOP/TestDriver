#ifndef ANALYSIS_H
#define ANALYSIS_H
#include "MathTool.h"
#include "ExpObBin.h"
#include "Algebra.h"

MathExpr CalcIntegralExpr(const MathExpr& e);
void MakeCalcDefIntegral(const MathExpr& e, MathExpr& ex1, MathExpr& ex2, MathExpr& ex3);

class TDerivative : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TDerivative( const MathExpr Expr );
    MATHEMATICS_EXPORT TDerivative();
  };

class TIndefIntegr : public Solver
  {
MATHEMATICS_EXPORT virtual void Solve();
public:
  TIndefIntegr( const MathExpr Expr );
  MATHEMATICS_EXPORT TIndefIntegr();
  };

class TLimitCreator : public Solver
  {
MATHEMATICS_EXPORT virtual void Solve();
public:
  TLimitCreator( const MathExpr Expr );
  MATHEMATICS_EXPORT TLimitCreator();
  };

class TDefIntegrate : public Solver
  {
MATHEMATICS_EXPORT virtual void Solve();
public:
  TDefIntegrate( const MathExpr Expr );
  MATHEMATICS_EXPORT TDefIntegrate();
  };

class TEigen : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TEigen( const MathExpr Expr );
    MATHEMATICS_EXPORT TEigen();
  };

class TDeterminant : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TDeterminant( const MathExpr Expr );
    MATHEMATICS_EXPORT TDeterminant();
  };

class TTransposer : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TTransposer( const MathExpr Expr );
    MATHEMATICS_EXPORT TTransposer();
  };

class TInverter : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TInverter( const MathExpr Expr );
    MATHEMATICS_EXPORT TInverter();
  };

class TAngle2 : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TAngle2( const MathExpr Expr );
    MATHEMATICS_EXPORT TAngle2();
  };

class TAngle3 : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TAngle3( const MathExpr Expr );
    MATHEMATICS_EXPORT TAngle3();
  };

class TTan2 : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TTan2( const MathExpr Expr );
    MATHEMATICS_EXPORT TTan2();
  };

class TAlpha2 : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TAlpha2( const MathExpr Expr );
    MATHEMATICS_EXPORT TAlpha2();
  };

class TPriv : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TPriv( const MathExpr Expr );
    MATHEMATICS_EXPORT TPriv();
  };

class TTrigoOfSumma : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TTrigoOfSumma( const MathExpr Expr );
    MATHEMATICS_EXPORT TTrigoOfSumma();
  };

class TSumm_Mult : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSumm_Mult( const MathExpr Expr );
    MATHEMATICS_EXPORT TSumm_Mult();
  };

class TTrigo_Summ : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TTrigo_Summ( const MathExpr Expr );
    MATHEMATICS_EXPORT TTrigo_Summ();
  };

class Permutations : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    Permutations( const MathExpr Expr );
    MATHEMATICS_EXPORT Permutations();
  };

class BinomCoeff : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    BinomCoeff( const MathExpr Expr );
    MATHEMATICS_EXPORT BinomCoeff();
  };

class Accomodations : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    Accomodations( const MathExpr Expr );
    MATHEMATICS_EXPORT Accomodations();
  };

class Statistics : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    Statistics( const MathExpr Expr );
    MATHEMATICS_EXPORT Statistics();
  };

class Correlation : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    Correlation( const MathExpr Expr );
    MATHEMATICS_EXPORT Correlation();
  };

class AlgToTrigo : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    AlgToTrigo( const MathExpr Expr );
    MATHEMATICS_EXPORT AlgToTrigo();
  };

class ComplexOper : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    ComplexOper( const MathExpr Expr );
    MATHEMATICS_EXPORT ComplexOper();
  };

#endif // ANALYSIS_H
