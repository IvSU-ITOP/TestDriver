#include "Analysis.h"
#include "SolChain.h"
#include "ExpStore.h"
#include "Statistics.h"

extern bool s_ShowDiviMessages;
extern ExpStore s_ExpStore;

MathExpr NewVariable(MathExpr& OldVar)
  {
  QByteArray VarName ;
  if( !OldVar.Variab( VarName ) ) return nullptr;
  if( VarName == "z" )
    VarName = "x";
  else
    VarName = QByteArray(1, VarName[0] + 1);
  return Variable( VarName );
  }

void MakeCalcDiff(const MathExpr& e, MathExpr& ex1, MathExpr& ex2)
  {
  MathExpr  ex, v;
  if( !e.Deriv(ex, v) || s_GlobalInvalid )
    {
    s_GlobalInvalid=true;
    s_LastError=X_Str("MCannotDiff","Cannot differentiate!");
    return;
    }
  if( ex.Eq(v) )
    {
    ex1 = new TConstant(0);
    ex2 = ex1;
    return;
    }
  s_RootToPower = true;
  bool OldNoRootReduce = s_NoRootReduce;
  bool OldNoLogReduce = s_NoLogReduce;
  s_NoRootReduce = true;
  s_NoLogReduce = true;
  ex = ex.Reduce();
  s_RootToPower = false;
  MathExpr df = ex.Diff(v.WriteE());
  if( s_GlobalInvalid )
    {
    s_LastError=X_Str("MCannotDiff","Cannot differentiate!");
    s_NoRootReduce = OldNoRootReduce;
    s_NoLogReduce = OldNoLogReduce;
    return;
    }
  s_GlobalInvalid=false;
  ex1 = df;
  s_PowerToRoot = true;
  ex2 = df.Reduce();
  ex = ExpandExpr( ex2 );
  if( !ex.Eq(ex2) )
    ex2 =  new TBinar( '=', ex2, ex );
  else
    s_PowerToRoot = false;
  s_NoRootReduce = OldNoRootReduce;
  s_NoLogReduce = OldNoLogReduce;
  }

MathExpr CalcDiffExpr(const MathExpr& e)
  {
  MathExpr  ex1, ex2, Result;
  MakeCalcDiff(e, ex1, ex2);
  if( s_GlobalInvalid )
    return nullptr;
  if( ex1 != ex2 )
    Result = new TBinar('=', ex1, ex2);
  else
    Result = ex1;
  s_XPStatus.SetMessage(X_Str("MDiffed","Differentiated!"));
  return new TBinar( '=', e.Clone(), Result );
  }

bool IsThereIntegral(const MathExpr& ex)
  {
  bool Result=false;
  std::function<void ( const MathExpr&)> PostOrder = [&] (const MathExpr& ex)
    {
    MathExpr op1,op2;
    QByteArray name;
    uchar c;
    if( ex.Integr_(op1,  op2) )
      throw ' ';
    else
      if( ex.Oper_(c, op1, op2) )
        {
        PostOrder(op1);
        PostOrder(op2);
        }
      else
        if( ex.Unarminus(op1) || ex.Funct(name,op1) )
          PostOrder(op1);
    };
  try
    {
    PostOrder(ex);
    }
  catch (char)
    {
    Result=true;
    }
  return Result;
  }

