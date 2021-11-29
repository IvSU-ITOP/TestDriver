#ifndef MATH_EXPSTORE
#define MATH_EXPSTORE
#include "ExpObBin.h"

typedef QMap<QByteArray, MathExpr> Mem_table;

class ExpStore
  {
  Mem_table m_Task_vars;
  Mem_table m_Calculator_vars;
  public:
    MATHEMATICS_EXPORT bool Known_var_expr( const QByteArray& N, MathExpr&E );
    MATHEMATICS_EXPORT static ExpStore* sm_pExpStore;
    ExpStore() { sm_pExpStore = this;  }
    MATHEMATICS_EXPORT int GetValue( const QByteArray& Info );
    MATHEMATICS_EXPORT void Init_var() { m_Task_vars.clear(); m_Calculator_vars.clear(); }
    MATHEMATICS_EXPORT void Store_var( const QByteArray& N, const MathExpr& E, bool TrackSelected = false );
    MATHEMATICS_EXPORT bool Known_var( const QByteArray& N, bool IsDouble = false );
    MathExpr Restore_var_expr( const QByteArray& N );
    MATHEMATICS_EXPORT void Clear() { m_Task_vars.clear(); }
  };

#endif