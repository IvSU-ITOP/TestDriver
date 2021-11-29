#include "LogExp.h"
#include "Parser.h"
#include "SolChain.h"
#include "Algebra.h"
#include "MathTool.h"
#include "Factors.h"
#include <functional>

extern int s_DegPoly;
extern bool s_ShowDiviMessages;

bool s_ExpControl = false;
bool s_FirstCalculation;

bool CheckUnknownInExp( const MathExpr& ex, const QByteArray& VarName )
  {
  MathExpr exLeft, exRight;
  QByteArray Name;
  char cOper;
  if( ex.Unarminus( exLeft ) )
    return CheckUnknownInExp( exLeft, VarName );
  if( ex.Oper_( cOper, exLeft, exRight ) )
    {
    if( cOper == '^' )
      {
      s_ExpControl = true;
      bool Result = CheckUnknownInExp( exRight, VarName );
      s_ExpControl = false;
      return Result;
      }
    return CheckUnknownInExp( exLeft, VarName ) || CheckUnknownInExp( exRight, VarName );
    }
  return s_ExpControl && ex.Variab( Name ) && ( Name == VarName || VarName.isEmpty( ) );
  }

/*

QByteArray FloatStr(double x) 
{
MathExpr  c;
c= Constant( ( x ) );
c.Constan(x);
Result=FloatToStr(x);
};

*/

MathExpr ExprToRatFrac( const MathExpr& ex, MathExpr& Nom, MathExpr& Den, const QByteArray& VarName )
  {
  int FactorCount;
  std::function<void( const MathExpr& )> SearchFactors = [&]( const MathExpr& ex )
    {
    MathExpr exLeft, exRight;
    QByteArray Name;

    std::function<void( const MathExpr& )> FactorAnalyse = [&]( const MathExpr& exp )
      {
      MathExpr exLeft, exRight, exArg;
      char cOper;
      if( exp.Unarminus( exArg ) )
        {
        FactorAnalyse( exArg );
        return;
        }
      if( exp.Power( exLeft, exRight ) )
        {
        if( !exLeft.HasUnknown( VarName ).isEmpty() || !exRight.HasUnknown( VarName ).isEmpty() ) FactorCount = -10000;
        return;
        }
      if( exp.Oper_( cOper, exLeft, exRight ) )
        {
        FactorAnalyse( exLeft );
        FactorAnalyse( exRight );
        return;
        }
      if( exp.Variab( Name ) && Name == VarName ) FactorCount++;
      };

    if( ex.Multp( exLeft, exRight ) )
      {
      SearchFactors( exLeft );
      SearchFactors( exRight );
      return;
      }
    if( ex.HasUnknown( VarName ).isEmpty() ) return;
    FactorAnalyse( ex );
    };

  Parser P;
  PNode eq = P.GetExpression( ex.WriteE() );
  if( IsFuncEqu( eq ) || CheckUnknownInExp( ex, VarName ) )
    {
    Nom = ex.Reduce();
    Den.Clear();
    return Nom;
    }
  Div2( eq );
  OpenBrackets( eq );
  MultDiv( eq );

  TDivis Divis(1);
  FindDivi( eq, Divis );

  int i = 1;
  for( ; i < Divis.count() && !Divis[i].m_DenDepX; i++ );
  if( i == Divis.count() )
    {
    MathExpArray p;
    Nom = ex.ReductionPoly( p, VarName );
    Den.Clear();
    return Nom;
    }

  TDivEx DivEx;
  if( Divis[0].m_Nom != nullptr )
    {
    MathExpArray p;
    DivEx.append( TDivExpr( P.OutPut( Divis[0].m_Nom ).ReductionPoly( p, VarName ).Reduce() ) );
    }
  else
    DivEx.append(Constant( 0 ));
  double d;
  MathExpr CommDen;
  if( Divis.count() != 2 || !( DivEx[0].m_NomEx.Constan( d ) && abs( d ) < 0.0000001 ) )
    {
    MathExpArray p;
    for( i = 1; i < Divis.count(); i++ )
      {
      DivEx.append( TDivExpr( P.OutPut( Divis[i].m_Nom ).ReductionPoly( p, VarName ) ) );
      p.clear();
      if( Divis[i].m_Sign == '-' )
        DivEx[i].m_NomEx = -DivEx[i].m_NomEx;
      Den = P.OutPut( Divis[i].m_Den );
      DivEx[i].m_DenEx = Den.ReductionPoly( p, VarName );
      }
    TExprs AddMult;
    AddMult.append( Constant( 1 ) );
    MathExpr Mult;
    CommDen = ToFactors( DivEx[0].m_DenEx );
    for( i = 2; i < Divis.count(); i++ )
      {
      Mult = ToFactors( DivEx[i].m_DenEx );
      MathExpr CommExp = GetCommon( CommDen, Mult );
      for( int j = 0; j < i; j++ )
        AddMult[j] = ( AddMult[j] * Mult ).Reduce();
      AddMult.append( CommDen.Reduce() );
      CommDen = MultExprs( CommExp, MultExprs( CommDen, Mult ) ).Reduce();
      }
    Mult = ExpandExpr( DivEx[0].m_NomEx * AddMult[0] );
    Nom = Mult;
    for( i = 1; i < Divis.count(); i++ )
      Nom += ExpandExpr( DivEx[i].m_NomEx * AddMult[i] );
    if( !( DivEx[0].m_NomEx.Reduce().Constan( d ) && abs( d ) < 0.0000001 ) )
      Nom += ExpandExpr( DivEx[0].m_NomEx * CommDen );
    }
  else 
    {
    Nom = P.OutPut( Divis[1].m_Nom );
    if( Divis[1].m_Sign == '-' ) Nom = -Nom;
    CommDen = P.OutPut( Divis[1].m_Den );
    }
  FactorCount = 0;
  SearchFactors( Nom );
  MathExpArray p;
  if( FactorCount > 2 )
    Nom.ReductionPoly( p, VarName );
  else
    Nom = Nom.ReductionPoly( p, VarName );
  Den = CommDen;
  return Nom / Den;
  }

Lexp Roots( MathExpr ex, const QByteArray& VarName )
  {
  Lexp Result = new TLexp;
  auto MultRoots = [&]( const MathExpr& ex )
    {
    s_ShowDiviMessages = false;
    TExprs a;
    ex.ReductionPoly( a, VarName );
    double v;
    for( int i = 3; i <= s_DegPoly; i++ )
      if( !( a[i].Constan( v ) && abs( v ) < 0.0000001 ) )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    int nom, den;
    for( int i = 0; i <= 2; i++ )
      if( !( a[i].Constan( v ) || a[i].SimpleFrac_( nom, den ) ) )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    if( a[2].Constan( v ) && abs( v ) < 0.0000001 )
      if( a[1].Constan( v ) && abs( v ) < 0.0000001 )
        return;
      else
        Result.Addexp( ( -a[0] / a[1] ).Reduce() );
    else 
      {
      MathExpr D = ( ( a[1] ^ 2 ) - Constant( 4 ) * a[2] * a[0] ).Reduce();
      if( D.Constan( v ) && v < 0 || ( D.SimpleFrac_( nom, den ) && nom < 0 ) )
        return;
      if( D.Constan( v ) && abs( v ) < 0.0000001 )
        Result.Addexp( ( -a[1] / ( Constant( 2 ) * a[2] ) ).Reduce() );
        else 
          {
          D = D.Root( 2 ).Reduce();
          MathExpr ex1 = Constant( 2 ) * a[2];
          MathExpr ex2 = -a[1];
          MathExpr x1 = ( ( ex2 - D ) / ex1 ).Reduce();
          MathExpr x2 = ( ( ex2 + D ) / ex1 ).Reduce();
          Result.Addexp( x1 );
          Result.Addexp( x2 );
          }
        }
    };

  std::function<void( const MathExpr& )> GetMult = [&]( const MathExpr& ex )
    {
    MathExpr op1, op2;
    if( ex.Multp( op1, op2 ) )
      {
      GetMult( op1 );
      GetMult( op2 );
      }
    else
      MultRoots( ex );
    };

  try
    {
    ex = ex.ReduceToMult();
    s_GlobalInvalid = false;
    GetMult( ex );
    }
  catch( ErrParser )
    {
    MathExpr Temp = new TBinar( '=', ex, Constant( 0 ) );
    if( CheckUnknownInExp( ex, VarName ) )
      {
      bool OldPutAnswer = s_PutAnswer;
      s_PutAnswer = true;
      bool OldCalcOnly = s_CalcOnly;
      s_CalcOnly = s_CalcOnly || !s_FirstCalculation;
      if( CalcExpEq( Temp.WriteE() ) )
        {       
        for( PExMemb Memb = CastPtr( TLexp, s_Answer )->First(); !Memb.isNull(); Memb = Memb->m_pNext )
          {
          MathExpr exLeft, exRight;
          Memb->m_Memb.Binar( '=', exLeft, exRight );
          Result.Addexp( exRight );        
          }
        }
      s_PutAnswer = OldPutAnswer;
      s_CalcOnly = OldCalcOnly;
      s_FirstCalculation = false;
      if( Result.First().isNull() && s_Answer.IsEmpty( ) )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      return Result;     
      }
    Lexp Cond;
    Parser P( &Cond );
    PNode node = P.AnyExpr( Temp.WriteE() );
    bool OldNoRootReduce = s_NoRootReduce;
    try
      {
      s_NoRootReduce = false;
      Lexp Sol = SolutionSimpleEquaion( node, VarName );
      s_NoRootReduce = OldNoRootReduce;
      if( Sol.Last().isNull() )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      Result.Addexp( CastPtr( TBinar, Sol.Last()->m_Memb )->Right() );
      }
    catch( ErrParser )
      {
      s_NoRootReduce = OldNoRootReduce;
      return Result;
      }
    }
  return Result;
  }

MathExpr SysRatInEq( MathExpr ex, int& Count, Lexp& PointVal, Lexp& PointSign, Lexp& IntervalVal, bool& IsSolv, const QByteArray& VarName = "x" )
  {
  struct Point
    {
    double m_X;
    bool m_Sign;
    MathExpr m_ExX;
    Point() {}
    };

  QVector<Point> Points;
  Points.resize( 100 );
  bool Interval[100];
  PascArray<bool> Signs( 1, 20 );
  PascArray<MathExpr> Nom( 1, 20 ), Den( 1, 20 );
  auto Solve = [&] ()
    {
    QByteArray sResult;
    int i = 0;
    do
      {
      for( ; i <= Count && !Interval[i]; i++ );
      if( i > Count ) break;
      if( !sResult.isEmpty() ) sResult += QByteArray( 1, msUnion );
      if( Points[i].m_Sign )
        sResult += "[";
      else
        sResult += "(";
      if( i == 0 )
        sResult += "-" + QByteArray( 1, msInfinity ) + ";";
      else
        sResult = sResult + Points[i].m_ExX.WriteEB() + ";";
      for( i++; i <= Count && Points[i].m_Sign && Interval[i]; i++ );
      if( i > Count )
        sResult += "+" + QByteArray( 1, msInfinity ) + ")";
      else
        {
        sResult += Points[i].m_ExX.WriteEB();
        if( Points[i].m_Sign )
          sResult += "]";
        else
          sResult += ")";
        }
      } while( i <= Count );

      for( i = 1; i <= Count; i++ )
      if( Points[i].m_Sign && !Interval[i - 1] && !Interval[i] )
        {
        if( !sResult.isEmpty() ) sResult += QByteArray( 1, msUnion );
        sResult += '{' + Points[i].m_ExX.WriteEB() + '}';
        }
      if( !sResult.isEmpty() )
        return Parser::StrToExpr( VarName + QByteArray( 1, msAddition ) + sResult );
    return MathExpr(new TCommStr( X_Str( "MNoSolution", "No Solutions!" ) ) );
    };

  auto AddPoints = [&]( Lexp& Roots, bool ASign )
    {
    for( PExMemb f = Roots.First(); !f.isNull(); f = f->m_pNext)
      {
      bool NoLogReduceOld = s_NoLogReduce;
      bool NoRootReduceOld = s_NoRootReduce;
      s_NoLogReduce = false;
      s_NoRootReduce = false;
      MathExpr ex = f->m_Memb.Reduce();
      s_NoLogReduce = NoLogReduceOld;
      s_NoRootReduce = NoRootReduceOld;
      double v;
      if( !ex.Constan( v ) )
        {
        int Nom, Den;
        if( !ex.SimpleFrac_( Nom, Den ) )
          throw  ErrParser( X_Str( "MnotSuitableInEq", "Improper inequality!" ), peNewErr );
        v = (double) Nom / Den;
        }
      int i = 1;
      for( ; i <= Count && v > Points[i].m_X; i++ );
      if( i > Count )
        {
        Points[++Count].m_X = v;
        Points[Count].m_ExX = ex;
        Points[Count].m_Sign = ASign;
        }
      else
        if( v == Points[i].m_X )
          Points[i].m_Sign = Points[i].m_Sign && ASign;
        else 
          {
          for( int j = Count; j >= i; j-- )
            Points[j + 1] = Points[j];
          Points[i].m_X = v;
          Points[i].m_ExX = ex;
          Points[i].m_Sign = ASign;
          Count++;
          }
      }
    };

  s_FirstCalculation = true;
  int OldDegPoly = s_DegPoly;
  s_DegPoly = 4;
  Lexp Solv = new TL2exp;
  MathExpr syst0 = ex, syst;

  Solv.Addexp( syst0 );
  Solv.Last()->m_Visi = false;
  if( !ex.Syst_( syst ) )
    syst = ex;
  PExMemb f;
  if( !syst.List2ex( f ) )
    {
    ex = new TL2exp;
    CastPtr( TL2exp, ex)->Addexp( syst );
    syst = ex;
    syst0 = syst;
    }
  Lexp syst1 = new TL2exp;
  f = CastPtr( TL2exp, syst )->First();
  int n = 0;
  bool IsInequality = false;
  uchar RelSign;
  for( ; !f.isNull(); f = f->m_pNext )
    {
    MathExpr op1, op2, ex1;
    if( !f->m_Memb.Binar_( RelSign, op1, op2 ) )
      throw  ErrParser( "Syntax error!", peSyntacs );
    Signs[++n] = RelSign == msMinequal || RelSign == msMaxequal;
    IsInequality = IsInequality || RelSign != '=';
    if( op1.IsFactorized( VarName ) && op2 == 0 )
      {
      ex1 = op1;
      Nom[n] = op1;
      Den[n].Clear();
      }
    else
      {
      ex = op1 - op2;
      ex1 = ExprToRatFrac( ex, Nom[n], Den[n], VarName );
      }

    if( ex1.HasUnknown().isEmpty() )
      {
      IsInequality = false;
      break;
      }
    ex = new TBinar( RelSign, ex1, Constant( 0 ) );
    syst1.Addexp( ex );
    syst1.Last()->m_Visi = false;
    }

  if( !IsInequality )
    {
    if( RelSign != '=' )
      throw  ErrParser( X_Str( "MnotSuitableInEq", "Improper inequality!" ), peNewErr );
    throw  ErrParser( X_Str( "MEnterInequality", "Enter inequality!" ), peNewErr );
    }

  if( n > 1 )
    syst = new TSyst( syst1 );
  else
  syst = syst1;
  if( !syst.Equal( syst0 ) )
    {
    Solv.Addexp( syst );
    Solv.Last()->m_Visi = false;
    }

  Count = 0;
  for( int i = 1; i <= n; i++ )
    {
    Lexp LN(Roots( Nom[i], VarName) );
    AddPoints( LN, Signs[i] );
    if( !Den[i].IsEmpty() )
      {
      Lexp LD(Roots( Den[i], VarName) );
      AddPoints( LD, false );
      }
    }
  double a = Points[1].m_X - 1;
  double Len = Points[Count].m_X + 1 - a;
  IsSolv = true;
  int j = 0;
  for( f = syst1.First(); !f.isNull(); f = f->m_pNext )
    {
    j++;
    if( n > 1 )
      {
      Solv.Addexp( f->m_Memb );
      Solv.Last()->m_Visi = false;
      }
    Solv.Addexp( new TCommStr( X_Str( "MCriticalPointExpr", "Critical point" ) ) );
    Solv.Last()->m_Visi = false;
    Solv.Addexp( new TBinar( '=', Nom[j].Reduce(), Constant( 0 ) ) );
    Solv.Addexp( new TStr( " " ) );
    Solv.Last()->m_Visi = false;
    Lexp NomRoots = Roots( Nom[j], VarName );
    int i = 0;
    syst0 = new TLexp;   
    for( PExMemb f1 = NomRoots.First(); !f1.isNull(); f1 = f1->m_pNext )
      {
      i++;
      CastPtr(TLexp, syst0 )->Addexp( new TBinar( '=', Variable( VarName + "_" + NumberToStr( i ) ), f1->m_Memb ) );    
      }
    if( i > 0 )
      Solv.Addexp( syst0 );
    else
    Solv.Addexp( new TCommStr( X_Str( "MNotFoundRootsExpr", "No roots found" ) ) );
    Solv.Last()->m_Visi = false;
    int i1 = i;
    Lexp DenRoots;
    if( !Den[j].IsEmpty() )
      {
      Solv.Addexp( new TBinar( '=', Den[j], Constant( 0 ) ) );
      Solv.Addexp( new TStr( " " ) );
      Solv.Last()->m_Visi = false;
      DenRoots = Roots( Den[j], VarName );
      syst0 = new TLexp;
      for( PExMemb f1 = DenRoots.First(); !f1.isNull(); f1 = f1->m_pNext )
        {
        i++;
        CastPtr( TLexp, syst0 )->Addexp( new TBinar( '=', Variable( VarName + "_" + NumberToStr( i ) ), f1->m_Memb ) );
        }
      if( i > i1 )
        Solv.Addexp( syst0 );
      else
      Solv.Addexp( new TCommStr( X_Str( "MNotFoundRootsExpr", "No roots found" ) ) );
      Solv.Last()->m_Visi = false;
      }
    Count = 0;
    AddPoints( NomRoots, Signs[j] );
    if( Den[j] != nullptr )
      AddPoints( DenRoots, false );
    if( n > 1 )
      {
      Lexp L1 = new TLexp, L2 = new TLexp;
      for( i = 1; i <= Count; i++ )
        {
        L1.Addexp( Points[i].m_ExX );
        L2.Addexp( new TBool( Points[i].m_Sign ) );
        }
      Points[0].m_X = Points[1].m_X - 1;
      Points[0].m_ExX = ( Points[1].m_ExX - 1 ).Reduce();
      Points[Count + 1].m_X = Points[Count].m_X + 1;
      Points[Count + 1].m_ExX = ( Points[Count].m_ExX + 1 ).Reduce();
      Lexp L3 = new TLexp;
      bool Value;
      for( i = 0; i <= Count; i++ )
        {
        MathExpr ValueX = Constant( ( Points[i].m_X + Points[i + 1].m_X ) / 2 );
        try
          {
          f->m_Memb.Substitute( VarName, ValueX ).Reduce().Boolean_( Value );
          }
        catch( ErrParser )
          {
           Value = false;
          }
        L3.Addexp( new TBool( Value ) );
        }
      if( Count > 0 )
        Solv.Addexp( new TCommStr( X_Str( "MAcceptablisSetExpr", "Solution set" ) ) );
      else
        if( Value )
          Solv.Addexp( new TCommStr( X_Str( "MAllNumbersExpr", "Solution set: all real numbers" ) ) );
      else 
        {
        Solv.Addexp( new TCommStr( X_Str( "MEmptySetExpr", "Solution set: the empty set" ) ) );
        IsSolv = false;
        }
      Solv.Addexp( new TInterval( a, Len, L1, L2, L3 ) );
      Solv.Last()->m_Visi = false;
      }
    }

  Count = 0;
  for( int i = 1; i <= n; i++ )
    {
    Lexp LN(Roots( Nom[i], VarName) );
    AddPoints( LN, Signs[i] );
    if( !Den[i].IsEmpty() )
      {
      Lexp LD(Roots( Den[i], VarName) );
      AddPoints( LD, false );
      }
    }
  Points[0].m_X = Points[1].m_X - 1;
  Points[0].m_Sign = false;
  Points[Count + 1].m_X = Points[Count].m_X + 1;
  Points[Count + 1].m_Sign = true;
  IntervalVal = new TLexp;
  syst.Syst_( syst );
  bool FinishValue;
  for( int i = 0; i <= Count; i++ )
    {
    FinishValue = true;
    MathExpr ValueX = Constant( ( Points[i].m_X + Points[i + 1].m_X ) / 2 );   
    for( f = CastPtr( TL2exp, syst )->First(); !f.isNull(); f = f->m_pNext )
      {
      try
        {
        bool Value;
        f->m_Memb.Substitute( VarName, ValueX ).Reduce().Boolean_( Value );
        FinishValue = FinishValue && Value;
        }
      catch( ErrParser )
        {
        FinishValue = false;
        }
      }
    Interval[i] = FinishValue;
    IntervalVal.Addexp( new TBool( FinishValue ) );
    }

  for( int i = 1; i <= Count; i++ )
    if( Points[i].m_Sign )
      {
      FinishValue = true;
      MathExpr ValueX = Constant( Points[i].m_X );
      for( f = CastPtr( TL2exp, syst )->First(); !f.isNull(); f = f->m_pNext )
        {
        bool Value;
        f->m_Memb.Substitute( VarName, ValueX ).Reduce().Boolean_( Value );
        FinishValue = FinishValue && Value;
        }
      Points[i].m_Sign = FinishValue;
      }

  PointVal = new TLexp;
  PointSign = new TLexp;
  for( int i = 1; i <= Count; i++ )
    {
    PointVal.Addexp( Points[i].m_ExX );
    PointSign.Addexp( new TBool( Points[i].m_Sign ) );
    }
  Solv.Addexp( new TCommStr( X_Str( "MResultingSetExpr", "Solution set" ) ) );

  Solv.Last()->m_Visi = false;
  Solv.Addexp( new TInterval( Points[0].m_X, Points[Count + 1].m_X - Points[0].m_X,
    PointVal, PointSign, IntervalVal ) );
  Solv.Last()->m_Visi = false;
  Solv.Addexp( Solve() );
  s_DegPoly = OldDegPoly;
  return Solv;
  }

