#ifndef ALGEBRA
#define ALGEBRA

#include "ExpOb.h"
#include "ExpObjOp.h"
#include "Parser.h"
#include "LogExp.h"
#include "Factors.h"
#include <functional>

typedef QVector<MathExpr> TExprs;

bool ReduceMeasure( MathExpr& ex );
QByteArrayList GetFactors( PNode eq );
bool ReduceFactor( PNode eq, MathExpr& ex );
bool ReduceExpr( MathExpr& ex );
MathExpr SubtCube( const MathExpr& exi );
MathExpr SummCube( const MathExpr& exi );
MathExpr CreateExprPoly( MathExpArray &Arr, int n, const QByteArray& Name = "x" );
MathExpr SqSbSm( const MathExpr& exi );
MathExpr RightExpr( const MathExpr& E );
MathExpr RemDenominator( MathExpr ex, Lexp& );
MathExpr RemDenominator( MathExpr ex );
MathExpr Transfer( MathExpr exp );
Lexp FractRatEq( const QByteArray& Source, const QByteArray& Name = "x", bool CalcBiQ = true );
Lexp CalcDetQuEqu( const QByteArray& Source, QByteArray VarName = "" );
bool IsFuncEqu( PNode Expr, const QByteArray& Excepted = "" );
bool TestFrac( PNode Expr );
bool IsExpEqu( PNode Expr );
Lexp CalcDetLinEqu( const QByteArray& Source, const QByteArray& Name = "x" );
MathExpr DelSplitted( const MathExpr& ex );
Lexp SolutionSimpleEquaion( PNode equation, const QByteArray& SelectName );
void FracToMult( PNode Expr );
PNode UnMinus( PNode Expr );
PNode Sub( PNode Reduced, PNode Subtracted );
PNode Add( PNode Addend1, PNode Addend2 );
bool EquationsSolving( const QByteArray& UserTask, const QByteArray& Name = "x" );
Lexp CalcDetBiQuEqu( const QByteArray& Source, const QByteArray& VarName = "x" );
bool ReduceEquation( MathExpr& ex, const QByteArray& SelName = "x" );
bool Div2( PNode eq );
bool OpenBrackets( PNode eq );
bool MultDiv( PNode eq );
PNode NewNode( Parser *P, const QByteArray& Name );
PNode Clone( PNode p );
int DegOfPoly( const MathExpr& ex, const QByteArray& Name );
bool CalcSimpleTrigo( const QByteArray& Source );
bool CalcTrigoEqu( const QByteArray& Source );
bool CalcHomogenTrigoEqu( const QByteArray& Source );
bool CalcSinCosEqu( const QByteArray& Source );
bool CalcSysInEqXY( const QByteArray& UserTask );
MathExpr SolSysInEq( MathExpr ex );
void AssignVar( const MathExpr& exi );
bool CalcSubstitution( const QByteArray& Equation, MathExpr BaseSubst = MathExpr() );
bool GetAnswer( const MathExpr& expr, QByteArray Name = "" );
bool CalcLinear( const QByteArray& Equation );
bool CalcExchange( const QByteArray& Equation );
bool CalcRootsQuEqu( const QByteArray& Source );
bool CalcDiscrim( const QByteArray& Source );
MathExpr DetVieEqu( const MathExpr& exi );
Lexp CalcPolinomEqu( const QByteArray& Source, const QByteArray&& VarName = "x" );
bool CalcEquation( const QByteArray& Source );
MathExpr CalcAnyEquation( const QByteArray& Source );

class TNodes : public PascArray<PNode>
  {
  public:
    TNodes() : PascArray<PNode>( 1, 100 ) {}
  };

struct TDiv
  {
  char m_Sign;
  bool m_DenDepX;
  PNode m_Nom, m_Den;
  TDiv() {}
  TDiv( PNode Nom, PNode Den ) : m_Nom( Nom ), m_Den( Den ) {}
  TDiv( const TDiv& D ) : m_Nom( D.m_Nom ), m_Den( D.m_Den ) {}
  };

struct TDivExpr
  {
  MathExpr m_NomEx;
  MathExpr m_DenEx;
  TDivExpr() {}
  TDivExpr( const MathExpr& NomEx ) : m_NomEx( NomEx ) {}
  TDivExpr( const TDivExpr& D ) : m_NomEx( D.m_NomEx ), m_DenEx(D.m_DenEx) {}
  };

typedef QVector<TDiv> TDivis;
typedef QVector<TDivExpr> TDivEx;
void FindDivi( PNode Expr, TDivis& Divis );

