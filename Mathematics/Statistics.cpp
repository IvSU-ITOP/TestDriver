#include "Statistics.h"
#include "Parser.h"
#include "SolChain.h"
#include "ExpObBin.h"

uLong  TRandom::m_A, TRandom::m_C;
double TRandom::m_Scale;
uLong TRandom::m_Next;
TRandom s_Random;

MathExpr GaussProbability( const MathExpr& Ex )
  { 
  double Value;
  if( !Ex.Constan( Value ) && !Ex.Reduce().Constan( Value ) )
    throw( ErrParser( "Invalid Argument!", peSyntacs ) );
  return Constant( erf( Value ) );
  }

void Randomize() 
  { 
  s_Random.Randomize();
  }

int Random( int Max ) 
  {
  return s_Random.GetNumber(Max);
  }

void TRandom::Initiate()
  {
  if( m_A > 0 ) return;
  uLong M = 1, M2;
  do
    {
    M2 = M;
    M *= 2;
    } while( M > M2 );
    double HalfM = M2;
    uLong Tmp = floor( HalfM * atan( 1.0 ) / 8.0 );
    m_A = 8 * Tmp + 5;
    Tmp = floor( HalfM * ( 0.5 - sqrt( 3.0 ) / 6.0 ) );
    m_C = 2 * Tmp + 1;
    m_Scale = 0.5 / HalfM;
  }

TRandom::TRandom() { Initiate(); }

void Expectation( const MathExpr& ex, const QByteArray& name, int& n, double& m)
  {
  MathExpr s,ex1;
  PExMemb f,f1;
  double v;
  bool DblList = ex.List2ex(f1);
  if( !DblList && !ex.Listex(f1) )
    throw  X_Str( "MNumSeqData", "The data should be a numerical sequence!" );
  n=0;
  m=0;
label:
  if( !f1->m_Memb.Listex(f) )
    f = f1;
  while( !f.isNull() )
    {
    if( !f->m_Memb.Newline() )
      {
      if( !f->m_Memb.Constan(v) )
        throw  X_Str( "MNumSeqData", "The data should be a numerical sequence!" );
      m += v;
      n++;
      if( n == 1 )
        s = f->m_Memb;
      else
        if( n <= SecLength )
          s += f->m_Memb;
      if( f == f1 && DblList ) break;
      }
    f = f->m_pNext;
    }
  if( DblList )
    {
    f1 = f1->m_pNext;
    if( !f1.isNull() ) goto label;
    }
  m /= n;
  if( n <= SecLength )
    ex1= new TBinar('=', new TStr("M(" + name + ")"), s / Constant( n ) );
  else
    ex1 = new TStr("M(" + name + ")");
  TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', ex1, Constant( m ) ),
    X_Str( "MExpectation", "The Mathematical Expectation" ) );
  }

void Dispersion( const MathExpr& ex, const QByteArray& name, double m, double& d)
  {
  MathExpr s,ex1;
  PExMemb f,f1;
  double v;
  int n;
  bool DblList;
  DblList = ex.List2ex(f1);
  if( !DblList && !ex.Listex(f1) )
    throw  X_Str( "MNumSeqData", "The data should be a numerical sequence!" );
  n=0;
  d=0;
label:
  if( !f1->m_Memb.Listex(f) )
    f = f1;
  while( !f.isNull() )
    {
    if( !f->m_Memb.Newline() )
      {
      if( !f->m_Memb.Constan(v) )
        throw  X_Str( "MNumSeqData", "The data should be a numerical sequence!" );
      d += (v - m)*(v - m);
      n++;
      if( n <= SecLength )
        {
        ex1 = ( f->m_Memb - Constant( m ) ) ^ Constant( 2 );
        if( n == 1 )
          s = ex1;
        else
          s = s + ex1;
        }
      if( f == f1 && DblList ) break;
      }
    f = f->m_pNext;
    }
  if( DblList )
    {
    f1 = f1->m_pNext;
    if( !f1.isNull() ) goto label;
    }
  if( n <= SecLength )
    ex1 = new TBinar('=', new TStr("D(" + name + ")"), s / Constant( n ));
  else
    ex1 = new TStr("D(" + name + ")");
  TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', ex1, Constant( d / n ) ),
    X_Str( "MDispersion", "The Dispersion!" ) );
  }

void CalcStatistics(MathExpr& ex)
  {
  MathExpr ex1;
  double m, d;
  int n;
//  memory_switch = SWcalculator;
  try
   {
   TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MInputData", "Input data!" ) );
   ex1 = ex.Reduce();
   if( !ex1.Eq(ex) )
     {
     ex = ex1;
     TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str("MSimplData", "The simplified data!" ) );
     }
   Expectation(ex, "X", n, m);
   Dispersion(ex, "X", m, d);
   }
 catch (const QString& Error)
   {
   TSolutionChain::sm_SolutionChain.AddComment( Error );
   }
 }

