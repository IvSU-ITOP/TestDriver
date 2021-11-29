#include "ExpOb.h"
#include "ExpObBin.h"
#include "ExpObjOp.h"
#include "MathTool.h"
#include "../FormulaPainter/Messtrs.h"
#include "Globalvars.h"
#include "Statistics.h"
#include "Parser.h"
#include "ExpStore.h"
#include "SolChain.h"
#include "LogExp.h"
#include "Factors.h"
#include <functional>
#include "../FormulaPainter/InEdit.h"

extern ExpStore s_ExpStore;


double TExpr::sm_Accuracy = 0.01;
double TExpr::sm_Precision = 0.0000000001;
bool TExpr::sm_FullReduce = false;
bool TPowr::sm_Factorize = true;
bool TConstant::sm_ConstToFraction = false;
TExpr::TrigonomSystem TExpr::sm_TrigonomSystem = tsRad;
bool TOper::sm_InsideChart = false;
bool TLexp::sm_Bracketed = true;

double TExpr::TriginomValue( double V ) { return sm_TrigonomSystem == tsRad ? V : DegToRad( V ); }
double TExpr::AngleValue( double V ) { return sm_TrigonomSystem == tsRad ? V : RadToDeg( V ); }
int TExpr::sm_CreatedCount = 0;
int TExpr::sm_DeletedCount = 0;
ResultReceiver* TExpr::sm_pResultReceiver = nullptr;
QList<TExpr*> TExpr::sm_CreatedList;

bool MathExpr::sm_NoReduceByCompare = true;
bool MathExpr::sm_NewReduce = false;

int TExMemb::sm_ExMembCreated = 0;
int TExMemb::sm_ExMembDeleted = 0;
int TMatr::sm_RecursionDepth = 0;
bool TStr::sm_Server = false;
bool TStr::sm_Encode = false;

double TExpr::Precision()
  {
//  if( s_MemorySwitch == SWtask )
//    if( sm_Accuracy == 0 )
      return sm_Precision;
//    else
//      return sm_Accuracy;
//    return s_Precision;
  }

double TExpr::ResetPrecision( double newPrec )
  {
  if( s_MemorySwitch == SWtask )
    {
    double OldPrec = sm_Precision;
    sm_Precision = newPrec;
    return OldPrec;
    }
  double OldPrec = s_Precision;
  s_Precision = newPrec;
  return OldPrec;
  }

MathExpr TExpr::GenerateFraction( int N, int D )
  {
  if( abs( N ) < ( long long ) INT_MAX + 1 && abs( D ) < ( long long ) INT_MAX + 1 )
    return new TSimpleFrac( N, D );
  else
    return new TDivi( new TConstant( N ), new TConstant( D ) );
  }

MathExpr TExpr::EVar2EConst() 
  {
  return Substitute( "e", Constant( M_E ) );
  }

QByteArray TExpr::WriteEB() const
  {
  QByteArray Result = WriteE();
  if( Result[0] == '(' )
    return Result.mid( 1, Result.length() - 2 );
  return Result;
  }

bool MathExpr::Replace( const MathExpr& Target, const MathExpr& Source ) 
  { 
  TestPtr(); 
  if( m_pExpr->m_Counter > 1 )
    {
    m_pExpr->m_Counter--;
    MathExpr Tmp = m_pExpr->Clone();
    m_pExpr = Tmp.Ptr();
    m_pExpr->m_Counter++;
    }
  Source.m_pExpr->m_WasReduced = false;
  m_pExpr->m_WasReduced = false;
  return m_pExpr->Replace( Target, Source ); 
  }

MathExpr& MathExpr::operator = ( const MathExpr& E )
  {
  if( m_pExpr != nullptr && --m_pExpr->m_Counter == 0 ) delete m_pExpr;
  m_pExpr = E.m_pExpr;
  if( m_pExpr != nullptr )
    {
    m_pExpr->m_Counter++;
#ifdef DEBUG_TASK
    m_Contents = m_pExpr->WriteE();
#endif
    }
  return *this;
  }

bool MathExpr::operator == ( int Value ) const
  {
  TestPtr();
  int ValInt;
  if( m_pExpr->Cons_int( ValInt ) )
    return ValInt == Value;
  MathExpr exi, exm;
  if( (m_pExpr->Measur_( exi, exm )) )
    return exi.Cons_int( ValInt ) && ValInt == Value;
  return false;
  }

void MathExpr::TestPtr() const
  { 
  if( m_pExpr == nullptr )
    throw ErrParser( X_Str( "MNullPtr", "Was null pointer of expression!" ), peNullptr);
  }

/*
#ifdef DEBUG_TASK
MathExpr::MathExpr( const MathExpr& E ) : m_pExpr( E.m_pExpr )
  {
  if( m_pExpr != nullptr ) m_pExpr->m_Counter++;
  m_Contents = m_pExpr->WriteE();
  }

MathExpr::MathExpr( TExpr* pE ) : m_pExpr( pE )
  {
  if( m_pExpr != nullptr ) m_pExpr->m_Counter++;
  m_Contents = m_pExpr->WriteE();
  }
#endif
*/
bool MathExpr::SimpleFrac_( double& N, double& D ) const
  {
  TestPtr();
  int iN, iD;
  bool Result = m_pExpr->SimpleFrac_( iN, iD );
  N = iN;
  D = iD;
  return Result;
  }

MathExpr MathExpr::SimplifyFull()
  {
  TestPtr();
  bool FR = TExpr::sm_FullReduce;
  TExpr::sm_FullReduce = true;
  MathExpr Result = m_pExpr->Reduce();
  TExpr::sm_FullReduce = FR;
  return Result;
  }

bool MathExpr::operator < ( const MathExpr& Expr ) const
  {
  if( m_pExpr == nullptr ) return true;
  return ( *m_pExpr ) < Expr;
  }

bool MathExpr::Eq( const MathExpr& E2 ) const
  {
  TestPtr();
  if(m_Contents == E2.m_Contents) return true;
  return m_pExpr->Eq( E2 );
  }

bool MathExpr::Equal( const MathExpr& E2 ) const
  {
  TestPtr();
  if(m_Contents == E2.m_Contents) return true;
  bool bResult = m_pExpr->Eq( E2 );
  if( bResult ) return true;
  bResult = m_pExpr->Equal( E2 );
  if( bResult || sm_NoReduceByCompare ) return bResult;
  if( m_pExpr->m_WasReduced && E2.m_pExpr->m_WasReduced ) return false;
  MathExpr Reduced = Reduce();
  MathExpr ReducedE = E2.Reduce();
  if( Eq( Reduced ) && E2.Eq( ReducedE ) ) return false;
  return Reduced.Equal( ReducedE );
  }

bool MathExpr::IsNumerical() const
  {
  TestPtr();
  m_pExpr->SetReduced( false );
  bool OldFullReduce = TExpr::sm_FullReduce;
  TExpr::sm_FullReduce = true;
  MathExpr Ex = m_pExpr->Reduce();
  TExpr::sm_FullReduce = OldFullReduce;
  return IsConstType( TConstant, Ex );
  }

MathExpr MathExpr::Reduce(bool NewReduce) const
  {
  if( m_pExpr == nullptr ) return nullptr;
  if(NewReduce) sm_NewReduce = true;
  if( m_pExpr->m_WasReduced && !sm_NewReduce) return m_pExpr;
  MathExpr Result = m_pExpr->Reduce();
  if(NewReduce) sm_NewReduce = false;
  if( Result.IsEmpty() ) return Result;
  Result.m_pExpr->m_WasReduced = true;
  return Result;
  }

QByteArray TStr::UnpackValue( const QByteArray& V )
  {
  if( !sm_Server || V.left(2) != "##" ) return V;
  bool bOK;
  int Count = V.mid(2,4).toInt( &bOK, 16 );
  if( !bOK ) return V;
  ushort *pS = new ushort[Count];
  for( int i = 0, n = 6; i < Count; i++, n += 4 )
    {
    pS[i] = V.mid( n, 4 ).toInt( &bOK, 16 );
    if( !bOK )
      {
      delete pS;
      throw ErrParser( "Error; Invalid String", ParserErr::peNewErr );
      }
    }
  QString S;
  S.setUtf16( pS, Count );
  delete pS;
  return FromLang( S );
  }

TStr::TStr( const QByteArray& V ) : TExpr(), m_Value(UnpackValue(V)) {}

QByteArray TStr::PackValue() const
  {
  if( !sm_Server )
    if( sm_Encode )
      return Encode( m_Value );
    else
      return m_Value;
  QString S( ToLang( m_Value ) );
  const ushort *pS = S.utf16();
  int Count = S.length();
  char Buf[6];
  QByteArray Val( itoa( Count, Buf, 16 ) );
  if( Val.length() < 4 ) Val = QByteArray( 4 - Val.length(), '0' ) + Val;
  QByteArray Result = "##" + Val;
  for( int i = 0; i < Count; i++ )
    {
    QByteArray Val( itoa( pS[i], Buf, 16 ) );
    if( Val.length() < 4 ) Val = QByteArray( 4 - Val.length(), '0' ) + Val;
    Result += Val;
    }
  return Result;
  }

QByteArray TStr::SWrite() const 
  {
  QByteArray Result("\\comment{");
  if (sm_Server) return Result + PackValue() + '}';
  QByteArrayList L(m_Value.split('\n') );
  for (int i = 0; i < L.count(); i++)
    {
    Result += L[i];
    if (i != L.count() - 1) Result += "\\newline";
    }
  return Result + '}'; 
  }

bool TStr::Eq( const MathExpr& E2 ) const
  {
  TStr const *pS = dynamic_cast< const TStr* >( E2() );
  if( pS == nullptr ) return false;
  return pS->Value() == m_Value;
  }

TLexp::~TLexp()
  {
  Clear();
  }

void TLexp::Clear()
  {
  PExMemb pFirst;
  pFirst.swap( m_pFirst );
  for( PExMemb pNext; !pFirst.isNull(); pNext.swap( pFirst ) )
    {
    pFirst->m_pPrev.clear();
    pNext.swap( pFirst->m_pNext );
    pFirst.clear();
    }
  }

MathExpr TLexp::Clone() const
  {
  TLexp *pLIST = new TLexp;
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    pLIST->Addexp( index->m_Memb.Clone() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    }
  pLIST->m_WasReduced = m_WasReduced;
  return pLIST;
  }

MathExpr TLexp::Reduce() const
  {
  TLexp *pLIST = new TLexp;
  PExMemb index = m_pFirst;
  while( !index.isNull() )
    {
    pLIST->Addexp( index->m_Memb.Reduce() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    index = index->m_pNext;
    }
  return pLIST;
  }

void TLexp::SetReduced( bool Reduced )
  {
  TExpr::SetReduced( Reduced );
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    index->m_Memb.SetReduced( Reduced );
  }

MathExpr TLexp::Perform() const
  {
  TLexp *pLIST = new TLexp;
  PExMemb index = m_pFirst;
  while( !index.isNull() )
    {
    pLIST->Addexp( index->m_Memb.Perform() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    index = index->m_pNext;
    }
  return pLIST;
  }

MathExpr TLexp::Diff( const QByteArray& d )
  {
  TLexp *pLIST = new TLexp;
  PExMemb index = m_pFirst;
  while( !index.isNull() )
    {
    pLIST->Addexp( index->m_Memb.Diff(d) );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    index = index->m_pNext;
    }
  return pLIST;
  }

bool TLexp::Eq( const MathExpr& E2 ) const
  {
  PExMemb index1, index2;
  if( !(E2->Listex( index2 ) || E2->Listord( index2 ) ) ) return false;
  if( m_Count != CastConstPtr( TLexp, E2 )->m_Count ) return false;
  for( index1 = m_pFirst; !index1.isNull(); index1 = index1->m_pNext, index2 = index2->m_pNext )
    {
    if( index2.isNull() ) return false;
    if( !index1->m_Memb->Eq( index2->m_Memb ) ) return false;
    }
  return index2.isNull();
  }

bool TLexp::Equal( const MathExpr& E2 ) const
  {
  PExMemb index1, index2;
  if( E2->Listord( index2 ) ) return E2->Equal( Ethis );
  if( !E2->Listex( index1 ) ) return false;
  if( m_Count != CastConstPtr( TLexp, E2 )->m_Count ) return false;
  MathExpr L2 = E2.Reduce();
  MathExpr L = Reduce();
  if (L2.Eq(L)) return true;
  index1 = dynamic_cast<TLexp*>(L.Ptr())->m_pFirst;
  bool status = true;
  for( ; status && !index1.isNull(); index1 = index1->m_pNext )
    {
    status = L2->FindEqual( index1->m_Memb, index2 );
    if( status ) dynamic_cast< TLexp*>( L2.Ptr() )->DeleteMemb( index2 );
    }
  L2->Listex( index2 );
  if( !index2.isNull() ) return false;
  return status;
  }

MathExpr TLexp::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  TLexp *pResult = new TLexp;
  for( PExMemb p = m_pFirst; !p.isNull(); p = p->m_pNext )
    {
    pResult->Addexp( p->m_Memb.Substitute( vr, vl ) );
    pResult->m_pLast->m_Visi = p->m_Visi;
    }
  return pResult;
  }

QByteArray TLexp::WriteE() const
  {
  QByteArray S;
  if(sm_Bracketed) S = "(";
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    S += index->m_Memb->WriteE();
    if( !index->m_pNext.isNull() )
      if( index->m_Visi )
        S += ',';
      else
        {
        S += msMetaSign;
        S += ',';
        }
    }
  if(sm_Bracketed) S += ')';
  return S;
  }

QByteArray TLexp::SWrite() const
  {
  QByteArray S;
  for( PExMemb index = m_pFirst, ind2; !index.isNull(); index = index->m_pNext )
    {
    if( index->m_Memb.Listex( ind2 ) || index->m_Memb.Listord( ind2 ) )
      S += '(' + index->m_Memb.SWrite() + ')';
    else
      {
      if( index->m_Spacer ) S += "\\spacer\n";
      S += index->m_Memb.SWrite();
      if( index->m_Spacer && index->m_pNext == nullptr ) S += "\\spacer\n";
      }
    if( !index->m_pNext.isNull() && !index->m_Memb.Newline() )
      {
      if( index->m_Visi )
        S += "\\setvisible\n";
      else
        S += "\\setunvisible\n";
      S += ",";
      }
    }
  return S;
  }

bool TLexp::Splitted() const 
  {
  bool Splitmemb = false;
  for( PExMemb index = m_pFirst; !index.isNull() && !Splitmemb; index = index->m_pNext )
    if( !index->m_Memb.IsEmpty() )
      Splitmemb = Splitmemb || index->m_Memb.Splitted();
  return Splitmemb;
  }

bool TLexp::ConstExpr() const
  {
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    if( index->m_Memb.IsEmpty() || !index->m_Memb.ConstExpr() ) return false;
  return true;
  }

bool TLexp::FindEq( const MathExpr& E, PExMemb& F )
  {
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    if( E->Eq( index->m_Memb ) )
      {
      F = index;
      return true;
      }
  return false;
  }

bool TLexp::FindEqual( const MathExpr& E, PExMemb& F ) const
  {
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    if( E->Equal( index->m_Memb ) )
      {
      F = index;
      return true;
      }
  return false;
  }

void TLexp::Addexp( const MathExpr& A )
  {
  PExMemb index = m_pLast;
  m_pLast = PExMemb( new TExMemb( m_pLast, A ) );
  if( m_pFirst.isNull() )
    m_pFirst = m_pLast;
  if( !index.isNull() )
    index->m_pNext = m_pLast;
  m_Count++;
#ifdef DEBUG_TASK 
  m_Contents = WriteE();   
#endif
  }

void TLexp::Appendz( const MathExpr& A )
  {
  if( A() == nullptr ) return;
  PExMemb index;
  if( A->Listex( index ) || A->Listord( index ) )
    for( ; !index.isNull(); index = index->m_pNext )
      {
      Addexp( index->m_Memb );
      m_pLast->m_Visi = index->m_Visi;
      index->m_Memb.Clear();
      }
  else
    Addexp( A );
#ifdef DEBUG_TASK 
  m_Contents = WriteE();
#endif
  }

void TLexp::DeleteMemb( PExMemb& M )
  {
  if( !M->m_pPrev.isNull() ) M->m_pPrev->m_pNext = M->m_pNext;
  if( !M->m_pNext.isNull() ) M->m_pNext->m_pPrev = M->m_pPrev;
  if( m_pFirst == M ) m_pFirst = M->m_pNext;
  if( m_pLast == M ) m_pLast = M->m_pPrev;
  m_Count--;
#ifdef DEBUG_TASK 
  m_Contents = WriteE();
#endif
  }

/*
MathExpr Lexp::CreateObject()
  {
  if( First().isNull() || Last().isNull() )
    throw ErrParser( "Invalid Argument!", peSyntacs );
  QByteArray sEditor = First()->m_Memb->WriteE();
  MathExpr Result;
  if( sEditor == "GraphEdit" )
    Result = Panel::sm_pGraphEdit->RestoreObject( Last()->m_Memb->WriteE() );
  if( Result.IsEmpty() )
    throw ErrParser( "Invalid Argument!", peSyntacs );
  return Result;
  }
  */

PExMemb& Lexp::First()
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->m_pFirst;
  }

PExMemb& Lexp::Last()
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->m_pLast;
  }

bool Lexp::FindEq( const MathExpr& E, PExMemb& F )
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->FindEq( E, F );
  }

bool Lexp::FindEqual( const MathExpr& E, PExMemb& F )
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->FindEqual( E, F );
  }

void Lexp::Addexp( const MathExpr& A )
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->Addexp( A );
  }

void Lexp::Appendz( const MathExpr& A )
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->Appendz( A );
  }

void Lexp::DeleteMemb( PExMemb& M )
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->DeleteMemb( M );
  }

int Lexp::Count()
  {
  TestPtr();
  return Cast( TLexp, m_pExpr )->Count();
  }

MathExpr TLord::Clone() const
  {
  TLord *pLIST = new TLord;
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    pLIST->Addexp( index->m_Memb->Clone() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    }
  pLIST->m_WasReduced = m_WasReduced;
  return pLIST;
  }

MathExpr TLord::Reduce() const
  {
  TLord *pLIST = new TLord;
  PExMemb index = m_pFirst;
  while( !index.isNull() )
    {
    pLIST->Addexp( index->m_Memb.Reduce() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    index = index->m_pNext;
    }
  return pLIST;
  }

MathExpr TLord::Perform() const
  {
  TLord *pLIST = new TLord;
  PExMemb index = m_pFirst;
  while( !index.isNull() )
    {
    pLIST->Addexp( index->m_Memb.Perform() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    index = index->m_pNext;
    }
  return pLIST;
  }

bool TLord::Equal( const MathExpr& E2 ) const
  {
  PExMemb index1, index2;
  if( !E2->Listex( index2 ) || !E2->Listord( index2 ) ) return false;
  for( index1 = m_pFirst; !index1.isNull(); index1 = index1->m_pNext, index2 = index2->m_pNext )
    {
    if( index2.isNull() ) return false;
    if( !index1->m_Memb->Equal( index2->m_Memb ) ) return false;
    }
  return index2.isNull();
  }

MathExpr TL2exp::Clone() const
  {
  TL2exp *pLIST = new TL2exp;
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    pLIST->Addexp( index->m_Memb.Clone() );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    }
  pLIST->m_WasReduced = m_WasReduced;
  return pLIST;
  }

MathExpr TL2exp::Reduce() const
  {
  TL2exp *pLIST = new TL2exp;
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    MathExpr Temp = index->m_Memb.Reduce();
    MathExpr ExLeft, ExRight;
    if( index->m_Memb->Binar( '=', ExLeft, ExRight ) )
      if( Temp.Binar()->m_Operand2->Binar( '=', ExLeft, ExRight ) )
        pLIST->Addexp( ExRight );
      else
        pLIST->Addexp( Temp );
    else
      if( Temp->Binar( '=', ExLeft, ExRight ) )
        pLIST->Addexp( ExRight );
      else
        pLIST->Addexp( Temp );
    pLIST->m_pLast->m_Visi = index->m_Visi;
    }
  return pLIST;
  }
  
MathExpr TL2exp::Perform() const
  {
  TL2exp *retexp = new TL2exp;
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    retexp->Addexp( index->m_Memb.Perform() );
    retexp->m_pLast->m_Visi = index->m_Visi;
    }

  return retexp;
  }

QByteArray TL2exp::WriteE() const
  {
  QByteArray S;
  if(sm_Bracketed) S = "(";

  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    S += index->m_Memb.WriteE();
    if( !( index->m_pNext.isNull() ) )
      if( index->m_Visi )
        S += ';';
      else
        S = S + ( char ) msMetaSign + ';';
    }
  if(sm_Bracketed) S += ')';
  return S;
  }

QByteArray TL2exp::SWrite() const
  {
  QByteArray S;
  for( PExMemb index = m_pFirst; !index.isNull(); index = index->m_pNext )
    {
    QByteArray Add = index->m_Memb.SWrite();
    if( Add.isEmpty() ) continue;
    S += Add;
    if( !( index->m_pNext.isNull() ) )
      {
      if( index->m_Visi )
        S += "\\setvisible\n";
      else
        S += "\\setunvisible\n";
      S += ';';
      }
    S += "\\setvisible\n";
    }
  return S;
  }

bool TL2exp::List2ex( PExMemb& F ) const
  {
  F = m_pFirst;
  return true;
  }

TVariable::TVariable( bool Meta, const QByteArray& Name ) : TVariable()
  {
  m_Meta_sign = Meta;
  for( int i = 0, j = 0; i < Name.size() && j < mc_IndCount; i++, j++ )
    {
    if( Name[i] == msLowReg )
      {
      m_Ind[j] = -1;
      m_IsIndexed = true;
      i++;
      }
    else
      if( Name[i] == msHighReg )
        {
        m_Ind[j] = 1;
        m_IsIndexed = true;
        i++;
        }
      else
    if( Name[i] == (char) msMean )
      {
      m_Ind[j] = 2;
      i++;
      }
    m_Name += Name[i];
    }
  if( m_Name == "pi" ) 
    m_Name = QByteArray( 1, msPi);
  else if( m_Name == "teta" )
    m_Name = QByteArray( 1, msTeta);
  }

MathExpr TVariable::Clone() const
  {
  return new TVariable(m_Meta_sign, WriteE());
  }

bool TVariable::IsLinear() const
  {
  return true;
  }

MathExpr TVariable::Reduce() const
  {
  MathExpr body;
  if( ( m_Meta_sign || s_MemorySwitch == SWcalculator ) && s_ExpStore.Known_var_expr( WriteE(), body ) )
    return body.Reduce();
  if( m_Name == "e" )
    return Constant( M_E );
  if( m_Name[0] == (char) msPi && TExpr::sm_FullReduce )
    return Constant( M_PI );
  return Ethis;
  }

bool TVariable::ConstExpr() const
  {
  return m_Name == "e" || m_Name[0] == ( char ) msPi;
  }

MathExpr TVariable::Perform() const
  {
  MathExpr body;
  if( ( m_Meta_sign || s_MemorySwitch == SWcalculator ) && s_ExpStore.Known_var_expr( WriteE(), body ) )
    return body;
  return Ethis;
  }

MathExpr TVariable::Diff( const QByteArray& d )
  {
  if( m_Name.toUpper() != d.toUpper() )
    return new TConstant( 0 );
  return new TConstant( 1 );
  }

MathExpr TVariable::Integral( const QByteArray& d )
  {
  if (m_Name.toUpper()==d.toUpper())
    return new TDivi(new TPowr(new TVariable(false, d), new TConstant(2)), new TConstant(2));
  else
    {
    TVariable V( false, d );
    return new TMult( this, &V );
    }
  }

MathExpr TVariable::Lim( const QByteArray& v, const MathExpr& lm ) const
  {
  if( m_Name.toUpper() == v.toUpper() )
    return lm;
  return Clone();
  }

bool TVariable::Eq( const MathExpr& E2 ) const
  {
  if( E2.IsEmpty() ) return false;
  QByteArray _eqName;
  bool var = E2.Variab( _eqName );
  return var && ( _eqName == WriteE() );
  }

bool TVariable::Equal( const MathExpr& E2 ) const
  {
  QByteArray WriteEStr = WriteE();
  QByteArray _eqName;

  if( E2.Variab( _eqName ) && _eqName == WriteEStr ) return true;

  MathExpr Ar1, Ar2;
  if( E2.Multp( Ar1, Ar2 ) )
    return FactList().Equal( E2.FactList() );
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );
  if( E2.Funct( _eqName, Ar2 ) && ( _eqName == "Simplify" ) )
    return Equal( Ar2 );

  if( s_ExpStore.Known_var_expr( WriteEStr, Ar1 ) )
    return Ar1.Equal( E2 );
  return false;
  }

MathExpr TVariable::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  if( m_Name.toUpper() == vr.toUpper() )
    return vl->Clone();
  return Clone();
  }

QByteArray TVariable::WriteE() const
  {
  QByteArray S;
  for( int i = 0; i < m_Name.length(); i++ )
    {
    if( m_Ind[i] == -1 )
      S.append( msLowReg );
    if( m_Ind[i] == 1 )
      S.append( msHighReg );
    if( m_Ind[i] == 2 )
      S.append( msMean );
    S.append( m_Name[i] );
    }
  return S;
  }

QByteArray TVariable::SWrite() const
  {
  if( m_Name == "sp" ) return "\\invisible\n";
  QByteArray S;
  bool Index = false;
  for( int i = 0; i < m_Name.length(); i++ )
    {
    if( m_Ind[i] == -1 )
      {
      S += "\\subscript{";
      Index = true;
      }
    if( m_Ind[i] == 1 )
      {
      S += "\\superscript{";
      Index = true;
      }
    if( m_Ind[i] == 2 )
      {
      S += "\\mean{";
      Index = true;
      }
    uchar C = m_Name[i];
    if( C < 128 )
      S.append( C );
    else
      if(TStr::sm_Server)
        S += charToTex( C );
      else
        S += charToTex( C ) + '\n';
    if( Index ) S += '}';
    }
  return S;
  }

bool TVariable::Variab( QByteArray& N ) const
  {
  N = WriteE();
  return true;
  }

bool TVariable::IsFactorized( const QByteArray& Name ) const
  {
  return Name == m_Name;
  }

TConstant::TConstant( double V, bool IsE ) : m_Value( V ), m_IsE( IsE ), m_Precision( TExpr::Precision() )
  {
  if( isinf( V ) ) throw ErrParser( X_Str( "INFVAL", "Result is infinity" ), peInfinity );
#ifdef DEBUG_TASK 
  m_Contents = WriteE();
#endif
  }

