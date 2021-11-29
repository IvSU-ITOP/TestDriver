#include "DxObjects.h"
#include "MathTool.h"
#include "Parser.h"

const double DxValue::sc_valInfinity = 1e16;
const double DxValue::sc_valZero = 1e-15;
const double DxValue::sc_valCompare = 1e-14;

QByteArray DxValue::FloatTo_Str( double r, int Decimals )
  {
  return QByteArray::number( r, 'f', Decimals );
  }

bool DxValue::Equal( double r, double t )
  {
  return Zero( r - t );
  }

QByteArray DxValue::ValueTo_Str( double r, int Decimals ){ return QByteArray::number( r, 'g', Decimals ); }

int DxValue::EqualGreaterInt( double r, double t ){
  if( Greater( r, t ) ) return Round( r );
  return Round( t );
  }

bool DxValue::Greater( double r, double t )
  {
  return GreaterZero( r - t );
  }

bool DxValue::GreaterZero( double r )
  {
  return r > sc_valZero;
  }

bool DxValue::Infinity( double r ){ return abs( r ) > sc_valInfinity / 2; }

bool DxValue::Less( double r, double t ){ return LessZero( r - t ); }

bool DxValue::LessZero( double r )
  {
  return r < -sc_valZero;
  }

QByteArray DxValue::Str( double r, int Decimals ){ return ValueTo_Str( r, Decimals ); }

bool DxValue::Zero( double r )
  {
  return abs( r ) < sc_valZero;
  }

void DxValues::Change()
  {
  if( m_pNotifyEvent != nullptr ) m_pNotifyEvent( this );
  }

DxValues::RNumber DxValues::operator[] ( int i)
  {
  return RNumber( &QVector<DxValue>::operator[]( i ).Value(), this );
  }

void DxIntValues::Change()
  {
  if( m_pNotifyEvent != nullptr ) m_pNotifyEvent( this );
  }

DxIntValues::INumber DxIntValues::operator[] ( int i )
  {
  return INumber( &QVector<DxIntValue>::operator[]( i ).Value(), this );
  }

DxIntValues& DxIntValues::operator = ( const DxIntValues& IV )
  {
  clear();
  append( IV );
  return *this;
  }

DxExpressionValues::RNumber DxExpressionValues::operator[]( double Val )
  {
  return RNumber( DxValues::operator[]( GetIndex( Val ) ).m_pValue );
  }

void DxExpressionValues::SetMax( double AMax )
  {
  m_FMax = AMax;
  if( DxValue::Greater( m_FMin, m_FMax ) ) m_FMin = m_FMax;
  Update();
  }

void DxExpressionValues::SetMin( double AMin )
  {
  m_FMin = AMin;
  if( DxValue::Less( m_FMax, m_FMin ) ) m_FMax = m_FMin;
  Update();
  }

void DxExpressionValues::SetStep( double AStep )
  {
  if( DxValue::GreaterZero( AStep ) )
    m_FStep = AStep;
  else
    m_FStep = 1;
  Update();
  }

void DxExpressionValues::Update()
  {
  SetCount( Round( ( m_FMax - m_FMin ) / m_FStep ) + 1 );
  }

DxExpressionValues::DxExpressionValues( double AMin, double AMax, double AStep ) : m_FMin( AMin ), 
  Max( TMax( this ) ), Min( TMin( this ) ), Step( TStep( this ) )
  {
  if( DxValue::Less( AMax, m_FMin ) )
    m_FMax = m_FMin;
  else
    m_FMax = AMax;
  if( DxValue::GreaterZero( AStep ) )
    m_FStep = AStep;
  else
    m_FStep = 1;
  Update();
  }

DxExpressionValues& DxExpressionValues::operator = ( const DxExpressionValues& EV )
  {
  m_FMin = EV.m_FMin; 
  m_FMax = EV.m_FMax;
  m_FStep = EV.m_FStep;
  clear();
  append( EV );
  return *this;
  }

