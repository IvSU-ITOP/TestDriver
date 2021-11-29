#include "DxObjects.h"
#include "Parser.h"
#include "MathTool.h"
#include "SolChain.h"
#include "ExpObBin.h"
#include "Factors.h"

DxExpressionValues _Vals;
DxExpression e;
TSpecPoint _SpcPnt;
int SigneDerivativ;
DxIntValues _bhv, _bhvr;
int Lim;
int Count;
bool StartXMin;
bool StartXMax;

typedef QVector<int> IntArray;
IntArray bhvDec, bhvInc, bhvLim;

int ConvexConcav( int k, DxExpressionValues _Vals_ )
  {
  double x1 = _Vals_.GetReal( k - 1 );
  double x2 = _Vals_.GetReal( k + 1 );
  double xo = _Vals_.GetReal( k );
  double y1 = _Vals_[k - 1];
  double y2 = _Vals_[k + 1];
  double yo = _Vals_[k];
  double ddy = yo - y1 + ( y1 - y2 )*( xo - x1 ) / ( x2 - x1 );
  if( ddy > 30 * DxValue::sc_valZero ) return bhvConvex;
  if( ddy < -30 * DxValue::sc_valZero ) return bhvConcavity;
  return bhvStrigth;
  }

double Derivativ( double x, double dx )
  {
  TCarPoint f1(x), f2;
  double C, acur, df2;
  f1 = e.GetValue( f1 );
  if( abs( x ) < 1 )
    C = 1e-200;
  else
  C = 1e-9;
  do
    {
    f2 += dx;
  f2 = bhvNormal;
  f2 = e.GetValue( f2 );
  double df1 = ( f2 - f1 ) / dx;
  dx /= 2;
  f2 = x + dx;
  f2 = bhvNormal;
  f2 = e.GetValue( f2 );
  df2 = ( f2 - f1 ) / dx;
  if( abs( df2 ) < 0.0001 )
    acur = abs( df1 - df2 );
  else
  acur = abs( df1 / df2 - 1 );
    } while ( acur > 0.001 && abs( dx ) < C );

  if( df2 > 1e-2 )
    SigneDerivativ = 1;
  else
    if( df2 < -1e-2 )
      SigneDerivativ = -1;
  else
  SigneDerivativ = 0;
  return df2;
  }

