#include "ExpStore.h"
#include "Parser.h"

ExpStore s_ExpStore;
ExpStore* ExpStore::sm_pExpStore;

bool ExpStore::Known_var_expr( const QByteArray& N, MathExpr& E )
  {
  MathExpr Expr;
  if( s_MemorySwitch == SWtask )
    Expr = m_Task_vars[N];
  if( s_MemorySwitch == SWcalculator )
    Expr = m_Calculator_vars[N];
  if( Expr.IsEmpty() ) return false;
  E = Expr.Perform();
  return true;
  }

int ExpStore::GetValue( const QByteArray& Info )
  {
  MathExpr body;
  int Result;
  if( Info[0] == msMetaSign )
    {
    if( !(Known_var_expr( Info.mid( 1 ), body ) && body.Cons_int( Result ) ) )
      throw  ErrParser( "Syntactical error", peSyntacs );
    return Result;
    }
  bool OK;
  Result = Info.toInt( &OK );
  if( !OK ) throw  ErrParser( "Syntactical error", peSyntacs );
  return Result;
  }

void ExpStore::Store_var( const QByteArray& N, const MathExpr& E, bool TrackSelected )
  {
  if( E.IsEmpty() ) return;
  QByteArray tmp = E.WriteE();
  if( tmp == N || tmp == ( char ) msMetaSign + N ) return;
  s_WasDefinition = true;
  if( s_MemorySwitch == SWtask )
    {
    MathExpr body = m_Task_vars[N];
//    if( body.IsEmpty() || s_pXPTask->m_pTrack->m_MultyTrack && s_pXPTask->m_pTrack->m_TrackSelected )
    if( body.IsEmpty() || TrackSelected )
      m_Task_vars[N] = E;
    }
  if( s_MemorySwitch == SWcalculator )
    m_Calculator_vars[N] = E;
  }

bool ExpStore::Known_var( const QByteArray& N, bool IsDouble)
  {
  if( s_MemorySwitch == SWtask )
    return m_Task_vars.contains( N );
  if( s_MemorySwitch == SWcalculator )
    return m_Calculator_vars.contains( N ) && !IsDouble;
  return false;
  }

MathExpr ExpStore::Restore_var_expr( const QByteArray& N )
  {
  MathExpr body;
  if( s_MemorySwitch == SWtask )
    body = m_Task_vars[N];
  if( s_MemorySwitch == SWcalculator )
    body = m_Calculator_vars[N];
  if( body.IsEmpty() ) return body;
  return body.Perform();
  }