DxIntValues DxExpressionValues::GetBehaviour( double AMin, double AMax, DxIntValues& bhv )
  {
  DxIntValues Result( bhv );
  for( int i = 1; i < count(); i++ )
    if( Result[i] = bhvPInfinity )
      Result[i] = bhvScreenSpace;
    else
      if( !( DxValue::Greater( ( *this )[i], AMin - DxValue::sc_valCompare ) && DxValue::Less( ( *this )[i], AMax + DxValue::sc_valCompare ) ) )
        if( Result[i] < bhvSpace || Result[i] > bhvAsmp )
          Result[i] = bhvScreenSpace;
  return Result;
  }

int DxExpressionValues::GetIndex( double r )
  {
  if( r > m_FMax ) r = m_FMax;
  return Round( ( r - m_FMin ) / m_FStep );
  }

void DxExpressionValues::GetRange( double& AMin, double& AMax, DxIntValues& Bhv )
  {
  DefMinMax( *this, Bhv, AMin, AMax, false );
  if( AMax > 1e99 ) AMax = 1e99;
  if( AMin < -1e99 ) AMax = -1e16;
  if( DxValue::Less( AMax, AMin ) )
    {
    double r = AMin;
    AMin = AMax;
    AMax = r;
    }
  if( AMax - AMin> 1 )
    {
    if( abs( Frac( AMax ) ) > 0.999 )
      if( AMax > 0 )
        AMax += 0.001;
      else 
      AMax -= 0.001;
    if( abs( Frac( AMin ) ) > 0.999 )
      if( AMin > 0 )
        AMin += 0.001;
      else
        AMin -= 0.001;
    }
  }

void DefMinMax( DxExpressionValues& Values, DxIntValues& Bhv, double& _Y_Min, double& _Y_Max, bool SymZer )
  {
  bool InitAMinMax;

  auto SetYMinMax = [&] ( double r )
    {
    if( !InitAMinMax )
      {
      _Y_Max = r;
      _Y_Min = r;
      InitAMinMax = true;
      }
    else
      if( r > _Y_Max )
        _Y_Max = r;
      else
        if( r < _Y_Min )
          _Y_Min = r;
    };

  int Count;
  auto MathSym = [&] ()
    {
    bool FlgLPlus = false;
    bool FlgLMinus = false;
    bool FlgRPlus = false;
    bool FlgRMinus = false;

    auto In = [] ( int V )
      {
      return V >= 1 && V <= 3;
      };

    for( int i = 1; i <= Count; i++ )
      if( Bhv[i] == 4 )
        {
        int j = i + 1;
        for( ; In( Bhv[j] ) && j < Count; j++ );
        if( Bhv[j] == 7 )
          FlgLPlus = true;
        else
          if( Bhv[j] == 8 )
            FlgLMinus = true;
        j = i - 1;
        if( j > 1 )
          for( ; In( Bhv[j] ); j++ );
        if( Bhv[j] == 7 )
          FlgRMinus = true;
        else
          if( Bhv[j] == 8 )
            FlgRPlus = true;
        if( FlgLPlus && FlgRMinus || FlgLMinus && FlgRPlus ) return true;
        }
    return false;
    };

  auto SetFinalAMinMax = [&] ()
    {
    if( MathSym() )
      {
      if( abs( _Y_Min ) < abs( _Y_Max ) )
        _Y_Min = -_Y_Max;
      else
        _Y_Max = -_Y_Min;
      }

    if( abs( _Y_Min - _Y_Max ) < 1e-12 )
      {
      if( ( abs( _Y_Min ) < 1e-12 ) )
        {
        _Y_Min = -1e-12;
        _Y_Max = 1e-12;
        }
      else
        {
        if( SymZer )
          if( _Y_Max > 0 )
            {
            _Y_Min = 0;
            _Y_Max = 2 * _Y_Max;
            }
          else
            {
            _Y_Max = 0;
            _Y_Min = 2 * _Y_Min;
            }
        else
          {
          if( _Y_Min > 0 )
            _Y_Min = 0;
          else
            _Y_Max = 0;
          }
        }
      }
    };

  double DopKrut, MaxKrut, MinKrut;
  int CountNormalPoints;
  auto KrutIsDop = [&] ( int I )
    {
    double dx = abs( Values.GetReal( I ) - Values.GetReal( I + 1 ) );
    double df1 = abs( Values[I] - Values[I + 1] ) / dx;
    return df1 < DopKrut;
    };

  auto In = [] ( int V )
    {
    return V >= 1 && V <= 4;
    };

  auto FindMinMaxKrut = [&] ()
    {
    CountNormalPoints = 0;
    for( int i = 1; i <= Count; i++ )
      if( !In( Bhv[i] ) && !In( Bhv[i + 1] ) )
        {
        double dx = abs( Values.GetReal( i ) - Values.GetReal( i + 1 ) );
        double df1 = abs( Values[i] - Values[i + 1] ) / dx;
        CountNormalPoints++;
        if( CountNormalPoints == 1 )
          {
          MaxKrut = df1;
          MinKrut = df1;
          }
        if( df1 > MaxKrut )
          MaxKrut = df1;
        else
          if( df1 < MinKrut )
            MinKrut = df1;
        }
    };

  auto DefMinMaxAsmp = [&] ()
    {
    FindMinMaxKrut();
    double x0 = Values.GetReal( 1 );
    double xt = Values.GetReal( Count );
    double dx = abs( x0 - xt );
    if( CountNormalPoints > 0 )
      if( MinKrut > 20 * 20 / dx )
        DopKrut = MinKrut * 2;
      else
        DopKrut = 20 * 20 / dx;
    int k = 0, J;
    do
      {
      J = 0;
      for( int i = 1; i <= Count; i++ )
        if( !In( Bhv[i] ) && !In( Bhv[i + 1] ) )
          if( KrutIsDop( i ) )
            {
            J++;
            SetYMinMax( Values[i] );
            }
      DopKrut *= 2;
      k++;
      } while( !( InitAMinMax && ( int ) J / CountNormalPoints > 0.7 || k > 50 ) );
    };

  auto DefMinMaxNoAsmp = [&] ()
    {
    if( Values.count() == 2 )
      for( int i = 0; i < Values.count(); i++ )
        SetYMinMax( Values[i] );
    else
      for( int i = 1; i <= Count; i++ )
        if( !In( Bhv[i] ) )
          SetYMinMax( Values[i] );
    };

  InitAMinMax = false;
  _Y_Min = Max__Val;
  _Y_Max = -Max__Val;

  if( Values.count() < 2 )
    Count = 0;
  else
    Count = Values.count() - 2;
  for( int I = 1; I <= Count; I++ )
    if( Bhv[I] == 4 )
      {
      DefMinMaxAsmp();
      SetFinalAMinMax();
      return;
      }
  DefMinMaxNoAsmp();
  SetFinalAMinMax();
  }