MathExpr CalcIntegralExpr(const MathExpr& e)
  {
  MathExpr  ex, it, rd, it1, exCoeff, exSubst, exRepl, NewVar, exTmp, exTmp1, v;
  bool OldShowDiviMessages, OldNoRootReduce, OldNoLogReduce;
  bool LogSign, WasRoot, WasExponent, expReplaced;
  QByteArray sName;
  int iTail;

  std::function<MathExpr( MathExpr&)> GetPowerX = [&] (MathExpr& exp)
    {
    MathExpr exLeft, exRight, exTmp, exL, exR, op1, op2, exCoeff;
    uchar cOper;
    int root;
    QByteArray sName;

    auto NegSqRoot = [] (const MathExpr& exp, MathExpr& exArg)
      {
      MathExpr exPower;
      int Nom, Denom;
      return exp.Power( exArg, exPower ) && exPower.SimpleFrac_( Nom, Denom ) &&
         ( Nom == -1 ) && ( Denom == 2 );
      };

    std::function<bool( MathExpr&, MathExpr&)> ReplaceExp = [&] ( MathExpr& exp, const MathExpr& exResult)
      {
      if(expReplaced) return true;
      MathExpr exLeft, exRight;
      uchar cOper;
      QByteArray sName;
      if( exp.Oper_( cOper, exLeft, exRight ) )
        {
        if( ( cOper == '/' ) || ( cOper == '*' ) )
          {
          if( exLeft == exResult )
            {
            CastPtr( TOper, exp )->Left() =  Constant( ( 1 ) );
            expReplaced = true;
            return true;
            }
          if( cOper == '*' )
            {
            if( exRight == exResult )
              {
              CastPtr( TOper, exp )->Right() =  Constant( ( 1 ) );
              expReplaced = true;
              return true;
              }
            MathExpr Result = exResult;
            return ReplaceExp( exLeft, Result ) || ReplaceExp( exRight, Result );
            }
          }
        }
      return false;
      };

    MathExpr Result;
    if( exp.Oper_( cOper, exLeft, exRight ) )
      {
      TOper *pOpOper = CastPtr( TOper, exp );
      if( cOper == '/' )
        {
        WasExponent = false;
        Result = GetPowerX( exLeft );
        if( Result.IsEmpty() )
          {
          if( exRight.Multp( exL, exR ) && exL.Eq(v) && exR.Root_( op1, op2, root ) )
            {
            if( ( root != 2 ) || !op1.Oper_( cOper, exL, exR ) ) return Result;
              if( cOper == '+' )
                {
                if( ! exR.ConstExpr() )
                  if( exL.ConstExpr() )
                    {
                    op1 = exR;
                    exR = exL;
                    exL = op1;
                    }
                  else
                    return Result;
                if( exL.Power( op1, op2 ) )
                  {
                  if( !op1.Eq(v) || !IsConst( op2, 2 ) ) return Result;
                  exTmp = pOpOper->Right();
                  pOpOper->Right() = exR * v^-2 + 1;
                  }
                else
                  {
                  if( !exL.Multp( exCoeff, op1 ) || !exCoeff.ConstExpr() || !op1.Power( op1, op2 ) || !op1.Eq(v) ||
                      !IsConst(op2, 2) ) return Result;
                  exTmp = pOpOper->Right();
                  pOpOper->Right() = (exR * v^-2) + exCoeff;
                  }
                }
              else
                if( cOper == '-' )
                  {
                  if( exL.ConstExpr() )
                    {
                    if( exR.Power( op1, op2 ) )
                      {
                      if( !op1.Eq(v) || !IsConst( op2, 2 ) ) return Result;
                      exTmp = pOpOper->Right();
                      pOpOper->Right() = exL * v^-2 - 1;
                      }
                    else
                      {
                      if( !exR.Multp( exCoeff, op1 ) || !exCoeff.ConstExpr() || !op1.Power( op1, op2) ||
                          !op1.Eq(v) || !IsConst(op2, 2) ) return Result;
                      exTmp = pOpOper->Right();
                      pOpOper->Right() = (exL * v^-2) - exCoeff;
                      }
                    }
                  else
                    {
                    if( !exR.ConstExpr() ) return Result;
                    if( exL.Power( op1, op2 ) )
                      {
                      if( !op1.Eq(v) || !IsConst( op2, 2 ) ) return Result;
                      exTmp = pOpOper->Right();
                      pOpOper->Right() = Constant(1) - exR * v^-2;
                      }
                    else
                      {
                      if( !exL.Multp( exCoeff, op1 ) || !exCoeff.ConstExpr() || !op1.Power( op1, op2) ||
                        !op1.Eq(v) || ! IsConst(op2, 2) ) return Result;
                      exTmp = pOpOper->Right();
                      pOpOper->Right() = exCoeff - exR * v^-2;
                      }
                    }
                  }
                else
                  return Result;
                return v^-2;
              }
            if( ( IsConstType( TSumm, exRight ) ) || ( IsConstType( TSubt, exRight ) ) )
              {
              exTmp = exRight.ReToMult();
              if( IsConstType( TMult, exTmp ) )
                {
                pOpOper->Right() = exTmp;
                exRight = exTmp;
                }
              else
                return Result;
              }
          WasExponent = false;
          Result = GetPowerX( exRight );
          if( Result.IsEmpty() ) return Result;
          if( WasExponent )
            {
            QByteArray Name;
            Result.Funct(Name, exTmp);
            exTmp = -exTmp;
            Result = new TFunc(false, Name, exTmp);
            return Result;
            }
          if( Result.Power( exLeft, exRight ) )
            {
            CastPtr( TOper, Result )->Right() = Constant( ( 0 ) );
            exTmp = -exRight;
            Result = exLeft.Clone() ^ exTmp.Reduce();
            pOpOper->Left() = pOpOper->Left().Clone() * Result.Clone();
            }
          else
            {
            exL.Clear();
            QByteArray Name = CastPtr(TVariable, v)->Name();
            if( exRight.Eq(v) || ( exRight.Multp( exL, exR ) && ( ( exL.Eq(v) && ( exR.HasUnknown(Name ) == "" ) )
              ||( exR.Eq(v) && ( exL.HasUnknown( Name ) == "" ) ) ) ) )
              {
              exTmp = Function( "log", v );
              if( exLeft.HasExpr( exTmp ) )
                {
                Result = Constant( 1 ) / v;
                if( exL.IsEmpty() )
                  pOpOper->Right() = 1;
                else
                  {
                  exTmp = pOpOper->Right();
                  if( exL.Eq(v) )
                    pOpOper->Right() = exR;
                  else
                    pOpOper->Right() = exL;
                  }
                return Result;
                }
              exTmp = Function( "log", exRight );
              if( exLeft.HasExpr( exTmp ) )
                {
                Result = Constant( 1 ) / exRight;
                pOpOper->Right() = 1;
                }
              return Result;
              }
            }
          }
        else
          if( WasExponent && ! ReplaceExp( exp, Result ) )
            Result.Clear();
        return Result;
        }
      if( cOper == '*' )
        {
        WasExponent = false;
        Result = GetPowerX( exLeft );
        if( Result.IsEmpty() ) Result = GetPowerX( exRight );
        if( WasRoot )
          {
          WasRoot = NegSqRoot( exLeft, op1 ) && NegSqRoot( exRight, op2 ) &&
           ( op1.Summa( exLeft, exRight ) || op2.Summa( exLeft, exRight ) );
          if( exLeft.ConstExpr() )
            LogSign = exRight.Eq(v) || ( exRight.Multp( op1, op2 ) && op2.Eq(v) && op1.ConstExpr() );
          else
            if( exRight.ConstExpr() )
              LogSign = exLeft.Eq(v) || ( exLeft.Multp( op1, op2 ) && op2.Eq(v) && op1.ConstExpr() );
          }
        if( WasExponent && ! ReplaceExp( exp, Result ) )
          Result.Clear();
        return Result;
        }
      if( ( cOper == '^' ) && exLeft.Eq(v) )
        {
        Result = exp;
        WasRoot = NegSqRoot( exp, exRight );
        }
      return Result;
      }
    if( exp.Eq(v) )
      Result = exp;
    else
      if( exp.Funct( sName, op1 ) && ( sName == "exp" ) )
        {
        op1.Unarminus( op1 );
        if( op1.Eq(v) || (op1.Multp(exLeft, exRight) && exLeft.ConstExpr() && exRight.Eq(v)) )
          {
          WasExponent = true;
          Result = exp;
          }
        }
    return Result;
    };

  std::function<void( MathExpr&)> ArgToAbsArg = [&] ( MathExpr& exp)
    {
    MathExpr exLeft, exRight, exArg;
    uchar cOper;
    if( exp.Funct( sName, exArg ) || exp.Abs_( exArg ) )
      {
      ArgToAbsArg( exArg );
      return;
      }
    if( exp.Oper_( cOper, exLeft, exRight ) )
      {
      if( cOper == '~' || cOper == '^' )
        {
        if( !exLeft.ConstExpr() )
        CastPtr( TOper, exp)->Left() = new TAbs( false, exLeft );
        return;
        }
      ArgToAbsArg( exLeft );
      ArgToAbsArg( exRight );
      return;
      }
    };

  MathExpr Result, ee = e;
  if( s_IntegralCount == 0 )
    TSolutionChain::sm_SolutionChain.AddExpr(e);
  exCoeff.Clear();
  e.Multp( exCoeff, ee );
  if( !ee.Integr_(ex,v) )
    {
    TExpr::sm_IntegralError = true;
    s_LastError = X_Str("MCannotInt","Cannot integrate!");
    return Result;
    }
  s_GlobalVarName = sName = CastPtr(TVariable, v)->Name();
  s_RootToPower = true;
  OldNoRootReduce = s_NoRootReduce;
  s_NoRootReduce = true;
  s_IsIntegral = true;
  s_SummExpFactorize = true;
  OldNoLogReduce = s_NoLogReduce;
  s_NoLogReduce = true;
  rd = ex.Reduce();
  s_SummExpFactorize = false;
  TExpr::sm_IntegralError = false;
  OldShowDiviMessages = s_ShowDiviMessages;
  s_ShowDiviMessages = false;
  it = rd.Integral(sName);
  MathExpr itOld = it;
  iTail = -1;
  while( IsThereIntegral( it ) && ! TExpr::sm_IntegralError )
    {
    try
      {
      if( s_IntegralCount == 1 )
        iTail = TSolutionChain::sm_SolutionChain.AddExpr( it ) - 1;
      else
        iTail = TSolutionChain::sm_SolutionChain.AddAndReplace( e, it ) - 1;
      it = ReduceTExprs( it );
      if(it.Eq(itOld))
        {
        TExpr::sm_IntegralError = true;
        break;
        }
      itOld = it;
      }
    catch( char* )
      {
      TExpr::sm_IntegralError = true;
      }
    catch( ErrParser )
      {
    TExpr::sm_IntegralError = true;
      }
    }
  if( TExpr::sm_IntegralError )
    {
    TSolutionChain::sm_SolutionChain.DockTail( iTail );
    if( s_IntegralCount > 0 )
      {
      s_IntegralCount = s_IntegralCount - 1;
      if( s_IntegralCount > 1 ) throw  ErrParser( X_Str("MCannotCalculate", "Cannot calculate!"), peNewErr );
        s_IntegralCount = 0;
      }
    }
  else
    {
    if( exCoeff != nullptr )
    it = (( exCoeff.Clone() ) * ( it ));
    Result = it + Variable( "C" );
    }
  if( TExpr::sm_IntegralError && ! ( IsConstType( TSumm, rd ) ) && ! ( IsConstType( TSubt, rd ) ) )
    {
    exRepl = rd.Clone();
    WasRoot = false;
    LogSign = false;
    expReplaced = false;
    ex = GetPowerX( exRepl );
    if( !ex.IsEmpty() )
      {
      TExpr::sm_IntegralError = false;
      if( ! ex.Funct( sName, exTmp ) || ( sName != "exp" ) )
        {
        exTmp1 = exRepl.Clone();
        exTmp =  Constant( 1 );
        exTmp1.Replace( ex, exTmp );
        expReplaced = exTmp1.HasUnknown( sName ) != "";
        if( expReplaced )
          exRepl = exTmp1;
        else
          TExpr::sm_IsAuxiliaryIntegral = true;
        it = ex.Integral( CastPtr(TVariable, v)->Name() );
        TExpr::sm_IsAuxiliaryIntegral = false;
        it1 = it.Reduce();
        }
      else
        {
        it1 = ex.Clone();
        if( exTmp.Negative() )
        it1 = Constant( -1 ) * it1;
        }
      if( it1.Unarminus( it1 ) )
        it1 = Constant(-1) * it1;
      if( it1.Funct( sName, exTmp ) )
        it1 = Constant( 1 ) * it1;
      TExpr::sm_IntegralError = true;
      if( it1.Multp( exTmp, exSubst ) )
        {
        NewVar = NewVariable( v );
        if( exRepl.Replace( exSubst, NewVar ) )
          {
          TSolutionChain::sm_SolutionChain.AddComment( "$Break" );
          TSolutionChain::sm_SolutionChain.AddExpr(  new TBinar( '=', NewVar, exSubst ), "$Break" );
          TSolutionChain::sm_SolutionChain.AddExpr(e);
          if( exCoeff == nullptr )
            exCoeff = exTmp;
          else
            exCoeff = ReduceTExprs( exCoeff.Clone() * exTmp.Clone() );
          if( !expReplaced )
            {
            exTmp =  Constant( 1 );
            exTmp1 = ex.Clone();
            exRepl.Replace( exTmp1, exTmp );
            }
         exTmp = exRepl.Reduce();
         if( exTmp.HasUnknown( CastPtr(TVariable, v)->Name() ) == "" )
           {
           TExpr::sm_IntegralError = false;
           s_NoAbsInLog = exSubst.Funct( sName, exTmp1 ) && ( sName == "exp" );
           exTmp = exCoeff * (  new TIntegral( false, exTmp, NewVar.Clone() ) );
           it = CalcIntegralExpr( exTmp );
           s_NoAbsInLog = false;
           if( it != nullptr )
             {
             it.Replace( NewVar, exSubst );
             if( LogSign )
               {
               ArgToAbsArg(it);
               it = Function( "sign", v.Clone() ) * it;
               }
             s_RootToPower = false;
             s_PowerToRoot = true;
             Result = ReduceTExprs( it );
             s_PowerToRoot = false;
             s_EqualPicture = false;
             s_ShowDiviMessages = OldShowDiviMessages;
             s_IsIntegral = false;
             TSolutionChain::sm_SolutionChain.AddExpr( Result );
             s_NoLogReduce = OldNoLogReduce;
             s_NoRootReduce = OldNoRootReduce;
             s_GlobalInvalid = false;
             return Result;
             }
           }
         }
       }
     }
   }
  s_ShowDiviMessages = OldShowDiviMessages;
  s_IsIntegral = false;
  if( Result == nullptr )
    {
    s_GlobalInvalid = true;
    s_LastError=X_Str("XPAnalysisCalcMess","Cannot integrate!");
    s_NoRootReduce = OldNoRootReduce;
    s_RootToPower = false;
    return Result;
    }
  if( s_IntegralCount == -1 )
    {
    TSolutionChain::sm_SolutionChain.Expand();
    s_NoRootReduce = OldNoRootReduce;

    s_RootToPower = false;
    return Result;
    }
  exTmp = Result.Clone();
  if( s_IntegralCount == 0 )
    TSolutionChain::sm_SolutionChain.AddExpr( Result );
  else
    TSolutionChain::sm_SolutionChain.AddAndReplace( e, exTmp );
  Result = ReduceTExprs( Result );
  s_RootToPower = false;
  s_PowerToRoot = true;
  Result = ReduceTExprs( Result );
  s_PowerToRoot = false;
  s_EqualPicture = false;
  s_ExpandPower = false;
  Result = ExpandExpr( Result );
  if( s_IntegralCount == 0 )
    TSolutionChain::sm_SolutionChain.AddExpr( Result );
  else
    TSolutionChain::sm_SolutionChain.AddAndReplace( exTmp, Result );
  s_IntegralCount = -1;
  s_ExpandPower = true;
  s_NoLogReduce = OldNoLogReduce;
  s_NoRootReduce = OldNoRootReduce;
  s_GlobalInvalid = false;
  s_EqualPicture = true;
  TExpr::sm_IntegralError = false;
  return Result;
  }

