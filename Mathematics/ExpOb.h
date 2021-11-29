#ifndef MATH_EXPOBH
#define MATH_EXPOBH

#define DEBUG_TASK

#include "mathematics.h"
#include <qbytearray.h>
#include "../FormulaPainter/FuncName.h"
#include <QSharedPointer>
#include <math.h>
#include <qvector.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qdebug.h>
#include "../FormulaPainter/YClasses.h"
#include "../FormulaPainter/InEdit.h"
#include "Globalvars.h"

#define Cast(Type,Val) dynamic_cast< Type* >( Val )
#define CastConst(Type,Val) dynamic_cast< const Type* >( Val() )
#define CastPtr(Type,Val) dynamic_cast< Type* >( Val.Ptr() )
#define IsType(Type, Val) dynamic_cast< Type* >( Val.Ptr() ) != nullptr
#define IsConstType(Type, Val) dynamic_cast< const Type* >( Val() ) != nullptr
#define CastConstPtr(Type,Val) dynamic_cast< const Type* >( Val() )
#define Ethis (TExpr*) this

class TExMemb;
class TVariable;
class TBool;
class TConstant;
class MathExpr;
class ResultReceiver;
class Parser;
class TUnar;
class TPowr;

//inline double Frac( double V ) { return V - floor( V ); }
typedef QSharedPointer<class TExMemb> PExMemb;
typedef QVector<MathExpr> MathExpArray;
enum Bracketed { brNone, brOperation, brPower };