void CalcCorrelation(MathExpr& ex)
  {
  MathExpr ex1,r,s;
  PExMemb f,q;
  MathExpr x,y;
  int nx,ny,k,n;
  double mx,my,dx,dy,vx,vy,d;
  MathExpr xi,yi,iv,nv,Formula;
  MathExpr Nom,Den1,Den2;

  auto NDispersion = [] (const MathExpr& ex, const MathExpr& Name, double m, double& d)
    {
    MathExpr s,ex1;
    PExMemb f;
    double v;
    int n;
    if( ex.Listex(f) )
      {
      n=0;
      d=0;
      while( !f.isNull() )
        {
        if( !f->m_Memb.Newline() )
          {
          if( !f->m_Memb.Constan(v) )
            throw  X_Str( "MNumSeqData", "The data should be a numerical sequence!" );
          d += (v - m) * (v - m);
         n++;
         if( n <= SecLength )
           {
           ex1 = ( f->m_Memb - Constant( m )) ^ Constant(2);
           if( n == 1 )
             s = ex1;
           else
             s += ex1;
           }
         }
       f = f->m_pNext;
       }
     if( n <= SecLength )
       ex1 = new TBinar('=', Name, s);
     else
       ex1 = Name;
    TSolutionChain::sm_SolutionChain.AddExpr(new TBinar('=', ex1, Constant( d ) ));
    }
  else
    throw  X_Str( "MNumSeqData", "The data should be a numerical sequence!" );
  };

  try
    {
    TSolutionChain::sm_SolutionChain.AddExpr(ex, X_Str("MInputData", "Input data!" ));
    ex1 = ex.Reduce();
    if( !ex1.Eq(ex) )
      {
      ex=ex1;
      TSolutionChain::sm_SolutionChain.AddExpr(ex, X_Str("MSimplData", "The simplified data!" ));
      }
    if( !ex.List2ex(f) )
      throw  X_Str( "MTwoSeqData", "The data should be two numerical sequences!" );
    k = 0;
    while( !f.isNull() )
      {
      k++;
      if( k > 2 )
        throw  X_Str( "MTwoSeqData", "The data should be two numerical sequences!" );
      if( k == 1 )
        x = f->m_Memb;
      else
        y = f->m_Memb;
      f = f->m_pNext;
      }
    if( k < 2 )
      throw  X_Str( "MTwoSeqData", "The data should be two numerical sequences!" );
    Expectation(x, "x", nx, mx);
    Expectation(y, "y", ny, my);
    if( nx != ny )
      throw  X_Str( "MTwoSeqLength", "The sequences should have identical length!" );
    f = Lexp(x).First();
    q = Lexp(y).First();
    n = 0;
    d = 0;
    while( !f.isNull() )
      {
      n++;
      if( f->m_Memb.Newline() )
        f = f->m_pNext;
      f->m_Memb.Constan( vx );
      if( q->m_Memb.Newline() )
        q = q->m_pNext;
      q->m_Memb.Constan( vy );
      d += (vx-mx)*(vy-my);
      if( n <= SecLength )
        {
        ex1 = ( f->m_Memb - Constant( mx ) ) * ( q->m_Memb - Constant( my ) );
        if( n == 1 )
          s = ex1;
        else
          s += ex1;
        }
      f = f->m_pNext;
      q = q->m_pNext;
      }
    xi = Variable( "x_i" );
    yi = Variable( "y_i" );
    iv = Variable( "i" );
    nv = Variable( "n" );
    Nom = new TGSumm(false,(( xi - new TStr("M(x)") ) * (yi - new TStr("M(y)") )),
      new TBinar('=', iv, Constant(1)), nv );
    if( n <= SecLength )
      TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', Nom, new TBinar('=', s, Constant( d ))));
    else
      TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', Nom, Constant( d )));
    Den1 = new TGSumm(false, ( xi - new TStr("M(x)")) ^ Constant( 2 ),
      new TBinar('=', iv, Constant( 1 )), nv );
    Den2 = new TGSumm(false, ( yi - new TStr("M(y)")) ^ Constant( 2 ),
      new TBinar('=', iv, Constant( 1 )), nv );
    Formula = Nom / ( Den1.Root( 2 ) * Den2.Root( 2 ) );
    NDispersion(x, Den1, mx, dx);
    NDispersion(y, Den2, my, dy);
    r = Constant( d )/ ( Constant(dx).Root( 2 ) * Constant( dy ).Root( 2 ));
    bool OldFullReduce = TExpr::sm_FullReduce;
    TExpr::sm_FullReduce = true;
    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', new TStr("r"),
      new TBinar('=', Formula, new TBinar('=',r , r.Reduce(true) ))));
    TExpr::sm_FullReduce = OldFullReduce;
    }
  catch (const QString& Message)
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""), Message );
    }
  }