void MakeCalcLimit(const MathExpr& e, MathExpr& ex1, MathExpr& ex2)
  {
  MathExpr  ex,exv,exl,lm,rd;
  QByteArray s;
  bool ng,sdm;
  if( !e.Limit(ex,exv,exl) || !exv.Variab(s) )
    {
    s_GlobalInvalid = true;
    s_LastError = X_Str("XPAnalysisCalcMess","Cannot calculate limit!");
    return;
    }
  exl = exl.Reduce();
  rd = ReduceTExprs(exl.Diff(s));
  if( (s_GlobalInvalid || !IsConst(rd,0)) && !exl.Infinity(ng) )
    {
    s_GlobalInvalid = true;
    s_LastError=X_Str("XPAnalysisCalcMess","Cannot calculate limit!");
    return;
    }
  sdm = s_ShowDiviMessages;
  s_ShowDiviMessages = false;
  s_GlobalInvalid = false;
  rd = ex.Reduce();
  s_OpenMultAmbiguity = false;
  s_NoExpReduce = true;
  lm = rd.Lim(s,exl);
  s_ShowDiviMessages = sdm;
  if( lm.IsEmpty() )
    s_GlobalInvalid = true;
  if( s_GlobalInvalid )
    {
    s_LastError=X_Str("XPAnalysisCalcMess","Cannot calculate limit!");
    s_NoExpReduce = false;
    return;
    }
  if( IsConstType( TUnar, lm ) ) lm = lm.Reduce();
  ex1 = lm;
  exl.Clear();
  while( lm.Binar( '=', exl, lm ) );
    if( IsConstType( TInfinity, lm ) )
      {
      if( exl != nullptr ) ex1 = exl;
      ex2 = lm.Clone();
      }
    else
      {
      if( lm.Funct( s, exv ) && exv.Binar( '=', exv, lm ) )
        ex2 = Function( s, lm.Reduce() );
      else
        {
        ex2 = lm.Reduce();
        if( IsConstType( TBinar, ex1 ) && ex2.Eq( lm ) )
          {
          ex1 = exl;
          ex2 = lm.Clone();
          }
        }
      }
   s_NoExpReduce = false;
   }

MathExpr CalcLimitExpr(const MathExpr& e)
  {
  MathExpr  ex1, ex2, Result;
  MakeCalcLimit(e, ex1, ex2);
  if( !s_GlobalInvalid )
    {
    if( !ex1.Eq(ex2) )
      Result = new TBinar('=', ex1, ex2);
    else
      Result = ex1;
    Result = new TBinar('=', e.Clone(), Result);
    s_XPStatus.SetMessage(X_Str("XPAnalysisCalcMess", "Limit calculated!"));
    }
  return Result;
  }

void MakeCalcDefIntegral(const MathExpr& e, MathExpr& ex1, MathExpr& ex2, MathExpr& ex3)
  {
  MathExpr  ex,it,rd,ll,hl, v, exTmp ;
  ex1.Clear();
  ex2.Clear();
  ex3.Clear();
  if( !e.Dfintegr_(ex,ll,hl,v) || s_GlobalInvalid )
    {
    s_GlobalInvalid = true;
    s_LastError = X_Str("XPAnalysisCalcMess", "Cannot integrate!");
    return;
    }
  bool OldRootToPower = s_RootToPower;
  s_RootToPower = true;
  rd = ex.Reduce();
  s_RootToPower = OldRootToPower;
  s_IntegralCount = 0;
  it = rd.Integral(v.WriteE());
  if( TExpr::sm_IntegralError )
    {
    s_GlobalInvalid = true;
    s_LastError=X_Str("XPAnalysisCalcMess","Cannot integrate!");
    return;
    }
  s_GlobalInvalid = false;
  rd = it.Reduce();
  while( IsThereIntegral(rd) )
    {
    it = rd.Reduce();
    rd = it;
    }
  ex1 = new TSubst(false, rd, ll, hl);
  if( ( IsConstType( TInfinity, ll ) ) || IsConst( ll, 0 ) )
    {
    ex =  new TLimit( false, rd, v, ll );
    MakeCalcLimit( ex, exTmp, ll );
    }
  else
    ll = rd.Substitute(v.WriteE(),ll);

  if( ( IsConstType( TInfinity, hl ) ) || IsConst( hl, 0 ) )
    {
    ex =  new TLimit( false, rd.Clone(), v, hl );
    MakeCalcLimit( ex, exTmp, hl );
    }
  else
    hl = rd.Substitute(v.WriteE(),hl);
  ex2 = hl - ll;
  ex3 = ex2.Reduce();
  }

MathExpr CalcDefIntegralExpr(const MathExpr& e)
  {
  MathExpr  ex1,ex2,ex3, Result;
  MakeCalcDefIntegral(e,ex1,ex2,ex3);
 if( !s_GlobalInvalid )
   {
   Result = new TBinar('=', e.Clone(), new TBinar('=', ex1, new TBinar('=', ex2, ex3)));
   s_XPStatus.SetMessage(X_Str("XPAnalysisCalcMess", "Integrated!"));
   }
  return Result;
  }
/*
bool AssigningNew(const MathExpr& exi)
  {
  if( exi.IsEmpty() ) return false;
  MathExpr op1, op2;
  if( !exi.Binar( '=', op1, op2 ) ) return false;
  QByteArray Name ;
  if( !op1.Variab( Name ) ) return false;
  if( s_ExpStore.Known_var( Name, true ) ) return false;
  MathExpr opr2 = op2.Reduce();
  double V ;
  if( opr2.Constan( V ) ) return true;
  return false;
  }
*/
MathExpr CalcTrigo2xEx(const MathExpr& Exi)
  {
  QByteArray FuncName ;
  MathExpr Argument ;
  Exi.Funct(FuncName, Argument);
  MathExpr ex2 = Argument / Constant( 2 );
  ex2 = ReduceTExprs(ex2);
  MathExpr ex, exm ;
  if( Argument.Measur_( ex, exm) )
    TExpr::sm_TrigonomSystem = TExpr::tsDeg;
  else
    if(TExpr::sm_TrigonomSystem == TExpr::tsDeg && IsType(TConstant, ex2 ))
      ex2 = new TMeaExpr( ex2, Variable( msDegree ) );
  MathExpr ex_sin = Function( "sin", ex2 );
  MathExpr ex_cos = Function( "cos", ex2 );
  MathExpr ex_tan = Function( "tan", ex2 );
  MathExpr ex_cot = Function( "cot", ex2 );
  MathExpr  exo;
  if( FuncName == "sin" ) exo = Constant( 2 ) * ex_sin * ex_cos;
  if( FuncName == "cos" ) exo = (ex_cos ^ Constant( 2 )) - (ex_sin ^ Constant( 2 ));
  if( FuncName == "tan" ) exo = ( Constant( 2 ) * ex_tan ) / ( Constant( 1 ) - (ex_tan ^ Constant( 2 )) );
  if( FuncName == "cot" ) exo = ( (ex_cot ^ Constant( 2 )) - Constant( 1 ) ) / ( Constant( 2 ) * ex_cot );
  bool OldFullReduce = TExpr::sm_FullReduce;
  TExpr::sm_FullReduce = true;
  exo = OutPutTrigonom(Exi,exo);
  TExpr::sm_FullReduce = OldFullReduce;
  return exo;
  }

MathExpr CalcTrigo3xEx(const MathExpr& Exi)
  {
  QByteArray FuncName ;
  MathExpr Argument ;
  Exi.Funct(FuncName, Argument);
  MathExpr ex2 = Argument / Constant( 3 );
  ex2 = ReduceTExprs(ex2);
  MathExpr ex, exm ;
  if( Argument.Measur_( ex, exm) )
    TExpr::sm_TrigonomSystem = TExpr::tsDeg;
  else
    if(TExpr::sm_TrigonomSystem == TExpr::tsDeg && IsType(TConstant, ex2 ))
      ex2 = new TMeaExpr( ex2, Variable( msDegree ) );
  MathExpr ex_sin = Function( "sin", ex2 );
  MathExpr ex_cos = Function( "cos", ex2 );
  MathExpr ex_tan = Function( "tan", ex2 );
  MathExpr ex_cot = Function( "cot", ex2 );
  MathExpr  exo;
  if( FuncName == "sin" ) exo = Constant( 3 ) * ex_sin - Constant( 4 ) * (ex_sin ^ Constant( 3 ));
  if( FuncName == "cos" ) exo = Constant( 4 ) * (ex_cos ^ Constant( 3 )) - Constant( 3 ) * ex_cos;
  if( FuncName == "tan" ) exo = (Constant( 3 ) * ex_tan - (ex_tan ^ Constant( 3 ))) /
    (Constant(1) - Constant( 3 ) * (ex_tan ^ Constant( 2 )));
  if( FuncName == "cot" ) exo = ((ex_cot ^ Constant(3)) - Constant(3) * ex_cot ) /
    ( Constant(3) * (ex_cot ^ Constant( 2 )) - Constant( 1 ));
  return OutPutTrigonom(Exi,exo);
  }

MathExpr CalcTan2Ex(const MathExpr& Exi)
  {
  QByteArray FuncName ;
  MathExpr Argument ;
  Exi.Funct(FuncName, Argument);
  MathExpr ex2 = Argument / Constant( 2 );
  ex2 = ReduceTExprs(ex2);
  MathExpr ex, exm ;
  if( Argument.Measur_( ex, exm) )
    TExpr::sm_TrigonomSystem = TExpr::tsDeg;
  else
    if(TExpr::sm_TrigonomSystem == TExpr::tsDeg && IsType(TConstant, ex2 ))
      ex2 = new TMeaExpr( ex2, Variable( msDegree ) );
  MathExpr ex_tan = Function( "tan", ex2 );
  MathExpr ex_tan2 = ex_tan ^ Constant( 2 );
  MathExpr ex_2tan= Constant( 2 ) * ex_tan;
  MathExpr  exo;
  if( FuncName == "sin" ) exo = ex_2tan / ( Constant( 1 ) + ex_tan2 );
  if( FuncName == "cos" ) exo = ( Constant( 1 ) - ex_tan2 ) / ( Constant( 1 ) + ex_tan2 );
  if( FuncName == "tan" ) exo = ex_2tan / ( Constant( 1 ) - ex_tan2 );
  if( FuncName == "cot" ) exo = ( Constant( 1 ) - ex_tan2 ) / ex_2tan;
  return OutPutTrigonom(Exi,exo);
  }