class TExpr
  {
  friend MathExpr;
  friend TUnar;
  friend TPowr;
  uint m_Counter;
  protected:
    bool m_WasReduced;
  public:
    MATHEMATICS_EXPORT static int sm_CreatedCount;
    MATHEMATICS_EXPORT static int sm_DeletedCount;
    MATHEMATICS_EXPORT static ResultReceiver *sm_pResultReceiver;
    MATHEMATICS_EXPORT static double sm_Accuracy;
    MATHEMATICS_EXPORT static double sm_Precision;
    MATHEMATICS_EXPORT static double Precision();
    MATHEMATICS_EXPORT static double ResetPrecision(double);
    static bool sm_CalcOnly;
    static bool sm_FullReduce;
    static QString sm_LastError;
    MATHEMATICS_EXPORT static QList<TExpr*> sm_CreatedList;
    enum TrigonomSystem { tsRad, tsDeg };
    MATHEMATICS_EXPORT static TrigonomSystem sm_TrigonomSystem;
    static double TriginomValue( double V );
    static double AngleValue( double V );
    static MathExpr GenerateFraction( int N, int D );     
    static int WhatIsIt( const MathExpr& exi, MathExpr& arg1, MathExpr& arg2 ); // Determine type( +, -, *, / ) of expression. 
    static bool CheckDivision( MathExpr Exp, MathExpr& Dividend, MathExpr& Divisor, int& SignOfDivision );
    static MathExpr SummSubtOper( int ssSign, const MathExpr& exi, const MathExpr& exi1, const MathExpr& exi2, bool IsDetails );
    static MathExpr MultOper( const MathExpr& exi, const MathExpr& exi1, const MathExpr& exi2, bool IsDetails );
    static MathExpr DivvOper( const MathExpr& exi, const MathExpr& exi1, const MathExpr& exi2, bool IsDetails );
    static MathExpr CalcMulti( int OperDef, const MathExpr& exi, bool IsDetails );
#ifdef LEAK_DEBUG
#define DEBUG_TASK
//    int m_Number;
//    TExpr() : m_Counter( 0 ), m_WasReduced( false ), m_IsIndexed(false) { sm_CreatedCount++; m_Number = sm_CreatedCount; sm_CreatedList.append( this ); }
//    virtual ~TExpr() { sm_DeletedCount++; sm_CreatedList.removeOne( this ); }
#else
    TExpr() : m_Counter( 0 ), m_WasReduced( false ) {}
#endif
#ifdef DEBUG_TASK
    QByteArray  m_Contents;
#endif
    virtual bool IsTemplate() {return false;}
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual MathExpr Integral( QByteArray d = "x" ) const;
    virtual MathExpr Lim( const QByteArray& v, const MathExpr& lm ) const;

    virtual bool Eq( const MathExpr& E2 ) const { return false; }
    virtual bool Equal( const MathExpr& E2 ) const { return false; }
    virtual bool FindEq( const MathExpr& E, PExMemb & F ) const { return false; }
    virtual bool FindEqual( const MathExpr& E, PExMemb & F ) const { return false; }

    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual bool Replace( const MathExpr& Target, const MathExpr& Source ) { return false; }
    MATHEMATICS_EXPORT virtual MathExpr EVar2EConst();
    
    virtual QByteArray WriteE() const { return ""; }
    virtual QByteArray SWrite() const { return ""; }
    MATHEMATICS_EXPORT virtual QByteArray WriteEB() const;

//    void CalcDraw( TPoint ltp, EditSets AEditSets ) {}
//    void Draw( QPainter& Canvas, EditSets AEditSets ) {}
    virtual bool AsFraction() { return false; }
    virtual bool BoolOper_( const QByteArray& N, MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Boolean_( bool& V ) const { return false; } 
    virtual bool Boolnot_( MathExpr& op ) const { return false; } 
    virtual bool Unarminus( MathExpr& A ) const { return false; }
    virtual bool Unapm_( MathExpr& A ) const { return false; }
    virtual bool Funct( QByteArray& N, MathExpr& A ) const { return false; }
    virtual bool Variab(  QByteArray& N ) const { return false; }
    virtual bool Infinity( bool& N ) { return false; }
    virtual bool Constan( double& V ) const { return false; }
    virtual bool Cons_int( int& I ) const { return false; }
    virtual bool Listex( PExMemb& F ) const { return false; }
    virtual bool Listord( PExMemb& F ) const { return false; }
    virtual bool Oper_( char& N, MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Binar( char N, MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Binar_( uchar &N, MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Indx_( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Summa( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Multp( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Multiplication( MathExpr& op1, MathExpr& op2 ) { return false; }
    virtual bool Subtr( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Divis( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool DiviEvi( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Power( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Root_( MathExpr& op1, MathExpr& op2, int& rt ) const { return false; }
    virtual bool Root1_( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Root1( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool SimpleFrac_( int& N, int& D ) const { return false; }
    virtual bool MixedFrac_( int& I, int& N, int& D ) const { return false; }
    virtual bool List2ex( PExMemb& F ) const { return false; }
    virtual bool Newline() const { return false; }
    virtual bool Splitted() const { return false; }
    virtual bool Negative() const { return false; }
    virtual bool Positive() const { return false; }
    virtual bool Integr_( MathExpr& ex, MathExpr& vr ) const { return false; }
    virtual bool Dfintegr_( MathExpr& exi, MathExpr& exll, MathExpr& exhl, MathExpr& vr ) const { return false; }
    virtual bool Multintegr_( MathExpr& exi, MathExpr& exll, MathExpr& vr ) const { return false; }
    virtual bool Curveintegr_( MathExpr& exi, MathExpr& exll, uchar& Type ) const { return false; }
    virtual bool Gsumma( MathExpr& exs, MathExpr& exll, MathExpr& exhl ) const { return false; }
    virtual bool Limit( MathExpr& ex, MathExpr& exv, MathExpr& exl ) const { return false; }
    virtual bool Gmult( MathExpr& exm, MathExpr& exll, MathExpr& exhl ) const { return false; }
    virtual bool Subst( MathExpr& ex, MathExpr& exll, MathExpr& exhl ) const { return false; }
    virtual bool Deriv( MathExpr& ex, MathExpr& vr ) const { return false; }
    virtual bool Abs_( MathExpr& ex ) const { return false; }
    virtual bool Syst_( MathExpr& ex ) const { return false; }
    virtual bool Vect( MathExpr& N ) const { return false; }
    virtual bool Matr( MathExpr& ex ) const { return false; }
    virtual bool Measur_( MathExpr& ex, MathExpr& exm ) const { return false; }
    virtual bool Detsumm( MathExpr& ex, char& ASign, bool& ACarry, int& ACarryOfs, const QByteArray& ACarryStr ) { return false; }
    virtual bool Detmult( MathExpr& ex, bool& AFinish ) const { return false; }
    virtual bool Detdiv( MathExpr& ex, bool& AFinish ) const { return false; }
    virtual bool Percent( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Degree_( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Log( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Complex( MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool Polygon() const { return false; }
    virtual bool Interval() const { return false; }
    virtual bool Str_() const { return false; }
    virtual bool HasStr() const { return false; }
    virtual bool Trigocomplex() const { return false; }
    virtual bool ConstExpr() const { return false; }
    virtual bool IsLinear() const { return false; }
    virtual bool IsLimit() const { return false; }
    virtual int Compare( const MathExpr& ex ) const { return 0; }
    virtual MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower );
    virtual MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg );
    virtual bool Chart( MathExpr& ex ) const { return false; }
    bool HasExpr( const MathExpr& ex ) const { return false; }
    virtual bool IsFactorized( const QByteArray& Name ) const { return false; }
    virtual bool SimpleInterval( const QByteArray& N, MathExpr& op1, MathExpr& op2 ) const { return false; }
    virtual bool HasComplex() const { return false; }
    virtual bool ImUnit() const { return false;  }
    virtual Bracketed MustBracketed() const { return brNone; }
    virtual void SetReduced( bool Reduced ) { m_WasReduced = Reduced; }
    MathExpr Lconcat( const MathExpr& B ) const;
    MathExpr FactList() const;
    MathExpr ExpandChain( MathExpr&, MathExpr& ) const;
    MathExpr SimpleBinar() const;
    MathExpr UnarMin() const;
    MathExpr UnarPM() const;
    MathExpr SummList() const;
    MathExpr SortMult() const; //  Sorts( alhpabet ) every term in multinominal.
    void DetMults( QList<MathExpr>& List ) const;
    MathExpr SimplBySummList();
    MathExpr CancellationOfMultiNominals( MathExpr& exf );
    MathExpr ReductionPoly( MathExpArray&, const QByteArray& VarName ) const;
    MathExpr ReducePoly() const;
    //MathExpr SolSysInEq();
    MathExpr AlgToGeometr( double Scale, bool PutExpr ) const;
    MathExpr Mixer();
    MathExpr SimplifyInDetail();
    MathExpr Fcase();
    MathExpr ChainToLCD();
    MathExpr GenChainInEq();
    MathExpr RtoDEC();
    MATHEMATICS_EXPORT virtual MathExpr FindGreatestCommDivisor() const;
    MathExpr Dividend();
    MathExpr Divisor();
    MathExpr RandList();
    virtual MathExpr SortList( bool ) const;
    MathExpr FindLeastCommDenom();
    MathExpr ToLeastCommDenom();
    virtual MathExpr FindLeastCommMult();
    bool SetOfFractions( MathExpr& );
    MathExpr ReToMult();
    MathExpr CalcFunc( const QByteArray& fname );
    QByteArray HasUnknown( const QByteArray& = "" ) const;
    void SetReduced() { m_WasReduced = true; }
    virtual bool operator <( const MathExpr& ) { return false; }
    virtual bool HasMatrix() const { return false; }
    virtual bool IsIndexed() const { return false; }   
  };

class Lexp;
class MathExpr
  {
  friend Lexp;
  static bool sm_NewReduce;
  protected:
  TExpr *m_pExpr;
//#ifdef DEBUG_TASK
    QByteArray  m_Contents;
//#endif
  public:
    MathExpr() : m_pExpr( nullptr ) {}
/*
#ifndef DEBUG_TASK
    MathExpr( const MathExpr& E ) : m_pExpr( E.m_pExpr ) { if( m_pExpr != nullptr ) m_pExpr->m_Counter++; }
    MathExpr( TExpr* pE ) : m_pExpr( pE ) 
      { 
      if( m_pExpr != nullptr ) m_pExpr->m_Counter++; 
      }
#else
*/
    MathExpr( const MathExpr& E ) : m_pExpr( E.m_pExpr )
      {
      if( m_pExpr == nullptr ) return;
      m_pExpr->m_Counter++;
      m_Contents = E.m_Contents;
      }
    MathExpr( TExpr* pE ) : m_pExpr( pE )
      {
      if( m_pExpr == nullptr ) return;
      m_pExpr->m_Counter++;
      m_Contents = m_pExpr->WriteE();
      }
//#endif
    MATHEMATICS_EXPORT MathExpr(const QString&);
    virtual ~MathExpr()   
      { 
      if( m_pExpr != nullptr && --m_pExpr->m_Counter == 0 ) 
        delete m_pExpr; 
      }
    MATHEMATICS_EXPORT void TestPtr() const;
    MATHEMATICS_EXPORT static bool sm_NoReduceByCompare;
    MATHEMATICS_EXPORT bool IsEmpty() const { return m_pExpr == nullptr; }
    MATHEMATICS_EXPORT bool IsTemplate() const { return m_pExpr != nullptr && m_pExpr->IsTemplate(); }
    const TExpr* operator ->( ) const { return m_pExpr; }
    const TExpr& operator * ( ) const { return *m_pExpr; }
    const TExpr* operator ()() const { return m_pExpr; }
    TExpr* Ptr() { return m_pExpr; }
    MATHEMATICS_EXPORT MathExpr& operator = ( const MathExpr& );
    class TBinar* Binar();
    void Clear()
      {
      if( m_pExpr != nullptr && --m_pExpr->m_Counter == 0 ) delete m_pExpr;
      m_pExpr = nullptr;
      }
    MathExpr Clone() const { TestPtr(); return m_pExpr->Clone(); }
    MATHEMATICS_EXPORT MathExpr Reduce( bool NewReduce = false ) const;
    MathExpr Perform() const { TestPtr(); return m_pExpr->Perform(); }
    MathExpr Diff( const QByteArray& d = "x" ) { TestPtr(); return m_pExpr->Diff( d ); }
    MathExpr Integral( QByteArray d = "x" ) { TestPtr(); return m_pExpr->Integral( d ); }
    MathExpr Lim( const QByteArray& v, const MathExpr& lm ) const { TestPtr(); return m_pExpr->Lim( v, lm ); }
    MathExpr RetNeg() const;
    bool operator == ( const MathExpr& E2 ) const { return  m_pExpr == E2.m_pExpr; } 
    bool operator != ( const MathExpr& E2 ) const { return  m_pExpr != E2.m_pExpr; }
    MATHEMATICS_EXPORT bool Eq( const MathExpr& E2 ) const;
    MATHEMATICS_EXPORT bool Equal( const MathExpr& E2 ) const;
    MATHEMATICS_EXPORT bool HasStr() const { TestPtr(); return m_pExpr->HasStr(); }
    bool FindEq( const MathExpr& E, PExMemb & F ) { TestPtr(); return m_pExpr->FindEq( E, F ); }
    bool FindEqual( const MathExpr& E, PExMemb & F ) { TestPtr(); return m_pExpr->FindEqual( E, F ); }
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl ) { TestPtr(); return m_pExpr->Substitute( vr, vl ); }
    bool Replace( const MathExpr& Target, const MathExpr& Source );
    MathExpr EVar2EConst() { TestPtr(); return m_pExpr->EVar2EConst(); }
    MATHEMATICS_EXPORT QByteArray WriteE() const { TestPtr(); return m_pExpr->WriteE(); }
    QByteArray SWrite() const { TestPtr(); return m_pExpr->SWrite(); }
    QByteArray WriteEB() const { TestPtr(); return m_pExpr->WriteEB(); }
    bool AsFraction() const { TestPtr(); return m_pExpr->AsFraction(); }
    bool BoolOper_( const QByteArray& N, MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->BoolOper_( N, op1, op2 ); }
    bool Boolean_( bool& V ) const { TestPtr(); return m_pExpr->Boolean_( V ); }
    bool Boolnot_( MathExpr& op ) const { TestPtr(); return m_pExpr->Boolnot_( op ); }
    bool Unarminus( MathExpr& A ) const { TestPtr(); return m_pExpr->Unarminus( A ); }
    bool Unapm_( MathExpr& A ) const { TestPtr(); return m_pExpr->Unapm_( A ); }
    bool Funct( QByteArray& N, MathExpr& A ) const { TestPtr(); return m_pExpr->Funct( N, A ); }
    bool Variab( QByteArray& N ) const { TestPtr(); return m_pExpr->Variab( N ); }
    bool Infinity( bool& N ) const { TestPtr(); return m_pExpr->Infinity( N ); }
    bool Constan( double& V ) const { TestPtr(); return m_pExpr->Constan( V ); }
    bool Cons_int( int& I ) const { TestPtr(); return m_pExpr->Cons_int( I ); }
    bool Listex( PExMemb& F ) const { TestPtr(); return m_pExpr->Listex( F ); }
    bool Listord( PExMemb& F ) const { TestPtr(); return m_pExpr->Listord( F ); }
    bool Oper_( char& N, MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Oper_( N, op1, op2 ); }
    bool Binar( char N, MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Binar( N, op1, op2 ); }
    bool Binar_( uchar& N, MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Binar_( N, op1, op2 ); }
    bool Indx_( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Indx_( op1, op2 ); }
    bool Summa( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Summa( op1, op2 ); }
    bool Multp( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Multp( op1, op2 ); }
    bool Multiplication( MathExpr& op1, MathExpr& op2 )  { TestPtr(); return m_pExpr->Multiplication( op1, op2 ); }
    bool Subtr( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Subtr( op1, op2 ); }
    bool Divis( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Divis( op1, op2 ); }
    bool DiviEvi( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->DiviEvi( op1, op2 ); }
    bool Power( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Power( op1, op2 ); }
    bool Root_( MathExpr& op1, MathExpr& op2, int& rt ) const { TestPtr(); return m_pExpr->Root_( op1, op2, rt ); }
    bool Root1_( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Root1_( op1, op2 ); }
    bool Root1( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Root1( op1, op2 ); }
    bool SimpleFrac_( int& N, int& D ) const { TestPtr(); return m_pExpr->SimpleFrac_( N, D ); }
    bool SimpleFrac_( double& N, double& D ) const;
    bool MixedFrac_( int& I, int& N, int& D ) const { TestPtr(); return m_pExpr->MixedFrac_( I, N, D ); }
    bool List2ex( PExMemb& F ) const { TestPtr(); return m_pExpr->List2ex( F ); }
    bool Newline() const { TestPtr(); return m_pExpr->Newline(); }
    bool Splitted() const { TestPtr(); return m_pExpr->Splitted(); }
    bool Negative() const { TestPtr(); return m_pExpr->Negative(); }
    bool Positive() const { TestPtr(); return m_pExpr->Positive(); }
    bool Integr_( MathExpr& ex, MathExpr& vr ) const { TestPtr(); return m_pExpr->Integr_( ex, vr ); }
    bool Dfintegr_( MathExpr& exi, MathExpr& exll, MathExpr& exhl, MathExpr& vr ) const { TestPtr(); return m_pExpr->Dfintegr_( exi, exll, exhl, vr ); }
    bool Multintegr_( MathExpr& exi, MathExpr& exll, MathExpr& vr ) const { TestPtr(); return m_pExpr->Multintegr_( exi, exll, vr ); }
    bool Curveintegr_( MathExpr& exi, MathExpr& exll, uchar& Type ) const { TestPtr(); return m_pExpr->Curveintegr_( exi, exll, Type ); }
    bool Gsumma( MathExpr& exs, MathExpr& exll, MathExpr& exhl ) const { TestPtr(); return m_pExpr->Gsumma( exs, exll, exhl ); }
    bool Limit( MathExpr& ex, MathExpr& exv, MathExpr& exl )  const { TestPtr(); return m_pExpr->Limit( ex, exv, exl ); }
    bool Gmult( MathExpr& exm, MathExpr& exll, MathExpr& exhl ) const { TestPtr(); return m_pExpr->Gmult( exm, exll, exhl ); }
    bool Subst( MathExpr& ex, MathExpr& exll, MathExpr& exhl ) const { TestPtr(); return m_pExpr->Subst( ex, exll, exhl ); }
    bool Deriv( MathExpr& ex, MathExpr& vr ) const { TestPtr(); return m_pExpr->Deriv( ex, vr ); }
    bool Abs_( MathExpr& ex ) const { TestPtr(); return m_pExpr->Abs_( ex ); }
    bool Syst_( MathExpr& ex ) const { TestPtr(); return m_pExpr->Syst_( ex ); }
    bool Vect( MathExpr& N ) const { TestPtr(); return m_pExpr->Vect( N ); }
    bool Matr( MathExpr& ex ) const { TestPtr(); return m_pExpr->Matr( ex ); }
    bool Measur_( MathExpr& ex, MathExpr& exm ) const { TestPtr(); return m_pExpr->Measur_( ex, exm ); }
    bool Detsumm( MathExpr& ex, char& ASign, bool& ACarry, int& ACarryOfs, const QByteArray& ACarryStr ) 
      {
      TestPtr(); return m_pExpr->Detsumm( ex, ASign, ACarry, ACarryOfs, ACarryStr );
      }
    bool Detmult( MathExpr& ex, bool& AFinish ) { TestPtr(); return m_pExpr->Detmult( ex, AFinish ); }
    bool Detdiv( MathExpr& ex, bool& AFinish ) { TestPtr(); return m_pExpr->Detdiv( ex, AFinish ); }
    bool Percent( MathExpr& op1, MathExpr& op2 ) { TestPtr(); return m_pExpr->Percent( op1, op2 ); }
    bool Degree_( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Degree_( op1, op2 ); }
    bool Log( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Log( op1, op2 ); }
    bool Complex( MathExpr& op1, MathExpr& op2 ) const { TestPtr(); return m_pExpr->Complex( op1, op2 ); }
    bool Polygon() const { TestPtr(); return m_pExpr->Polygon(); }
    bool Interval() { TestPtr(); return m_pExpr->Interval(); }
    bool Str_() const { TestPtr(); return m_pExpr->Str_(); }
    bool Trigocomplex() { TestPtr(); return m_pExpr->Trigocomplex(); }
    bool ConstExpr() const { TestPtr(); return m_pExpr->ConstExpr(); }
    bool IsLinear() const { TestPtr(); return m_pExpr->IsLinear(); }
    int Compare( const MathExpr& ex ) const { TestPtr(); return m_pExpr->Compare( ex ); }
    MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower = nullptr ) { TestPtr(); return m_pExpr->TrigTerm( sName, exArg, exPower ); }
    bool Chart( MathExpr& ex ) const { TestPtr(); return m_pExpr->Chart( ex ); }
    bool HasExpr( const MathExpr& ex ) { TestPtr(); return m_pExpr->HasExpr( ex ); }
    bool IsFactorized( const QByteArray& Name ) const { TestPtr(); return m_pExpr->IsFactorized( Name ); }
    bool SimpleInterval( const QByteArray& N, MathExpr& op1, MathExpr& op2 ) { TestPtr(); return m_pExpr->SimpleInterval( N, op1, op2 ); }
    MATHEMATICS_EXPORT bool IsLimit() { TestPtr(); return m_pExpr->IsLimit(); }
    MATHEMATICS_EXPORT bool HasComplex() const { TestPtr(); return m_pExpr->HasComplex(); }
    MATHEMATICS_EXPORT bool HasMatrix() const { TestPtr(); return m_pExpr->HasMatrix(); }
    bool IsNumericalValue( double &V );
    MATHEMATICS_EXPORT bool IsNumerical() const;
    MathExpr Lconcat( const MathExpr& B ) const { return m_pExpr->Lconcat( B ); }
    MathExpr FactList() const { return m_pExpr->FactList(); }
    MathExpr ExpandChain( MathExpr& Left, MathExpr& Right ) const { return m_pExpr->ExpandChain( Left, Right ); }
    MathExpr SimpleBinar() const  { return  m_pExpr->SimpleBinar(); }
    MathExpr UnarMin() const { TestPtr(); return m_pExpr->UnarMin(); }
    MathExpr UnarPM() const { TestPtr(); return m_pExpr->UnarPM(); }
    MathExpr SummList() const { TestPtr(); return m_pExpr->SummList(); }
    MathExpr SortMult() const { TestPtr(); return m_pExpr->SortMult(); }
    MathExpr SimplBySummList() const { TestPtr(); return m_pExpr->SimplBySummList(); }
    MathExpr ReduceToMult() const;
    MathExpr ReduceTExprs() { return m_pExpr == nullptr ? nullptr : m_pExpr->Reduce(); }
    MathExpr CancellationOfMultiNominals( MathExpr& exf ) { return m_pExpr == nullptr ? nullptr : m_pExpr->CancellationOfMultiNominals(exf); }
    MathExpr ReductionPoly( MathExpArray& Array, const QByteArray& VarName ) const { TestPtr(); return m_pExpr->ReductionPoly( Array, VarName ); }
    MathExpr ReducePoly() const { TestPtr(); return m_pExpr->ReducePoly(); }
//    MathExpr SolSysInEq() { TestPtr(); return m_pExpr->SolSysInEq(); }
    MathExpr AlgToGeometr( double Scale = 1.0, bool PutExpr = true ) const { TestPtr(); return m_pExpr->AlgToGeometr( Scale, PutExpr ); }
    MathExpr Mixer() { TestPtr(); return m_pExpr->Mixer(); }
    MathExpr SimplifyInDetail() { TestPtr(); return m_pExpr->SimplifyInDetail(); }
    MathExpr Fcase() { TestPtr(); return m_pExpr->Fcase(); }
    MathExpr GenChainInEq() { TestPtr(); return m_pExpr->GenChainInEq(); } 
    MathExpr ChainToLCD() { TestPtr(); return m_pExpr->ChainToLCD(); }
    MathExpr RtoDEC() { TestPtr(); return m_pExpr->RtoDEC(); }
    MathExpr ReduceToMultiplicators();
    MATHEMATICS_EXPORT MathExpr Simplify() { TestPtr(); return m_pExpr->Reduce(); }
    MATHEMATICS_EXPORT MathExpr SimplifyFull();
    MathExpr FindGreatestCommDivisor() { TestPtr(); return m_pExpr->FindGreatestCommDivisor(); }
    MathExpr Dividend() { TestPtr(); return m_pExpr->Dividend(); }
    MathExpr Divisor() { TestPtr(); return m_pExpr->Divisor(); }
    MathExpr RandList() { TestPtr(); return m_pExpr->RandList(); }
    MathExpr SortList( bool Order ) const { TestPtr(); return m_pExpr->SortList(Order); }
    MathExpr FindLeastCommDenom() { TestPtr(); return m_pExpr->FindLeastCommDenom(); }
    MathExpr ToLeastCommDenom() { TestPtr(); return m_pExpr->ToLeastCommDenom(); }
    MathExpr FindLeastCommMult() { TestPtr(); return m_pExpr->FindLeastCommMult(); }
    bool WasReduced() const { TestPtr(); return m_pExpr->m_WasReduced; }
    MathExpr& SetReduced() { m_pExpr->m_WasReduced = true; return *this; }
    MathExpr ReToMult() { TestPtr(); return m_pExpr->ReToMult(); }
    MathExpr CalcFunc( const QByteArray& fname ) { TestPtr(); return m_pExpr->CalcFunc( fname ); }
    bool ImUnit() const { TestPtr(); return m_pExpr->ImUnit(); }
    Bracketed MustBracketed() const { return m_pExpr == nullptr ? brNone : m_pExpr->MustBracketed(); }

    bool SetOfFractions( MathExpr& E ) { TestPtr(); return m_pExpr->SetOfFractions(E); }
    QByteArray HasUnknown( const QByteArray& Name = "" ) const { TestPtr(); return m_pExpr->HasUnknown( Name ); }
    bool operator == ( int Value ) const;
    bool operator != ( int Value ) const { return !( ( *this ) == Value ); }
    MathExpr& operator *= ( const MathExpr& E );
    MathExpr& operator /= ( const MathExpr& E );
    MathExpr& operator += ( const MathExpr& E );
    MathExpr& operator -= ( const MathExpr& E );
    MathExpr& operator = ( int Value );
    MathExpr& operator = ( const QByteArray& Value );
    MathExpr operator * ( const MathExpr& E ) const;
    MathExpr operator / ( const MathExpr& E ) const;
    MathExpr operator / ( double V ) const;
    MathExpr operator + ( double V ) const;
    MATHEMATICS_EXPORT MathExpr operator + ( const MathExpr& E ) const;
    MathExpr operator - ( const MathExpr& E ) const;
    MathExpr operator - ( double V ) const;
    MathExpr operator ^ ( const MathExpr& P ) const;
    MathExpr operator ^ ( int P ) const;
    MathExpr Log( const MathExpr& E ) const;
    MathExpr operator - () const;
    MathExpr Root( int R ) const;
    MathExpr Unapm() const;
    MathExpr operator | ( const MathExpr& E ) const;
    MathExpr operator | ( double V ) const;
    MathExpr Abs() const;
    MathExpr Matr() const;
    MathExpr Divisor2Polinoms() const;
    bool operator < (const MathExpr& ) const;
    void MakeBiOrSq( MathExpArray& _RootList, bool IsDuplicate = false );
    MathExpr DetQuaEqu( MathExpArray& List ) const;
    MathExpr DetLinEqu() const;
    bool EquaCh() const;
    void LinEquCh( MathExpr& a, MathExpr& b, bool& check ) const;
    bool LinearCh( int& a ) const;
    void BinomX( MathExpr& a, MathExpr& b, bool& Check, const QByteArray& v = "x" ) const;
    MathExpr LinEqu( MathExpArray& List ) const;
    void QuaEquCh( MathExpr& ea, MathExpr& eb, MathExpr& ec, bool& check ) const;
    void TrinomCh( MathExpr& ea, MathExpr& eb, MathExpr& ec, bool& check, const QByteArray& v = "x" ) const;
    MathExpr SqRootSm() const;
    int CheckAndCheckDivision( MathExpr& exo, MathExpr& limit, Lexp DenomList );
    void SetReduced( bool Reduced ) { TestPtr(); m_pExpr->SetReduced(Reduced); }
    bool IsIndexed() const { return m_pExpr != nullptr && m_pExpr->IsIndexed(); }
  };

class TStr : public TExpr
  {
  protected:
    QByteArray m_Value;
  public:
    MATHEMATICS_EXPORT static bool sm_Server;
    MATHEMATICS_EXPORT static bool sm_Encode;
    MATHEMATICS_EXPORT static QByteArray UnpackValue( const QByteArray& V );
    QByteArray PackValue() const;
    MATHEMATICS_EXPORT TStr( const QByteArray& V );
    MathExpr Clone() const { return new TStr( m_Value ); }
    QByteArray WriteE() const { return '"' + PackValue() + '"'; }
    QByteArray SWrite() const;
    bool Str_() const { return true; }
    virtual bool HasStr() const { return true; }
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const { return Eq(E2); }
    const QByteArray& Value() const { return m_Value; }
  };

class TExMemb
  {
  public:
    static int sm_ExMembCreated;
    static int sm_ExMembDeleted;
    MathExpr m_Memb;
    PExMemb m_pPrev;
    PExMemb m_pNext;
    bool m_Visi;
    bool m_Spacer;
    TExMemb( PExMemb pP, const MathExpr& M ) : m_pPrev( pP ), m_Memb( M ), m_Visi( true ), m_Spacer(false) { sm_ExMembCreated++; }
    ~TExMemb() { sm_ExMembDeleted++; }
  };

class TLexp : public TExpr
  {
  friend class Lexp;
  friend class Parser;
  friend class TFunc;
  friend class TPowr;
  friend class TDeriv;
  friend class TMultIntegral;
  friend class MathExpr;
  friend class TTable;
  uint m_Count;
  protected:
    static bool sm_Bracketed;
    PExMemb m_pFirst;
    PExMemb m_pLast;
  public:
    TLexp() : TExpr(), m_Count(0) {}
    uint Count() const { return m_Count; }
    virtual ~TLexp();
    void Clear();
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual void SetReduced( bool );
    virtual MathExpr Perform() const;
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;
    virtual MathExpr FindGreatestCommDivisor() const;
    virtual bool Listex( PExMemb& F ) const { F = m_pFirst; return true; }
    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual bool Splitted() const;
    virtual bool FindEq( const MathExpr& E, PExMemb& F );
    virtual bool FindEqual( const MathExpr& E, PExMemb& F ) const;
    virtual void Addexp( const MathExpr& A );
    virtual void Appendz( const MathExpr& A );
    virtual void DeleteMemb( PExMemb& M );
    virtual MathExpr FindLeastCommMult();
    const PExMemb First() { return m_pFirst; }
    const PExMemb Last() { return m_pLast; }
    bool AcceptMinus();
    MathExpr SortList( bool Order ) const;
    virtual bool ConstExpr() const;
    bool Syst_(MathExpr& ex ) const { ex = MathExpr( (TExpr*) this); return false; }
  };

class Lexp : public MathExpr
  {
  friend class Parser;
  public:
    Lexp() : MathExpr() {}
    PExMemb& First();
    PExMemb& Last();
    Lexp( const MathExpr& E ) : MathExpr( E ) {}
    Lexp( TExpr *pE ) : MathExpr( pE ) {}
    bool FindEq( const MathExpr& E, PExMemb& F );
    bool FindEqual( const MathExpr& E, PExMemb& F );
    void Addexp( const MathExpr& A );
    void Appendz( const MathExpr& A );
    void DeleteMemb( PExMemb& M );
    Lexp LeastCommonDenominator( bool bReduce );
    int Count();
//    MathExpr CreateObject();
  };

class TLord : public TLexp
  {
  friend class TMatr;
  public:
  TLord() : TLexp() {}
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;
    virtual bool Equal( const MathExpr& E2 ) const;
    virtual bool Listex( PExMemb& F ) { F.clear(); return false; }
    virtual bool Listord( PExMemb& F ) const { F = m_pFirst; return true; }
  };

class TL2exp : public TLexp
  {
  friend class TMatr;
  public:
    TL2exp() : TLexp() {}
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual bool List2ex( PExMemb& F ) const;
  };

class TVariable : public TExpr
  {
  bool m_Meta_sign;
  QByteArray m_Name;
  static const int mc_IndCount = 24;
  char m_Ind[mc_IndCount];    // { atributes: -1 subscript, +1 - overscript 2 - mean}
  bool m_IsIndexed;
  public:
    TVariable() : m_Meta_sign( false ), m_IsIndexed(false) { SetReduced(); memset( m_Ind, 0, sizeof( m_Ind ) ); }
    MATHEMATICS_EXPORT TVariable( bool Meta, const QByteArray& Name );
    TVariable( bool Meta, uchar Name ) : TVariable() { m_Meta_sign = Meta; m_Name = QByteArray(1, Name); }
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;
    virtual MathExpr Diff( const QByteArray& d = "x" );
    virtual MathExpr Integral( const QByteArray& d = "x" );
    virtual MathExpr Lim( const QByteArray& v, const MathExpr& lm ) const;

    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const; // не реализована

    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    MATHEMATICS_EXPORT virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual bool Variab( QByteArray& N ) const;
    virtual bool IsLinear() const;
    virtual bool IsFactorized( const QByteArray& Name ) const;
    QByteArray Name() const { return m_Name;  }
    bool ConstExpr() const;
    bool IsIndexed() const { return m_IsIndexed; }
  };

class TInfinity : public TExpr
  {
  bool m_Neg;
  public:
    TInfinity( bool ANeg ) : m_Neg( ANeg ) { SetReduced(); }
    virtual MathExpr Clone() const { return new TInfinity( m_Neg ); }
    virtual MathExpr Diff( const QByteArray& d = "x" ) { s_GlobalInvalid = true; return Clone(); }
    virtual bool Eq( const MathExpr& exp ) const { bool  ng;  return exp.Infinity( ng ) && ( m_Neg == ng ); }
    virtual bool Equal( const MathExpr& exp ) const { return Eq(exp); }
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual bool Infinity( bool& ANeg ) { ANeg = m_Neg; return true; }
    virtual bool Negative() const { return m_Neg; }
  };

class TConstant : public TExpr
  {
  friend class TDivi;
  friend class DxValue;
  double m_Value;
  double m_Precision;
  bool m_IsE;
  public:
    static bool sm_ConstToFraction;
    TConstant() : TExpr(), m_Value( 0.0 ), m_Precision( 0.0 ), m_IsE( false ) {}
    MATHEMATICS_EXPORT TConstant( double V, bool IsE = false );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" );
    MathExpr Lim( const QByteArray& v, const MathExpr lm ) const;

    bool Eq( const MathExpr& E2 ) const;  //проверить завершение сравнений
    bool Equal( const MathExpr& E2 ) const; //проверить завершение сравнений
    MATHEMATICS_EXPORT QByteArray WriteE() const;
    virtual QByteArray SWrite() const { return WriteE(); }

    bool Constan( double& V ) const;
    bool Cons_int( int& I ) const;
    bool Negative() const;
    bool ConstExpr() const;
    bool IsLinear() const;

    int Compare( const MathExpr& ex ) const;
    bool Positive() const;
    double Value() { return m_Value; }
    double Precision() { return m_Precision; }
    bool IsLimit() { return m_IsE; }
  };

class TSimpleFrac : public TExpr
  {
  friend class TMixedFrac;
  int m_Nom;
  int m_Denom;
  public:
    TSimpleFrac( int N, int D );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" );
    MathExpr Lim( const QByteArray& v, const MathExpr lm ) const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    bool AsFraction();
    bool SimpleFrac_( int& N, int& D ) const;
    bool Unarminus( MathExpr& A ) const;
    bool Negative() const;
    bool ConstExpr() const;
    bool IsLinear() const;
    int Compare( const MathExpr& ex ) const;
    bool Positive() const;
    virtual Bracketed MustBracketed() const;
  };

class TFunc : public TExpr
  {
  bool m_Meta_sign;
  QByteArray m_Name;
  QByteArray m_ShortName;
  MathExpr m_Arg;
  MathExpr m_LogBase;
  public:
    TFunc( bool Meta, const QByteArray& Name, const MathExpr& Arg );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" ) { return nullptr; }
    MathExpr Lim( const QByteArray& v, const MathExpr lm ) const { return nullptr; }
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    bool Replace( const MathExpr& Target, const MathExpr& Source );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Funct( QByteArray& N, MathExpr& A ) const;
    bool Log( MathExpr& op1, MathExpr& op2 ) const;
    bool Splitted() const;
    bool ConstExpr() const;
  MathExpr TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower = nullptr );
  bool Positive() const;
  bool HasComplex() const;
  void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Arg.SetReduced( Reduced ); }
  };

class TUnar : public TExpr
  {
  MathExpr m_Coeff;
  MathExpr m_Term;                        
  MathExpr m_Arg;
  public:
    TUnar( const MathExpr& Arg );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" );
    MathExpr Lim( const QByteArray& v, const MathExpr& lm ) const;

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    bool Replace( const MathExpr& Target, const MathExpr& Source );
    QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    bool Unarminus( MathExpr& A ) const;
    bool Splitted() const;
    bool Negative() const;
    bool ConstExpr() const;
    bool IsLinear() const;
    bool Cons_int( int& I ) const;
    bool Multiplication( MathExpr& op1, MathExpr& op2 );
    MathExpr TrigTerm( const QByteArray& Name, const MathExpr& exArg, const MathExpr& exPower = nullptr );
    bool HasComplex() const;
    bool HasMatrix() const;
    void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Arg.SetReduced( Reduced ); }
  };

class TOper : public TExpr
  {
  friend TL2exp;
  friend class TDivi;
  friend class TChart;
  protected:
    static bool sm_InsideChart;
    char m_Name;
    MathExpr m_Operand1;
    MathExpr m_Operand2;
  public:
    TOper() : TExpr() {}
    TOper( const MathExpr& ex1, const MathExpr& ex2 ) : TExpr(), m_Operand1( ex1 ), m_Operand2( ex2 ), m_Name(0) {}
    TOper( const MathExpr& ex1, const MathExpr& ex2, char Name ) : TExpr(), m_Operand1( ex1 ), m_Operand2( ex2 ), m_Name(Name) {}
    MATHEMATICS_EXPORT virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;

    bool Oper_( char& N, MathExpr& op1, MathExpr& op2 ) const;
    virtual bool Replace( const MathExpr& Target, const MathExpr& Source );
    virtual bool Splitted() const;
    virtual bool Negative() const;
    virtual bool ConstExpr() const;
    virtual bool IsFactorized( const QByteArray& Name ) const;
    virtual bool HasComplex() const;
    virtual bool HasMatrix() const;
    MathExpr& Left() { return m_Operand1; }
    MathExpr& Right() { return m_Operand2; }
    void SetName( char Name ) { m_Name = Name; }
    void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Operand1.SetReduced( Reduced ); m_Operand2.SetReduced( Reduced ); }
  };

class TIndx : public TOper
  {
  public:
    TIndx( const MathExpr& Ex1, const MathExpr& Ex2 );

    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    QByteArray WriteE() const;
    QByteArray SWrite() const;
//    void To_keyl( XPInEdit& Ed );

    bool Indx_( MathExpr& op1, MathExpr& op2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
  };

class TPowr : public TOper
  {
  public:
    static bool sm_Factorize;
    TPowr() : TOper() {}
    TPowr( const MathExpr& Ex1, const MathExpr& Ex2 );
    TPowr( const MathExpr& base, int Power );
    TPowr( const MathExpr& base, int Nom, int Denom );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" ) { return nullptr; }
    MathExpr Lim( const QByteArray& v, const MathExpr& lm ) const { return nullptr; }

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
 //   void To_keyl( XPInEdit& Ed );

    QByteArray WriteE() const;
    virtual QByteArray SWrite() const;

    bool Power( MathExpr& op1, MathExpr& op2 ) const;
    bool Negative() const;
    bool IsLinear() const;
    MathExpr TrigTerm( const QByteArray& Name, const MathExpr& exArg, const MathExpr& exPower = nullptr );
    bool EqualBase( const MathExpr& ex );
    bool Replace( const MathExpr& Target, const MathExpr& Source );
    bool Root_( MathExpr& op1, MathExpr& op2, int& rt ) const;
    bool Positive() const;
  };

class TRoot :public  TPowr
  {
  int m_Root;
  public:
    TRoot( const MathExpr& base, int Root );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Diff( const QByteArray& d = "x" );
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
//    void To_keyl( XPInEdit& Ed );
    bool Root_( MathExpr& op1, MathExpr& op2, int& rt ) const;
    bool Positive() const;
  };

class TRoot1 : public TPowr
  {
  public:
    TRoot1( const MathExpr& Ex1, const MathExpr& Ex2 );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    virtual QByteArray SWrite() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;

    bool Root1_( MathExpr& op1, MathExpr& op2 ) const;
    bool Root1( MathExpr& op1, MathExpr& op2 ) const;
  };

class TMixedFrac: public TSimpleFrac
  {
  int m_IntPart;
  int m_NomPart;
  public:
    TMixedFrac( int I, int N, int D );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    QByteArray WriteE() const;
    QByteArray SWrite() const;
//    void To_keyl( XPInEdit& Ed );

    bool MixedFrac_( int& I, int& N, int& D ) const;
    bool Negative() const;
    bool ConstExpr() const;
    bool IsLinear() const;
    bool Unarminus( MathExpr& A ) const;
  };

class TNewLin : public TExpr
  {
  public:
    MathExpr Clone() const { return nullptr; }
    MathExpr Diff(const QByteArray& d = "x" ) { return nullptr; }

    bool Eq( const MathExpr& E2 ) const { return E2.Newline(); }

    QByteArray WriteE() const { return QByteArray( 1, msCharNewLine); }
    QByteArray SWrite() const { return "\\newline\n"; }

    bool Newline() const { return true; }
    bool Splitted() const { return true; }
  };

class TIntegral : public TExpr
  {
  friend class TDefIntegral;
  friend class TMultIntegral;
  friend class TCurveIntegral;
  MathExpr m_Expint;
  MathExpr m_Varint;
  bool m_Meta_sign;
  public:
    TIntegral() : TExpr(), m_Meta_sign( false ) {}
    TIntegral( bool Meta, const MathExpr& ex, const MathExpr& Var ) : TExpr(), m_Meta_sign( Meta ), m_Expint( ex ), m_Varint(Var) {}
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;
    virtual MathExpr CalcIntegralExpr() const { return Clone(); }
    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;

    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual bool Replace( const MathExpr& Target, const MathExpr& Source );
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;

    virtual bool Splitted() const { return false; }
    virtual bool Integr_( MathExpr& ex, MathExpr& vr ) const { ex = m_Expint; vr = m_Varint; return true; }
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Expint.SetReduced( Reduced ); m_Varint.SetReduced( Reduced ); }
  };

class TDefIntegral : public TIntegral
  {
  MathExpr m_Lolimit;
  MathExpr m_Hilimit;
  public:
    TDefIntegral( bool Meta, const MathExpr& exi, const MathExpr& exll, const MathExpr& exhl, const MathExpr& Var );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;

    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;

    virtual MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    bool Dfintegr_( MathExpr& exi, MathExpr& exll, MathExpr& exhl, MathExpr& vr ) const;
    void SetReduced( bool Reduced ) { TIntegral::SetReduced( Reduced ); m_Lolimit.SetReduced( Reduced ); m_Hilimit.SetReduced( Reduced ); }
  };

class TMultIntegral : public TIntegral
  {
  MathExpr m_Region;
  TMultIntegral( bool Meta, const MathExpr& exi, const MathExpr& exll, const MathExpr& vr );
  public:
    TMultIntegral( bool Meta, const MathExpr& exi, const MathExpr& exll, const MathExpr& vr, const MathExpr& vr2,
      const MathExpr& vr3 = MathExpr() );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;

    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;

    virtual MathExpr Substitute( const QByteArray &vr, const MathExpr& vl );
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual bool Multintegr_( MathExpr& exi, MathExpr& exll, MathExpr& vr ) const;
    void SetReduced( bool Reduced ) { TIntegral::SetReduced( Reduced ); m_Region.SetReduced( Reduced ); }
  };

class TCurveIntegral : public TIntegral
  {
  uchar m_Type;
  public:
    TCurveIntegral( bool Meta, uchar Type, const MathExpr& exi, const MathExpr& exll );
    virtual MathExpr Clone() const;
    virtual MathExpr Reduce() const;
    virtual MathExpr Perform() const;

    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;

    virtual MathExpr Substitute( const QByteArray &vr, const MathExpr& vl );
    virtual QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    virtual bool Curveintegr_( MathExpr& exi, MathExpr& exll, uchar& Type ) const;
  };

class TGSumm : public TExpr
  {
  MathExpr m_Expsum;
  MathExpr m_Lolimit;
  MathExpr m_Hilimit;
  bool m_Meta_sign;
  public:
    TGSumm( bool Meta, const MathExpr& exs, const MathExpr& exll, const MathExpr& exhl );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    virtual QByteArray SWrite() const;

    bool Splitted() const;
    bool Gsumma( MathExpr& exs, MathExpr& exll, MathExpr& exhl ) const;
    void SetReduced( bool Reduced )
      {
      TExpr::SetReduced( Reduced ); 
      m_Lolimit.SetReduced( Reduced ); 
      m_Hilimit.SetReduced( Reduced );
      m_Expsum.SetReduced( Reduced );
      }
  };

class TLimit : public TExpr
  {
  MathExpr m_Exp;
  MathExpr m_Varlimit;
  MathExpr m_Explimit;
  bool m_Meta_sign;
  public:
    TLimit( bool Meta, const MathExpr& exs, const MathExpr& exv, const MathExpr& exhl ) : TExpr(),
      m_Meta_sign( Meta ), m_Exp( exs ), m_Varlimit( exv ), m_Explimit( exhl )
      {
#ifdef DEBUG_TASK
      m_Contents = WriteE();
#endif
      }
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    virtual QByteArray SWrite() const;
    bool Splitted() const { return m_Exp.Splitted(); }
    bool Limit( MathExpr& ex, MathExpr& exv, MathExpr& exl ) const;
    void SetReduced( bool Reduced )
      {
      TExpr::SetReduced( Reduced );
      m_Varlimit.SetReduced( Reduced );
      m_Explimit.SetReduced( Reduced );
      m_Exp.SetReduced( Reduced );
      }
  };

class TLog : public TExpr
  {
  MathExpr m_Basis;
  MathExpr m_Arg;
  public:
    TLog( const MathExpr& ex1, const MathExpr& ex2 );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" );

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    bool Replace( const MathExpr& Target, const MathExpr& Source );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Log( MathExpr& ex1, MathExpr& ex2 ) const;
    bool ConstExpr() const { return m_Basis.ConstExpr() && m_Arg.ConstExpr(); }
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Basis.SetReduced( Reduced ); m_Arg.SetReduced( Reduced ); }
  };

class TGMult : public TExpr
  {
  MathExpr m_Expmul;
  MathExpr m_Lolimit;
  MathExpr m_Hilimit;
  bool m_Meta_sign;
  public:
    TGMult( bool Meta, const MathExpr& exm, const MathExpr& exll, const MathExpr& exhl );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl ) { return nullptr; }
    QByteArray WriteE() const { return ""; }
//    void To_keyl( XPInEdit& Ed ) {}

    bool Splitted() const { return false; }
    bool Gmult( MathExpr& exm, MathExpr& exll, MathExpr& exhl ) { return false; }
    void SetReduced( bool Reduced )
      {
      TExpr::SetReduced( Reduced );
      m_Lolimit.SetReduced( Reduced );
      m_Hilimit.SetReduced( Reduced );
      m_Expmul.SetReduced( Reduced );
      }
  };

class TSubst : public TExpr
  {
  MathExpr m_Exp;
  MathExpr m_Lolimit;
  MathExpr m_Hilimit;
  bool m_Meta_sign;
  public:
    TSubst( bool Meta, const MathExpr& ex, const MathExpr& exll, const MathExpr& exhl );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;

    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;

    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Splitted() const;
    bool Subst( MathExpr& ex, MathExpr& exll, MathExpr& exhl ) const;
    void SetReduced( bool Reduced )
      {
      TExpr::SetReduced( Reduced );
      m_Lolimit.SetReduced( Reduced );
      m_Hilimit.SetReduced( Reduced );
      m_Exp.SetReduced( Reduced );
      }
  };

class TDeriv : public TExpr
  {
  MathExpr m_Expdif;
  MathExpr m_Vardif;
  MathExpr m_DerivAsStroke;
  MathExpr m_Partial;
  public:
    bool m_HasParenthesis;
    TDeriv( const MathExpr& Stroke, const MathExpr& Part, const MathExpr& ex, const MathExpr& vr );
    TDeriv( MathExpr& );
    MathExpr Clone() const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Integral( const QByteArray& d = "x" );
    MathExpr Reduce() const;
    MathExpr Perform() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Deriv( MathExpr& ex, MathExpr& vr ) const;
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Expdif.SetReduced( Reduced ); m_Vardif.SetReduced( Reduced ); }
  };

class TAbs : public TExpr
  {
  MathExpr m_Exp;
  bool m_Meta_sign;
  public:
    TAbs( bool Meta, const MathExpr& ex );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Lim( const QByteArray& v, const MathExpr& lm ) const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Abs_( MathExpr& ex ) const;
    bool ConstExpr() const;
    bool Replace( const MathExpr& Target, const MathExpr& Source );
    bool Positive() const { return true; }
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Exp.SetReduced( Reduced ); }
    bool HasMatrix() const;
  };

class TSyst : public TExpr
  {
  MathExpr m_Exp;
  public:
    TSyst( const MathExpr& ex );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Syst_( MathExpr& ex ) const;
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Exp.SetReduced( Reduced ); }
    virtual bool ConstExpr() const;
  };

class TVect : public TExpr
  {
  MathExpr m_Name;
  public:
    TVect( const MathExpr& Name ) : m_Name(Name) {}
    MathExpr Clone() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Vect( MathExpr& N ) const;
  };

class TInterval : public TExpr //This object stores the information on acceptablis intervals
  {
  double m_Ax;       //{Coordinate of the left point of an axis}
  double m_Len; //      {Length of an axis}
  Lexp m_Points; //     {Coordinates of critical points}
  Lexp m_Signs; //     {Belonging of points to intervals}
  Lexp m_Intervals; //     {Acceptablis intervals}
  public:
    TInterval( double a, double L, const Lexp& P, const Lexp& S, const Lexp& I );
    MathExpr Clone() const;
    QByteArray SWrite() const;
    bool Interval() const { return true; }
  };

class TPolygon : public TExpr
  {
  MathExpr m_Points;
  int m_Kind;
  public:
    TPolygon( const MathExpr& APoints, int Akind = 0 ) : TExpr(), m_Points( APoints ), m_Kind( Akind) {}
    MathExpr Clone() const { return new TPolygon( m_Points, m_Kind ); }
    bool Polygon() const { return true; }
    const MathExpr Points() { return m_Points; }
    QByteArray SWrite() const;
  };

class TComplexExpr : public TExpr
  {
  friend Parser;
  friend MathExpr;
  MathExpr m_Re;
  MathExpr m_Im;
  public:
    TComplexExpr( const MathExpr& ex1, const MathExpr& ex2 );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
//    void To_keyl( XPInEdit& Ed );
    bool Complex( MathExpr& op1, MathExpr& op2 ) const;
    bool HasComplex() const;
    bool ImUnit() const;
    bool Cons_int( int& I ) const;
    virtual Bracketed MustBracketed() const;
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Re.SetReduced( Reduced ); m_Im.SetReduced( Reduced ); }
    bool ConstExpr() const;
  };

class TTrigoComplex : public TExpr
  {
  double m_Re;
  double m_Im;
  double m_Scale;
  public:
    TTrigoComplex( double a, double b, double aScale = 1 );
    MathExpr Clone() const;
    bool Trigocomplex() { return true; }
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Equal( const MathExpr& E2 ) const;
    bool Eq( const MathExpr& E2 ) const;
  };

typedef QVector< QVector< MathExpr > > MatrixArry;

class TMatr : public TExpr
  {
  friend Parser;
  protected:
  MatrixArry m_A;
  int m_RowCount;
  int m_ColCount;
  int m_Rank;
  bool m_IsNumerical;
  MathExpr m_Exp;
  public:
    bool m_IsVisible;
    static int sm_RecursionDepth;
    TMatr() : TExpr(), m_RowCount( 0 ), m_ColCount( 0 ), m_Rank( 0 ), m_IsVisible( true ), m_IsNumerical( true ) {}
    TMatr( const MathExpr& ex );
    TMatr( const MatrixArry& matr );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    virtual bool Eq( const MathExpr& E2 ) const;
    virtual bool Equal( const MathExpr& E2 ) const;
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Matr( MathExpr& ex ) const;
    MathExpr Mult( const MathExpr& exp ) const;
    MathExpr Add( const MathExpr& exp ) const;
    MathExpr Subt( const MathExpr& exp ) const;
    MathExpr Inversion() const;
    MathExpr Transpose() const;
    MathExpr Determinant() const;
    int RowCount() const { return m_RowCount; }
    int ColCount() const { return m_ColCount; }
    bool IsSquare() const { return m_RowCount == m_ColCount; }
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Exp.SetReduced( Reduced ); }
    virtual bool ConstExpr() const { return m_IsNumerical; }
    bool HasMatrix() const { return true; }
    virtual bool IsTemplate();
  };

class TTable : public TMatr
  {
  friend Parser;
  TableGrid m_GridState;
  bool m_NoFreeze;
  QVector<int> m_UnvisibleColumns;
  public:
    TTable( const MathExpr& ex );
    QByteArray SWrite() const;
    MathExpr Clone() const;
    QByteArray WriteE() const;
    bool HasStr() const { return true; }
    void SetGridState( TableGrid State ) { m_GridState = State; }
    virtual bool IsTemplate();
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
  };

class TMeaExpr : public TOper
  {
  public:
    TMeaExpr( const MathExpr& ex1, const MathExpr& ex2 ) : TOper( ex1, ex2, '`' ) {}
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Measur_( MathExpr& ex, MathExpr& exm ) const;
    virtual bool ConstExpr() const;
  };

class TDegExpr : public TOper
  {
  public:
    TDegExpr( const QByteArray& Value );
    TDegExpr( const MathExpr& ex1, const MathExpr& ex2 );
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Diff( const QByteArray& d = "x" );
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Degree_( MathExpr& op1, MathExpr& op2 ) const;
    bool Measur_( MathExpr& op1, MathExpr& op2 ) const;
  };

class TUnapm : public TExpr
  {
  MathExpr m_Arg;
  public:
    TUnapm( const MathExpr& A ) : TExpr(), m_Arg(A) {}
    MathExpr Clone() const;
    MathExpr Reduce() const;
    MathExpr Perform() const;
    MathExpr Diff( const QByteArray& d = "x" );
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    MathExpr Substitute( const QByteArray& vr, const MathExpr& vl );
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Unapm_( MathExpr& A ) const;
    bool Splitted() const { return m_Arg.Splitted(); }
    bool Negative() const { return true; } 
    virtual void SetReduced( bool Reduced ) { TExpr::SetReduced( Reduced ); m_Arg.SetReduced( Reduced ); }
  };

class TBool : public TExpr
  {
  bool m_Valueb;
  public:
    TBool( bool V );
    MathExpr Clone() const;
    MathExpr Diff( const QByteArray& d = "x" );
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Boolean_( bool& V ) const;
  };

class TExprPict : public TExpr
  {
  QByteArray m_Path;
  public:
    TExprPict(const MathExpr& APath);
    MATHEMATICS_EXPORT TExprPict( QByteArray APath);
    MATHEMATICS_EXPORT virtual ~TExprPict();
    MathExpr Clone() const { return new TExprPict(m_Path); }
    bool Eq(const MathExpr& E2) const;
    bool Equal(const MathExpr& E2) const;
    QByteArray WriteE() const;
    MATHEMATICS_EXPORT QByteArray SWrite() const;
  };

class TChart : public TExpr
  {
  MatrixArry m_A;
  int m_N;
  double m_Scale;
  MathExpr m_Exp;
  QByteArray m_LabelX, m_LabelY, m_NameX, m_NameY, m_Type;
  bool m_FromTemplate;
  public:
    TChart( const MathExpr& ex );
    MathExpr Clone() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    bool HasStr() const { return true; }
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool Chart( MathExpr& ex ) const;
  };

class TCommStr : public TStr
  {
  public:
    TCommStr( const QByteArray& V ) : TStr( V ) {}
    TCommStr( const QString& S ) : TStr( FromLang( S ) ) {}
    MathExpr Clone() const { return new TCommStr( m_Value ); }
  };

class TSimpleInterval : public TOper
  {
  public:
    QByteArray m_Brackets;
    TSimpleInterval( const QByteArray& sName, const MathExpr& ex1, const MathExpr& ex2 );
    MathExpr Clone() const;
    bool Eq( const MathExpr& E2 ) const;
    bool Equal( const MathExpr& E2 ) const;
    QByteArray WriteE() const;
    QByteArray SWrite() const;
    bool SimpleInterval( QByteArray& N, MathExpr& op1, MathExpr& op2 ) const;
  };

typedef QVector< QVector< bool > > ConjugationTable;

class TMathGraph : public TExpr
  {
  ConjugationTable m_ConjugationTable;
  QBitmap m_Picture;
  QByteArrayList m_Labels;
  QByteArray m_Source;
  public:
    TMathGraph( ConjugationTable& Table, QBitmap& Bmp, QByteArrayList& ALabels, QByteArray& ASource ) {  SetReduced(); }
    MathExpr Clone() { return nullptr; }
    bool Eq( const MathExpr& E2 ) const { return false; }
    bool Equal( const MathExpr& E2 ) const { return false; }
    QByteArray WriteE() { return ""; }
//    void To_keyl( XPInEdit& Ed ) {}
    QByteArray Source() const { return m_Source; }
  };

inline MathExpr::MathExpr(const QString& S) : MathExpr( new TCommStr(S) ) { }
inline MathExpr TExpr::ReducePoly() const { MathExpArray A; return  ReductionPoly( A, "x" ); }
inline MathExpr Power( const QByteArray& Name, int V ) { return new TPowr( new TVariable( false, Name ), new TConstant( V ) ); }
inline MathExpr MathExpr::operator ^ ( int P ) const { return new TPowr( m_pExpr, new TConstant( P ) ); }
inline MathExpr MathExpr::operator ^ ( const MathExpr& E ) const { return new TPowr( m_pExpr, E ); }
inline MathExpr MathExpr::Log( const MathExpr& E ) const { return new TLog( m_pExpr, E ); }
inline MathExpr MathExpr::Root( int Root ) const { return new TRoot( m_pExpr, Root ); }
inline MathExpr MathExpr::operator - ( ) const { return new TUnar( m_pExpr ); }
inline MathExpr MathExpr::Unapm() const { return new TUnapm( m_pExpr ); }
inline MathExpr MathExpr::Abs() const { return new TAbs( false, m_pExpr ); }
inline MathExpr MathExpr::Matr() const { return new TMatr( m_pExpr ); }
inline MathExpr Constant( double V ) { return new TConstant( V ); }
inline MathExpr MathExpr::RetNeg() const { return Constant( -1 ) * ( *this ); }
inline MathExpr& MathExpr::operator = ( int Value ) { return *( this ) = Constant( ( double ) Value ); }
inline MathExpr& MathExpr::operator = ( const QByteArray& Value ) { return *( this ) = new TStr( Value ); }
inline MathExpr Function( const QByteArray& Name, const MathExpr& E ) { return new TFunc( false, Name, E ); }
inline MathExpr Variable( const QByteArray& Name, bool Meta = false ) { return new TVariable( Meta, Name ); }
inline MathExpr Variable( uchar Name, bool Meta = false ) { return new TVariable( Meta, Name ); }
MathExpr CreateComplex( const MathExpr& Re, const MathExpr& Im );
inline MathExpr CreateComplex( double Re, double Im ) { return new TComplexExpr( new TConstant( Re ), new TConstant( Im ) ); }
inline MathExpr TExpr::TrigTerm( const QByteArray& sName, const MathExpr& exArg ) { return TrigTerm( sName, exArg, MathExpr() ); }
inline MathExpr TExpr::Clone() const { return nullptr; }
inline MathExpr TExpr::Reduce() const { return Clone(); }
inline MathExpr TExpr::Perform() const { return Clone(); }
inline MathExpr TExpr::Diff( const QByteArray& ) { return nullptr; }
inline MathExpr TExpr::Integral( QByteArray d ) const { return nullptr; }
inline MathExpr TExpr::Lim( const QByteArray& v, const MathExpr& lm ) const { return nullptr; }
inline MathExpr TExpr::Substitute( const QByteArray& vr, const MathExpr& vl ) { return this; }
inline MathExpr TExpr::TrigTerm( const QByteArray& sName, const MathExpr& exArg, const MathExpr& exPower ) { return nullptr; }
//inline MathExpr TExpr::SolSysInEq() { return this; }
inline MathExpr TExpr::SimplifyInDetail() { return this; }
inline MathExpr TExpr::RandList() { return this; }
inline MathExpr TExpr::FindLeastCommMult() { s_GlobalInvalid = true; return this; }
inline MathExpr TExpr::SortList( bool ) const { return Clone(); }

bool CheckInputOfEquationsSystem( const MathExpr&, MathExpArray& List, bool MakeFull, Lexp DenomList = MathExpr() );
MathExpr CalcR( MathExpr divisor, MathExpr dividend, Lexp& limits );

#endif