bool CalcSysInEq( const QByteArray& InEq )
  {
  if( InEq.isEmpty() ) return false;
  QByteArray Res;
  QByteArray VarName;
  std::function<void( const MathExpr& )> UnpackBinary = [&] ( MathExpr exBinary )
    {
    MathExpr op1, op2;
    if( exBinary.Binar( msUnion, op1, op2 ) )
      {
      UnpackBinary( op1 );
      Res += ";";
      UnpackBinary( op2 );
      return;
      }

    TSimpleInterval &SInt = *CastPtr( TSimpleInterval, exBinary );
    if( IsType( TInfinity, SInt.Left() ) )
      {
      Res += VarName;
      if( SInt.m_Brackets[1] == ']' )
        Res += QByteArray( 1, msMinequal );
      else
        Res += '<';
      Res += SInt.Right().Reduce().WriteE();
      }
    else
      {
      Res += SInt.Left().Reduce().WriteE();
      if( SInt.m_Brackets[0] == '[' )
        Res += QByteArray( 1, msMinequal );
      else
        Res += '<';
      Res += VarName;
      if( !( IsType( TInfinity, SInt.Right() ) ) )
        {
        if( SInt.m_Brackets[1] == ']' )
          Res += QByteArray( 1, msMinequal );
        else
          Res += '<';
        Res += SInt.Right().Reduce().WriteE();
        }
      }
    };

  int OldDegPoly = s_DegPoly;
  s_DegPoly = 3;
  bool Result = false;

  auto Final = [&] ()
    {
    s_DegPoly = OldDegPoly;
    return Result;
    };

  MathExpr ex1;
  int Count;
  Lexp L1 = new TLexp, L2 = new TLexp, L3 = new TLexp;
  bool IsSolv;
  Parser P;
  auto Label1 = [&] ()
    {
    try
      {
      ex1 = SysRatInEq( ex1, Count, L1, L2, L3, IsSolv, VarName );
      PExMemb f = CastPtr( TLexp, ex1 )->First();
      MathExpr op1, op2;
      for( ; !f.isNull(); f = f->m_pNext )
        if( f->m_Memb.Binar( msAddition, op1, op2 ) ) break;
      if( op1.IsEmpty() ) throw 1;
      UnpackBinary( op2 );
      f = CastPtr( TLexp, ex1 )->First();
      IsSolv = false;
      do
        {
        if( IsSolv )
          if( IsConstType( TStr, f->m_Memb ) )
            break;
          else
            TSolutionChain::sm_SolutionChain.AddExpr( f->m_Memb );
        else
          {
          IsSolv = IsConstType( TStr, f->m_Memb );
          if( IsSolv ) TSolutionChain::sm_SolutionChain.AddExpr( f->m_Memb );
          }
        f = f->m_pNext;
        } while( true );
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( Res ) );
        if( s_PutAnswer && !s_Answer.IsEmpty() ) s_Answer = P.StrToExpr( Res );
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MCalced", "Calculated!" ) );
        Result = true;
      }
    catch( int )
      {
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNotSuitableInEq", "Wrong kind of inequality!" ) );
      }
    return Final();
    };

  try
    {
    PNode eq = P.AnyExpr( P.FullPreProcessor( InEq, "x" ) );
    if( IsFuncEqu( eq ) || TestFrac( eq ) ) return Final();
    MathExpr  ex = P.OutPut( eq );
    VarName = ex.HasUnknown();
    if( VarName.isEmpty() ) return Label1();
    MathExpr  syst0 = ex;
    uchar RelSign;
    MathExpr op1, op2, syst;
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    if( !ex.Syst_( syst ) )
      syst = ex;
    PExMemb f;
    if( !syst.List2ex( f ) )
      {
      Lexp Cond;
      ex1 = RemDenominator( ex, Cond );
      if( !ex.Eq( ex1 ) ) {
        ex = ex1;
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        }
      ex1 = Transfer( ex );
      ex = ex1.Reduce();
      uchar BinName;
      if( !ex.Eq( ex1 ) && ex.Binar_( BinName, op1, op2 ) && ( IsConstType( TVariable, op1 ) ) )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        if( s_PutAnswer ) s_Answer = ex;
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MCalced", "Calculated!" ) );
        Result = true;
        }
      else
        {
        ex1 = ex;
        ex = new TL2exp;
        CastPtr( TL2exp, ex )->Addexp( ex1 );
        syst = ex;
        syst0 = syst;
        }
      }
    if( !Result )
      {
      int n = 0;
      int MinBound = 0;
      int MaxBound = 0;
      uchar LastMinSign = '<';
      uchar LastMaxSign = '>';
      double MinV = 0.0;
      double MaxV = 0.0;
      Lexp syst1 = new TL2exp;
      for( f = CastPtr( TL2exp, syst )->First(); !f.isNull(); f = f->m_pNext )
        {
        n++;
        if( !f->m_Memb.Binar_( RelSign, op1, op2 ) )
          throw  ErrParser( "Syntax error!", peSyntacs );
        ex = op1 - op2;
        MathExpArray a;
        ex.ReductionPoly( a, VarName );
        double v;
        for( int i = 2; i <= s_DegPoly; i++ )
          if( !( a[i].Constan( v ) && abs( v ) < 0.0000001 ) ) return Label1();
        if( a[1].Constan( v ) && abs( v ) < 0.0000001 ) return Label1();
        int nom, den;
        if( !( a[0].Constan( v ) || a[0].SimpleFrac_( nom, den ) ) ) return Label1();
        if( !a[1].Constan( v ) )
          if( a[1].SimpleFrac_( nom, den ) )
            v = ( double ) nom / den;
          else
            return Label1();
        if( v < 0 )
          switch( RelSign )
          {
            case '<':
              RelSign = '>';
              break;
            case msMinequal:
              RelSign = msMaxequal;
              break;
            case '>':
              RelSign = '<';
              break;
            case msMaxequal:
              RelSign = msMinequal;
          }
        MathExpr b = ( -a[0] / a[1] ).Reduce();
        if( !b.Constan( v ) )
          if( b.SimpleFrac_( nom, den ) )
            v = ( double ) nom / den;
        switch( RelSign )
          {
          case '<':
          case msMinequal:
            if( MinBound == 0 )
              {
              LastMinSign = RelSign;
              MinV = v;
              }
            else
              if( v < MinV || v == MinV && RelSign == '<' )
                {
                LastMinSign = RelSign;
                MinV = v;
                }
            MinBound = 1;
            break;
          case '>':
          case msMaxequal:
            if( MaxBound == 0 )
              {
              LastMaxSign = RelSign;
              MaxV = v;
              }
            else
              if( v > MaxV || v == MaxV && RelSign == '>' )
                {
                LastMaxSign = RelSign;
                MaxV = v;
                }
            MaxBound = 1;
            break;
          default:
            throw  ErrParser( "Syntax error", peSyntacs );
          }

        ex = new TBinar( RelSign, Variable( ( VarName ) ), b );
        syst1.Addexp( ex );
        syst1.Last()->m_Visi = false;
        }
      if( n > 1 )
        syst = new TSyst( syst1 );
      else
        syst = syst1;
      if( !syst.Equal( syst0 ) )
        TSolutionChain::sm_SolutionChain.AddExpr( syst );
      if( MinBound == 1 && MaxBound == 1 && ( MinV < MaxV || ( MinV == MaxV && ( LastMinSign == '<' || LastMaxSign == '>' ) ) ) )
        {
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solution" ) );
        s_Answer.Clear();
        }
      else
        {
        switch( 2 * MinBound + MaxBound )
          {
          case 1:
            Res = VarName + QByteArray( 1, LastMaxSign ) + QByteArray::number( MaxV );
            break;
          case 2:
            Res = VarName + QByteArray( 1, LastMinSign ) + QByteArray::number( MinV );
            break;
          case  3:
            if( MaxV == MinV )
              Res = VarName + "=" + QByteArray::number( MinV );
            else
              {
              switch( LastMaxSign )
                {
                case '>':
                  LastMaxSign = '<';
                  break;
                case msMaxequal:
                  LastMaxSign = msMinequal;
                }
              Res = QByteArray::number( MaxV ) + QByteArray( 1, LastMaxSign ) + VarName + QByteArray( 1, LastMinSign ) + QByteArray::number( MinV );
              }
          }
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( Res ) );
        if( s_PutAnswer && s_Answer.IsEmpty() ) s_Answer = P.StrToExpr( Res );
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MCalced", "Calculated!" ) );
        Result = true;
        }
      }
    if( !Result )
      {
      ex1 = P.OutPut( eq );
      return Label1();
      }
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    }
  return Final();
  }

bool CalcRatInEq( const QByteArray& InSource )
  {
  if( InSource.isEmpty() ) return false;
  MathExpr exLogBase, exLogArg;
  std::function<bool( const MathExpr& )> ImpossibleFunc = [&] ( const MathExpr& exp )
    {
    MathExpr exLeft, exRight;
    char cOper;
    if( exp.Log( exLeft, exRight ) )
      {
      if( !exLogArg.IsEmpty() ) return true;
      exLogArg = exRight;
      exLogBase = exLeft;
      return !exLogBase.HasUnknown().isEmpty() || ImpossibleFunc( exLogArg );
      }
    if( IsConstType( TFunc, exp ) ) return true;
    if( exp.Oper_( cOper, exLeft, exRight ) )
      return ImpossibleFunc( exLeft ) || ImpossibleFunc( exRight );
    if( exp.Unarminus( exLeft ) ) return ImpossibleFunc( exLeft );
    return false;
    };

  bool Result = false;
  try
    {
    Parser P;
    MathExpr ex = P.StrToExpr( InSource );
    QByteArray VarName;
    if( !ImpossibleFunc( ex ) )
      {
      VarName = ex.HasUnknown();
      if( VarName.isEmpty() )
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MnotSuitableInEq", "Improper inequality!" ) );
      else
        {
        if( !exLogArg.IsEmpty() )
          {
          Lexp Syst = new TL2exp;
          Syst.Addexp( ex );
          Syst.Addexp( new TBinar( '>', exLogArg, Constant( 0 ) ) );
          ex = Syst;
          }
        s_ShowDiviMessages = false;
        int Count;
        Lexp L1 = new TLexp, L2 = new TLexp, L3 = new TLexp;
        bool IsSolv;
        MathExpr ex1 = SysRatInEq( ex, Count, L1, L2, L3, IsSolv, VarName );
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MCalced", "Calculated!" ) );
        Result = true;
        }
      }
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ) );
    if( E.Name() == "MNoSolvType" )
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MnotSuitableInEq", "Improper inequality!" ) );
    else
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( E.Name(), E.Message() ) );
    }
  s_ShowDiviMessages = true;
  return Result;
  }

