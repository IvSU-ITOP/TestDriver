#ifndef MATH_EXPOBJOP
#define MATH_EXPOBJOP
#include "ExpOb.h"

class TSumm : public TOper
  {
  public:
    MATHEMATICS_EXPORT static bool sm_ShowMinusByAddition;
    TSumm( const MathExpr& ex1, const MathExpr& ex2 );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform();
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual MathExpr Integral( const QByteArray& d = "x" );
    virtual MathExpr Lim( const QByteArray& v, const MathExpr& lm );
    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;
    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual bool Summa( MathExpr& op1, MathExpr& op2 ) const;
    virtual bool IsLinear() const;
    virtual bool Negative() const;
    virtual MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower = nullptr );
    virtual bool Positive() const;
    virtual Bracketed MustBracketed() const { return brOperation; }
    virtual QByteArray SWrite() const;
  };

class TSubt : public TOper
  {
  public:
    TSubt( const MathExpr& ex1, const MathExpr& ex2 );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform();
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual MathExpr Integral( const QByteArray& d = "x" );
    virtual MathExpr Lim( const QByteArray& v, const MathExpr& lm );
    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;
    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual bool Subtr( MathExpr& op1, MathExpr& op2 ) const;
    virtual bool IsLinear() const;
    virtual MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower = nullptr );
    virtual Bracketed MustBracketed() const { return brOperation; }
    virtual QByteArray SWrite() const;
  };

class TMult : public TOper
  {
  public:
    MATHEMATICS_EXPORT static bool sm_ShowUnarMinus;
    MATHEMATICS_EXPORT static bool sm_ShowMultSign;
    TMult( const MathExpr& ex1, const MathExpr& ex2, uchar sign = '*');
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform();
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual MathExpr Integral( const QByteArray& d = "x" );
    virtual MathExpr Lim( const QByteArray& v, const MathExpr& lm );
    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual bool Multp( MathExpr& op1, MathExpr& op2 ) const;
    virtual bool Multiplication( MathExpr& op1, MathExpr& op2 ) const;
    virtual bool Negative() const;
    virtual bool IsLinear() const;
    virtual MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower = nullptr );
    virtual bool Positive() const;
    virtual bool IsFactorized( const QByteArray& Name );
    virtual Bracketed MustBracketed() const { return brPower; }
    bool IsIndexed() const { return m_Operand2.IsIndexed(); }
  };

class TDivi : public TOper
  {
  protected:
  bool m_AsFrac;   // sign that expression drawing as fraction TRUE - fraction, FALSE - with ":" char 
  public:
    TDivi() {}
    TDivi( const MathExpr& ex1, const MathExpr& ex2, bool AsFrac = true, char Name = '/' );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform();
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual MathExpr Integral( const QByteArray& d = "x" );
    virtual MathExpr Lim( const QByteArray& v, const MathExpr& lm ) { return nullptr; }

    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual bool AsFraction();
    virtual bool Divis( MathExpr& op1, MathExpr& op2 ) const { op1 = m_Operand1; op2 = m_Operand2; return true; }
    virtual bool IsLinear() const;
    virtual MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower = nullptr );
    virtual bool Positive() const;
    virtual bool Negative() const;
    virtual Bracketed MustBracketed() const { return brPower; }
  };

/*
class TDiviEv : public TDivi
  {
  public:
    TDiviEv( const MathExpr& ex1, const MathExpr& ex2 ) :TDivi( ex1, ex2, false ) { m_Name = '|'; }
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform();
    virtual MathExpr Diff( const QByteArray& d = "x" );

    virtual bool Eq( const MathExpr& E2 ) const;

    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    //void To_keyl( txpinedit Ed ) {}

    virtual bool AsFraction();
    virtual bool DiviEvi( MathExpr& op1, MathExpr& op2 ) const;
  };
  */
inline MathExpr MathExpr::operator / ( const MathExpr& E ) const { return new TDivi( m_pExpr, E ); }
inline MathExpr MathExpr::operator / ( double V ) const { return new TDivi( m_pExpr, new TConstant( V ) ); }
//inline MathExpr MathExpr::operator | ( const MathExpr& E ) const { return new TDiviEv( m_pExpr, E ); }
//inline MathExpr MathExpr::operator | ( double V ) const { new TDiviEv( m_pExpr, new TConstant( V ) ); }

#endif