MathExpr CalcAlpha2Ex(const MathExpr& Exi)
  {
  QByteArray FuncName ;
  MathExpr Argument ;
  Exi.Funct(FuncName, Argument);
  MathExpr ex2 = Constant( 2 ) * Argument;
  ex2 = ReduceTExprs(ex2);
  MathExpr ex, exm ;
  if( Argument.Measur_( ex, exm) )
    TExpr::sm_TrigonomSystem = TExpr::tsDeg;
  else
    if(TExpr::sm_TrigonomSystem == TExpr::tsDeg && IsType(TConstant, ex2 ))
      ex2 = new TMeaExpr( ex2, Variable( msDegree ) );
  MathExpr ex_cos = Function( "cos", ex2 );
  MathExpr  exo;
  if( FuncName == "sin" ) exo = (Constant( 1 ) - ex_cos) / Constant( 2 );
  if( FuncName == "cos" ) exo = (Constant( 1 ) + ex_cos) /  Constant( 2 );
  if( FuncName == "tan" ) exo = ( Constant( 1 ) - ex_cos ) / ( Constant( 1 ) + ex_cos );
  if( FuncName == "cot" ) exo = ( Constant( 1 ) + ex_cos ) / ( Constant( 1 ) - ex_cos );
  exo = exo.Root(2);
  return OutPutTrigonom(Exi,exo);
  }

bool CalcSimplifyPrvEx(const MathExpr& exiI)
  {
  label:
  MathExpr exi, exo, prev,exi00;
  MathExpr op1, op2, op11,op21, arg ;
  MathExpr op2n;
  double SavePrec = s_Precision;
  MathExpr ex, exm ;
  QByteArray FuncName, Name ;
  MathExpr Argument ;
  double V, V1, V2, min ;
  double Coeff1,Coeff2;
  bool b1,b2,unar, ConvToRad, Result = false;
  int i,k, iMin, iMax ;

  TExpr::TrigonomSystem OldTrigonomSystem = TExpr::sm_TrigonomSystem;
  double angles[] = {-360,-270,-180,-90,0,90,180,270,360};

  std::function<void( MathExpr& )> ConvertToRad = [&]( MathExpr& ex)
    {
    uchar cOper ;
    MathExpr exLeft, exRight, exArg ;

    auto Convert = [&]( MathExpr& arg)
      {
      if( arg.Measur_( op11, op21 ) )
        {
        op11.Constan( V );
        return Constant( V / 180 ) * Variable( msPi );
        }
      ConvertToRad( arg );
      return arg;
      };

    if( ex.Unarminus( exArg ) )
      {
      ex = MathExpr(new TUnar(Convert( exArg )));
      return;
      }
    if( ex.Oper_( cOper, exLeft, exRight ) )
      {
      if( cOper == '=' )
        {
        ConvertToRad( exLeft );
        ConvertToRad( exRight );
        return;
        }
      ex = MathExpr( new TOper(Convert( exLeft ).Reduce(), Convert( exRight ).Reduce(), cOper ));
      return;
      }
    if( ex.Funct( Name, exArg ) )
      ex = MathExpr( new TFunc( false, Name, Convert( exArg ) ) );
   };

   SavePrec = s_Precision;
   s_GlobalTrigonomError = false;
   try
    {
    exi00 = exiI;
    s_Precision=0.0000000001;
    if( !exi00.IsEmpty() )
      {
      if( exi00.Funct(FuncName,Argument) && IsTrigonom(FuncName) )
        {
        iMin = -4;
        iMax = 4;
        if( ( FuncName == "tan" ) || ( FuncName == "cot" ) )
          {
          iMin = -2;
          iMax =  2;
          }
        ConvToRad = false;
        if( !(Argument.Summa(op1,op2) || Argument.Subtr(op1,op2)) )
          {
          if( !Argument.Unarminus(ex) )
            {
            unar = false;
            ex = Argument;
            }
          else
            unar = true;
          if( ex.Multp( op1, op2 ) )
            {
            if( ! op2.Variab( Name ) || ( Name[0] != msPi ) || ! op1.Constan( V ) ) goto label;
            ConvToRad = true;
            ex =  Constant( V * 180 );
            }
          else
            if( TExpr::sm_TrigonomSystem == TExpr::tsRad && ex.Constan( V ) )
              {
              ConvToRad = true;
              ex =  Constant( V / 3.1415926535 * 180 );
              }
         if( (ex.Constan(V) || (ex.Reduce().Measur_(exo,exm) && exo.Constan(V))) )
           {
           if( unar )
             V = -V;
           if( ex.Constan(V1) )
             prev = Function( FuncName, new TMeaExpr( Constant( V ), Variable( msDegree ) ) );
           else
             prev = exi00;
           min = abs(V - angles[iMin + 4]);
           k = iMin;
           for( i = iMin + 1; i <= iMax; i++ )
             if( abs(V - angles[i + 4]) < min )
               {
               min = abs(V - angles[i + 4]);
               k = i;
               }
             V1 = angles[k + 4];
             V2 = abs( V - angles[k + 4] );
             i = 1;
             if( abs(k) == iMax )
                i = floor( V2 ) / round( abs( V1 ) ) + 1;
             if( i > 1 )
               {
               if( V1 > 0 )
                 op1 = MathExpr( new TMeaExpr(  Constant( V1 ), Variable( msDegree ) ) ) * Constant( i );
               else
                 op1 =  -( MathExpr(  new TMeaExpr(  Constant( -V1 ), Variable( msDegree ) ) ) * Constant( i ) );
               op2 =  new TMeaExpr(  Constant( V2 - abs( V1 ) * ( i - 1 ) ), Variable( msDegree ) );
               }
             else
               {
               op1 =  new TMeaExpr(  Constant( V1 ), Variable( msDegree ));
               op2 =  new TMeaExpr(  Constant( V2 ), Variable( msDegree ));
               }
             if( V < angles[k + 4] )
               Argument = op1 - op2;
             else
               Argument = op1 + op2;
           }
         else
          goto label;
        }
      else
        {
        prev.Clear();
        k = -5;
        }
      if( !op1.Unarminus(arg) )
        arg = op1;
      int AbsV = qAbs(V);
      int iRest = 0;
//      if(angles[iMax + 4] < AbsV )
//        iRest = qRound( qAbs(V) ) % qRound( angles[iMax + 4] );
      b1=( (PiProcess(arg, Coeff1, Coeff2) && ( (Coeff1==0.5 || ( Coeff1 == 1 && Coeff2 == 2 )) || ( Coeff1==1 && Coeff2 == 1 )
        || ( Coeff1==1.5 || ( Coeff1==3 && Coeff2==2) ) || ( Coeff1==2 && Coeff2==1))) ||
        ( (arg.Constan(V) || ( arg.Reduce().Measur_(ex,exm) && ex.Constan(V))) && ( V==0 || AbsV == 90 ||
        AbsV==180 || AbsV==270 || iRest == 0)));
      if( !op2.Unarminus(arg) )
        arg = op2;
      b2=( (PiProcess(arg, Coeff1, Coeff2) &&(( Coeff1==0.5 || ( Coeff1==1 && Coeff2==2)) || ( Coeff1==1 && Coeff2==1 ) ||
        ( Coeff1==1.5 ||( Coeff1==3 && Coeff2==2)) || (Coeff1==2 && Coeff2==1 ))) || ((arg.Constan(V) ||
        ( arg.Reduce().Measur_(ex,exm) && ex.Constan(V))) && ( V==0 || AbsV==90 || AbsV==180 || AbsV==270 || AbsV==360)));
      if( b1 || b2 )
        {
        if( b1 )
          {
          if( op1.Constan(V) || op1.Reduce().Measur_(ex,exm) )
            TExpr::sm_TrigonomSystem = TExpr::tsDeg;
          if( op1.Constan(V) )
            {
            op1 = new TMeaExpr(op1,Variable( ( msDegree ) ));
            if( Argument.Summa(op11,op21) )
              exo = op1 + op2;
            else
              exo = op1 - op2;
            exi00 = Function( FuncName, exo );
            }
          if( op2.Constan(V) || (op2.Reduce().Measur_(ex,exm) && ex.Constan(V)) )
            {
            if( TExpr::sm_TrigonomSystem == TExpr::tsDeg )
              op2n = new TMeaExpr( Constant( V ), Variable( msDegree ));
            else
              op2n = op2;
            if( Argument.Summa(op1,op2) )
              {
              if( op1.Constan(V) )
                op1 = new TMeaExpr(op1,Variable( ( msDegree ) ));
              ex = op1 + Variable( "z" );
              }
            else
              {
              if( op1.Constan(V) )
                op1 = new TMeaExpr(op1, Variable( msDegree ));
              ex = op1 - Variable( "z" );
              }
            exi00 = Function( FuncName, ex );
            }
         }
       else
         {
         if( op2.Constan(V) || op2.Reduce().Measur_(ex,exm) )
           TExpr::sm_TrigonomSystem = TExpr::tsDeg;
         if( op2.Constan(V) )
           {
           op2= new TMeaExpr(op2,Variable( msDegree ));
           if( Argument.Summa(op11,op21) )
             exo = op1 + op2;
           else
             exo = op1 - op2;
           exi00 = Function( FuncName, exo );
           }
         op2n.Clear();
         if( op1.Constan(V) || op1.Reduce().Measur_(ex,exm) && ex.Constan(V) )
           {
           if( TExpr::sm_TrigonomSystem == TExpr::tsDeg )
             op2n = new TMeaExpr( Constant( V ), Variable( msDegree ));
           else
             op2n = op1;
           if( Argument.Summa(op1,op2) )
             {
             if( op2.Constan(V) )
               op2 = new TMeaExpr(op2,Variable( msDegree ));
             ex = op2 + Variable( "z" );
             }
           else
             {
             if( op2.Constan(V) )
               op2 = new TMeaExpr(op2, Variable( msDegree ));
             ex = op2 - Variable( "z" );
             }
           exi00 = Function( FuncName, ex );
           }
        }
      if( MakeTrigonometric(exi00, exi, true) && !s_GlobalTrigonomError )
        {
        ex = exi.Reduce();
        exi = ex;
        if( !op2n.IsEmpty() )
          {
          exm = exi00.Substitute("z", op2n );
          exi00 = exm;
          ex = exi.Substitute("z", op2n );
          exi = ex;
          }
        s_Precision = SavePrec;
        if( !prev.IsEmpty() )
          exi00 = new TBinar('=', prev, exi00);
        exo = exi.Reduce();
        if( !exo.Equal(exi) )
          {
          exi = new TBinar('=', exi, exo);
          ex = exo.Reduce(true);
          if( !ex.Equal(exo) )
          exi = new TBinar('=', exi, ex);
          }
        if( !prev.IsEmpty() && k==0 )
          exo = exi;
        else
          exo = new TBinar('=', exi00, exi );
        if( ConvToRad )
           ConvertToRad( exo );
        TSolutionChain::sm_SolutionChain.AddExpr( exo, X_Str( "MCalculated", "Calculated!" ) );
        if( s_PutAnswer && s_Answer.IsEmpty() )
          s_Answer = exi;
        Result = true;
       }
     }
   else
     TSolutionChain::sm_SolutionChain.AddExpr( exi00, X_Str( "MInvalInput", "Invalid input for( this operation!" ) );
   }
  else
   TSolutionChain::sm_SolutionChain.AddExpr( exi00, X_Str( "MInvalInput", "Invalid input for( this operation!" ) );
  }
 }
 catch (...)
   {}
   TExpr::sm_TrigonomSystem = OldTrigonomSystem;
   s_Precision = SavePrec;
   return Result;
 }