Lexp CalcIrratEq( const QByteArray& Source, bool PutSource, bool Recurs )
  {
  if( Source.isEmpty() ) return Lexp();

  typedef QVector<PNode> TRoots;
  typedef QVector<MathExpr> TRootArgs;

  Parser P;
  bool bWasRoots;

  auto Analiz = [&] ( PNode eq, TRoots& Roots, TRootArgs& Args, bool bEvalArgs )
    {
    std::function<void( PNode )> PreOrder = [&] ( PNode p )
      {
      if( p == nullptr ) return;
      if( p->m_OpSign == '~' )
        {
        bWasRoots = true;
        if( p->m_IsLeft )
          {
          Roots.append( p );
          Args.append( MathExpr() );
          if( bEvalArgs )
            Args.last() = P.OutPut( p->m_pLeft );
          return;
          }
        }
      PreOrder( p->m_pLeft );
      PreOrder( p->m_pRight );
      };
    Roots.clear();
    Args.clear();
    PreOrder( eq );
    };

  auto Analiz1 = [&] ( PNode eq, TRoots& Roots, TRootArgs& Args, bool bEvalArgs )
    {
    std::function<void( PNode )> PreOrder = [&] ( PNode p )
      {
      if( p == nullptr ) return;
      if( ( p->m_OpSign == '~' || ( p->m_OpSign == '^' && p->m_pRight->m_OpSign == '/' ) ) && p->m_IsLeft )
        {
        Roots.append( p );
        Args.append( MathExpr() );
        if( bEvalArgs )
          Args.last() = P.OutPut( p->m_pLeft );
        return;
        }
      PreOrder( p->m_pLeft );
      PreOrder( p->m_pRight );
      };
    Roots.clear();
    Args.clear();
    PreOrder( eq );
    };

  TRoots LRoots, RRoots, ARoots;
  TRootArgs LArgs, RArgs;
  int NomDegs[5], Degs[5];

  auto DefDeg = [&] ( PNode eq )
    {
    bWasRoots = false;
    Analiz( eq, ARoots, LArgs, false );
    if( !bWasRoots ) return -1;
    int Result = 0;
    for( int i = 0; i < ARoots.count(); i++ )
      {
      if( ARoots[i]->m_pRight->m_OpSign != 'i' )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      int Deg = ARoots[i]->m_pRight->m_Info.toInt();
      int j = 0;
      for( ; j < Result && Deg != Degs[j]; j++ );
      if( j >= Result ) Degs[Result++] = Deg;
      }
    return Result;
    };

  auto DefDeg1 = [&] ( PNode eq )
    {
    Analiz1( eq, ARoots, LArgs, false );
    int Result = 0;
    for( int i = 0; i < ARoots.count(); i++ )
      {
      if( !In( ARoots[i]->m_pRight->m_OpSign, "i/" ) )
        throw  ErrParser( "The type of the equation does ! correspond", peNoSolvType );
      switch( ARoots[i]->m_pRight->m_OpSign )
        {
        case 'i':
          {
          int Deg = ARoots[i]->m_pRight->m_Info.toInt();
          int j = 0;
          for( ; j < Result && Deg != Degs[j]; j++ );
          if( j > Result )
            {
            Degs[Result] = Deg;
            NomDegs[Result++] = 1;
            }
          }
          break;
        case '/':
          {
          int Nom = ARoots[i]->m_pRight->m_pLeft->m_Info.toInt();
          int Deg = ARoots[i]->m_pRight->m_pRight->m_Info.toInt();
          int j = 0;
          for( ; j < Result && ( Deg != Degs[j] || Nom != NomDegs[j] ); j++ );
          if( j > Result )
            {
            Degs[Result] = Deg;
            NomDegs[Result++] = Nom;
            }
          }
        }
      }
    return Result;
    };

  std::function<PNode( PNode )> PostOrder = [&] ( PNode p )
    {
    if( p == nullptr ) return p;
    p->m_pLeft = PostOrder( p->m_pLeft );
    p->m_pRight = PostOrder( p->m_pRight );
    switch( p->m_OpSign )
      {
      case '+':
      case '*':
        if( p->m_pLeft == nullptr )  return p->m_pRight;
        if( p->m_pRight == nullptr ) return p->m_pLeft;
        return p;
      case '-':
        if( p->m_pLeft == nullptr )
          {
          if( p->m_pRight != nullptr ) return UnMinus( p->m_pRight );
          return p->m_pRight;
          }
        if( p->m_pRight == nullptr ) return p->m_pLeft;
        return p;
      case '/': return p;
      case 'u':
        if( p->m_pRight == nullptr ) p->m_pRight;
        return p;
      default:
        return p;
      }
    };

  std::function<bool( PNode&, PNode, int )> MoveTerm = [&] ( PNode& eq, PNode r, int D )
    {
    PNode q1 = r;
    PNode q2 = q1->m_pParent;
    int Minus = 0;
    while( q2 != nullptr && !In( q2->m_OpSign, "=*/" ) )
      {
      if( q2->m_OpSign == '-' && q1 == q2->m_pRight || q2->m_OpSign == 'u' ) Minus++;
      q1 = q2;
      q2 = q2->m_pParent;
      }
    if( q2 != nullptr && In( q2->m_OpSign, "*/" ) )
      {
      MoveTerm( eq, q2, D );
      return false;
      }
    PNode q = r->m_pParent;
    if( q->m_pLeft == r )
      q->m_pLeft = nullptr;
    else
      q->m_pRight = nullptr;
    if( Minus % 2 == 0 )
      switch( D )
      {
        case 0:
          eq->m_pRight = Sub( eq->m_pRight, r );
          break;
        case 1:
          eq->m_pLeft = Sub( eq->m_pLeft, r );
      }
    else
      switch( D )
      {
        case 0:
          eq->m_pRight = Add( eq->m_pRight, r );
          break;
        case 1:
          eq->m_pLeft = Add( eq->m_pLeft, r );
      }
    eq = PostOrder( eq );
    return true;
    };

  auto MoveRadical = [&] ( PNode& eq, PNode r, int D )
    {
    MoveTerm( eq, r, D );
    PostOrder( eq );
    };

  auto ThereAreRadicals = [&] ( PNode eq )
    {
    std::function<void( PNode )> PreOrder = [&] ( PNode p )
      {
      if( p == nullptr ) return;
      if( ( p->m_OpSign == '~' || p->m_OpSign == '^' && p->m_pRight->m_OpSign == '/' ) && p->m_IsLeft )
        throw  1;
      PreOrder( p->m_pLeft );
      PreOrder( p->m_pRight );
      };
    try
      {
      PreOrder( eq );
      }
    catch( int )
      {
      return true;
      }
    return false;
    };

  auto MoveRadFreeMemb = [&] ( PNode& eq, int D )
    {
    bool Done = false;
    std::function<void( PNode )> PreOrder = [&] ( PNode p )
      {
      if( p == nullptr ) return;
      if( p->m_OpSign == '~' ) return;
      if( In( p->m_OpSign, "+-" ) )
        {
        if( !ThereAreRadicals( p->m_pLeft ) )
          Done = MoveTerm( eq, p->m_pLeft, D );
        if( !ThereAreRadicals( p->m_pRight ) )
          Done = MoveTerm( eq, p->m_pRight, D );
        }
      if( In( p->m_OpSign, "*/" ) )
        if( !( ThereAreRadicals( p->m_pLeft ) || ThereAreRadicals( p->m_pRight ) ) )
          {
          Done = MoveTerm( eq, p, D );
          return;
          }
      PreOrder( p->m_pLeft );
      PreOrder( p->m_pRight );
      };

    switch( D )
      {
      case 0:
        PreOrder( eq->m_pLeft );
        break;
      case 1:
        PreOrder( eq->m_pRight );
      }
    if( Done )
      PostOrder( eq );
    return Done;
    };

  auto ThereRadFreeMemb = [&] ( PNode& eq )
    {
    std::function<void( PNode )> PreOrder = [&] ( PNode p )
      {
      if( p == nullptr ) return;
      if( p->m_OpSign == '~' ) return;
      if( In( p->m_OpSign, "+-" ) )
        if( !( ThereAreRadicals( p->m_pLeft ) && ThereAreRadicals( p->m_pRight ) ) )
          throw  1;
      if( In( p->m_OpSign, "*/" ) )
        if( !( ThereAreRadicals( p->m_pLeft ) || ThereAreRadicals( p->m_pRight ) ) )
          throw  1;
        else
          return;
      PreOrder( p->m_pLeft );
      PreOrder( p->m_pRight );
      };

    if( !ThereAreRadicals( eq ) ) return true;
    try
      {
      PreOrder( eq );
      }
    catch( int )
      {
      return true;
      }
    return false;
    };

  std::function<bool( PNode )> NotOnlyInRad = [&] ( PNode p )
    {
    if( p == nullptr ) return false;
    switch( p->m_OpSign )
      {
      case '~': return false;
      case '^': return p->m_pRight->m_OpSign != '/';
      case 'v':
        if( p->m_Info.toUpper() == "X" ) return true;
        return false;
      default:
        return NotOnlyInRad( p->m_pLeft ) || NotOnlyInRad( p->m_pRight );
      }
    };

  auto Balance = [&] ( PNode& eq, bool& Move )
    {
    Analiz( eq->m_pLeft, LRoots, LArgs, true );
    Analiz( eq->m_pRight, RRoots, RArgs, true );
    int LCount = LRoots.count();
    int RCount = RRoots.count();
    if( LCount + RCount == 0 ) return false;
    bool LFree = ThereRadFreeMemb( eq->m_pLeft );
    bool RFree = ThereRadFreeMemb( eq->m_pRight );
    Move = false;
    switch( LCount + RCount )
      {
      case 1:
        {
        if( LCount == 1 )
          if( LFree )
            Move = MoveRadFreeMemb( eq, 0 );
          else
            Move = false;
        else
          if( RFree )
            Move = MoveRadFreeMemb( eq, 1 );
          else
            Move = false;
        }
        break;
      case 2:
        {
        if( LCount == RCount )
          if( LFree && RFree )
            Move = MoveRadFreeMemb( eq, 0 );
          else
            Move = false;
        else
          if( LCount == 2 )
            {
            MoveRadical( eq, LRoots[1], 0 );
            LRoots.removeAt( 1 );
            LArgs.removeAt( 1 );
            LCount = 1;
            RCount++;
            RArgs.append( LArgs[1] );
            if( LFree )
              MoveRadFreeMemb( eq, 0 );
            Move = true;
            }
          else
            {
            MoveRadical( eq, RRoots[1], 1 );
            LRoots.removeAt( RCount - 1 );
            LArgs.removeAt( RCount - 1 );
            RCount = RCount - 1;
            LCount = LCount + 1;
            LArgs.append( RArgs[1] );
            if( RFree )
              MoveRadFreeMemb( eq, 1 );
            Move = true;
            }
        for( int I = 1; I < LCount; I++ )
          {
          if( !LArgs[I].Equal( LArgs[0] ) )
            {
            MoveRadical( eq, LRoots[I], 0 );
            Move = true;
            }
          }

        if( LCount > 0 )
          {
          for( int I = 0; I < RCount; I++ )
            {
            if( LArgs[0].Equal( RArgs[I] ) ) {
              MoveRadical( eq, RRoots[I], 1 );
              Move = true;
              }
            }
          }
        }
      }
    return true;
    };

  MathExpr Subst;
  std::function<MathExpr( const MathExpr& )> SubstY = [&] ( const MathExpr& ex )
    {
    MathExpr ex1, ex2, op1, op2;
    int rt;
    if( ex.Root_( op1, op2, rt ) || ex.Power( op1, op2 ) )
      {
      if( ex.Equal( Subst ) ) return Variable( "y" );
      return Variable( "y" ) ^ 2;
      }
    char c;
    if( ex.Oper_( c, op1, op2 ) )
      {
      ex1 = SubstY( op1 );
      ex2 = SubstY( op2 );
      switch( c )
        {
        case '+': return ex1 + ex2;
        case '-': return ex1 - ex2;
        case '*': return ex1 * ex2;
        case '/': return ex1 / ex2;
        case '^':
        case '~': return ex1 ^ ex2;
        case '=': return MathExpr( new TBinar( '=', ex1, ex2 ) );
        default:
          return MathExpr( new TBinar( c, ex1, ex2 ) );
        }
      }
    if( ex.Unarminus( op1 ) ) return -SubstY( op1 );
    return MathExpr( ex );
    };

  std::function<Lexp( MathExpr, int& )> GetMults = [&] ( MathExpr ex, int& Count )
    {
    Lexp Result = new TLexp;

    std::function<void( const MathExpr& )> PreOrder = [&] ( const MathExpr& p )
      {
      MathExpr op1, op2;
      double v;
      if( p.Multp( op1, op2 ) )
        {
        PreOrder( op1 );
        PreOrder( op2 );
        }
      else
        if( !p.Constan( v ) )
          {
          Result.Addexp( p );
          Count++;
          }
      };

    MathExpArray Radicals;
    MathExpr SubstExpr, Mult;

    std::function<MathExpr( const MathExpr& )> SubstVar = [&] ( const MathExpr& ex )
      {
      MathExpr op1, op2;
      int rt;
      if( ex.Root_( op1, op2, rt ) )
        {
        int num = 0;
        for( ; num < Radicals.count() && ex.Equal( Radicals[num] ); num++ );
        if( num == Radicals.count() )
          Radicals.append( ex );
        return Variable( "y" + NumberToStr( num ) );
        }

      char c;
      if( ex.Oper_( c, op1, op2 ) )
        {
        MathExpr ex1 = SubstVar( op1 );
        MathExpr ex2 = SubstVar( op2 );
        switch( c )
          {
          case '+': return ex1 + ex2;
          case '-': return ex1 - ex2;
          case '*': return ex1 * ex2;
          case '/': return ex1 / ex2;
          case '^':
          case '~': return ex1 ^ ex2;
          case '=': return MathExpr( new TBinar( '=', ex1, ex2 ) );
          default:
            return MathExpr( new TBinar( c, ex1, ex2 ) );
          }
        }
      if( ex.Unarminus( op1 ) ) return -SubstVar( op1 );
      return MathExpr( ex );
      };

    SubstExpr = SubstVar( ex );
    if( SubstExpr.Unarminus( ex ) )
      Mult = ex.ReduceToMult();
    else
      Mult = SubstExpr.ReduceToMult();
    for( int i = 0; i < Radicals.count(); i++ )
      Mult = Mult.Substitute( "y" + NumberToStr( i ), Radicals[i] );
    Count = 0;
    PreOrder( Mult );
    return Result;
    };

  std::function<MathExpr( const MathExpr& )> MultRootReduce = [&] ( const MathExpr& ex )
    {
    MathExpr op1, op2;
    char c;
    if( ex.Oper_( c, op1, op2 ) )
      {
      MathExpr ex1 = MultRootReduce( op1 );
      MathExpr ex2 = MultRootReduce( op2 );
      int rt1, rt2;
      MathExpr op11, op12, op21, op22;
      switch( c )
        {
        case '^': return ex1 ^ ex2;
        case '~': return MathExpr( ex );
        case '+': return ex1 + ex2;
        case '-': return ex1 - ex2;
        case '*':
          if( ex1.Root_( op11, op12, rt1 ) && ex2.Root_( op21, op22, rt2 ) && rt1 == rt2 )
            return ( ( op11 * op21 ).Root( rt1 ) ).Reduce();
          return ex1 * ex2;
        case '/': return ex1 / ex2;
        case '=': return MathExpr( new TBinar( '=', ex1, ex2 ) );
        default:
          return MathExpr( new TBinar( c, ex1, ex2 ) );
        }
      }
    if( ex.Unarminus( op1 ) )
      return -MultRootReduce( op1 );
    return MathExpr( ex );
    };

  auto ReduceFrac = [&] ( const MathExpr& ex )
    {
    int count = 0;
    MathExpr root;

    std::function<void( const MathExpr& )> PreOrder = [&] ( const MathExpr& e )
      {
      MathExpr op1, op2, op3, op4;
      int rt;
      char c;
      if( e.IsEmpty() || e.Root_( op1, op2, rt ) ) return;
      if( e.Divis( op1, op2 ) && op2.Root_( op3, op4, rt ) )
        {
        root = op2;
        count++;
        }
      else
        if( e.Oper_( c, op1, op2 ) )
          {
          PreOrder( op1 );
          PreOrder( op2 );
          }
      };

    PreOrder( ex );
    if( count > 1 )
      throw  ErrParser( "NoSolvType", peNoSolvType );
    if( root.IsEmpty() ) return MathExpr( ex );
    MathExpr op1, op2;
    ex.Binar( '=', op1, op2 );
    return MultRootReduce( new TBinar( '=', Expand( op1 * root ).Reduce(), Expand( op2 * root ).Reduce() ) );
    };

  bool bThereIsRoot = false, bWrongEquation = false;
  std::function<bool( MathExpr& )> PowerToRoot = [&] ( MathExpr& ex )
    {
    bool PTRResult = false;
    auto ConvertIfPossible = [&] ( MathExpr& op )
      {
      MathExpr exBase, exPower;
      int Nom, Denom;
      MathExpr Result = op;
      if( op.Power( exBase, exPower ) && exPower.SimpleFrac_( Nom, Denom ) )
        {
        if( !( IsConstType( TRoot, op ) ) )
          {
          PTRResult = true;
          if( Nom == 1 )
            Result = exBase.Root( Denom );
          else
            Result = ( exBase ^ Nom ).Root( Denom );
          }
        PowerToRoot( exBase );
        bThereIsRoot = true;
        }
      else
        if( PowerToRoot( op ) )
          PTRResult = true;
      return Result;
      };

    MathExpr exLeft, exRight;
    char cOp;
    if( !ex.Oper_( cOp, exLeft, exRight ) ) return PTRResult;
    CastPtr( TOper, ex )->Left() = ConvertIfPossible( CastPtr( TOper, ex )->Left() );
    if( cOp != '^' && cOp != '~' )
      CastPtr( TOper, ex )->Right() = ConvertIfPossible( CastPtr( TOper, ex )->Right() );
    else
      bWrongEquation = bWrongEquation || IsConstType( TPowr, CastPtr( TOper, ex )->Right() );
    return PTRResult;
    };

  double OldPrecision = s_Precision;
  Lexp Result = new TLexp;
  bool NoRootReduceOld = s_NoRootReduce;
  s_NoRootReduce = true;
  bool bPutAnswerOld = s_PutAnswer;
  bool OldCalcOnly = s_CalcOnly;
  bool OldExpandPower = s_ExpandPower;
  s_ExpandPower = true;

  auto Final = [&] ()
    {
    s_Precision = OldPrecision;
    s_NoRootReduce = NoRootReduceOld;
    s_PutAnswer = bPutAnswerOld;
    s_CalcOnly = OldCalcOnly;
    s_ExpandPower = OldExpandPower;
    return Result;
    };

  TL2exp Roots;
  MathExpr SourceEq, ex, arg;
  bool IsRoot, Define, bOtherTask;
  auto PostCase = [&] ()
    {
    PExMemb f;
    if( Roots.Count() > 0 && Roots.Listex( f ) && !f.isNull() )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MFinishRoot", "final result!" ) );
      int n = 0;
      int n1 = 0;
      if( s_PutAnswer ) s_Answer = new TL2exp;
      for( ; !f.isNull(); f = f->m_pNext )
        {
        n++;
        MathExpr ResultExp = new TBinar( '=', Variable( "x_" + NumberToStr( n ) ), f->m_Memb );
        MathExpr ex1;
        try
          {
          ex1 = SourceEq.Substitute( "x", f->m_Memb );
          ex1.Reduce().Boolean_( IsRoot );
          Define = true;
          }
        catch( ErrParser )
          {
          Define = false;
          }
        if( Define )
          {
          TSolutionChain::sm_SolutionChain.AddExpr( ResultExp, "", IsRoot );
          if( IsRoot )
            {
            n1++;
            TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
            Result.Addexp( f->m_Memb );
            if( s_PutAnswer )
              CastPtr( TL2exp, s_Answer )->Addexp( f->m_Memb );
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootEq", "The root of equation" ) );
            }
          else
            {
            MathExpr op1, op2;
            ex1.Binar( '=', op1, op2 );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, op1, op2 ), X_Str( "MRootStranger", "false Root!" ) );
            }
          }
        else
          {
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MForGivenValue", "for( a given value of a variable the expression" ) );
          TSolutionChain::sm_SolutionChain.AddExpr( SourceEq, X_Str( "MExprNotDef", " is ! defined\n" ) +
            X_Str( "MRootStranger", "false Root!" ) );
          }
        }
      if( n1 > 0 )
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MRootsFound", "Roots are found." ) );
      }
    else
      if( !bOtherTask ) throw  ErrParser( "NoSolvType", peNoSolvType );
    return Final();
    };

  int jj;
  auto Label2 = [&] ()
    {
    TSolutionChain::sm_SolutionChain.AddExpr( ex );
    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Subst, Variable( "y" ) ) );
    MathExpr ex1 = SubstY( ex );
    s_FinalComment = false;
    s_Calculations = false;
    Lexp RootsY = FractRatEq( ex1.WriteE(), "y" );
    s_FinalComment = true;
    s_Calculations = true;
    PExMemb f;
    if( !RootsY.IsEmpty() && RootsY.Listex( f ) && !f.isNull() )
      for( ; !f.isNull(); f = f->m_pNext )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Subst, f->m_Memb ) );
        double v;
        if( f->m_Memb.Constan( v ) && v < 0 && Degs[jj] % 2 != 0 )
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions!" ) );
        else
          {
          ex1 = new TBinar( '=', arg, ( f->m_Memb ^ Degs[jj] ).Reduce() );
          s_FinalComment = false;
          s_Calculations = false;
          Roots.Appendz( FractRatEq( ex1.WriteE(), "x" ) );
          s_FinalComment = true;
          s_Calculations = true;
          }
        }
    return PostCase();
    };

  auto Label3 = [&] ()
    {
    Subst = arg.Root( Degs[jj] );
    return Label2();
    };

  try
    {
    PNode eq = P.AnyExpr( P.FullPreProcessor( Source, "x" ), "x" );
    ex = P.OutPut( eq );
    SourceEq = ex;
    MathExpr op1, op2, expOut;
    if( !ex.Binar( '=', op1, op2 ) || op1.HasUnknown().isEmpty() && op2.HasUnknown().isEmpty() )
      throw  ErrParser( "Syntax error!", peSyntacs );
    if( IsFuncEqu( eq ) )
      throw  ErrParser( "NoSolvType", peNoSolvType );
    MathExpr ex1 = ex;
    if( PowerToRoot( ex ) )
      {
      eq = P.AnyExpr( ex.WriteE() );
      if( PutSource )
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      }
    else
      if( bWrongEquation || !bThereIsRoot )
        throw  ErrParser( "NoSolvType", peNoSolvType );
    if( DefDeg( eq ) == 1 )
      {
      bool MaySubst = true;
      arg = P.OutPut( ARoots[0]->m_pLeft );
      for( int i = 1; i < ARoots.count(); i++ )
        if( !arg.Equal( P.OutPut( ARoots[i]->m_pLeft ) ) )
          {
          MaySubst = false;
          break;
          }
      if( MaySubst && !NotOnlyInRad( eq ) && ARoots.count() > 1 )
        {
        jj = 1;
        return Label3();
        }
      }
    expOut = ex;
    TSolutionChain::sm_SolutionChain.AddExpr( expOut );
    ex1 = ReduceFrac( ex );
    if( !ex1.Equal( ex ) )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      ex = ex1;
      eq = P.AnyExpr( ex.WriteE() );
      }
    ex1 = ExpandExpr( ex );
    MathExpr ex2, ex3;
    if( !ex1.Equal( ex ) )
      {
      ex.Binar( '=', ex2, ex3 );
      ex1.Binar( '=', op1, op2 );
      if( !( IsConstType( TPowr, ex2 ) && !( IsConstType( TPowr, op1 ) ) || IsConstType( TPowr, ex3 ) && !( IsConstType( TPowr, op2 ) ) ) )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
        ex = ex1;
        eq = P.AnyExpr( ex.WriteE() );
        }
      }
    bOtherTask = false;
    int iResCode = DefDeg( eq );
    ex.Binar( '=', ex1, ex2 );
    int n;
    if( ( ex1.Root_( ex3, ex3, n ) || ex2.Root_( ex3, ex3, n ) ) && n % 2 != 0 && ( ex1.Negative() || ex2.Negative() ) ) iResCode = 3;
    switch( iResCode )
      {
      case -1:
        {
        bOtherTask = true;
        s_Answer.Clear();
        s_PutAnswer = true;
        OldCalcOnly = s_CalcOnly;
        try
          {
          PExMemb memMemb;
          if( Recurs && EquationsSolving( ex.WriteE() ) && !s_Answer.IsEmpty() )
            if( s_Answer.Listex( memMemb ) )
              for( ; !memMemb.isNull(); memMemb = memMemb->m_pNext )
                Result.Addexp( memMemb->m_Memb );
            else
              Result.Addexp( s_Answer );
          else
            throw  ErrParser( "NoSolvType", peNoSolvType );
          }
        catch( ErrParser )
          {
          s_PutAnswer = bPutAnswerOld;
          s_CalcOnly = OldCalcOnly;
          }
        }
        break;
      case 1:
        {
        Lexp Equations = new TLexp;
        Equations.Appendz( ex );
        int CommonDeg = Degs[0];
        for( PExMemb q = Equations.First(); !q.isNull(); q = q->m_pNext )
          {
          ex = q->m_Memb;
          eq = P.AnyExpr( ex.WriteE() );
          bool Move;
          bool PowMake = Balance( eq, Move );
          if( !PowMake )
            ex1 = ex;
          else
            {
            if( !expOut.Eq( ex ) )
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
            ex1.Clear();
            }
          int RepeatCount = 0;
          bool ParentContinue = false;
          while( PowMake && !ParentContinue )
            {
            RepeatCount++;
            if( RepeatCount > 3 ) break;
            ex2 = P.OutPut( eq ).Reduce();
            if( Move )
              TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
            ex2.Binar( '=', op1, op2 );
            int MultCount;
            Lexp MultList = GetMults( ( op1 - op2 ).Reduce(), MultCount );
            if( MultCount > 1 )
              {
              for( PExMemb f = MultList.First(); !f.isNull(); f = f->m_pNext )
                Equations.Appendz( new TBinar( '=', f->m_Memb, Constant( 0 ) ) );
              ParentContinue = true;
              continue;
              }
            MathExpr left = op1 ^ Constant( CommonDeg );
            MathExpr right = op2 ^ Constant( CommonDeg );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', left, right ) );
            left = ExpandExpr( left );
            right = ExpandExpr( right );
            ex2 = new TBinar( '=', left, right );
            ex1 = ex2.Reduce();
            if( !ThereAreRadicals( P.AnyExpr( ex1.WriteE() ) ) )
              {
              bOtherTask = true;
              break;
              }
            TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
            if( ex1.Equal( ex2 ) )
              {
              ex2 = left - right;
              ex3 = ex2.Reduce();
              if( !ex3.Eq( ex2 ) )
                ex1 = new TBinar( '=', ex3, Constant( 0 ) );
              }
            eq = P.AnyExpr( ex1.WriteE() );
            if( DefDeg1( eq ) == 2 )
              {
              if( NotOnlyInRad( eq ) )
                throw  ErrParser( "NoSolvType", peNoSolvType );
              MathExpr r1 = P.OutPut( ARoots[1] );
              MathExpr r2 = P.OutPut( ARoots[2] );
              if( ( r1 ^ Constant( 2 ) ).Reduce().Equal( r2 ) )
                Subst = r1;
              else
                if( ( r2 ^ Constant( 2 ) ).Reduce().Equal( r1 ) )
                  Subst = r2;
                else
                  throw  ErrParser( "NoSolvType", peNoSolvType );
              ex = ex1;
              Subst.Root_( arg, op2, Degs[1] );
              jj = 1;
              return Label2();
              }
            if( Degs[0] != CommonDeg )
              CommonDeg = Degs[0];
            PowMake = Balance( eq, Move );
            if( !PowMake ) break;
            if( !Move )
              TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
            }
          if( ParentContinue ) continue;
          s_FinalComment = false;
          s_Calculations = false;
          Roots.Appendz( FractRatEq( ex1.WriteE(), "x", false ) );
          s_FinalComment = true;
          s_Calculations = true;
          }
        }
        break;
      case 2:
        if( Degs[1] > Degs[2] )
          if( Degs[1] / Degs[2] == 2 && Degs[1] % Degs[2] == 0 )
            jj = 1;
          else
            throw  ErrParser( "NoSolvType", peNoSolvType );
        else
          if( Degs[2] / Degs[1] == 2 && Degs[2] % Degs[1] == 0 )
            jj = 2;
          else
            throw  ErrParser( "NoSolvType", peNoSolvType );
        arg = P.OutPut( ARoots[0]->m_pLeft );
        for( int i = 1; i < ARoots.count(); i++ )
          if( !arg.Equal( P.OutPut( ARoots[i]->m_pLeft ) ) )
            throw  ErrParser( "NoSolvType", peNoSolvType );
        if( NotOnlyInRad( eq ) )
          throw  ErrParser( "NoSolvType", peNoSolvType );
        return Label3();
      case 3:
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions!" ) );
        bOtherTask = true;
        break;
      default:
        throw  ErrParser( "NoSolvType", peNoSolvType );
      }
    return PostCase();
    }
  catch( ErrParser E )
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    Result.Clear();
    s_PutAnswer = bPutAnswerOld;
    }
  return Final();
  }

  /*
TExpr* SysInEq(const MathExpr& ex, double& a1, double& b1, int& MinType, int& MaxType) 
{


MathExpr syst,syst0;
TL2exp syst1, Solv;
TExMemb f;
MathExpr op1,op2, b;
char RelSign, LastMinSign, LastMaxSign;
TExprs a;
int OldDegPoly, i;
int nom, den;
double v, MinV,MaxV ;
QByteArray Res;
int MinBound, MaxBound;

OldDegPoly=DegPoly;
DegPoly=3;
Solv= new TL2exp;
syst0=ex; 
Solv.Addexp(ex);
Solv.Last.Visi=false;
 if( ! ex.Syst_(syst) ) 
syst=ex;
 if( ! syst.List2ex(f) ) 
{ 
ex= new TL2exp; 
(ex as TL2exp).Addexp(syst);
syst=ex;
syst0=syst;
};
MinBound = 0;
MaxBound = 0;
LastMinSign = "<";
LastMaxSign = ">";
MinV = 0;
MaxV = 0;
syst1= new TL2exp;
f=(syst as TL2exp).First;
while( f!=nullptr ) { 

 if( ! f->m_Memb.Binar_(RelSign,op1,op2) )
throw  ErrParser( "Syntax error", peSyntacs);
ex=(( op1 ) - ( op2 ));

ReductionPoly(ex,a,"x");

for( i=2 to DegPoly )
 if( ! (a[i].Constan(v) && (abs(v)<0.0000001)) )
throw  ErrParser( "Wrong type of equation!", peNoSolvType);

 if( (a[1].Constan(v) && (abs(v)<0.0000001)) )
throw  ErrParser( "Wrong type of equation!", peNoSolvType);

 if( ! (a[0].Constan(v) || a[0].SimpleFrac_(nom,den)) )
throw  ErrParser( "Syntax error", peSyntacs);

 if( ! a[1].Constan(v) )
 if( a[1].SimpleFrac_(nom,den) )
v=nom/den
else
throw  ErrParser( "Wrong type of equation!", peNoSolvType);

 if( v < 0 )
case RelSign of {
"<": RelSign = ">";
msMinequal: RelSign = msMaxequal;
">": RelSign = "<";
msMaxequal: RelSign = msMinequal;
};

b = ((  -( ( a[0] ) ) ) / ( a[1] )).Reduce();
 if( ! b.Constan(v) )
 if( b.SimpleFrac_(nom,den) )
v=nom/den;

case RelSign of {
"<", msMinequal:
{
 if( MinBound==0 )
{
LastMinSign=RelSign;
MinV = v
}
else
 if( (v<MinV) || ((v==MinV) && (RelSign=="<")) )
{
LastMinSign=RelSign;
MinV = v;
};
MinBound = 1;
};
">", msMaxequal:
{
 if( MaxBound==0 )
{
LastMaxSign=RelSign;
MaxV = v
}
else
 if( (v>MaxV) || ((v==MaxV) && (RelSign==">")) )
{
LastMaxSign=RelSign;
MaxV = v;
};
MaxBound = 1;
}
else
throw  ErrParser( "Syntax error", peSyntacs);
};

ex= new TBinar(RelSign, Variable( ( "x" ) ),b);
syst1.Addexp(ex);
syst1.Last.Visi = false;
for( i=0 to DegPoly ) a[i].Free;
f=f.Next;
};

syst= new TSyst(syst1);

 if( ! syst.Equal(syst0) )
{
Solv.Addexp(syst);
Solv.Last.Visi=false;
};
 if( (MinBound==1) && (MaxBound==1) && ((MinV<MaxV) || ((MinV==MaxV) && ((LastMinSign=="<") || (LastMaxSign==">")))) )
throw  ErrParser( "No Solutions", peNoSolv)
else
{
case 2*MinBound+MaxBound of {
1: {
Res = "x" + LastMaxSign + FloatToStr(MaxV); 
a1=MaxV;
MaxType=0;
 if( LastMaxSign==">" )
MinType=1
else
MinType=2
};
2: {
Res = "x" + LastMinSign + FloatToStr(MinV); 
b1=MinV;
MinType=0;
 if( LastMinSign=="<" )
MaxType=1
else
MaxType=2
};
3: { 
 if( LastMaxSign==">" )
MinType=1
else
MinType=2;
a1=MaxV;
 if( LastMinSign=="<" )
MaxType=1
else
MaxType=2;
b1=MinV;
 if( MaxV==MinV )
Res = "x=="+FloatToStr(MinV)
else {
case LastMaxSign of {
">": LastMaxSign = "<";
msMaxequal: LastMaxSign = msMinEqual;
};
Res = FloatToStr(MaxV)+ LastMaxSign + "x" + LastMinSign + FloatToStr(MinV)
};
};
};
ex= new TStr(Res);
Solv.Addexp(ex);
Solv.Last.Visi=false;
};
DegPoly=OldDegPoly;
Result=Solv;
};

void ChangeBasisLog(txpeditor Edt, txpoutwindow OutWin) 
{


MathExpr ex,ex1;
memory_switch=SWcalculator;
 if( Edt.Write!="" )
{
try{
ex = OutPut(AnyExpr(FullPreProcessor(Edt.Write,"x")));
TSolutionChain::sm_SolutionChain.AddExpr(ex);
ex1=CalcChangeBasisLog(ex);
TSolutionChain::sm_SolutionChain.AddExpr(ex1);
OutWin.Addcomm(X_Str("XPSimpleCalcMess","MTransToOtherBasis", "The transition to other basis is executed"));
Edt.Clear;
except
on E: ErrParser )
{
TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""));
OutWin.Addcomm(X_Str("XPSimpleCalcMess", E.MsgName, E.Message ));
}
};
};
memory_switch=SWtask;
OutWin.Show;
Edt.RefreshXPE;
};
*/

