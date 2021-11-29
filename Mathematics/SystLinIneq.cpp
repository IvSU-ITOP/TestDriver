#include "Algebra.h"
#include "ExpObBin.h"
#include "MathTool.h"
#include "Factors.h"
#include <qalgorithms.h>
#include "SolChain.h"

extern int s_DegPoly;
struct PoligonPoint
  {
  double X, Y;
  PoligonPoint( double x = 0, double y = 0 ) : X( x ), Y( y ) {}
  };
PascArray<PoligonPoint> s_Poligon( 1, 20 );
bool s_ZLP, s_isZLPSol;
int s_Solution;

/*

TExpr* StrToExprEditW(const QByteArray& EditStr) 
{

PNode eq;
MathExpr ex;
eq = AnyExpr( FullPreProcessor( EditStr,"x"));
ex=OutPut( eq);
FreeTree(eq);
Result= ex.Clone();
};

void InitFuncStr(const QByteArray&& Func1, const QByteArray&& Func2) 
{

PNode eq;
MathExpr ex,syst;
TExMemb f;
MathExpr op1,op2;
int n;
array[0..10] of char RelSign;
try{
Func2= "";
eq = AnyExpr( FullPreProcessor( Func1,"x"));
Func1= "";
ex=OutPut( eq); 
 if( ! ex.Syst_( syst) ) 
syst=ex;
 if( ! syst.List2ex( f)  ) { 
ex= new TL2exp; 
(ex as TL2exp).Addexp( syst);
syst=ex;
};
n=0;
f=( syst as TL2exp).First;
while( f != nullptr ) { 
inc(n);
 if( ! f->m_Memb.Binar_( RelSign[n],op1,op2) )
throw  ErrParser( "Syntax error!", peSyntacs);
 if( ! (RelSign[n] =='=') )
throw  ErrParser( "Syntax error!", peSyntacs);
case n of {
1: Func1= f->m_Memb.WriteE;
2: Func2= f->m_Memb.WriteE;
};
f=f->m_pNext
};
FreeTree(eq);
Except

};
};
*/
bool CalcSysInEqXY( const QByteArray& UserTask )
  {
  if( UserTask.isEmpty() ) return false;
  bool Result = false;
  int OldDegPoly = s_DegPoly;
  try
    {
    Parser P;
    PNode eq = P.AnyExpr( P.FullPreProcessor( UserTask, "x" ) );
    if( !IsFuncEqu( eq ) )
      {
      MathExpr ex = P.OutPut( eq );
      TSolutionChain::sm_SolutionChain.AddExpr( ex.Clone() );
      TSolutionChain::sm_SolutionChain.AddExpr( SolSysInEq( ex ) );
      Result = true;
      if( s_ZLP )
        {
        if( s_isZLPSol )
          {
          Lexp Sol = new TLexp;
          Sol.Addexp( new TBinar( '=', new TVariable( false, "x" ), Constant( s_Poligon[s_Solution].X ) ) );
          Sol.Addexp( new TBinar( '=', new TVariable( false, "y" ), Constant( s_Poligon[s_Solution].Y ) ) );
          TSolutionChain::sm_SolutionChain.AddExpr( Sol );
          if( s_PutAnswer ) s_Answer = Sol;
          }
        else
          TSolutionChain::sm_SolutionChain.AddExpr( new TCommStr( X_Str( "MNoSolution", "No Solutions" ) ) );
        }
      }
    else
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotSuitableInEq", "Wrong kind of inequality!" ) );
    }
  catch( ErrParser E )
    {
    if( E.ErrStatus() == peNoSolvType )
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotSuitableInEq", "Wrong kind of inequality!" ) );
    else
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  s_DegPoly = OldDegPoly;
  return Result;
  }

MathExpr SolSysInEq( MathExpr ex )
  {
  double a[11], b[11], c[11], PosString[11], x1[21], y1[21], x2[21], y2[21];
  uchar RelSign[11];

  struct Point
    {
    double X, Y;
    int i1, i2;
    bool Belongs;
    bool SimpleLine;
    Point(){}
    Point( double x, double y, int I1, int I2, bool SL ) : X( x ), Y( y ), i1( I1 ), i2( I2 ), Belongs( false ), SimpleLine( SL ) {}
    };

  PascArray<Point>  Points( 1, 20 );

  struct NamedPoint
    {
    double x, y;
    int i1, i2;
    QByteArray Name;
    NamedPoint() {}
    };

  PascArray<NamedPoint>  NamedPoints( 1, 20 );
  int NamedPointCount, PCount, n, PoligonCount;
  bool BoundsDefined, Closed;
  double MaxX, MinX, MaxY, MinY, Scale, SizeCoeff, colorY, colorX;
  Lexp BoundaryPoints;
  QByteArray ExstrType;

  auto CalcBounds = [&]()
    {
    double Shift;
    auto CalcShift = [&]( double x )
      {
      x = abs( x );
      if( x < 1 ) return;
      int S = 1;
      for( int i = 1; i <= floor( log10( x ) ); i++, S *= 10 );
      if( S > Shift ) Shift = S;
      };

    if( !BoundsDefined )
      {
      MaxX = 1;
      MinX = -1;
      MaxY = 1;
      MinY = -1;
      double x, y;
      if( PCount == 0 )
        for( int i = 1; i <= n; i++ )
          {
          if( abs( a[i] ) > 0.0001 )
            x = -c[i] / a[i];
          else
            x = 1;
          if( abs( b[i] ) > 0.0001 )
            {
            if( abs( c[i] ) > 1 )
              y = -c[i] / b[i];
            else
              y = -a[i] / b[i];
            }
          else
            y = 1;
          if( x - 1 < MinX )
            MinX = x - 1;
          else
            if( x + 1 > MaxX )
              MaxX = x + 1;
          if( y - 1 < MinY )
            MinY = y - 1;
          else
            if( y + 1 > MaxY )
              MaxY = y + 1;
          }
      Shift = 1;
      for( int i = 1; i <= PCount; i++ )
        {
        CalcShift( Points[i].X );
        if( Points[i].X - Shift < MinX )
          {
          MinX = Points[i].X - Shift;
          MaxX = max( MaxX, Shift );
          }
        else
          {
          if( Points[i].X + Shift > MaxX )
            MaxX = Points[i].X + Shift;
          MinX = min( MinX, -Shift );
          }
        CalcShift( Points[i].Y );
        if( Points[i].Y - Shift < MinY )
          {
          MinY = Points[i].Y - Shift;
          MaxY = max( MaxY, Shift );
          }
        else
          {
          if( Points[i].Y + Shift > MaxY )
            MaxY = Points[i].Y + Shift;
          MinY = min( MinY, -Shift );
          }
        }
      }
    MinX = MinX * Scale;
    MinY = MinY * Scale;
    MaxX = MaxX * Scale;
    MaxY = MaxY * Scale;
    BoundaryPoints = new TLexp;
    BoundaryPoints.Addexp( Constant( SizeCoeff ) );
    BoundaryPoints.Addexp( Constant( MinX ) );
    BoundaryPoints.Addexp( Constant( MinY ) );
    BoundaryPoints.Addexp( Constant( MaxX ) );
    BoundaryPoints.Addexp( Constant( MaxY ) );
    int i = 1;
    while( i <= PCount )
      if( Points[i].SimpleLine )
        {
        for( int k = i; k < PCount; k++ )
          Points[k] = Points[k + 1];
        PCount = PCount - 1;
        }
      else
        i = i + 1;
    };

  auto CalcBoundaryPoint = [&]( int i )
    {
    int k;
    double x[3], y[3];
    if( a[i] == 0 )
      {
      x[1] = MinX; x[2] = MaxX; y[1] = -c[i] / b[i]; y[2] = y[1];
      }
    else
      if( b[i] == 0 )
        {
        x[1] = -c[i] / a[i]; x[2] = x[1]; y[1] = MinY; y[2] = MaxY;
        }
      else 
        {
        k = 1;
        x[k] = MinX;
        y[k] = -( c[i] + a[i] * x[k] ) / b[i];
        if( ( y[k] > MinY - 0.0001 ) && ( y[k] < MaxY + 0.0001 ) ) k++;
        x[k] = MaxX;
        y[k] = -( c[i] + a[i] * x[k] ) / b[i];
        if( ( y[k] > MinY - 0.0001 ) && ( y[k] < MaxY + 0.0001 ) ) k++;
        if( k > 2 ) goto breakElse;
        if( ( k == 1 ) || ( abs( y[k - 1] - MinY ) > 0.0001 ) )
          {
          y[k] = MinY;
          x[k] = -( c[i] + b[i] * y[k] ) / a[i];
          if( ( x[k] > MinX - 0.0001 ) && ( x[k] < MaxX + 0.0001 ) ) k++;
          if( k > 2 ) goto breakElse;
          };
        y[k] = MaxY;
        x[k] = -( c[i] + b[i] * y[k] ) / a[i];
        }
  breakElse: 
    if( i != 0 )
        {
        BoundaryPoints.Addexp( Constant( x[1] ) );
        BoundaryPoints.Addexp( Constant( y[1] ) );
        BoundaryPoints.Addexp( Constant( x[2] ) );
        BoundaryPoints.Addexp( Constant( y[2] ) );

        switch( RelSign[i] )
          {
          case '>':
          case msMaxequal: 
            BoundaryPoints.Addexp( Constant( a[i] ) );
            BoundaryPoints.Addexp( Constant(  b[i] ) );
            break;
          case '<':
          case msMinequal: 
            BoundaryPoints.Addexp( Constant( -a[i] ) );
            BoundaryPoints.Addexp( Constant( -b[i] ) );
            break;
          case '=': 
            BoundaryPoints.Addexp( Constant( 0 ) );
            BoundaryPoints.Addexp( Constant( 0 ) );
          }
        }
      x1[i] = x[1];
      y1[i] = y[1];
      x2[i] = x[2];
      y2[i] = y[2];
    };

  auto CalcBoundaryPoints = [&]()
    {
    BoundaryPoints.Addexp( Constant( n ) );
    for( int i = 1; i <= n; i++ )
      {
      BoundaryPoints.Addexp( Constant( PosString[i] ) );
      CalcBoundaryPoint( i );
      if( RelSign[i] == msMaxequal || RelSign[i] == msMinequal )
        BoundaryPoints.Addexp( Constant( 1 ) );
      else
      BoundaryPoints.Addexp( Constant( 0 ) );
      }
    };

  auto CalcCrosspoints = [&]()
    {
    PCount = 0;
    for( int i = 1; i < n; i++ )
      for( int j = i + 1; j <= n; j++ )
        {
        double d = a[i] * b[j] - a[j] * b[i];
        if( abs( d ) < 0.00001 ) continue;
        Points[++PCount] = Point( ( c[j] * b[i] - c[i] * b[j] ) / d, ( a[j] * c[i] - a[i] * c[j] ) / d, i, j,
          RelSign[i] == '=' || RelSign[j] == '=' );
        }
    };

  auto CalcPolygonPoints = [&] ()
    {
    int BelongsCount;
    int LineCount[21];
    uchar RSign[11];

    auto IsBelong = [&] ( double x, double y, int j )
      {
      double d = a[j] * x + b[j] * y + c[j];
      switch( RelSign[j] )
        {
        case '<':
        case msMinequal:
          return d < 0.000001;
        case '>':
        case msMaxequal:
          return d > -0.000001;
        case '=': return true;
        }
      return false;
      };

    for( int i = 1; i <= PCount; i++ )
      {
      Point &P = Points[i];
      P.Belongs = true;
      for( int j = 1; j <= n; j++ )
        if( j != P.i1 && j != P.i2 )
          P.Belongs = P.Belongs && IsBelong( P.X, P.Y, j );
      }

    for( int i = 1; i <= n; i++ ) LineCount[i] = 0;
    for( int i = 1; i <= PCount; i++ )
      if( Points[i].Belongs )
        {
        LineCount[Points[i].i1]++;
        LineCount[Points[i].i2]++;
        }

    for( int j = 1; j <= n; j++ )
      for( int i = 1; i < PCount; i++ )
        for( int k = i + 1; k <= PCount; k++ )
          if( ( Points[i].i1 == j || Points[i].i2 == j ) &&
            ( Points[k].i1 == j || Points[k].i2 == j ) &&
            abs( Points[i].X - Points[k].X ) + abs( Points[i].Y - Points[k].Y ) < 0.00001 && LineCount[j] == 2 )
            {
            if( i > k )
              {
              --PCount;
              for( int i1 = i; i1 <= PCount; i1++ )
                Points[i1] = Points[i1 + 1];
              --PCount;
              for( int i1 = k; i1 <= PCount; i1++ )
                Points[i1] = Points[i1 + 1];
              }
            else
              {
              --PCount;
              for( int i1 = k; i1 <= PCount; i1++ )
                Points[i1] = Points[i1 + 1];
              --PCount;
              for( int i1 = i; i1 <= PCount; i1++ )
                Points[i1] = Points[i1 + 1];
              }
            }

    BelongsCount = 0;
    for( int i = 1; i <= n; i++ ) LineCount[i] = 0;
    for( int i = 1; i <= PCount; i++ )
      if( Points[i].Belongs )
        {
        BelongsCount++;
        LineCount[Points[i].i1]++;
        LineCount[Points[i].i2]++;
        }

    PoligonCount = 0;
    Closed = false;

    if( BelongsCount == 0 )
      {
      double cmax = -1e10;
      double cmin = 1e10;
      int imax = 0;
      int imin = 0;
      for( int i = 1; i <= n; i++ )
        {
        double cc;
        if( a[1] * a[i] < 0 || b[1] * b[i] < 0 )
          {
          cc = -c[i];
          switch( RelSign[i] )
            {
            case '<':
            case msMinequal:
              RSign[i] = '>';
              break;
            case '=':
              RSign[i] = '=';
              break;
            default: RSign[i] = '<';
            }
          }
        else
          {
          cc = c[i];
          RSign[i] = RelSign[i];
          }
        cc /= sqrt( a[i] * a[i] + b[i] * b[i] );
        switch( RSign[i] )
          {
          case '<':
          case msMinequal:
            if( cc > cmax ) { cmax = cc; imax = i; }
            break;
          case '>':
          case msMaxequal:
            if( cc < cmin ) { cmin = cc; imin = i; };
          }
        }
      if( ( imax > 0 ) ^ ( imin > 0 ) )
        {
        int k = imax > 0 ? k = imax : k = imin;
        BoundaryPoints.Addexp( Constant( 2 ) );
        BoundaryPoints.Addexp( Constant( x1[k] ) );
        BoundaryPoints.Addexp( Constant( y1[k] ) );
        BoundaryPoints.Addexp( Constant( x2[k] ) );
        BoundaryPoints.Addexp( Constant( y2[k] ) );
        if( x1[k] != x2[k] )
          {
          BoundaryPoints.Addexp( Constant( ( x1[k] + x2[k] ) / 2 ) );
          colorY = ( y1[k] + y2[k] ) / 2;
          switch( RelSign[k] )
            {
            case '<':
            case msMinequal:
              if( b[k] > 0 )
                colorY = ( colorY + MinY ) / 2;
              else
                colorY = ( colorY + MaxY ) / 2;
              break;
            case '>':
            case msMaxequal:
              if( b[k] > 0 )
                colorY = ( colorY + MaxY ) / 2;
              else
                colorY = ( colorY + MinY ) / 2;
            }
          BoundaryPoints.Addexp( Constant( colorY ) );
          }
        else
          {
          colorX = ( x1[k] + x2[k] ) / 2;
          switch( RelSign[k] )
            {
            case '<':
            case msMinequal:
              if( a[k] > 0 )
                colorX = ( colorX + MinX ) / 2;
              else
                colorX = ( colorX + MaxX ) / 2;
              break;
            case '>':
            case msMaxequal:
              if( a[k] > 0 )
                colorX = ( colorX + MaxX ) / 2;
              else
                colorX = ( colorX + MinX ) / 2;
            }
          BoundaryPoints.Addexp( Constant( colorX ) );
          BoundaryPoints.Addexp( Constant( ( y1[k] + y2[k] ) / 2 ) );
          }
        PoligonCount = 1;
        s_Poligon[1].X = ( x1[k] + x2[k] ) / 2;
        s_Poligon[1].Y = ( y1[k] + y2[k] ) / 2;
        }
      else
        if( cmax > cmin || imax == 0 && imin == 0 )
          BoundaryPoints.Addexp( Constant( 0 ) );
        else
          {
          BoundaryPoints.Addexp( Constant( -4 ) );
          BoundaryPoints.Addexp( Constant( x1[imax] ) );
          BoundaryPoints.Addexp( Constant( y1[imax] ) );
          BoundaryPoints.Addexp( Constant( x2[imax] ) );
          BoundaryPoints.Addexp( Constant( y2[imax] ) );
          BoundaryPoints.Addexp( Constant( x1[imin] ) );
          BoundaryPoints.Addexp( Constant( y1[imin] ) );
          BoundaryPoints.Addexp( Constant( x2[imin] ) );
          BoundaryPoints.Addexp( Constant( y2[imin] ) );
          PoligonCount = 2;
          s_Poligon[1].X = ( x1[imax] + x2[imax] ) / 2;
          s_Poligon[1].Y = ( y1[imax] + y2[imax] ) / 2;
          s_Poligon[2].X = ( x1[imin] + x2[imin] ) / 2;
          s_Poligon[2].Y = ( y1[imin] + y2[imin] ) / 2;
          }
      return;
      }

    int i = 1;
    for( ; i <= n && LineCount[i] != 1; i++ );
    int CurentLine;
    if( i <= n )
      {
      Closed = false;
      CurentLine = i;
      BoundaryPoints.Addexp( Constant( BelongsCount + 2 ) );

      bool bb = true;
      PoligonCount++;
      for( int j = 1; j <= n; j++ )
        if( i != j ) bb = bb && IsBelong( x1[i], y1[i], j );
      if( bb )
        {
        BoundaryPoints.Addexp( Constant( x1[i] ) );
        BoundaryPoints.Addexp( Constant( y1[i] ) );
        s_Poligon[PoligonCount].X = x1[i];
        s_Poligon[PoligonCount].Y = y1[i];
        }
      else
        {
        BoundaryPoints.Addexp( Constant( x2[i] ) );
        BoundaryPoints.Addexp( Constant( y2[i] ) );
        s_Poligon[PoligonCount].X = x2[i];
        s_Poligon[PoligonCount].Y = y2[i];
        }

      i = 1;
      for( ; i <= PCount && ( !Points[i].Belongs || Points[i].i1 != CurentLine && Points[i].i2 != CurentLine ); i++ );
      BoundaryPoints.Addexp( Constant( Points[i].X ) );
      BoundaryPoints.Addexp( Constant( Points[i].Y ) );
      PoligonCount++;
      s_Poligon[PoligonCount].X = Points[i].X;
      s_Poligon[PoligonCount].Y = Points[i].Y;

      if( Points[i].i1 == CurentLine )
        CurentLine = Points[i].i2;
      else
        CurentLine = Points[i].i1;

      for( int k = 1; k < BelongsCount; k++ )
        {
        int j = 1;
        for( ; !( i != j && Points[j].Belongs && ( Points[j].i1 == CurentLine || Points[j].i2 == CurentLine ) ); j++ );
        Points[j].Belongs = false;
        BoundaryPoints.Addexp( Constant( Points[j].X ) );
        BoundaryPoints.Addexp( Constant( Points[j].Y ) );
        s_Poligon[++PoligonCount].X = Points[j].X;
        s_Poligon[PoligonCount].Y = Points[j].Y;
        if( Points[j].i1 == CurentLine )
          CurentLine = Points[j].i2;
        else
          CurentLine = Points[j].i1;
        i = j;
        }

      bb = true;
      PoligonCount++;
      for( int j = 1; j <= n; j++ )
        if( CurentLine != j )
          bb = bb && IsBelong( x1[CurentLine], y1[CurentLine], j );
      if( bb )
        {
        BoundaryPoints.Addexp( Constant( x1[CurentLine] ) );
        BoundaryPoints.Addexp( Constant( y1[CurentLine] ) );
        s_Poligon[PoligonCount].X = x1[CurentLine];
        s_Poligon[PoligonCount].Y = y1[CurentLine];
        }
      else
        {
        BoundaryPoints.Addexp( Constant( x2[CurentLine] ) );
        BoundaryPoints.Addexp( Constant( y2[CurentLine] ) );
        s_Poligon[PoligonCount].X = x2[CurentLine];
        s_Poligon[PoligonCount].Y = y2[CurentLine];
        }
      }
    else
      {
      Closed = true;
      BoundaryPoints.Addexp( Constant( BelongsCount + 1 ) );
      i = 1;
      for( ; i <= PCount && !Points[i].Belongs; i++ );
      int BegI = i;
      CurentLine = Points[i].i1;
      BoundaryPoints.Addexp( Constant( Points[i].X ) );
      BoundaryPoints.Addexp( Constant( Points[i].Y ) );
      s_Poligon[++PoligonCount].X = Points[i].X;
      s_Poligon[PoligonCount].Y = Points[i].Y;
      int k = 1;
      do
        {
        int j = 1;
        for( ; !( i != j && Points[j].Belongs && ( Points[j].i1 == CurentLine || Points[j].i2 == CurentLine ) ); j++ );
        Points[j].Belongs = false;
        BoundaryPoints.Addexp( Constant( Points[j].X ) );
        BoundaryPoints.Addexp( Constant( Points[j].Y ) );
        s_Poligon[++PoligonCount].X = Points[j].X;
        s_Poligon[PoligonCount].Y = Points[j].Y;
        if( Points[j].i1 == CurentLine )
          CurentLine = Points[j].i2;
        else
          CurentLine = Points[j].i1;
        i = j;
        } while( ++k < BelongsCount );
        BoundaryPoints.Addexp( Constant( Points[BegI].X ) );
        BoundaryPoints.Addexp( Constant( Points[BegI].Y ) );
      }
    };

  auto ZLPSolution = [&]()
    {
    if( PoligonCount == 0 ) return false;
    double m = a[0] * s_Poligon[1].X + b[0] * s_Poligon[1].Y + c[0];
    s_Solution = 1;
    for( int i = 2; i <= PoligonCount; i++ )
      {
      double r = a[0] * s_Poligon[i].X + b[0] * s_Poligon[i].Y + c[0];
      if( ExstrType == "max" )
        {
        if( r > m ) { m = r; s_Solution = i; }
        }
      else
        if( r < m ) { m = r; s_Solution = i; }
      }
    if( Closed || s_Solution != 1 && s_Solution != PoligonCount )
      return true;
    else
      if( a[0] * b[s_Solution] == b[0] * a[s_Solution] )
        return true;
    return false;
    };

  int Digits;
  auto ToStr = [&]( double val )
    {
    if( abs( val - round( val ) ) > s_Precision )
      return QByteArray::number( val, 'f', Digits );
    return QByteArray::number( val );
    };

  int OldDegPoly = s_DegPoly;
  bool NoInscriptions = false;
  BoundsDefined = false;
  s_DegPoly = 3;
  Scale = 1.0;
  MathExpr syst;
  if( !ex.Syst_( syst ) )
    syst = ex;
  PExMemb f;
  if( !syst.List2ex( f ) )
    {
    ex = new TL2exp;
    CastPtr( TL2exp, ex)->Addexp( syst );
    syst = ex;
    } 
  bool WasModify = false;
  for( f = CastPtr( TL2exp, syst )->First(); !f.isNull(); f = f->m_pNext )
    {
    MathExpr ex1 = RemDenominator( f->m_Memb );
    if( !ex1.Eq( f->m_Memb ) )
      {
      WasModify = true;
      f->m_Memb = ex1.Reduce();
      }
    }
  if( WasModify )
    TSolutionChain::sm_SolutionChain.AddExpr( syst );
  n = 0;
  int nn = 0;
  s_ZLP = false;
  SizeCoeff = 1.0;
  NamedPointCount = 0;
  for( f = CastPtr( TL2exp, syst )->First(); !f.isNull(); f = f->m_pNext )
    {
    if( ++n % 2 != 0 )
      PosString[n] = 0.55;
    else
      PosString[n] = 0.05;
    nn++;
    MathExpr op1, op2;
    if( f->m_Memb.Binar_( RelSign[n], op1, op2 ) )
      {
      if( nn == 1 )
        s_ZLP = RelSign[1] == '=' && op2.Variab( ExstrType ) && ( ExstrType == "max" || ExstrType == "min" );
      if( !s_ZLP || nn > 1 )
        {
        const char SetBinaries[] = { '<', '>', (char) msMaxequal, (char) msMinequal, 0 };
        if( !( In( RelSign[n], SetBinaries ) ) )
          {
          if( RelSign[n] != '=' )
            throw  ErrParser( "", peNoSolvType );
          QByteArray st;
          if( !op1.Variab( st ) || st != "m" && st != "label" && st != "pos" && st != "ms" )
            {
            if( st == "bounds" )
              {
              op2 = op2.Perform().Reduce();
              if( !( IsConstType( TLexp, op2 ) ) )
                throw  ErrParser( "", peNoSolvType );
              PExMemb memBound = CastPtr( TLexp, op2 )->First();
              if( memBound == nullptr )
                throw  ErrParser( "", peNoSolvType );
              memBound->m_Memb.Constan( MinX );
              memBound = memBound->m_pNext;
              memBound->m_Memb.Constan( MaxX );
              memBound = memBound->m_pNext;
              if( memBound.isNull() )
                throw  ErrParser( "", peNoSolvType );
              memBound->m_Memb.Constan( MinY );
              memBound = memBound->m_pNext;
              if( memBound == nullptr )
                throw  ErrParser( "", peNoSolvType );
              memBound->m_Memb.Constan( MaxY );
              BoundsDefined = true;
              n = n - 1;
              continue;
              }
            }
          else
            {
            if( st == "m" )
              {
              if( !op2.Constan( SizeCoeff ) )
                throw  ErrParser( "", peNoSolvType );
              }
            else
              if( st == "pos" )
                {
                if( ( n == 1 ) || !op2.Constan( PosString[n - 1] ) )
                  throw  ErrParser( "", peNoSolvType );
                }
              else
                if( st == "ms" )
                  {
                  if( !op2.Constan( Scale ) )
                    throw  ErrParser( "", peNoSolvType );
                  }
                else
                  {
                  if( !op2.Variab( st ) || ( st != "no" ) )
                    throw  ErrParser( "", peNoSolvType );
                  NoInscriptions = true;
                  }
            n = n - 1;
            continue;
            }
          }
        ex = op1 - op2;
        }
      else
        {
        ex = op1;
        n = 0;
        }

      TExprs p;
      ex.ReductionPoly( p, "x" );
      double v;
      for( int i = 2; i <= s_DegPoly; i++ )
        if( !( p[i].Constan( v ) && abs( v ) < 0.0000001 ) )
          throw  ErrParser( "", peNoSolvType );
      int nom, den;
      if( !p[1].Constan( a[n] ) )
        if( p[1].SimpleFrac_( nom, den ) )
          a[n] = ( double ) nom / den;
        else
          throw  ErrParser( "", peNoSolvType );
      p.clear();
      ex.ReductionPoly( p, "y" );
      for( int i = 2; i <= s_DegPoly; i++ )
        if( !( p[i].Constan( v ) && abs( v ) < 0.0000001 ) )
          throw  ErrParser( "", peNoSolvType );
      if( !p[1].Constan( b[n] ) )
        if( p[1].SimpleFrac_( nom, den ) )
          b[n] = ( double ) nom / den;
        else
          throw  ErrParser( "", peNoSolvType );
      p.clear();
      if( a[n] == 0 && b[n] == 0 )
        throw  ErrParser( "", peNoSolvType );

      MathExpr ex1 = ex.Substitute( "y", Variable( "x" ) );
      ex1.ReductionPoly( p, "x" );
      if( !p[0].Constan( c[n] ) )
        if( p[0].SimpleFrac_( nom, den ) )
          c[n] = ( double ) nom / den;
        else
          throw  ErrParser( "", peNoSolvType );
      }
    else
      {
      PExMemb memPoint;
      if( !f->m_Memb.Listex( memPoint ) )
        throw ErrParser( "Syntax error!", peSyntacs );
      NamedPointCount = NamedPointCount + 1;
      if( !memPoint->m_Memb.Cons_int( NamedPoints[NamedPointCount].i1 ) )
        throw  ErrParser( "Syntax error!", peSyntacs );
      memPoint = memPoint->m_pNext;
      if( ( memPoint == nullptr ) || !memPoint->m_Memb.Cons_int( NamedPoints[NamedPointCount].i2 ) )
        throw  ErrParser( "Syntax error!", peSyntacs );
      memPoint = memPoint->m_pNext;
      if( ( memPoint == nullptr ) || !memPoint->m_Memb.Variab( NamedPoints[NamedPointCount].Name ) )
        throw  ErrParser( "Syntax error!", peSyntacs );
      NamedPoints[NamedPointCount].x = 0;
      NamedPoints[NamedPointCount].y = 0;
      n--;
      nn--;
      }
    }

  CalcCrosspoints();

  CalcBounds();

  CalcBoundaryPoints();

  CalcPolygonPoints();

  if( PoligonCount > 1 )
    {
    colorX = 0;
    colorY = 0;
    for( int i = 1; i <= PoligonCount; i++ )
      {
      colorX = colorX + s_Poligon[i].X;
      colorY = colorY + s_Poligon[i].Y;
      }
    BoundaryPoints.Addexp( Constant( colorX / PoligonCount ) );
    BoundaryPoints.Addexp( Constant( colorY / PoligonCount ) );
    }

  Digits = -round( log10( s_Precision ) );
  for( int i = 1; i <= n; i++ )
    {
    if( NoInscriptions )
      {
      BoundaryPoints.Addexp( new TStr( "" ) );
      continue;
      }
    QByteArray Inscriptions, st;
    if( a[i] != 0 )
      if( a[i] == 1 ) Inscriptions = "x";
      else
        if( a[i] == -1 ) Inscriptions = "-x";
        else
          Inscriptions = ToStr( a[i] ) + "x";
    else
    Inscriptions = "";
    if( b[i] != 0 )
      {
      if( b[i] == 1 ) st = "y";
      else
        if( b[i] == -1 ) st = "-y";
      else st = ToStr( b[i] ) + "y";
      if( ( Inscriptions != "" ) && ( b[i] > 0 ) )
        Inscriptions += "+" + st;
      else
        Inscriptions += st;
      }
    Inscriptions += '=' + ToStr( -c[i] );
    BoundaryPoints.Addexp( new TStr( Inscriptions ) );
    }

  BoundaryPoints.Addexp( Constant( NamedPointCount ) );
  for( int i = 1; i <= NamedPointCount; i++ )
    {
    for( int k = 1; k <= PCount; k++ )
      if( NamedPoints[i].i1 == Points[k].i1 && NamedPoints[i].i2 == Points[k].i2 || 
         NamedPoints[i].i1 == Points[k].i2 && NamedPoints[i].i2 == Points[k].i1 )
        {
        NamedPoints[i].x = Points[k].X;
        NamedPoints[i].y = Points[k].Y;
        };
    BoundaryPoints.Addexp( Constant( NamedPoints[i].x ) );
    BoundaryPoints.Addexp( Constant( NamedPoints[i].y ) );
    BoundaryPoints.Addexp( Variable( NamedPoints[i].Name ) );
    }

  s_isZLPSol = false;
  if( s_ZLP )
    {
    s_isZLPSol = ZLPSolution();
    if( s_isZLPSol )
      {
      c[0] = -( a[0] * s_Poligon[s_Solution].X + b[0] * s_Poligon[s_Solution].Y );
      CalcBoundaryPoint( 0 );
      BoundaryPoints.Addexp( Constant( x1[0] ) );
      BoundaryPoints.Addexp( Constant( y1[0] ) );
      BoundaryPoints.Addexp( Constant( x2[0] ) );
      BoundaryPoints.Addexp( Constant( y2[0] ) );
      if( ExstrType == "max" )
        {
        BoundaryPoints.Addexp( Constant( a[0] ) );
        BoundaryPoints.Addexp( Constant( b[0] ) );
        }
      else {
        BoundaryPoints.Addexp( Constant( -a[0] ) );
        BoundaryPoints.Addexp( Constant( -b[0] ) );
        };
      BoundaryPoints.Addexp( Constant( s_Poligon[s_Solution].X ) );
      BoundaryPoints.Addexp( Constant( s_Poligon[s_Solution].Y ) );
      }
    }
  s_DegPoly = OldDegPoly;
  return new TPolygon( BoundaryPoints );
  }
