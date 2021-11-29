#include "ExpOb.h"

typedef QByteArray TLexem;

const int bhvNormal = 0,
bhvSpace = 1,
bhvPInfinity = 2,
bhvNInfinity = 3,
bhvAsmp = 4,
bhvScreenSpace = 5,
bhvLimit = 6,
bhvIncrease = 7,
bhvDecrease = 8,
bhvMax = 9,
bhvMin = 10,
bhvJump = 11,
bhvRoot = 12,
bhvConcavity = 14,
bhvConvex = 15,
bhvInflConcav = 16,
bhvStrigth = 17,
bhvInflConvex = 18,
bhvDelta = 19,
bhvLimitY = 20,
bhvPlotSpace = 21;

double const Min__Val = 1e-12, Max__Val = 1e10, _Max__Val = 0.95*Max__Val;

void DefMinMax( class DxExpressionValues& Values, class DxIntValues& Bhv, double& _Y_Min, double& _Y_Max, bool SymZer );

class TCarPoint
  {
  double m_X;
  int m_Bhv;
  public:
  TCarPoint() : m_X( 0 ), m_Bhv( bhvNormal ) {}
  TCarPoint( double x, int bhv = bhvNormal ) : m_X( x ), m_Bhv( bhv ) {}
  TCarPoint( int bhv ) : m_X( 0 ), m_Bhv( bhv ) {}
  TCarPoint& operator = ( double x ) { m_X = x; return *this; }
  TCarPoint& operator = ( int bhv ) { m_Bhv = bhv; return *this; }
  operator double() const { return m_X; }
  operator int() const { return m_Bhv; }
  bool operator == ( int bhv ) const { return m_Bhv == bhv; }
  bool operator != ( int bhv ) const { return m_Bhv != bhv; }
  bool operator > ( double x ) const { return m_X > x; }
  bool operator < ( double x ) const { return m_X < x; }
  TCarPoint& operator += ( double x ) { m_X += x; return *this; }
  double operator - ( const TCarPoint& CP ) { return m_X - CP.m_X; }
  double operator / ( const TCarPoint& CP ) { return m_X / CP.m_X; }
  };

class IntValue : public TExpr
  {
  friend class DxIntValue;
  int m_Value;
  public:
    IntValue( int V = 0 ) : m_Value( V ) {}
  };

class DxIntValue : public MathExpr
  {
  public:
    DxIntValue( int V = 0 ) : MathExpr( new IntValue( V ) ) {}
    int &Value() { return Cast( IntValue, m_pExpr )->m_Value; }
  };

class DxValue : public MathExpr
  {
  public:
    static const double sc_valInfinity;
    static const double sc_valZero;
    static const double sc_valCompare;
    DxValue() : MathExpr( new TConstant( 0 ) ) {}
    DxValue( double V ) : MathExpr( new TConstant( V ) ) {}
    static QByteArray FloatTo_Str( double r, int Decimals );
    static bool Equal( double r, double t );
    static QByteArray ValueTo_Str( double r, int Decimals );  
    static int EqualGreaterInt( double r, double t );
    static bool Greater( double r, double t );
    static bool GreaterZero( double r );
    static bool Infinity( double r );
    static bool Less( double r, double t );
    static bool LessZero( double r );
    static QByteArray Str( double r, int Decimals );
    static bool Zero( double r );
    double& Value() { return Cast( TConstant, m_pExpr )->m_Value; }
  };

class DxValues : public QVector<DxValue>
  {
  public:
    struct RNumber
      {
      double *m_pValue;
      DxValues *m_pOvner;
      RNumber( double *pValue, DxValues *pOvner ) : m_pValue( pValue ), m_pOvner( pOvner ) {}
      operator double() const { return *m_pValue; }
      RNumber& operator = ( double V ) { *m_pValue = V; m_pOvner->Change(); return *this; }
      };
    void( *m_pNotifyEvent )( DxValues* );
    DxValues( int Count ) : QVector<DxValue>( Count ), m_pNotifyEvent( nullptr ) {}
    DxValues() : DxValues( 0 ) {}
  void Change();
  void SetCount( int Count ) { resize( Count ); Change(); }
  virtual RNumber operator[] ( int );
  };