bool CalcLog1Eq( const QByteArray& Source, QByteArray VarName, int StartIndex )
  {
  MathExpr basis, InEq, arg, exTransferred;
  bool ChBasis, OutOper2, InOper1, LogInExp;
  Lexp syst1, syst2, Roots;
  PExMemb f;
  int SystCount1, SystCount2;
  double v;
  MathExpArray basis0, arg0;
  Lexp Points, Signs, Intervals;
  Lexp Cond = new TLexp();
  TSolutionChain::sm_SolutionChain.Clear();

  std::function<MathExpr( const MathExpr& )> Exponent = [&] ( const MathExpr& ex )
    {
    MathExpr op1, op2;
    if( ex.Log( op1, op2 ) ) return op2.Reduce();
    if( ex.Summa( op1, op2 ) ) return Exponent( op1 ) * Exponent( op2 );
    if( ex.Subtr( op1, op2 ) ) return Exponent( op1 ) / Exponent( op2 );
    int N;
    if( ex.Multp( op1, op2 ) && op1.Cons_int( N ) && IsConstType( TLog, op2 ) ) return Exponent( op2 ) ^ op1;
    if( ex.Unarminus( op1 ) )
      {
      if( IsConstType( TLog, op1 ) ) return Constant( 1 ) / Exponent( op1 );
      if( IsConstType( TVariable, op1 ) || IsConstType( TSimpleFrac, op1 ) ) return basis^ex;
      throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      }
    if( ex.ConstExpr() || IsConstType( TVariable, ex ) ) return basis ^ ex;
    throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    };

  std::function<MathExpr( MathExpr& )> LogProp = [&] ( MathExpr& ex )
    {
    MathExpr op1, op2, op3, op4;
    if( ex.Log( op1, op2 ) )
      {
      if( op2.Multp( op3, op4 ) ) return ( op1.Log( op3 ) + op1.Log( op4 ) ).Reduce();
      if( op2.Divis( op3, op4 ) ) return ( op1.Log( op3 ) - op1.Log( op4 ) ).Reduce();
      if( IsConstType( TFunc, ex ) ) return op1.Log( op2 );
      return ex;
      }
    char c;
    if( ex.Oper_( c, op1, op2 ) )
      {
      MathExpr ex1 = LogProp( op1 );
      MathExpr ex2 = LogProp( op2 );
      switch( c )
        {
        case '+':
          return ex1 + ex2;
        case '-':
          return ex1 - ex2;
        case '*':
          return ex1 * ex2;
        case '/':
          return ex1 / ex2;
        case '^':
        case   '~':
          return ex1 ^ ex2;
        case '=':
          return MathExpr( new TBinar( '=', ex1, ex2 ) );
        default:
          return MathExpr( new TBinar( c, ex1, ex2 ) );
        }
      }
    if( ex.Unarminus( op1 ) ) return -LogProp( op1 );
    return ex;
    };

  std::function<MathExpr( MathExpr& )> ReLog = [&] ( MathExpr& ex )
    {
    MathExpr op1, op2;
    if( ex.Log( op1, op2 ) )
      {
      if( IsType( TPowr, op2 ) ) return ex;
      return op1.Log( op2.ReduceToMult() );
      }
    char c;
    if( ex.Oper_( c, op1, op2 ) )
      {
      MathExpr ex1 = ReLog( op1 );
      MathExpr ex2 = ReLog( op2 );
      switch( c )
        {
        case '+':
          return ex1 + ex2;
        case '-':
          return ex1 - ex2;
        case '*':
          return ex1 * ex2;
        case '/':
          return ex1 / ex2;
        case '^':
        case   '~':
          return ex1 ^ ex2;
        case '=':
          return MathExpr( new TBinar( '=', ex1, ex2 ) );
        default:
          return MathExpr( new TBinar( c, ex1, ex2 ) );
        }
      }
    if( ex.Unarminus( op1 ) ) return -LogProp( op1 );
    return ex;
    };

  std::function<MathExpr( MathExpr& )> ChangeBasis = [&] ( MathExpr& ex )
    {
    MathExpr op1, op2;
    if( ex.Log( op1, op2 ) )
      {
      if( !basis.Equal( op1 ) )
        {
        ChBasis = true;
        if( ( basis * op1 ).Reduce().Constan( v ) && abs( v - 1 ) < 0.0000001 )
          {
          op1 = basis;
          MathExpr op3, op4;
          if( op2.Divis( op3, op4 ) ) return op1.Log( op4 / op3 );
          return op1.Log( Constant( 1 ) / op2 );
          }
        return basis.Log( op2 ) / basis.Log( op1 );
        }
      return ex;
      }
    char c;
    if( ex.Oper_( c, op1, op2 ) )
      {
      MathExpr ex1 = ChangeBasis( op1 );
      MathExpr ex2 = ChangeBasis( op2 );
      switch( c )
        {
        case '+':
          return ex1 + ex2;
        case '-':
          return ex1 - ex2;
        case '*':
          return ex1 * ex2;
        case '/':
          return ex1 / ex2;
        case '^':
        case   '~':
          return ex1 ^ ex2;
        case '=':
          return MathExpr( new TBinar( '=', ex1, ex2 ) );
        default:
          return MathExpr( new TBinar( c, ex1, ex2 ) );
        }
      }
    if( ex.Unarminus( op1 ) ) return -ChangeBasis( op1 );
    return ex;
    };

  std::function<void( const MathExpr& )> FindSystems = [&] ( const MathExpr& ex )
    {
    auto SystCreate1 = [&] ( const MathExpr& Op, char smb, int iConst )
      {
      v = 2;
      int  N, D;
      MathExpr op = Op;
      if( !( op.Constan( v ) || op.SimpleFrac_( N, D ) ) )
        {
        MathExpr op1, op2, op3, op4;
        int rt;
        if( op.Root_( op1, op2, rt ) || ( op.Power( op1, op2 ) && op2.Divis( op3, op4 ) ) )
          op = op1;
        InEq = new TBinar( smb, op, Constant( iConst ) );
        if( !syst1.FindEqual( InEq, f ) )
          {
          syst1.Addexp( InEq );
          syst1.Last()->m_Visi = false;
          SystCount1++;
          }
        }
      else
        if( v <= 0 || ( op.SimpleFrac_( N, D ) && N <= 0 ) )
          throw  ErrParser( "No Solutions!", peNoSolv );
      };

    auto SystCreate2 = [&] ( const MathExpr& op )
      {
      int  N, D;
      v = 2;
      if( !( op.Constan( v ) || op.SimpleFrac_( N, D ) ) )
        {
        if( !syst2.FindEqual( op, f ) )
          {
          syst2.Addexp( op );
          syst2.Last()->m_Visi = false;
          SystCount2++;
          }
        }
      else
        if( v == 1 || ( op.SimpleFrac_( N, D ) && N / D == 1 ) )
          throw  ErrParser( X_Str( "MBasisLog", "basis>0 && basis!=1" ), peNewErr );
      };

    MathExpr op1, op2;
    bool OldNoRootReduce;
    char c;
    if( ex.Log( op1, op2 ) && ( !op1.ConstExpr() || !op2.ConstExpr() ) )
      {
      OldNoRootReduce = s_NoRootReduce;
      s_NoRootReduce = false;
      op1 = op1.Reduce();
      op2 = op2.Reduce();
      s_NoRootReduce = OldNoRootReduce;
      SystCreate1( op1, '>', 0 );
      SystCreate1( op1, msNotequal, 1 );
      SystCreate2( op1 );
      SystCreate1( op2, '>', 0 );
      FindSystems( op2 );
      }
    else
      if( ex.Oper_( c, op1, op2 ) )
        {
        FindSystems( op1 );
        FindSystems( op2 );
        }
      else
        if( ex.Unarminus( op1 ) )
          FindSystems( op1 );
    };

  std::function<void( const MathExpr& )> FindBasisArg = [&] ( const MathExpr& ex )
    {
    MathExpr op1, op2, op3, op4;
    char c;
    if( ex.Log( op1, op2 ) )
      {
      int i = 0;
      for( ; i < basis0.count() && !basis0[i].Equal( op1 ) && ( !op1.Power( op3, op4 ) || !basis0[i].Equal( op3 ) ); i++ );
      if( i == basis0.count() ) basis0.append( op1 );
      i = 0;
      for( ; i < arg0.count() && !arg0[i].Equal( op2 ); i++ );
      if( i == arg0.count() ) arg0.append( op2 );
      }
    else
      if( ex.Oper_( c, op1, op2 ) )
        {
        OutOper2 = OutOper2 || c == '^' || ( In( c, "*/" ) && !op1.Constan( v ) && !op2.Constan( v ) );
        FindBasisArg( op1 );
        FindBasisArg( op2 );
        }
      else
        if( ex.Unarminus( op1 ) )
          FindBasisArg( op1 );
    };

  auto CheckRoot = [&] ( double x )
    {
    double v;
    PExMemb f;
    int n;
    for( n = 0, f = Points.First(); !f.isNull(); n++, f = f->m_pNext )
      {
      int Nom, Denom;
      if( !f->m_Memb.Constan( v ) )
        {
        f->m_Memb.SimpleFrac_( Nom, Denom );
        v = Nom / ( double ) Denom;
        }
      if( abs( x - v ) < 0.0000001 || x < v ) break;
      }
    int i = 0;
    if( n < SystCount1 && abs( x - v ) < 0.0000001 )
      for( f = Signs.First(); i < n; i++, f = f->m_pNext );
    else
      for( f = Intervals.First(); i < n; i++, f = f->m_pNext );
    bool Result;
    f->m_Memb.Boolean_( Result );
    return Result;
    };

  auto NewBasis = [&] ()
    {
    switch( basis0.count() )
      {
      case 0: return MathExpr();
      case 1: return basis0[0];
      }
    TLexp ConstList;
    MathExpr op1, op2;
    int C;
    char op;
    for( int i = 0; i < basis0.count(); i++ )
      if( basis0[i].Cons_int( C ) || ( basis0[i].Oper_( op, op1, op2 ) && ( op1.Cons_int( C ) || op2.Cons_int( C ) ) ) )
        ConstList.Addexp( Constant( C ) );
    MathExpr Result = ConstList.FindGreatestCommDivisor();
    if( Result.Cons_int( C ) && C == 1 ) return basis0[0];
    return Result;
    };

  auto ArgAnaliz = [&] ()
    {
    InOper1 = false;
    if( arg0.count() < 2 ) return;
    MathExpr op1, op2;
    char c;
    for( int i = 0; i < arg0.count(); i++ )
      if( arg0[i].Oper_( c, op1, op2 ) )
        InOper1 = InOper1 || In( c, "+-" );
    };

  int n, SCount, nRoots;
  MathExpr Root;
  std::function<bool( MathExpr )> Solution = [&] ( MathExpr ex )
    {
    int iVal;
    bool NewEquation;
    MathExpArray Factors;
    std::function<bool( const MathExpr& )> FindLogPower = [&] ( const MathExpr& ex )
      {
      MathExpr exp1;
      if( ex.Unarminus( exp1 ) ) return FindLogPower( exp1 );
      bool Result = false;
      MathExpr exLeft, exRight;
      char cOper;
      if( ex.Oper_( cOper, exLeft, exRight ) )
        {
        if( cOper == '^' )
          {
          FindLogPower( exLeft );
          LogInExp = true;
          Result = FindLogPower( exRight );
          LogInExp = false;
          return Result;
          }
        return FindLogPower( exLeft ) || FindLogPower( exRight );
        }
      MathExpr exp2;
      if( ex.Log( exp1, exp2 ) )
        {
        if( basis.IsEmpty() )
          basis = exp1;
        else
          if( !basis.Eq( exp1 ) )
            throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        if( arg.IsEmpty() || arg.ConstExpr() )
          arg = exp2;
        else
          if( !exp2.ConstExpr() && !arg.Eq( exp2 ) )
            {
            NewEquation = true;
            return Result;
            }
        Result = LogInExp;
        }
      return Result;
      };

    std::function<bool( const MathExpr& )> FindLog = [&] ( const MathExpr& Exp )
      {
      MathExpr exLeft, exRight;
      if( Exp.Log( exLeft, exRight ) ) return true;
      MathExpr exp( Exp );
      if( exp.Unarminus( exp ) )
        return FindLog( exp );
      char cOper;
      if( exp.Oper_( cOper, exLeft, exRight ) )
        return FindLog( exLeft ) || FindLog( exRight );
      return false;
      };

    auto ChangeArg = [&] ( MathExpr& ex )
      {
      int iArg, iVal, iX;
      bool IsUnknown;
      int ArgExp[10], Args[10];
      auto CalcExp = [&] ()
        {
        for( int i = 0; i < arg0.count(); i++ )
          {
          iVal = iArg;
          ArgExp[i] = 1;
          while( iVal < Args[i] )
            {
            iVal *= iArg;
            ArgExp[i]++;
            }
          if( iVal != Args[i] ) return false;
          }
        return true;
        };

      std::function<void( MathExpr& )> ReplaceLog = [&] ( MathExpr& exp )
        {
        auto GetLog = [&] ()
          {
          for( int i = 0; i < arg0.count(); i++ )
            if( Args[i] == iVal )
              {
              if( ArgExp[i] == 1 )
                return basis0[0].Log( Constant( iArg ) );
              return Constant( ArgExp[i] ) * basis0[0].Log( Constant( iArg ) );
              }
          return MathExpr();
          };

        MathExpr exArg;
        if( exp.Unarminus( exArg ) )
          {
          ReplaceLog( exArg );
          return;
          }

        MathExpr exLeft, exRight, exBase;
        char cOper;
        if( exp.Oper_( cOper, exLeft, exRight ) )
          {
          MathExpr &Left = CastPtr( TOper, exp )->Left();
          if( Left.Log( exBase, exArg ) )
            {
            exArg.Cons_int( iVal );
            exp.Replace( Left, GetLog() );
            }
          else
            ReplaceLog( exLeft );
          MathExpr &Right = CastPtr( TOper, exp )->Right();
          if( Right.Log( exBase, exArg ) )
            {
            exArg.Cons_int( iVal );
            exp.Replace( Right, GetLog() );
            }
          else
            ReplaceLog( exRight );
          }
        };

      if( basis0.count() == 0 ) return;
      IsUnknown = !basis0[0].HasUnknown( VarName ).isEmpty();
      if( IsUnknown && basis0.count() > 1 )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      for( int i = 1; i < basis0.count(); i++ )
        if( !basis0[i].HasUnknown( VarName ).isEmpty() )
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      if( !IsUnknown || arg0.count() == 1 ) return;
      for( int i = 0; i < arg0.count(); i++ )
        if( !arg0[i].Cons_int( Args[i] ) )
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        else
          if( i == 0 )
            iArg = Args[0];
          else
            iArg = std::min( iArg, Args[i] );
      if( iArg <= 0 )
        throw  ErrParser( X_Str( "MArgLgLess0", "Logarithm(x) is defined while( x>0!" ), peNewErr );
      if( iArg == 1 )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      if( !CalcExp() )
        {
        iVal = 1;
        for( int i = 2; i <= round( sqrt( iArg ) ); i++ )
          {
          iVal = i;
          while( iVal < iArg )
            iVal = iVal * i;
          iX = i;
          if( iVal == iArg ) break;
          };
        if( iVal != iArg )
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        iArg = iX;
        if( !CalcExp() )
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        }
      ReplaceLog( ex );
      };

    std::function<bool( const MathExpr& )> HasConstOperand = [&] ( const MathExpr& ex )
      {
      MathExpr exLeft, exRight;
      char cOper;
      if( ex.Oper_( cOper, exLeft, exRight ) && In( cOper, "+-" ) )
        {
        if( exLeft.ConstExpr() || HasConstOperand( exLeft ) ) return true;
        return exRight.ConstExpr() || HasConstOperand( exRight );
        }
      return false;
      };

    std::function<void( const MathExpr& )> UndefIsNoSquare = [&] ( const MathExpr& ex )
      {
      if( !s_NoLogSquarReduce ) return;

      MathExpr exArg;
      if( ex.Unarminus( exArg ) )
        {
        UndefIsNoSquare( exArg );
        return;
        }

      MathExpr exLeft, exRight;
      char cOper;
      QByteArray Name;
      if( ex.Oper_( cOper, exLeft, exRight ) )
        {
        if( cOper == '^' )
          if( exLeft.Variab( Name ) && Name == VarName )
            {
            int iVal;
            s_NoLogSquarReduce = exRight.Cons_int( iVal ) && iVal % 2 == 0;
            return;
            }
        UndefIsNoSquare( exLeft );
        UndefIsNoSquare( exRight );
        return;
        }

      MathExpr exBase;
      if( ex.Log( exBase, exArg ) )
        {
        UndefIsNoSquare( exBase );
        UndefIsNoSquare( exArg );
        return;
        }
      if( ex.Variab( Name ) && Name == VarName )
        s_NoLogSquarReduce = false;
      };

    std::function<bool( const MathExpr& )> FactorsList = [&] ( const MathExpr& ex )
      {
      MathExpr FactorBase, FactorArg;

      std::function<bool( const MathExpr& )> GetFactor = [&] ( const MathExpr& exp )
        {
        MathExpr exArg;
        if( exp.Unarminus( exArg ) ) return GetFactor( exArg );

        MathExpr exLeft, exRight, exBase;
        char cOper;
        if( exp.Oper_( cOper, exLeft, exRight ) )
          return GetFactor( exLeft ) && GetFactor( exRight );
        if( exp.Log( exBase, exArg ) )
          {
          if( FactorBase.IsEmpty() )
            {
            FactorBase = exBase;
            FactorArg = exArg;
            return true;
            }
          return !FactorBase.Eq( exBase ) && !FactorArg.Eq( exArg );
          }
        return true;
        };

      MathExpr exLeft, exRight;
      if( ex.Multp( exLeft, exRight ) ) return FactorsList( exLeft ) && FactorsList( exRight );
      FactorBase.Clear();
      if( !GetFactor( ex ) || FactorBase.IsEmpty() ) return false;
      Factors.append( ex );
      return true;
      };

    std::function<void( MathExpr& )> BaseReduction = [&] ( MathExpr& ex )
      {
      MathExpr exRight, exLeft, exArg, exBase;
      char cOper;

      auto GetOperand = [&] ( MathExpr& exp )
        {
        MathExpr exBase, exArg, exPowBase, exExp;
        MathExpr Result = exp;
        if( exp.Log( exBase, exArg ) )
          {
          if( exArg.Eq( arg ) )
            {
            if( basis.IsEmpty() )
              {
              basis = exBase;
              if( exBase.Power( exPowBase, exExp ) ) basis = exPowBase;
              }
            if( exBase.Eq( basis ) ) return Result;
            if( exBase.Power( exPowBase, exExp ) ) return ( basis.Log( exArg ) / exExp ).Reduce();
            return basis.Log( exArg ) / basis.Log( exBase );
            }
          }
        BaseReduction( exp );
        return Result;
        };

      if( ex.Unarminus( arg ) )
        {
        BaseReduction( arg );
        return;
        }
      if( ex.Oper_( cOper, exLeft, exRight ) )
        {
        MathExpr New = GetOperand( exLeft );
        ex.Replace( exLeft, New );
        New = GetOperand( exRight );
        ex.Replace( exRight, New );
        }
      };

    s_ReductionMustBeContinued = false;
    s_SmartReduceLogArg = true;
    MathExpr ex1 = ex.Reduce();
    if( !ex.Equal( ex1 ) )
      {
      ex = ex1;
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }
    if( s_ReductionMustBeContinued )
      {
      ex = ex.Reduce();
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }

    s_SmartReduceLogArg = false;
    ex1 = RemDenominator( ex, Cond );
    arg = ex;
    if( !ex.Equal( ex1 ) )
      {
      ex = ex1.Reduce();
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }

    MathExpr op1, op2, Op11, Op12, ex2;
    ex.Binar( '=', op1, op2 );
    if( op1.Power( Op11, Op12 ) && Op11.HasUnknown( VarName ).isEmpty() && Op12.Log( ex1, ex2 ) && Op12.HasUnknown( VarName ).isEmpty() )
      {
      ex1 = new TBinar( '=', ex1.Log( Op11 ) * Op12, ex1.Log( op2 ) );
      ex = ex1;
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      }

    if( CastPtr( TOper, ex )->Right()->ConstExpr() || HasConstOperand( CastPtr( TOper, ex )->Left() ) )
      {
      ex1 = Transfer( ex );
      ex1.Binar( '=', op1, op2 );
      MathExpr op3;
      if( op2.Unarminus( op3 ) )
        ex1 = new TBinar( '=', -op1.Reduce(), op3 );
      if( !ex.Eq( ex1 ) )
        {
        ex = ex1;
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        }
      exTransferred = ex.Reduce();
      ex = exTransferred;
      }
    else
      exTransferred = ex;
    bool Result = true;
    try
      {
      if( syst2.Count() != 0 )
        for( f = syst2.First(); !f.isNull(); f = f->m_pNext )
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, f->m_Memb, Constant( 1 ) ) );
      ex1 = ReLog( ex );
      if( !ex1.Equal( ex ) )
        {
        ex2 = ex1.Reduce();
        if( !ex2.Equal( ex1 ) )
          {
          ex2.Binar( '=', op1, op2 );
          MathExpr ex3 = op1 - op2;
          MathExpr ex4 = ex3.Reduce();
          if( !ex4.Equal( ex3 ) )
            {
            TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
            ex = new TBinar( '=', ex4, Constant( 0 ) );
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            }
          }
        }
      OutOper2 = false;
      FindBasisArg( ex );
      basis = NewBasis();
      if( basis.IsEmpty() )
        {
        s_FinalComment = false;
        s_Calculations = false;
        try
          {
          Roots = FractRatEq( ex.WriteE(), VarName );
          }
        catch( ErrParser )
          {
          throw;
          }
        s_FinalComment = true;
        s_Calculations = true;
        }
      else
        {
        ArgAnaliz();
        if( OutOper2 && InOper1 )
          {
          ex.Binar( '=', op1, op2 );
          Factors.clear();
          if( op2.Cons_int( iVal ) && iVal == 0 )
            {
            if( FactorsList( op1 ) && Factors.count() > 1 )
              {
              Result = true;
              for( int I = 0; I < Factors.count(); I++ )
                Result = Solution( new TBinar( '=', Factors[I], Constant( 0 ) ) ) && Result;
              return Result;
              }
            TSolvReToMult Solv;
            Solv.SetExpression( op1 );
            ex1 = Solv.Result();
            if( ex1.Binar( '=', op1, op2 ) )
              {
              ex = new TBinar( '=', op2, Constant( 0 ) );
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              return Solution( ex );
              }
            if( arg0.count() > 0 )
              {
              for( int I = 0; I < arg0.count(); I++ )
                {
                arg = arg0[I];
                basis.Clear();
                BaseReduction( ex );
                }
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              return Solution( ex );
              }
            }
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
          }
        ChBasis = false;
        ex1 = ChangeBasis( ex );
        if( ChBasis )
          {
          TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
          ex = ex1.Reduce();
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        else
          ex.Binar( '=', op1, op2 );
        MathExpr Op21, Op22;
        if( op1.Multp( Op21, Op22 ) && Op21.ConstExpr() && op2.ConstExpr() )
          {
          op2 /= Op21;
          ex1 = new TBinar( '=', Op22, op2.Reduce() );
          ex = ex1;
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          ex.Binar( '=', op1, op2 );
          }
        bool Simple;
        MathExpr op3, op4, op5, op6;
        int rt;
        if( op1.Unarminus( ex1 ) )
          {
          Simple = ex1.Log( op3, op4 ) && !op4.Root_( op5, op6, rt ) && op2.ConstExpr();
          op1 = ex1;
          op2 = -op2;
          ex = new TBinar( '=', op1, op2 );
          }
        else
          Simple = op1.Log( op3, op4 ) && !op4.Root_( op5, op6, rt ) && op2.ConstExpr();
        if( !OutOper2 && InOper1 || Simple )
          {
          ex.Binar( '=', op1, op2 );
          MathExpr right0 = op2;
          MathExpr left = Exponent( op1 );
          MathExpr right = Exponent( op2 );

          if( op2.ConstExpr() )
            ex1 = new TBinar( '=', basis.Log( left ), right0 );
          else
            ex1 = new TBinar( '=', basis.Log( left ), basis.Log( right ) );
          if( !ex1.Equal( ex ) )
            {
            TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
            }
          ex = new TBinar( '=', left, right );

          if( FindLog( left ) )
            {
            TSolutionChain::sm_SolutionChain.AddExpr( ex );
            op3 = right.Reduce();
            if( !op3.Equal( right ) )
              {
              ex1 = new TBinar( '=', left, op3 );
              ex = ex1;
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              }
            return Solution( ex );
            }

          s_FinalComment = false;
          s_Calculations = false;
          if( CheckUnknownInExp( ex, VarName ) )
            {
            bool OldPutAnswer = s_PutAnswer;
            s_PutAnswer = true;
            if( CalcExpEq( ex.WriteE() ) )
              {
              Roots = new TL2exp;
              for( PExMemb f = CastPtr( TLexp, s_Answer )->First(); !f.isNull(); f = f->m_pNext )
                {
                f->m_Memb.Binar( '=', left, right );
                Roots.Addexp( right );
                }
              }
            else
              throw  ErrParser( "Wrong type of equation!", peNoSolvType );
            s_PutAnswer = OldPutAnswer;
            }
          else
            Roots = FractRatEq( ex.WriteE(), VarName );
          s_FinalComment = true;
          s_Calculations = true;
          }
        else
          {
          ex.Binar( '=', op1, op2 );
          if( op1.Log( Op11, Op12 ) && op2.Log( Op21, Op22 ) && Op11.Eq( Op21 ) )
            {
            ex = new TBinar( '=', Op12, Op22 );
            Roots = new TL2exp;
            Roots.Appendz( FractRatEq( ex.WriteE(), VarName ) );
            }
          else
            {
            s_NoLogSquarReduce = true;
            UndefIsNoSquare( ex );
            s_IsLogEquation = false;
            ex1 = ExpandExpr( LogProp( ex ) );
            s_IsLogEquation = true;
            basis.Clear();
            arg.Clear();
            ex2 = ex1.Clone();
            FindBasisArg( ex1 );
            ChangeArg( ex1 );
            TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
            LogInExp = false;
            NewEquation = false;
            if( FindLogPower( ex1 ) )
              {
              ex1.Binar( '=', op1, op2 );
              if( op1.Power( op3, op4 ) && op3.Multp( op5, op6 ) && op5.Eq( basis ) )
                ex2 = new TBinar( '=', op4 + basis.Log( op6 ) * op4, basis.Log( op2 ) );
              else
                ex2 = new TBinar( '=', basis.Log( op1 ), basis.Log( op2 ) );
              s_ExpandLog = true;
              ex1 = Expand( ex2 );
              s_ExpandLog = false;
              TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
              }
            else
              if( NewEquation )
                {
                if( ex2.Eq( ex ) )
                  throw  ErrParser( "Wrong type of equation!", peNoSolvType );
                TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
                return Solution( ex2 );
                }
            QByteArray NewVarName;
            if( VarName == "z" )
              NewVarName = "x";
            else
              NewVarName = QByteArray( 1, VarName[0] + 1 );
            MathExpr SLog = basis.Log( arg );
            ex2 = Variable( ( NewVarName ) );
            ex1.Replace( SLog, ex2 );
            if( !ex1.HasUnknown( VarName ).isEmpty() )
              throw  ErrParser( "Wrong type of equation!", peNoSolvType );
            TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', SLog, ex2 ) );
            s_FinalComment = false;
            s_Calculations = false;
            Lexp RootsY;
            try
              {
              s_NoLogReduce = false;
              RootsY = FractRatEq( ex1.WriteE(), NewVarName );
              }
            catch( ErrParser )
              {
              throw;
              }
            s_FinalComment = true;
            s_Calculations = true;
            if( !RootsY.IsEmpty() )
              {
              n = 0;
              Roots = new TL2exp;
              for( PExMemb f = RootsY.First(); !f.isNull(); f = f->m_pNext )
                {
                n++;
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', SLog, f->m_Memb ) );
                ex = new TBinar( '=', arg, ( basis ^ f->m_Memb ).Reduce() );
                s_FinalComment = false;
                s_Calculations = false;
                try
                  {
                  Roots.Appendz( FractRatEq( ex->WriteE(), VarName ) );
                  }
                catch( ErrParser )
                  {
                  throw;
                  }
                s_FinalComment = true;
                s_Calculations = true;
                }
              }
            }
          }
        }
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MFinishRoot", "final result!" ) );
      SCount = 0;
      n = 0;
      if( s_PutAnswer )
        s_Answer = new TL2exp;
      if( !Roots.IsEmpty() )
        {
        for( PExMemb f = Roots.First(); !f.isNull(); f = f->m_pNext )
          {
          nRoots++;
          n++;
          Root = f->m_Memb.Reduce();
          bool bNoRootReduceOld = s_NoRootReduce;
          bool bNoLogReduceOld = s_NoLogReduce;
          s_NoRootReduce = false;
          s_NoLogReduce = false;
          ex1 = Root.Reduce();
          s_NoRootReduce = bNoRootReduceOld;
          s_NoLogReduce = bNoLogReduceOld;
          bool stranger = IsConstType( TComplexExpr, ex1 );
          if( !stranger )
            {
            if( !( ex1.Constan( v ) ) )
              {
              int Nom, Den;
              if( !ex1.SimpleFrac_( Nom, Den ) )
                throw  ErrParser( "Wrong type of equation!", peNoSolvType );
              v = (double) Nom / Den;
              }
            stranger = !CheckRoot( v );
            }
          Root = new TBinar( '=', Variable( VarName + "_" + NumberToStr( nRoots + StartIndex ) ), f->m_Memb );
          TSolutionChain::sm_SolutionChain.AddExpr( Root );
          if( stranger )
            SCount++;
          else
            if( !syst2.IsEmpty() )
              {
              for( PExMemb f2 = syst2.First(); !f2.isNull(); f2 = f2->m_pNext )
                {
                ex = f2->m_Memb.Substitute( VarName, Variable( VarName + "_" + NumberToStr( nRoots ) ) );
                ex1 = f2->m_Memb.Substitute( VarName, f->m_Memb ).Reduce();
                if( ex1.Constan( v ) && abs( v - 1 ) < 0.0000001 )
                  {
                  if( !f->m_Memb.Equal( Constant( 1 ) ) )
                    TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex, Constant( 1 ) ) );
                  stranger = true;
                  SCount++;
                  }
                else
                  TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, new TBinar( '=', ex, ex1 ), Constant( 1 ) ) );
                }
              }
          if( !stranger )
            {
            for( PExMemb f2 = Cond.First(); !f2.isNull(); f2 = f2->m_pNext )
              {
              ex = f2->m_Memb.Substitute( VarName, Variable( ( VarName + "_" + NumberToStr( nRoots ) ) ) );
              ex1 = f2->m_Memb.Substitute( VarName, f->m_Memb ).Reduce();
              if( ex1.Cons_int( iVal ) && ( iVal == 0 ) )
                {
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex, Constant( 0 ) ) );
                stranger = true;
                SCount++;
                }
              else
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( msNotequal, new TBinar( '=', ex, ex1 ), Constant( 0 ) ) );
              }
            }
          if( s_PutAnswer && !stranger )
            CastPtr( TL2exp, s_Answer )->Addexp( Root );
          if( stranger )
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootStranger", "false Root!" ) );
          else
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootEq", "Root of equation" ) );
          }
        }
      if( n - SCount > 1 )
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MRootsFound", "Roots are found." ) );
      }
    catch( ErrParser E )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
      Result = false;
      exTransferred.Clear();
      }
    return Result;
    };

  double OldPrecision = s_Precision;
  bool Result = false;
  bool bNoRootReduceOld = s_NoRootReduce;
  bool bNoLogReduceOLd = s_NoLogReduce;
  s_NoLogReduce = true;
  s_NoRootReduce = true;
  s_IsLogEquation = true;
  bool IsSolvInEq = false;
  MathExpr ex1, ex;
  nRoots = 0;
  if( !Source.isEmpty() )
    {
    Result = true;
    try
      {
      Parser P;
      ex = P.OutPut( P.AnyExpr( P.FullPreProcessor( Source, "x" ) ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      VarName = ex.HasUnknown();
      MathExpr op1, op2;
      if( !ex.Binar( '=', op1, op2 ) || VarName.isEmpty() )
        throw  ErrParser( X_Str( "MEnterEquat", "enter equation!" ), peNewErr );
      if( ex.Splitted() )
        ex = DelSplitted( ex );
      SystCount1 = 0;
      SystCount2 = 0;
      syst1 = new TL2exp;
      syst2 = new TL2exp;
      FindSystems( ex );
      if( SystCount1 == 0 )
        throw  ErrParser( "Wrong type of equation!", peNoSolvType );
      if( SystCount1 > 1 )
        ex1 = new TSyst( syst1 );
      else
        ex1 = syst1;
      MathExpr ex2 = SysRatInEq( ex1, SystCount1, Points, Signs, Intervals, IsSolvInEq, VarName );
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MDomain", "DOMAIN" ) );
      TSolutionChain::sm_SolutionChain.AddExpr( ex2 );
      }
    catch( ErrParser E )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
      s_FinalComment = true;
      s_Calculations = true;
      Result = E.Name() == "MNewErr" && !ex1.IsEmpty();
      }
    }
  Result = Result && Solution( ex );
  if( !Result && !exTransferred.IsEmpty() )
    {
    MathExpr op1, op2;
    exTransferred.Binar( '=', op1, op2 );
    if( op2.Cons_int( n ) && n == 0 )
      {
      MathExpr op3, op4;
      char cOper;
      op1.Oper_( cOper, op3, op4 );
      if( cOper == '+' || cOper == '-' )
        try
        {
        ex1 = op1.ReduceToMult();
        if( !ex1.Eq( op1 ) )
          {
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, op2 ) );
          op1 = ex1;
          }
        }
      catch( ErrParser )
        {
        }
      if( op1.Multp( op3, op4 ) )
        {
        ex = new TBinar( '=', op3, op2 );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        MathExpr exLeft = op4;
        MathExpr exRight = op2;
        Result = Solution( ex );
        if( !Result )
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
        ex = new TBinar( '=', exLeft, exRight );
        TSolutionChain::sm_SolutionChain.AddExpr( ex );
        if( Solution( ex ) )
          Result = true;
        else
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
        }
      else
        TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoSolution", "No Solutions!" ) );
      }
    else
      TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real Solutions!" ) );
    }
  s_NoLogReduce = bNoLogReduceOLd;
  s_NoRootReduce = bNoRootReduceOld;
  s_IsLogEquation = false;
  s_Precision = OldPrecision;
  return Result;
  }