TRealPoint DeterminBhv( DxExpressionValues& Vals, DxExpression& AExpression )
  {

  auto SearchLeftBorder = [&] ()
    {
    int Result = _bhv[0];
    double XLeft = _Vals.GetReal( 0 );
    double XRight = _Vals.GetReal( 1 );
    double interval = abs( XRight - XLeft ) / 400;
    double x0 = XLeft;
    double x1 = x0 + interval;
    double x2 = x0 + 3 * interval;
    TCarPoint YRight( x1 );
    YRight = e.GetValue( YRight );
    TCarPoint YLeft( x1 );
    YLeft = e.GetValue( YLeft );
    if( YRight == bhvNormal && YLeft == bhvNormal && _bhv[1] == bhvNormal )
      {
      double dif1 = Derivativ( x1, interval );
      double dif2 = Derivativ( x2, interval );
      double XProb = abs( x2 - x0 );
      double XExprleft = abs( dif2*abs( x2 - x0 )*log( XProb ) );
      XProb = abs( x1 - x0 );
      double XExprRight = abs( dif1*abs( x1 - x0 )*log( XProb ) );
      if( XExprleft <= XExprRight )
        {
        Result = bhvAsmp;
        Lim = 1;
        if( SigneDerivativ == -1 )
          bhvDec.append( Lim );
        else
          if( SigneDerivativ == 1 )
            bhvInc.append( Lim );
          else
            Result = bhvSpace;
        }
      };
    _SpcPnt.AddPoint( 0, Result, XLeft, YLeft );
    return Result;
    };

  int i;
  auto SearchRightBorder = [&] ()
    {
    int Result = _bhv[Count - 1];
    double XLeft = _Vals.GetReal( Count - 2 );
    double XRight = _Vals.GetReal( Count - 1 );
    double interval = abs( XRight - XLeft ) / 400;
    double x0 = XRight;
    double x1 = x0 - interval;
    double x2 = x0 - 3 * interval;
    TCarPoint YRight( x1 );
    YRight = e.GetValue( YRight );
    TCarPoint YLeft( x1 );
    YLeft = e.GetValue( YLeft );
    if( YRight == bhvNormal && YLeft == bhvNormal && _bhv[Count - 2] == bhvNormal )
      {
      double dif1 = Derivativ( x1, interval );
      double dif2 = Derivativ( x2, interval );
      double XProb = abs( x2 - x0 );
      double XExprleft = abs( dif2*abs( x2 - x0 )*log( XProb ) );
      XProb = abs( x1 - x0 );
      double XExprRight = abs( dif1*abs( x1 - x0 )*log( XProb ) );
      if( XExprleft <= XExprRight ) {
        Result = bhvAsmp;
        Lim = Count - 2;
        if( SigneDerivativ == -1 )
          bhvDec.append( Lim );
        else
          if( SigneDerivativ == 1 )
            bhvInc.append( Lim );
          else
            Result = bhvSpace;
        }
      }
    _SpcPnt.AddPoint( i, Result, XRight, YRight );
    return Result;
    };

  auto SearchRootByPoints = [&] ( double x1, double x2 )
    {
    double a = x1;
    double b = x2;
    double xRoot1 = a;
    double xRoot2 = b;
    do
      {
      TCarPoint YTC( a );
      YTC = e.GetValue( YTC );
      double y1 = YTC;
      YTC = b;
      YTC = e.GetValue( YTC );
      double y2 = YTC;
      if( y2 == y1 ) break;
      xRoot1 = a - ( b - a )*y1 / ( y2 - y1 );
      double h = abs( b - a ) / 10;
      double yd = Derivativ( a, h );
      xRoot2 = a - y1 / yd;
      if( xRoot1<xRoot2 )
        {
        a = xRoot1; b = xRoot2;
        }
      else
        {
        a = xRoot2; b = xRoot1;
        }
      } while( abs( xRoot2 - xRoot1 ) >( x2 - x1 ) / 1000 );
      double Result = ( xRoot1 + xRoot2 ) / 2;
      _SpcPnt.AddPoint( i, bhvRoot, 0.001*Round( 1000 * Result ), 0 );
      return Result;
    };

  auto SearchLimitSpace = [&] ()
    {
    double YLimit = 0;
    double x0 = 0;
    bool FlgRoot = false;
    double XLeftRoot = 0;
    double XRightRoot = 0;
    double XLeft = _Vals.GetReal( i - 1 );
    TCarPoint YLeft( _Vals[i - 1], _bhv[i - 1] );
    double XRight = _Vals.GetReal( i + 1 );
    TCarPoint YRight( _Vals[i + 1], _bhv[i + 1] );
    double interval = abs( XRight - XLeft ) / 4000;
    do
      {
      if( YLeft == bhvSpace && YRight != bhvSpace )
        {
        double XProb = ( XRight + XLeft ) / 2;
        TCarPoint YProb( XProb );
        YProb = e.GetValue( YProb );
        if( YProb == bhvSpace )
          {
          YLimit = YRight;
          XLeft = XProb;
          YLeft = YProb;
          }
        else
          {
          if( YRight > 0.0 && YProb < 0.0 || YProb > 0.0 && YRight < 0.0 )
            {
            FlgRoot = true;
            XLeftRoot = XProb;
            XRightRoot = XRight;
            }
          YLimit = YProb;
          XRight = XProb;
          YRight = YProb;
          }
        }
      else
        if( YLeft != bhvSpace && YRight == bhvSpace )
          {
          double XProb = ( XRight + XLeft ) / 2;
          TCarPoint YProb( XProb );
          YProb = e.GetValue( YProb );
          if( YProb == bhvSpace )
            {
            YLimit = YLeft;
            XRight = XProb;
            YRight = YProb;
            }
          else
            {
            if( YLeft > 0.0 && YProb < 0.0 || YProb > 0.0 && YLeft < 0.0 )
              {
              FlgRoot = true;
              XLeftRoot = XLeft;
              XRightRoot = XProb;
              };
            YLimit = YProb;
            XLeft = XProb;
            YLeft = YProb;
            }
          }
      } while( YLeft != bhvPInfinity && YRight != bhvPInfinity && abs( XRight - XLeft ) > 0.01*interval );
      int Result;
      if( !( YLeft == bhvPInfinity || YRight == bhvPInfinity ) )
        {
        double x1, x2, dif1, dif2;
        if( YLeft != bhvSpace )
          {
          x0 = XLeft;
          x1 = XLeft - interval;
          dif1 = Derivativ( x1, interval );
          x2 = XLeft - 3 * interval;
          dif2 = Derivativ( x2, interval );
          }
        else
          {
          x0 = XRight;
          x1 = XRight + interval;
          dif1 = Derivativ( x1, interval );
          x2 = XRight + 3 * interval;
          dif2 = Derivativ( x2, interval );
          }
        double XProb = abs( x2 - x0 );
        double XExprleft = abs( dif2*abs( x2 - x0 )*log( XProb ) );
        XProb = abs( x1 - x0 );
        double XExprRight = abs( dif1*abs( x1 - x0 )*log( XProb ) );
        if( XExprleft <= XExprRight )
          {
          Result = bhvAsmp;
          if( _bhv[i + 1] == bhvNormal )
            Lim = ( i + 1 );
          else
            Lim = ( i - 1 );
          if( SigneDerivativ == -1 )
            bhvDec.append( Lim );
          else
            if( SigneDerivativ == 1 )
              bhvInc.append( Lim );
            else
              Result = bhvSpace;
          }
        else
          {
          Result = bhvLimit;
          if( _bhv[i + 1] == bhvSpace )
            _Vals[i - 1] = YLimit;
          else
            _Vals[i + 1] = YLimit;
          }
        }
      else
        Result = bhvAsmp;
      _SpcPnt.AddPoint( i, Result, 0.001*Round( 1000 * x0 ), YLimit );
      if( FlgRoot && abs( YLimit ) > 0.0001 )
        SearchRootByPoints( XLeftRoot, XRightRoot );
      return Result;
    };

  TCarPoint yTC;
  double Ylimit;
  auto FindInfinity = [&] ( int k, double dx, double x0 )
    {
    int m = 1;
    double xx1;
    do
      {
      xx1 = x0 + k*m*dx;
      yTC = xx1;
      yTC = bhvNormal;
      yTC = e.GetValue( yTC );
      if( m++ > 260 )
        return false;
      } while( yTC != bhvNormal );
      Ylimit = yTC;
      double dif1 = Derivativ( xx1, dx );
      double xx2 = x0 + k*m*dx;
      double dif2 = Derivativ( xx2, dx );
      double dx1 = abs( xx1 - x0 );
      double dx2 = abs( xx2 - x0 );
      if( abs( dif1 ) < 1e-2 || abs( dif2 ) < 1e-2 ) return false;
      if( abs( dif2*dx2*log( dx2 ) ) - abs( dif1*dx1*log( dx1 ) ) < 0 ) return true;
      return false;
    };

  auto FindJump = [&] ( int k )
    {
    double XMin = _Vals.GetReal( i - 1 );
    double XMax = _Vals.GetReal( i + 1 );
    double XP;
    TCarPoint Y1, Y2, YP;
    do
      {
      Y1 = XMin;
      Y1 = bhvNormal;
      Y1 = e.GetValue( Y1 );

      Y2 = XMax;
      Y2 = bhvNormal;
      Y2 = e.GetValue( Y2 );

      XP = ( XMin + XMax ) / 2;
      YP = XP;
      YP = bhvNormal;
      YP = e.GetValue( YP );
      if( YP == 1 || YP == 2 )
        {
        if( k == 1 )
          {
          _Vals[i] = Y1;
          _Vals[i + 1] = Y2;
          }
        _SpcPnt.AddPoint( i, bhvJump, Round( 100 * XP ) / 100, YP );
        return true;
        }
      if( abs( Y1 - YP ) < abs( Y2 - YP ) )
        XMin = XP;
      else
        XMax = XP;
      } while( abs( Y1 - YP ) >= 0.001  && abs( Y2 - YP ) >= 0.001 && abs( XMin - XMax ) >= 0.001 );

      if( abs( Y1 - YP ) < 0.01 && abs( Y2 - YP ) < 0.01 ) return false;
      if( abs( Y2 - Y1 ) > 1000 ) return false;
      if( k == 1 )
        {
        _Vals[i] = Y1;
        _Vals[i + 1] = Y2;
        }
      _SpcPnt.AddPoint( i, bhvJump, Round( 100 * XP ) / 100, YP );
      return true;
    };

  auto SearchLimit = [&] ( int k )
    {
    double XMini, XMaxi;
    TCarPoint YTC;
    auto ChekMinMax = [&] ()
      {
      double xl = Round( 1000 * XMini ) / 1000;
      double xr = Round( 1000 * XMaxi ) / 1000;
      YTC = xl;
      YTC = bhvNormal;
      YTC = e.GetValue( YTC );
      if( YTC == bhvNormal )
        {
        YTC = xr;
        YTC = bhvNormal;
        YTC = e.GetValue( YTC );
        if( YTC == bhvNormal ) return true;
        return false;
        }
      return true;
      };

    int Result = bhvNormal;
    double X0 = 0;
    int N = 5;
    double relation1, relation2, X1, X2, dX, Y;

    do
      {
      X1 = _Vals.GetReal( k - 1 );
      X2 = _Vals.GetReal( k + 1 );
      N *= 2;
      dX = ( X2 - X1 ) / N;
      double X = X1;
      XMaxi = X1;
      XMini = X1;
      YTC = X1;
      YTC = bhvNormal;
      YTC = e.GetValue( YTC );
      double YMaxi = YTC;
      double YMini = YMaxi;
      double YMaxi1 = YMaxi;
      double YMini1 = YMini;
      do
        {
        X += dX;
        YTC = X;
        YTC = bhvNormal;
        YTC = e.GetValue( YTC );
        Y = YTC;
        if( YTC == bhvNormal )
          {
          if( Y > YMaxi + DxValue::sc_valZero )
            {
            YMaxi = Y;
            XMaxi = X;
            }
          else
            if( Y < YMini - DxValue::sc_valZero )
              {
              YMini = Y;
              XMini = X;
              }
          }
        else
          {
          X0 = X;
          break;
          }
        } while( X < X2 );

        if( abs( YMini ) < 0.01 )
          relation1 = abs( YMini1 - YMini );
        else
          relation1 = abs( YMini1 / YMini - 1 );
        if( abs( YMaxi ) < 0.01 )
          relation2 = abs( YMaxi1 - YMaxi );
        else
          relation2 = abs( YMaxi1 / YMaxi - 1 );
      } while( ( relation1 >= 0.0001 || relation2 >= 0.0001 ) && ( N < 160 ) && YTC == bhvNormal );
      if( YTC == bhvNormal )
        if( XMaxi > X1 + DxValue::sc_valZero && XMaxi < X2 - DxValue::sc_valZero )
          X0 = XMaxi;
        else
          X0 = XMini;

      dX *= 3;
      bool FlgLeftLimit = false;
      if( !FindInfinity( -1, dX, X0 ) )
        FlgLeftLimit = true;
      else
        {
        Result = bhvAsmp;
        if( SigneDerivativ == -1 )
          {
          Lim = ( k - 1 );
          bhvDec.append( Lim );
          }
        else
          if( SigneDerivativ == 1 )
            {
            Lim = ( k - 1 );
            bhvInc.append( Lim );
            }
        }

      if( !FindInfinity( 1, dX, X0 ) )
        {
        if( Result != bhvAsmp )
          {
          if( FindJump( 1 ) )
            {
            Result = bhvJump;
            if( SigneDerivativ == -1 )
              {
              Lim = ( k + 1 );
              bhvDec.append( Lim );
              }
            else
              if( SigneDerivativ == 1 )
                {
                Lim = ( k + 1 );
                bhvInc.append( Lim );
                }
            }
          }
        else
          {
          Lim = ( k + 1 );
          bhvLim.append( Lim );
          }
        }
      else
        {
        Result = bhvAsmp;
        if( FlgLeftLimit )
          {
          Lim = ( k - 1 );
          bhvLim.append( Lim );
          };
        if( SigneDerivativ == -1 )
          {
          Lim = ( k + 1 );
          bhvDec.append( Lim );
          }
        else
          if( SigneDerivativ == 1 )
            {
            Lim = k + 1;
            bhvInc.append( Lim );
            }
        }

      if( Result == bhvAsmp || Result == bhvJump )
        {
        if( Result == bhvAsmp )
          _SpcPnt.AddPoint( i, Result, Round( 100 * X0 ) / 100, Y );
        return Result;
        }

      if( ChekMinMax() )
        {
        if( abs( XMini - X1 ) > abs( XMini - X2 ) )
          XMini = abs( XMini - X2 );
        else
          XMini = abs( XMini - X1 );
        if( abs( XMaxi - X1 ) > abs( XMaxi - X2 ) )
          XMaxi = abs( XMaxi - X2 );
        else
          XMaxi = abs( XMaxi - X1 );
        if( XMaxi > XMini )
          Result = bhvMax;
        else
          Result = bhvMin;
        }
      else
        Result = bhvLimit;
      _SpcPnt.AddPoint( i, Result, 0.001*( round( 1000 * X0 ) ), 0.001*( round( 1000 * Ylimit ) ) );
      return Result;
    };

  auto SearchRoot = [&] ( int k )
    {
    double x1 = _Vals.GetReal( k - 1 );
    double x2 = _Vals.GetReal( k );
    return SearchRootByPoints( x1, x2 );
    };

  auto Searchinflation = [&] ( int n, int k )
    {
    TCarPoint YTC;
    auto Y_fx = [&] ( double x )
      {
      YTC = x;
      YTC = bhvNormal;
      YTC = e.GetValue( YTC );
      return ( double ) YTC;
      };

    double x1 = _Vals.GetReal( n - 1 );
    double x2 = _Vals.GetReal( n + 2 );
    double dx = ( x2 - x1 ) / 100;
    double xmid = ( x1 + x2 ) / 2;
    int Ncount = 0;
    double ddyR, ddyL, yf;
    do
      {
      Ncount++;
      double dy = Derivativ( xmid, dx );
      yf = Y_fx( xmid );
      double yfR = Y_fx( xmid + dx );
      double yfL = Y_fx( xmid - dx );
      ddyR = yfR - ( yf + dy*dx );
      ddyL = yfL - ( yf - dy*dx );
      if( ddyR > DxValue::sc_valZero && ddyL > DxValue::sc_valZero && k == bhvConcavity || ddyR < DxValue::sc_valZero && ddyL < DxValue::sc_valZero
        && k == bhvConvex )
        {
        x1 = xmid;
        xmid = ( x2 + xmid ) / 2;
        }
      else
        if( ddyR<DxValue::sc_valZero && ddyL<DxValue::sc_valZero && k == bhvConcavity || ddyR>DxValue::sc_valZero && ddyL>DxValue::sc_valZero &&
          k == bhvConvex )
          {
          x2 = xmid;
          xmid = ( x1 + xmid ) / 2;
          }
      } while( ( ddyR >= DxValue::sc_valZero || ddyL <= DxValue::sc_valZero ) && ( ddyR <= DxValue::sc_valZero || ddyL >= DxValue::sc_valZero )
        && Ncount < 260 );
      if( k == bhvConcavity )
        k = bhvInflConcav;
      else
        k = bhvInflConvex;

      _SpcPnt.AddPoint( n, k, 0.001*Round( 1000 * xmid ), 0.001*Round( 1000 * yf ) );
    };

  auto NormInfInf = [&] ()
    {
    return _bhv[i - 1] == bhvNormal && _bhv[i] == bhvPInfinity && _bhv[i + 1] == bhvPInfinity;
    };

  auto InfInfNorm = [&] ()
    {
    return _bhv[i - 1] == bhvPInfinity && _bhv[i] == bhvPInfinity && _bhv[i + 1] == bhvNormal;
    };

  auto NormSpcInf = [&] ()
    {
    return _bhv[i - 1] == bhvNormal && _bhv[i] == bhvSpace && _bhv[i + 1] == bhvPInfinity;
    };

  auto InfSpcNorm = [&] ()
    {
    return _bhv[i - 1] == bhvPInfinity && _bhv[i] == bhvSpace && _bhv[i + 1] == bhvNormal;
    };

  auto NormInfSpc = [&] ()
    {
    return _bhv[i - 1] == bhvNormal && _bhv[i] == bhvPInfinity && _bhv[i + 1] == bhvSpace;
    };

  auto SpcInfNorm = [&] ()
    {
    return _bhv[i - 1] == bhvSpace && _bhv[i] == bhvPInfinity && _bhv[i + 1] == bhvNormal;
    };

  auto NormalPInfinityPInfinity = [&] ()
    {
    return _bhv[i - 1] == bhvNormal &&
      ( _bhv[i] == bhvPInfinity && _bhv[i + 1] == bhvSpace || _bhv[i] == bhvPInfinity && _bhv[i + 1] == bhvPInfinity || _bhv[i] == bhvSpace &&
      _bhv[i + 1] == bhvPInfinity );
    };

  auto PInfinityPInfinityNormal = [&] ()
    {
    return _bhv[i + 1] == bhvNormal &&
      ( _bhv[i] == bhvPInfinity && _bhv[i - 1] == bhvSpace || _bhv[i] == bhvPInfinity && _bhv[i - 1] == bhvPInfinity || _bhv[i] == bhvSpace &&
      _bhv[i - 1] == bhvPInfinity );
    };

  auto NormalNormalNormal = [&] ()
    {
    return _bhv[i - 1] == bhvNormal && _bhv[i] == bhvNormal && _bhv[i + 1] == bhvNormal;
    };

  auto ValEqual = [&] ()
    {
    double Vi1, Vi2;
    if( abs( _Vals[i] ) < 1e-8 ) {
      Vi1 = 1e8*abs( _Vals[i - 1] - _Vals[i] );
      Vi2 = 1e8*abs( _Vals[i] - _Vals[i + 1] );
      }
    else
      {
      Vi1 = abs( ( _Vals[i - 1] - _Vals[i] ) / _Vals[i] );
      Vi2 = abs( ( _Vals[i] - _Vals[i + 1] ) / _Vals[i] );
      }
    return Vi1 < 1e-6 && Vi2 < 1e-6;
    };

  auto ValChangeZine = [&] ( int i )
    {
    return _Vals[i - 1] <= 0 && _Vals[i] >0 || _Vals[i - 1] >= 0 && _Vals[i] < 0;
    };

  auto ValIncrease = [&] ( int i )
    {
    return _Vals[i - 1] < _Vals[i] - DxValue::sc_valZero * 30 && _Vals[i] < _Vals[i + 1] - DxValue::sc_valZero * 30;
    };

  auto ValDecrease = [&] ( int i )
    {
    return _Vals[i - 1] > _Vals[i] + DxValue::sc_valZero * 30 && _Vals[i] > _Vals[i + 1] + DxValue::sc_valZero * 30;
    };

  auto ValJump = [&] ()
    {
    return i < _Vals.count() - 5 &&
      ( ValIncrease( i ) && ValIncrease( i + 1 ) && ValIncrease( i + 2 ) || ValDecrease( i ) && ValDecrease( i + 1 ) && ValDecrease( i + 2 ) ) &&
      ( abs( _Vals[i] - _Vals[i - 1] ) < 0.3*abs( _Vals[i + 1] - _Vals[i] ) && abs( _Vals[i + 1] - _Vals[i + 2] ) < 0.3*abs( _Vals[i + 1] - _Vals[i] ) ) &&
      FindJump( 0 );
    };

  auto ValJumpDoun = [&] ()
    {
    return !( _bhvr[i - 1] == bhvAsmp || _bhvr[i - 1] == bhvJump ) &&
      ( _Vals[i - 1] <= _Vals[i] || abs( _Vals[i - 1] - _Vals[i] ) < 1e-12 && _Vals[i] > _Vals[i + 1] + DxValue::sc_valZero );
    };

  auto ValJumpUp = [&] ()
    {
    return !( _bhvr[i - 1] == bhvAsmp || _bhvr[i - 1] == bhvJump ) &&
      ( ( _Vals[i - 1] >= _Vals[i] || abs( _Vals[i - 1] - _Vals[i] ) < 1e-12 && _Vals[i] < _Vals[i + 1] - DxValue::sc_valZero ) ||
      abs( _Vals[i + 1] - _Vals[i] ) < 1e-12 && _Vals[i] > _Vals[i - 1] - DxValue::sc_valZero );
    };

  auto CheckInflation = [&] ()
    {
    for( int k = 1; k <= 4; k++ )
      if( !( _bhvr[i - k] == bhvIncrease || _bhvr[i - k] == bhvDecrease || _bhvr[i - k] == bhvLimit ) ) return;
    int cnv3 = ConvexConcav( i - 3, _Vals );
    int cnv2 = ConvexConcav( i - 2, _Vals );
    if( cnv3 == bhvConcavity && ( cnv2 == bhvConvex || cnv2 == bhvStrigth ) )
      Searchinflation( i - 3, bhvConcavity );
    if( cnv3 == bhvConvex && ( cnv2 == bhvConcavity || cnv2 == bhvStrigth ) )
      Searchinflation( i - 3, bhvConvex );
    };

  auto PointBhvNormal = [&] ()
    {
    if( i >= 5 ) CheckInflation();
    if( ValEqual() )
      _bhvr[i] = bhvNormal;
    else
      if( ValJumpDoun() )
        _bhvr[i] = SearchLimit( i );
      else
        if( ValJumpUp() )
          _bhvr[i] = SearchLimit( i );
        else
          if( ValJump() )
            _bhvr[i] = bhvJump;
          else
            if( ValIncrease( i ) )
              {
              _bhvr[i] = bhvIncrease;
              if( ValChangeZine( i ) )
                SearchRoot( i );
              }
            else
              if( ValDecrease( i ) )
                {
                _bhvr[i] = bhvDecrease;
                if( ValChangeZine( i ) )
                  SearchRoot( i );
                }
    };

  auto ValSinglSpace = [&] ()
    {
    return _bhv[i - 1] == bhvNormal && ( _bhv[i] == bhvSpace || _bhv[i] == bhvPInfinity ) && _bhv[i + 1] == bhvNormal;
    };

  auto SpaceRightInfinit = [&] ()
    {
    return _bhv[i - 1] == bhvPInfinity && _bhv[i] == bhvSpace && _bhv[i + 1] == bhvSpace;
    };

  auto SpaceRight = [&] ()
    {
    return _bhv[i - 1] == bhvNormal && _bhv[i] == bhvSpace && _bhv[i + 1] == bhvSpace;
    };

  auto SpaceLeftInfinit = [&] ()
    {
    return _bhv[i - 1] == bhvSpace && _bhv[i] == bhvSpace && _bhv[i + 1] == bhvPInfinity;
    };

  auto SpaceLeft = [&] ()
    {
    return _bhv[i - 1] == bhvSpace && _bhv[i] == bhvSpace && _bhv[i + 1] == bhvNormal;
    };

  double X0, dX;
  auto InitBorder = [&] ()
    {
    int j = i;
    for( ; _bhv[j] != bhvNormal && j <= Count - 2; j++ )
      if( j > Count - 2 ) return false;
    double x1 = _Vals.GetReal( i );
    double x2 = _Vals.GetReal( i + 1 );
    x1 = abs( x1 - x2 );
    dX = x1*( j - i ) * 5;
    X0 = _Vals.GetReal( ( ( i + j ) / 2 ) );
    return true;
    };

  bool FlagAsmp;
  auto ResearchLeftBorder = [&] ()
    {
    FlagAsmp = false;
    if( !InitBorder() ) return;
    if( !FindInfinity( -1, dX, X0 ) )
      _bhvr[i - 1] = bhvLimit;
    else
      {
      FlagAsmp = true;
      if( SigneDerivativ < 0 )
        _bhvr[i - 1] = bhvDecrease;
      else
        _bhvr[i - 1] = bhvIncrease;
      }
    };

  auto ResearchRightBorder = [&] ()
    {
    if( !InitBorder() ) return;
    if( !FindInfinity( 1, dX, X0 ) )
      _bhvr[i + 1] = bhvLimit;
    else
      {
      FlagAsmp = true;
      if( SigneDerivativ < 0 )
        _bhvr[i + 1] = bhvDecrease;
      else
        _bhvr[i + 1] = bhvIncrease;
      }
    int j = i - 1;
    for( ; !_bhv[j] != bhvNormal; j-- );
    if( _bhvr[i + 1] == bhvNormal && _bhvr[j] == bhvNormal ) return;
    if( FlagAsmp )
      if( _bhvr[i + 1] == bhvLimit )
        _bhvr[i] = bhvAsmp;
      else
        if( _bhvr[j] == bhvLimit )
          _bhvr[j + 1] = bhvAsmp;
        else
          _bhvr[( i + j ) / 2] = bhvAsmp;
    };

  Count = Vals.count();
  _bhv.resize( Count );
  _bhvr.resize( Count );
  _SpcPnt.Resize( Count );
  e = AExpression;
  _Vals = Vals;
  TRealPoint Result( Vals.GetReal( 0 ), Vals.GetReal( Count - 1 ) );
  StartXMin = true;
  StartXMax = true;
  double xold = Vals.GetReal( 0 );
  for( i = 0; i < Count; i++ )
    {
    yTC = bhvNormal;
    yTC = Vals.GetReal( i );
    if( StartXMin ) Result.m_X = xold;
    xold = yTC;
    if( StartXMax && !StartXMin ) Result.m_Y = yTC;
    yTC = e.GetValue( yTC );
    _Vals[i] = yTC;
    _bhv[i] = yTC;
    _bhvr[i] = yTC;
    StartXMin = StartXMin && ( yTC != bhvNormal );
    if( !StartXMin )
      StartXMax = StartXMax && ( yTC == bhvNormal );
    }

  _bhvr[0] = SearchLeftBorder();

  i = 1;
  int SLS;
  for( ; i < Count - 1 && _bhvr[i] != bhvPlotSpace; i++ )
    {
    if( NormalNormalNormal() )
      try
      {
      PointBhvNormal();
      }
    catch( ErrParser ) {}
    else
      if( ValSinglSpace() )
        _bhvr[i] = SearchLimit( i );
      else
        if( SpaceRight() )
          {
          SLS = SearchLimitSpace();
          if( SLS == bhvAsmp )
            _bhvr[i] = SLS;
          else
            _bhvr[i - 1] = SLS;
          }
        else
          if( SpaceLeft() )
            {
            SLS = SearchLimitSpace();
            if( SLS == bhvAsmp )
              _bhvr[i] = SLS;
            else
              _bhvr[i + 1] = SLS;
            }
          else
            if( NormalPInfinityPInfinity() )
              {
              ResearchLeftBorder();
              do
                i++;
              while( !PInfinityPInfinityNormal() && i <= Count - 2 );
              if( !( i > Count - 2 ) )
                ResearchRightBorder();
              }
    }

  if( !( _bhvr[Count - 2] == bhvAsmp ) )
    _bhvr[Count - 1] = SearchRightBorder();

  for( i = 0; i < bhvLim.count(); i++ )
    {
    Lim = bhvLim[i];
    if( _bhvr[Lim] != bhvAsmp )
      _bhvr[Lim] = bhvLimit;
    }
  for( i = 0; i < bhvDec.count(); i++ )
    {
    Lim = bhvDec[i];
    if( _bhvr[Lim] != bhvAsmp )
      _bhvr[Lim] = bhvDecrease;
    }
  for( i = 0; i < bhvInc.count(); i++ )
    {
    Lim = bhvInc[i];
    if( _bhvr[Lim] != bhvAsmp )
      _bhvr[Lim] = bhvIncrease;
    }

  for( i = 0; i < _bhvr.count() - 1; i++ )
    if( _bhvr[i] == bhvIncrease || _bhvr[i] == bhvDecrease )
      if( _bhvr[i + 1] == bhvNormal )
        _bhvr[i + 1] = _bhvr[i];

  for( i = _bhvr.count() - 1; i >= 1; i-- )
    if( _bhvr[i] == bhvIncrease || _bhvr[i] == bhvDecrease )
      if( _bhvr[i - 1] == bhvNormal )
        _bhvr[i - 1] = _bhvr[i];
  Vals = _Vals;
  return Result;
  }