MathExpr OutDegree( MathExpr& exi)
  {
  QByteArray FuncName;
  MathExpr Argument;
  MathExpr Op1, Op2;
  MathExpr ex1,exm1;
  MathExpr ex2,exm2;
  double V;
  uchar Oper;
  if( TExpr::sm_TrigonomSystem == TExpr::tsDeg && exi.Funct(FuncName, Argument) && IsTrigonom(FuncName) &&
    (Argument.Oper_(Oper,Op1, Op2 ) && (Oper == '+' || Oper == '-' )) && !Op1.Measur_(ex1,exm1) && !Op2.Measur_(ex2,exm2) )
    {
    if( Op1.Constan(V) )
      Op1 = new TMeaExpr(Op1 ,Variable( msDegree ));
    if( Op2.Constan(V) )
      Op2 = new TMeaExpr(Op2, Variable( msDegree ));
    if(Oper == '+')
      Argument = new TSumm(Op1, Op2);
    else
      Argument = new TSubt(Op1, Op2);
    exi = Function( FuncName, Argument );
    }
 return exi;
 }

MathExpr OutDegree1(MathExpr& exi)
  {
  MathExpr Op1, Op2;
  QByteArray FuncName1, FuncName2;
  MathExpr Argument1, Argument2;
  MathExpr ex, exm;
  double V ;
  uchar Oper;
  if( TExpr::sm_TrigonomSystem == TExpr::tsDeg && (exi.Oper_(Oper,Op1, Op2 ) && (Oper == '+' || Oper == '-' || Oper == '*') ) &&
    (Op1.Funct(FuncName1,Argument1) && IsTrigonom(FuncName1)) && (Op2.Funct(FuncName2, Argument2) && FuncName1 == FuncName2) &&
    ( !Argument1.Measur_(ex,exm) && !Argument2.Measur_(ex,exm)) )
    {
    if( Argument1.Constan(V) )
      Op1 = new TFunc( false, FuncName1, new TMeaExpr(Argument1, Variable( ( msDegree ) )) );
    if( Argument2.Constan(V) )
      Op2 = new TFunc( false, FuncName1, new TMeaExpr(Argument2, Variable( ( msDegree ) )) );
    if(Oper == '+') return new TSumm(Op1, Op2);
    if(Oper == '-') return new TSubt(Op1, Op2);
    if(Oper == '*') return new TMult(Op1, Op2);
    }
  return exi;
  }

void CalcSimplifyNew(const MathExpr& exiI, int Oper )
  {
  if( exiI.IsEmpty() ) return;
  MathExpr exi, exi00, exo, exol;

  MathExpr op1, op2;
  double SavePrec;
  double V;

  QByteArray FuncName;
  MathExpr Argument;
  bool IsOther;
  QByteArray FuncName1, FuncName2;
  QByteArray fname;
  MathExpr A, ex, exm;
  MathExpr Argument1;
  MathExpr Argument2;
  TExpr::TrigonomSystem OldTrigonomSystem = TExpr::sm_TrigonomSystem;
  s_GlobalTrigonomError = false;
  SavePrec = s_Precision;
  s_Precision=0.0000000001;
  exi00 = exiI;
  IsOther = false;
 if( exi00.Funct(FuncName,Argument) && IsTrigonom(FuncName) &&  (Argument.Summa(op1,op2) || Argument.Subtr(op1,op2))
   && ( Oper == 1 || Oper == 3 ) )
   {
   if( (op1.Measur_(ex,exm)  || op2.Measur_(ex,exm)) ) TExpr::sm_TrigonomSystem = TExpr::tsDeg;
   if( MakeTrigonometric(exi00, exi, false) && !s_GlobalTrigonomError )
     {
     exo = new TBinar('=', OutDegree(exi00), exi);
     Trigo2Str(exo,exi);
     s_Precision = SavePrec;
     exol = exi.SimplifyFull();
     if( exi.WriteE() != exol.WriteE() && !exi.Eq(exol) )
        TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', exo, exol ), X_Str("MCalculated", "Calculated!") );
     else
       TSolutionChain::sm_SolutionChain.AddExpr( exo, X_Str("MCalculated", "Calculated!"));
     if( s_PutAnswer && s_Answer.IsEmpty() ) s_Answer = exol;
//     TSolutionChain::sm_SolutionChain.AddComment(  );
     }
   IsOther = true;
   }

   if( (( exi00.WriteE().indexOf(msDegree) == -1 && (exi00.Summa(op1,op2) || exi00.Subtr(op1,op2) )&&
     op1.Funct(FuncName1,Argument) && IsTrigonom(FuncName1) && op2.Funct(FuncName2,Argument) && FuncName1==FuncName2)||
     (( exi00.Summa(op1,op2) || exi00.Subtr(op1,op2)) && op1.Funct(FuncName1,Argument1) && IsTrigonom(FuncName1) &&
     op2.Funct(FuncName2,Argument2) && FuncName1 == FuncName2 && Argument1.Measur_(ex,exm) && Argument2.Measur_(ex,exm)))
     && Oper == 2 )
     {
     if(!Argument1.IsEmpty() && Argument1.Measur_(ex,exm) && Argument2.Measur_(ex,exm)) TExpr::sm_TrigonomSystem = TExpr::tsDeg;
     if( MakeSummaTrigonometric(exi00,exi) )
       {
       exo = new TBinar('=',OutDegree1(exi00), exi );
       Trigo2Str(exo,exi);
       s_Precision = SavePrec;
       exol = exi.Simplify();
       if( exi.WriteE() != exol.WriteE() )
         TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', exo, exol));
       else
         TSolutionChain::sm_SolutionChain.AddExpr(exo);
       if( s_PutAnswer && s_Answer.IsEmpty() ) s_Answer = exol;
       TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCalculated", "Calculated!") );
       }
     IsOther = true;
     }
   Argument1.Clear();
   if( ((exi00.WriteE().indexOf(msDegree) == -1 && exi00.Multp(op1,op2) && op1.Funct(FuncName1,Argument) &&
     op2.Funct(FuncName2, Argument) && (((( FuncName1 == "sin" || FuncName1 == "cos"))&&(((FuncName2=="sin")||
     (FuncName2=="cos"))))||((FuncName1==FuncName2)&&((FuncName1=="tan")||(FuncName1=="cot"))))) ||
     ((exi00.Multp(op1,op2))&&(op1.Funct(FuncName1,Argument1))&&(op2.Funct(FuncName2,Argument2))&&(((((FuncName1=="sin")||
     (FuncName1=="cos")))&&(((FuncName2=="sin")||(FuncName2=="cos"))))||((FuncName1==FuncName2)&&((FuncName1=="tan")||
     (FuncName1=="cot"))))&&(Argument1.Measur_(ex,exm) && Argument2.Measur_(ex,exm)))) && Oper == 1 )
     {
     if( !Argument1.IsEmpty() && Argument1.Measur_(ex,exm) && Argument2.Measur_(ex,exm) ) TExpr::sm_TrigonomSystem = TExpr::tsDeg;
     if( MakeMultTrigonometric(exi00,exi) )
       {
       exo = new TBinar('=', OutDegree1(exi00), exi.Clone());
       Trigo2Str(exo,exi);
       s_Precision = SavePrec;
       exol = exi.Simplify();
       if( exi.WriteE() != exol.WriteE())
         TSolutionChain::sm_SolutionChain.AddExpr( new TBinar('=', exo, exol), X_Str("MCalculated", "Calculated!"));
       else
         TSolutionChain::sm_SolutionChain.AddExpr(exo, X_Str("MCalculated", "Calculated!"));
       if( s_PutAnswer && s_Answer.IsEmpty() ) s_Answer = exol;
//       TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCalculated", "Calculated!") );
       }
     IsOther = true;
     }
   if(!IsOther )
     {
     TSolutionChain::sm_SolutionChain.AddExpr( exi00, X_Str("MInvalInput", "Invalid input for( this operation!") );
//     TSolutionChain::sm_SolutionChain.AddComment( X_Str("MInvalInput", "Invalid input for( this operation!"));
     }
  s_Precision = SavePrec;
  TExpr::sm_TrigonomSystem = OldTrigonomSystem;
  }

void TDerivative::Solve()
  {
  MathExpr Result = CalcDiffExpr(m_Expr);
  bool bCalc = !(IsType( TLexp, m_Expr ));
  if( Result.IsEmpty())
    {
    s_GlobalInvalid = false;
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr );
    if(bCalc)
      {
      Result = m_Expr.Diff().Reduce();
      bCalc = !Result.IsEmpty() && !s_GlobalInvalid;
      }
    }
  if(bCalc)
    TSolutionChain::sm_SolutionChain.AddExpr( Result, X_Str( "MDiffying", "derivative calculated!" ) );
  else
    TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCannotCalculate", "Cannot calculate!") );
  m_Expr = new TBool(true);
  }