void LogPropertiesEx( MathExpr Ex, int Prop )
  {
  auto IsLogPowerDivMul = [&]( const MathExpr& ex )
    {
    MathExpr  op21, op22;
    int N, D;
    if( ex.Multp( op21, op22 ) ) return true;
    if( ex.Divis( op21, op22 ) || ex.SimpleFrac_( N, D ) ) return true;
    if( ex.Power( op21, op22 ) ) return true;
    return false;
    };

  MathExpr logoper;
  QByteArray  Name1;
  auto CrLog = [&] ( const MathExpr& Ex )
    {
    if( !logoper.IsEmpty() ) return logoper.Log( Ex );
    return Function( Name1, Ex );
    };

  std::function<MathExpr( const MathExpr& )> Rasobr = [&]( const MathExpr& Ex )
    {
    MathExpr  op21, op22, Result;
    int N, D;
    if( Ex.Multp( op21, op22 ) )
      Result = Rasobr( op21 ) + Rasobr( op22 );
    else
      if( Ex.SimpleFrac_( N, D ) )
        Result = CrLog( Constant( N ) ) - CrLog( Constant( D ) );
      else
        if( Ex.Divis( op21, op22 ) )
          Result = Rasobr( op21 ) - Rasobr( op22 );
        else
          if( Ex.Power( op21, op22 ) ) 
            Result = op22 * Rasobr( op21 );
    if( Result.IsEmpty() ) Result = CrLog( Ex );
    return Result;
    };

  auto IsLog = [&]( const MathExpr& ex, MathExpr& op2 )
    {
    MathExpr  logoper2;
    QByteArray Name2;
    bool Result = false;
    if( ex.Log( logoper2, op2 ) && Name1.isEmpty( ) ) 
      {
      if( !logoper.IsEmpty( ) ) 
        {
        logoper = logoper2;
        return true;
        }
      if( logoper2.Equal( logoper ) ) Result = true;
      }
    if( ex.Funct( Name2, op2 ) && !logoper.IsEmpty( ) && ( Name1 == "log" || Name1 == "lg" ) && 
       ( Name1.isEmpty() || Name1 == Name2 ) ) 
      {
      Name1 = Name2;
      Result = true;
      }
    return Result;
    };

  std::function<bool( const MathExpr& )> IsLogSumSubMul = [&]( const MathExpr& ex )
    {
    MathExpr  op1, op2;
    if( ex.Summa( op1, op2 ) && IsLogSumSubMul( op1 ) && IsLogSumSubMul( op2 ) ) return true;
    if( ex.Subtr( op1, op2 ) && IsLogSumSubMul( op1 ) && IsLogSumSubMul( op2 ) ) return true;
    if( IsLog( ex, op2 ) ) return true;
    if( ex.Multp( op1, op2 ) && IsLogSumSubMul( op1 ) || IsLogSumSubMul( op2 ) ) return true;
    return false;
    };

  std::function<MathExpr( const MathExpr& )> Rasobr2 = [&]( const MathExpr& ex )
    {
    MathExpr  op1, op2, op11, op21;
    if( ex.Summa( op1, op2 ) ) 
      {
      op1 = Rasobr2( op1 );
      op2 = Rasobr2( op2 );
      if( IsLog( op1, op11 ) && IsLog( op2, op21 ) )
        return CrLog( op11 * op21 );
      }
    if( ex.Subtr( op1, op2 ) ) 
      {
      op1 = Rasobr2( op1 );
      op2 = Rasobr2( op2 );
      if( IsLog( op1, op11 ) && IsLog( op2, op21 ) )
        return CrLog( op11 / op21 );
      }

    if( ex.Multp( op1, op2 ) ) 
      {
      op1 = Rasobr2( op1 );
      op2 = Rasobr2( op2 );
      if( IsLog( op1, op11 ) ) return CrLog( op11 ^ op2 );
      if( IsLog( op2, op21 ) ) return CrLog( op21 ^ op1 );
      }
    return MathExpr(ex);
    };

  MathExpr  expr = Ex.Reduce(), op2, exInit = Ex;
  MathExpr ResExpr1, ResExpr2;
  if( expr.Equal( Ex ) || expr.ConstExpr() )
    expr = Ex;
  else 
    Ex = new TBinar( '=', Ex, expr );
  if( IsLog( expr, op2 ) && IsLogPowerDivMul( op2 ) ) ResExpr1 = Rasobr( op2 );
  if( ResExpr1.IsEmpty() && IsLogSumSubMul( expr ) && ( !Name1.isEmpty( ) || !logoper.IsEmpty() ) ) ResExpr1 = Rasobr2( expr );
  if( ( ResExpr1 == nullptr ) && IsLog( exInit, op2 ) ) ResExpr1 = expr;

  if( !ResExpr1.IsEmpty() )
    {
    if( ResExpr1.Equal( exInit ) )
      TSolutionChain::sm_SolutionChain.AddExpr( Ex );
    else
      {
      ResExpr2 = ResExpr1.Reduce();
      TSolutionChain::sm_SolutionChain.AddExpr( ResExpr1 );
      TSolutionChain::sm_SolutionChain.AddExpr( ResExpr2 );
      if( ResExpr2.Equal( ResExpr1 ) )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Ex, ResExpr1 ) );
        ResExpr2 = ResExpr1;
        }
      else TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Ex, new TBinar( '=', ResExpr1, ResExpr2 ) ) );
      }
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MCalced", "Calculated!" ) );
    }
  else
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MNotSuitableExpr", "Wrong expression!" ) );
  if( s_PutAnswer && !ResExpr2.IsEmpty( ) && s_Answer.IsEmpty( ) )
    s_Answer = ResExpr2;
  }