TSpecPoint::TSpecPoint( int ACount )
  {
  Resize( ACount );
  }

void TSpecPoint::Resize( int ACount )
  {
  m_FSpcInd.resize( ACount );
  m_FSpcBhv.resize( ACount );
  m_FSpcXVal.resize( ACount );
  m_FSpcYVal.resize( ACount );
  m_FCount = 0;
  }

int TSpecPoint::GetIndValues( int Index )
  {
  return m_FSpcInd[Index];
  }

int TSpecPoint::GetBhvValues( int Index )
  {
  return m_FSpcBhv[Index];
  }

double TSpecPoint::GetRYValues( int Index )
  {
  return m_FSpcYVal[Index];
  }

double TSpecPoint::GetRXValues( int Index )
  {
  return m_FSpcXVal[Index];
  }

void TSpecPoint::AddPoint( int Ind, int Bhv, double x, double y )
  {
  if( m_FCount > 390 ) return;
  m_FSpcInd[m_FCount] = Ind;
  m_FSpcBhv[m_FCount] = Bhv;
  m_FSpcXVal[m_FCount] = x;
  m_FSpcYVal[m_FCount++] = y;
  }

TSpecPoint& TSpecPoint::operator = ( const TSpecPoint& Source )
  {
  m_FSpcInd = Source.m_FSpcInd;
  m_FSpcBhv = Source.m_FSpcBhv;
  m_FSpcXVal = Source.m_FSpcXVal;
  m_FSpcYVal = Source.m_FSpcYVal;
  m_FCount = Source.m_FCount;
  return *this;
  }

int TInflSpec::GetBhvByIndex( int Ind )
  {
  for( int i = 0; i < m_FCount; i++ )
    if( Ind <= m_FSpcInd[i] ) return m_FSpcBhv[i];
  return -1;
  }