class DxIntValues : public QVector<DxIntValue>
  {
  public:
    struct INumber
      {
      int *m_pValue;
      DxIntValues *m_pOvner;
      INumber( int *pValue, DxIntValues *pOvner ) : m_pValue( pValue ), m_pOvner( pOvner ) {}
      operator double() const { return *m_pValue; }
      INumber& operator = ( double V ) { *m_pValue = V; m_pOvner->Change(); return *this; }
      };
    void( *m_pNotifyEvent )( DxIntValues* );
    DxIntValues( int Count ) : QVector<DxIntValue>( Count ), m_pNotifyEvent( nullptr ) {}
    DxIntValues() : DxIntValues( 0 ) {}
    void Change();
    INumber operator[] ( int );
    DxIntValues& operator = ( const DxIntValues& IV );
  };

class DxExpressionValues : public DxValues
  {
  struct RNumber
    {
    double *m_pValue;
    RNumber( double *pValue ) : m_pValue( pValue ) {}
    operator double() const { return *m_pValue; }
    RNumber& operator = ( double V ) { *m_pValue = V; return *this; }
    };

  double m_FMax;
  double m_FMin;
  double m_FStep;
  void SetMax( double AMax );
  void SetMin( double AMin );
  void SetStep( double AStep );
  struct TMax
    {
    DxExpressionValues *m_pOvner;
    TMax() {}
    TMax( DxExpressionValues *pOwner ) : m_pOvner( pOwner ) {}
    operator double() const { return m_pOvner->m_FMax; }
    TMax& operator = ( double AMax ) { m_pOvner->SetMax( AMax ); return *this; }
    };

  struct TMin
    {
    DxExpressionValues *m_pOvner;
    TMin() {}
    TMin( DxExpressionValues *pOwner ) : m_pOvner( pOwner ) {}
    operator double() const { return m_pOvner->m_FMin; }
    TMin& operator = ( double AMin ) { m_pOvner->SetMin( AMin ); return *this; }
    };

  struct TStep
    {
    DxExpressionValues *m_pOvner;
    TStep() {}
    TStep( DxExpressionValues *pOwner ) : m_pOvner( pOwner ) {}
    operator double() const { return m_pOvner->m_FStep; }
    TStep& operator = ( double AStep ) { m_pOvner->SetStep( AStep ); return *this; }
    };

  void Update();
  public:
    QVector<double> m_ValuesX;
    DxExpressionValues( double AMin, double AMax, double AStep );
    DxExpressionValues() : DxExpressionValues( 0, 0, 0 ) {}
    DxIntValues GetBehaviour( double AMin, double AMax, DxIntValues& bhv );
    int GetIndex( double r );
    void GetRange( double& AMin, double& AMax, DxIntValues& Bhv );
    double GetReal( int Index ) { return m_FMin + Index * m_FStep; }
    TMax Max;
    TMin Min;
    TStep Step;
    RNumber operator[]( double Val );
    DxExpressionValues& operator = ( const DxExpressionValues& EV );
  };

class TSpecPoint
  {
  protected:
  uint m_FCount;
  int GetBhvValues( int Index );
  public:
    DxIntValues m_FSpcInd;
    DxIntValues m_FSpcBhv;
    DxValues m_FSpcXVal;
    DxValues m_FSpcYVal;
    TSpecPoint( int ACount );
    TSpecPoint() : TSpecPoint( 0 ) {}
    void AddPoint( int Ind, int Bhv, double x, double y );
    virtual int GetBhvByIndex( int Ind );
    uint& Count() { return m_FCount; }
    double GetRYValues( int Index );
    double GetRXValues( int Index );
    int GetIndValues( int Index );
    void Resize( int ACount );
    TSpecPoint& operator = ( const TSpecPoint& Source );
  };

class TInflSpec : public TSpecPoint
  {
  public:
    TInflSpec( int ACount ) : TSpecPoint( ACount ) {}
    TInflSpec( const TInflSpec& Source ) : TSpecPoint( Source ) {}
    int GetBhvByIndex( int Ind );
  };

class DxExpression : public MathExpr
  {
  public:
    DxExpression() : MathExpr() {}
    DxExpression( const MathExpr& ME ) : MathExpr( ME ) {}
    void GetDerivativeValues( DxExpressionValues& Vals );
    QByteArray GetOutString();
    QByteArray GetMathString();
    QByteArray GetDifMathString();
    TCarPoint GetValue( const TCarPoint& x_bhv );
    void GetValues( DxExpressionValues& Vals );
  };

struct TRealPoint
  {
  double m_X;
  double m_Y;
  TRealPoint() : m_X(0), m_Y(0) {}
  TRealPoint(double X, double Y) : m_X( X ), m_Y( Y) {}
  };

TRealPoint DeterminBhv( DxExpressionValues& Vals, DxExpression& AExpression );