void TIndefIntegr::Solve()
  {
  s_IntegralCount = 0;
  m_Expr = CalcIntegralExpr( m_Expr );
  if(m_Expr.IsEmpty())
    TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCannotCalculate", "Cannot calculate!") );
  else
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr, X_Str( "MIntegrate", "Integral calculated!" ) );
  m_Expr = new TBool(true);
  }

void TDefIntegrate::Solve()
  {
  s_GlobalInvalid = false;
  m_Expr = CalcDefIntegralExpr( m_Expr );
  if(m_Expr.IsEmpty())
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""), X_Str("MCannotCalculate", "Cannot calculate!") );
  else
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr, X_Str( "MIntegrate", "Integral calculated!" ) );
  m_Expr = new TBool(true);
  }

void TLimitCreator::Solve()
  {
  s_GlobalInvalid = false;
  m_Expr = CalcLimitExpr( m_Expr );
  if(m_Expr.IsEmpty())
    TSolutionChain::sm_SolutionChain.AddExpr(new TStr(""), X_Str("MCannotLim", "Cannot Limit calculate!") );
  else
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr, X_Str( "MLimmed", "Limit calculated!" ) );
  m_Expr = new TBool(true);
  }

void TEigen::Solve()
  {
  MathExpr ex;
  if( !m_Expr.Matr(ex) )
    TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCannotEigen", "This is no Matrix!") );
  else
    {
    ex = CastConstPtr(TMatr, m_Expr)->Eigen();
    if(ex.IsEmpty())
      TSolutionChain::sm_SolutionChain.AddExpr(new TStr(""), X_Str("MCannotEigen", "Cannot Eigen Values calculate!") );
    else
      TSolutionChain::sm_SolutionChain.AddExpr( m_Expr, X_Str( "MEigenEd", "Eigen Values calculated!" ) );
    }
  m_Expr = new TBool(true);
  }

void TDeterminant::Solve()
  {
  MathExpr ex;
  if( !m_Expr.Matr(ex) )
    TSolutionChain::sm_SolutionChain.AddComment( X_Str("MEnterMatr", "Please enter a matrix!") );
  else
    {
    TMatr::sm_RecursionDepth = 2;
    ex = CastConstPtr(TMatr, m_Expr)->Determinant();
    if(ex.IsEmpty())
      TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""), X_Str("MCannotDeterminant", "Cannot Determinant calculate!") );
    else
      {
      TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
      TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MDeterminant", "Determinant calculated!" ) );
      }
    }
  m_Expr = new TBool(true);
  }

void TTransposer::Solve()
  {
  MathExpr ex;
  if( !m_Expr.Matr(ex) )
    {
    MathExpr Op1, Op2, ex2;
    if(m_Expr.Power(Op1, Op2))
      {
      QByteArray sVar;
      if(Op1.Matr(ex2) && Op2.Variab( sVar ) && sVar == "T" )
        {
        ex = CastPtr(TMatr, Op1)->Transpose();
        MathExpr TempX = new TBinar( '=', m_Expr, ex );
        TSolutionChain::sm_SolutionChain.AddExpr( TempX);
        m_Expr = new TBool(true);
        TSolutionChain::sm_SolutionChain.AddComment( X_Str("MTranspose", "Transpose calculated") );
        return;
        }
      }
    TSolutionChain::sm_SolutionChain.AddExpr( new TStr(""), X_Str("MCannotTranspose", "This is no Matrix!") );
    }
  else
    {
    ex = CastConstPtr(TMatr, m_Expr)->Transpose();
    if(ex.IsEmpty())
      TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCannotTranspose", "Cannot Transpose!") );
    else
      {
      MathExpr T = new TVariable(true, "T");
      MathExpr MT = m_Expr ^ T;
      MathExpr TempX = new TBinar( '=', MT, ex );
      TSolutionChain::sm_SolutionChain.AddExpr( TempX );
      TSolutionChain::sm_SolutionChain.AddComment( X_Str("MTranspose", "Transpose calculated") );
//      TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MDeterminant", "Transpose calculated" ) );
      }
    }
  m_Expr = new TBool(true);
  }

void TInverter::Solve()
  {
  MathExpr ex;
  if( !m_Expr.Matr(ex) )
    TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCannotInversion", "This is no Matrix!") );
  else
    {
    ex = CastConstPtr(TMatr, m_Expr)->Inversion();
    if(ex.IsEmpty())
      TSolutionChain::sm_SolutionChain.AddComment( X_Str("MCannotInversion", "Cannot Inversion!") );
    else
      {
      MathExpr P = new TPowr(m_Expr, -1);
      ex = new TBinar('=', P, ex);
//      TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
      TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MInversion", "Inversion calculated" ) );
      }
    }
  m_Expr = new TBool(true);
  }