int TSpecPoint::GetBhvByIndex( int Ind )
  {
  for( int i = 0; i < m_FCount; i++ )
    if( Ind == m_FSpcInd[i] ) return m_FSpcBhv[i];
  return -1;
  }

void DxExpression::GetDerivativeValues( DxExpressionValues& Vals )
  { 
  double x, y;

  auto Quotient = [&] ( double d )
    {
    TCarPoint tcx( x + d );
    TCarPoint tcr = GetValue( tcx );
    double r = tcr;
    if( DxValue::Infinity( r ) ) return DxValue::sc_valInfinity;
    return ( r - y ) / d;
    };

  x = Vals.Min;
  TCarPoint  x_bhv, y_bhv;
  for( int i = 0; i < Vals.count(); i++ )
    {
    x_bhv = x;
    y_bhv = GetValue( x_bhv );
    y = y_bhv;
    if( DxValue::Infinity( y ) )
      {
      Vals[i] = DxValue::sc_valInfinity;
      x = x + Vals.Step;
      continue;
      }
    double qp = Quotient( Vals.Step / 10 );
    double qm = Quotient( -Vals.Step / 10 );
    if( DxValue::Infinity( qp ) || DxValue::Infinity( qm ) )
      Vals[i] = DxValue::sc_valInfinity;
    else
      Vals[i] = ( qp + qm ) / 2;
    x = x + Vals.Step;
    }
  }

void DxExpression::GetValues( DxExpressionValues& Vals )
  {
  double r = Vals.Min;
  for( int i = 0; i < Vals.count(); i++ )
    {
    TCarPoint rTc( r );
    TCarPoint VTc = GetValue( rTc );
    Vals[i] = VTc;
    r += Vals.Step;
    }
  }

QByteArray DxExpression::GetOutString()
  {
  return WriteE();
  };

QByteArray DxExpression::GetMathString()
  {
  return WriteE();
  }

QByteArray DxExpression::GetDifMathString()
  {
  return m_pExpr->Diff().WriteE();
  }

TCarPoint DxExpression::GetValue( const TCarPoint& x_bhv )
  {
  DxExpression Left, Right;
  if( Divis( Left, Right ) )
    {
    auto Lopital = [&] ( const TCarPoint& x_bhv )
      {
      TCarPoint Result;
      Result = bhvSpace;
      DxExpression el = Left.Diff();
      DxExpression er = Right.Diff();
      QByteArray sel = el.GetMathString();
      QByteArray ser = er.GetMathString();
      if( sel.toUpper().indexOf( "ABS" ) != 1 || ser.toUpper().indexOf( "ABS" ) != 1 ) return Result;
      TCarPoint dlc = el.GetValue( x_bhv );
      TCarPoint drc = er.GetValue( x_bhv );
      if( fabs( (double) drc ) > DxValue::sc_valZero )
        {
        Result =  dlc / drc;
        if( !Right.HasUnknown().isEmpty() )
          Result = bhvLimit;
        else
          Result = bhvLimitY;
        }
      return Result;
      };

    TCarPoint lc = Left.GetValue( x_bhv );
    TCarPoint rc = Right.GetValue( x_bhv );

    if( lc == bhvSpace || rc == bhvSpace || lc == bhvPInfinity && rc == bhvPInfinity )
      return TCarPoint( bhvSpace );

    if( rc == bhvPInfinity )
      return TCarPoint();

    if( lc == bhvPInfinity )
      return TCarPoint( bhvPInfinity );

    if( fabs( (double) rc ) < DxValue::sc_valZero )
      {
      if( fabs( (double) lc ) < DxValue::sc_valZero )
        return Lopital( x_bhv );
      else
        return TCarPoint( bhvSpace );
      }

    return TCarPoint( lc / rc );
    }

  double X;
  bool OldFullReduce = TExpr::sm_FullReduce;
  TExpr::sm_FullReduce = true;
  try
    {
    Reduce().Constan(X);
    }
  catch( ErrParser E )
    {
    TExpr::sm_FullReduce = OldFullReduce;
    if( E.ErrStatus() == peInfinity ) return TCarPoint( bhvPInfinity );
    return TCarPoint( bhvSpace );
    }
  TExpr::sm_FullReduce = OldFullReduce;
  return TCarPoint( X );
  }