bool CalcExpEq( const QByteArray& Source, bool Recurs )
  {
  if( Source.isEmpty() ) return false;
  bool bWithoutLog, bNumericalBasis;
  QVector< PNode> pw;
  int expn[20];
  
  std::function<void( PNode )> PreOrder = [&] ( PNode p )
    {
    if( p == nullptr ) return;
    if( bWithoutLog && p->m_OpSign == '-' || p->m_OpSign == '+' )
      {
      bWithoutLog = false;
      for( PNode qParent = p->m_pParent; qParent != nullptr && !bWithoutLog; qParent = qParent->m_pParent )
        bWithoutLog = qParent->m_OpSign == '^';
      }
    if( p->m_OpSign == '^' )
      {
      PNode q = p->m_pParent;
      for( ; q != nullptr && q->m_OpSign != '^'; q = q->m_pParent );
      if( q == nullptr ) pw.append( p );
      }
    PreOrder( p->m_pLeft );
    PreOrder( p->m_pRight );
    };

  auto CommonBasis = [&] ( int& Nom, int& Den )
    {
    double a[20];
    for( int i = 0; i < pw.count(); i++ )
      {
      PNode b = pw[i]->m_pLeft;
      switch( b->m_OpSign )
        {
        case '/':
          if( b->m_pLeft->m_OpSign != 'i' || b->m_pRight->m_OpSign != 'i' ) return false;
          a[i] = b->m_pLeft->m_Info.toDouble() / b->m_pRight->m_Info.toDouble();
          break;
        case 'i':
        case 'f':
          a[i] = b->m_Info.toDouble();
          break;
        default:
          bNumericalBasis = false;
          return false;
        }
      }
    for( int j = 100; j >= 2; j-- )
      {
      double x = j;
      bool bb = false;
      for( int i = 0; i < pw.count() && !bb; i++ )
        {
        double z = log( a[i] ) / log( x );
        bb = abs( z - Round( z ) ) > 0.00001;
        expn[i] = Round( log( a[i] ) / log( x ) );
        }
      if( !bb )
        {
        Nom = j;
        Den = 0;
        return true;
        }
      }
    for( int j = 2; j <= 100; j++ )
      for( int k = 2; k <= 100; k++ )
        if( j != k )
          {
          double x = j;
          double y = k;
          bool bb = false;
          for( int i = 0; i < pw.count() && !bb; i++ )
            {
            double z = log( a[i] ) / log( x / y );
            bb = abs( z - Round( z ) ) > 0.00001;
            expn[i] = Round( log( a[i] ) / log( x / y ) );
            }
          if( !bb )
            {
            Nom = j;
            Den = k;
            return true;
            }
          }
    return false;
    };

  int Nom, Den;
  Parser P;
  MathExpr bas;
  auto ReduceToCommBasis = [&] ()
    {
    PNode basis, exp, p1, p2;
    if( Den == 0 ) {
      basis = NewNode( &P, NumberToStr( Nom ) );
      basis->m_OpSign = 'i';
      }
    else
      {
      p1 = NewNode( &P, NumberToStr( Nom ) );
      p1->m_OpSign = 'i';
      p2 = NewNode( &P, NumberToStr( Den ) );
      p2->m_OpSign = 'i';
      basis = NewNode( &P, "/" );
      basis->m_OpSign = '/';
      basis->m_pLeft = p1;
      basis->m_pRight = p2;
      }
    bas = P.OutPut( basis );
    exp = NewNode( &P, "" );
    exp->m_OpSign = 'i';
    p1 = NewNode( &P, "*" );
    p1->m_OpSign = '*';
    for( int i = 0; i < pw.count(); i++ )
      {
      pw[i]->m_pLeft = Clone( basis );
      if( expn[i] == -1 )
        {
        p2 = Clone( p1 );
        p2->m_OpSign = 'u';
        p2->m_pRight = pw[i]->m_pRight;
        pw[i]->m_pRight = p2;
        }
      else
        if( expn[i] != 1 )
          {
          exp->m_Info = NumberToStr( expn[i] );
          p2 = Clone( p1 );
          p2->m_pLeft = Clone( exp );
          p2->m_pRight = pw[i]->m_pRight;
          pw[i]->m_pRight = p2;
          }
      }
    };

  QByteArray VarName;
  MathExpArray pw1;
  int deg[20];
  std::function<MathExpr( const MathExpr&, MathExpr& )> PowToMult = [&] ( const MathExpr& ex, MathExpr& Res )
    {
    MathExpr op1, op2, Result;
    TExprs a;
    if( ex.Power( op1, op2 ) )
      {
      double v;
      if( !( op2.Diff().Reduce().Diff().Reduce().Diff().Reduce().Constan( v ) && abs( v ) < 0.0000001 ) )
        throw  1;
      MathExpr ex1 = op2.ReductionPoly( a, VarName );
      MathExpr diff = ( ex1 - a[0] ).Reduce();
      if( diff.Constan( v ) && abs( v ) < 0.0000001 )
        {
        Res = ex;
        return MathExpr( ex );
        }
      int j = 0;
      for( ; j < pw1.count() && !diff.Equal( pw1[j] ); j++ );
      if( j == pw1.count() )
        {
        pw1.append( diff );
        int k = s_DegPoly;
        for( ; a[k].Constan( v ) && abs( v ) < 0.0000001; k-- );
        deg[pw1.count() - 1] = k;
        }
      if( !( a[0].Constan( v ) && abs( v ) < 0.0000001 ) )
        {
        Result = ( op1 ^ a[0] ) * ( op1 ^ diff );
        Res = ( op1 ^ a[0] ) * Variable( "z" + NumberToStr( j ) );
        }
      else
        {
        Result = ex;
        Res = Variable( "z" + NumberToStr( j ) );
        }
      }
    else
      {
      char c;
      MathExpr res1, res2;
      if( ex.Oper_( c, op1, op2 ) )
        {
        MathExpr ex1 = PowToMult( op1, res1 );
        MathExpr ex2 = PowToMult( op2, res2 );
        switch( c )
          {
          case '+':
            Result = ex1 + ex2;
            Res = res1 + res2;
            break;
          case '-':
            Result = ex1 - ex2;
            Res = res1 - res2;
            break;
          case '*':
            Result = ex1 * ex2;
            Res = res1 * res2;
            break;
          case '/':
            Result = ex1 / ex2;
            Res = res1 / res2;
            break;
          case '=':
            Result = new TBinar( '=', ex1, ex2 );
            Res = new TBinar( '=', res1, res2 );
            break;
          default:
            Result = new TBinar( c, ex1, ex2 );
            Res = new TBinar( c, res1, res2 );
          }
        }
      else
        if( ex.Unarminus( op1 ) )
          {
          Result = -PowToMult( op1, res1 );
          Res = -res1;
          }
        else
          {
          Result = ex;
          Res = ex;
          }
      }
    return Result;
    };

  QByteArray Name;
  auto Solve = [&] ( const MathExpr& ex, const MathExpr& exp )
    {
    MathExpr ex1;
    if( exp.Variab( Name ) )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      ex1 = ex.Reduce();
      if( s_PutAnswer )
        CastPtr( TL2exp, s_Answer )->Addexp( ex1 );
      if( !ex1.Eq( ex ) )
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      }
    else
      {
      QByteArray sExp = ex.WriteE();
      s_FinalComment = false;
      switch( deg[0] )
        {
        case 1:
          ex1 = CalcDetLinEqu( sExp );
          break;
        case 2:
          {
          bool InterRes = TSolutionChain::sm_InterimResult;
          TSolutionChain::sm_InterimResult = true;
          CalcDetQuEqu( sExp );
          TSolutionChain::sm_InterimResult = InterRes;
          }
        }
      if( s_PutAnswer && !ex1.IsEmpty() )
        CastPtr( TL2exp, s_Answer )->Appendz( ex1 );
      s_FinalComment = true;
      }
    };

  std::function<MathExpr( const MathExpr& )> MultPowReduce = [&] ( const MathExpr& ex )
    {
    MathExpr ex1, ex2, op1, op2;
    MathExpr op11, op12, op21, op22;
    char c;
    if( ex.Oper_( c, op1, op2 ) )
      {
      ex1 = MultPowReduce( op1 );
      ex2 = MultPowReduce( op2 );
      switch( c )
        {
        case '^': return ex1 ^ ex2;
        case '~': return MathExpr( ex );
        case '+': return ex1 + ex2;
        case '-': return ex1 - ex2;
        case '*':
          if( ex1.Power( op11, op12 ) && ex2.Power( op21, op22 ) && op12.Equal( op22 ) )
            return ( ( op11 * op21 ) ^ op22 ).Reduce();
          return  ex1 * ex2;
        case '/': return ex1 / ex2;
        case '=': return MathExpr( new TBinar( '=', ex1, ex2 ) );
        default:
          return MathExpr( new TBinar( c, ex1, ex2 ) );
        }
      }
    else
      if( ex.Unarminus( op1 ) )
        return -MultPowReduce( op1 );
    return MathExpr( ex );
    };

  std::function<MathExpr( const MathExpr&, const MathExpr& )> GetEquation = [&] ( const MathExpr& ex, const MathExpr& exOld )
    {
    MathExpr exLeft, exRight;
    if( ex.Power( exLeft, exRight ) && !( IsConstType( TRoot, ex ) ) )
      {
      if( exOld.IsEmpty() ) return exRight;
      return exOld + exRight;
      };
    if( !ex.Multp( exLeft, exRight ) )
      throw  ErrParser( "Syntax Error!", peSyntacs );
    return GetEquation( exRight, GetEquation( exLeft, exOld ) );
    };

  std::function<bool( MathExpr& )> ReplaceFractions = [&] ( MathExpr& ex )
    {
    MathExpr exLeft, exRight, exNom, exDenom;
    char cOper;
    int Nom, Denom;
    bool Result = true;
    if( ex.Oper_( cOper, exLeft, exRight ) && cOper != '~' && cOper != '^' )
      {
      if( exLeft.Divis( exNom, exDenom ) )
        CastPtr( TOper, ex )->Left() = exNom * ( exDenom ^ Constant( -1 ) ).Reduce();
      else
        if( exLeft.SimpleFrac_( Nom, Denom ) && Nom == 1 )
          CastPtr( TOper, ex )->Left() = Constant( Denom ) ^ Constant( -1 );
        else
          Result = ReplaceFractions( exLeft );
      if( exRight.Divis( exNom, exDenom ) )
        {
        CastPtr( TOper, ex )->Right() = exNom * ( exDenom ^ Constant( -1 ) ).Reduce();
        Result = true;
        }
      else
        if( exRight.SimpleFrac_( Nom, Denom ) && Nom == 1 )
          {
          CastPtr( TOper, ex )->Right() = Constant( Denom ) ^ Constant( -1 );
          return true;
          }
        else
          Result = ReplaceFractions( exRight ) || Result;
      return Result;
      }
    return false;
    };

  std::function<bool( MathExpr& )> RootToPower = [&] ( MathExpr& ex )
    {
    MathExpr exLeft, exRight, exBase, exPower;
    int root;
    char cOper;
    bool Result = true;
    if( ex.Oper_( cOper, exLeft, exRight ) )
      {
      if( exLeft.Root1_( exBase, exPower ) )
        {
        RootToPower( exPower );
        CastPtr( TOper, ex )->Left() = exBase ^ ( Constant( 1 ) / exPower );
        }
      else
        if( exLeft.Root_( exBase, exPower, root ) )
          CastPtr( TOper, ex )->Left() = exBase ^ exPower;
        else
          Result = RootToPower( exLeft );
      if( exRight.Root1_( exBase, exPower ) )
        {
        RootToPower( exPower );
        if( cOper == '^' )
          CastPtr( TOper, ex )->Right() = ( exBase ^ exPower ).Reduce();
        else
          CastPtr( TOper, ex )->Right() = exBase ^ ( Constant( 1 ) / exPower );
        }
      else
        if( cOper != '^' && cOper != '~' && exRight.Root_( exBase, exPower, root ) )
          {
          CastPtr( TOper, ex )->Right() = exBase ^ exPower;
          Result = true;
          }
        else
          Result = RootToPower( exRight ) || Result;
      return Result;
      };
    return false;
    };

  bool bNewSystem, bWasSumSub;
  std::function<MathExpr( MathExpr& )> SummAndSub = [&] ( MathExpr& ex )
    {
    if( bNewSystem ) return ex;
    MathExpr exLeft, exRight, exBase, exPower, ex1;
    char cOper;
    MathExpr Result = ex;
    if( ex.Oper_( cOper, exLeft, exRight ) )
      {
      if( cOper == '-' || cOper == '+' )
        {
        CastPtr( TOper, ex )->Left() = SummAndSub( exLeft );
        CastPtr( TOper, ex )->Right() = SummAndSub( exRight );
        s_PowerToMult = true;
        ex.SetReduced( false );
        Result = ex.Reduce();
        s_PowerToMult = false;
        bWasSumSub = true;
        bNewSystem = Result.Oper_( cOper, exLeft, exRight ) && ( cOper == '+' || cOper == '-' );
        return Result;
        }
      if( cOper == '=' )
        return MathExpr( new TBinar( '=', SummAndSub( exLeft ), SummAndSub( exRight ) ) );
      if( cOper == '*' )
        {
        ex1 = ex.Reduce();
        if( ex1.Eq( ex ) ) return ex1;
        return SummAndSub( ex1 );
        }
      }
    return ex;
    };

  std::function<MathExpr( const MathExpr& )> GetExponent = [&] ( const MathExpr& ex )
    {
    MathExpr exLeft, exRight, exp;
    char cOper;
    if( ex.Oper_( cOper, exLeft, exRight ) )
      {
      if( cOper == '^' ) return MathExpr( ex );
      if( cOper == '*' )
        {
        exp = GetExponent( exLeft );
        MathExpr Result = GetExponent( exRight );
        if( Result.IsEmpty() ) return exp;
        if( !exp.IsEmpty() ) return MathExpr();
        return Result;
        }
      }
    return MathExpr();
    };

  std::function<bool( MathExpr& )> IsExponentialEquation = [&] ( MathExpr& ex )
    {
    MathExpr exLeft, exRight, exLeftOld, exRightOld;
    char cOper;
    QByteArray Name;
    bool Result = false;
    if( ex.Funct( Name, exRight ) )
      {
      Result = Name == "exp" && !exRight.HasUnknown().isEmpty();
      if( Result )
        {
        exLeft = Variable( "e" ) ^ exRight;
        ex = exLeft;
        }
      return Result;
      }
    if( !ex.Oper_( cOper, exLeft, exRight ) ) return Result;
    switch( cOper )
      {
      case '^':
      case '~':
        return !exRight.HasUnknown().isEmpty();
      default:
        exLeftOld = exLeft;
        exRightOld = exRight;
        Result = IsExponentialEquation( exLeft ) || IsExponentialEquation( exRight );
        if( exLeftOld != exLeft )
          CastPtr( TOper, ex )->Left() = exLeft;
        if( exRightOld != exRight )
          CastPtr( TOper, ex )->Right() = exRight;
      }
    return Result;
    };

  MathExpr exBaseRight;
  auto TransferBase = [&] ( const MathExpr& ex )
    {
    struct TTerm
      {
      int m_Sign;
      MathExpr m_Ex;
      MathExpr m_Base;
      TTerm() : m_Sign( 0 ) {}
      TTerm( int sign, MathExpr ex, MathExpr base ) : m_Sign( sign ), m_Ex( ex ), m_Base( base ) {}
      TTerm( const TTerm& T ) : m_Sign( T.m_Sign ), m_Ex( T.m_Ex ), m_Base( T.m_Base ) {}
      };

    QVector< TTerm> Terms;
    int TermCount, TermsSize, sign, i;
    MathExpr exBaseLeft, exLeft, exRight;

    std::function<void( MathExpr& )> SearchBase = [&] ( MathExpr& ex )
      {
      char cOper;
      MathExpr exLeft, exRight, exBase, exPower;
      int val;

      auto AddTerm = [&] ( const MathExpr& exTerm, MathExpr exBase )
        {
        if( !exBase.IsEmpty() )
          if( exBaseLeft.IsEmpty() )
            exBaseLeft = exBase;
          else
            if( exBaseLeft.Eq( exBase ) )
              exBase = exBaseLeft;
            else
              if( exBaseRight.IsEmpty() )
                exBaseRight = exBase;
              else
                if( exBaseRight.Eq( exBase ) )
                  exBase = exBaseRight;
                else
                  throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        if( exTerm.Cons_int( val ) && val == 0 ) return;
        Terms.append( TTerm( sign, exTerm, exBase ) );
        };

      if( !ex.Oper_( cOper, exLeft, exRight ) )
        {
        if( ex.Unarminus( exLeft ) )
          {
          sign = -sign;
          SearchBase( exLeft );
          sign = -sign;
          }
        else
          AddTerm( ex, nullptr );
        return;
        }
      switch( cOper )
        {
        case '+':
          SearchBase( exLeft );
          SearchBase( exRight );
          break;
        case '-':
          SearchBase( exLeft );
          sign = -sign;
          SearchBase( exRight );
          sign = -sign;
          break;
        case '^':
          AddTerm( ex, exLeft );
          break;
        case '*':
          if( !exLeft.Power( exBase, exPower ) && !exRight.Power( exBase, exPower ) )
            throw  ErrParser( "Wrong type of equation!", peNoSolvType );
          AddTerm( ex, exBase );
          break;
        case  '~': return;
        default:
          throw  ErrParser( "Wrong type of equation!", peNoSolvType );
        }
      };
    ex.Binar( '=', exLeft, exRight );
    sign = 1;
    exBaseRight.Clear();
    exBaseLeft.Clear();
    SearchBase( exLeft );
    sign = -1;
    SearchBase( exRight );
    if( Terms.count() < 3 || exBaseRight.IsEmpty() ) return MathExpr( ex );
    exLeft.Clear();
    exRight.Clear();
    for( int i = 0; i < Terms.count(); i++ )
      if( Terms[i].m_Base == exBaseLeft )
        if( exLeft.IsEmpty() )
          if( Terms[i].m_Sign == 1 )
            exLeft = Terms[i].m_Ex;
          else
            exLeft = -Terms[i].m_Ex;
        else
          if( Terms[i].m_Sign == 1 )
            exLeft += Terms[i].m_Ex;
          else
            exLeft -= Terms[i].m_Ex;
      else
        if( exRight.IsEmpty() )
          if( Terms[i].m_Sign == -1 )
            exRight = Terms[i].m_Ex;
          else
            exRight = -Terms[i].m_Ex;
        else
          if( Terms[i].m_Sign == -1 )
            exRight += Terms[i].m_Ex;
          else
            exRight -= Terms[i].m_Ex;
    return MathExpr( new TBinar( '=', exLeft.Reduce(), exRight.Reduce() ) );
    };

  std::function<void( MathExpr& )> MultToCommonBasis = [&] ( MathExpr& ex )
    {
    MathExpr exLeft, exRight, exBase, exPower;
    char cOper;

    auto GetPower = [&] ( const MathExpr& exp )
      {
      int pwr;
      MathExpr Tmp, Log;
      MathExpr Result = exp;
      if( exp.Eq( bas ) )
        return bas ^ 1;
      if( !bas.ConstExpr() || !exp.ConstExpr() ) return Result;
      Log = bas.Log( exp );
      Tmp = Log.Reduce();
      if( Tmp.Cons_int( pwr ) ) return bas ^ Tmp;
      return Result;
      };

    if( ex.Oper_( cOper, exLeft, exRight ) )
      {
      if( cOper == '*' )
        {
        if( !( IsConstType( TPowr, exLeft ) ) )
          CastPtr( TMult, ex )->Left() = GetPower( CastPtr( TMult, ex )->Left() );
        else
          if( !( IsConstType( TPowr, exRight ) ) )
            CastPtr( TMult, ex )->Right() = GetPower( CastPtr( TMult, ex )->Right() );
        return;
        }
      if( cOper == '=' )
        {
        MultToCommonBasis( exLeft );
        MultToCommonBasis( exRight );
        }
      }
    };

  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  int OldDegPoly = s_DegPoly;
  s_DegPoly = 3;
  bool CalcOnlyOld = s_CalcOnly;
  bool OldNoRootReduce = s_NoRootReduce;
  bool Result = false;
  auto Final = [&] ()
    {
    s_CalcOnly = CalcOnlyOld;
    s_NoRootReduce = OldNoRootReduce;
    s_Precision = OldPrecision;
    s_DegPoly = OldDegPoly;
    return Result;
    };

  auto ErrResult = [&] ()
    {
    s_CalcOnly = CalcOnlyOld;
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( "MNotSuitableEq", "Wrong type of equation!" ) );
    Result = false;
    return Final();
    };

  MathExpr ex;
  auto Label3 = [&] ()
    {
    MathExpr op1, op2;
    if( ex.Binar( '=', op1, op2 ) )
      {
      MathExpr op11, op12, op21, op22;
      if( op1.Power( op11, op12 ) && op2.Power( op21, op22 ) )
        {
        if( op12.Equal( op22 ) || op12.Equal( -op22.Reduce() ) )
          {
          double v;
          if( !( op12.Diff().Reduce().Diff().Reduce().Diff().Reduce().Constan( v ) && abs( v ) < 0.0000001 ) )
            return ErrResult();
          MathExpr ex1 = new TBinar( '=', op12, Constant( 0 ) );
          deg[0] = DegOfPoly( op12, VarName );
          Solve( ex1, op12 );
          TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
          }
        }
      else
        {
        if( pw.count() == 0 ) return ErrResult();
        bas = Constant( pw[0]->m_pLeft->m_Info.toInt() );
        MathExpr ex1 = bas.Log( op1 );
        TSolutionChain::sm_SolutionChain.Clear();
        s_CalcOnly = true;
        LogPropertiesEx( ex1, 1 );
        if( TSolutionChain::sm_SolutionChain.Count() == 0 ) return ErrResult();
        ex1 = TSolutionChain::sm_SolutionChain.GetExpr( 0 );
        MathExpr ex2 = TSolutionChain::sm_SolutionChain.GetLastExpr();
        MathExpr ex3 = bas.Log( op2 );
        TSolutionChain::sm_SolutionChain.Clear();
        LogPropertiesEx( ex3, 1 );
        if( TSolutionChain::sm_SolutionChain.Count() == 0 ) return ErrResult();
        s_CalcOnly = CalcOnlyOld;
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, TSolutionChain::sm_SolutionChain.GetExpr( 0 ) ) );
        ex1 = new TBinar( '=', ex2, TSolutionChain::sm_SolutionChain.GetLastExpr() );
        Result = EquationsSolving( ex1.WriteE(), VarName );
        if( !Result ) return ErrResult();
        }
      }
    else
      return ErrResult();
    return Final();
    };
  try
    {
    PNode eq = P.AnyExpr( P.FullPreProcessor( Source, "x" ) );
    MathExpr ex0 = P.OutPut( eq );
    TSolutionChain::sm_SolutionChain.AddExpr( ex0, X_Str( "MExponentialEquation", " Exponential equation" ), false);
    ex = ex0.Reduce();
    if(ex != ex0) TSolutionChain::sm_SolutionChain.AddExpr( ex, "", false);
    VarName = ex.HasUnknown();
    MathExpr op1, op2;
    MathExpArray a;
    if( !ex.Binar( '=', op1, op2 ) || !IsExponentialEquation( ex ) || VarName.isEmpty() )
      throw  ErrParser( "Wrong type of equation!", peNoSolvType );
    TSolutionChain::sm_SolutionChain.AddExpr( ex.Clone() );
    if( s_PutAnswer ) s_Answer = new TL2exp;
    Result = true;
    bNewSystem = false;
    bWasSumSub = false;
    s_NoRootReduce = true;
    MathExpr ex1 = TransferBase( ex ), ex2;
    if( !ex.Eq( ex1 ) )
      {
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
      ex = ex1;
      }
    if( exBaseRight.IsEmpty() )
      {
      try
        {
        MathExpr op11, op12, ex2;
        if( op1.Power( op11, op12 ) && op11.HasUnknown() == VarName && op12.Log( ex1, ex2 ) && op12.HasUnknown() == VarName )
          {
          ex1 = new TBinar( '=', ex1.Log( op11 ) * op12, ex1.Log( op2 ) );
          Result = CalcLog1Eq( ex1.WriteE(), VarName );
          }
        else
          {
          ex1 = new TBinar( '=', GetEquation( op1, nullptr ), GetEquation( op2, nullptr ) );
          s_DegPoly = OldDegPoly;
          Result = EquationsSolving( ex1.WriteE(), VarName );
          }
        }
      catch( ErrParser )
        {
        Result = false;
        }
       if( Result ) return Final();
      Result = true;
      }
    ex1 = SummAndSub( ex );
    if( bWasSumSub && !bNewSystem )
      TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
    else
      {
      s_ReductionMustBeContinued = false;
      ex1 = ex.Reduce();
      if( s_ReductionMustBeContinued )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
        ex1 = ex1.Reduce();
        }
      if( !IsExponentialEquation( ex1 ) )
        {
        s_DegPoly = OldDegPoly;
        Result = EquationsSolving( ex1.WriteE(), VarName );
        return Final();
        }

      bNewSystem = ReplaceFractions( ex1 );
      if( RootToPower( ex1 ) ) 
        {
        ex2 = ex1.Reduce();
        ex1 = ex2;
        bNewSystem = true;
        }
      }

    ex2 = MultPowReduce( ex1 );
    ex1 = ex2;

    ex1.Binar( '=', op1, op2 );
    ex2 = GetExponent( op1 );
    MathExpr ex3 = GetExponent( op2 );
    if( !ex2.IsEmpty() && !ex3.IsEmpty() && CastPtr( TPowr, ex2 )->Right().Eq( CastPtr( TPowr, ex3 )->Right() ) )
      {
      CastPtr( TPowr, ex2 )->Left() = CastPtr( TPowr, ex2 )->Left() / CastPtr( TPowr, ex3 )->Left();
      ex2 = new TBinar( '=', op1.Reduce(), ( op2 / ex3 ).Reduce() );
      ex1 = ex2;
      ex1.Binar( '=', op1, op2 );
      }

    if( op1.HasUnknown().isEmpty() )
      {
      ex2 = CastPtr(TBinar, ex1 )->Left();
      CastPtr( TBinar, ex1 )->Left() = CastPtr( TBinar, ex1 )->Right();
      CastPtr( TBinar, ex1 )->Right() = ex2;
      ex1.Binar( '=', op1, op2 );
      }
    if( op2.HasUnknown().isEmpty() && op1.Multp( ex2, ex3 ) )
      if( ex2.HasUnknown().isEmpty() )
        ex1 = new TBinar( '=', ex3, ( op2 / ex2 ).Reduce() );
      else
        if( ex3.HasUnknown().isEmpty() )
        ex1 = new TBinar( '=', ex2, ( op2 / ex3 ).Reduce() );
    if( !ex1.Eq( ex ) )
      {
      ex = ex1;
      ex.Binar( '=', op1, op2 );
      TSolutionChain::sm_SolutionChain.AddExpr( ex );
      bNewSystem = true;
      }
    bWithoutLog = true;
    if( op1.ConstExpr() ) 
      {
      ex1 = op1;
      op1 = op2;
      op2 = ex1;
      bNewSystem = true;
      }

    if( op1.Negative() )
      if( op2.Negative() ) 
        {
        bNewSystem = true;
        ex = new TBinar( '=', -op1.Reduce(), -op2.Reduce() );
        }
      else bWithoutLog = false;
    else
    if( op2.Unarminus( ex1 ) ) bWithoutLog = false;
    s_NoRootReduce = OldNoRootReduce;
    char cOper;
    if( op2.Cons_int( Nom ) )
      if( Nom == 0 )
        if( !op1.Oper_( cOper, ex1, ex2 ) || cOper != '-' )
          bWithoutLog = false;
        else 
          {
          ex = new TBinar( '=', ex1, ex2 );
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          bNewSystem = true;
          }
      else
        {
        ex = new TBinar( '=', op1, op2 ^ 1 );
        bNewSystem = true;
        }

    if( op2.SimpleFrac_( Nom, Den ) )
      {
      ex = new TBinar( '=', op1, op2 ^ 1 );
      bNewSystem = true;
      }

    if( bNewSystem ) 
      {
      QByteArray S = ex.WriteE();
      while( true )
        {
        int i = S.indexOf("(e)" );
        if( i == -1 ) break;
        S = S.left( i ) + NumberToStr( exp( 1 ) ) + S.mid( i + 3 );
        }
      eq = P.AnyExpr( S );
      }

    PreOrder( eq );
    bNumericalBasis = true;
    bool bCommonBasis = CommonBasis( Nom, Den );
    if( !bCommonBasis )
      {
      ex.Binar( '=', op1, op2 );
      MathExpr ex4;
      if( op1.Power( ex1, ex2 ) && op2.Power( ex3, ex4 ) )
        {
        bCommonBasis = ex1.Equal( ex3 );
        if( bCommonBasis ) MultToCommonBasis( ex );
        if( Recurs && !bCommonBasis )
          {
          if( ex2.Equal( ex4 ) )
            {
            s_DegPoly = OldDegPoly;
            if( !bNumericalBasis )
              Result = EquationsSolving( TBinar( '=', ex1, ex3 ).WriteE(), VarName );
            Result = EquationsSolving( TBinar( '=', ex2, Constant( 0 ) ).WriteE(), VarName ) || Result;
            }
          else
            {
            if( !bNumericalBasis ) return ErrResult();
            if( !ex2.HasUnknown().isEmpty() )
              {
              if( ( ex2 / ex4 ).Divisor2Polinoms().Binar( '=', ex3, ex4 ) && ex4.ConstExpr() )
                Result = EquationsSolving( TBinar( '=', ex2, Constant( 0 ) ).WriteE(), VarName );
              else
                {
                ex1 = new TBinar( '=', ex2, ex1.Log( op2.Reduce() ) );
                TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
                ex1 = ex1.Reduce();
                int ResultCount = TSolutionChain::sm_SolutionChain.Count();
                bool OldToFraction = TConstant::sm_ConstToFraction;
                TConstant::sm_ConstToFraction = false;
                Result = EquationsSolving( ex1.WriteE(), VarName );
                TConstant::sm_ConstToFraction = OldToFraction;
                if( TSolutionChain::sm_SolutionChain.Count() == ResultCount )
                  TSolutionChain::sm_SolutionChain.AddExpr( ex1, X_Str( "MFinishRoot", "final result!" ) );
                }
              }
            else
              Result = EquationsSolving( TBinar( '=', ex4, ex3.Log( op1.Reduce() ) ).WriteE(), VarName );
            }
          }
        bNumericalBasis = false;
        }
      }
    if( bCommonBasis )
      {
      if( bNumericalBasis )
        {
        ReduceToCommBasis();
        ex1 = P.OutPut( eq );
        MultToCommonBasis( ex1 );
        if( !ex1.Equal( ex ) )
          {
          ex = ex1;
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        }
      if( bWithoutLog ) 
        {
        ex.Binar( '=', op1, op2 );
        try
          {
          ex = new TBinar( '=', GetEquation( op1, nullptr ), GetEquation( op2, nullptr ) );
          if( Recurs ) Result = EquationsSolving( ex.WriteE(), VarName );
          }
        catch(ErrParser )
          {
          bWithoutLog = false;
          }
        }
      if( !bWithoutLog )
        {

        pw1.clear();
        bool Error;
        try
          {
          ex1 = PowToMult( ex, ex2 );
          Error = false;
          }
        catch( ErrParser )
          {
          Error = true;
          }
        if( Error ) return Label3();
        if( !ex1.Equal( ex ) )
          {
          ex = ex1;
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        ex1 = ex2.Reduce();
        bool solv;
        if( ex1.Boolean_( solv ) && solv )
          {
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Constant( 0 ), Constant( 0 ) ), 
            X_Str( "MInfiniteSetSol", "Infinite set of solutions" ) );
          return Final();
          }
        ex2 = ex1;
        for( int i = 0; i < pw1.count(); i++ )
          {
          ex3 = ex1.Substitute( "z" + NumberToStr( i ), bas ^ pw1[i] );
          ex1 = ex3.Reduce();
          }
        if( !ex1.Equal( ex ) )
          {
          ex = ex1;
          TSolutionChain::sm_SolutionChain.AddExpr( ex );
          }
        if( pw1.count() > 3 ) return ErrResult();
        ex2.Binar( '=', op1, op2 );
        ex3 = op1 - op2;
        if( pw1.count() == 3 )
          {
          int k1, k2;
          if( ( pw1[1] - pw1[0] ).Reduce().Equal( ( pw1[0] - pw1[2] ).Reduce() ) ) k2 = 1;
          else
            if( ( pw1[0] - pw1[1] ).Reduce().Equal( ( pw1[1] - pw1[2] ).Reduce() ) ) k2 = 2;
            else
              if( ( pw1[0] - pw1[2] ).Reduce().Equal( ( pw1[2] - pw1[1] ).Reduce() ) ) k2 = 3;
          else 
            k2 = 0;
          if( k2 == 0 ) return ErrResult();
          MathExpr ex4 = ex3.Substitute( "z1", Variable( "z0" ) ).Substitute( "z2", Variable( "z0" ) );
          ex4.ReductionPoly( a, "z0" );
          double v, v1, v2, v3;
          if( !( a[0].Constan( v ) && abs( v ) < 0.0000001 ) ) return ErrResult();
          a.clear();
          pw1[0].ReductionPoly( a, VarName );
          a[deg[0]].Constan( v1 );
          a.clear();
          pw1[1].ReductionPoly( a, VarName );
          a[deg[0]].Constan( v2 );
          a.clear();
          pw1[2].ReductionPoly( a, VarName );
          a[deg[0]].Constan( v3 );
          a.clear();
          switch( k2 )
            {
            case 1:  
              if( v2 < v3 )
                k1 = 2;
              else 
                k1 = 3;
              break;
            case 2:  
              if( v1 < v3 )
                k1 = 1;
              else 
                k1 = 3;
              break;
            default:  
              if( v1 < v2 )
                k1 = 1;
              else 
                k1 = 2;
            }
          ex4 = ex3.Substitute( "z" + NumberToStr( k1 - 1 ), Constant( 1 ) ).Reduce();
          ex3 = ex4;
          switch(k1)
            {
            case 1: 
              pw1[1] = ( pw1[1] - pw1[0] ).Reduce();
              pw1[0] = ( pw1[2] - pw1[0] ).Reduce();
              ex4 = ex3.Substitute( "z2", Variable( "z0" ) );
              ex3 = ex4;
              break;
            case 2: 
              pw1[0] = ( pw1[0] - pw1[1] ).Reduce();
              pw1[1] = ( pw1[2] - pw1[1] ).Reduce();
              ex4 = ex3.Substitute( "z2", Variable( ( "z1" ) ) );
              ex3 = ex4;
              break;
            case 3: 
              pw1[0] = ( pw1[0] - pw1[2] ).Reduce();
              pw1[1] = ( pw1[1] - pw1[2] ).Reduce();
            }
          pw1.resize(2);
          ex1 = ex3;
          for( int i = 0; i < pw1.count(); i++ )
            {
            ex2 = ex1.Substitute( "z" + NumberToStr( i ), bas ^ pw1[i] );
            ex1 = ex2;
            }
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex1, Constant( 0 ) ) );
          }

        MathExpr exx = ex3;
        double v;
        switch( pw1.count() )
          {
          case 1: 
            ex3.ReductionPoly( a, "z0" );
            ex2 = -( a[0] / a[1] ).Reduce();
            if( ex2.Constan( v ) && v < 0.0000001 )
              throw  ErrParser( "No Solutions!", peNoSolv );
            ex3 = bas.Log( ex2 );
            ex1 = new TBinar( '=', pw1[0], ex3 );
            if( !ex1.Eq( ex ) )
              TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
            ex = new TBinar( '=', pw1[0], ex3.Reduce() );
            Solve( ex, pw1[0] );
            TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
            a.clear();
            break;
          case 2: 
            {
            ex3.ReductionPoly( a, "z0" );
            MathExpr a1 = a[1];
            a.clear();
            ex3.ReductionPoly( a, "z1" );
            MathExpr b1 = a[1];
            a.clear();
            MathExpr ex4 = ex3.Substitute( "z1", Variable( "z0" ) );
            ex3 = ex4;
            ex3.ReductionPoly( a, "z0" );
            MathExpr c1 = a[0];
            a.clear();
            if( c1.Constan( v ) && abs( v ) < 0.0000001 )
              {
              ex1 = bas ^ pw1[0];
              if( a1.Constan( v ) && abs( v - 1 ) < 0.0000001 )
                ex3 = ex1;
              else
                if( a1.Constan( v ) && abs( v + 1 ) < 0.0000001 )
                  ex3 = -ex1;
              else
              ex3 = a1 * ex1;
              ex1 = bas ^ pw1[1];
              if( b1.Constan( v ) && abs( v - 1 ) < 0.0000001 )
                ex4 = -ex1;
              else
                if( b1.Constan( v ) && abs( v + 1 ) < 0.0000001 )
                  ex4 = ex1;
              else
              ex4 = -b1.Reduce() * ex1;
              ex = new TBinar( '=', ex3, ex4 );
              TSolutionChain::sm_SolutionChain.AddExpr( ex );
              if( a1.Constan( v ) && abs( v - 1 ) < 0.0000001 && b1.Constan( v ) && abs( v + 1 ) < 0.0000001 )
                {
                ex = new TBinar( '=', pw1[0], pw1[1] );
                ex4 = Constant( 0 );
                }
              else 
                {
                ex2 = -( b1 / a1 ).Reduce();
                ex = new TBinar( '=', ( bas ^ pw1[0]  ) / ( bas ^ pw1[1] ), ex2 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex );
                ex3 = pw1[0] - pw1[1];
                ex = new TBinar( '=', bas ^ ex3, ex2 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex );
                if( ex2.Constan( v ) && ( v < 0.0000001 ) )
                  throw  ErrParser( "No Solutions!", peNoSolv );
                ex4 = ex3.Reduce();
                if( !ex4.Eq( ex3 ) )
                  {
                  ex = new TBinar( '=', bas ^ ex4, ex2 );
                  TSolutionChain::sm_SolutionChain.AddExpr( ex );
                  }
                if( ex2.Power( op1, op2 ) && op1.Equal( bas ) )
                  ex = new TBinar( '=', ex4, op2 );
                else
                ex = new TBinar( '=', ex4, bas.Log( ex2 ) );
                }
              Solve( ex, ex4 );
              TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
              }
            else 
              {
              int z;
              if( deg[0] != deg[1] ) return ErrResult();
              if( ( pw1[0] + pw1[1] ).Reduce().Equal( Constant( 0 ) ) )
                {
                pw1[0].ReductionPoly( a, VarName );
                double v1;
                a[deg[0]].Constan( v1 );
                a.clear();
                if( v1 > 0 )
                  {
                  z = 2;
                  pw1[1] = pw1[0];
                  pw1[0] = ( Constant( 2 ) * pw1[0] ).Reduce();
                  exx = ( a1 * Variable( "z0" ) +  b1 + c1 * Variable( "z1" ) ).Reduce();
                  }
                else 
                  {
                  z = 2;
                  pw1[0] = ( Constant( 2 ) * pw1[1] ).Reduce();
                  exx = ( a1 + b1 * Variable( "z0" ) + c1 * Variable( "z1" ) ).Reduce();
                  }
                ex1 = exx.Substitute( "z0", bas ^ pw1[0] );
                ex2 = ex1.Substitute( "z1", bas ^ pw1[1] );
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex2, Constant( 0 ) ) );
                }
              else
                {
                pw1[0].ReductionPoly( a, VarName );
                MathExpArray b;
                pw1[1].ReductionPoly( b, VarName );
                a[deg[0]].Constan( v );
                double v1;
                b[deg[0]].Constan( v1 );
                if( abs( 2 * v - v1 ) < 0.0000001 )
                  z = 1;
                else
                  if( abs( 2 * v1 - v ) < 0.0000001 )
                    z = 2;
                  else return ErrResult();
                  for( int i = deg[0] - 1; i >= 0; i-- )
                    {
                    a[i].Constan( v );
                    b[i].Constan( v1 );
                    switch( z )
                      {
                      case 1:
                        if( abs( 2 * v - v1 ) > 0.0000001 ) return ErrResult();
                        break;
                      case 2:  if( abs( 2 * v1 - v ) > 0.0000001 ) return ErrResult();
                      }
                    }
                }
              MathExpr y = Variable( "y" );
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', y, bas ^ pw1[z-1] ) );
              switch( z )
                {
                case 1:
                  ex1 = exx.Substitute( "z0", y );
                  ex2 = ex1.Substitute( "z1", y ^ 2 );
                  break;
                case 2:
                  ex1 = exx.Substitute( "z1", y );
                  ex2 = ex1.Substitute( "z0", y ^ 2 );
                }
              a.clear();
              ex3 = ex2.ReductionPoly( a, "y" );
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', ex3, Constant( 0 ) ) );
              MathExpr d = ( ( a[1] ^ 2 ) - Constant( 4 ) * a[2] * a[0] ).Reduce();
              TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( "d" ), d ) );
              if( d.Constan( v ) && ( v < 0 ) )
                throw  ErrParser( "No Solution", peNoSolv );
              if( d.Constan( v ) && abs( v ) < 0.0000001 )
                {
                MathExpr y1 = ( -a[1] / ( Constant( 2 ) * a[2] ) ).Reduce();
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( "y_1" ), y1 ) );
                ex1 = new TBinar( '=', bas ^ pw1[z-1], y1 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
                if( y1.Constan( v ) && v < 0.0000001 )
                  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions" ) );
                else 
                  {
                  ex = new TBinar( '=', pw1[z-1], new TLog( bas, y1 ) );
                  Solve( ex, pw1[z-1] );
                  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
                  }
                }
              else 
                {
                d = d.Root( 2 );
                ex1 = Constant( 2 ) * a[2];
                MathExpr y1 = ( ( -a[1] - d  ) / ex1 ).Reduce();
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( "y_1" ), y1 ) );
                MathExpr y2 = ( ( -a[1] + d ) / ex1 ).Reduce();
                TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '=', Variable( "y_2" ), y2 ) );
                ex1 = new TBinar( '=', bas ^ pw1[z-1], y1 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
                solv = false;
                if( y1.Constan( v ) && v < 0.0000001 )
                  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions!" ) );
                else
                  solv = CalcExpEq( ex1.WriteE() );
                ex1 = new TBinar( '=', bas ^ pw1[z-1], y2 );
                TSolutionChain::sm_SolutionChain.AddExpr( ex1 );
                if( y2.Constan( v ) && v < 0.0000001 )
                  TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MNoRealSolutions", "No real solutions!" ) );
                else
                  {
                  solv = CalcExpEq( ex1.WriteE() );
                  if( solv )
                    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MRootsFound", "Roots are found." ) );
                  }
                }
              }
            }
          }
        }
      }
    else
      return Label3();
    }
  catch( ErrParser E )
    {
    s_CalcOnly = CalcOnlyOld;
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ), X_Str( E.Name(), E.Message() ) );
    s_FinalComment = true;
    Result = false;
    if( s_PutAnswer && E.ErrStatus() != peNoSolv )
      s_Answer.Clear();
    }
  return Final();
  }

