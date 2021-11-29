#include "SolChain.h"
#include "ExpObBin.h"
#include "../FormulaPainter/InEdit.h"
#include "MathTool.h"
#include "Parser.h"
#include "Algebra.h"

TSolutionChain TSolutionChain::sm_SolutionChain;
bool TSolutionChain::sm_InterimResult = false;

TSolutionChain::TSolutionChain() : m_Accumulate(false), m_Expanded(false), m_AddBinar(false)
  {
  }

int TSolutionChain::AddExpr( const MathExpr& ex, const QString& msg, bool Main )
  {
  if( !m_Accumulate ) return 0;
  for( auto pPair = m_Chains.begin(); pPair != m_Chains.end(); pPair++)
    if( pPair->m_Exprs.Eq(ex) ) return m_Chains.count();
  m_Chains.append( Pair( ex, Main && !sm_InterimResult ) );
  m_Comments.append( msg );
  return m_Chains.count();
  }

int TSolutionChain::AddExpr( double Val, const QString& msg, bool Main )
  {
  return AddExpr( Constant( Val ), msg, Main );
//  m_Comments.append( msg );
  }

int TSolutionChain::AddAndReplace( const MathExpr& Target, const MathExpr& Source, bool Main )
  {
  if( !m_Accumulate || m_Chains.count() == 0 ) return 0;
  MathExpr ex = m_Chains.last().m_Exprs;
  if( ex.Eq( Target ) )
    ex = Source;
  else
    ex.Replace( Target, Source );
  m_Chains.append( Pair( ex, Main && !sm_InterimResult ) );
  m_Comments.append( "" );
  return m_Chains.count();
  }

void TSolutionChain::DockTail( int Rest )
  {
  if( Rest == -1 ) return;
  if( m_Accumulate )
    {
    while( m_Chains.count() > Rest )
      {
      m_Chains.pop_back();
      m_Comments.pop_back();
      }
    return;
    }
  m_Chains.clear();
  m_Comments.clear();
  }