MathExpr TConstant::Clone() const
  {
  /*
  double  Acc;
  if( s_MemorySwitch == SWtask )
    {
    Acc = sm_Accuracy;
    sm_Accuracy = m_Precision;
    }
  else
    {
    Acc = sm_Precision;
    sm_Precision = m_Precision;
    }
    */
  TConstant *pResult = new TConstant( m_Value );
  /*
  if( s_MemorySwitch == SWtask )
    sm_Accuracy = Acc;
  else
    sm_Precision = Acc;
    */
  pResult->m_Precision = m_Precision;
  pResult->m_IsE = m_IsE;
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TConstant::Reduce() const
  {
  int V;
  if( !sm_ConstToFraction || Cons_int( V ) ) return Ethis;
  double acc = m_Precision;
  int Denom = 1;
  double RNom = abs(m_Value);
  bool bSign = m_Value < 0;
  for( ; acc < 1.0 && Frac( RNom ) >= acc; Denom *= 10, RNom *= 10.0, acc *= 10.0 );
  int Nom = Round( RNom );
  CancFrac( Nom, Denom );
  MathExpr Result = new TSimpleFrac( Nom, Denom );
  if( bSign ) return -Result;
  return Result;
  }

MathExpr TConstant::Diff( const QByteArray& d )
  { 
  return new TConstant(0);
  }

MathExpr TConstant::Integral( const QByteArray& d )
  {
  return ( Variable( d ) * MathExpr( this ) );
  }

MathExpr TConstant::Lim( const QByteArray& v, const MathExpr lm ) const
  {
  return Clone();
  }

bool TConstant::Eq( const MathExpr& E2 ) const
  {
  double Value2;
  double Prec = TExpr::sm_Precision;
//  double Prec = m_Precision;
  if( !E2.Constan( Value2 ) )
    {
    MathExpr ex;
    if( !E2.Unarminus( ex ) || !ex.Constan( Value2 ) ) return false;
    Prec = max( Prec, CastPtr( TConstant, ex )->m_Precision );
    Value2 = -Value2;
    }
  else
    Prec = max( Prec, CastConstPtr( TConstant, E2 )->m_Precision );
  return m_Value == Value2 || ( m_Value != 0 && Value2 != 0 && abs( m_Value - Value2 ) < Prec );
  }

bool TConstant::Equal( const MathExpr& E2 ) const
  {
  MathExpr Ar2;
  double Value2;
  int Nom, Denom;
  QByteArray _eqName;

  if( E2.Constan( Value2 ) || E2.Unarminus( Ar2 ) )
    return Eq( E2 );

  if( E2.SimpleFrac_( Nom, Denom ) )
    {
    if( Denom == 0 ) return false;
    return abs( m_Value - ( double ) Nom / Denom ) < m_Precision;
    }

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( E2.Perform() );
  return false;
  }

bool TConstant::Constan( double& V )  const
  {
  V = m_Value;
  return true;
  }

QByteArray TConstant::WriteE() const
  {
  double Acc;
  if( m_IsE ) return "e";
  Acc = TExpr::sm_Accuracy;
  uint powr, nprec = 0;
  for( ; Acc < 1.0; nprec++ ) Acc *= 10.0;
  if( ( m_Value < -1.0001e7 ) || ( m_Value > 2.0001e9 ) )
    {
    double RR = m_Value;
    for( powr = 0; abs( RR ) >= 10.0; powr++ ) RR /= 10.0;
    QByteArray S( QByteArray::number( RR, 'f', nprec ) );
    while( S.right( 1 ) == "0" )
      S = S.left( S.length() - 1 );
    QByteArray S1( QByteArray::number( powr ) );
    if( S.right( 1 ) == "." )
      S = S.left( S.length() - 1 );
    return S + 'E' + S1;
    }
  QByteArray S( QByteArray::number( m_Value, 'f', nprec ) );
  while( S.right( 1 ) == "0" )
    S = S.left( S.length() - 1 );
  if( S.right( 1 ) == "." )
    S = S.left( S.length() - 1 );
  if( S == "-0" ) S = "0";   
  return S;
  }

bool TConstant::Cons_int( int& I ) const
  {
  if( abs( m_Value ) < 2000000001 && abs( m_Value - Round( m_Value ) ) <  m_Precision )
    {
    I = Round( m_Value );
    return true;
    }
  return false;
  }

bool TConstant::Negative() const
  {
  return ( m_Value < 0 );
  }

bool TConstant::ConstExpr() const
  {
  return true;
  }

bool TConstant::IsLinear() const
  {
  return true;
  }

bool TConstant::Positive() const
  {
  return ( m_Value >= 0 );
  }

int TConstant::Compare( const MathExpr& ex ) const
  {
  int N, D;
  double v;
  if( Equal( ex ) ) return 0;
  if( ex.Constan( v ) )
    if( m_Value > v )
      return 1;
    else
      return -1;
  if( ex.SimpleFrac_( N, D ) )
    if( round( m_Value ) * D > N )
      return 1;
    else
      return -1;
  return 2;
  }

TFunc::TFunc( bool Meta, const QByteArray& Name, const MathExpr& Arg ) : TExpr(), m_Meta_sign( Meta ),
m_Name( Name ), m_Arg( Arg ), m_ShortName( OutputFunctName( Name ) )
  {
#ifdef DEBUG_TASK 
  m_Contents = WriteE();
#endif
  }
   
MathExpr TFunc::Clone() const
  { 
  TFunc *pResult = new TFunc( m_Meta_sign, m_Name, m_Arg->Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TFunc::TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower )
  {
  if( sName == m_Name && m_Arg == exArg && ( exPower.IsEmpty() || exPower == 1 ) )
    return new TConstant( 1 );
  return nullptr;
  }

MathExpr TFunc::Reduce() const
  {
  if( m_Arg() == nullptr ) return Ethis;
  MathExpr arg_Reduced;
  if( TExpr::sm_FullReduce || s_MemorySwitch == SWtask )
    arg_Reduced = m_Arg.Reduce();
  else
    {
    bool OldFullReduce = TExpr::sm_FullReduce;
    TExpr::sm_FullReduce = true;
    arg_Reduced = m_Arg.Reduce();
    TExpr::sm_FullReduce = OldFullReduce;
    }
  int N, K;
  double V, Vn;
  PExMemb first;

  if( m_Name == "factorial" || m_Name == "PerCount" )
    {
    if( arg_Reduced.Cons_int( N ) )
      {
      if( N < 0 )  throw ErrParser( X_Str( "MNotAInt", "Operands must be positive integers!" ), peNoSolv );
      int f = 1;
      for( int i = 1; i <= N; f *= i++ );
      return new TConstant( f );
      }
    }
  if( m_Name == "BinomCoeff" )
    {
    first = CastConst( TLexp, m_Arg )->m_pFirst;
    if( first->m_Memb.Reduce().Cons_int( K ) )
      {
      first = first->m_pNext;
      if( first->m_Memb.Reduce().Cons_int( N ) )
        {
        if( K < 0 || N < 0 || K > N ) throw ErrParser( X_Str( "MNotAInt", "Operands must be positive integers!" ), peNoSolv );
        if( K > N / 2 ) K = N - K;
        int D = 1;
        for( int i = 1; i <= K; i++ )
          {
          D *= N;
          D /= i;
          N--;
          }
        return new TConstant( D );
        }
      }
    }

  if( m_Name == "ACoeff" )
    {
    first = CastConst( TLexp, m_Arg )->m_pFirst;
    if( first->m_Memb.Reduce().Cons_int( K ) )
      {
      first = first->m_pNext;
      if( first->m_Memb.Reduce().Cons_int( N ) )
        {
        if( K < 0 || N < 0 || K > N ) throw ErrParser( X_Str( "MNotAInt", "Operands must be positive integers!" ), peNoSolv );
        int D = 1;
        for( int i = N; i >= N - K + 1; i-- ) D *= i;
        return new TConstant( D );
        }
      }
    }

  if( ( m_Name == "div" || m_Name == "mod" ) && arg_Reduced.Listex( first ) && first != nullptr && first->m_pNext != nullptr &&
    first->m_Memb.Constan( V ) && first->m_pNext->m_Memb.Constan( Vn ) )
    {
    if( Vn == 0.0 )
      {
      Error_m( X_Str( "MDivisBy0", "Division by 0!" ) );
      s_GlobalInvalid = false;
      return new TConstant( 0 );
      }
    int Trunc = V / Vn;
    if( m_Name == "div" ) return new TConstant( Trunc );
    return new TConstant( V - Vn * Trunc );
    }

  if( m_Name == "ERF" ) return GaussProbability( arg_Reduced );

  if( m_Name == "ln" )
    {
    if( s_NoLogReduce && arg_Reduced.ConstExpr() )
      return new TFunc( false, "ln", arg_Reduced );
    MathExpr ex = arg_Reduced.EVar2EConst().Reduce();
    if( ex.Constan( V ) && V > 1E-6 &&  abs( Frac( log( V ) ) ) < Precision() )
      return new TConstant( Trunc( log( V ) ) );
    }

  if( m_Name == "rand_with_prohib" )
    {
    first = CastConst( TLexp, m_Arg )->m_pFirst;
    qint32 A, B, C;
    if( !first->m_Memb.Cons_int( A ) ) throw ErrParser( "Invalid argment rand_with_prohib", peNoSolv );
    first = first->m_pNext;
    if( !first->m_Memb.Cons_int( B ) ) throw ErrParser( "Invalid argment rand_with_prohib", peNoSolv );
    QVector<int> Exclude;
    for(; !first.isNull(); first = first->m_pNext)
      if( first->m_Memb.Cons_int( C ) )
        Exclude.append(C);
    std::seed_seq SQ( &A, &B);
    QRandomGenerator RG(SQ);
    int Result = RG.generate();
    while( Exclude.indexOf(Result) != -1 )
      Result = RG.generate();
    return new TConstant(Result);
    }

  MathExpr ext, e;
  if( sm_TrigonomSystem == tsRad )
    ext = Parser::StrToExpr( PiVar2PiConst( arg_Reduced.WriteE() ) );
  else
    ext = Parser::StrToExpr( PiVar2Pi180( arg_Reduced.WriteE() ) );
  bool Neg = ext.Unarminus( e );
  if( Neg ) ext = e;
  MathExpr op1, op2;
  QByteArray MeaName;
  if( !ext.IsNumericalValue( V ) && !( ext.Measur_( op1, op2 ) && op1.IsNumericalValue( V ) && op2.Variab( MeaName ) &&
    ( uchar ) MeaName[0] == msDegree && IsTrigonom( m_Name ) ) ) return new TFunc( m_Meta_sign, m_Name, arg_Reduced );
  if( Neg ) V = -V;
  int D;
  if( arg_Reduced.SimpleFrac_( N, D ) ) V = (double) N / D;

  if( m_Name == "abs" ) return new TConstant( abs( V ) );

  if( m_Name == "sign" )
    {
    if( V < 0 ) return new TConstant( -1 );
    if( V == 0 ) return new TConstant( 0 );
    return new TConstant( 1 );
    }

  if( m_Name == "int" ) return new TConstant( Trunc( V ) );
  if( m_Name == "frac" ) return new TConstant( Frac( V ) );
  if( m_Name == "round" ) return new TConstant( Round( V ) );
  if( m_Name == "exp" )
    {
    if( s_NoExpReduce )
      return new TFunc( false, "exp", arg_Reduced );
    return new TConstant( exp( V ) );
    }
  if( m_Name == "ln" )
    {
    if( V <= 0 ){ s_LastError="MArgLnLess0"; return Ethis; }
    if( s_NoLogReduce && arg_Reduced.Cons_int( N ) ) return Ethis;
    return new TConstant( log( V ) );
    }

  if( m_Name == "lg" )
    {
    if( V <= 0 ){ s_LastError="MArgLgLess0"; return Ethis; }
    if( s_NoLogReduce && arg_Reduced.Cons_int( N ) ) return Ethis;
    return new TConstant( log10( V ) );
    }

  TrigonomSystem CurrTS = sm_TrigonomSystem;
  MathExpr ex, exm, PiRes, DegRes, Result;
  double Coeff1, Coeff2;

  if( arg_Reduced.Measur_( ex, exm ) || ( arg_Reduced.Unarminus( op1 ) && op1.Measur_( ex, exm ) ) ) sm_TrigonomSystem = tsDeg;
  if( m_Name == "sin" )
    {
    switch( sm_TrigonomSystem )
      {
      case tsRad:
        if( PiProcess( arg_Reduced, Coeff1, Coeff2 ) && PiSinStandard( Coeff1, Coeff2, PiRes ) )
          Result = PiRes;
        else
          Result = new TConstant( sin( TriginomValue( V ) ) );
        break;
      case tsDeg:
        if( DegSinStandard( V, DegRes ) )
          Result = DegRes;
        else
          Result = new TConstant( sin( TriginomValue( V ) ) );
      }
    sm_TrigonomSystem = CurrTS;
    return Result;
    }

  if( m_Name == "cos" )
    {
    if( sm_TrigonomSystem == tsRad )
      if( PiProcess( arg_Reduced, Coeff1, Coeff2 ) && PiCosStandard( Coeff1, Coeff2, PiRes ) )
        Result = PiRes;
      else
        Result = new TConstant( cos( TriginomValue( V ) ) );
    else
      if( DegCosStandard( V, DegRes ) )
        Result = DegRes;
      else
        Result = new TConstant( cos( TriginomValue( V ) ) );
    sm_TrigonomSystem = CurrTS;
    return Result;
    }

  if( m_Name == "tan" )
    {
    PiRes = MathExpr(nullptr);
    s_GlobalTrigonomError = s_GlobalTrigonomError || abs( cos( TriginomValue( V ) ) <= Precision()  );
    if( abs( cos( TriginomValue( V ) ) ) <= Precision() )
      s_GlobalInvalid = true;
    else
      {
      if( sm_TrigonomSystem == tsRad )
        if( PiProcess( arg_Reduced, Coeff1, Coeff2 ) && PiTanStandard( Coeff1, Coeff2, PiRes ) )
          Result = PiRes;
        else
          Result = new TConstant( tan( TriginomValue( V ) ) );
      else
        if( DegTanStandard( V, DegRes ) )
          Result = DegRes;
        else
          Result = new TConstant( tan( TriginomValue( V ) ) );
      }
    if( s_GlobalInvalid )
      {
      s_TanCotError = true;
      s_LastError = X_Str( "MDivisBy0", "Division by 0!" ) + X_Str( "MDivInTan", "(tan x == sin x / cos x)" );
      throw ErrParser( "No Solution", peNoSolv );
      }
    sm_TrigonomSystem = CurrTS;
    return Result;
    }

  if( m_Name == "cot" )
    {
    PiRes = MathExpr( nullptr );
    s_GlobalTrigonomError = s_GlobalTrigonomError || ( abs( sin( TriginomValue( V ) ) ) <= Precision() );
    if( ( abs( sin( TriginomValue( V ) ) ) <= Precision() ) )
      s_GlobalInvalid = true;
    else
      {
      switch( sm_TrigonomSystem )
        {
        case tsRad:
          if( PiProcess( arg_Reduced, Coeff1, Coeff2 ) && PiCotStandard( Coeff1, Coeff2, PiRes ) )
            Result = PiRes;
          else
            Result = new TConstant( 1.0 / tan( TriginomValue( V ) ) );
          break;
        case tsDeg:
          if( DegCotStandard( V, DegRes ) )
            Result = DegRes;
          else
            Result = new TConstant( 1.0 / tan( TriginomValue( V ) ) );
        }
      }
    if( s_GlobalInvalid )
      {
      s_TanCotError = true;
      s_LastError = X_Str( "MDivisBy0", "Division by 0!" ) + X_Str( "MDivInCot", "(cot x == cos x / sin x)" );
      throw ErrParser( "No Solution", peNoSolv );
      }
    sm_TrigonomSystem = CurrTS;
    return Result;
    }

  double V1, V2;
  MathExpr PI = ExpStore::sm_pExpStore->Restore_var_expr(QByteArray(1, msPi));

  if( m_Name == "arcsin" )
    {
    V1 = abs( V ), V2;
    if( V1 > 1.0 ) throw ErrParser( X_Str( "MArgArcsinOut", "Argument out of range!" ), peNoSolv );
    if( V1 == 1.0 )
      V2 = 2.0;
    else
      if( V1 == 0.5 )
        V2 = 6.0;
      else
        if( abs( V1 - sqrt( 2 ) / 2 ) <= Precision() )
          V2 = 4.0;
        else
          if( abs( V1 - sqrt( 3 ) / 2 ) <= Precision() )
            V2 = 3.0;
          else
            {
            if( sm_TrigonomSystem == tsRad ) return new TConstant( AngleValue( asin( V ) ) );
            return new TDegExpr( QByteArray::number( AngleValue( asin( V ) ) ) + ";" );
            }
    if( sm_TrigonomSystem == tsRad )
      if( PI.IsEmpty() )
        Result = new TDivi( new TVariable( false, msPi ), new TConstant( V2 ) );
      else
        Result = (PI / V2).Reduce();
    else
      Result = new TDegExpr( QByteArray::number( 180.0 / V2 ) + ";" );
    if( V < 0 ) Result = new TUnar( Result );
    return Result;
    }

  if( m_Name == "arccos" )
    {
    if( ( V > 1 ) || ( V < -1 ) ) throw ErrParser( X_Str( "MArgArcsinOut", "Argument out of range!" ), peNoSolv );
    if( V == 0.0 )
      {
      V1 = 1; V2 = 2;
      }
    else  if( V == 0.5 )
      {
      V1 = 1; V2 = 3;
      }
    else  if( abs( V - sqrt( 2 ) / 2 ) <= Precision() )
      {
      V1 = 1; V2 = 4;
      }
    else  if( abs( V - sqrt( 3 ) / 2 ) <= Precision() )
      {
      V1 = 1; V2 = 6;
      }
    else  if( V == -1 )
      {
      V1 = 1; V2 = 1;
      }
    else  if( V == -0.5 )
      {
      V1 = 2; V2 = 3;
      }
    else  if( abs( V + sqrt( 2 ) / 2 ) <= Precision() )
      {
      V1 = 3; V2 = 4;
      }
    else  if( abs( V + sqrt( 3 ) / 2 ) <= Precision() )
      {
      V1 = 5; V2 = 6;
      }
    else
      {
      if( sm_TrigonomSystem == tsRad ) return new TConstant( AngleValue( acos( V ) ) );
      return new TDegExpr( QByteArray::number( AngleValue( acos( V ) ) ) + ";" );
      }
    if( sm_TrigonomSystem == tsRad )
      if( PI.IsEmpty())
        Result = new TVariable( false, msPi );
      else
        Result = PI;
    else
      return new TDegExpr( QByteArray::number( 180.0 * V1 / V2 ) + ";" );
    if( V1 != 1 )
      Result = new TMult( new TConstant( V1 ), Result );
    if( V2 != 1 )
      Result = new TDivi( Result, new TConstant( V2 ) );
    return Result.Reduce();
    }

  if( m_Name == "arctan" )
    {
    V1 = abs( V );
    if( V1 == 1 )
      V2 = 4;
    else  if( abs( V1 - sqrt( 3 ) ) <= Precision() )
      V2 = 3;
    else  if( abs( V1 - sqrt( 3 ) / 3 ) <= Precision() )
      V2 = 6;
    else
      {
      if( sm_TrigonomSystem == tsRad ) return new TConstant( AngleValue( atan( V ) ) );
      return new TMeaExpr( new TConstant( AngleValue( atan( V ) ) ), new TVariable( false, msDegree ) );
      }
    if( sm_TrigonomSystem == tsRad )
      if( PI.IsEmpty())
        Result = new TDivi( new TVariable( false, msPi ), new TConstant( V2 ) );
      else
        Result = (PI / V2).Reduce();
    else
      Result = new TMeaExpr( new TConstant( 180 / V2 ), new TVariable( false, msDegree ) );
    if( V < 0 )
      Result = new TUnar( Result );
    return Result;
    }

  if( m_Name == "arccot" )
    {
    if( V == 0 )
      {
      V1 = 1; V2 = 2;
      }
    else  if( V == 1 )
      {
      V1 = 1; V2 = 4;
      }
    else  if( abs( V - sqrt( 3 ) ) <= Precision() )
      {
      V1 = 1; V2 = 6;
      }
    else  if( abs( V - sqrt( 3 ) / 3 ) <= Precision() )
      {
      V1 = 1; V2 = 3;
      }
    else  if( V == -1 )
      {
      V1 = 3; V2 = 4;
      }
    else  if( abs( V + sqrt( 3 ) ) <= Precision() )
      {
      V1 = 5; V2 = 6;
      }
    else  if( abs( V + sqrt( 3 ) / 3 ) <= Precision() )
      {
      V1 = 2; V2 = 3;
      }
    else
      {
      V1 = atan( 1.0 / V );
      if( V1 < 0 ) V1 = V1 + M_PI;
      if( sm_TrigonomSystem == tsRad ) return new TConstant( AngleValue( V1 ) );
      return new TDegExpr( QByteArray::number( AngleValue( V1 ) ) + ";" );
      }
    if( sm_TrigonomSystem == tsRad )
      if( PI.IsEmpty())
        Result = new TVariable( false, msPi );
      else
        Result = PI;
    else
      return new TDegExpr( QByteArray::number( 180.0 * V1 / V2 ) + ";" );
    if( V1 != 1 )
      Result = new TMult( new TConstant( V1 ), Result );
    if( V2 != 1 )
      Result = new TDivi( Result, new TConstant( V2 ) );
    return Result.Reduce();
    }

  if( m_Name == "sec" )
    {
    if( ( cos( TriginomValue( V ) ) == 0 ) )
      {
      Error_m( X_Str( "MDivisBy0", "Division by 0!" ) + X_Str( "MDivInSec", "(sec x == 1 / cos x)" ) );
      s_GlobalInvalid = true;
      return new TConstant( 0 );
      }
    return new TConstant( 1.0 / cos( TriginomValue( V ) ) );
    }

  if( m_Name == "cosec" )
    {
    if( ( sin( TriginomValue( V ) ) == 0 ) )
      {
      Error_m( X_Str( "MDivisBy0", "Division by 0!" ) + X_Str( "MDivInCoSec", "(cosec x == 1 / sin x)" ) );
      s_GlobalInvalid = true;
      return new TConstant( 0 );
      }
    return new TConstant( 1.0 / sin( TriginomValue( V ) ) );
    }

  if( m_Name == "SetTrigoSystem" )
    {
    if( arg_Reduced.Cons_int( N ) )
      {
      if( N == 0 )
        sm_TrigonomSystem = tsRad;
      else
        sm_TrigonomSystem = tsDeg;
      } // XPRESSCalculator.UpdateRadDeg; // kag 16.04_kag_01 SendMessage( HWND_XPCALCFORM, SX_XPCALC_RDNBTN_MESSAGE, 0, 0 );
    return nullptr;
    }

  if( m_Name == "SetShowMinute" )
    {
    if( arg_Reduced.Cons_int( N ) )
      s_ShowMinute = N;
      // XPRESSCalculator.UpdateRadDeg; // kag 16.04_kag_01 SendMessage( HWND_XPCALCFORM, SX_XPCALC_RDNBTN_MESSAGE, 0, 0 );
    return nullptr;
    }

  return new TFunc( m_Meta_sign, m_Name, arg_Reduced );
  }

  MathExpr TFunc::Perform() const
    {
    if( !m_Meta_sign ) return Clone();

    MathExpr arg_Performed( m_Arg.Perform() ), arg_Reduced, op1, op2;
    PExMemb pMember;
    double Scale;
    bool PutExpr;

    if( m_Name == "Simplify" )
      {
      arg_Reduced = arg_Performed.Reduce();
      if (IsType(TSimpleFrac, arg_Reduced))
        {
        bool FullReduce = TExpr::sm_FullReduce;
        TExpr::sm_FullReduce = true;
        arg_Reduced.SetReduced(false);
        arg_Reduced = arg_Reduced.Reduce();
        TExpr::sm_FullReduce = FullReduce;
        }
      return arg_Reduced;
      }

    if( m_Name == "SimplifyEquation" )
      {
      s_CanExchange = false;
      arg_Reduced = arg_Performed.Reduce();
      s_CanExchange = true;
      return arg_Reduced;
      }

    if( m_Name == "SimplifyFull" )
      {
      bool FullReduce = TExpr::sm_FullReduce;
      TExpr::sm_FullReduce = true;
      arg_Reduced = arg_Performed.Reduce();
      arg_Reduced.SetReduced(false);
      arg_Reduced = arg_Reduced.Reduce();
      TExpr::sm_FullReduce = FullReduce;
      return arg_Reduced;
      }

    if( m_Name == "PolinomSmplf" )
      return m_Arg.ReducePoly();

//    if( m_Name == "SolutionSystNonEq" )
//      return m_Arg.SolSysInEq();

    if( m_Name == "TrigoComplex" )
      {
      bool OldReduce = TExpr::sm_FullReduce;
      TExpr::sm_FullReduce = true;
      MathExpr Arg = m_Arg.Reduce();
      TExpr::sm_FullReduce = OldReduce;
      if( Arg.Listex( pMember ) )
        {
        op1 = pMember->m_Memb;
        Scale = 1.0;
        PutExpr = false;
        pMember = pMember->m_pNext;
        if( pMember != nullptr )
          {
          pMember->m_Memb.Constan( Scale );
          pMember->m_Memb.Boolean_( PutExpr );
          pMember = pMember->m_pNext;
          if( !pMember.isNull() )
            {
            pMember->m_Memb.Constan( Scale );
            pMember->m_Memb.Boolean_( PutExpr );
            }
          }
        return op1.AlgToGeometr( Scale, PutExpr );
        }
      return Arg.AlgToGeometr();
      }

    if( m_Name == "Table" )
      return new TTable( m_Arg );

    if( m_Name == "Picture" )
      return new TExprPict( m_Arg );

    if( m_Name == "UnvisibleTable" )
      {
      op1 = new TTable( m_Arg );
      CastPtr( TTable, op1 )->SetGridState( TGRUnvisible );
      return op1;
      }

    if( m_Name == "PartlyVisibleTable" )
      {
      op1 = new TTable( m_Arg );
      CastPtr( TTable, op1 )->SetGridState( TGRPartlyVisible );
      return op1;
      }

    if( m_Name == "ERF" )
      return GaussProbability( m_Arg );

    if( m_Name == "Chart" && ( !s_TaskEditorOn || !m_Meta_sign ) )
      return new TChart( m_Arg );
    /*
    if( m_Name == "CreateObject" )
      return CastPtr(Lexp, m_Arg )->CreateObject();
      */
    if( m_Name == "mixer" )
      return arg_Performed.Mixer();

    if( m_Name == "List" )
      {
      if( arg_Performed.Summa( op1, op2 ) )
        return op1.Lconcat( op2 );
      return arg_Performed.Lconcat( nullptr );
      }

    if( m_Name == "ListOrd" )
      return arg_Performed.Lconcat( nullptr );

    if( m_Name == "SimplifyInDetail" )
      return arg_Performed.SimplifyInDetail();

    if( m_Name == "fcase" )
      return arg_Performed.Fcase();

    if( m_Name == "GenChainInEq" )
      return arg_Performed.GenChainInEq();

    if( m_Name == "RtoDEC" )
      return arg_Performed.RtoDEC();

    if( m_Name == "ReduceToMultiplicators" )
      return arg_Performed.ReduceToMultiplicators();

    if( m_Name == "FindGreatestCommDivisor" )
      return arg_Performed.FindGreatestCommDivisor();

    if( m_Name == "diff" )
      return arg_Performed.Diff();

    if( m_Name == "Dividend" )
      return arg_Performed.Dividend();

    if( m_Name == "Divisor" )
      return arg_Performed.Divisor();

    if( m_Name == "RandList" )
      return arg_Performed.RandList();

    if( m_Name == "SortListA" )
      return arg_Performed.SortList( true );

    if( m_Name == "SortListD" )
      return arg_Performed.SortList( false );

    if( m_Name == "FindLeastCommDenom" )
      return arg_Performed.FindLeastCommDenom();

    if( m_Name == "ToLeastCommDenom" )
      return arg_Performed.ToLeastCommDenom();

    if( m_Name == "FindLeastCommMult" )
      return arg_Performed.FindLeastCommMult();

    if( m_Name == "GetEl" )
      {
      if( !arg_Performed.List2ex( pMember ) || pMember->m_pNext.isNull() )
        throw ErrParser( "Invalid Argument!", peSyntacs );
      op1 = pMember->m_pNext->m_Memb.Reduce();
      int iLast;
      if( !op1.Cons_int( iLast ) || !pMember->m_Memb.Listex( pMember ) )
        throw ErrParser( "Invalid Argument!", peSyntacs );

      for( int iPos = 1; iPos < iLast; iPos++ )
        {
        pMember = pMember->m_pNext;
        if( pMember.isNull() )
          throw ErrParser( "Invalid Argument!", peSyntacs );
        }
      return pMember->m_Memb;
      }

    return MathExpr(new TFunc( m_Meta_sign, m_Name, arg_Performed ) ).Reduce();
    }

   MathExpr TFunc::Diff( const QByteArray& d  )
     {
     MathExpr Result;
     if( m_Name == "exp" )
       Result = this;
     if( m_Name == "sin" )
       Result = new TFunc( false, "cos", m_Arg );
     if( m_Name == "cos" )
       Result = Function( "sin", m_Arg ).Negative();
     if( m_Name == "ln"  )
       Result = Constant(1) / m_Arg;
     if( m_Name == "lg"  )
       Result = Constant ( 0.4342944 )/m_Arg;
     if( m_Name == "tan" )
       Result = Constant ( 1 )/Function("cos", m_Arg )^2;
     if( m_Name == "cot" )
       Result = Constant ( 1 )/Function("sin", m_Arg )^2;
     if( m_Name == "arccot" )
       Result = Constant ( -1 )/(Constant ( 1 ) + m_Arg^2);
     if( m_Name == "arctan" )
       Result = Constant ( 1 )/( Constant ( 1 ) + m_Arg^2);
     if( m_Name == "arccos" )
       Result = Constant ( -1 )/( Constant ( 1 ) - m_Arg^2 ).Root(2);
     if( m_Name == "arcsin" )
       Result = Constant ( 1 )/( Constant ( 1 ) - m_Arg^2 ).Root(2);
     if( m_Name == "sec" )
       Result = Function( "tan", m_Arg ) / Function( "cos", m_Arg );
     if( m_Name == "cosec" )
       Result = Function( "cot", m_Arg ) / Function( "sin", m_Arg ).Negative();
     if( Result.IsEmpty() )
       return Clone();
     return Result * m_Arg.Diff(d);
     }

  bool TFunc::Eq( const MathExpr& E2 ) const
    {
    MathExpr  Ar1, Ar2;
    QByteArray _eqName;
    int iVal;

    if( E2.Funct( _eqName, Ar2 ) )
      return m_Name == _eqName && m_Arg.Eq( Ar2 );
    return m_Name == "lg" && E2.Log( Ar1, Ar2 ) && Ar1.Cons_int( iVal ) && iVal == 10 && Ar2.Eq( m_Arg );
    }

  bool TFunc::Equal( const MathExpr& E2 ) const
    {
    if( Eq( E2 ) ) return true;

    MathExpr     Ar2;
    QByteArray _eqName;

    if( E2.Funct( _eqName, Ar2 ) )
      return m_Name == _eqName && m_Arg.Equal( Ar2 );

    if( m_Name == "Simplify" )
      return m_Arg.Equal( E2 );

    if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
      return Equal( Ar2 );
    return false;
    }

  MathExpr TFunc::Substitute( const QByteArray& vr, const MathExpr& vl )
    {
    return new TFunc( m_Meta_sign, m_Name, m_Arg.Substitute( vr, vl ) );
    }

  QByteArray TOper::WriteE() const
    {
    QByteArray S;
    MathExpr arg1, arg2, arg3;
    bool SignBR;
    if( m_Name == 0 )
      return m_Operand1.WriteE() + m_Operand2.WriteE();
    QByteArray Name;
    Name.fill( m_Name, 1 );
    SignBR = Power( arg1, arg2 ) && !arg1.IsEmpty() && arg1.Unarminus( arg3 );
    if( !m_Operand1.IsEmpty() )
      {
      if( SignBR )
        S = "((" + m_Operand1.WriteE() + ')' + Name;
      else
        if( IsConstType( TNewLin, m_Operand2 ) )
          S += m_Operand1.WriteE() + Name;
        else
          S += m_Operand1.WriteE() + Name + '(';
      }
    else S = "(???)" + Name;
    if( !m_Operand2.IsEmpty() )
      if( IsConstType( TNewLin, m_Operand2 ) )
        S += m_Operand2.WriteE();
      else
        S += m_Operand2.WriteE() + ')';
    else S += "(???)";
    return S;
    }

  QByteArray TOper::SWrite() const
    {
    MathExpr Op1, Op2;
    QByteArray Name ( charToTex( m_Name ) );
    if( Name[0] == '\\' ) Name += '\n';
    if( m_Name == '-' && !sm_InsideChart ) Name = "\\longminus\n";
    if( m_Name == '+' ) Name = "\\longplus\n";
    char Op;
    double Val;
    QByteArray Left = m_Operand1.SWrite();
    if (m_Name == ':' && m_Operand1.MustBracketed() == brOperation ) Left = '(' + Left + ')';
    if( ( m_Name == '-' || m_Name == ':' || ( m_Name == '+' && !(IsConstType( TSumm, m_Operand2 ) ) ) ) &&
      ( IsConstType( TUnar, m_Operand2 ) || IsConstType( TUnapm, m_Operand2 ) ||
      ( m_Operand2.Constan(Val) && Val < 0.0  ) ||
      m_Operand2.MustBracketed() == brOperation || ( m_Operand2.Oper_( Op, Op1, Op2 ) && 
      ( IsConstType( TUnapm, Op1 ) || Op1.Negative() ) ) )  )
      return Left + Name + '(' + m_Operand2.SWrite() + ')';
    return Left + Name + m_Operand2.SWrite();
    }

TDegExpr::TDegExpr( const QByteArray& Value ) : TOper()
  {
  int n = Value.indexOf( ';' );
  QByteArray Deg = Value.left( n );
  QByteArray Min = Value.mid( n + 1 );
  if( s_ShowMinute )
    {
    if( !Deg.isEmpty() )
      m_Operand1 = new TMeaExpr( new TConstant( Deg.toDouble() ), new TVariable( false, msDegree ) );
    else
      m_Operand1 = new TMeaExpr( new TConstant( 0 ), new TVariable( false, msDegree ) );
    if( !Min.isEmpty() )
      m_Operand2 = new TMeaExpr( new TConstant( Min.toDouble() ), new TVariable( false, msMinute ) );
    else
      m_Operand2 = new TMeaExpr( new TConstant( 0 ), new TVariable( false, msMinute) );
    }
  else
    {
    double ValDeg = 0.0;
    if( !Deg.isEmpty() ) ValDeg = Deg.toDouble();
    if( !Min.isEmpty() ) ValDeg += Min.toDouble() / 60.0;
    MathExpr ConstValue = new TConstant( ValDeg );
    m_Operand1 = new TMeaExpr( ConstValue, new TVariable( false, msDegree ) );
    m_Operand2 = new TMeaExpr( new TConstant( 0 ), new TVariable( false, msMinute ) );
    }
  m_Name = 'd';
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

TDegExpr::TDegExpr( const MathExpr& ex1, const MathExpr& ex2 ) : TOper( ex1, ex2, 'd'  )
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TDegExpr::Reduce() const
  {
  MathExpr op11, op12;
  m_Operand1.Measur_( op11, op12 );

  double Deg;
  op11.Constan( Deg );

  MathExpr  op21, op22;
  m_Operand2.Measur_( op21, op22 );

  double Min;
  op21.Constan( Min );

  double Value = Deg + Min / 60;

  return new TMeaExpr( Constant( Value ), Variable( msDegree ) );
  }

MathExpr TDegExpr::Clone() const
  {
  TDegExpr *pResult = new TDegExpr( m_Operand1.Clone(), m_Operand2.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TDegExpr::Perform() const
  {
  return new TDegExpr( m_Operand1.Perform(), m_Operand2.Perform() );
  }

bool TDegExpr::Degree_( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TDegExpr::Measur_( MathExpr& op1, MathExpr& op2 ) const
  {
  bool Result = m_Operand1.Measur_( op1, op2 );
  if( !Result ) return false;
  op2 = Variable( msDegree );
  return true;
  }

bool TDegExpr::Eq( const MathExpr& E2 ) const
  {
  MathExpr  Op21, Op22;
  if( E2.Degree_( Op21, Op22 ) )
    return m_Operand1.Eq( Op21 ) && m_Operand2.Eq( Op22 );
  if( E2.Measur_( Op21, Op22 ) )
    return m_Operand1.Eq( E2 );
  return false;
  }

bool TDegExpr::Equal( const MathExpr& E2 ) const
  {
  return Eq( E2 );
  }

MathExpr TDegExpr::Diff( const QByteArray& d )
  {
  return Constant( 0 );
  }

MathExpr TDegExpr::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TDegExpr( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

QByteArray TDegExpr::WriteE() const
  {
  QByteArray  S;
  MathExpr op1, op2;
  double v;
  S = m_Operand1.WriteE();
  m_Operand2.Measur_( op1, op2 );
  if( !( op1.Constan( v ) && ( abs( v ) < 0.0000001 ) ) )
    S += m_Operand2.WriteE();
  return S;
  }

QByteArray TDegExpr::SWrite() const
  {
  MathExpr op1, op2;
  m_Operand1.Measur_( op1, op2 );
  QByteArray Result;
  if (op1 != 0) Result = op1.SWrite() + "\\deg";
  m_Operand2.Measur_(op1, op2);
  if (op1 != 0) Result += ' ' + op1.SWrite() + "\\min";
  return Result;
  }

MathExpr TMeaExpr::Clone() const
  {
  TMeaExpr *pResult = new TMeaExpr( m_Operand1.Clone(), m_Operand2.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TMeaExpr::Perform() const
  {
  return new TMeaExpr( m_Operand1.Perform(), m_Operand2.Perform() );
  }

bool TMeaExpr::Eq( const MathExpr& E2 ) const
  {
  MathExpr  Op21, Op22;
  if( E2.Measur_( Op21, Op22 ) )
    return m_Operand1.Eq( Op21 ) && m_Operand2.Eq( Op22 );
  return false;
  }

MathExpr TMeaExpr::Diff( const QByteArray& d )
  {
  return new TMeaExpr( m_Operand1.Diff( d ), m_Operand2 );
  }

bool TMeaExpr::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, Op21, Op22, E(E2);
  QByteArray _eqName;

  if( E2.Degree_( Op21, Op22 ) )
    E = E2.Reduce();

  if( E.Measur_( Op21, Op22 ) )
    return ( m_Operand1.Equal( Op21 ) && m_Operand2.Equal( Op22 ) );

  if( E.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E.Funct( _eqName, Ar2 ) && ( _eqName == "Simplify" ) )
    return Equal( Ar2 );
  return false;
  }

MathExpr TMeaExpr::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TMeaExpr( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

QByteArray TMeaExpr::SWrite() const
  {
  return "\\units{" + m_Operand1.SWrite() + "}{" + m_Operand2.SWrite() + '}';
  }

QByteArray TMeaExpr::WriteE() const
  {
  QByteArray  S;
  if( !m_Operand1.IsEmpty() )
    S = '(' + m_Operand1.WriteE() + ')';
  else
    S = "(???)";

  if( !m_Operand2.IsEmpty() )
    {
//    uchar Name = m_Operand2.WriteE()[0];
//    if( Name == msDegree || Name == msMinute || m_Operand2.WriteE() == "%" )
//      S += m_Operand2.WriteE();
//    else
      S += "`" + m_Operand2.WriteE() + '\'';
    }
  else
    S += "`???\"";
  return S;
  }

bool TMeaExpr::Measur_( MathExpr& ex, MathExpr& exm ) const
  {
  ex = m_Operand1;
  exm = m_Operand2;
  return true;
  }

TRoot::TRoot( const MathExpr& base, int Root ) : TPowr()
  {
  m_Name = '~';
  m_Operand1 = base;
  m_Root = Root;
  m_Operand2 = new TSimpleFrac( 1, m_Root );
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TRoot::Clone() const
  {
  TRoot* pResult = new TRoot(m_Operand1.Clone(), m_Root);
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TRoot::Reduce() const
  {
  MathExpr P, opr1, opr2, opr11, opr12, opr21, opr22, Temp1, Temp2, Temp;
  double re, im, r, f, fk, Value1, V;
  int N, D, M, iPrev, iOutRoot, iInRoot, iMult;
  MathExpr RootList;
  bool OldRootToPower, OldCalcOnly, IsNegative;
  char cOper;
  static MathExpr PreviousExp;

  std::function<void( const MathExpr& ex )>  Multiplicators = [&] ( const MathExpr& ex )
    {
    MathExpr exL, exR;
    int iV;

    if( ex.Multp( exL, exR ) )
      {
      Multiplicators( exL );
      Multiplicators( exR );
      }
    else
      {
      ex.Cons_int( iV );
      if( iV == iPrev )
        {
        N = N + 1;
        if( N == m_Root )
          {
          iOutRoot = iOutRoot * iV;
          iMult = 1;
          N = 0;
          }
        else
          iMult = iMult * iV;
        }
      else
        {
        iInRoot = iInRoot * iMult;
        iMult = iV;
        N = 1;
        }
      iPrev = iV;
      }
    };

  auto Value = [&] ()
    {
    if( Value1 == 0 ) return opr1;
    if( Value1 > 0 ) return Constant( exp( log( Value1 ) / m_Root ) );
    if( m_Root & 1 ) return Constant( -exp( log( -Value1 ) / m_Root ) );
    return CreateComplex( Constant( 0 ), Constant( exp( log( -Value1 ) / m_Root ) ) );
    };

  opr1 = m_Operand1.Reduce();
  if( sm_FullReduce && m_Operand1.Constan( Value1 ) )
    return Value();
  
  if( s_NoRootReduce && opr1.Cons_int( N ) )
    {
    if( N == 0 || N == 1 )
      return Constant( N );
    IsNegative = N < 0;
    if( IsNegative )
      opr1 = Constant( -N );
    N = 0;
    iPrev = 0;
    iOutRoot = 1;
    iInRoot = 1;
    iMult = 1;
    Multiplicators( opr1.ReduceToMultiplicators() );
    iInRoot = iInRoot * iMult;
    if( iOutRoot > 1 )
      {
      if( iInRoot == 1 )
        if( IsNegative )
          if( m_Root & 1 )
            return Constant( -iOutRoot );
          else
            return CreateComplex( Constant( 0 ), Constant( iOutRoot ) );
        else
          return Constant( iOutRoot );
      else
        {
        if( IsNegative )
          iInRoot = -iInRoot;
        return Constant( iOutRoot ) * ( Constant( iInRoot ).Root( m_Root ) );
        }
      }
    if( IsNegative )
      {
      opr1.Cons_int( N );
      opr1 = Constant( -N );
      }
    if( s_RootToPower )
      return opr1 ^ ( new TSimpleFrac( 1, m_Root ) );
    return opr1.Root( m_Root );
    }

  if( opr1.Power( opr11, opr12 ) && opr12.Cons_int( N ) )
    {
    if( N % m_Root == 0 )
      {
      N = N / m_Root;
      if( N != 1 )
        if( s_RootToPower )
          return opr11.Reduce() ^ N;
        else
          if( N > 0 )
            return opr11.Reduce() ^ N;
          else
            return Constant( 1 ) / ( opr11.Reduce() ^ -N );
      return opr11.Reduce();
      }
    else
      {
      if( s_RootToPower )
        return opr11.Reduce() ^ ( new TSimpleFrac( N, m_Root ) );
      else
        if( abs( N ) > m_Root )
          {
          M = abs( N ) / m_Root;
          P = opr11.Reduce();
          if( M > 1 )
            P = P ^ M;
          iInRoot = abs( N ) % m_Root;
          MathExpr Result;
          if( iInRoot == 0 )
            Result = P * ( opr11.Reduce().Root( m_Root ) );
          else
            Result = P * ( ( opr11.Reduce() ^ iInRoot ).Root( m_Root ) );
          if( N < 0 )
            return Constant( 1 ) / Result;
          }
      return opr1.Root( m_Root );
      }
    }

  opr2 = m_Operand2.Reduce();
  if( opr1.Root_( opr11, opr12, N ) )
    if( s_RootToPower )
      return opr11 ^ ( m_Root * N );
    else
      return opr11.Root( m_Root * N );

  if( opr1.Complex( opr11, opr12 ) && opr11.Constan( re ) && opr12.Constan( im ) )
    {
    r = sqrt( pow( re, 2 ) + pow( im, 2 ) );
    if( re == 0 ) 
      f = M_PI / 2;
    else
      f = atan( im / re );
    r = exp( log( r ) / m_Root );
    RootList = new TLexp;
    for( int N = 0; N < m_Root; N++ )
      {
      fk = ( f + 2 * M_PI * N ) / m_Root;
      if( im < 0 && re < 0 )
        P = CreateComplex( Constant( -r*sin( fk ) ), Constant( r*cos( fk ) ) );
      else
        P = CreateComplex( Constant( r*cos( fk ) ), Constant( r*sin( fk ) ) );
      CastPtr( TLexp, RootList )->Addexp( P );
      }
    return RootList;
    }

  MathExpr Result;
  if( opr1.SimpleFrac_( N, D ) )
    {
    if( N * D < 0 )
      {
      MathExpr C = new TComplexExpr( Constant(0), new TRoot( new TSimpleFrac( abs( N ), abs( D ) ), 2 ) );
      if( m_Root % 2 == 0 ) return C;
      return -C;
      }
    if( s_RootToPower )
      Temp = ( Constant( N ) ^ 1 ) / Constant( abs( m_Root ) );
    else
      Temp = Constant( N ).Root( abs( m_Root ) );

    opr1 = Temp.Reduce();
    Temp = Constant( D ).Root( abs( m_Root ) );
    opr2 = Temp.Reduce();

    if( m_Root < 0 )
      Temp = opr2 / opr1;
    else
      Temp = opr1 / opr2;

    if( Eq( PreviousExp ) )
      {
      Result = PreviousExp;
      PreviousExp.Clear();
      return Result;
      }
    PreviousExp = Ethis;
    Result = Temp.Reduce();
    PreviousExp.Clear();
    return Result;
    }

  if( !opr1.Constan( Value1 ) )
    {
    if( opr1.Multp( opr11, opr12 ) && opr11.Constan( V ) && opr11.Root( m_Root ).Reduce().Cons_int( N ) )
      if( s_RootToPower )
        Result = Constant( N ) * ( opr12 ^ ( new TSimpleFrac( 1, m_Root ) ) );
      else
        Result = Constant( N ) * opr12.Root( m_Root );
    else
      if( opr1.Divis( opr11, opr12 ) )
        {
        OldRootToPower = s_RootToPower;
        s_RootToPower = false;
        Temp1 = opr11.Root( m_Root );
        opr21 = Temp1.Reduce();
        Temp2 = opr12.Root( m_Root );
        opr22 = Temp2.Reduce();
        s_RootToPower = OldRootToPower;
        if( !Temp1.Eq( opr21 ) || !Temp2.Eq( opr22 ) )
          {
          if( s_RootToPower )
            {
            opr21 = opr21.ReduceTExprs();
            opr22 = opr22.ReduceTExprs();
            }
          return opr21 / opr22;
          }
        }
      else
        {
        OldCalcOnly = s_CalcOnly;
        s_CalcOnly = true;
        if( s_IsIntegral && opr1.SetOfFractions( Temp ) && Temp.Divis( opr21, opr22 ) && !( opr22.Cons_int( N ) && N == 1 ) )
          {
          OldRootToPower = s_RootToPower;
          s_RootToPower = false;
          Temp1 = Temp.Reduce();
          if( !opr1.Eq( Temp1 ) )
            {
            Temp = Temp1.Root( m_Root ).ReduceTExprs();
            if( !( IsConstType( TRoot, Temp ) ) )
              {
              TSolutionChain::sm_SolutionChain.AddAndReplace( Ethis, Temp );
              s_CalcOnly = OldCalcOnly;
              s_RootToPower = OldRootToPower;
              return Temp;
              }
            }
          else
            s_RootToPower = OldRootToPower;
          }
        s_CalcOnly = OldCalcOnly;
        s_GlobalInvalid = false;
        if( s_RootToPower && opr1.Power( opr11, opr12 ) )
          Result = ( opr11 ^ ( opr12 / Constant( ( m_Root ) ) ) ).Reduce();
        }
    }
  else
    if(sm_FullReduce)
      return Value();
    else
      return opr1.Root( m_Root );
  if( Result.IsEmpty() )
    {
    if( s_IsIntegral && opr1.Oper_( cOper, Temp1, Temp2 ) && ( cOper == '+' || cOper == '-' ) )
      {
      try
        {
        Temp1 = opr1.ReToMult();
        }
      catch( char* )
        {
        Temp1.Clear();
        }
      catch( ErrParser )
        {
        Temp1.Clear();
        }
      if( !Temp1.IsEmpty() && !s_GlobalInvalid && !Temp1.Eq( opr1 ) )
        {
        Temp2 = Temp1;
        while( Temp2.Binar( '=', P, Temp2 ) );
        Temp2 = Temp2.Reduce();
        if( Temp2.Power( Temp1, P ) && P.Cons_int( N ) )
          if( N % m_Root == 0 )
            {
            N = N / m_Root;
            if( N == 1 )
              Result = Temp1;
            else
              Result = Temp1 ^ N;
            }
          else
            if( s_RootToPower )
              Result = Temp1 ^ ( new TSimpleFrac( N, m_Root ) );
        }
      }
    if( Result.IsEmpty() )
      if( s_RootToPower )
        Result = ( opr1 ^ ( new TSimpleFrac( 1, m_Root ) ) ).Reduce();
      else
        Result = opr1.Root( m_Root );
    s_GlobalInvalid = false;
    }

  return Result;
  }

MathExpr TRoot::Perform() const
  {
  return m_Operand1.Perform().Root( m_Root );
  }

bool TRoot::Eq( const MathExpr& E2 ) const
  {
  MathExpr Op21, Op22;
  int rt2;

  if( E2.IsEmpty() ) return false;
  if( E2.Root_( Op21, Op22, rt2 ) )
    return m_Operand1.Eq( Op21 ) && m_Root == rt2;
  return false;
  }

bool TRoot::Equal( const MathExpr& E2 ) const
  {
  MathExpr Ar2, op21, op22;
  int rt2;
  QByteArray _eqName;

  if( E2.IsEmpty() ) return false;

  if( E2.Root_( op21, op22, rt2 ) )
    return m_Operand1.Equal( op21 ) && m_Root == rt2;

  if( E2.Power( op21, op22 ) )
    return m_Operand1.Equal( op21 ) && m_Operand2.Equal( op22 );

  if( E2.Divis( op21, op22 ) )
    return E2.Equal( Ethis );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );
  
  if( IsConstType( TConstant, E2 ) )
    return Reduce().Eq( E2 );

  return false;
  }

MathExpr TRoot::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return m_Operand1.Substitute( vr, vl ).Root( m_Root );
  }

QByteArray TRoot::WriteE() const
  {
  return "((" + m_Operand1.WriteE() + ")~" + QByteArray::number( m_Root ) + ")";
  }

bool TRoot::Root_( MathExpr& op1, MathExpr& op2, int& rt ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  rt = m_Root;
  return true;
  }

QByteArray TRoot::SWrite() const
  {
  if( m_Root == 2 )
    return "\\sqrt{" + m_Operand1.SWrite() + '}';
  return "\\root{" + QByteArray::number( m_Root ) + "}{" + m_Operand1.SWrite() + '}';
  }
/*
void TRoot::To_keyl( XPInEdit& Ed )
  {
  MathExpr value;
  if( m_Root == 2 )
    {
    Ed.UnRead( "SQROOT" );
    m_Operand1.To_keyl( Ed );
    Ed.UnRead( "vk_Right" );
    }
  else
    {
    Ed.UnRead( "ROOT" );
    value = Constant( (m_Root) );
    value.To_keyl( Ed );
    Ed.UnRead( "vk_Down" );
    m_Operand1.To_keyl( Ed );
    Ed.UnRead( "vk_Right" );
    }
  }
  */
MathExpr TRoot::Diff( const QByteArray& d )
  {
  MathExpr Op1;
  if( ConstExpr() )
    {
    return Constant( 0 );
    }
  if( s_TruePolynom )
    throw  ErrParser( "Wrong type of equation!", peNoSolvType ); 
  Op1 = m_Operand1.Reduce();
  if( m_Root == 2 )
    return Op1.Diff( d ) / (Constant( m_Root ) * Op1.Root( m_Root ));
  else
    return Op1.Diff( d ) / (Constant( m_Root ) * (Op1 ^ (new TSimpleFrac( m_Root - 1, m_Root ))));
  }

bool TRoot::Positive() const
  {
  return !(m_Root & 1);  
  }

TRoot1::TRoot1( const MathExpr& Ex1, const MathExpr& Ex2 ) : TPowr( Ex1, Ex2 )
  {
  m_Name = '~';
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TRoot1::Clone() const
  {
  TRoot1* pResult = new TRoot1(m_Operand1.Clone(), m_Operand2.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TRoot1::Reduce() const
  {
  MathExpr oper2;
  int root, Nom, Denom;

  oper2 = m_Operand2.Reduce();
  if( oper2.Cons_int( root ) )
    return m_Operand1.Reduce().Root( root ).Reduce();
  else
    if( oper2.SimpleFrac_( Nom, Denom ) )
      return (m_Operand1 ^ Denom ).Root( Nom ).Reduce();
    else
      return new TRoot1( m_Operand1.Reduce(), oper2 );
  }

MathExpr TRoot1::Perform() const
  {
  return new TRoot1( m_Operand1.Perform(), m_Operand2.Perform() );
  }

bool TRoot1::Eq( const MathExpr& E2 ) const
  {
  MathExpr Op21, Op22;
  if( E2.Root1_( Op21, Op22 ) )
    return m_Operand1.Eq( Op21 ) && m_Operand2.Eq( Op22 );
  return false;
  }

bool TRoot1::Equal( const MathExpr& E2 ) const
  {
  MathExpr Op21, Op22;
  if( E2.Root1_( Op21, Op22 ) )
    return m_Operand1.Equal( Op21 ) && m_Operand2.Equal( Op22 );
  return false;
  }

MathExpr TRoot1::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TRoot1( m_Operand1.Substitute( vr, vl ), m_Operand2.Substitute( vr, vl ) );
  }

QByteArray TRoot1::WriteE() const
  {
  return "((" + m_Operand1.WriteE() + ")~" + m_Operand2.WriteE() + ")";
  }


QByteArray TRoot1::SWrite() const
  {
  return "\\root{" + m_Operand2.SWrite() + "}{" + m_Operand1.SWrite() + "}";
  }

bool TRoot1::Root1_( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TRoot1::Root1( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = Constant( 1 ) / m_Operand2;
  return true;
  }
  
TMixedFrac::TMixedFrac( int I, int N, int D ): TSimpleFrac( N, D )
  {
  if( (abs( N ) + abs( 1.0 * I * D )) > 1.0E7 )
    {
    if( !s_CalcOnly )
      Error_m( X_Str( "MTooLargeForFrac", "Too large" ) );
    s_CheckError = true;
    m_Nom = sc_NumError; //s_NumError
    m_Denom = D;
    m_IntPart = I; //IntP
    m_NomPart = N; //NomP
    }
  else
    {
    if( D < 0 )
      {
      D = -D;
      I = -I;
      N = -N;
      }
    if( I != 0 && N < 0 )
      N = -N;
    if( I == 0 )
      {
      m_Nom = N;
      m_Denom = D;
      if( D == 0 )
        {

        s_CheckError = true;

        if( !s_CalcOnly )
          Error_m( X_Str( "MDenomIs0", "Denominator equals 0" ) );
        m_IntPart = 0;
        m_NomPart = N;
        }
      else
        {
        m_IntPart = N / D;
        m_NomPart = N - m_IntPart * D;
        if( (m_IntPart < 0) && (m_NomPart < 0) ) m_NomPart = -m_NomPart;
        }
      }
    else
      {
      if( (I < 0) && (N > 0) )
        m_NomPart = -N;
      else
        m_NomPart = N;
      m_IntPart = I;
      m_Denom = D;
      m_Nom = m_NomPart + m_IntPart * m_Denom;
      }
    }
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TMixedFrac::Clone() const
  {
  TMixedFrac* pResult = new TMixedFrac( m_IntPart, m_NomPart, m_Denom );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TMixedFrac::Reduce() const
  {
  int N = m_Nom;
  int D = m_Denom;
  CancFrac( N, D );

  if( D == 1 )
    return Constant( N );
  else
    return  GenerateFraction( N, D );
  }

bool TMixedFrac::IsLinear() const
  {
  return true;
  }

bool TMixedFrac::Eq( const MathExpr& E2 ) const
  {
  int  I2, N2, D2;
  if( E2.MixedFrac_( I2, N2, D2 ) )
    return m_IntPart == I2 && m_NomPart == N2 && m_Denom == D2;
  else
    return false;
  }

bool TMixedFrac::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2;
  int I2, N2, D2;
  QByteArray _eqName;

  if( E2.MixedFrac_( I2, N2, D2 ) )
    return Eq( E2 );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  if( IsConstType( TDivi, E2 ) || IsConstType( TSimpleFrac, E2 ) )
    return Reduce().Equal( E2 );

  return false;
  }

QByteArray TMixedFrac::WriteE() const
  {
  return QByteArray::number( m_IntPart ) + '{' + QByteArray::number( m_NomPart ) + '/' + QByteArray::number( m_Denom ) + '}';
  }

QByteArray TMixedFrac::SWrite() const
  {
  return QByteArray::number( m_IntPart ) + "\\frac{" + QByteArray::number( m_NomPart ) + "}{" + QByteArray::number( m_Denom ) + '}';
  }
/*
void TMixedFrac::To_keyl( XPInEdit& Ed )
  {
  int i;
  QByteArray Nstr =  QByteArray::number( m_IntPart );
  for( i = 0; i < Nstr.length(); i++ )
    Ed.UnRead( Nstr[i] );
  Ed.UnRead( "FRACTION" );

  Nstr = QByteArray::number( abs( m_NomPart ) );
  for( i = 0; i < Nstr.length(); i++ )
    Ed.UnRead( Nstr[i] );
  Ed.UnRead( "vk_Down" );

  Nstr = QByteArray::number( m_Denom );
  for( i = 0; i < Nstr.length(); i++ )
    Ed.UnRead( Nstr[i] );
  Ed.UnRead( "vk_Right" );
  }
  */
bool TMixedFrac::MixedFrac_( int& I, int& N, int& D ) const
  {
  I = m_IntPart;
  N = m_NomPart;
  D = m_Denom;
  return true;
  }

bool TMixedFrac::Negative() const
  {
  return m_IntPart < 0;
  }

bool TMixedFrac::ConstExpr() const
  {
  return true;
  }

bool TMixedFrac::Unarminus( MathExpr& A ) const
  {
  if( !(m_IntPart < 0) )
    return false;

  A = new TMixedFrac( -m_IntPart, m_NomPart, m_Denom );
  return true;
  }

TPowr::TPowr( const MathExpr& Ex1, const MathExpr& Ex2 ) : TOper()
  {
  QByteArray VarName;
  if( ( Ex1 == nullptr ) || ( Ex2 == nullptr ) )
    throw  ErrParser( "Internal Error!", peSyntacs );
  if( Ex1.Variab( VarName ) && ( VarName == "i" ) )
    m_Operand1 = CreateComplex( Constant( 0 ), Constant( 1 ) );
  else
    m_Operand1 = Ex1;
  m_Operand2 = Ex2;
  m_Name = '^';
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

TPowr::TPowr( const MathExpr& base, int Power ) : TOper()
  {
  QByteArray VarName;

  if( base.Variab( VarName ) && VarName == "i" )
    m_Operand1 = CreateComplex( Constant( 0 ), Constant( 1 ) );
  else
    m_Operand1 = base;
  m_Operand2 = Constant( Power );
  m_Name = '^';
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

TPowr::TPowr( const MathExpr& base, int Nom, int Denom ) : TOper()
  {
  QByteArray VarName;
  if( base.Variab( VarName ) && ( VarName == "i" ) )
    m_Operand1 = CreateComplex( Constant( 0 ), Constant( 1 ) ); 
  else
    m_Operand1 = base;
  m_Operand2 = new TSimpleFrac( Nom, Denom );
  m_Name = '^';
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  };

MathExpr TPowr::Clone() const
  {
  MathExpr Result = m_Operand1.Clone() ^ m_Operand2.Clone();
  Result.Ptr()->m_WasReduced = m_WasReduced;
  return Result;
  }

bool TPowr::IsLinear() const
  {
  return !( IsConstType( TRoot, MathExpr( (TExpr*) this ) ) ) && IsConstType( TConstant, m_Operand2 );
  }

MathExpr TPowr::TrigTerm( const QByteArray& Name, const MathExpr& exArg, const MathExpr& exPower )
  {
  MathExpr Result = m_Operand1.TrigTerm( Name, exArg );
  if( !Result.IsEmpty() && !exPower.IsEmpty() && m_Operand2.Eq( exPower ) )
    return ( Result ^ m_Operand2 ).Reduce();
  return Result;
  }

MathExpr TPowr::Reduce() const
  {
  MathExpr  P, OldAnswer;
  double b, re, im;
  double V, Value1, Value2;
  int N, D, N1, D1;
  MathExpr opr1, opr2, op11, op12, op21, op22;
  int iPrev, iOutRoot, iInRoot, iMult;
  MathExpr Temp;
  bool SignValue, OldPutAnswer, OldCalcOnly, IsNegative;
  int Pow, i;
  QByteArray sVar, sName;

  std::function<void( const MathExpr& )> Multiplicators = [&] ( const MathExpr& ex )
    {
    MathExpr exL, exR;
    int iV;

    if( ex.Multp( exL, exR ) )
      {
      Multiplicators( exL );
      Multiplicators( exR );
      }
    else
      {
      ex.Cons_int( iV );
      if( iV == iPrev )
        {
        if( ++N == D1 )
          {
          iOutRoot *= iV;
          iMult = 1;
          N = 0;
          }
        else
          iMult *= iV;
        }
      else
        {
        iInRoot *= iMult;
        iMult = iV;
        N = 1;
        }
      iPrev = iV;
      }
    };

  std::function<int( const MathExpr& )> IsOdd = [&] ( const MathExpr& ex )
    {
    MathExpr exLeft, exRight;
    int N, M;
    QByteArray Name;

    if( ex.Cons_int( N ) )
      if( N & 1 )
        return 1;
      else
        return 2;

    if( ex.Multp( exLeft, exRight ) )
      {
      if( exRight.Variab( Name ) && ( Name == "N" ) && ( IsOdd( exLeft ) == 2 ) )
        return 2;
      return 0;
      }

    if( ex.Summa( exLeft, exRight ) || ex.Subtr( exLeft, exRight ) )
      {
      N = IsOdd( exLeft );
      M = IsOdd( exRight );
      if( N * M == 0 )
        return 0;
      if( ( N + M ) & 1 )
        return 1;
      else
        return 2;
      }
    return 0;
    };

  Pow = 0;
  if( !m_Operand2.Cons_int( Pow ) && m_Operand2.Unarminus( opr2 ) )
    {
    opr2.Cons_int( Pow );
    Pow = -Pow;
    }
  if( IsConstType( TMatr, m_Operand1 ) )
    if( Pow == -1 )
      return CastConstPtr( TMatr, m_Operand1 )->Inversion();
    else
      if( m_Operand2.Variab( sVar ) && sVar == "T" )
        return CastConstPtr( TMatr, m_Operand1 )->Transpose();

  if( m_Operand2.Log( op11, op12 ) )
    {
    if( m_Operand1.Eq( op11 ) )
      return op12;
    else
      {
      if( !m_Operand1.ConstExpr() )
        return Ethis;
      OldPutAnswer = s_PutAnswer;
      s_PutAnswer = true;
      P = new TBinar( '=', op11 ^ Variable( "y" ), m_Operand1 );
      OldAnswer = s_Answer;

      OldCalcOnly = s_CalcOnly;
      s_CalcOnly = true;
      MathExpr Result;
      if( CalcExpEq( P.WriteE(), sm_pResultReceiver ) )
        {
        Temp = s_Answer;
        if( IsConstType( TLexp, s_Answer ) )
          Temp = CastPtr( TLexp, s_Answer )->m_pLast->m_Memb;
        Temp.Binar( '=', opr1, Temp );
        Temp = op12 ^ Temp;
        Result = m_Operand1 ^ m_Operand1.Log( Temp.Reduce() );
        s_ReductionMustBeContinued = true;
        }
      else
        Result = Ethis;
      s_CalcOnly = OldCalcOnly;
      s_PutAnswer = OldPutAnswer;
      s_Answer = OldAnswer;
      return Result;
      }
    }

  if( m_Operand2.Multp( opr1, opr2 ) && opr2.Log( op11, op12 ) )
    {
    P = m_Operand1 ^ op11.Log( op12 ^ opr1 );
    return P.Reduce();
    };

  if( m_Operand1.Root_( op11, op12, N ) && op11.ConstExpr() )
    return ( op11 ^ ( op12 * m_Operand2 ) ).Reduce();

  if( m_Operand1.Unarminus( opr1 ) && m_Operand2.Cons_int( Pow ) && opr1.Root_( op11, op12, N ) && op11.ConstExpr() )
    {
    P = ( op11 ^ ( op12 * m_Operand2 ) ).Reduce();
    if( Pow & 1 )
      return -P;
    return P;
    }

  opr1 = m_Operand1.Reduce();

  if( opr1 == 1 )
    return opr1;

  opr2 = m_Operand2.Reduce();

  if( opr1 == -1 )
    {
    switch( IsOdd( opr2 ) )
      {
      case 0:
        return opr1 ^ opr2;
      case 1:
        return Constant( -1 );
      case 2:
        return Constant( 1 );
      }
    };

  if( opr1.Divis( op11, op12 ) )
    {
    P = op11 ^ opr2;
    op21 = P.Reduce();
    P = op12 ^ opr2;
    op22 = P.Reduce();
    if( !( IsConstType( TPowr, op21 ) ) || !( IsConstType( TPowr, op22 ) ) )
      if( op22 == 1 )
        return op21;
      else
        return op21 / op22;
    }

  if( s_NoExpReduce && opr1.Funct( sVar, op11 ) && sVar == "exp" )
    return Function( "exp", ( op11 * opr2 ).Reduce() );

  if( s_PowerToRoot && opr2.SimpleFrac_( N, D ) )
    {
    N1 = N % D;
    P = opr1;
    if( abs( N1 ) > 1 )
      P = P ^ abs( N1 );
    P = P.Root( D );
    Pow = abs( N / D );
    MathExpr Result;
    if( Pow == 0 )
      Result = P;
    else
      if( ( IsConstType( TSumm, opr1 ) ) || ( IsConstType( TSubt, opr1 ) ) )
        return opr1 ^ opr2;
      else
        if( Pow == 1 )
          Result = opr1 * P;
        else
          Result = ( opr1 ^ Pow ) * P;
    if( opr2.Negative() )
      Result = Constant( 1 ) / Result;
    return Result;
    }

  if( s_PowerToFraction && opr2.Negative() )
    {
    P = -( opr2 ).Reduce();
    return Constant( 1 ) / ( opr1 ^ P ).Reduce();
    }

  if( s_NoRootReduce && opr1.ConstExpr() && !opr1.SimpleFrac_( N, D ) && opr2.SimpleFrac_( N1, D1 ) )
    {
    if( !opr1.Cons_int( N ) )
      return opr1 ^ opr2;
    if( N == 0 || N == 1 )
      return Constant( N );
    IsNegative = N < 0;
    if( IsNegative )
      opr1 = Constant( -N );
    N = 0;
    iPrev = 0;
    iOutRoot = 1;
    iInRoot = 1;
    iMult = 1;
    Multiplicators( opr1.ReduceToMultiplicators() );
    iInRoot = iInRoot * iMult;
    if( iOutRoot > 1 )
      {
      MathExpr Result;
      if( N1 != 1 )
        {
        if( IsNegative ) iOutRoot = -iOutRoot;
        P = Constant( iOutRoot ) ^ N1;
        Result = P.Reduce();
        }
      else
        if( IsNegative )
          if( D1 & 1 )
            Result = Constant( -iOutRoot );
          else
            Result = CreateComplex( Constant( 0 ), Constant( iOutRoot ) );
        else
          Result = Constant( iOutRoot );

      if( iInRoot > 1 )
        return Result * ( Constant( iInRoot ) ^ opr2 );
      return Result;
      }
    if( IsNegative )
      {
      opr1.Cons_int( N );
      opr1 = Constant( ( -N ) );
      }
    return opr1 ^ opr2;
    }

  if( opr1.Root_( op11, op12, N ) )
    return ( op11 ^ ( op12 * m_Operand2 ) ).Reduce();

  if( opr1.Funct( sName, op11 ) )
    {
    if( sName == "exp" )
      {
      P = op11 * opr2;
      return Function( "exp", P.Reduce() );
      }
    return opr1 ^ opr2;
    }

  if( opr2.Log( op11, op12 ) && opr1.Equal( op11 ) )
    return op12;

  if( opr1.Complex( op11, op12 ) && opr2.Cons_int( Pow ) && op11.Constan( re ) && op12.Constan( im ) )
    {
    if( re == 0.0 )
      {
      for( i = 1; i < Pow; i++ )
        im *= im;
      switch( Pow % 4 )
        {
        case 1:
          return CreateComplex( 0.0, im );
        case 2: 
          return Constant( -im );
        case 3:
          return CreateComplex( 0.0, -im );
        case 4:
          return Constant( im );
        }
      }
    MathExpr Result = opr1;
    for( i = 1; i < Pow; i++ )
      Result = ( Result * opr1 ).Reduce();
    return Result;
    };

  if( opr1.Measur_( op11, op12 ) && opr2.Cons_int( Pow ) )
    {
    P = op12;
    for( i = 2; i <= Pow; i++ )
      P = P * op12;
    P = P.Reduce();
    return new TMeaExpr( ( op11 ^ Pow ).Reduce(), P );
    }

  if( opr2.Constan( Value1 ) && abs( Value1 ) < 0.0000001 )
    return Constant( 1 );

  //
  if(this->m_Name == '^' && opr1.Constan( Value1 ) && abs( Value1 )==0 && opr2.Constan( Value2 ) &&  Value2<0)
  {s_LastError="INFVAL"; s_GlobalInvalid=true; return Ethis;}

  if( opr1.Constan( Value1 ) && abs( Value1 ) < 0.0000001 )
    return Constant( 0 );

  if( opr1.Constan( Value1 ) && opr2.Cons_int( Pow ) )
    {
    V = 1;
    for( i = 1; i <= abs( Pow ); i++ )
      V *= Value1;
    if( Pow > 0 )
      return Constant( V );
    else
      if( opr1.Cons_int( N ) && ( V < INT_MAX ) )
        return new TSimpleFrac( 1, Round( V ) );
      else
        return Constant( ( 1.0 / V ) );
    }

  if( opr1.Constan( Value1 ) && m_Operand2.SimpleFrac_( N, D ) )
    {
    b = PowerF( Value1, N, D );
    if( s_GlobalInvalid )
      return Ethis;
    else
      return Constant( b );
    }

  if( m_Operand1.SimpleFrac_( N1, D1 ) && m_Operand2.SimpleFrac_( N, D ) )
    {
    if( s_NoRootReduce )
      {
      opr1 = Constant( N1 ) ^ m_Operand2;
      opr2 = opr1.Reduce();
      if( opr2.Cons_int( N1 ) )
        {
        opr1 = Constant( D1 ) ^ m_Operand2;
        opr2 = opr1.Reduce();
        if( opr2.Cons_int( D1 ) )
          return MathExpr( new TSimpleFrac( N1, D1 ) ) ^ m_Operand2;
        }
      return Ethis;
      }
    b = PowerF( N1, N, D ) / PowerF( D1, N, D );
    if( s_GlobalInvalid )
      return Ethis;
    else
      return Constant( b );
    }

  if( opr1.SimpleFrac_( N1, D1 ) && opr2.Constan( Value2 ) )
    {
    SignValue = ( Value2 < 0 );
    Value2 = abs( Value2 );

    Temp = Constant( N1 ) ^ Constant( Value2 );
    opr1 = Temp.Reduce();

    Temp = Constant( D1 ) ^ Constant( Value2 );
    opr2 = Temp.Reduce();

    if( SignValue )
      Temp = opr2 / opr1;
    else
      Temp = opr1 / opr2;
    return Temp.Reduce();
    }

  if( opr1.Constan( Value1 ) && opr2.Cons_int( N ) )
    {
    b = PowerI( Value1, N );
    if( s_GlobalInvalid )
      return Ethis;
    return Constant( b );
    }

  //Correction( a*b ) ^ n = a^n*b^n
  if( opr2.Cons_int( Pow ) || opr2.SimpleFrac_( N, D ) )
    {
    Temp = opr1;
    if( !opr1.Multp( op11, op12 ) && s_SummExpFactorize && ( QByteArray( "exp" ).indexOf( opr1.WriteE() ) != -1 ) )
      Temp = opr1.ReToMult();
    if( sm_Factorize && Temp.Multp( op11, op12 ) )
      {
      P = op11 ^ opr2;
      if( ( IsConstType( TPowr, op11 ) ) || ( op11.Funct( sName, op11 ) && sName == "exp" ) )
        op21 = P.Reduce();
      else
        op21 = P;
      P = op12 ^ opr2;
      if( ( IsConstType( TPowr, op12 ) ) || ( op12.Funct( sName, op11 ) && sName == "exp" ) )
        op22 = P.Reduce();
      else
        op22 = P;
      return op21 * op22;
      }
    }

  if( opr1.Constan( Value1 ) && opr2.Constan( Value2 ) && ( Value1 != 0 ) )
    {
    b = PowerR( Value1, Value2 );
    if( s_GlobalInvalid )
      return Ethis;
    return Constant( b );
    }

  if( opr1.Unarminus( op11 ) && opr2.Cons_int( N ) )
    {
    P = op11 ^ opr2;
    if( N % 2 == 0 )
      return P;
    return -P;
    }

  if( opr1.Constan( Value1 ) )
    {
    if( Value1 == 0 || Value1 == 1 )
      P = opr1;
    }
  else
    if( opr2.Constan( Value2 ) )
      {
      if( Value2 == 0 )
        P = Constant( ( 1 ) );
      if( Value2 == 1 )
        P = opr1.Reduce();
      }

  if( opr1.Power( op11, op12 ) )
    P = op11 ^ ( op12 * opr2 );

  if( s_PowerToMult )
    if( opr2.Summa( op11, op12 ) )
      {
      if( op11.Cons_int( N ) )
        P = ( opr1 ^ N ) * ( opr1 ^ op12 );
      else
        if( op12.Cons_int( N ) )
          P = ( opr1 ^ N ) * ( opr1 ^ op11 );
      }
    else
      if( opr2.Subtr( op11, op12 ) )
        {
        if( op11.Cons_int( N ) )
          P = ( opr1 ^ N ) * ( opr1 ^ ( -( op12 ) ) );
        else
          if( op12.Cons_int( N ) )
            P = ( opr1 ^ -N ) * ( opr1 ^ op11 );
        }

  if( P.IsEmpty() )
    if( s_IsIntegral && opr1.ConstExpr() && opr2.Multp( op11, op12 ) && ( IsConstType( TConstant, op11 ) ) )
      {
      P = opr1 ^ op11;
      return P.Reduce() ^ op12;
      }
    else
      return opr1 ^ opr2;
  else
    return P.Reduce();
  }

MathExpr TPowr::Perform() const
  {
  return m_Operand1.Perform() ^ m_Operand2.Perform();
  }

MathExpr TPowr::Diff( const QByteArray& d )
  {
  MathExpr  Op1, Op2, Diff1, Ex;
  int Pr, Nom, Den;

  Op1 = m_Operand1.Reduce();
  Op2 = m_Operand2.Reduce();

  if( Op2.Cons_int( Pr ) )
    return Constant( Pr ) * ( Op1 ^ ( Pr - 1 ) ) * Op1.Diff( d );

  if( Op2.SimpleFrac_( Nom, Den ) )
    return Op2 * ( Op1 ^ ( Op2 - Constant( 1 ) ) ) * Op1.Diff( d );

  Diff1 = Op1.Diff( d );
  if( Diff1 == 0 )
    return MathExpr( this ) * Op2.Diff( d ) * new TFunc( true, "ln", Op1 );
  return MathExpr( this ) * ( Op2.Diff( d ) * new TFunc( true, "ln", Op1 ) + Diff1 * Op2 / Op1 );
  }

bool TPowr::Eq( const MathExpr& E2 ) const
  {
  MathExpr  Op21, Op22;
  if( E2.Power( Op21, Op22 ) && !( IsConstType( TRoot, E2 ) ) && !( IsConstType( TRoot1, E2 ) ) )
    return m_Operand1.Eq( Op21 ) && m_Operand2.Eq( Op22 );
  return false;
  }

bool TPowr::Equal( const MathExpr& E2 ) const
  {
  MathExpr Op21, Op22;

  if( E2.Power( Op21, Op22 ) )
    return ( m_Operand1.Equal( Op21 ) && m_Operand2.Equal( Op22 ) );

  int rt2;

  if( E2.Root_( Op21, Op22, rt2 ) )
    return ( m_Operand1.Equal( Op21 ) && m_Operand2.Equal( Op22 ) );

  if( E2.Divis( Op21, Op22 ) )
    return E2.Equal( Ethis );

  MathExpr  Ar2;
  QByteArray _eqName;

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );

  return false;
  }

bool TPowr::EqualBase( const MathExpr& ex )
  {
  if( m_Operand1.Equal( ex ) )
    return true;

  int Nom, Denom;
  MathExpr exTemp1, exTemp2;
  if( ( m_Operand2.Cons_int( Nom ) || m_Operand2.SimpleFrac_( Nom, Denom ) ) && !( Nom & 1 ) )
    {
    exTemp1 = m_Operand1 + ex;
    exTemp2 = exTemp1.Reduce();
    return  exTemp2 == 0;
    }
  return false;
  }

MathExpr TPowr::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return m_Operand1.Substitute( vr, vl ) ^ m_Operand2.Substitute( vr, vl );
  }

bool TPowr::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  MathExpr exBase, exPower;
  int Nom, Denom;
  if( TOper::Replace( Target, Source ) ) return true;
  if( Target.Power( exBase, exPower ) && m_Operand2.Cons_int( Nom ) && m_Operand1.Equal( exBase ) && exPower.Cons_int( Denom ) && ( Nom % Denom == 0 ) )
    {
    m_Operand1 = Source;
    m_Operand2 = Constant( Nom / Denom );
#ifdef DEBUG_TASK
    m_Contents = WriteE();
#endif
    return true;
    }
  return false;
  }

bool TPowr::Root_( MathExpr& op1, MathExpr& op2, int& rt ) const
  {
  int  Nom;

  if( m_Operand2.SimpleFrac_( Nom, rt ) && Nom == 1 )
    {
    op1 = m_Operand1;
    op2 = m_Operand2;
    return true;
    }
  return false;
  }
/*
void TPowr::To_keyl( XPInEdit& Ed )
  {
  QByteArray  N;
  double V;
  bool Vb;
  MathExpr ex;

  Ed.UnRead( "POWER" );

  if( m_Operand1.Variab( N ) || ( m_Operand1.Constan( V ) && !m_Operand1.Negative() ) || m_Operand1.Boolean_( Vb ) || m_Operand1.Matr( ex ) )
    m_Operand1.To_keyl( Ed );
  else
    {
    Ed.UnRead( "(" );
    m_Operand1.To_keyl( Ed );
    Ed.UnRead( ")" );
    }

  Ed.UnRead( "vk_Up" );
  m_Operand2.To_keyl( Ed );
  Ed.UnRead( "vk_Right" );
  }
  */
bool TPowr::Power( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TPowr::Negative() const
  {
  if( !m_Operand1.Negative() ) return false;

  int Nom, Denom;
  if( m_Operand2.Cons_int( Nom ) )
    return Nom & 1;
  if( m_Operand2.SimpleFrac_( Nom, Denom ) && Denom & 1 )
    return Nom & 1;

  return false;
  }

bool TPowr::Positive() const
  {
  if( m_Operand1.Positive() ) return true;

  int Nom, Denom;
  if( m_Operand2.Cons_int( Nom ) )
    return !( Nom & 1 );
  if( m_Operand2.SimpleFrac_( Nom, Denom ) )
    return !( Nom & 1 ) || !( Denom & 1 );

  return false;
  }

QByteArray TPowr::WriteE() const
  {
  return '(' + m_Operand1.WriteE() + ')' + m_Name + '(' + m_Operand2.WriteE() + ')';
  }

QByteArray TPowr::SWrite() const
  {
  QByteArray Base;
  bool V;
  MathExpr ex;
  if( m_Operand1.MustBracketed() != brPower && ( ( IsConstType( TVariable, m_Operand1 ) ) ||
    ( ( IsConstType(TConstant, m_Operand1) ) && !m_Operand1.Negative() ) || m_Operand1.Boolean_(V) || m_Operand1.Matr(ex) ) )
    Base = "\\power{" + m_Operand1.SWrite();
  else
    Base = "\\power{(" + m_Operand1.SWrite() + ')';
  return Base + "}{" + m_Operand2.SWrite() + '}';
  }

bool TOper::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  MathExpr exBase, exPower, exArg1, exArg2, exTmp;
  int Nom, Denom;
  QByteArray sName1, sName2;
  bool Result = false;

  if( m_Operand1.Equal( Target ) )
    {
    m_Operand1 = Source;
    Result = true;
    }
  else
    Result = m_Operand1.Replace( Target, Source );

  if( m_Operand2.Equal( Target ) )
    {
    m_Operand2 = Source;
    Result = true;
    }
  else
    Result = m_Operand2.Replace( Target, Source ) || Result;

  if( Target.Funct( sName2, exArg2 ) && sName2 == "exp" )
    {
    if( m_Operand1.Funct( sName1, exArg1 ) && sName1 == "exp" )
      {
      exTmp = exArg1 / exArg2;
      exPower = exTmp.Reduce();
      m_Operand1 = Source ^ exPower;
      Result = true;
      }
    if( m_Operand2.Funct( sName1, exArg1 ) && sName1 == "exp" )
      {
      exTmp = exArg1 / exArg2;
      exPower = exTmp.Reduce();
      m_Operand2 = Source ^ exPower;
      Result = true;
      }
    }

  if( m_Name == '^' || m_Name == '~' )
    return Result;

  if( Target.Power( exBase, exPower ) && exPower.SimpleFrac_( Nom, Denom ) && Nom == 1 )
    if( m_Operand1.Equal( exBase ) )
      {
      m_Operand1 = Source ^ Denom;
      Result = true;
      }
    else
      if( m_Operand2.Equal( exBase ) )
        {
        m_Operand2 = Source ^ Denom;
        Result = true;
        }
#ifdef DEBUG_TASK
  if( Result ) m_Contents = WriteE();
#endif
  return Result;
  }

bool TOper::Oper_( char& N, MathExpr& op1, MathExpr& op2 ) const
  {
  N = m_Name;
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

bool TOper::Splitted() const
  {
  bool Result = false;

  if( !m_Operand1.IsEmpty() )
    Result = m_Operand1.Splitted();

  if( !m_Operand2.IsEmpty() )
    Result = Result || m_Operand2.Splitted();

  return Result;
  }

bool TOper::Negative() const
  {
  if( !m_Operand1.IsEmpty() )
    return m_Operand1.Negative();
  return false;
  }

bool TOper::ConstExpr() const
  {
  return m_Operand1.ConstExpr() && m_Operand2.ConstExpr();
  }

bool TOper::IsFactorized( const QByteArray& VarName ) const
  {
  QByteArray Name;

  auto IsProduct = [&] ( const MathExpr& op )
    {
    MathExpr  op1, op2;
    bool Result = op.Variab( Name ) && Name == VarName;
    if( Result )
      return true;
    if( !op.Multp( op1, op2 ) )
      return Result;
    if( ( !op1.Variab( Name ) && !op2.Variab( Name ) ) || Name != VarName )
      return Result;

    if( ( IsConstType( TConstant, op1 ) ) || ( IsConstType( TConstant, op2 ) ) )
      return true;
    if( op1.Variab( Name ) && ( Name != VarName ) )
      return true;

    return op2.Variab( Name ) && ( Name != VarName );
    };

  if( !( IsConstType( TSumm, MathExpr( ( TOper* ) this ) ) || IsConstType( TSubt, MathExpr( ( TOper* ) this ) ) ) )
    return false;

  return IsProduct( m_Operand1 ) && ( ( IsConstType( TConstant, m_Operand2 ) ) || ( m_Operand2.Variab( Name ) && Name != VarName ) ) ||
    IsProduct( m_Operand2 ) && ( ( IsConstType( TConstant, m_Operand1 ) ) || ( m_Operand1.Variab( Name ) && Name != VarName ) );
  }

bool TOper::HasComplex() const
  {
  return m_Operand1.HasComplex() || m_Operand2.HasComplex();
  }

bool TOper::HasMatrix() const
  {
  return m_Operand1.HasMatrix() || m_Operand2.HasMatrix();
  }

TSimpleFrac::TSimpleFrac( int N, int D ) : TExpr()
  {
  if( D == 0 )
    {
    if( !s_CalcOnly )
      Error_m( X_Str( "MDenomIs0", "Denominator equals 0" ) );
    s_CheckError = true;
    };
  m_Nom = N;
  m_Denom = D;
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TSimpleFrac::Clone() const
  {
  TSimpleFrac *pResult = new TSimpleFrac( m_Nom, m_Denom );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TSimpleFrac::Reduce() const
  {
  int N = m_Nom;
  int D = m_Denom;

  CancFrac( N, D );
  if( s_CheckError )
    return Ethis;

  if( sm_FullReduce)
    return Constant( (double) N / D ) ;

  if( N == 0 )
    return Constant( 0 );
  if( abs( D ) == 1 )
    return Constant( N * D );
  if( s_FractionToPower && N == 1 )
    return Constant( D ) ^ -1;
  if( N < 0 && D < 0 )
    {
    N = -N;
    D = -D;
    }

  return GenerateFraction( N, D );  
  }

MathExpr TSimpleFrac::Diff( const QByteArray& d )
  {
  return Constant( 0 );
  }

MathExpr TSimpleFrac::Integral( const QByteArray& d )
  {
  return MathExpr( this) * Variable( d );
  }

MathExpr TSimpleFrac::Lim( const QByteArray& v, const MathExpr lm ) const
  {
  return Clone();
  }

bool TSimpleFrac::Eq( const MathExpr& E2 ) const
  {
  int  N2, D2, I2, Np2;

  if( E2.SimpleFrac_( N2, D2 ) && !E2.MixedFrac_( I2, Np2, D2 ) )
    {
    int n = 0;
    if( m_Nom>0 )
      n++;
    if( m_Denom>0 )
      n++;
    if( N2>0 )
      n++;
    if( D2>0 )
      n++;
    return abs( m_Nom ) == abs( N2 ) && abs( m_Denom ) == abs( D2 ) && !(n & 1);
    }
  return false;
  }

bool TSimpleFrac::Equal( const MathExpr& E2 ) const
  {

  int N2, D2;

  if( E2.SimpleFrac_( N2, D2 ) )
    {
    if( Eq( E2 ) ) return true;
    return Reduce().Eq( E2.Reduce() );
    }

  MathExpr Ar2 = E2;
  double v;
  TConstant *pConst = CastPtr( TConstant, Ar2 );
  if( pConst != nullptr )
    {
    if( m_Denom == 0 )
      return false;
    return abs( pConst->Value() - (double) m_Nom / m_Denom ) < pConst->Precision();
    }
  QByteArray _eqName;
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( E2.Perform() );
  MathExpr Nom, Denom;
  if( E2.Divis( Nom, Denom ) )
    {
    if( Nom == m_Nom && Denom == m_Denom ) return true;
    return Reduce().Eq( ( Nom / Denom ).Reduce() );
    }
  return false;
  }

QByteArray TSimpleFrac::WriteE() const
  {
  return '(' + QByteArray::number( m_Nom ) + '/' + QByteArray::number( m_Denom ) + ')';
  }

QByteArray TSimpleFrac::SWrite() const
  {
  if( Negative() )
    return "-\\frac{" + QByteArray::number( abs( m_Nom ) ) + "}{" + QByteArray::number( abs( m_Denom ) ) + '}';
  return "\\frac{" + QByteArray::number( m_Nom ) + "}{" + QByteArray::number( m_Denom ) + '}';
  }

bool TSimpleFrac::AsFraction()
  {
  return true;
  }

bool TSimpleFrac::SimpleFrac_( int& N, int& D ) const
  {
  N = m_Nom;
  D = m_Denom;
  return true;
  }

bool TSimpleFrac::Negative() const
  {
  return (m_Nom < 0 && m_Denom > 0) || (m_Nom > 0 && m_Denom < 0);
  }

bool TSimpleFrac::Positive() const
  {
  return (m_Nom >= 0 && m_Denom > 0) || (m_Nom < 0 && m_Denom < 0);
  }

bool TSimpleFrac::Unarminus( MathExpr& A ) const
  {
  if( !Negative() )
    return false;
  A = new TSimpleFrac( abs( m_Nom ), abs( m_Denom ) );
  return true;
  }

bool TSimpleFrac::ConstExpr() const
  {
  return true;
  }

int TSimpleFrac::Compare( const MathExpr& ex ) const
  {
  int N, D;
  if( Equal( ex ) )
    return 0;
  else if( IsConstType( TConstant, ex ) )
    return -ex.Compare( Ethis );
  else if( ex.SimpleFrac_( N, D ) )
    if( m_Nom * D > N * m_Denom )
      return 1;
    else
      return -1;
  else
    return 2;
  }

bool TSimpleFrac::IsLinear() const
  {
  return true;
  }

Bracketed TSimpleFrac::MustBracketed() const
  {
  if( m_Nom < 0 || m_Denom < 0 ) return brOperation;
  return brNone;
  }

TUnar::TUnar( const MathExpr& A ): m_Arg( A )
  {
  m_WasReduced = false;
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

QByteArray TInfinity::WriteE() const 
  { 
  return m_Neg ? QByteArray( "-" ).append( msInfinity ) : QByteArray().append( msInfinity ); 
  }

QByteArray TInfinity::SWrite() const 
  { 
  if( m_Neg ) return "-\\infty";
  return "\\infty"; 
  }

MathExpr TUnar::Clone() const
  {
  return -(m_Arg.Clone());
  }

bool TUnar::IsLinear() const
  {
  return m_Arg.IsLinear();
  }

bool TUnar::Cons_int( int& I ) const
  {
  bool Result = m_Arg.Cons_int( I );
  if( Result )
    I = -I;
  return Result;
  }

bool TUnar::ConstExpr() const
  {
  return m_Arg.ConstExpr();
  }

bool TUnar::Multiplication( MathExpr& op1, MathExpr& op2 ) 
  {
  MathExpr exLeft, exRight;
  int Nom, Denom;
  double Val;

  if( m_Coeff.IsEmpty() )
    if( m_Arg.Multp( exLeft, exRight ) )
      {
      if( exLeft.Constan( Val ) )
        m_Coeff = Constant( -Val );
      else if( exLeft.SimpleFrac_( Nom, Denom ) )
        m_Coeff = new TSimpleFrac( -Nom, Denom );
      else
        m_Coeff = -(exLeft);
      m_Term = exRight;
      }
    else
      {
      m_Coeff = Constant( -1 );
      m_Term = m_Arg;
      }
  op1 = m_Coeff;
  op2 = m_Term;
  return true;
  }

MathExpr TUnar::TrigTerm( const QByteArray& Name, const MathExpr& exArg, const MathExpr& exPower )
  {
  MathExpr Result = m_Arg.TrigTerm( Name, exArg, exPower );
  if( !Result.IsEmpty() )
    return -Result;
  return Result;
  }

MathExpr TUnar::Perform() const
  {
  return -(m_Arg.Perform());
  }

MathExpr TUnar::Diff( const QByteArray& d )
  {
  return -(m_Arg.Diff( d ));
  }

MathExpr TUnar::Integral( const QByteArray& d )
  {
  return -(m_Arg.Integral( d ));
  }

MathExpr TUnar::Lim( const QByteArray& v, const MathExpr& lm ) const
  {
  bool ng;
  MathExpr elm = m_Arg.Lim( v, lm );
  if( elm.IsEmpty() )
    return nullptr;
  else if( elm.Infinity( ng ) )
    return new TInfinity( !ng );
  else
    return -(elm);
  }

bool TUnar::Eq( const MathExpr& E2 ) const
  {
  MathExpr Ar2, ex;
  if( E2.Unarminus( Ar2 ) )
    return m_Arg.Eq( Ar2 );
  else if( s_EqualPicture )
    {
    bool Result = false;
    if( ( ( IsConstType( TConstant, m_Arg ) ) || ( IsConstType( TSimpleFrac, m_Arg ) ) || ( IsConstType( TComplexExpr, m_Arg ) ) ) && 
      ( ( IsConstType( TConstant, E2 ) ) || ( IsConstType( TSimpleFrac, E2 ) ) || ( IsConstType( TComplexExpr, E2 ) ) ) )
      {
      s_EqualPicture = false;
      ex = Reduce();
      Result = ex.Eq( E2 );
      s_EqualPicture = true;
      }
    else 
      if( ( IsConstType( TMult, m_Arg ) ) && ( IsConstType( TMult, E2 ) )  )
        {
        s_EqualPicture = false;
        ex = E2.Reduce();
        if( !ex.Eq( E2 ) )
          Result = Eq( ex );
        s_EqualPicture = true;
        }
    return Result;
    }
  else
    return false;
  }

bool TUnar::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2;
  if( E2.Unarminus( Ar2 ) )
    return m_Arg.Equal( Ar2 );

  double Value1, Value2;
  if( m_Arg.Constan( Value1 ) && E2.Constan( Value2 ) )
    return Value1 == -Value2;

  QByteArray _eqName;
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  MathExpr  Ar1, Op21, Op22;
  if( m_Arg.Measur_( Ar1, Ar2 ) && E2.Measur_( Op21, Op22 ) )
    return Ar2.Eq( Op22 ) && Ar1.Constan( Value1 ) && Op21.Constan( Value2 ) &&
    (Value1 == -Value2);

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  if( E2.Summa( Op21, Op22 ) || E2.Subtr( Op21, Op22 ) )
    {
    Ar1 = MathExpr( Ethis ).SummList();
    Ar2 = E2.SummList();
    return Ar1.Equal( Ar2 );
    }

  int N;
  if( E2.Multp( Op21, Op22 ) || (E2.Divis( Op21, Op22 ) && Op21.Cons_int( N ) && (N == 1)) || E2.Variab( _eqName ) )
    {
    Ar1 = MathExpr( Ethis ).FactList();
    Ar2 = E2.FactList();
    return Ar1.Equal( Ar2 );
    }

  return false;
  }

MathExpr TUnar::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return -(m_Arg.Substitute( vr, vl ));
  }

bool TUnar::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  if( m_Arg.Equal( Target ) )
    {
    m_Arg = Source;
#ifdef DEBUG_TASK
    m_Contents = WriteE();
#endif
    return true;
    }

#ifdef DEBUG_TASK
  bool Result = m_Arg.Replace( Target, Source );
  if( Result )
    m_Contents = WriteE();
  return Result;
#else
  return m_Arg.Replace( Target, Source );
#endif
  }

QByteArray TUnar::WriteE() const
  {
  if( !m_Arg.IsEmpty() )
    return "-(" + m_Arg.WriteE() + ")";
  else
    return "(-?)";
  }

QByteArray TUnar::SWrite() const 
  {
  QByteArray Arg = m_Arg.SWrite();
  if( m_Arg.MustBracketed() == brOperation || Arg[0] == '-' )
    return "-(" + Arg + ')';   
  return '-' + Arg;
  }

/*
void TUnar::To_keyl( XPInEdit& Ed )
  {
  MathExpr  op1, op2;
  char ch;
  QByteArray N;
  PExMemb ind;

  Ed.UnRead( "-" );

  if( m_Arg.Summa( op1, op2 ) || m_Arg.Subtr( op1, op2 ) || m_Arg.Binar_( ch, op1, op2 ) || m_Arg.BoolOper_( N, op1, op2 ) || m_Arg.Boolnot_( op1 ) || m_Arg.Listex( ind ) || m_Arg.Listord( ind ) || m_Arg.Negative() )
    {
    Ed.UnRead( "(" );
    m_Arg.To_keyl( Ed );
    Ed.UnRead( ")" );
    }
  else
    m_Arg.To_keyl( Ed );
  }
  */
bool TUnar::Unarminus( MathExpr& A ) const
  {
  A = m_Arg;
  return true;
  }

bool TUnar::Splitted() const
  {
  return m_Arg.Splitted();
  }

bool TUnar::Negative() const
  {
  return true;
  }

bool TUnar::HasComplex() const
  {
  return m_Arg.HasComplex();
  }

bool TUnar::HasMatrix() const
  {
  return m_Arg.HasMatrix();
  }

MathExpr TUnar::Reduce() const
  {
  MathExpr  Op, Op2;
  double Value;
  int I, N, D;
  bool ng;

  MathExpr argument = m_Arg.Reduce();

  if( argument.Complex( Op, Op2 ) )
    return CreateComplex( -Op, -Op2 ).Reduce();

  if( argument.Constan( Value ) && ( abs( Value ) < 0.0000001 ) )
    return Constant( 0 );

  if( argument.Constan( Value ) )
    return Constant( -Value );

  if( argument.Binar( '=', Op, Op2 ) )
    return new TBinar( '=', Expand( -Op ), Expand( -Op2 ) );

  if( argument.Unarminus( Op ) )
    return Op;

  if( argument.Constan( Value ) )
    return Constant( -Value );

  if( argument.SimpleFrac_( N, D ) )
    {
    if( D < 0 )
      D = -D;
    else
      N = -N;
    return new TSimpleFrac( N, D );
    }

  if( argument.MixedFrac_( I, N, D ) )
    return new TMixedFrac( -I, -N, D );

  if( argument.Subtr( Op, Op2 ) )
    return Op2 - Op;

  if( argument.Infinity( ng ) )
    return new TInfinity( !ng );

  if( argument.Unarminus( Op ) )
    return argument;

  if( argument.Power( Op, Op2 ) && argument.Negative() )
    {
    Op = -Op;
    return Op.Reduce() ^ Op2;
    }

  return -argument;
  }

MathExpr TUnapm::Clone() const
  {
  TUnapm *pResult = new TUnapm( m_Arg.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TUnapm::Perform() const
  {
  return new TUnapm( m_Arg.Perform() );
  }

MathExpr TUnapm::Diff( const QByteArray& d )
  {
  return new TUnapm( m_Arg.Diff( d ) );
  }

bool TUnapm::Eq( const MathExpr& E2 ) const
  {
  MathExpr  Ar2;
  return E2.Unapm_( Ar2 ) && m_Arg.Eq( Ar2 );
  }

bool TUnapm::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar1, Ar2, Op21, Op22;
  int N;
  QByteArray _eqName;

  if( E2.Unapm_( Ar2 ) )
    return m_Arg.Equal( Ar2 );

  if( E2.Divis( Op21, Op22 ) )
    return E2.Equal( Ethis );

  if( E2.Multp( Op21, Op22 ) || ( E2.Divis( Op21, Op22 ) && Op21.Cons_int( N ) && N == 1 ) )
    {
    Ar1 = FactList();
    Ar2 = E2.FactList();
    return Ar1.Equal( Ar2 );
    }

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );
  return false;
  }

MathExpr TUnapm::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TUnapm( m_Arg.Substitute( vr, vl ) );
  }

QByteArray TUnapm::WriteE() const
  {
  QByteArray Result;
  Result.append( msPlusMinus );
  if( !m_Arg.IsEmpty() )
    return Result + m_Arg.WriteE();
  return '(' + Result + "?)";
  }

QByteArray TUnapm::SWrite() const
  {
  QByteArray Result( charToTex( msPlusMinus) );
  if( !m_Arg.IsEmpty() ) Result += m_Arg.SWrite();
  return Result;
  }
/*
void TUnapm::To_keyl( XPInEdit& Ed )
  {
  MathExpr  op1, op2;
  char ch;
  QByteArray N;
  PExMemb ind;

  Ed.UnRead( msPlusMinus );

  if( m_Arg.Summa( op1, op2 ) || m_Arg.Subtr( op1, op2 ) || m_Arg.Binar_( ch, op1, op2 ) || m_Arg.BoolOper_( N, op1, op2 ) || m_Arg.Boolnot_( op1 ) ||
    m_Arg.Listex( ind ) || m_Arg.Listord( ind ) || m_Arg.Negative() )
    {
    Ed.UnRead( "(" );
    m_Arg.To_keyl( Ed );
    Ed.UnRead( ")" );
    }
  else
    m_Arg.To_keyl( Ed );
  }
  */
bool TUnapm::Unapm_( MathExpr& A ) const
  {
  A = m_Arg;
  return true;
  }

TIndx::TIndx( const MathExpr& Ex1, const MathExpr& Ex2 ): TOper( Ex1, Ex2 )
  {
  m_Name = '[';
  }

MathExpr TIndx::Clone() const
  {
  TIndx* bResult = new TIndx( m_Operand1.Clone(), m_Operand2.Clone() );
  bResult->m_WasReduced = m_WasReduced;
  return bResult;
  }

MathExpr TIndx::Reduce() const
  {
  return new TIndx( m_Operand1.Reduce(), m_Operand2.Reduce() );
  }

MathExpr TIndx::Perform() const
  {
  return new TIndx( m_Operand1.Perform(), m_Operand2.Perform() );
  }

bool TIndx::Eq( const MathExpr& E2 ) const
  {
  MathExpr  Op21, Op22;
  if( E2.Indx_( Op21, Op22 ) )
    return m_Operand1.Eq( Op21 ) && m_Operand2.Eq( Op22 );
  return false;
  }

bool TIndx::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, Op21, Op22;
  QByteArray _eqName;

  if( E2.Indx_( Op21, Op22 ) )
    return m_Operand1.Equal( Op21 ) && m_Operand2.Equal( Op22 );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && ( _eqName == "Simplify" ) )
    return Equal( Ar2 );
  return false;
  }

QByteArray TIndx::WriteE() const
  {
  QByteArray  S;

  if( !m_Operand1.IsEmpty() )
    S = '(' + m_Operand1.WriteE() + ')';
  else
    S = "(???)";

  if( !m_Operand2.IsEmpty() )
    S += '[' + m_Operand2.WriteE() + ']';
  else
    S += "[???]";
  return S;
  }

QByteArray TIndx::SWrite() const
  {
  return "\\index{" + m_Operand1->SWrite() + "}{" + m_Operand2->SWrite() + '}';
  }

bool TIndx::Indx_( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

MathExpr TIndx::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  if( WriteE() == vr )
    return vl.Clone();
  return Clone();
  }

MathExpr TIntegral::Clone() const
  {
  TIntegral* pResult = new TIntegral( m_Meta_sign, m_Expint->Clone(), m_Varint->Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TIntegral::Reduce() const
  {
  if( !s_IsIntegral )
    s_IntegralCount = 0;

  MathExpr Result = CalcIntegralExpr();
  if( Result == ( TExpr* ) this )
    return Result;

  MathExpr ex1, ex2;
  if( Result.Summa( ex1, ex2 ) )
    return ex1;

  if( Result.Subtr( ex1, ex2 ) )
    return -ex2;

  return Result;
  }

MathExpr TIntegral::Perform() const
  {
  return new TIntegral( m_Meta_sign, m_Expint.Perform(), CastPtr( TVariable, m_Varint.Perform() ) );
  }

bool TIntegral::Eq( const MathExpr& E2 )  const
  {
  MathExpr  ex, exll, exhl, vr;
  if( E2.Integr_( ex, vr ) && !(E2.Dfintegr_( ex, exll, exhl, vr )) )
    return m_Expint.Eq( ex ) && m_Varint.Eq( vr );
  else
    return false;
  }

bool TIntegral::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, ex, exll, exhl, vr;
  QByteArray _eqName;

  if( E2.Integr_( ex, vr ) && !E2.Dfintegr_( ex, exll, exhl, vr ) )
    return (m_Expint.Equal( ex ) && m_Varint.Equal( vr ));

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  return false;
  }


MathExpr TIntegral::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TIntegral( m_Meta_sign, m_Expint.Substitute( vr, vl ), m_Varint );
  }

bool TIntegral::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  if( m_Expint.Equal( Target ) )
    {
    m_Expint = Source;
#ifdef DEBUG_TASK
    m_Contents = WriteE();
#endif
    return true;
    }

  if( m_Expint.Replace( Target, Source ) )
    {
#ifdef DEBUG_TASK
    m_Contents = WriteE();
#endif
    return true;
    }
  return false;
  }

QByteArray TIntegral::WriteE() const
  {
  return QByteArray( IntegrName ) + "(" + m_Expint.WriteE() + "," + m_Varint.WriteE() + ")";
  }

QByteArray TIntegral::SWrite() const
  {
  QByteArray Result = "\\int{";
  MathExpr op1, op2;
  uchar ch;
  QByteArray N;
  PExMemb ind;
  if( m_Expint.Summa( op1, op2 ) || m_Expint.Subtr( op1, op2 ) || m_Expint.Multp( op1, op2 ) || 
    ( m_Expint->Divis( op1, op2 ) && !m_Expint.AsFraction() ) || m_Expint.Binar_( ch, op1, op2 ) || 
    m_Expint.BoolOper_( N, op1, op2 ) || m_Expint.Boolnot_( op1 ) || m_Expint.Negative() || 
    m_Expint.Listex( ind ) || m_Expint.Listord( ind ) )
    return Result + '(' + m_Expint.SWrite() + ")}{" + m_Varint.SWrite() + '}';
  return Result + m_Expint.SWrite() + "}{" + m_Varint.SWrite() + '}';
  }

TDefIntegral::TDefIntegral( bool M, const MathExpr& exi, const MathExpr& exll, const MathExpr& exhl, const MathExpr& Var )
  : TIntegral(), m_Lolimit(exll), m_Hilimit(exhl)
  {
  m_Expint = exi;
  m_Varint = Var;
  m_Meta_sign = M;
  }

MathExpr TDefIntegral::Clone() const
  {
  TDefIntegral* pResult = new TDefIntegral( m_Meta_sign, m_Expint, m_Lolimit, m_Hilimit, CastPtr( TVariable, m_Varint.Clone() ) );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TDefIntegral::Perform() const
  {
  return new TDefIntegral( m_Meta_sign, m_Expint.Perform(), m_Lolimit.Perform(), m_Hilimit.Perform(), CastPtr( TVariable, m_Varint.Perform() ) );
  }

MathExpr TDefIntegral::Reduce() const
  {
  return new TDefIntegral( m_Meta_sign, m_Expint.Reduce(), m_Lolimit.Reduce(), m_Hilimit.Reduce(), CastPtr( TVariable, m_Varint.Reduce() ) );
  }

bool TDefIntegral::Eq( const MathExpr& E2 ) const
  {
  MathExpr  ex, exll, exhl, vr;
  if( E2.Dfintegr_( ex, exll, exhl, vr ) )
    return m_Expint.Eq( ex ) && m_Lolimit.Eq( exll ) && m_Hilimit.Eq( exhl ) && m_Varint.Eq( vr );

  return false;
  }

bool TDefIntegral::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, ex, exll, exhl, vr;
  QByteArray _eqName;

  if( E2.Dfintegr_( ex, exll, exhl, vr ) )
    return m_Expint.Equal( ex ) && m_Lolimit.Equal( exll ) && m_Hilimit.Equal( exhl ) && m_Varint.Equal( vr );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  return false;
  }

MathExpr TDefIntegral::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TDefIntegral( m_Meta_sign, m_Expint.Substitute( vr, vl ), m_Lolimit.Substitute( vr, vl ), 
    m_Hilimit.Substitute( vr, vl ), m_Varint );
  }

QByteArray TDefIntegral::WriteE() const
  {
  return QByteArray( IntegrName ) + '(' + m_Expint.WriteE() + ',' + m_Lolimit.WriteE() + ',' + m_Hilimit.WriteE() + ',' +
    m_Varint.WriteE() + ')';
  }

bool TDefIntegral::Dfintegr_( MathExpr& exi, MathExpr& exll, MathExpr& exhl, MathExpr& vr ) const
  {
  exi = m_Expint;
  exll = m_Lolimit;
  exhl = m_Hilimit;
  vr = m_Varint;
  return true;
  }

QByteArray TDefIntegral::SWrite() const
  {
  QByteArray Result = "\\dint{" + m_Lolimit.SWrite() + "}{" + m_Hilimit.SWrite() + "}{";
//  if( CastConst( TDefIntegral, m_Expint ) )
//    return Result + "\\invisible\n}{" + m_Varint.SWrite() + "\\setunvisible\n*" + m_Expint.SWrite() + '}';
  MathExpr op1, op2;
  uchar ch;
  int Val;
  QByteArray N;
  PExMemb ind;
  if( m_Expint.Summa( op1, op2 ) || m_Expint.Subtr( op1, op2 ) || m_Expint.Multp( op1, op2 ) ||
    ( m_Expint.Divis( op1, op2 ) && !m_Expint.AsFraction() ) || m_Expint.Binar_( ch, op1, op2 ) ||
    m_Expint.BoolOper_( N, op1, op2 ) || m_Expint.Boolnot_( op1 ) || m_Expint.Negative() || m_Expint.Listex( ind ) || m_Expint.Listord( ind ) )
    Result += '(' + m_Expint.SWrite() + ")}{";
  else
    if( m_Expint.Cons_int( Val ) && Val == 1 )
      Result += "\\setunvisible\n1\\setvisible\n}{";
    else
      Result += m_Expint.SWrite() + "}{";
  if( m_Varint.Multp( op1, op2 ) && IsConstType( TDefIntegral, op2 ) )
    return Result + op1.SWrite() + "\\setunvisible\n*\\setvisible\n" + op2.SWrite() + '}';
  return Result + m_Varint.SWrite() + '}';
  }

TMultIntegral::TMultIntegral( bool M, const MathExpr& exi, const MathExpr& exll, const MathExpr& vr, const MathExpr& vr2,
  const MathExpr& vr3 ) : TIntegral( M, exi, MathExpr() ), m_Region( exll )
  {
  TLexp *pLexp = new TLexp();
  pLexp->Addexp( vr );
  pLexp->Addexp( vr2 );
  if( !vr3.IsEmpty() ) pLexp->Addexp( vr3 );
  m_Varint = pLexp;
  }

TMultIntegral::TMultIntegral( bool Meta, const MathExpr& exi, const MathExpr& exll, const MathExpr& vr ) :
  TIntegral( Meta, exi, vr ), m_Region( exll ) {}
  
MathExpr TMultIntegral::Clone() const
  {
  TMultIntegral *pResult = new TMultIntegral( m_Meta_sign, m_Expint.Clone(), m_Region.Clone(), m_Varint.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TMultIntegral::Reduce() const
  {
  return new TMultIntegral( m_Meta_sign, m_Expint.Reduce(), m_Region.Reduce(), m_Varint.Reduce() );
  }

MathExpr TMultIntegral::Perform() const
  {
  return new TMultIntegral( m_Meta_sign, m_Expint.Perform(), m_Region.Perform(), m_Varint.Perform() );
  }

bool TMultIntegral::Eq( const MathExpr& E2 ) const
  {
  const TMultIntegral *pIntegral = CastConst( TMultIntegral, E2 );
  if( pIntegral == nullptr ) return false;
  return m_Expint.Eq( pIntegral->m_Expint ) && m_Region.Eq( pIntegral->m_Region ) && m_Varint.Eq( pIntegral->m_Varint );
  }

bool TMultIntegral::Equal( const MathExpr& E2 ) const
  {
  const TMultIntegral *pIntegral = CastConst( TMultIntegral, E2 );
  if( pIntegral == nullptr ) return false;
  return m_Expint.Equal( pIntegral->m_Expint ) && m_Region.Equal( pIntegral->m_Region ) && m_Varint.Equal( pIntegral->m_Varint );
  }

MathExpr TMultIntegral::Substitute( const QByteArray &vr, const MathExpr& vl )
  {
  return new TMultIntegral( m_Meta_sign, m_Expint.Substitute( vr, vl ), m_Region.Substitute( vr, vl ), m_Varint.Substitute( vr, vl ) );
  }

QByteArray TMultIntegral::WriteE() const
  {
  const TLexp *pLexp = CastConst( TLexp, m_Varint );
  if( pLexp->m_Count == 2 )
    return QByteArray( DoubleIntegrName ) + '(' + m_Expint.WriteE() + ',' + m_Region.WriteE() + ',' +
    pLexp->m_pFirst->m_Memb.WriteE() + ',' + pLexp->m_pFirst->m_pNext->m_Memb.WriteE() + ')';
  return QByteArray( TripleIntegrName ) + '(' + m_Expint.WriteE() + ',' + m_Region.WriteE() + ',' +
    pLexp->m_pFirst->m_Memb.WriteE() + ',' + pLexp->m_pFirst->m_pNext->m_Memb.WriteE() + pLexp->m_pFirst->m_pNext->m_pNext->m_Memb.WriteE() + ')';
  }

QByteArray TMultIntegral::SWrite() const
  {
  QByteArray ExpInt;
  MathExpr op1, op2;
  uchar ch;
  QByteArray N;
  PExMemb ind;
  if( m_Expint.Summa( op1, op2 ) || m_Expint.Subtr( op1, op2 ) || m_Expint.Multp( op1, op2 ) ||
    ( m_Expint.Divis( op1, op2 ) && !m_Expint.AsFraction() ) || m_Expint.Binar_( ch, op1, op2 ) ||
    m_Expint.BoolOper_( N, op1, op2 ) || m_Expint.Boolnot_( op1 ) || m_Expint.Negative() || m_Expint.Listex( ind ) || m_Expint.Listord( ind ) )
    ExpInt = '(' + m_Expint.SWrite() + ")}{";
  else
    ExpInt = m_Expint.SWrite() + "}{";
  const TLexp *pLexp = CastConst( TLexp, m_Varint );
  if( pLexp->m_Count == 2 )
    return "\\dblint{" + m_Region.SWrite() + "}{" + ExpInt + pLexp->m_pFirst->m_Memb.SWrite() + "}{" +
    pLexp->m_pFirst->m_pNext->m_Memb.SWrite() + '}';
  return "\\trplint{" + m_Region.SWrite() + "}{" + ExpInt + pLexp->m_pFirst->m_Memb.SWrite() + "}{" +
    pLexp->m_pFirst->m_pNext->m_Memb.SWrite() + "}{" + pLexp->m_pFirst->m_pNext->m_pNext->m_Memb.SWrite() + '}';
  }

bool TMultIntegral::Multintegr_( MathExpr& exi, MathExpr& exll, MathExpr& vr ) const
  {
  exi = m_Expint;
  exll = m_Region;
  vr = m_Varint;
  return true;
  }

TCurveIntegral::TCurveIntegral( bool Meta, uchar Type, const MathExpr& exi, const MathExpr& exll ) : TIntegral( Meta, exi, exll ), 
  m_Type( Type ) {}

MathExpr TCurveIntegral::Clone() const
  {
  TCurveIntegral *pResult = new TCurveIntegral( m_Meta_sign, m_Type, m_Expint.Clone(), m_Varint.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TCurveIntegral::Reduce() const
  {
  return new TCurveIntegral( m_Meta_sign, m_Type, m_Expint.Reduce(), m_Varint.Reduce() );
  }

MathExpr TCurveIntegral::Perform() const
  {
  return new TCurveIntegral( m_Meta_sign, m_Type, m_Expint.Perform(), m_Varint.Perform() );
  }

bool TCurveIntegral::Eq( const MathExpr& E2 ) const
  {
  const TCurveIntegral *pIntegral = CastConst( TCurveIntegral, E2 );
  if( pIntegral == nullptr ) return false;
  return pIntegral->m_Type == m_Type && m_Expint.Eq( pIntegral->m_Expint ) && m_Varint.Eq( pIntegral->m_Varint );
  }

bool TCurveIntegral::Equal( const MathExpr& E2 ) const
  {
  const TCurveIntegral *pIntegral = CastConst( TCurveIntegral, E2 );
  if( pIntegral == nullptr ) return false;
  return pIntegral->m_Type == m_Type && m_Expint.Equal( pIntegral->m_Expint ) && m_Varint.Equal( pIntegral->m_Varint );
  }

MathExpr TCurveIntegral::Substitute( const QByteArray &vr, const MathExpr& vl )
  {
  return new TCurveIntegral( m_Meta_sign, m_Type, m_Expint.Substitute( vr, vl ), m_Varint.Substitute( vr, vl ) );
  }

QByteArray TCurveIntegral::WriteE() const
  {
  switch( m_Type )
    {
    case msIntegral:
      return QByteArray( CurveIntegrName ) + '(' + m_Expint.WriteE() + ',' + m_Varint.WriteE() + ')';
    case msDoubleIntegral:
      return QByteArray( SurfaceIntegrName ) + '(' + m_Expint.WriteE() + ',' + m_Varint.WriteE() + ')';
    }
  return QByteArray( ContourIntegrName ) + '(' + m_Expint.WriteE() + ',' + m_Varint.WriteE() + ')';
  }

QByteArray TCurveIntegral::SWrite() const
  {
  return "\\crvint{" + SimpleChar( m_Type ) + "}{" + m_Varint->SWrite() + "}{" + m_Expint->SWrite() + '}';
  }

bool TCurveIntegral::Curveintegr_( MathExpr& exi, MathExpr& exll, uchar& Type ) const
  {
  exi = m_Expint;
  exll = m_Varint;
  Type = m_Type;
  return true;
  }

TGSumm::TGSumm( bool M, const MathExpr& exs, const MathExpr& exll, const MathExpr& exhl ) : TExpr(), m_Expsum( exs ),
  m_Lolimit( exll ), m_Hilimit( exhl ), m_Meta_sign( M ) {}

MathExpr TGSumm::Clone() const
  {
  TGSumm *pResult = new TGSumm( m_Meta_sign, m_Expsum.Clone(), m_Lolimit.Clone(), m_Hilimit.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TGSumm::Perform() const
  {
  return new TGSumm( m_Meta_sign, m_Expsum.Perform(), m_Lolimit.Perform(), m_Hilimit.Perform() );
  }

bool TGSumm::Eq( const MathExpr& E2 ) const
  {
  MathExpr  exs, exll, exhl;
  if( E2.Gsumma( exs, exll, exhl ) )
    return m_Expsum.Eq( exs ) && m_Lolimit.Eq( exll ) && m_Hilimit.Eq( exhl );
  return false;
  }

bool TGSumm::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, exs, exll, exhl;
  QByteArray _eqName;
  if( E2.Gsumma( exs, exll, exhl ) )
    return m_Expsum.Equal( exs ) && m_Lolimit.Equal( exll ) && m_Hilimit.Equal( exhl );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );
  return false;
  }

MathExpr TGSumm::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TGSumm( m_Meta_sign, m_Expsum.Substitute( vr, vl ), m_Lolimit.Substitute( vr, vl ), m_Hilimit.Substitute( vr, vl ) );
  }

QByteArray TGSumm::WriteE() const
  {
  return QByteArray( GSummName ) + '(' + m_Expsum.WriteE() + ',' + m_Lolimit.WriteE() + ',' + m_Hilimit.WriteE() + ')';
  }

QByteArray TGSumm::SWrite() const
  {
  QByteArray Result = "\\summa{" + m_Lolimit->SWrite() + "}{" + m_Hilimit->SWrite() + "}{";
  MathExpr op1, op2;
  uchar ch;
  QByteArray N;
  PExMemb ind;
  if( m_Expsum.Summa( op1, op2 ) || m_Expsum.Subtr( op1, op2 ) || m_Expsum.Multp( op1, op2 ) ||
    ( m_Expsum.Divis( op1, op2 ) && !m_Expsum.AsFraction() ) || m_Expsum.Binar_( ch, op1, op2 ) ||
    m_Expsum.BoolOper_( N, op1, op2 ) || m_Expsum.Boolnot_( op1 ) || m_Expsum.Negative() || m_Expsum.Listex( ind ) || m_Expsum.Listord( ind ) )
    return Result + '(' + m_Expsum.SWrite() + ")}";
  return Result + m_Expsum.SWrite() + '}';
  }

bool TGSumm::Splitted() const
  {
  return m_Expsum.Splitted();
  }

bool TGSumm::Gsumma( MathExpr& exs, MathExpr& exll, MathExpr& exhl ) const
  {
  exs = m_Expsum;
  exll = m_Lolimit;
  exhl = m_Hilimit;
  return true;
  }


MathExpr TGSumm::Reduce() const
  {
  return new TGSumm( m_Meta_sign, m_Expsum.Reduce(), m_Lolimit.Reduce(), m_Hilimit.Reduce() );
  }

MathExpr TLimit::Clone() const
  {
  TLimit *pResult = new TLimit( m_Meta_sign, m_Exp.Clone(), m_Varlimit.Clone(), m_Explimit.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }
  
MathExpr TLimit::Reduce() const
  {
  return new TLimit( m_Meta_sign, m_Exp.Reduce(), m_Varlimit.Reduce(), m_Explimit.Reduce() );
  }

MathExpr TLimit::Perform() const
  {
  return new TLimit( m_Meta_sign, m_Exp.Perform(), m_Varlimit.Perform(), m_Explimit.Perform() );
  }

bool TLimit::Eq( const MathExpr& E2 ) const
  {
  MathExpr  ex, exv, exl;
  if( E2.Limit( ex, exv, exl ) )
    return m_Exp.Eq( ex ) && m_Varlimit.Eq( exv ) && m_Explimit.Eq( exl );
  else
    return false;
  }

bool TLimit::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, ex, exv, exl;
  QByteArray _eqName;

  if( E2.Limit( ex, exv, exl ) )
    return m_Exp.Equal( ex ) && m_Varlimit.Equal( exv ) && m_Explimit.Equal( exl );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  return false;
  }

MathExpr TLimit::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TLimit( m_Meta_sign, m_Exp.Substitute( vr, vl ), m_Varlimit.Substitute( vr, vl ), m_Explimit.Substitute( vr, vl ) );
  }

QByteArray TLimit::WriteE() const
  {
  return QByteArray(LimitName) + '(' + m_Exp.WriteE() + ',' + m_Varlimit.WriteE() + ',' + m_Explimit.WriteE() + ')';
  }

QByteArray TLimit::SWrite() const
  {
  QByteArray Result = "\\lim{" + m_Varlimit.SWrite() + "}{" + m_Explimit.SWrite() + "}{";
  MathExpr op1, op2;
  uchar ch;
  PExMemb ind;
  QByteArray N;
  if( m_Exp.Summa( op1, op2 ) || m_Exp.Subtr( op1, op2 ) || m_Exp.Multp( op1, op2 ) || ( m_Exp.Divis( op1, op2 ) && !m_Exp.AsFraction() ) || 
    m_Exp.Binar_( ch, op1, op2 ) || m_Exp.BoolOper_( N, op1, op2 ) || m_Exp.Boolnot_( op1 ) || m_Exp.Negative() || 
    m_Exp.Listex( ind ) || m_Exp.Listord( ind ) )
    return  Result + '(' + m_Exp.SWrite() + ")}";
  return Result + m_Exp.SWrite() + '}';
  }

bool TLimit::Limit( MathExpr& ex, MathExpr& exv, MathExpr& exl ) const
  {
  ex = m_Exp;
  exv = m_Varlimit;
  exl = m_Explimit;
  return true;
  }

TLog::TLog( const MathExpr& ex1, const MathExpr& ex2 ): TExpr(), m_Basis( ex1 ), m_Arg( ex2 )
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TLog::Clone() const
  {
  TLog *pResult = new TLog( m_Basis.Clone(), m_Arg.Clone());
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TLog::Reduce() const
  {
  double r, i;
  MathExpr arg1, basis1, op1, op2;
  bool powr = false;

  auto GetResult = [&] ()
    {
    i = round( r );
    if( abs( r - i ) < 0.0000001 )
      r = i;
    else
      if( s_NoLogReduce )
        return basis1.Log( arg1 );
    return Constant( r );
    };

  auto END = [&] ( MathExpr Result)
    {
    if( powr )
      return ( op2 * Result ).Reduce();
    return Result;
    };
  
  arg1 = m_Arg.Reduce();
  basis1 = m_Basis.Reduce();

  if( basis1.Constan( r ) && ( r <= 0 || r == 1 ) )
    {
    s_LastError = X_Str( "MBasisLog", "basis>0 and basis <> 1 " );
    //throw  ErrParser( s_LastError.toLocal8Bit(), peNewErr );
    s_GlobalInvalid=1;
    return Ethis;
  }

  if( arg1.Power( op1, op2 ) && !s_IsLogEquation && !s_NoLogSquarReduce )
    {
    arg1 = op1;
    powr = true;
    }

  if( arg1.Equal( basis1 ) )
    return END( Constant( 1 ) );

  if( ( arg1 * basis1 ).Reduce().Constan( r ) && r == 1 )
    return END( Constant( -1 ) );

  double a, b;
  bool c1 = basis1.Constan( b );
  bool c2 = arg1.Constan( a );
  if( c1 && c2 )
    {
    if( a < 0.0000001 || b < 0.0000001 || abs( b - 1 ) < 0.0000001 )
      {
      s_GlobalInvalid=1;
      return Ethis;
      }
    r = log( a ) / log( b );
    return END( GetResult() );
    }

  int nom1, den1, nom2, den2;
  bool f1 = basis1.SimpleFrac_( nom1, den1 );
  bool f2 = arg1.SimpleFrac_( nom2, den2 );
  if( c1 && f2 )
    {
    if( ( abs( b - 1 ) < 0.0000001 ) )
      {
      s_LastError = X_Str( "MArgLgLess0", "Argument <= 0!" );
      s_GlobalInvalid=1;
      return Ethis;
      //throw ErrParser( "No Solutions!", peNoSolv );
      }
    if(s_GlobalInvalid)return Ethis;
    r = ( log( nom2 ) - log( den2 ) ) / log( b );
    return END( GetResult() );
    }

  if( c2 && f1 )
    {
    if( ( abs( nom1 - den1 ) < 0.0000001 ) )
      {
      //s_LastError = X_Str( "MArgLgLess0", "Argument <= 0!" );
      //throw  ErrParser( "No Solutions!", peNoSolv );
        s_LastError = "INFVAL";
        s_GlobalInvalid=1;
        return Ethis;
      }
    if(s_GlobalInvalid)return Ethis;
    r = log( a ) / ( log( nom1 ) - log( den1 ) );
    return END( GetResult() );
    }

  if( f1 && f2 )
    {
    if( ( abs( nom1 - den1 ) < 0.0000001 ) )
      {
      //s_LastError = X_Str( "MArgLgLess0", "Argument <= 0!" );
      //throw  ErrParser( "No Solutions!", peNoSolv );
      s_LastError = "INFVAL";
      s_GlobalInvalid=1;
      return Ethis;
      }
    if(s_GlobalInvalid)return Ethis;
    r = ( log( nom2 ) - log( den2 ) ) / ( log( nom1 ) - log( den1 ) );
    return END( GetResult() );
    }

  return END( basis1.Log( arg1 ) );
  }

MathExpr TLog::Perform() const
  {
  return m_Basis.Perform().Log( m_Arg.Perform() );
  }

bool TLog::Eq( const MathExpr& E2 ) const
  {
  MathExpr ex1, ex2;
  QByteArray Name;
  int iVal;
  if( E2.Log( ex1, ex2 ) )
    return m_Basis.Eq( ex1 ) && m_Arg.Eq( ex2 );
  else
    return E2.Funct( Name, ex2 ) && (Name == "lg") && m_Basis.Cons_int( iVal ) && (iVal == 10) && m_Arg.Eq( ex2 );
  }

bool TLog::Equal( const MathExpr& E2 ) const
  {
  MathExpr ex1, ex2;
  if( E2.Log( ex1, ex2 ) )
    return m_Basis.Equal( ex1 ) && m_Arg.Equal( ex2 );
  return false;
  }

bool TLog::Log( MathExpr& ex1, MathExpr& ex2 ) const
  {
  ex1 = m_Basis;
  ex2 = m_Arg;
  return true;
  }

MathExpr TLog::Diff( const QByteArray& d )
  {
  MathExpr  ex, arg1, basis1;
  double r;
  arg1 = m_Arg.Diff( d );
  basis1 = m_Basis.Diff( d );
  if( basis1.Constan( r ) && abs( r ) < 0.0000001 )
    ex = arg1 / ( Function( "log", m_Basis ) * m_Arg );
  else
    if( arg1.Constan( r ) && abs( r ) < 0.0000001 )
      ex = Function( "log", m_Arg ) * m_Basis.Diff( d ) / ( ( Function( "log", m_Basis ) ^ 2 ) * m_Basis );
    else
      ex = m_Arg.Diff( d ) / m_Arg * Function( "log", m_Basis ) -
        m_Basis.Diff(d) / m_Basis * Function( "log", m_Arg ) / ( Function( "log", m_Basis ) ^ 2 );
  return ex.Reduce();
  }

MathExpr TLog::Integral( const QByteArray& d )
  {
  return ( Function( "log", m_Arg ) / Function( "log", m_Basis ) ).Integral( d );
  }

MathExpr TLog::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return m_Basis.Substitute( vr, vl ).Log( m_Arg.Substitute( vr, vl ) );
  }

bool TLog::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  bool Result = false;
  if( m_Arg.Equal( Target ) )
    {
    m_Arg = Source;
    Result = true;
    }
  else
    Result = m_Arg.Replace( Target, Source );
  if( m_Basis.Equal( Target ) )
    {
    m_Basis = Source;
    Result = true;
    }
  else
    Result = m_Basis.Replace( Target, Source ) || Result;
#ifdef DEBUG_TASK
  if( Result ) m_Contents = WriteE();
#endif
  return Result;
  }

QByteArray TLog::WriteE() const
  {
  return "log(" + m_Basis.WriteE() + ',' + m_Arg.WriteE() + ')';
  }

QByteArray TLog::SWrite() const
  {
  return "\\log{" + m_Basis.SWrite() + "}{" + m_Arg.SWrite() + '}';
  }

TGMult::TGMult( bool Meta, const MathExpr& exm, const MathExpr& exll, const MathExpr& exhl )
  : TExpr(), m_Expmul( exm ), m_Lolimit( exll ), m_Hilimit( exll ), m_Meta_sign( Meta )
  {}

MathExpr TGMult::Clone() const
  {
  TGMult *pResult = new TGMult( m_Meta_sign, m_Expmul.Clone(), m_Lolimit.Clone(), m_Hilimit.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TGMult::Reduce() const
  {
  return new TGMult( m_Meta_sign, m_Expmul.Reduce(), m_Lolimit.Reduce(), m_Hilimit.Reduce() );
  }

MathExpr TGMult::Perform() const
  {
  return new TGMult( m_Meta_sign, m_Expmul.Perform(), m_Lolimit.Perform(), m_Hilimit.Perform() );
  }

bool TGMult::Eq( const MathExpr& E2 ) const
  {
  MathExpr  exm, exll, exhl;
  if( E2.Gmult( exm, exll, exhl ) )
    return m_Expmul.Eq( exm ) && m_Lolimit.Eq( exll ) && m_Hilimit.Eq( exhl );
  else
    return false;
  }

bool TGMult::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, exm, exll, exhl;
  QByteArray _eqName;

  if( E2.Gmult( exm, exll, exhl ) )
    return m_Expmul.Equal( exm ) && m_Lolimit.Equal( exll ) && m_Hilimit.Equal( exhl );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  return false;
  }

TSubst::TSubst( bool Meta, const MathExpr& ex, const MathExpr& exll, const MathExpr& exhl )
  : TExpr(), m_Exp( ex ), m_Lolimit( exll ), m_Hilimit( exhl ), m_Meta_sign( Meta )
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TSubst::Clone() const
  {
  TSubst *pResult = new TSubst( m_Meta_sign, m_Exp.Clone(), m_Lolimit.Clone(), m_Hilimit.Clone()  );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TSubst::Reduce() const
  {
  return new TSubst( m_Meta_sign, m_Exp.Reduce(), m_Lolimit.Reduce(), m_Hilimit.Reduce() );
  }

MathExpr TSubst::Perform() const
  {
  return new TSubst( m_Meta_sign, m_Exp.Perform(), m_Lolimit.Perform(), m_Hilimit.Perform() );
  }

bool TSubst::Eq( const MathExpr& E2 ) const
  {
  MathExpr  ex, exll, exhl;
  if( E2.Subst( ex, exll, exhl ) )
    return m_Exp.Eq( ex ) && m_Lolimit.Eq( exll ) && m_Hilimit.Eq( exhl );
  else
    return false;
  }

bool TSubst::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, ex, exll, exhl;
  QByteArray _eqName;

  if( E2.Subst( ex, exll, exhl ) )
    return m_Exp.Equal( ex ) && m_Lolimit.Equal( exll ) && m_Hilimit.Equal( exhl );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && (_eqName == "Simplify") )
    return Equal( Ar2 );

  return false;
  }

MathExpr TSubst::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TSubst( m_Meta_sign, m_Exp.Substitute( vr, vl ), m_Lolimit.Substitute( vr, vl ), m_Hilimit.Substitute( vr, vl ) );
  }

QByteArray TSubst::WriteE() const
  {
  return QByteArray( SubstName ) + '(' + m_Exp.WriteE() + ',' + m_Lolimit.WriteE() + ',' + m_Hilimit.WriteE() + ')';
  }

bool TSubst::Splitted() const 
  {
  return m_Exp.Splitted();
  }

bool TSubst::Subst( MathExpr& ex, MathExpr& exll, MathExpr& exhl ) const
  {
  ex = m_Exp;
  exll = m_Lolimit;
  exhl = m_Hilimit;
  return true;
  }

QByteArray TSubst::SWrite() const
  {
  QByteArray  Result = "\\subst{";
  MathExpr op1, op2;
  uchar ch;
  QByteArray N;
  PExMemb ind;
  if( m_Exp.Summa( op1, op2 ) || m_Exp.Subtr( op1, op2 ) || m_Exp.Multp( op1, op2 ) ||
    ( m_Exp.Divis( op1, op2 ) && !m_Exp.AsFraction() ) || m_Exp.Binar_( ch, op1, op2 ) ||
    m_Exp.BoolOper_( N, op1, op2 ) || m_Exp.Boolnot_( op1 ) || m_Exp.Negative() || m_Exp.Listex( ind ) || m_Exp.Listord( ind ) )
    Result += '(' + m_Exp.SWrite() + ")}{";
  else
    Result += m_Exp.SWrite() + "}{";
  return Result + m_Lolimit->SWrite() + "}{" + m_Hilimit->SWrite() + '}';
  }

TDeriv::TDeriv( const MathExpr& Stroke, const MathExpr& Part, const MathExpr& ex, const MathExpr& vr ) : TExpr(),
  m_Expdif( ex ), m_Vardif( vr ), m_DerivAsStroke( Stroke ), m_Partial( Part )
  {
  m_HasParenthesis = !(IsType(TVariable, m_Expdif ));
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

TDeriv::TDeriv( MathExpr& Parms) : TExpr()
  {
  TLexp *pLexp = CastPtr( TLexp, Parms );
  if( pLexp->m_Count < 4 ) throw ErrParser( "Syntax error!", peSyntacs );
  m_DerivAsStroke = pLexp->m_pFirst->m_Memb;
  pLexp->DeleteMemb( pLexp->m_pFirst );
  if( !( IsType( TBool, m_DerivAsStroke ) ) || pLexp->m_pFirst.isNull() ) throw ErrParser( "Syntax error!", peSyntacs );
  m_Partial = pLexp->m_pFirst->m_Memb;
  pLexp->DeleteMemb( pLexp->m_pFirst );
  if( !( IsType( TBool, m_Partial ) ) || pLexp->m_pFirst.isNull() ) throw ErrParser( "Syntax error!", peSyntacs );
  m_Expdif = pLexp->m_pFirst->m_Memb;
  m_HasParenthesis = !(IsType(TVariable, m_Expdif ));
  pLexp->DeleteMemb( pLexp->m_pFirst );
  if( pLexp->m_pFirst.isNull() ) throw ErrParser( "Syntax error!", peSyntacs );
  for( PExMemb Memb = pLexp->m_pFirst; !Memb.isNull(); Memb = Memb->m_pNext )
    if( !( IsType( TVariable, Memb->m_Memb ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
  if( pLexp->m_Count == 1 )
    m_Vardif = pLexp->m_pFirst->m_Memb;
  else
    m_Vardif = pLexp;
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TDeriv::Reduce() const
  {
  return new TDeriv( m_DerivAsStroke, m_Partial, m_Expdif.Reduce(), m_Vardif.Reduce() );
  }

MathExpr TDeriv::Clone() const
  {
  TDeriv *pResult = new TDeriv( m_DerivAsStroke.Clone(), m_Partial.Clone(), m_Expdif.Clone(), m_Vardif.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TDeriv::Diff( const QByteArray& d )
  {
  if( !( IsType( TVariable, m_Vardif ) ) )
    {
    Info_m( X_Str( "MCannotDiff", "Cannot calculate!" ) );
    return this;
    }
  MathExpr  ex = m_Expdif.Diff( CastConst( TVariable, m_Vardif)->Name() );
  if( s_GlobalInvalid || s_CheckError ) return ex;
  return ex.Diff( d );
  }

MathExpr TDeriv::Integral( const QByteArray& d )
  {
  if( !( IsType( TVariable, m_Vardif ) ) )
    {
    Info_m( X_Str( "MCannotDiff", "Cannot calculate!" ) );
    return this;
    }
  if( CastConst( TVariable, m_Vardif )->Name().toUpper() == d.toUpper() )
    return m_Expdif;
  return MathExpr( this ) * Variable( d );
  }

MathExpr TDeriv::Perform() const
  {
  return new TDeriv( m_DerivAsStroke, m_Partial, m_Expdif.Perform(), m_Vardif.Perform() );
  }

bool TDeriv::Eq( const MathExpr& E2 ) const
  {
  MathExpr  ex, vr;
  if( E2.Deriv( ex, vr ) )
    return m_Partial.Eq( CastConst( TDeriv, E2 )->m_Partial ) && m_Expdif.Eq( ex ) && m_Vardif.Eq( vr );
  return false;
  }

bool TDeriv::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, ex, vr;
  QByteArray _eqName;
  if( E2.Deriv( ex, vr ) )
    return m_Partial.Equal( CastConst( TDeriv, E2 )->m_Partial ) && m_Expdif.Equal( ex ) && m_Vardif.Equal( vr );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );
  return false;
  }

MathExpr TDeriv::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TDeriv( m_DerivAsStroke, m_Partial, m_Expdif.Substitute( vr, vl ), m_Vardif );
  }

QByteArray TDeriv::WriteE() const
  {
  return QByteArray( DerivName ) + '(' + m_DerivAsStroke.WriteE() + ',' + m_Partial.WriteE() + ',' + m_Expdif.WriteE() + ',' + m_Vardif.WriteE() + ')';
  }

bool TDeriv::Deriv( MathExpr& ex, MathExpr& vr ) const
  {
  ex = m_Expdif;
  vr = m_Vardif;
  return true;
  }

QByteArray TDeriv::SWrite() const
  {
  QByteArray Result;
  bool bStroke, bPartial;
  m_DerivAsStroke.Boolean_( bStroke );
  m_Partial.Boolean_( bPartial );
  if( bStroke )
    if( bPartial )
      Result = "\\strkprtder{";
    else
      Result = "\\strkder{";
  else
    if( bPartial )
      Result = "\\prtder{";
    else
      Result = "\\der{";
  MathExpr op1, op2;
  QByteArray N;
  if( m_Expdif.Summa( op1, op2 ) || m_Expdif.Subtr( op1, op2 ) || m_Expdif.Multp( op1, op2 ) || m_Expdif.Divis( op1, op2 ) ||
    m_Expdif.Negative() || m_Expdif.Measur_( op1, op2 ) || m_HasParenthesis ) 
    Result += '(' + m_Expdif.SWrite() + ")}{";
  else
    Result += m_Expdif.SWrite() + "}{";
  if( IsConstType( TVariable, m_Vardif ) ) return Result + m_Vardif.SWrite() + '}';
  const TLexp *pList = CastConstPtr( TLexp, m_Vardif );
  Result += pList->m_pFirst->m_Memb.SWrite();
  if( pList->Count() == 1 ) return Result + '}';
  for( PExMemb pMemb = pList->m_pFirst->m_pNext; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    Result += pMemb->m_Memb.SWrite();
  return Result + '}';
  }

TAbs::TAbs( bool M, const MathExpr& ex ) : TExpr()
  {
  if( IsConstType( TMatr, ex ) )
    {
    MathExpr Ex = ex.Clone();
    ( CastPtr( TMatr, Ex ) )->m_IsVisible = false;
    m_Exp = Ex;
    }
  else
    m_Exp = ex;
  m_Meta_sign = M;
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TAbs::Clone() const
  {
  TAbs *pResult = new TAbs( m_Meta_sign, m_Exp.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TAbs::Reduce() const
  {
  double V;
  MathExpr texp;
  MathExpr modul;
  int N, D;
  if( IsConstType( TMatr, m_Exp ) )
    {
    const TMatr &Matr = *( CastConstPtr( TMatr, m_Exp ) );
    if( Matr.IsSquare() )
      {
      TMatr::sm_RecursionDepth = 0;
      return Matr.Determinant();
      }
    }
  texp = m_Exp.Reduce();
  if( texp.Positive() )
    return texp.Ptr();

  texp = texp.ReduceTExprs();

  if( texp.Unarminus( modul ) )
    return modul.Ptr();

  if( texp.Constan( V ) )
    return Constant( abs( V ) ).Ptr();

  if( texp.SimpleFrac_( N, D ) && ( N < 0 && D >= 0 || N >= 0 && D < 0 ) )
    return new TSimpleFrac( abs( N ), abs( D ) );

  return new TAbs( m_Meta_sign, m_Exp.Reduce() );
  }

MathExpr TAbs::Perform() const
  {
  return new TAbs( m_Meta_sign, m_Exp.Perform() );
  }

MathExpr TAbs::Lim( const QByteArray& v, const MathExpr& lm ) const
  {
  MathExpr  Elm( m_Exp.Lim( v, lm ) );
  if( Elm.IsEmpty() ) return nullptr;
  bool Ng;
  if( Elm.Infinity( Ng ) ) return new TInfinity( false );
  return new TAbs( false, Elm );
  }

bool TAbs::Eq( const MathExpr& E2 ) const
  {
  MathExpr  Ex;
  if( E2.Abs_( Ex ) ) return m_Exp.Eq( Ex );
  return false;
  }

bool TAbs::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ex;
  if( E2.Abs_( Ex ) ) return m_Exp.Equal( Ex );

  QByteArray EqName;
  if( E2.Variab( EqName ) && s_ExpStore.Known_var_expr( EqName, Ex ) ) return Equal( Ex );
  if( E2.Funct( EqName, Ex ) && EqName == "Simplify" ) return Equal( Ex );
  return false;
  }

MathExpr TAbs::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TAbs( m_Meta_sign, m_Exp.Substitute( vr, vl ) );
  }

QByteArray TAbs::WriteE() const
  {
  return QByteArray(AbsName) + '(' + m_Exp.WriteE() + ')';
  }

QByteArray TAbs::SWrite() const
  {
  return "\\abs{" + m_Exp.SWrite() + '}';
  }

bool TAbs::Abs_( MathExpr& ex ) const
  {
  ex = m_Exp;
  return true;
  }

bool TAbs::HasMatrix() const
  {
  return m_Exp.HasMatrix();
  }

bool TAbs::ConstExpr() const
  {
  return m_Exp.ConstExpr();
  }

bool TAbs::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  bool Result = m_Exp.Replace( Target, Source );
#ifdef DEBUG_TASK
  if( Result ) m_Contents = WriteE();
#endif
  return Result;
  }

TComplexExpr::TComplexExpr( const MathExpr& ex1, const MathExpr& ex2 ) : TExpr(), m_Re( ex1 ), m_Im( ex2 )
  {
  SetReduced( ( IsConstType( TConstant, ex1 ) || IsConstType( TVariable, ex1 ) ) &&
    ( IsConstType( TConstant, ex2 ) || IsConstType( TVariable, ex2 ) ) );
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TComplexExpr::Clone() const
  {
  TComplexExpr *pResult = new TComplexExpr( m_Re.Clone(), m_Im.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr CreateComplex( const MathExpr& Re, const MathExpr& Im ) 
  {
  if (Im == 0) return Re;
  if( !MathExpr::sm_NoReduceByCompare && Re.Reduce() == 0 && !( IsConstType( TConstant, Im ) ) && !( IsConstType( TVariable, Im ) ) )
    {
    TMult *pMult = new TMult( Im, CreateComplex( 0, 1 ) );
    pMult->SetReduced( Im.WasReduced() );
    return pMult;
    }
  return new TComplexExpr( Re, Im ); 
  }

MathExpr TComplexExpr::Reduce() const
  {
  if( m_Im.Reduce() == 0 )
    return m_Re.Reduce();
  MathExpr Re( m_Re.Reduce() );
  MathExpr Im( m_Im.Reduce() );
  if(Re.Eq(m_Re) && Im.Eq(m_Im))
    if(Re == 0 && Im == 0)
      return Constant(0);
    else
      return CreateComplex( Re, Im );
  if( Re == 0 && !( IsType( TConstant, Im ) ) && !(IsType( TVariable, Im ) ) )
    return new TMult( Im, CreateComplex( 0, 1 ) );
  return CreateComplex( Re, Im );
  }

MathExpr TComplexExpr::Perform() const
  {
  return CreateComplex( m_Re.Perform(), m_Im.Perform() );
  }

bool TComplexExpr::Eq( const MathExpr& E2 ) const
  {
  MathExpr op1, op2;
  return E2.Complex( op1, op2 ) && m_Re.Eq( op1 ) && m_Im.Eq( op2 );
  }

bool TComplexExpr::Equal( const MathExpr& E2 ) const
  {
  MathExpr op1, op2, exp;
  if( E2.Unarminus( exp ) )
    {
    if( !exp.Complex( op1, op2 ) )
      return false;
    op1 = -(op1);
    op2 = -(op2);
    return m_Re.Equal( op1 ) && m_Im.Equal( op2 );
    }
  return E2.Complex( op1, op2 ) && m_Re.Equal( op1 ) && m_Im.Equal( op2 );
  }

MathExpr TComplexExpr::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return CreateComplex( m_Re.Substitute( vr, vl ), m_Im.Substitute( vr, vl ) );
  }

QByteArray TComplexExpr::WriteE() const
  {
  QByteArray Im( m_Im.WriteE() );
  if( Im[0] == '-' )
    return m_Re.WriteE() + Im + '*' + SimpleChar( msImUnit );
  if( IsConstType( TConstant, m_Im ) || IsConstType( TFunc, m_Im ) )
    return m_Re.WriteE() + '+' + Im + '*' + SimpleChar( msImUnit );
  return m_Re.WriteE() + "+(" + Im + ")*" + SimpleChar( msImUnit );
  }

QByteArray TComplexExpr::SWrite() const
  {
  QByteArray Im( m_Im.SWrite() ), Re( m_Re.SWrite() );
  if( Re == "0" ) Re.clear();
  if( Im == "1" ) Im.clear();
  if( Im == "-1" ) Im = "-";
  if( Im[0] == '-' || Re.isEmpty() )
    return Re + Im + "\\im";
  return Re + '+' + Im + "\\im";
  }

bool TComplexExpr::Complex( MathExpr& op1, MathExpr& op2 ) const
  {
  op1 = m_Re;
  op2 = m_Im;
  return true;
  }

Bracketed TComplexExpr::MustBracketed() const
  {
  if( m_Re != 0 ) return brOperation;
  if( m_Im != 1 ) return brPower;
  return brNone;
  }

bool TComplexExpr::HasComplex() const
  {
  return true;
  }

bool TComplexExpr::ConstExpr() const
  {
  return m_Re.ConstExpr() && m_Im.ConstExpr();
  }

bool TComplexExpr::ImUnit() const 
  { 
  return m_Re == 0 && ( m_Im == 1 || m_Im == -1);
  }

bool TComplexExpr::Cons_int( int& I ) const
  {
  return m_Im == 0 && m_Re.Cons_int( I );
  }

TTrigoComplex::TTrigoComplex( double a, double b, double aScale ) : TExpr(),
  m_Re( a ), m_Im( b ), m_Scale( aScale )
  {
#ifdef AO_DEBUG
  Contents = WriteE;
#endif
  }

MathExpr TTrigoComplex::Clone() const
  {
  TTrigoComplex *pResult = new TTrigoComplex( m_Re, m_Im, m_Scale );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

QByteArray TTrigoComplex::SWrite() const
  {
  return "\\trigocomp {" + NumberToStr( m_Re ) + ',' + NumberToStr( m_Im ) + ',' + NumberToStr(m_Scale) + '}';
  }

QByteArray TTrigoComplex::WriteE() const
  {
  return "@TrigoComplex(" + NumberToStr( m_Re ) + "+" + NumberToStr( m_Im ) + "*i," +
    NumberToStr( m_Scale ) + ",false)";
  }

bool TTrigoComplex::Equal( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TTrigoComplex, E2 ) ) ) return false;
  return ( abs( CastConstPtr(TTrigoComplex, E2 )->m_Re - m_Re ) < 0.001 ) && ( abs( CastConstPtr(TTrigoComplex, E2 )->m_Im - m_Im ) < 0.001 );
  }

bool TTrigoComplex::Eq( const MathExpr& E2 ) const
  {
  return Equal( E2 );
  }

TMatr::TMatr(const MathExpr& ex) : TExpr(), m_IsVisible(true), m_IsNumerical(true), m_Exp(ex), m_RowCount(0), m_ColCount(0)
  {
  PExMemb f1, f2;
  MathExpr arg;
  double val;
  if( ex.List2ex( f1 ) )
    {
    for( ; !f1.isNull(); f1 = f1->m_pNext )
      {
      if( f1->m_Memb.Listex( f2 ) )
        {
        QVector< MathExpr > Row;
        for( ; !f2.isNull(); f2 = f2->m_pNext )
          {
          if( f2->m_Memb.Unarminus( arg ) && arg.Constan( val ) )
            f2->m_Memb = Constant( ( -val ) );
          Row.push_back( f2->m_Memb );
          m_IsNumerical = m_IsNumerical && f2->m_Memb.IsNumerical();
          }
          if( m_ColCount == 0 )
            m_ColCount = Row.count();
          else
            if( Row.count() != m_ColCount )
              throw  ErrParser( X_Str( "MWrongMartrix", "Wrong martrix!" ), peSyntacs );
          m_A.push_back( Row );
        }
      else
        {
        if( m_ColCount > 1 )
          throw  ErrParser( X_Str( "MWrongMartrix", "Wrong martrix!" ), peSyntacs );
        if( f1->m_Memb.Unarminus( arg ) && arg.Constan( val ) )
          f1->m_Memb = Constant( ( -val ) );
        m_ColCount = 1;
        m_A.push_back( QVector< MathExpr >( 1, f1->m_Memb ) );
        m_IsNumerical = m_IsNumerical && f1->m_Memb.IsNumerical();
        }
      }
    }
  else
    {
    if( ex.Listex( f2 ) )
      {
      QVector< MathExpr > Row;
      for( ; !f2.isNull(); f2 = f2->m_pNext )
        {
        if( f2->m_Memb.Unarminus( arg ) && arg.Constan( val ) )
          f2->m_Memb = Constant( ( -val ) );
        Row.push_back( f2->m_Memb );
        m_IsNumerical = m_IsNumerical && f2->m_Memb.IsNumerical();
        }
      m_ColCount = Row.count();
      m_A.push_back( Row );
      }
    else
      {
      m_A.push_back( QVector< MathExpr >( 1, ex ) );
      m_IsNumerical = ex.IsNumerical();
      m_ColCount = 1;
      }
    }
  m_RowCount = m_A.count();
  m_Rank = std::min( m_RowCount, m_ColCount );
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

TMatr::TMatr(const MatrixArry& Matr) : TExpr(), m_IsVisible(true), m_IsNumerical(true), m_A(Matr)
  {
  m_RowCount = m_A.count();
  m_ColCount = m_A[0].count();
  m_Rank = std::min( m_RowCount, m_ColCount );
  if( m_RowCount == 1 )
    if( m_ColCount == 1 )
      {
      m_Exp = m_A[0][0];
      m_IsNumerical = m_Exp.IsNumerical();
#ifdef DEBUG_TASK
      m_Contents = WriteE();
#endif
      return;
      }
    else
      {
      TLexp *pLexp = new TLexp;
      m_Exp = pLexp;
      for( int j = 0; j < m_ColCount; j++ )
        {
        m_IsNumerical = m_IsNumerical && m_A[0][j].IsNumerical();
        pLexp->Addexp( m_A[0][j] );
        }
#ifdef DEBUG_TASK
      m_Contents = WriteE();
#endif
      return;
      }

  TL2exp *pL2exp = new TL2exp;
  m_Exp = pL2exp;
  if( m_ColCount == 1 )
    {
    for( int i = 0; i < m_RowCount; i++ )
      {
      pL2exp->Addexp( m_A[i][0] );
      m_IsNumerical = m_IsNumerical && m_A[i][0].IsNumerical();
      pL2exp->m_pLast->m_Visi = false;
      }
#ifdef DEBUG_TASK
    m_Contents = WriteE();
#endif
    return;
    }

  for( int i = 0; i < m_RowCount; i++ )
    {
    TLexp *pRow = new TLexp;
    pL2exp->Addexp( pRow );
    pL2exp->m_pLast->m_Visi = false;
    for( int j = 0; j < m_ColCount; j++ )
      {
      pRow->Addexp( m_A[i][j] );
      m_IsNumerical = m_IsNumerical && m_A[i][j].IsNumerical();
      }
    }
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TMatr::Clone() const
  {
  TMatr *pResult = new TMatr( m_Exp.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

MathExpr TMatr::Reduce() const
  {
  return new TMatr( m_Exp.Reduce() );
  }

MathExpr TMatr::Perform() const 
  {
  return new TMatr( m_Exp.Perform() );
  }

bool TMatr::Eq( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TMatr, E2 ) ) ) return false;
  const TMatr *pMatrix = CastConstPtr( TMatr, E2 );
  if( m_IsNumerical ^ pMatrix->m_IsNumerical ) return false;
  if( m_RowCount != pMatrix->m_RowCount || m_ColCount != pMatrix->m_ColCount ) return false;
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      if( !m_A[i][j].Eq( pMatrix->m_A[i][j] ) ) return false;
  return true;
  }

bool TMatr::Equal( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TMatr, E2 ) ) ) return false;
  const TMatr *pMatrix = CastConstPtr( TMatr, E2 );
  if( m_IsNumerical ^ pMatrix->m_IsNumerical ) return false;
  if( m_RowCount != pMatrix->m_RowCount || m_ColCount != pMatrix->m_ColCount ) return false;
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      if( !m_A[i][j].Equal( pMatrix->m_A[i][j] ) ) return false;
  return true;
  }

QByteArray TMatr::WriteE() const
  {
  return QByteArray(MatrName) + '(' + m_Exp.WriteE() + ')';
  }

QByteArray TMatr::SWrite() const
  {
  if( m_RowCount > 1 )
    {
    PExMemb f1, f2;
    m_Exp.List2ex( f1 );
    for( int i = 0; i < m_RowCount; i++, f1 = f1->m_pNext )
      {
      if( !f1->m_Memb.Listex( f2 ) )
        {
        MathExpr exTmp = f1->m_Memb;
        f1->m_Memb =  new TLexp;
        CastPtr(TLexp, f1->m_Memb)->Addexp( exTmp );
        f1->m_Memb.Listex( f2 );
        }
      f1->m_Visi = false;
      for( int j = 0; j < m_ColCount; j++, f2 = f2->m_pNext )
        f2->m_Spacer = true;
      }
    }
  QByteArray S( m_IsVisible ? "\\matrix" : "\\matrix_unv" );
  return S + '{' + QByteArray::number( m_RowCount ) + ',' + QByteArray::number( m_ColCount ) + "}{" + m_Exp.SWrite() + '}';
  }

bool TMatr::Matr( MathExpr& ex ) const
  {
  ex = m_Exp;
  return true;
  }

MathExpr TMatr::Mult( const MathExpr& exp ) const
  {
  bool OldGlobInvalid = s_GlobalInvalid;
  if (IsConstType(TMatr, exp))
    {
    const TMatr& B = *(CastConstPtr(TMatr, exp));
    if (B.m_RowCount != m_ColCount)
      {
      s_LastError = "Was mistake at dimensions of matrixes";
      throw  ErrParser("No Solution", peNoSolv);
      }
    int ColCount = B.m_ColCount;
    const MatrixArry &BA = B.m_A;
    MatrixArry TA(m_RowCount);
    for (int i = 0; i < m_RowCount; i++)
      for (int j = 0; j < ColCount; j++)
        {
        MathExpr Term = m_A[i][0] * BA[0][j], Ex, Ex1;
        if (m_IsNumerical || Term.HasComplex() )
          Ex = Term.Reduce();
        else
          Ex = CalcMulti(3, Term, false);
        for (int k = 1; k < m_ColCount; k++)
          {
          Term = m_A[i][k] * BA[k][j];
          if (m_IsNumerical || Term.HasComplex())
            Ex1 = Term.Reduce();
          else
            Ex1 = CalcMulti(3, Term, false);
          Term = Ex + Ex1;
          if (m_IsNumerical || Term.HasComplex())
            Ex = Term.Reduce();
          else
            Ex = CalcMulti(1, Term, false);
          }
        TA[i].push_back(Ex);
        }

    if (!m_IsNumerical)
      for (int i = 0; i < m_RowCount; i++)
        for (int j = 0; j < ColCount; j++)
          TA[i][j] = TA[i][j].Reduce();
    s_GlobalInvalid = OldGlobInvalid;
    if (m_RowCount == 1 && ColCount == 1)
      return TA[0][0];
    return new TMatr(TA);
    }
  MatrixArry TA( m_RowCount );
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      TA[i].push_back( ( m_A[i][j] * exp ).Reduce() );
  s_GlobalInvalid = OldGlobInvalid;
  return new TMatr( TA );
  }

MathExpr TMatr::Add( const MathExpr& exp ) const
  {
  if( !( IsConstType( TMatr, exp ) ) || ( CastConstPtr( TMatr, exp )->m_RowCount != m_RowCount ) || ( CastConstPtr( TMatr, exp )->m_ColCount != m_ColCount ) )
    {
    s_LastError = "Was mistake at dimensions of matrixes";
    throw  ErrParser( "No Solution", peNoSolv );
    }
  const MatrixArry &BA = ( CastConstPtr( TMatr, exp ) )->m_A;
  MatrixArry TA( m_RowCount );
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      TA[i].push_back( ( m_A[i][j] + BA[i][j] ).Reduce() );
  return new TMatr( TA );
  }

MathExpr TMatr::Subt( const MathExpr& exp ) const
  {
  if( !( IsConstType( TMatr, exp ) ) || ( CastConstPtr( TMatr, exp )->m_RowCount != m_RowCount ) || ( CastConstPtr( TMatr, exp )->m_ColCount != m_ColCount ) )
    {
    s_LastError = "Was mistake at dimensions of matrixes";
    throw  ErrParser( "No Solution", peNoSolv );
    }
  const MatrixArry &BA = ( CastConstPtr( TMatr, exp ) )->m_A;
  MatrixArry TA( m_RowCount );
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      TA[i].push_back( ( m_A[i][j] - BA[i][j] ).Reduce() );
  return new TMatr( TA );
  }

MathExpr TMatr::Inversion() const
  {
  if (m_RowCount != m_ColCount)
    {
    s_LastError = "Matrix is ! square";
    throw  ErrParser("No Solution", peNoSolv);
    }
  if (m_IsNumerical)
    {
    QVector<QVector<double>> A(m_RowCount);
    double Mean = 0, Val;
    for (int i = 0; i < m_RowCount; i++)
      for (int j = 0; j < m_ColCount; j++)
        {
        m_A[i][j].Constan(Val);
        A[i].push_back(Val);
        Mean += abs(Val);
        }
    double Tol = Mean / (m_RowCount * m_RowCount) * 1e-8;
    QVector<int> L(m_RowCount);
    QVector<int> Mm(m_RowCount);
    for (int k = 0; k < m_RowCount; k++)
      {
      L[k] = k;
      Mm[k] = k;
      double Biga = A[k][k];
      for (int j = k; j < m_RowCount; j++)
        for (int i = k; i < m_RowCount; i++)
          if (abs(Biga) < abs(A[i][j]))
            {
            Biga = A[i][j];
            L[k] = i;
            Mm[k] = j;
            }
      int j = L[k];
      if (j > k)
        for (int i = 0; i < m_RowCount; i++)
          {
          double Hold = -A[k][i];
          A[k][i] = A[j][i];
          A[j][i] = Hold;
          }
      int i = Mm[k];
      if (i > k)
        for (int j = 0; j < m_RowCount; j++)
          {
          double Hold = -A[j][k];
          A[j][k] = A[j][i];
          A[j][i] = Hold;
          }
      if (abs(Biga) < Tol)
        {
        //        m_Rank = k;
        s_LastError = "Matrix is singular";
        throw  ErrParser(X_Str("MEnterNonSingMatr", "Enter nonsingular matrix!"), peNewErr);
        }
      for (int i = 0; i < m_RowCount; i++)
        if (i != k) A[i][k] = -A[i][k] / Biga;
      for (int i = 0; i < m_RowCount; i++)
        {
        double Hold = A[i][k];
        for (int j = 0; j < m_RowCount; j++)
          if ((i != k) && (j != k)) A[i][j] += Hold * A[k][j];
        }
      for (int j = 0; j < m_RowCount; j++)
        if (j != k) A[k][j] /= Biga;
      A[k][k] = 1.0 / Biga;
      }
    if (m_RowCount > 1)
      for (int k = m_RowCount - 2; k >= 0; k--)
        {
        int i = L[k];
        if (i > k)
          for (int j = 0; j < m_RowCount; j++)
            {
            double Hold = A[j][k];
            A[j][k] = -A[j][i];
            A[j][i] = Hold;
            }
        int j = Mm[k];
        if (j > k)
          for (int i = 0; i < m_RowCount; i++)
            {
            double Hold = A[k][i];
            A[k][i] = -A[j][i];
            A[j][i] = Hold;
            }
        }
    MatrixArry TA(m_RowCount);
    for (int i = 0; i < m_RowCount; i++)
      for (int j = 0; j < m_ColCount; j++)
        TA[i].push_back(Constant(A[i][j]));
    return new TMatr(TA);
    }
  MatrixArry TA(m_RowCount);
  if (m_RowCount == 1)
    {
    TA[0].push_back(Constant(1) / m_A[0][0]);
    return new TMatr(TA);
    }
  int RecursionDepth = 0;
  MathExpr  ExDet = ToFactorsDel(Expand(Determinant()));
  if (ExDet == 0)
    {
    s_LastError = "Singular matrix";
    throw  ErrParser(X_Str("MEnterNonSingMatr", "Enter nonsingular matrix!"), peNewErr);
    }
  MatrixArry aMinor(m_RowCount - 1);
  for (int i = 0; i < m_RowCount - 1; aMinor[i++].resize(m_RowCount - 1));
  for (int i = 0; i < m_RowCount; TA[i++].resize(m_RowCount));
  for (int i = 0; i < m_RowCount; i++)
    for (int j = 0; j < m_RowCount; j++)
      {
      int i2 = 0;
      for (int i1 = 0; i1 < m_RowCount; i1++)
        if (i1 != j)
          {
          int j2 = 0;
          for (int j1 = 0; j1 < m_RowCount; j1++)
            if (j1 != i)
              {
              aMinor[i2][j2] = m_A[i1][j1];
              j2++;
              }
          i2++;
          }
      MathExpr EMinor = new TMatr(aMinor);
      const TMatr &Minor = *(CastConstPtr(TMatr, EMinor));
      MathExpr exAlgAdd = Minor.Determinant();
      if ((i + j) % 2 > 0) exAlgAdd = -exAlgAdd;
      MathExpr   divisor = ExDet;
      MathExpr ex = exAlgAdd.Reduce();
      exAlgAdd = ex;
      if (divisor == 1)
        TA[i][j] = exAlgAdd;
      else
        {
        MathExpr dividend;
        ex.Clear();
        if (exAlgAdd.Unarminus(dividend) ^ divisor.Unarminus(ex))
          {
          if (dividend.IsEmpty()) dividend = exAlgAdd;
          if (ex.IsEmpty()) ex = divisor;
          TA[i][j] = -(dividend / ex);
          }
        else
          TA[i][j] = exAlgAdd / divisor;
        }
      }
  for (int i = 0; i < m_RowCount; i++)
    for (int j = 0; j < m_RowCount; j++)
      TA[i][j] = TA[i][j].Reduce();
  MathExpr Result(new TMatr(TA));
  s_GlobalInvalid = false;
  return Result.Reduce();
  }

MathExpr TMatr::Transpose() const
  {
  MatrixArry A(m_ColCount);
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      A[j].push_back( m_A[i][j] );
  return new TMatr( A );
  }

bool TMatr::IsTemplate()
  {
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      {
      TStr *pS = Cast(TStr, m_A[i][j].Ptr());
      if(pS != nullptr && pS->Value() == " ")  return true;
      }
  return false;
  }

MathExpr TMatr::Determinant() const
  {
  sm_RecursionDepth--;
  if( !IsSquare() )
    {
    s_LastError = "Nonsquare matrix";
    throw  ErrParser( X_Str(  "MEnterSqMatr", "Enter square matrix!" ), peNoSolv );
    }
  if( m_RowCount == 1 ) return m_A[0][0];

  MathExpr Texp;
  if( m_RowCount == 2 )
    Texp = m_A[0][0] * m_A[1][1] - m_A[0][1] * m_A[1][0];
    else
      {
      QVector<MatrixArry> TA( m_RowCount );
      for( int i = 0; i < m_RowCount; i++ )
        {
        TA[i].resize( m_RowCount - 1 );
        int i2 = 0;
        for( int i1 = 0; i1 < m_RowCount; i1++ )
          if( i1 != i )
            {
            for( int j = 1; j < m_ColCount; j++ )
              TA[i][i2].push_back( m_A[i1][j] );
            i2++;
            }
        MathExpr Term = m_A[i][0] * MathExpr( new TMatr(TA[i] ) ).Abs();
        if( i == 0 )
          Texp = Term;
        else
          if( i % 2 )
            Texp = Texp - Term;
        else
          Texp = Texp + Term;
        }
      }
  if( sm_RecursionDepth == 1 )
    return new TBinar( '=', Texp, Expand( Texp ).Reduce() );
  MathExpr Result = Expand(Texp).Reduce();
  s_GlobalInvalid = false;
  return Result;
  }

MathExpr TMeaExpr::Reduce() const
  {
  return new TMeaExpr( m_Operand1.Reduce(), m_Operand2.Reduce() );
  }

bool TMeaExpr::ConstExpr() const
  {
  return m_Operand1.ConstExpr();
  }

MathExpr TUnapm::Reduce() const
  {
  MathExpr argument = m_Arg.Reduce();
  MathExpr Op;
  if( argument.Unarminus( Op ) )
    return Op.Unapm().Ptr();

  double Value;
  if( argument.Constan( Value ) && Value < 0 )
    return Constant( -Value ).Unapm().Ptr();

  int N, D;
  if( argument.SimpleFrac_( N, D ) && argument.Negative() )
    {
    if( D < 0 )
      D = -D;
    else
      N = -N;
    return MathExpr( new TSimpleFrac( N, D ) ).Unapm();
    }

  int I;
  if( argument.MixedFrac_( I, N, D ) && argument.Negative() )
    return MathExpr( new TMixedFrac( -I, -N, D ) ).Unapm();

  if( argument.Unapm_( Op ) )
    return argument.Ptr();

  return argument.Unapm().Ptr();
  }

bool TFunc::Replace( const MathExpr& Target, const MathExpr& Source )
  {
  bool Result = m_Arg.Equal( Target );
  if( Result )
    m_Arg = Source;
  else
    Result = m_Arg.Replace( Target, Source );
#ifdef DEBUG_TASK  
  if( Result ) m_Contents = WriteE(); 
#endif
  return Result;
  }

QByteArray TFunc::WriteE() const
  {
  if( !m_Arg.IsEmpty() )
    return m_ShortName + '(' + m_Arg.WriteE() + ')';
  return m_ShortName + "( ? )";
  }

QByteArray TFunc::SWrite() const
  {
  if(m_Name == "exp")
    return "\\power{e}{" + m_Arg->SWrite() + '}';
  if( m_Name == "BinomCoeff" )
    {
    MathExpr T = m_Arg.Clone();
    TLexp *pTmp = CastPtr( TLexp, T );
    return "\\bcoeff{" + pTmp->m_pFirst->m_Memb->SWrite() + "}{" + pTmp->m_pFirst->m_pNext->m_Memb->SWrite() + '}';
    }
  if( m_Name == "ACoeff" )
    {
    MathExpr T = m_Arg.Clone();
    TLexp *pTmp = CastPtr( TLexp, T );
    return "\\acoeff{" + pTmp->m_pFirst->m_Memb->SWrite() + "}{" + pTmp->m_pFirst->m_pNext->m_Memb->SWrite() + '}';
    }
  if( m_Name == "PerCount" )
    return "\\percount{" + m_Arg->SWrite() + '}';
  if( m_Name == "factorial" )
    {
    if( ( IsConstType( TVariable, m_Arg ) ) || ( IsConstType( TConstant, m_Arg ) ) )
      return m_Arg->SWrite() + '!';
    return '(' + m_Arg->SWrite() + ")!";
    }
  if( m_Name == "func" )
    {
    MathExpr T = m_Arg.Clone();
    TLexp *pTmp = CastPtr( TLexp, T );
    if(pTmp == nullptr)
      {
      s_LastError = "Invalid function argument";
      throw  ErrParser(X_Str("MBadArgument", "Invalid function argument!"), peNewErr);
      }
    QByteArray Result( "\\func{" + pTmp->m_pFirst->m_Memb->SWrite() + "}{" );
    pTmp->DeleteMemb( pTmp->m_pFirst );
    return Result + pTmp->SWrite() + '}';
    }
  if (m_Name == "@Picture")
    {
    return m_Name + '(' + m_Arg->WriteE() + ')';
    }

  if(m_Name == "SaveBrackets")
    {
    return '(' + m_Arg->WriteE() + ')';
    }

  QByteArray Name;
  MathExpr Op1, Op2;
  uchar C;
  PExMemb Memb;
  if( !m_Name.isEmpty() && ( m_Arg->Summa( Op1, Op2 ) || m_Arg->Subtr( Op1, Op2 ) || m_Arg->Multp( Op1, Op2 ) ||
    ( m_Arg->Divis( Op1, Op2 ) && !m_Arg.AsFraction() ) || m_Arg->Binar_( C, Op1, Op2 ) || m_Arg->BoolOper_( Name, Op1, Op2 ) ||
    m_Arg->Boolnot_( Op1 ) || m_Arg->Negative() || m_Arg->Listex( Memb ) || m_Arg->Listord( Memb ) || m_Arg->Funct( Name, Op1 ) ||
    m_Arg->Measur_( Op1, Op2 ) || m_Arg->Power( Op1, Op2 ) || m_Name == "P" || m_Name == "ERF" ) ) 
    return m_Name + '(' + m_Arg->SWrite() + ')';
  return m_Name + m_Arg->SWrite();
  }

bool TFunc::Funct( QByteArray& N, MathExpr& A ) const
  {
  N = m_Name;
  A = m_Arg;
  return true;
  }

bool TFunc::Splitted() const
  {
  return m_Arg.Splitted();
  }

bool TFunc::Log( MathExpr& op1, MathExpr& op2 ) const
  {
  MathExpr LogBase;
  if( m_Name == "lg" && m_LogBase.IsEmpty() )
    LogBase = new TConstant( 10 );
  if( m_Name == "ln" && m_LogBase.IsEmpty() )
    LogBase = new TConstant( M_E );
  if( !LogBase.IsEmpty() )
    {
    op1 = LogBase;
    op2 = m_Arg;
    return true;
    }
  return false;
  }

bool TFunc::Positive() const
  {
  return m_Name == "exp";
  }

bool TFunc::ConstExpr() const
  {
  return m_Arg.ConstExpr();
  }

bool TFunc::HasComplex() const
  {
  return m_Arg.HasComplex();
  }

TBool::TBool(bool V) : m_Valueb(V)
  {
  SetReduced();
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif 
  }

MathExpr TBool::Clone() const
  {
  return new TBool( m_Valueb );
  }

MathExpr TBool::Diff( const QByteArray& d )
  {
  return Function( "diff", this);
  }

bool TBool::Eq( const MathExpr& E2 ) const
  {
  bool  V;
  return E2.Boolean_( V ) && m_Valueb == V;
  }

bool TBool::Equal( const MathExpr& E2 ) const
  {
  bool  V;
  MathExpr Ar2;
  QByteArray _eqName;

  if( E2.Boolean_( V ) )
    return Eq( E2 );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );

  Ar2 = E2.Reduce();
  if( Ar2.Boolean_( V ) )
    return Eq( Ar2 );
  return false;
  }

QByteArray TBool::WriteE() const
  {
  if( m_Valueb ) return "TRUE";
  return "FALSE";
  }

QByteArray TBool::SWrite() const
  {
  if( m_Valueb ) return "true";
  return "false";
  }

bool TBool::Boolean_( bool& V ) const
  {
  V = m_Valueb;
  return true;
  }

TSyst::TSyst(const MathExpr& ex) : TExpr(), m_Exp(ex)
  {
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TSyst::Clone() const
  {
  TSyst *pResult = new TSyst( m_Exp.Clone() );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

bool TSyst::ConstExpr() const
  {
  return m_Exp->ConstExpr();
  }

MathExpr TSyst::Reduce() const
  {
  TBinar::sm_NoBoolReduce = true;
  MathExpr Result = m_Exp->Reduce();
  TBinar::sm_NoBoolReduce = false;
  return Result;
  }

MathExpr TSyst::Perform() const
  {
  return new TSyst( m_Exp->Perform() );
  }

bool TSyst::Eq( const MathExpr& E2 ) const
  {
  MathExpr  ex;
  if( E2.Syst_( ex ) ) return m_Exp.Eq( ex );
  return false;
  }

bool TSyst::Equal( const MathExpr& E2 ) const
  {
  MathExpr  Ar2, ex;
  QByteArray _eqName;
  if( E2.Syst_( ex ) ) return m_Exp->Equal( ex );
  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) ) return m_Exp->Equal( Ar2 );
  if( E2.Funct( _eqName, Ar2 ) && ( _eqName == "Simplify" ) ) return m_Exp->Equal( Ar2 );
  return false;
  }

MathExpr TSyst::Substitute( const QByteArray& vr, const MathExpr& vl )
  {
  return new TSyst( m_Exp.Substitute( vr, vl ) );
  }

QByteArray TSyst::WriteE() const
  {
  return QByteArray(SystName) + '(' + m_Exp->WriteE() + ')';
  }

QByteArray TSyst::SWrite() const
  {
  return "\\system{" + m_Exp->SWrite() + '}';
  }

bool TSyst::Syst_( MathExpr& ex ) const
  {
  if( ex.Ptr() == this )
    ex = m_Exp.Clone();
  else
    ex = m_Exp;
  return true;
  }

TTable::TTable(const MathExpr& ex) : TMatr(), m_NoFreeze(false)
  {
  PExMemb f1, f2, FirstMemb;
  QByteArray S;
  m_Exp = ex;
  m_GridState = TGRVisible;
  m_IsVisible = true;
  m_RowCount =  m_ColCount = 0;
  m_IsNumerical = false;
  if( ex.List2ex( f1 ) )
    {
    FirstMemb = f1;
    if( !f1.isNull() && f1->m_Memb.Variab( S ) )
      if( S == "NoFreeze" )
        {
        m_NoFreeze = true;
        f1 = f1->m_pNext;
        }
      else
        if( S == "Unvisible" )
          {
          int i;
          do {
            f1 = f1->m_pNext;
            if( !f1->m_Memb.Cons_int( i ) ) break;
            m_UnvisibleColumns.push_back( i );
            } while( !f1.isNull() );
          }
        else
          throw  ErrParser( X_Str( "MWrongTable", "Wrong table!" ), peSyntacs );
    }
  if( ex.List2ex( f1 ) )
    {
    f1 = FirstMemb;
    for( ; !f1.isNull(); f1 = f1->m_pNext )
      {
      if( f1->m_Memb.Listex( f2 ) )
        {
        QVector< MathExpr > Row;
        for( ; !f2.isNull(); f2 = f2->m_pNext )
          Row.push_back( f2->m_Memb );
        if( m_ColCount == 0 || m_NoFreeze )
          m_ColCount = Row.count();
        else
          if( Row.count() != m_ColCount )
            throw  ErrParser( X_Str( "MWrongTable", "Wrong table!" ), peSyntacs );
        m_A.push_back( Row );
        }
      else
        if( m_UnvisibleColumns.empty() )
          {
          if( m_ColCount == 0 )
            m_ColCount = 1;
          else
            if( m_ColCount != 1 )
              throw  ErrParser( X_Str( "MWrongTable", "Wrong table!" ), peSyntacs );
          QVector< MathExpr > Row;
          Row.push_back( f1->m_Memb );
          m_A.push_back( Row );
          }
        else
          continue;
      }
    }
  else
    {
    QVector< MathExpr > Row;
    if( ex.Listex( f2 ) )
      {
      for( ; !f2.isNull(); f2 = f2->m_pNext )
        Row.push_back( f2->m_Memb );
      }
    else
      Row.push_back( ex );
    m_ColCount = Row.count();
    m_A.push_back( Row );
    }
  m_RowCount = m_A.count();
  m_Rank = std::min( m_ColCount, m_RowCount );
#ifdef DEBUG_TASK
  m_Contents = WriteE();
#endif
  }

MathExpr TTable::Clone() const
  {
  TTable  *pTable = new TTable( m_Exp.Clone() );
  pTable->m_GridState = m_GridState;
  return pTable;
  }

QByteArray TTable::SWrite() const
  {
  int RCount = m_RowCount;
  if(m_NoFreeze) RCount--;
  QByteArray Result( "\\table{" + QByteArray::number( RCount ) + ',' + QByteArray::number( m_ColCount ) + ',' +
    QByteArray::number( m_GridState ) + ',' + QByteArray::number( m_NoFreeze ) + ',' + QByteArray::number( m_UnvisibleColumns.count() ) );
  for( int i = 0; i < m_UnvisibleColumns.count(); i++ )
    Result += ',' + QByteArray::number( m_UnvisibleColumns[i] );
  Result += '}';
  int i = 0;
  if(m_NoFreeze) i = 1;
  for(; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      Result += "\\cell{" + m_A[i][j].SWrite() + '}';
  return Result + "\\endtable";
  }

QByteArray TTable::WriteE() const
  {
  QByteArray Name;
  switch (m_GridState )
    {
  case TGRVisible: 
    Name = "Table";
    break;
  case TGRUnvisible: 
    Name = "UnvisibleTable";
    break;
  case TGRPartlyVisible: 
    Name = "PartlyVisibleTable";
    }
  TLexp::sm_Bracketed = false;
  Name += '(' + m_Exp.WriteE() + ')';
  TLexp::sm_Bracketed = true;
  return Name;
  }

bool TTable::IsTemplate()
  {
  if(m_NoFreeze) return false;
  return TMatr::IsTemplate();
  }

bool TTable::Eq( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TTable, E2 ) ) ) return false;
  const TTable *pTable = CastConstPtr( TTable, E2 );
  if(pTable->m_NoFreeze == m_NoFreeze) return TMatr::Eq(E2);
  const TTable *pLeft = pTable, *pRight = this;
  if(m_NoFreeze)
    {
    pLeft = this;
    pRight = pTable;
    }
  if(pLeft->m_RowCount - 1 != pRight->m_RowCount) return false;
  for( int i = 0; i < pRight->m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      if( !pLeft->m_A[i+1][j].Eq( pRight->m_A[i][j] ) ) return false;
  return true;
  }

bool TTable::Equal( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TTable, E2 ) ) ) return false;
  const TTable *pTable = CastConstPtr( TTable, E2 );
  if(pTable->m_NoFreeze == m_NoFreeze) return TMatr::Equal(E2);
  const TTable *pLeft = pTable, *pRight = this;
  if(m_NoFreeze)
    {
    pLeft = this;
    pRight = pTable;
    }
  if(pLeft->m_RowCount - 1 != pRight->m_RowCount) return false;
  for( int i = 0; i < pRight->m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      if( !pLeft->m_A[i+1][j].Equal( pRight->m_A[i][j] ) ) return false;
  return true;
  }

TChart::TChart( const MathExpr& ex ) : m_Exp( ex ), m_N( 0 ), m_Scale( 1.0 ), m_FromTemplate( false )
  {
  SetReduced();
  PExMemb F1;
  if( !m_Exp.List2ex( F1 ) )
    throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
  QByteArray S;
  MathExpr exLeft, exRight;
  if( F1->m_Memb->Binar( '=', exLeft, exRight ) && exLeft.Variab( S ) && S == "m" && exRight.Constan( m_Scale ) ) F1 = F1->m_pNext;
  PExMemb FirstMemb = F1, F2;
  int M = 0;
  for( ; !F1.isNull(); F1 = F1->m_pNext )
    {
    if( !F1->m_Memb->Listex( F2 ) )
      throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
    if( ++M == 1 )
      for( ; !F2.isNull(); m_N++, F2 = F2->m_pNext );
    }
  if( M != 3 )
    throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
  F1 = FirstMemb;
  double rPrev = -3e50;
  bool WasError = false;
  for( int i = 0; !F1.isNull(); i++, F1 = F1->m_pNext )
    {
    int j = 0;
    m_A.push_back( QVector<MathExpr>( m_N ) );
    for( F1->m_Memb->Listex( F2 ); !F2.isNull() && j < m_N; F2 = F2->m_pNext, j++ )
      {
      m_A[i][j] = F2->m_Memb;
      int iVal;
      if( i == 0 )
        {
        if( j != 0 )
          if( F2->m_Memb.Variab( S ) )
            if( MapColor::s_MapColor.contains( S ) )
              m_A[i][j] = (int) MapColor::s_MapColor[S];
            else
              WasError = true;
          else
            if( F2->m_Memb.Str_() )
              {
              bool OK;
              m_A[i][j] = (int) CastPtr( TStr, F2->m_Memb )->Value().toUInt( &OK, 16 );
              WasError = !OK;
              }
          else
            WasError = !F2->m_Memb.Cons_int( iVal );
        else
          WasError = !F2->m_Memb.Variab( m_Type ) || ( m_Type != "ThinCol" && m_Type != "ThickCol" );
        }
      else
        if( j == 0 )
          WasError = !( IsConstType( TVariable, F2->m_Memb ) ) && !( IsConstType( TStr, F2->m_Memb ) );
        else
          {
          char cOper;
          if( m_Type == "ThickCol" && i == 1 )
            WasError = !F2->m_Memb->Oper_( cOper, exLeft, exRight ) || cOper != '-' || !( IsConstType( TConstant, exLeft ) ) ||
            !( IsConstType( TConstant, exRight ) );
          else
            {
            double rVal;
            WasError = !F2->m_Memb->Constan( rVal );
            if( !WasError && i == 1 )
              {
              WasError = rPrev >= rVal;
              rPrev = rVal;
              }
            }
          if( WasError && EdTable::sm_WasTemplate )
            WasError = !F2->m_Memb->Str_();
          }
      if( WasError )
        throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
      }
    if( j < m_N || !F2.isNull() )
      throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
    }
  if( !m_A[1][0].Variab( m_NameX ) )
    if( m_A[1][0].Str_() )
      m_NameX = CastPtr( TStr, m_A[1][0])->Value();
    else
      throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
  if( !m_A[2][0].Variab( m_NameY ) )
    if( m_A[2][0].Str_() )
      m_NameY = CastPtr(TStr, m_A[2][0])->Value();
    else
      throw  ErrParser( X_Str( "MWrongChart", "Wrong chart!" ), peSyntacs );
//  m_LabelX = "x";
  int i = m_NameX.indexOf( ':' );
  if( i != -1 )
    {
    m_LabelX = m_NameX.mid( i + 1, 1 );
    m_NameX = m_NameX.left( i );
    }
//  m_LabelY = "y";
  i = m_NameY.indexOf( ':' );
  if( i != -1 )
    {
    m_LabelY = m_NameY.mid( i + 1, 1 );
    m_NameY = m_NameY.left(i);
    }
#ifdef AO_DEBUG
  Contents = WriteE;
#endif
  }

MathExpr TChart::Clone() const
    {
    return new TChart( m_Exp.Clone() );
    }

QByteArray TChart::SWrite() const
  {
  QByteArray Result;
  if( !TStr::sm_Server )
    Result = "\\chart{" + m_Type + ',' + QByteArray::number( m_N - 1 ) + ',' + m_LabelX + ',' + m_LabelY + ',' + m_NameX + ',' +
      m_NameY + ',' + QByteArray::number( m_Scale ) + ',' + QByteArray::number( m_FromTemplate );
  else
    Result = "\\chart{" + m_Type + ',' + QByteArray::number( m_N - 1 ) + ',' + TStr( m_LabelX ).PackValue() + ',' +
    TStr( m_LabelY ).PackValue() + ',' + TStr( m_NameX ).PackValue() + ',' + TStr( m_NameY ).PackValue() + ',' +
      QByteArray::number( m_Scale ) + ',' + QByteArray::number( m_FromTemplate );
  TOper::sm_InsideChart = true;
  for( int i = 0; i < 3; i++ )
    for( int j = 1; j < m_N; j++ )
      if( IsConstType( TOper, m_A[i][j] ) )
        Result += ',' + m_A[i][j].SWrite();
      else
        if( i == 0 )
          {
          int Color;
          m_A[i][j].Cons_int( Color );
          Result += ',' + QByteArray::number( Color, 16 );
          }
        else
          Result += ',' + m_A[i][j].WriteE();
  TOper::sm_InsideChart = false;
   return Result + '}';
  }

bool TChart::Eq( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TChart, E2 ) ) || m_N != CastConstPtr( TChart, E2 )->m_N ) return false;
  for( int i = 1; i < 3; i++ )
    for( int j = 1; j < m_N; j++ )
      if( !m_A[i][j].Eq( CastConstPtr( TChart, E2 )->m_A[i][j] ) ) return false;
  return true;
  }

bool TChart::Equal( const MathExpr& E2 ) const
  {
  if( !( IsConstType( TChart, E2 ) ) || m_N != CastConstPtr( TChart, E2 )->m_N ) return false;
  for( int i = 1; i < 3; i++ )
    for( int j = 1; j < m_N; j++ )
      if( !m_A[i][j].Equal( CastConstPtr( TChart, E2 )->m_A[i][j] ) ) return false;
  return true;
  }

QByteArray TChart::WriteE() const
  {
  return "Chart(" + m_Exp.WriteE() + ')';
  }

bool TChart::Chart( MathExpr& ex ) const
  {
  ex = m_Exp;
  return true;
  }

TInterval::TInterval( double a, double L, const Lexp& P, const Lexp& S, const Lexp& I ) : TExpr(),
  m_Ax( a ), m_Len( L ), m_Points( P ), m_Signs(S), m_Intervals(I) {}

MathExpr TInterval::Clone() const
  {
  return new TInterval( m_Ax, m_Len, m_Points.Clone(), m_Signs.Clone(), m_Intervals.Clone() );
  }

QByteArray TInterval::SWrite() const
  {
  QByteArray Result = "\\intervals{" + QByteArray::number( m_Ax ) + ',' + QByteArray::number( m_Len );
  bool Sign, Interval;
  for( PExMemb fP = ( ( Lexp ) m_Points ).First(), fS = ( ( Lexp ) m_Signs ).First(), fI = ( ( Lexp ) m_Intervals ).First();
    !fP.isNull(); fP = fP->m_pNext, fS = fS->m_pNext, fI = fI->m_pNext )
    {
    double Val;
    if( fP->m_Memb.Constan( Val ) )
      Result += ',' + QByteArray::number( Val );
    else
      {
      int Nom, Denom;
      fP->m_Memb.SimpleFrac_( Nom, Denom );
      Result += ',' + QByteArray::number( Nom ) + '/' + QByteArray::number( Denom );
      }
    fS->m_Memb.Boolean_( Sign );
    Result += ',';
    Result += Sign ? '1' : '0';
    fI->m_Memb.Boolean_( Interval );
    Result += ',';
    Result += Interval ? '1' : '0';
    }
  (( Lexp ) m_Intervals).Last()->m_Memb.Boolean_( Interval );
  Result += ',';
  Result += Interval ? '1' : '0';
  return Result + '}';
  }

MathExpr TVect::Clone() const
  {
  TVect *pResult = new TVect( m_Name );
  pResult->m_WasReduced = m_WasReduced;
  return pResult;
  }

bool TVect::Eq( const MathExpr& E2 ) const
  {
  MathExpr _eqName;
  if( E2.Vect( _eqName ) )
    return m_Name.Eq( _eqName );
  return false;
  }

bool TVect::Equal( const MathExpr& E2 ) const
  {
  MathExpr exp, Ar2;
  QByteArray _eqName;
  if( E2.Vect( exp ) )
    return m_Name.Eq( exp );

  if( E2.Variab( _eqName ) && s_ExpStore.Known_var_expr( _eqName, Ar2 ) )
    return Equal( Ar2 );

  if( E2.Funct( _eqName, Ar2 ) && _eqName == "Simplify" )
    return Equal( Ar2 );
  return false;
  }

QByteArray TVect::WriteE() const
  {
  return QByteArray(VectName) + '(' + m_Name.WriteE() + ')';
  }

QByteArray TVect::SWrite() const
  {
  return "\\vector{" + m_Name.SWrite() + '}';
  }

bool TVect::Vect( MathExpr& N ) const
  {
  N = m_Name;
  return true;
  }

TSimpleInterval::TSimpleInterval( const QByteArray& sName, const MathExpr& ex1, const MathExpr& ex2 ) :
  TOper( ex1, ex2 ), m_Brackets( sName )
  {  TExpr::SetReduced(); }

MathExpr TSimpleInterval::Clone() const 
  { 
  return new TSimpleInterval( m_Brackets, m_Operand1.Clone(), m_Operand2.Clone() );
  }

bool TSimpleInterval::Eq( const MathExpr& E2 ) const
  {
  const TSimpleInterval* pSI = CastConst( TSimpleInterval, E2 );
  if( pSI == nullptr ) return false;
  return m_Brackets == pSI->m_Brackets && m_Operand1.Eq( pSI->m_Operand1 ) && m_Operand2.Eq( pSI->m_Operand2 );
  }

bool TSimpleInterval::Equal( const MathExpr& E2 ) const 
  { 
  const TSimpleInterval* pSI = CastConst( TSimpleInterval, E2 );
  if( pSI == nullptr ) return false;
  return m_Brackets == pSI->m_Brackets && m_Operand1.Equal( pSI->m_Operand1 ) && m_Operand2.Equal( pSI->m_Operand2 );
  }

QByteArray TSimpleInterval::WriteE() const 
  { 
  return m_Brackets[0] + m_Operand1.WriteE() + ';' + m_Operand2.WriteE() + m_Brackets[1];
  }

QByteArray TSimpleInterval::SWrite() const
  {
  return "\\interval{" + m_Brackets + "}{" + m_Operand1.SWrite() + "}{" + m_Operand2.SWrite() + '}';
  }

bool TSimpleInterval::SimpleInterval( QByteArray& N, MathExpr& op1, MathExpr& op2 ) const
  {
  N = m_Brackets;
  op1 = m_Operand1;
  op2 = m_Operand2;
  return true;
  }

QByteArray TPolygon::SWrite() const
  {
  QByteArray Result = "\\polygon{";
  PExMemb f;
  for( m_Points.Listex( f ); !f.isNull(); f = f->m_pNext )
    {
    Result += f->m_Memb.WriteE();
    if( !f->m_pNext.isNull() ) Result += ',';
    }
  return Result + '}';
  }

TExprPict::TExprPict( QByteArray APath) : m_Path(APath) {}
TExprPict::~TExprPict() {}

TExprPict::TExprPict(const MathExpr& APath) // : m_Path(CastConstPtr(TCommStr, APath)->Value()) 
  {
  SetReduced();
  const TCommStr *pStr = CastConstPtr(TCommStr, APath);
  if(pStr == nullptr )
    m_Path = APath.WriteE() + ".jpg";
  else
    m_Path = pStr->Value();
  }

bool TExprPict::Eq(const MathExpr& E2) const
  {
  const TExprPict *pPict = CastConst(TExprPict, E2);
  if (pPict == nullptr) return false;
  return m_Path == pPict->m_Path;
  }

bool TExprPict::Equal(const MathExpr& E2) const
  {
  return Eq(E2);
  }

QByteArray TExprPict::WriteE() const
  {
  return "Picture(\"" + m_Path + "\")";
  }

QByteArray TExprPict::SWrite() const
  {
  return "\\picture{" + m_Path + '}';
  }
