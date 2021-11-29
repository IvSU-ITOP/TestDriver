#ifndef FACTORS
#define FACTORS
#include "ExpObjOP.h"

enum DivPolResult {
  dpOk = 0,     // ok
  dpNPdd = -1,     // dividend it is not polinom
  dpNPdr = -2,     // divisor it is not polinom
  dpIMPS = -4,     // divisor have degree more high than dividend
  dpDB = -8,     // dividend and divisor have a difference base
  dpRCR = -16   // real coefficients if result
  };    

bool CompareExpr( const MathExpr& ExprI, const MathExpr& ExprJ );
DivPolResult Divide2Polinoms( const MathExpr& Dividend, const MathExpr& Divisor, MathExpr& Quotient, MathExpr& Rest );
inline MathExpr ReduceTExprs( const MathExpr& E ) { return E.IsEmpty() ? E : E.Reduce(); }
MathExpr MakePower( const MathExpr& Base, const MathExpr& Power );
bool IsEqualExprs( const MathExpr& Expr1, const MathExpr& Expr2 );
int CheckPower( int const Number, int NPower );
MathExpr NegTExprs( const MathExpr& ExprX, bool IsNeg = true );
MathExpr CallExpand( const MathExpr& exp );
bool SearchMulti( MathExpr exi, class TTerm& Term, bool& ResultInt );
int FindPower( int const Number );
bool CheckThreeTerms( TTerm& TermXs2, TTerm& TermYs2, TTerm& Term2xy );
bool CheckFourTerms( TTerm& TermXs3, TTerm& TermYs3, TTerm& Term3x2y, TTerm& Term3xy2 );
bool CheckFourTerms1( TTerm& TermXs2, TTerm& TermYs2, TTerm& TermZs2, TTerm& Term2xy );
MathExpr CommonTerm( MathExpr& TermX, MathExpr& TermY );
void SwapExpr( MathExpr& ExprX, MathExpr& ExprY );
MathExpr IToFactors( const MathExpr& exi );
bool ReducePWR( int& NPwr, int& MinPwr );
MathExpr ExpandOpRes( const MathExpr& exi );
MathExpr InsertY( const MathExpr& exi, const MathExpr& TermY );
bool IsSubtSq( MathExpr& TermX, MathExpr& TermY );
MathExpr ToFactors( const MathExpr& exi );
MathExpr ToFactorsDel(const MathExpr& exi);
MathExpr MakeTrinom( const MathExpr& exp );
MathExpr CuSbSm( const MathExpr& exi );
MathExpr Cancellation( MathExpr& ExprX, MathExpr& ExprY );
MathExpr GetCommon( MathExpr& ExprX, MathExpr& ExprY );
MathExpr MultExprs( const MathExpr& ExprX, const MathExpr& ExprY );
int GetFactorCount( const MathExpr& ex, const QByteArray& Name );
MathExpr GetFactorEquation( const QByteArray& VarName );
MathExpr GetPutRoot( const MathExpr& ex, const QByteArray& VarName );
TL2exp* RootPolinom( MathExpr ex );
bool IsMultiNominal( const MathExpr& exi );

class TCounter
  {
  int m_QBits;
  int m_QBins;
  int m_Limit;
  int m_Value;
  int m_Cbin;
  int m_Czer;
  public:
    static const int MaxCountersBits = 9;
  private:
    int m_Bins[MaxCountersBits];
    int m_Zers[MaxCountersBits];
  public:
    TCounter( int AQBits, int AQBins );
    void SearchBits();
    bool BinsOk();
    int GetB( int I );
    int GetZ( int I );
    void Next();
    bool IsFinish();
    void SetFinish();
  };

class TMultiplier
  {
  friend class TTerm;
  friend class TMultiNominal;
  void PutPwrInt( int Value );
  public:
    MathExpr m_Base;
    MathExpr m_Power;
    TMultiplier( const MathExpr& ABase, const MathExpr& APower );
    MathExpr RetMulti() const;
    bool Eq( const MathExpr& exi, int& NewSign );
    bool IsDegree( int const NDeg );
    MathExpr SQRT( int degree );
    void DecPwrInt();
    int GetPwrInt() const;
  };

class TTerm : public QList<TMultiplier>
  {
  friend class TMultiNominal;
  public:
    static const QByteArray sm_ParamNames;
    static const int sm_TermNotExist;
    MathExpr m_Coefficient;
    TTerm(int Sign = 1);
    int m_Sign;
  int GetCoeffInt();
  void PutCoeffInt( int const NewCoeff );
  double GetCoeffReal() const;
  void PutCoeffReal( double const NewCoeff );
    void MultCoeff( const MathExpr& MCoeff );
    bool IsDegree( int const NDeg );
    MathExpr TermSQRT( int degree );
    MathExpr FullTerm();
    MathExpr FullTermWithout( MathExpr& X );
    MathExpr AbsFullTerm();
    MathExpr Term_SC();
    MathExpr CoeffAndSign();
    MathExpr TermBase() const;
    MathExpr TermBaseGCD();
    bool IsOneVar( MathExpr& VarX, int& AttrX );
    int PowerInt();
    int PowerGCD() const;
    int Pwr();
    bool IsLinear();
    QByteArray VarName();
    MathExpr Params();
    void MakeCommonBase();
    bool Compare( const MathExpr& exp1, const MathExpr& exp2 );
    void ClearExpr( const MathExpr& exp );
    void InsertExpr( int MaxPwr, const MathExpr& exp );
    MathExpr GetVarsOnly( QList<TMultiplier> *pList = nullptr );
    MathExpr GetTermWithoutVars();
    bool HasVariable( const QByteArray& Name, int Power );
    bool HasProduct( const QByteArray& Var1, const QByteArray& Var2, int Power1, int Power2 );
  };

