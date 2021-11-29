#include "ExpObBin.h"
#include "ExpStore.h"

extern ExpStore s_ExpStore;
bool TBinar::sm_NoBoolReduce = false;

TBinar::TBinar( uchar Name, const MathExpr& ex1, const MathExpr& ex2 ) : TOper( ex1, ex2, Name )
  {
  MathExpr Left = m_Operand1;
  TBinar *pBinLeft;
  for( TBinar *pBin = pBinLeft = CastPtr( TBinar, Left ); (pBin = CastPtr( TBinar, Left ) ) != nullptr; Left = pBin->m_Operand1, pBinLeft = pBin )
    if( pBin->m_Name != Name ) return;
  if( Left == m_Operand1 ) return;
  m_Operand1 = pBinLeft->m_Operand1;
  TBinar *pBinRight = ( TBinar*) CastConstPtr( TBinar, ex1 );
  pBinLeft->m_Operand1 = pBinRight->m_Operand2;
  if( pBinLeft == pBinRight )
    {
    pBinLeft->m_Operand2 = ex2;
    m_Operand2 = pBinLeft;
    return;
    }
  m_Operand2 = pBinLeft->m_Operand2;
  pBinLeft->m_Operand2 = ex2;
  pBinRight->m_Operand2 = pBinLeft;
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TBinar::Clone() const
  {
  return new TBinar( m_Name, m_Operand1.Clone(), m_Operand2.Clone() );
  }

MathExpr TBinar::Reduce() const
  {
  if((uchar)  m_Name == msIdentity )
    return new TBool( m_Operand1.Equal( m_Operand2 ) );
  MathExpr exOp;
  double V1, V2;
  MathExpr op1_reduced = m_Operand1.Reduce();
  MathExpr op2_reduced = m_Operand2.Reduce();

  if( m_Name == msPlusMinus && op2_reduced.Constan( V2 ) && V2 == 0 )
    return op1_reduced;

  if( op1_reduced.ConstExpr() && op2_reduced.ConstExpr() )
    {
    if( s_NoRootReduce || s_NoLogReduce )
      {
      bool OldRootReduce = s_NoRootReduce;
      bool OldLogReduce = s_NoLogReduce;
      s_NoRootReduce = false;
      s_NoLogReduce = false;
      exOp = op1_reduced.Reduce();
      op1_reduced = exOp;
      exOp = op2_reduced.Reduce();
      op2_reduced = exOp;
      s_NoRootReduce = OldRootReduce;
      s_NoLogReduce = OldLogReduce;
      }
    int N1, D1, N2, D2;
    if( ( op1_reduced.Constan( V1 ) || op1_reduced.SimpleFrac_( N1, D1 ) ) && ( op2_reduced.Constan( V2 ) || op2_reduced.SimpleFrac_( N2, D2 ) ) )
      {
      if( op1_reduced.SimpleFrac_( N1, D1 ) )
        V1 = ( double ) N1 / D1;
      if( op2_reduced.SimpleFrac_( N2, D2 ) )
        V2 = ( double ) N2 / D2;
      if( sm_NoBoolReduce )
        return new TBinar(m_Name, op1_reduced, op2_reduced);
      if( m_Name == '=' )
        return new TBool( abs( V1 - V2 ) < 10 * s_Precision );
      if( m_Name == '<' )
        return new TBool( V1 < V2 );
      if( m_Name == '>' )
        return new TBool( V1 > V2 );
      if( m_Name == msNotequal )
        return new TBool( V1 != V2 );
      if( m_Name == msMaxequal )
        return new TBool( V1 >= V2 );
      if( m_Name == msMinequal )
        return new TBool( V1 <= V2 );
      }
    }

  MathExpr exOp1, exOp2;
  bool CheckNameChange = false;
  if( op1_reduced.Unarminus( exOp1 ) )
    if( op2_reduced.Unarminus( exOp2 ) )
      {
      op1_reduced = exOp1;
      op2_reduced = exOp2;
      CheckNameChange = true;
      }
    else
      if( op2_reduced.Constan( V1 ) )
        {
        op1_reduced = exOp1;
        op2_reduced = Constant( -V1 );
        CheckNameChange = true;
        }

  if( op2_reduced.Unarminus( exOp2 ) && op1_reduced.Constan( V1 ) )
    {
    op2_reduced = exOp2;
    op1_reduced = Constant( -V1 );
    CheckNameChange = true;
    }

  char NewName = m_Name;
  if( CheckNameChange )
    switch( m_Name )
    {
      case '<':
        NewName = '>';
        break;
      case '>':
        NewName = '<';
        break;
      case msMaxequal:
        NewName = msMinequal;
        break;
      case msMinequal:
        NewName = msMaxequal;
        break;
    }
  return new TBinar( NewName, op1_reduced, op2_reduced );
  }

bool TBinar::Eq( const MathExpr& E2 ) const
  {
  MathExpr  op21, op22;
  if( !E2.Binar( m_Name, op21, op22 ) ) return false;
  return m_Operand1.Eq( op21 ) && m_Operand2.Eq( op22 );
  }

MathExpr TBinar::Perform()
  {
  return new TBinar( m_Name, m_Operand1.Perform(), m_Operand2.Perform() );
  }

MathExpr TBinar::Diff( const QByteArray& d ) 
  {
  s_GlobalInvalid = true;
  return new TBinar( m_Name, m_Operand1.Diff( d ), m_Operand2.Diff( d ) );
  }

MathExpr TBinar::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TBinar( m_Name, m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

bool TBinar::Binar( char N, MathExpr& op1, MathExpr& op2 ) const
  {
  if( N != m_Name ) return false;
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TBinar::Binar_( uchar& N, MathExpr& op1, MathExpr& op2 ) const
  {
  N = m_Name;
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TBinar::Infinity( bool& ANeg )
  {
  return m_Name == '=' && m_Operand2.Infinity( ANeg );
  }

bool TBinar::Equal( const MathExpr& E2 ) const
  {
  if( E2.IsEmpty() )
    return false;
  if( E2.Polygon() )
    return false;
  MathExpr  Ar1, Ar2;
  QByteArray _eqName;
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );

  MathExpr op21, op22;
  if( E2.Summa( op21, op22 ) || E2.Subtr( op21, op22 ) || E2.Unarminus( op21 ) )
    return E2.Equal( Ethis );

  if( E2.Binar( m_Name, op21, op22 ) )
    {
    if( m_Operand1.Equal( op21 ) && m_Operand2.Equal( op22 ) )
      return true;

    if( ( m_Name == '=' || m_Name == msNotequal ) && m_Operand1.Equal( op22 ) && m_Operand2.Equal( op21 ) )
      return true;
    }
  
  uchar N;
  if( ( m_Name == '<' || m_Name == '>' ) && E2.Binar_( N, op21, op22 ) && ( N == '<' || N == '>' ) )
    {
    Ar1 = m_Operand1;
    if( m_Operand2 != 0 )
      Ar1 = m_Operand1 - m_Operand2;
    Ar2 = op21;
    if( op22 != 0 )
      if( N == m_Name )
        Ar2 = op21 - op22;
      else
        Ar2 = -(op21 - op22);
    else
      if( N != m_Name )
        Ar2 = -op21;
    return Ar1.Equal( Ar2 );
    }

  Ar1 = SimpleBinar();
  Ar2 = E2.SimpleBinar();

  MathExpr lf, ri;
  op21 = Ar1.ExpandChain( lf, ri );
  op22 = Ar2.ExpandChain( lf, ri );

  if( !Eq( op21 ) || !op22.Eq( E2 ) )
    return op21.Equal( op22 );

  return false;
  }

bool TBinar::operator < ( const MathExpr& Expr )
  {
  if( Expr.IsEmpty() || m_Name != '=' ) return false;
  MathExpr exLeft, exRight;
  uchar Op;
  if( !Expr.Binar_( Op, exLeft, exRight ) || Op != '=' ) return false;
  return m_Operand1->WriteE() < exLeft.WriteE();
  }

QByteArray TBinar::SWrite() const
  {
  QByteArray Name(charToTex(m_Name));
  if (Name[0] == '\\') Name += '\n';
  return m_Operand1.SWrite() + Name + m_Operand2.SWrite();
  }