MathExpr TSolutionChain::GetChain()
  {
  m_Accumulate = false;
  if( m_Chains.count() == 0 ) return MathExpr();
  MathExpArray Final;
  auto ReturnFinal = [&] ( MathExpr& Result )
    {
    if( Final.isEmpty() ) return Result;
    bool FullReduce = TExpr::sm_FullReduce;
//    TExpr::sm_FullReduce = true;
    bool bWasChanged = false;
    for( auto pExpr = Final.begin(); pExpr != Final.end(); pExpr++ )
      {
      try
        {
        MathExpr Left, Right, Left1, Rigt1;
        if( pExpr->Binar( '=', Left, Right ) && Right.Binar( '=', Left1, Rigt1 ) )
          *pExpr = new TBinar( '=', Left, Rigt1 );
        pExpr->SetReduced( false );
        MathExpr ExpResult = pExpr->Reduce();
        bWasChanged = bWasChanged || !pExpr->Eq( ExpResult );
        *pExpr = ExpResult;
        }
      catch( ErrParser Err )
        {
        }
      }
    TExpr::sm_FullReduce = FullReduce;
    if( !bWasChanged ) return Result;
    Lexp LResult = CastPtr( TL2exp, Result );
    if( LResult.IsEmpty() )
      {
      LResult = new TL2exp;
      LResult.Addexp( Result );
      LResult.Last()->m_Visi = false;
      }
    for( auto pExpr = Final.begin(); pExpr != Final.end(); pExpr++ )
      {
      LResult.Addexp( *pExpr );
      LResult.Last()->m_Visi = false;
      }
    LResult.Addexp( new TStr(EdStr::sm_pCodec->fromUnicode( X_Str( "MAllNumbersExpr", "Solution set: all real numbers" ) ) ) );
    LResult.Last()->m_Visi = false;
    return MathExpr(LResult);
    };

  if( !m_AddBinar )
    {
    if( m_Chains.count() == 1 && m_Comments[0].isEmpty() ) return ReturnFinal( m_Chains[0].m_Exprs );
    Lexp List = new TL2exp;
    for( int i = 0; i < m_Chains.count(); i++ )
      {
      Pair &P = m_Chains[i];
      List.Addexp( P.m_Exprs );
      List.Last()->m_Visi = false;     
      if( P.m_FinalResult ) Final.append( P.m_Exprs );
      if( m_Comments[i].isEmpty() ) continue;
      List.Addexp( new TStr( FromLang( m_Comments[i] ) ) );
      List.Last()->m_Visi = false;
      }
    Solver::m_OldExpr.Clear();
    return ReturnFinal(List);
    }
  Lexp List;
  MathExpr exTmp;
  int iExpCount = 0;
  for( int i = 0; i < m_Chains.count(); i++ )
    {
    if( !m_Comments[i].isEmpty() )
      {
      if( List.IsEmpty() ) List = new TL2exp;
      if( exTmp.IsEmpty() )
        List.Addexp( m_Chains[i].m_Exprs );
      else
        List.Addexp( new TBinar( '=', exTmp, m_Chains[i].m_Exprs ) );
      List.Last()->m_Visi = false;
      if( m_Comments[i] != "$Break" )
        {
        List.Addexp( new TStr( FromLang( m_Comments[i] ) ) );
        List.Last()->m_Visi = false;
        }
      iExpCount = 0;
      exTmp.Clear();
      }
    else
      {
      if( exTmp.IsEmpty() )
        if( i == m_Chains.count() - 1 && !List.IsEmpty() )
          List.Addexp( m_Chains[i].m_Exprs );
        else
          exTmp = m_Chains[i].m_Exprs;
      else
        exTmp = new TBinar( '=', exTmp, m_Chains[i].m_Exprs );
      iExpCount = iExpCount + 1;
      if( iExpCount == 3 && i < m_Chains.count() - 1 )
        {
        exTmp = new TBinar( '=', exTmp, new TNewLin );
        iExpCount = 0;
        }
      }
    if( m_Chains[i].m_FinalResult ) Final.append( m_Chains[i].m_Exprs );
    }
  if( List.IsEmpty() ) return ReturnFinal( exTmp );
  if( !exTmp.IsEmpty() )
    {
    List.Addexp( exTmp );
    List.Last()->m_Visi = false;
    }
  Solver::m_OldExpr.Clear();
  return ReturnFinal( List );
  }
  
void TSolutionChain::Delete()
  {
  DockTail( 0 );
  m_Accumulate = false;
  }

void TSolutionChain::Clear()
  {
  if( !m_Chains.isEmpty() && !Solver::m_OldExpr.IsEmpty() && Solver::m_OldExpr.Eq(m_Chains[0].m_Exprs ) ) return;
  DockTail( 0 );
  m_Accumulate = true;
  m_Expanded = false;
  }

int TSolutionChain::Count()
  {
  return m_Chains.count();
  }

MathExpr TSolutionChain::GetLastExpr()
  {
  return m_Chains.last().m_Exprs;
  }

MathExpr TSolutionChain::GetExpr( int i )
  {
  return m_Chains[i].m_Exprs;
  }

void TSolutionChain::Expand()
  {
  if( m_Expanded || ( m_Chains.count() == 0 ) ) return;
  m_Chains.last().m_Exprs = ExpandExpr( m_Chains.last().m_Exprs );
  m_Expanded = true;
  }

void TSolutionChain::AddComment( const QString& s, int i )
  {
  if( !m_Accumulate || m_Chains.count() == 0 ) return;
  if( i == -1 )
    m_Comments[m_Chains.count() - 1] = s;
  else
    m_Comments[i] = s;
  }

QString TSolutionChain::GetLastComment()
  {
  if(m_Comments.isEmpty()) return "";
  QString Result = m_Comments.last();
//  m_Comments.clear();
  return Result;
  }