class TMultiNominal : public QList<TTerm>
  {
  static const int sm_MaxFractionRootIters = 200;
  public:
      bool m_IsCorrect;
    bool m_ResultInt;
    TMultiNominal() {}
    TMultiNominal( const MathExpr& exi );
    TMultiNominal( const TMultiNominal& MN, const MathExpr& sName );
    void MakeCommonBase();
    bool IsVarOnly1time( MathExpr& X );
    void ChangeVar( MathExpr& X, MathExpr& VarY );
    void GetAllVars( QByteArrayList& sList );
    bool IsFullNegative();
    void SetFullPositive();
    MathExpr SearchCommonCoeff();
    void DivideCoeff( int CoeffInt );
    void DefaultCoefficient();
    MathExpr FullExpr();
    MathExpr SearchCommonTerm();
    bool IsLinear();
    int StandardCase( MathExpr& TermX, MathExpr& TermY );
    bool StandardCase_DEG2( MathExpr& TermX, MathExpr& TermY, int& SignDEG2 );
    bool StandardCase_DEG3( MathExpr& TermX, MathExpr& TermY, int& SignDEG3 );
    bool StandardCase_DEG2p2( MathExpr& TermX, MathExpr& TermY, int& SignDEG2 );
    bool StandardCaseGroupS3( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 );
    bool StandardCaseGroupS( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 );
    bool StandardCaseGroupS4( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 );
    bool StandardCaseGroupS5( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 );
    bool StandardCaseGroupS6( MathExpr& TermX, MathExpr& TermY, MathExpr& TermXY, int& SignDEG2 );
    bool IsForm( MathExpr& TermX, MathExpr& TermY );
    void ClearExpr( const MathExpr& exp );
    void InsertExpr( const MathExpr& exp );
    bool IsPolinom();
    bool IsClassic();
    bool IsMultiTerm( MathExpr& TermX );
    bool ClassicSquare( MathExpr& TermX, MathExpr& TermX1, MathExpr& TermX2, int& Part_a1, int& Part_a2 );
    bool IsRoot( int X1, MathExpr& ExprX1 );
    bool IsFractionRoot( int& X1, int& X2 );
    bool IsIntegerRootInFreeTerm( MathExpr& ExprX, MathExpr& ExprX1 );
    bool IsFractionRootInFreeTerm( int& X1, int& X2 );
    bool IsRootInInterval( MathExpr& ExprX, MathExpr& ExprX1 );
    MathExpr Divide( const MathExpr& ExprX1 );
    int LimitOfRoot();
    void Grouping();
    TTerm HasVariable( const QByteArray& Name, int Power );
    TTerm HasProduct( const QByteArray& Var1, const QByteArray& Var2, int Power1, int Power2 );
    bool SearchSumma( const MathExpr& exi, bool& ResultInt );
  };

class TMatrix2 : public QVector<MathExpArray>
  {
  QByteArrayList m_Variables;
  MathExpr m_CountedDet;
  QVector<int> m_RowsOrder;
  int m_Status;
  public:
    MathExpr m_OldStep;
    TMatrix2() {}
    TMatrix2( int Size );
    TMatrix2( TMatrix2& AMatrix2 );
  void InitMatrix( int ASize );
  QByteArray& Variable( int iVar ) { return m_Variables[iVar]; }
  MathExpr Det();
  MathExpr DetI( int I );
  void InsertFreeToI( int No );
  int IndexOfName( const QByteArray& Name );
  void CountOfDet();
  MathExpr& operator ()(int, int);
  MathExpr& FreeTerms(int);
  void PutToWindow();
  void ChangeOrder( int I, int J );
  void Sort();
  MathExpr GetEquation( int I );
  int Status() { return m_Status; }
  int VariableCount() { return m_Variables.count(); }
  MathExpr CountedDet() { return m_CountedDet; }
  };

class TLinear
  {
  int m_Status;    // status of object of class TLinear
  MathExpArray m_InputList;      // input list of equations
  MathExpArray m_Solution;
  bool m_Homogeneous;
  public:
    TMatrix2 m_Matrix2;    // matrix of coefficients
    TLinear( const MathExpArray& );
    bool Run();  // solve the system of linear equations
    // output of roots
    MathExpr GetRoot( int );
    void ResetMatrix();
    MathExpr GetSolution( int );
    int Status() { return m_Status; }
    void PutToWindow() { m_Matrix2.PutToWindow(); }
  };

class MultNomVector : public QVector<TMultiNominal*>
  {
  public:
    MultNomVector() : QVector<TMultiNominal*>() {}
    virtual ~MultNomVector();
    void Add( const MathExpr& E ) { append( new TMultiNominal( E ) ); }
  };
#endif