void TAngle2::Solve()
  {
  TExpr::TrigonomSystem OldTrigonomSystem = TExpr::sm_TrigonomSystem;
  QByteArray FuncName;
  MathExpr Argument;
  if( m_Expr.Funct(FuncName, Argument) && IsTrigonom(FuncName) )
    {
    MathExpr ex = CalcTrigo2xEx(m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MCalced", "Calculated!" ) );
    }
  else
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInvalInput", "Invalid input for this operation!") ) ;
  m_Expr = new TBool(true);
  TExpr::sm_TrigonomSystem = OldTrigonomSystem;
  }

void TAngle3::Solve()
  {
  TExpr::TrigonomSystem OldTrigonomSystem = TExpr::sm_TrigonomSystem;
  double OldPrecision = s_Precision;
  s_Precision = 0.0000001;
  QByteArray FuncName;
  MathExpr Argument;
  if( m_Expr.Funct(FuncName, Argument) && IsTrigonom(FuncName) )
    {
    MathExpr ex = CalcTrigo3xEx(m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MCalced", "Calculated!" ) );
    }
  else
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInvalInput", "Invalid input for this operation!") ) ;
  m_Expr = new TBool(true);
  TExpr::sm_TrigonomSystem = OldTrigonomSystem;
  s_Precision = OldPrecision;
  }

void TTan2::Solve()
  {
  TExpr::TrigonomSystem OldTrigonomSystem = TExpr::sm_TrigonomSystem;
  QByteArray FuncName;
  MathExpr Argument;
  if( m_Expr.Funct(FuncName, Argument) && IsTrigonom(FuncName) )
    {
    MathExpr ex = CalcTan2Ex(m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MCalced", "Calculated!" ) );
    }
  else
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInvalInput", "Invalid input for this operation!") ) ;
  m_Expr = new TBool(true);
  TExpr::sm_TrigonomSystem = OldTrigonomSystem;
  }

void TAlpha2::Solve()
  {
  TExpr::TrigonomSystem OldTrigonomSystem = TExpr::sm_TrigonomSystem;
  QByteArray FuncName;
  MathExpr Argument;
  if( m_Expr.Funct(FuncName, Argument) && IsTrigonom(FuncName) )
    {
    MathExpr ex = CalcAlpha2Ex(m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( ex, X_Str( "MCalced", "Calculated!" ) );
    }
  else
    TSolutionChain::sm_SolutionChain.AddComment( X_Str( "MInvalInput", "Invalid input for this operation!") ) ;
  m_Expr = new TBool(true);
  TExpr::sm_TrigonomSystem = OldTrigonomSystem;
  }

void TPriv::Solve()
  {
  CalcSimplifyPrvEx(m_Expr);
  m_Expr = new TBool(true);
  }

void TTrigoOfSumma::Solve()
  {
  CalcSimplifyNew(m_Expr, 3);
  m_Expr = new TBool(true);
  }

void TSumm_Mult::Solve()
  {
  CalcSimplifyNew(m_Expr, 2);
  m_Expr = new TBool(true);
  }

void TTrigo_Summ::Solve()
  {
  CalcSimplifyNew(m_Expr, 1);
  m_Expr = new TBool(true);
  }

void Permutations::Solve()
  {
  MathExpr Arg;
  QByteArray Name;
  if(!m_Expr.Funct(Name, Arg) || Name != "PerCount")
    {
    s_LastError = X_Str( "MNoPermutation", "This is no permutation!" );
    return;
    }
  Simplify();
  }

void BinomCoeff::Solve()
  {
  MathExpr Arg;
  QByteArray Name;
  if(!m_Expr.Funct(Name, Arg) || Name != "BinomCoeff")
    {
    s_LastError = X_Str( "MNoBinomCoeff", "This is no binomial coefficient!" );
    return;
    }
  Simplify();
  }

void Accomodations::Solve()
  {
  MathExpr Arg;
  QByteArray Name;
  if(!m_Expr.Funct(Name, Arg) || Name != "ACoeff")
    {
    s_LastError = X_Str( "MNoAccommodation", "This is no permutations of k of n" );
    return;
    }
  Simplify();
  }

void Statistics::Solve()
  {
  CalcStatistics(m_Expr);
  m_Expr = new TBool(true);
  }

void Correlation::Solve()
  {
  CalcCorrelation(m_Expr);
  m_Expr = new TBool(true);
  }

void AlgToTrigo::Solve()
  {
  MathExpr exResult = m_Expr.AlgToGeometr();
  if( !exResult.IsEmpty())
    {
    TSolutionChain::sm_SolutionChain.AddExpr( m_Expr);
    TSolutionChain::sm_SolutionChain.AddExpr( exResult, X_Str( "MCalced", "Calculated!" ) );
    }
  m_Expr = new TBool(true);
  }

void ComplexOper::Solve()
  {
  if(!m_Expr.HasComplex() )
    {
    s_LastError = X_Str( "MNoComplex", "There are no complex numbers here" );
    return;
    }
  Simplify();
  }

TIndefIntegr::TIndefIntegr( const MathExpr Expr ) : Solver( Expr ) {}
TIndefIntegr::TIndefIntegr() : Solver() { m_Code = EIndefInt; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TDerivative::TDerivative( const MathExpr Expr ) : Solver( Expr ) {}
TDerivative::TDerivative() : Solver() { m_Code = EDeriv; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TLimitCreator::TLimitCreator( const MathExpr Expr ) : Solver( Expr ) {}
TLimitCreator::TLimitCreator() : Solver() { m_Code = ELimit; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TDefIntegrate::TDefIntegrate( const MathExpr Expr ) : Solver( Expr ) {}
TDefIntegrate::TDefIntegrate() : Solver() { m_Code = EDefInt; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TEigen::TEigen( const MathExpr Expr ) : Solver( Expr ) {}
TEigen::TEigen() : Solver() { m_Code = EEigenVals; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TDeterminant::TDeterminant( const MathExpr Expr ) : Solver( Expr ) {}
TDeterminant::TDeterminant() : Solver() { m_Code = EDeterminant; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TTransposer::TTransposer( const MathExpr Expr ) : Solver( Expr ) {}
TTransposer::TTransposer() : Solver() { m_Code = ETranspose; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TInverter::TInverter( const MathExpr Expr ) : Solver( Expr ) {}
TInverter::TInverter() : Solver() { m_Code = EMatrixInv; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TAngle2::TAngle2( const MathExpr Expr ) : Solver( Expr ) {}
TAngle2::TAngle2() : Solver() { m_Code = EAngle2; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TAngle3::TAngle3( const MathExpr Expr ) : Solver( Expr ) {}
TAngle3::TAngle3() : Solver() { m_Code = EAngle3; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TTan2::TTan2( const MathExpr Expr ) : Solver( Expr ) {}
TTan2::TTan2() : Solver() { m_Code = ETan2; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TAlpha2::TAlpha2( const MathExpr Expr ) : Solver( Expr ) {}
TAlpha2::TAlpha2() : Solver() { m_Code = EAlpha2; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TPriv::TPriv( const MathExpr Expr ) : Solver( Expr ) {}
TPriv::TPriv() : Solver() { m_Code = EPriv; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TTrigoOfSumma::TTrigoOfSumma( const MathExpr Expr ) : Solver( Expr ) {}
TTrigoOfSumma::TTrigoOfSumma() : Solver() { m_Code = ETrigoOfSumma; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TSumm_Mult::TSumm_Mult( const MathExpr Expr ) : Solver( Expr ) {}
TSumm_Mult::TSumm_Mult() : Solver() { m_Code = ESumm_Mult; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

TTrigo_Summ::TTrigo_Summ( const MathExpr Expr ) : Solver( Expr ) {}
TTrigo_Summ::TTrigo_Summ() : Solver() { m_Code = ETrigo_Summ; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

Permutations::Permutations( const MathExpr Expr ) : Solver( Expr ) {}
Permutations::Permutations() : Solver() { m_Code = EPermutations; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

BinomCoeff::BinomCoeff( const MathExpr Expr ) : Solver( Expr ) {}
BinomCoeff::BinomCoeff() : Solver() { m_Code = EBinomCoeff; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

Accomodations::Accomodations( const MathExpr Expr ) : Solver( Expr ) {}
Accomodations::Accomodations() : Solver() { m_Code = EAccomodations; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

Statistics::Statistics( const MathExpr Expr ) : Solver( Expr ) {}
Statistics::Statistics() : Solver() { m_Code = EStatistics; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

Correlation::Correlation( const MathExpr Expr ) : Solver( Expr ) {}
Correlation::Correlation() : Solver() { m_Code = ECorrelation; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

AlgToTrigo::AlgToTrigo( const MathExpr Expr ) : Solver( Expr ) {}
AlgToTrigo::AlgToTrigo() : Solver() { m_Code = EAlgToTrigo; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

ComplexOper::ComplexOper( const MathExpr Expr ) : Solver( Expr ) {}
ComplexOper::ComplexOper() : Solver() { m_Code = EComplexOper; m_Name = "MCalced"; m_DefaultName = "Calculated!"; }

MathExpr TPowr::Integral(const QByteArray& d)
  {
  MathExpr  a1,b1,c1,a2,b2,t, Result;
  double a,b,c;
  bool Check;
  QByteArray s;
  int n;

  if( TExpr::sm_IntegralError )
    return Clone();

  if( m_Operand1.HasUnknown() != d )
    return Variable(d) * Clone();
  if( m_Operand1.Funct( s, a1 ) && s == "tan" && IsConst( m_Operand2, 2 ) )
    {
    t = Constant(1) /Function( "cos", a1.Clone())^m_Operand2.Clone() - Constant(1);
    return t.Integral(d);
    }
  if( m_Operand1.Funct( s, a1 ) && s == "cot" && IsConst( m_Operand2, 2 ) )
    {
    t = Constant(1)/Function( "sin", a1.Clone() )^m_Operand2.Clone() - Constant(1);
    return t.Integral(d);
    }
  m_Operand2.BinomX(a2,b2,Check);
  if( !Check )
    {
    TExpr::sm_IntegralError = true;
    return Clone();
    }
  m_Operand1.TrinomCh(a1,b1,c1,Check, d);
  if( !Check )
    {
    if( IsConst(a2,0) && b2.Cons_int(n) )
    if( n==-2 )
    if( m_Operand1.Funct(s, a1) && ( s=="sin" || s=="cos" ) )
      {
      a1.BinomX(b1,c1,Check);
      if( !Check )
        {
        TExpr::sm_IntegralError = true;
        Result=Clone();
        }
      else
        {
        if( s=="sin" )
          Result= -((Constant(1)/b1) * Function("cot", a1.Clone()));
        else
          Result = (Constant(1)/b1) * Function("tan", a1.Clone() );
        }
      }
    else
      {
      TExpr::sm_IntegralError = true;
      Result=Clone();
      }
  else
    if( n==2 )
      if( m_Operand1.Funct(s, a1) && (s=="sin" || s=="cos") )
        {
        if( s=="sin" )
          b1=(Constant(1) - Function( "cos", Constant(2) * a1.Clone()))/Constant(2);
        else
          b1=(Constant(1) + Function( "cos", Constant(2) * a1.Clone()))/Constant(2);
       Result=b1.Integral(d);
       }
     else
      {
      a1=Expand(this);
      if( a1.Eq(this) )
        {
        TExpr::sm_IntegralError = true;
        Result=Clone();
        }
      else
        Result=a1.Integral(d);
      }
   else
    {
    a1=Expand(this);
    if( a1.Eq(this) )
      {
      TExpr::sm_IntegralError = true;
      Result=Clone();
      }
    else
     Result=a1.Integral(d);
    }
  else
    {
    TExpr::sm_IntegralError = true;
    Result=Clone();
    }
  return Result;
  }
  if( IsConst(a2,0) )
    {
    if( IsConst(b2,-1) )
      {
      if( IsConst(a1,0) )
        {
        if( s_NoAbsInLog && !(IsConstType( TSubt, m_Operand1 )) )
          Result=( Constant(1)/b1 ) * Function( "log", m_Operand1.Clone());
        else
          if( TExpr::sm_IsAuxiliaryIntegral )
             Result = (Constant(1)/b1) * Function("log", m_Operand1.Clone());
          else
             Result = (Constant(1)/b1 ) * Function( "log", new TAbs(false,m_Operand1.Clone()));
        }
      else
        if( a1.Constan(a) && b1.Constan(b) && c1.Constan(c) )
          {
          if( abs(b) < 1E-6 )
            if( a/c > 0 )
              Result=((Constant(1)/Constant( c * a ).Root(2))) *
                Function("arctan", (Constant(a) / Constant(c)).Root(2) * Variable(d) );
            else
              {
              if( a>0 )
                a1= (Constant( abs(a) ).Root( 2 ) * Variable(d) - Constant(abs(c)).Root(2) ) /
                  (Constant(abs(a)).Root( 2 ) * Variable(d) + Constant(abs(c)).Root(2));
              else
                a1 = (Constant( abs(a)).Root(2) * Variable(d) + Constant(abs(c)).Root(2)) /
                  (Constant( abs(a)).Root( 2 ) * Variable( d ) - Constant( abs(c)).Root( 2 ));
              Result = (Constant(1) / ( Constant(2) * (Constant( abs(a)) * Constant(abs(c)) ).Root(2))) *
                  Function("log", new TAbs(false, a1) );
              }
            else
              {
              TExpr::sm_IntegralError = true;
              Result=Clone();
              }
           }
         else
           {
           TExpr::sm_IntegralError = true;
           Result=Clone();
           }
         return Result;
         }
       t = GenerateFraction(-1,2);
       if( b2.Constan(b) && (abs(b+0.5)<1E-6) || b2.Equal(t) )
         {
         if( a1.Constan(a) && b1.Constan(b) && c1.Constan(c) )
           {
           if( abs(a)<1E-6 )
             Result = (Constant(2)/Constant(b)) * (Constant(b) * Variable(d) +
               Constant(c))^(Constant(1) / Constant(2));
           else
             if( abs(b)<1E-6 )
               if( a>0 )
                 if( c >= 0 )
                   Result = (Constant(1) / Constant(a).Root(2)) * Function("log",
                     (Constant(a) * Variable(d)^Constant(2) + Constant(c)).Root(2) + Variable(d) * Constant(a).Root(2));
                 else
                   Result = (Constant(1) / Constant(a).Root(2)) * Function("log",
                     new TAbs( false, (Constant(a) * Variable(d)^Constant(2) +
                     Constant(c)).Root(2) + Variable(d) * Constant(a).Root(2) ) );
               else
                 if( c>0 )
                   Result = (Constant(1) / Constant(abs(a)).Root(2)) * Function("arcsin",
                     (Constant(abs(a)) / Constant(abs(c))).Root(2) * Variable(d) );
                 else
                   {
                   TExpr::sm_IntegralError = true;
                   Result=Clone();
                   }
             else
               {
               TExpr::sm_IntegralError = true;
               Result=Clone();
               }
          }
        else
          {
          TExpr::sm_IntegralError = true;
          return Clone();
          }
       return Result;
       }
    if( IsConst(a1,0) )
       Result = ( m_Operand1.Clone() ^ (b2 + Constant(1)) ) / ( (b1 * (b2.Clone() + Constant(1))) );
    else
      {
      a1 = Expand(this);
      if( a1.Eq(this) )
        {
        TExpr::sm_IntegralError = true;
        Result=Clone();
        }
      else
        {
        b1 = a1.Reduce();
        Result = b1.Integral(d);
        }
      }
   }
 else
   if( IsConst(a1,0) && IsConst(b1,0) )
     {
     if( c1.Constan(c) )
       {
       if( c<0 )
         {
         TExpr::sm_IntegralError = true;
         return Clone();
         }
       if( abs(c)<1E-6 )
         return Constant( 0 );
     }
   Result = (Constant(1)/a2) * Clone() * ( Constant(1) / Function("log", c1 ) );
   }
 else
  {
  TExpr::sm_IntegralError = true;
  return Clone();
  }
return Result;
}

MathExpr TPowr::Lim(const QByteArray& v, const MathExpr& lm) const
  {
  MathExpr op1, op2, op3, op4, rd, Result;
  bool nl1, nl2, un1, cn1, lg1, in2, od2, ng1, ng2;
  double vl;
  int n, dn;
  uchar cOper ;

  if( m_Operand1.Divis( op1, op2 ) && op1.HasUnknown(v) != "" &&
      op2.HasUnknown( v ) != "" && m_Operand2.HasUnknown( v ) != "" )
    {
    rd = m_Operand1.Divisor2Polinoms();
    if( rd.Binar( '=', op1, rd ) )
      {
      s_GlobalInvalid = false;
      rd = rd ^ m_Operand2.Clone();
      Result = rd.Lim( v, lm );
      if( !Result.IsEmpty() )
         Result =  new TBinar( '=',  new TLimit( false, rd.Clone(), Variable( v ),lm.Clone() ), Result );
      }
    return Result;
    }
  if( m_Operand1.Oper_( cOper, op1, op2 ) && IsConst( op1, 1 ) && ( cOper == '+' || cOper == '-' ) && op2.Divis( op3, op4 ) )
    {
    rd = op2.Lim( v, lm );
    if( !rd.IsEmpty() && IsConst( rd, 0 ) )
      {
      if( cOper == '-' )
        {
        op2 =  -op2;
        op3 =  -op3;
        }
      rd = Expand( op3 * m_Operand2) / op4;
      Result = rd.Lim( v, lm );
      s_GlobalInvalid = false;
      if( !Result.IsEmpty() )
        Result =  new TBinar( '=',  new TLimit( false, (( op1 + op2 )^( op4 / op3 ))^rd,
          Variable(v), lm ), Function( "exp", Result ) );
      return Result;
      }
    }
  un1 = false;
  cn1 = false;
  lg1 = false;
  in2 = false;
  od2 = false;
  op1 = m_Operand1.Lim(v,lm).Reduce();
  if( !op1.IsEmpty() )
    {
    cn1 = op1.Constan(vl) || op1.SimpleFrac_(n,dn);
    if( op1.Constan(vl) )
      if( vl == 1 && IsConstType( TInfinity, m_Operand2.Lim( v, lm ) ) )
        {
        rd = ( m_Operand1 - Constant(1)) * m_Operand2;
        op1 = rd.Reduce();
        Result = op1.Lim( v, lm );
        if( !Result.IsEmpty() )
          Result = new TBinar( '=',  new TLimit( false, ( m_Operand1^(Constant(1) /
            ( m_Operand1 - Constant(1))))^rd, Variable(v), lm ), Function( "exp", Result ) );
        else
          return Result;
        }
      else
        lg1 = vl > 1;
    else
      if( cn1 )
        lg1 = n > dn;
    rd = ReduceTExprs(op1.EVar2EConst());
    nl1 = IsConst(rd,0);
    un1 = IsConst(rd,1);
    }
  else
    nl1=false;
  op2 = m_Operand2.Lim(v,lm).Reduce();
  if( !op2.IsEmpty() )
    {
    rd = op2.Reduce();
    in2 = op2.Cons_int(n);
    od2 = n % 2 != 0;
    nl2 = IsConst(rd,0);
    }
  else
    nl2=false;
  if( op1.IsEmpty() )
    {
    if( op2.IsEmpty() )
      {
      s_GlobalInvalid = true;
      Result=Clone();
      }
    else
      if( !op2.Infinity(ng2) && !op2.Negative() )
        Result.Clear();
      else
        {
        s_GlobalInvalid = true;
        Result = Clone();
        }
    return Result;
    }
  else
    if( op2.IsEmpty() )
      {
      if( un1 )
        Result= Constant(1);
      else
        if( nl1 )
          Result= Constant(0);
        else
          if( !op1.Infinity(ng1) && !op1.Negative() )
            Result = op1;
          else
            {
            s_GlobalInvalid = true;
            Result = Clone();
            }
      return Result;
      }
  if( op1.Infinity(ng1) )
    if( op2.Infinity(ng2) )
      if( !ng1 )
        if( ng2 )
          Result = Constant(0);
        else
          Result = new TInfinity(false);
      else
        {
        s_GlobalInvalid = true;
        Result = Clone();
        }
  else
    if( nl2 )
      Result = Function( "exp", (m_Operand2 * Function("log", m_Operand1 ) ).Lim( v, lm) );
    else
      if( op2.Negative() )
        Result = Constant( 0 );
      else
        if( in2 )
          if( od2 )
            Result = new TInfinity(ng1);
          else
            Result = new TInfinity(false);
        else
          if( ng1 )
            {
            s_GlobalInvalid = true;
            Result = Clone();
            }
          else
            Result= new TInfinity(false);
  else
    if( op2.Infinity(ng2) )
      if( op1.Negative() )
        Result.Clear();
      else
        if( nl1 )
          {
          rd= Variable("e")^((m_Operand1 - Constant(1)) * m_Operand2);
          Result = rd.Lim(v,lm);
          }
        else
          if( un1 )
            Result=Function("exp", (m_Operand2 * Function( "log", m_Operand1 )).Lim(v,lm) );
          else
            if( cn1 )
              if( lg1^ng2 )
                Result= new TInfinity(false);
              else
                Result = Constant( 0 );
            else
              {
              s_GlobalInvalid=true;
              Result=Clone();
              }
    else
      if( nl1 && op2.Negative() )
        Result = new TInfinity(false);
      else
        Result = op1^op2;
  }

MathExpr TFunc::Integral(const QByteArray& d)
  {
  MathExpr  a, b;
  bool check;

  if( m_Arg.HasUnknown() != d )
    return Variable(d) * Clone();
  m_Arg.BinomX( a,b,check,d);
  if( !check )
    {
    sm_IntegralError = true;
    return Clone();
    }
  b.Clear();
  if( m_Name == "exp" )
     b = Clone();
  if(m_Name == "ln" )
    b = m_Arg.Clone() * Function( "ln", m_Arg.Clone() ) - m_Arg.Clone();
  if( m_Name == "sin" )
    b= -Function( "cos" , m_Arg.Clone() );
  if( m_Name == "cos" )
    b = Function("sin", m_Arg.Clone());
  if( m_Name == "tan" )
    b = -Function( "ln", new TAbs(false,Function( "cos", m_Arg.Clone() ) ) );
  if( m_Name == "cot" )
    b = Function("ln", new TAbs(false,Function( "sin", m_Arg.Clone() ) ) );
  if( m_Name == "sh" )
    b = Function( "ch", m_Arg.Clone() );
  if( m_Name == "ch" )
    b = Function( "sh", m_Arg.Clone() );
  if( b.IsEmpty() )
    {
    sm_IntegralError = true;
    return Clone();
    }
  if( !IsConst(a,1) )
    return (Constant(1)/a) * b;
  else
    return b;
  }

MathExpr TFunc::Lim(const QByteArray& v, const MathExpr& lm) const
  {
  MathExpr  elm,rd, Result;
  bool ng;

  elm = m_Arg.Lim(v,lm);
  if( m_Name == "exp" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        if( ng )
          Result = Constant( 0 );
        else
          Result= new TInfinity(false);
      else
        Result = Function("exp", elm );
  if( m_Name == "sin" || m_Name == "cos" )
    if( !elm.IsEmpty() && !elm.Infinity(ng) )
      Result = Function( m_Name, elm );
  if( m_Name == "tan" || m_Name == "cot" )
    if( !s_GlobalInvalid && !elm.IsEmpty() && !elm.Infinity(ng) )
      {
      rd = Function(  m_Name, elm );
      try
        {
        Result = rd.Reduce();
        }
      catch(...)
        {
        Result = new TInfinity(false);
        s_GlobalInvalid=false;
        }
      }
  if( m_Name == "sh" )
    if( !elm.IsEmpty() )
       if( elm.Infinity(ng) )
         Result = new TInfinity(ng);
       else
         Result = Function( m_Name, elm );
  if( m_Name == "log" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        Result= new TInfinity(ng);
      else
        {
        rd = elm.Reduce();
        if( IsConst(rd, 0) || rd.Negative() )
          Result = new TInfinity(true);
        else
          Result = Function( m_Name, elm );
        }
    else
      {
      s_GlobalInvalid = true;
      Result = Clone();
      }
  if( m_Name == "ch" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        Result = new TInfinity(false);
      else
        Result = Function( "ch", elm );
  if( m_Name=="th" || m_Name=="cth" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        if( ng )
          Result = Constant( -1 );
        else
          Result = Constant( 1 );
      else
        Result = Function( m_Name, elm );
  if( m_Name == "arcsin" || m_Name == "arccos" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        {
        s_GlobalInvalid = true;
        Result = Clone();
        }
    else
     {
     Result = Function( m_Name, elm );
     rd = Result.Reduce();
     if( s_GlobalInvalid )
       {
       Result = Clone();
       }
     }
  if( m_Name == "arctan" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        if( ng )
          Result = -(Variable( msPi ) / Constant(2));
        else
          Result = Variable( msPi ) / Constant(2);
      else
        Result = Function( "arctan", elm );
  if( m_Name == "arccot" )
    if( !elm.IsEmpty() )
      if( elm.Infinity(ng) )
        if( ng )
          Result = Variable( msPi );
        else
          Result = Constant( 0 );
      else
        Result = Function( "arccot", elm );
  if(!Result.IsEmpty()) Result = Result.Reduce();
  return Result;
  }