class TSubstitution
  {
  int m_Status;
  MathExpArray m_List;
  MultNomVector m_Multis;
  MathExpArray m_Roots;
  int m_EqNo;
  int m_TrNo;
  MathExpr m_VarX;
  MathExpr m_VarXGi;
  QByteArrayList m_VarsList;
  public:
    TSubstitution( const MathExpr& exi );
    bool CheckPossibleOfMethod();
    void CalculateVarXGi();
    void CalculateY();
    bool Run();
    void MakeRoot( const MathExpr& root );
    MathExpr GetVarY();
    MathExpr RootX( int I );
    MathExpr operator[]( int i ) { return m_Roots[ i ]; }
    int Status() { return m_Status; }
    MathExpr GetVarX() { return m_VarX; }
    MathExpr GetVarXGi() { return m_VarXGi; }
    int MultisCount() { return m_Multis.count(); }
    TMultiNominal& operator ()( int i ) { return *m_Multis[i]; }
    int EqNo() { return m_EqNo; }
    MathExpr List( int i ) { return m_List[i]; }
    int RootsCount() { return m_Roots.count(); }
    QByteArray VarName( int i ) { return m_VarsList[i]; }
  };

class TSimItem
  {
  friend class TSimInfo;
  int m_DegreeX;
  int m_DegreeY;
  MathExpr m_Coeff;
  bool m_IsCheck;
  public:
    TSimItem() {}
    TSimItem( int ADegreeX, int ADegreeY, const MathExpr& ACoeff ) : m_DegreeX( ADegreeX ), m_DegreeY( ADegreeY ),
      m_Coeff( ACoeff ), m_IsCheck( ADegreeX == ADegreeY ) {}
    bool IsSim( const TSimItem& SimItem ) 
      {
      return m_DegreeX != m_DegreeY && m_DegreeX == SimItem.m_DegreeY && m_DegreeY == SimItem.m_DegreeX; 
      }
  bool EqDegrees() { return m_DegreeX == m_DegreeY; }
  MathExpr MakeExchange();
  };

class TSimInfo : public QVector<TSimItem>
  {
  public:
  void Duplicates();
  bool Check();
  void DeleteDuplicates();
  MathExpr MakeExchange();
  };

class TExchange
  {
  int m_Status;
  MathExpArray m_List;
  MultNomVector m_Multis;
  QByteArrayList m_VarsList;
  QByteArray m_VarX;
  QByteArray m_VarY;
  TSimInfo m_SimInfo1;
  TSimInfo m_SimInfo2;
  MathExpr m_ExchEqu1;
  MathExpr m_ExchEqu2;
  public:
    MathExpArray m_RootsU;
    MathExpArray m_RootsV;
    MathExpArray m_RootsX;
    MathExpArray m_RootsY;
    Lexp m_DenomList;
    TExchange( const MathExpr& exi );
    bool CheckPossibleOfMethod();
    bool CheckEquation( const TMultiNominal& multi, TSimInfo& SimInfo );
    MathExpr GetExprXsY();
    MathExpr GetExprXpY();
    MathExpr GetNewVarU();
    MathExpr GetNewVarV();
    bool Run();
    MathExpr GetExchEqu1();
    MathExpr GetExchEqu2();
    void InsertRoot( const QByteArray& Name1, const MathExpr& Root1, const MathExpr& Root2 );
    void CalculateRoot( int I );
    void NewRoot( const MathExpr& RootU, const MathExpr& RootY );
    void CalculateFinish();
    MathExpr GetPrintRoot( int I );
    int Status() { return m_Status; }
  };

class Solver
  {
  double m_OldPrecision;
  protected:
    MathExpr m_Expr;
    virtual void Solve(){};
    QByteArray m_Name;
    QByteArray m_DefaultName;
    int m_Code;
    void SimplifyExpand();
  public:
    enum Solvers { ESolvReToMult, ESolvExpand, ESolvSubSqr, ESolvSqrSubSum, ESolvSumCub, ETrinom, ESolvCubSubSum, ESolvSubCub, EAlg1Calculator, ELg, Eog1Eq,
      ESysInEq, ESysInEqXY, ERatInEq, EExpEq, EMakeSubstitution, ESolveLinear, EMakeExchange, ESolvDetLinEqu , ESolvQuaEqu, ESolvDetQuaEqu, ESolvDisQuaEqu,
      ESolvDetVieEqu, ESolvCalcDetBiQuEqu, ESolvFractRatEq, ESolvCalcIrratEq, ESolvCalcPolinomEqu, ESolvCalcSimpleTrigoEq, ESolvCalcSinCosEqu,
      ESolvCalcTrigoEqu, ESolvCalcHomogenTrigoEqu, ESolvCalcEquation, ESin, ECos, ETan, ELn, EDegRad, ERadDeg, ESciCalc };
    MATHEMATICS_EXPORT static MathExpr m_OldExpr;
    MATHEMATICS_EXPORT static Solver* sm_TestSolvers;
    MATHEMATICS_EXPORT static bool sm_TestMode;
    Solver() : m_Code( -1 ), m_OldPrecision( s_Precision ) { s_MemorySwitch = SWcalculator; }
    Solver( const MathExpr Expr ) : m_Expr( Expr ), m_OldPrecision( s_Precision ) { m_OldExpr = Expr; s_MemorySwitch = SWcalculator; Solve(); }
    virtual ~Solver() { s_MemorySwitch = SWtask; s_Precision = m_OldPrecision; }
    MATHEMATICS_EXPORT void SetExpression( const QByteArray& Expr ) { m_OldExpr = m_Expr = Parser::StringToExpr( Expr ); Solve(); }
    MATHEMATICS_EXPORT MathExpr Result() { return m_Expr; }
    MATHEMATICS_EXPORT bool Success() { return !m_Expr.IsEmpty() && !m_Expr.Eq( m_OldExpr ); }
    MATHEMATICS_EXPORT void SetExpression( const MathExpr& Expr ) { m_OldExpr = m_Expr = Expr; Solve(); }
    MATHEMATICS_EXPORT QString Name() { return X_Str( m_Name, m_DefaultName ); }
    MATHEMATICS_EXPORT int Code() { return m_Code; }
  };

