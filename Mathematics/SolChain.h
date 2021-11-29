#ifndef SOLCHAIN
#define SOLCHAIN
#include "ExpOb.h"
#include <qlist.h>

class TSolutionChain
  {
  struct Pair
    {
    MathExpr m_Exprs;
    bool m_FinalResult;
    Pair( const MathExpr& Expr, bool FinRes ) : m_Exprs( Expr ), m_FinalResult( FinRes ) {}
    bool operator == ( const Pair& P ) const { return m_Exprs == P.m_Exprs; }
    };
  QList<Pair> m_Chains;
  QStringList m_Comments;
  bool m_Expanded;
  public:
    MATHEMATICS_EXPORT static TSolutionChain sm_SolutionChain;
    static bool sm_InterimResult;
    bool m_Accumulate;
    bool m_AddBinar;
    TSolutionChain();
    int AddExpr( const MathExpr& ex, const QString& msg = "", bool Main = false );
    int AddExpr( double Val, const QString& msg = "", bool Main = false );
    int AddAndReplace( const MathExpr& Target, const MathExpr& Source, bool Main = false );
    void DockTail( int Rest );
    MATHEMATICS_EXPORT MathExpr GetChain();
    int Count();
    MathExpr GetLastExpr();
    void Delete();
    MATHEMATICS_EXPORT void Clear();
    MathExpr GetExpr( int i );
    void Expand();
    void AddComment( const QString& s, int i = -1 );
    MATHEMATICS_EXPORT QString GetLastComment();
  };

#endif