MathExpr CalcAnyEquation( const QByteArray& Source )
  {
  if( Source.isEmpty() ) return MathExpr();
  auto ExprValue = [] ( const MathExpr& ex1, double& x )
    {
    MathExpr ex = ex1.Reduce();
    bool Result = true;
    bool sign = ex.Unarminus( ex );
    int N, D;
    if( ex.SimpleFrac_( N, D ) )
      x = ( double ) N / D;
    else
      if( !ex.Constan( x ) )
        Result = false;
    if( sign )
      x = -x;
    return Result;
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  MathExpr Result;

  auto Final = [&] ()
    {
    s_Precision = OldPrecision;
    return Result;
    };

  auto BadResult = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MLinInv", "Invalid input data!" ) );
    return Final();
    };

  try
    {
    Parser P;
    PNode eq = P.AnyExpr( PiVar2PiConst( P.FullPreProcessor( Source, "x" ) ) );
    MathExpr ex = P.OutPut( eq );
    PExMemb f;
    if( ex.Listex( f ) )
      {
      if( f.isNull() ) return BadResult();
      MathExpr ex1 = f->m_Memb;
      MathExpr op1, op2;
      if( !ex1.Binar( '=', op1, op2 ) ) return BadResult();
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      ex1 = op1 - op2;
      f = f->m_pNext;
      if( f.isNull() ) return BadResult();
      PExMemb f1;
      if( f->m_Memb.Listex( f1 ) ) f = f1;
      double a;
      if( !ExprValue( f->m_Memb, a ) ) return BadResult();
      f = f->m_pNext;
      if( f.isNull() ) return BadResult();
      double b;
      if( !ExprValue( f->m_Memb, b ) ) return BadResult();
      f = f->m_pNext;
      if( f.isNull() ) return BadResult();
      if( a < b )
        {
        DxExpressionValues FValues( a, b, abs( ( a - b ) / 400 ) );
        DxIntValues FBhv( 401 );
        TSpecPoint FSpecPoint( 400 );
        try
          {
          DxExpression DxEx( ex1 );
          DeterminBhv( FValues, DxEx );
          FBhv = _bhvr;
          bool FlgFindRoot;
          FSpecPoint = _SpcPnt;
          FlgFindRoot = false;
          for( int i = 0; i < FSpecPoint.Count(); i++ )
            if( FSpecPoint.m_FSpcBhv[i] == bhvRoot )
              {
              FlgFindRoot = true;
              double c = FSpecPoint.m_FSpcXVal[i];
              MathExpr  ex2 = new TBinar( '=', Variable( "x" ), Constant( c ) );
              Result = ex2;
              TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
              }
          if( FlgFindRoot == true )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRoot", "Roots are found!" ) );
          else
            return BadResult();
          }
        catch( ErrParser ) {}
        }
      else
        return BadResult();
      }
    else
      {
      Lexp ex1 = RootPolinom( ex );
      if( ex1.IsEmpty() || ex1.First().isNull() ) return BadResult();
      Result = ex1;
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      TSolutionChain::sm_SolutionChain.AddExpr( ex1, X_Str( "MRoot", "Roots are found!" ) );
      }
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Final();
  }
 