class TSolvReToMult : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvReToMult( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvReToMult();
  };

class TSolvExpand : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvExpand( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvExpand();
  };

class TSolvSubSqr : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvSubSqr( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvSubSqr();
  };

class TSolvSqrSubSum : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvSqrSubSum( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvSqrSubSum();
  };

class TSolvSumCub : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvSumCub( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvSumCub();
  };

class TTrinom : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TTrinom( const MathExpr Expr );
    MATHEMATICS_EXPORT TTrinom();
  };

class TSolvCubSubSum : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCubSubSum( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCubSubSum();
  };

class TSolvSubCub : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvSubCub( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvSubCub();
  };

class Alg1Calculator : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    Alg1Calculator( const MathExpr Expr );
    MATHEMATICS_EXPORT Alg1Calculator();
  };

class TLg : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TLg( const MathExpr Expr );
    MATHEMATICS_EXPORT TLg();
  };

class Log1Eq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    Log1Eq( const MathExpr Expr );
    MATHEMATICS_EXPORT Log1Eq();
  };

class SysInEq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    SysInEq( const MathExpr Expr );
    MATHEMATICS_EXPORT SysInEq();
  };

class SysInEqXY : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    SysInEqXY( const MathExpr Expr );
    MATHEMATICS_EXPORT SysInEqXY();
  };

class RatInEq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    RatInEq( const MathExpr Expr );
    MATHEMATICS_EXPORT RatInEq();
  };

class ExpEq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    ExpEq( const MathExpr Expr );
    MATHEMATICS_EXPORT ExpEq();
  };

class MakeSubstitution : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    MakeSubstitution( const MathExpr Expr );
    MATHEMATICS_EXPORT MakeSubstitution();
  };

class SolveLinear : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    SolveLinear( const MathExpr Expr );
    MATHEMATICS_EXPORT SolveLinear();
  };

class MakeExchange : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    MakeExchange( const MathExpr Expr );
    MATHEMATICS_EXPORT MakeExchange();
  };

class TSolvDetLinEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvDetLinEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvDetLinEqu();
  };

class TSolvQuaEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvQuaEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvQuaEqu();
  };

class TSolvDetQuaEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvDetQuaEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvDetQuaEqu();
  };

class TSolvDisQuaEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvDisQuaEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvDisQuaEqu();
  };

class TSolvDetVieEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvDetVieEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvDetVieEqu();
  };

class TSolvCalcDetBiQuEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcDetBiQuEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcDetBiQuEqu();
  };

class TSolvFractRatEq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvFractRatEq( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvFractRatEq();
  };

class TSolvCalcIrratEq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcIrratEq( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcIrratEq();
  };

class TSolvCalcPolinomEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcPolinomEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcPolinomEqu();
  };

class TSolvCalcSimpleTrigoEq : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcSimpleTrigoEq( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcSimpleTrigoEq();
  };

class TSolvCalcSinCosEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcSinCosEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcSinCosEqu();
  };

class TSolvCalcTrigoEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcTrigoEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcTrigoEqu();
  };

class TSolvCalcHomogenTrigoEqu : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcHomogenTrigoEqu( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcHomogenTrigoEqu();
  };

class TSolvCalcEquation : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSolvCalcEquation( const MathExpr Expr );
    MATHEMATICS_EXPORT TSolvCalcEquation();
  };

class TSin : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSin( const MathExpr Expr );
    MATHEMATICS_EXPORT TSin();
  };

class TCos : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TCos( const MathExpr Expr );
    MATHEMATICS_EXPORT TCos();
  };

class TTan : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TTan( const MathExpr Expr );
    MATHEMATICS_EXPORT TTan();
  };

class TLn : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TLn( const MathExpr Expr );
    MATHEMATICS_EXPORT TLn();
  };

class TDegRad : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TDegRad( const MathExpr Expr );
    MATHEMATICS_EXPORT TDegRad();
  };

class TRadDeg : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TRadDeg( const MathExpr Expr );
    MATHEMATICS_EXPORT TRadDeg();
  };

class TSciCalc : public Solver
  {
  MATHEMATICS_EXPORT virtual void Solve();
  public:
    TSciCalc( const MathExpr Expr );
    MATHEMATICS_EXPORT TSciCalc();
  };

#endif