/*
TExpr* CalcChangeBasisLog( const MathExpr& ex )
  {


  TExMemb f;
  MathExpr log, arg, NewBasis, OldBasis;
  MathExpr Nom, Den;
  int N, D;
  double r;

  if( !( ex.Listex( f ) || ex.List2ex( f ) ) )
    throw  ErrParser( "Syntax error!", peSyntacs );
  log = f->m_Memb;
  f = f.Next;
  NewBasis = f->m_Memb;
  f = f.Next;

  if( !f.isNull() )
    throw  ErrParser( "Syntax error!", peSyntacs );

  if( !log.Log( OldBasis, arg ) )
    throw  ErrParser( "Syntax error", peSyntacs );

  Den = ( ( NewBasis ).Log( OldBasis ) ).Reduce();

  if( Den.SimpleFrac_( N, D ) )
    {
    if( N == 1 )
      Result = ( ( NewBasis ).Log( ( ( arg ) ^ ( Constant( ( D ) ) ) ).Reduce() ) )
    else
    Result = ( ( NewBasis ).Log( ( ( arg ) ^ ( new TSimpleFrac( D, N ) ) ).Reduce() ) );
    exit;
    };

  if( Den.Cons_int( N ) )
    {
    if( N == 1 )
      Result = ( ( NewBasis ).Log( arg ) )
    else
    Result = ( ( NewBasis ).Log( ( ( arg ) ^ ( new TSimpleFrac( 1, N ) ) ).Reduce() ) );
    exit;
    };

  if( !( arg.Constan( r ) || arg.SimpleFrac_( N, D ) ) )
    Nom = ( ( NewBasis ).Log( arg ) ).Reduce()
  else
  Nom = ( ( NewBasis ).Log( arg ) );

  if( Den.Constan( r ) )
    Result = ( ( Constant( ( 1 / r ) ) ) * ( Nom ) )
  else
  Result = ( ( Nom ) / ( Den ) )
  }

/*
void LogPropertiesEx( const MathExpr& Ex, txpeditor Edt, txpoutwindow OutWin, int Prop )
  {
  QByteArray  Name1;
  MathExpr logoper;

  bool IsLogPowerDivMul( const MathExpr& ex )
    {
    MathExpr  op21, op22;
    int N, D;
    Result = false;
    if( ex.Multp( op21, op22 ) ) Result = true;
    if( ex.Divis( op21, op22 ) || ex.SimpleFrac_( N, D ) ) Result = true;
    if( ex.Power( op21, op22 ) ) Result = true;
    };

  TExpr* CrLog( const MathExpr& Ex )
    {
    if( logoper != nullptr ) Result = ( ( logoper ).Log( Ex ) ) else
      Result = Function( ( Name1 ), ( Ex ) );
    };

  TExpr* Rasobr( const MathExpr& Ex )
    {
    MathExpr  op21, op22;
    int N, D;
    Result = nullptr;
    if( Ex.Multp( op21, op22 ) ) Result = ( ( Rasobr( op21 ) ) + ( Rasobr( op22 ) ) ) else
      if( Ex.SimpleFrac_( N, D ) ) Result = ( ( CrLog( Constant( ( N ) ) ) ) - ( CrLog( Constant( ( D ) ) ) ) ) else
        if( Ex.Divis( op21, op22 ) ) Result = ( ( Rasobr( op21 ) ) - ( Rasobr( op22 ) ) ) else
          if( ex.Power( op21, op22 ) ) Result = ( ( op22 ) * ( Rasobr( op21 ) ) );
    if( Result == nullptr ) Result = CrLog( Ex );
    };

  bool IsLog( const MathExpr& ex, MathExpr& op2 )
    {
    MathExpr  logoper2;
    QByteArray Name2;
    Result = false;
    if( ex.Log( logoper2, op2 ) && ( Name1 == "" ) ) {
      if( ( !Assigned( logoper ) ) ) {
        logoper = logoper2;
        Result = true;
        exit;
        };
      if( ( logoper2.Equal( logoper ) ) ) Result = true;
      };
    if( ex.Funct( Name2, op2 ) && ( !Assigned( logoper ) ) && ( ( Name1 == "log" ) || ( Name1 == "lg" ) ) && ( ( Name1 == "" ) || ( Name1 == Name2 ) ) ) {
      Name1 = Name2;
      Result = true;
      };
    };

  bool IsLogSumSubMul( const MathExpr& ex )
    {
    MathExpr  op1, op2;
    Result = false;
    if( ex.Summa( op1, op2 ) && IsLogSumSubMul( op1 ) && IsLogSumSubMul( op2 ) )
      Result = true;
    if( ex.Subtr( op1, op2 ) && IsLogSumSubMul( op1 ) && IsLogSumSubMul( op2 ) )
      Result = true;
    if( IsLog( ex, op2 ) ) Result = true;
    if( ex.Multp( op1, op2 ) && ( IsLogSumSubMul( op1 ) || IsLogSumSubMul( op2 ) ) )
      Result = true;
    };

  TExpr* Rasobr2( const MathExpr& ex )
    {
    MathExpr  op1, op2, op11, op21;
    Result = nullptr;
    if( ex.Summa( op1, op2 ) ) {
      op1 = Rasobr2( op1 );
      op2 = Rasobr2( op2 );
      if( IsLog( op1, op11 ) && IsLog( op2, op21 ) )
        Result = CrLog( ( ( op11 ) * ( op21 ) ) );
      exit;
      };
    if( ex.Subtr( op1, op2 ) ) {
      op1 = Rasobr2( op1 );
      op2 = Rasobr2( op2 );
      if( IsLog( op1, op11 ) && IsLog( op2, op21 ) )
        Result = CrLog( ( ( op11 ) / ( op21 ) ) );
      exit;
      };

    if( ex.Multp( op1, op2 ) ) {
      op1 = Rasobr2( op1 );
      op2 = Rasobr2( op2 );
      if( IsLog( op1, op11 ) )
        Result = CrLog( ( ( op11 ) ^ ( op2 ) ) ) else
        if( IsLog( op2, op21 ) )
          Result = CrLog( ( ( op21 ) ^ ( op1 ) ) );
      exit;
      };
    Result = ex;
    };

  MathExpr  expr, op2, exInit;
  MathExpr ResExpr1, ResExpr2;

  Name1 = "";
  logoper = nullptr;
  exInit = ex;
  expr = ex.Reduce();
  if( expr.Equal( ex ) || expr.ConstExpr )
    {
    expr = ex;
    }
  else ex = new TBinar( '='ex, expr );

  ResExpr1 = nullptr;
  ResExpr2 = nullptr;
  Name1 = "";
  if( IsLog( expr, op2 ) && IsLogPowerDivMul( op2 ) ) ResExpr1 = Rasobr( op2 );
  if( ( ResExpr1 == nullptr ) && ( IsLogSumSubMul( expr ) ) && ( ( Name1 != "" ) || ( logoper != nullptr ) ) ) ResExpr1 = Rasobr2( expr );
  if( ( ResExpr1 == nullptr ) && IsLog( exInit, op2 ) ) ResExpr1 = expr;

  if( ResExpr1 != nullptr )
    {
    if( ResExpr1.Equal( exInit ) )
      TSolutionChain::sm_SolutionChain.AddExpr( ex )
    else
      {
      ResExpr2 = ResExpr1.Reduce();
      SolutionChain.AddExpr( ResExpr1 );
      SolutionChain.AddExpr( ResExpr2 );
      if( ResExpr2.Equal( ResExpr1 ) )
        {
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '='ex, ResExpr1 ) );
        ResExpr2 = ResExpr1;
        }
      else TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '='ex, new TBinar( '='ResExpr1, ResExpr2 ) ) );
      };
    OutWin.ReplaceComm( X_Str( "XPSimpleCalcMess", "MCalced", "Calculated!" ) );
    if( Edt != nullptr ) Edt.Clear;
    }
  else
    {
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ) );
    OutWin.ReplaceComm( X_Str( "XPSimpleCalcMess", "MNotSuitableExpr", "Wrong expression!" ) );
    };
  if( PutAnswer && Assigned( ResExpr2 ) && !Assigned( Answer ) )
    Answer = ResExpr2;
  if( Assigned( logoper ) ) logoper.Free;
  }

/*
void LogProperties( txpeditor Edt, txpoutwindow OutWin, int Prop )
  {
  MathExpr  ex;
  memory_switch = SWcalculator;
  if( Edt.Write != "" ) {
    try{
      ex = OutPut( GetExpression( FullPreProcessor( Edt.Write, "x" ) ) );
      LogPropertiesEx( ex, Edt, OutWin, Prop );
      except
        on E : ErrParser ) {
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ) );
        OutWin.Addcomm( X_Str( "XPSimpleCalcMess", E.MsgName, E.Message ) );
}
      };
    };
  memory_switch = SWtask;
  OutWin.Show;
  Edt.RefreshXPE;
  }

/*

bool CheckMulPower( const MathExpr& Ex, MathExpr& ePowBase, MathExpr& ePowEx )
  {
  MathExpr  PowBase, PowEx;
  bool First, Second;

  bool CheckPower( const MathExpr& Ex )
    {
    MathExpr  op1, op2;
    Result = false;
    if( Ex.Power( op1, op2 ) || Ex.Root1( op1, op2 ) ) {
      if( First ) {
        PowBase = op1.Reduce();
        PowEx = op2.Reduce();
        First = false;
        Result = true;
        }
      else {
        Second = true;
        op1 = op1.Reduce();
        op2 = op2.Reduce();
        if( Assigned( PowBase ) )  if( PowBase.Equal( op1 ) ) Result = true else {
          PowBase = nullptr;
          };
        if( Assigned( PowEx ) )  if( PowEx.Equal( op2 ) ) Result = true else {
          PowEx = nullptr;
          };
        };
      };
    };

  bool MultDivOfPowerCheck( const MathExpr& Ex )
    {
    MathExpr  op1, op2;
    if( Ex.Multp( op1, op2 ) ) {
      Result = MultDivOfPowerCheck( op1 ) && MultDivOfPowerCheck( op2 );
      }
    else  if( Ex.Divis( op1, op2 ) ) {
      Result = MultDivOfPowerCheck( op1 ) && MultDivOfPowerCheck( op2 );
      }
    else Result = CheckPower( Ex );
    };

  First = true;
  Second = false;
  Result = false;
  if( MultDivOfPowerCheck( Ex ) && Second ) {
    ePowBase = PowBase;
    ePowEx = PowEx;
    Result = true;
    };
  }

/*
TExpr* ExpPr( const MathExpr& Ex, int N, MathExpr& ex2 )
  {

  TExpr* PowerToMulOfPower( const MathExpr& Ex, const MathExpr& ePow, const MathExpr& eRoot )
    {
    MathExpr  op1, op2;
    int n;
    Result = nullptr;
    if( ex.Root_( op1, op2, n ) ) {
      if( Assigned( eRoot ) ) eRoot = ( ( eRoot ) * ( Constant( ( n ) ) ) ) else
        eRoot = Constant( ( n ) );
      Result = PowerToMulOfPower( op1, ePow, eRoot );
      }
    else  if( ex.Root1_( op1, op2 ) ) {
      if( Assigned( eRoot ) ) eRoot = ( ( eRoot ) * ( op2 ) ) else
        eRoot = op2;
      Result = PowerToMulOfPower( op1, ePow, eRoot );
      }
    else
      if( ex.Power( op1, op2 ) ) {
        if( Assigned( ePow ) ) ePow = ( ( ePow ) * ( op2 ) ) else
          ePow = op2;
        Result = PowerToMulOfPower( op1, ePow, eRoot );
        }
      else
        if( ex.Multp( op1, op2 ) )
          Result = ( ( PowerToMulOfPower( op1, ePow, eRoot ) ) * ( PowerToMulOfPower( op2, ePow, eRoot ) ) )
        else
        if( ex.Divis( op1, op2 ) )
          Result = ( ( PowerToMulOfPower( op1, ePow, eRoot ) ) / ( PowerToMulOfPower( op2, ePow, eRoot ) ) )
        else {
          if( Assigned( ePow ) && Assigned( eRoot ) ) {
            op1 = ( ( ePow ) / ( eRoot ) );
            Result = ( ( ex ) ^ ( op1 ) );
            }
          else  if( Assigned( ePow ) ) Result = ( ( ex ) ^ ( ePow ) ) else
            if( Assigned( eRoot ) ) {
              if( eRoot.Cons_int( n ) ) Result = ( ( ex ).Root( n ) ) else
                Result = new TRoot1( ex, eRoot );
              };
          };
    };

  TExpr* Step( const MathExpr& Exam, const MathExpr& oper1, const MathExpr& oper2, bool Back = false )
    {
    MathExpr  op1, op2;
    Result = nullptr;
    if( Back ) {
      if( Exam.Power( op1, op2 ) ) Result = new TRoot1( oper1, oper2 ) else
        if( Exam.Root1_( op1, op2 ) ) Result = ( ( oper1 ) ^ ( oper2 ) );
      }
    else {
      if( Exam.Power( op1, op2 ) ) Result = ( ( oper1 ) ^ ( oper2 ) ) else
        if( Exam.Root1_( op1, op2 ) ) Result = new TRoot1( oper1, oper2 );
      };
    };

  TExpr* SumSubtMulInPower( const MathExpr& Ex )
    {
    MathExpr  op1, op2, op21, op22, op11, op12;
    Result = nullptr;
    if( ex.Power( op1, op2 ) || ex.Root1_( op1, op2 ) ) {
      if( op2.Multp( op21, op22 ) ) {
        op11 = Step( Ex, op1, op21 );
        op12 = Step( Ex, SumSubtMulInPower( op11 ), op22 );
        Result = SumSubtMulInPower( op12 );
        }
      else  if( op2.Divis( op21, op22 ) ) {
        op11 = Step( Ex, op1, op21 );
        op12 = Step( Ex, SumSubtMulInPower( op11 ), op22, true );
        Result = SumSubtMulInPower( op12 );
        }
      else  if( op2.Summa( op21, op22 ) ) {
        op11 = Step( Ex, op1, op21 );
        op12 = Step( Ex, op1, op22 );
        Result = ( ( SumSubtMulInPower( op11 ) ) * ( SumSubtMulInPower( op12 ) ) );
        }
      else  if( op2.Subtr( op21, op22 ) ) {
        op11 = Step( Ex, op1, op21 );
        op12 = Step( Ex, op1, op22 );
        Result = ( ( SumSubtMulInPower( op11 ) ) / ( SumSubtMulInPower( op12 ) ) );
        }
      else Result = Ex;
      };
    };

  TExpr* DeleteStep( const MathExpr& Ex )
    {
    MathExpr  op1, op2;
    Result = nullptr;
    if( Ex.Multp( op1, op2 ) ) Result = ( ( DeleteStep( op1 ) ) * ( DeleteStep( op2 ) ) )
    else  if( Ex.Divis( op1, op2 ) ) Result = ( ( DeleteStep( op1 ) ) / ( DeleteStep( op2 ) ) )
    else  if( Ex.Power( op1, op2 ) || Ex.Root1_( op1, op2 ) ) Result = op1;
    };

  TExpr* DeleteBase( const MathExpr& Ex )
    {
    MathExpr  op1, op2;
    Result = nullptr;
    if( Ex.Multp( op1, op2 ) ) Result = ( ( DeleteBase( op1 ) ) + ( DeleteBase( op2 ) ) )
    else  if( Ex.Divis( op1, op2 ) ) Result = ( ( DeleteBase( op1 ) ) - ( DeleteBase( op2 ) ) )
    else  if( Ex.Power( op1, op2 ) || Ex.Root1_( op1, op2 ) ) Result = op2;
    };

  MathExpr  op1, op2, op11, op12;
  Result = nullptr;
  if( N < 6 ) {
    if( ( N == 4 ) || ( N == 5 ) || ( N == 3 ) ) {
      if( ( ex.Power( op1, op2 ) || ex.Root1_( op1, op2 ) ) && ( op1.Power( op11, op12 ) || op1.Root1_( op1, op2 ) orop1.Multp( op11, op12 ) || op1.Divis( op11, op12 ) ) )
        Result = PowerToMulOfPower( Ex, nullptr, nil );
      if( ( N != 3 ) && ( Result != nullptr ) ) ex2 = Result;
      };
    if( ( ( N == 1 ) || ( N == 2 ) || ( N == 3 ) ) && ( Result == nullptr ) )
      if( ( ex.Power( op1, op2 ) || ex.Root1_( op1, op2 ) ) && ( op2.Multp( op11, op12 ) || op2.Divis( op11, op12 ) orop2.Summa( op11, op12 ) || op2.Subtr( op11, op12 ) ) ) {
        Result = SumSubtMulInPower( Ex );
        ex2 = Result;
        };

    if( ( Result == nullptr ) && CheckMulPower( Ex, op1, op2 ) ) {
      if( Assigned( op1 ) && ( ( N == 1 ) || ( N == 2 ) ) )
        Result = ( ( op1 ) ^ ( DeleteBase( Ex ) ) )
      else  if( Assigned( op2 ) && ( ( N == 4 ) || ( N == 5 ) ) )
      Result = ( ( DeleteStep( Ex ) ) ^ ( op2 ) );
      };
    }
  else {
    N = N - 10;
    case N of {
      1: {
        Result = PowerToMulOfPower( Ex, nullptr, nil );
        ex2 = Result;
        };
      2: {
        Result = SumSubtMulInPower( Ex );
        ex2 = Result;
        };
      3: {
        CheckMulPower( Ex, op1, op2 );
        Result = ( ( op1 ) ^ ( DeleteBase( Ex ) ) );
        };
      4: {
        CheckMulPower( Ex, op1, op2 );
        Result = ( ( DeleteStep( Ex ) ) ^ ( op2 ) );
        };
      };
    };
  }

/*
void ExpProperties( txpeditor Edt, txpoutwindow OutWin, int Prop )
  {


  MathExpr ex, ex1, ex2;

  bool Error;

  memory_switch = SWcalculator;
  if( Edt.Write != "" )
    {
    try{
      ex = OutPut( GetExpression( FullPreProcessor( Edt.Write, "x" ) ) );
      Error = false;
      ex1 = nullptr;
      ex2 = nullptr;
      case prop of {
        1: ex1 = ExpPr( ex, 1, ex2 );

        2:ex1 = ExpPr( ex, 2, ex2 );

        3: ex1 = ExpPr( ex, 3, ex2 );

        4: ex1 = ExpPr( ex, 4, ex2 );

        5: ex1 = ExpPr( ex, 5, ex2 );

        };
      if( !Error )
        {
        if( ex2 == nullptr )
          ex2 = ex1.Reduce();
        if( ex2.Eq( ex1 ) )
          TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '='ex, ex1 ) )
        else
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar( '='ex, new TBinar( '='ex1, ex2 ) ) );
        OutWin.ReplaceComm( X_Str( "XPSimpleCalcMess", "MCalced", "Calculated!" ) );
        Edt.Clear;
        }
      else {
        TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ) );
        OutWin.ReplaceComm( X_Str( "", "MNotSuitableExpr", "Wrong expression!" ) );
        }
      except
        on E : ErrParser )
{
TSolutionChain::sm_SolutionChain.AddExpr( new TStr( "" ) );
OutWin.Addcomm( X_Str( "XPSimpleCalcMess", E.MsgName, E.Message ) );
}
      };
    };
  memory_switch = SWtask;
  OutWin.Show;
  Edt.RefreshXPE;
  }

/*
int CheckLogOp(const MathExpr& ex) 
{
QByteArray  Name1;
MathExpr logoper;

bool IsLogPowerDivMul(const MathExpr& ex) 
{
MathExpr  op21,op22;
int N,D;
Result=false;
 if( ex.Multp(op21,op22) ) Result=true;
 if( ex.Divis(op21,op22) || ex.SimpleFrac_(N,D) ) Result=true;
 if( ex.Power(op21,op22) ) Result=true;
};

bool IsLog(const MathExpr& ex, MathExpr& op2) 
{
MathExpr  logoper2;
QByteArray Name2;
Result=false;
 if( ex.Log(logoper2,op2) && (Name1=="") ) {
 if( (! Assigned(logoper)) ) {
logoper=logoper2;
Result=true;
exit;
};
 if( (logoper2.Equal(logoper)) ) Result=true;
};
 if( ex.Funct(Name2,op2) && (! Assigned(logoper)) && ((Name1=="log") || (Name1=="lg")) && ((Name1=="") || (Name1==Name2)) ) {
Name1=Name2;
Result=true;
};
};

bool IsLogSumSubMul(const MathExpr& ex) 
{
MathExpr  op1,op2;
Result=false;
 if( ex.Summa(op1,op2) && IsLogSumSubMul(op1) && IsLogSumSubMul(op2) )
Result=true;
 if( ex.Subtr(op1,op2) && IsLogSumSubMul(op1) && IsLogSumSubMul(op2) )
Result=true;
 if( IsLog(ex,op2) ) Result=true;
 if( ex.Multp(op1,op2) && (IsLogSumSubMul(op1) || IsLogSumSubMul(op2)) )
Result=true;
};

MathExpr  expr,op2;

Name1="";
logoper=nullptr;
expr=ex.Reduce();
 if( expr.Equal(ex) ) {
expr=ex;
};

Name1="";
Result=0;
 if( IsLog(expr,op2) && IsLogPowerDivMul(op2) ) Result=1;
 if( (Result!=1) && (IsLogSumSubMul(expr))&& ((Name1!="")||(logoper!=nullptr)) ) Result=1;
 if( Assigned(logoper) ) logoper.Free;
};

void CalcLogOp(const MathExpr& Ex, txpoutwindow OutWin, txpeditor Edt) 
{
memory_switch=SWcalculator;
try{
LogPropertiesEx(Ex,Edt,OutWin,1);
except
on E: ErrParser ) {
TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""));
OutWin.Addcomm(X_Str("XPSimpleCalcMess", E.MsgName, E.Message ));
}
};
memory_switch=SWtask;
OutWin.Show;
Edt.RefreshXPE;
};

void ChangeBasisLogEx(const MathExpr& ex, txpeditor Edt, txpoutwindow OutWin) 
{

MathExpr  ex1;
memory_switch=SWcalculator;
try{
TSolutionChain::sm_SolutionChain.AddExpr(ex);
ex1=CalcChangeBasisLog(ex);
TSolutionChain::sm_SolutionChain.AddExpr(ex1);
OutWin.Addcomm(X_Str("XPSimpleCalcMess","MTransToOtherBasis", "The transition to other basis is executed"));
 if( PutAnswer && Assigned(ex1) && ! Assigned(Answer) ) 
Answer = ex1;
Edt.Clear;
except
on E: ErrParser ) {
TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""));
OutWin.Addcomm(X_Str("XPSimpleCalcMess", E.MsgName, E.Message ));
}
};
memory_switch=SWtask;
OutWin.Show;
Edt.RefreshXPE;
};

int CheckPowerOper(const MathExpr& Ex) 
{
MathExpr  op1,op2,op11,op12;
Result=0;
 if( (ex.Power(op1,op2) || ex.Root1_(op1,op2)) && (op1.Power(op11,op12) || op1.Root1_(op1,op2) orop1.Multp(op11,op12) || op1.Divis(op11,op12)) )
Result=Result+1;
 if( (ex.Power(op1,op2) || ex.Root1_(op1,op2)) && (op2.Multp(op11,op12) || op2.Divis(op11,op12) orop2.Summa(op11,op12) || op2.Subtr(op11,op12)) )
Result=Result+2;

 if( CheckMulPower(Ex,op1,op2) ) {
 if( Assigned(op1) ) Result=Result+4;
 if( Assigned(op2) ) Result=Result+8;
};
};

void PowerOper(const MathExpr& ex, txpeditor Edt, txpoutwindow OutWin, int N) 
{

MathExpr  ex1,ex2;
memory_switch=SWcalculator;
try{
TSolutionChain::sm_SolutionChain.AddExpr(ex);
ex2=nullptr;
ex1=ExpPr(ex,N+10,ex2);
 if( ex2==nullptr ) ex2=ex1.Reduce();
 if( ex2.Eq(ex1) ) TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('='ex,ex1))
else TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('='ex, new TBinar('='ex1,ex2)));
OutWin.ReplaceComm(X_Str("XPSimpleCalcMess","MCalced","Calculated!"));
 if( PutAnswer && Assigned(ex1) && ! Assigned(Answer) ) 
Answer = ex2;
Edt.Clear;
except
on E: ErrParser ) {
TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""));
OutWin.Addcomm(X_Str("XPSimpleCalcMess", E.MsgName, E.Message ));
}
};
memory_switch=SWtask;
OutWin.Show;
Edt.RefreshXPE;
};



}. 
*/
