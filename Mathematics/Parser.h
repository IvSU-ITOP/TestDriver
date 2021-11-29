#ifndef MATH_PARSER
#define MATH_PARSER

#include "../FormulaPainter/XPTypes.h"
#include "Globalvars.h"
#include "../FormulaPainter/XPSymbol.h"
#include <QStaticText>
#include "ExpOb.h"

typedef struct TNode* PNode;

MATHEMATICS_EXPORT bool IsListExpression( const QByteArray& Text );
bool NIsConst( PNode Expr );

struct TNode
  {
  QByteArray m_Info;
  uchar m_OpSign;
  uchar m_Priority;
  bool m_IsLeft, m_IsRight;
  PNode m_pPrev;
  PNode m_pParent;
  PNode m_pLeft, m_pRight;
  class Parser* m_pOwner;
  TNode( Parser *pOwner, const PNode Parent = nullptr );
  ~TNode();
  };

enum ParserErr { peSyntacs, peName, peNullptr, peNoOpnBrckt, peNoClsBrckt,
  peDegree, peMultipl, peOpertn, peFunctn, peNoSolv, peNoSolvType, peCanNotExpand, peUnaptMethod,  peNewErr, peInfinity };

class Parser
  {
  friend TNode;
  friend TExpr;
  static const int toAnd = 1,    // AND
    toOr = 2,    // OR 
    toXor = 3,    // XOR 
    toNot = 4,    // NOT 
    toLeftParenthesis = 5,    // ( 
    toRightParenthesis = 6,    // ) 
    toPlus = 7,    // +
    toMinus = 8,    // -
    toMultiply = 9,    // *
    toDivide = 10,    // / 
    toSemicolon = 11,    //; 
    toEqual = 12,    // = 
    toComma = 13,    //, 
    toEndOfText = 14,    // #0 
    toLeftBracket = 15,    // [
    toRightBracket = 16,    //] 
    toPower = 17,   //  ^ 
    toTwoPoints = 18,    // .. 
    toPoint = 19,    // . 
    toColon = 20,    // : 
    toBinary = 21,    // <= 
    toLessOrEqual = 21,    // <= 
    toNoEqual = 22,    // <> 
    toLess = 23,    // < 
    toGreaterOrEqual = 24,    // >= 
    toGreater = 25,    // > 
    toInteger = 26,    // Integer 
    toFloat = 27,    // Float 
    toIdentifier = 28,    // Identifier 
    toRoot = 29,    //// ~
    toFunction = 30,    // Function 
    toDegree = 31,    // Degree 
    toMinute = 32,    // Minute 
    toLeftCurly = 33,    // {
    toRightCurly = 34,    // }
    toApostrophe = 35,    // ' 
    toBackApostrophe = 36,    // ` 
    toMetaSign = 37,    // @ 
    toSymbol = 38,    // Symbols 
    toDiviEv = 39,    // / 
    toExclMark = 40,    // !
    toPercents = 41,    // % 
    toPlusMinus = 42,    //+-
    toAmpersand = 43,    // &
    toQuote = 44,    // " 
    toMinusPlus = 45;    //-+

  int m_Pos;
  uchar m_Char, m_CurrChar;
  int m_Token;
  bool m_DerivativeWithBrackets;
  int m_FixCount;
  QByteArray m_Name;
  QByteArray m_Source;
  QByteArray m_SelectName;
  bool m_MultUnknown;
  bool m_Constan;
  bool m_NewLine;
  PNode Term( bool &IsName, PNode AParent );
  PNode Factor( bool &IsName, PNode AParent );
  PNode Factor1( PNode AParent );
  PNode MonadicMinus( bool &IsName, PNode AParent );
  PNode OneRel( PNode AParent );
  PNode Measure( PNode AParent );
  PNode Interval( PNode AParent );
  PNode Constant( PNode AParent );
  PNode List( bool &IsName, PNode AParent );
  PNode List1( bool &IsName, PNode AParent );
  PNode List2( bool &IsName, PNode AParent );
  PNode Power( bool &IsName, PNode AParent );
  PNode RelSeq( bool &IsName, PNode AParent );
  PNode Power1( PNode AParent );
  PNode System();
  PNode Expression( bool &IsName, PNode AParent );
  PNode SystemEq( QByteArray& ASource );
  PNode TestAnyExpr( bool &AllOk, QByteArray& ASource, const QByteArray& UncnownName = "x" );
  bool IsUnknown( QByteArray& ASource );
  void GetToken();
  QByteArray PreProcessor( const QByteArray& ASource, const QByteArray& ASelectName );
  QByteArray Number();
  QByteArray GetWord();
  void FreeTree();
  uchar GetChar()
    {
    if(m_Pos >= m_Source.length()) return 0;
    return m_Source[m_Pos++];
    }
  int m_NodeCreated;
  int m_NodeDeleted;
  PNode m_pTree;
  PNode m_pFirst;
  Lexp* m_pCond;
  public:
    bool m_FirstListMemberIsList;
    MATHEMATICS_EXPORT static QByteArray PackUnAscii( const QByteArray& Expr );
    MATHEMATICS_EXPORT static QByteArray UnpackUnAscii( const QByteArray& Expr );
    MATHEMATICS_EXPORT Parser() : m_MultUnknown( false ), m_Constan( false ), m_NewLine( false ), m_FirstListMemberIsList( false ),
      m_NodeCreated( 0 ), m_NodeDeleted( 0 ), m_pTree( nullptr ), m_pFirst( nullptr ), m_pCond(nullptr),  m_DerivativeWithBrackets(false), m_FixCount(0)
    {}
    Parser( Lexp* pCond ) : Parser() { m_pCond = pCond; }
    MATHEMATICS_EXPORT ~Parser() 
      { 
      FreeTree(); 
#ifdef LEAK_DEBUG
      qDebug() << "Node Created: " << m_NodeCreated << ", Node deleted: " << m_NodeDeleted;
#endif
      }
    PNode AnyExpr( const QByteArray& ASource, const QByteArray& UncnownName = "x" );
    QByteArray FullPreProcessor( const QByteArray& ASource, const QByteArray& ASelectName );
    PNode GetExpression( const QByteArray& ASource );
    PNode Equation( const QByteArray& ASource, const QByteArray& ASelectName, bool &IsName, bool &AMultUnknown, bool FullProc = false );
    MathExpr OutPut( PNode );
    static MathExpr FullStrToExpr( const QByteArray& ASource, const QByteArray& Uncnown );
    MATHEMATICS_EXPORT static MathExpr PureStrToExpr( const QByteArray& ASource );
    MATHEMATICS_EXPORT static MathExpr StrToExpr( const QByteArray& ASource );
    MATHEMATICS_EXPORT static MathExpr StringToExpr( const QByteArray& ASource );
    PNode GetMeasure( const QByteArray& ASource );
  };

class ErrParser 
  {
  ParserErr m_ErrStatus;
  QString m_Message;
  QByteArray m_MsgName;
  public:
    ErrParser() : m_ErrStatus( peNoSolv ) {}
    MATHEMATICS_EXPORT ErrParser( const QString& Message, ParserErr Error );
    ParserErr ErrStatus() { return m_ErrStatus; }
    MATHEMATICS_EXPORT const QByteArray& Name() { return m_MsgName; }
    void operator = ( ParserErr NewStatus ) { m_ErrStatus = NewStatus; }
    void operator = ( const QByteArray &NewName ) { m_MsgName = NewName; }
    void ShowMessage();
    MATHEMATICS_EXPORT QByteArray Message();
  };

#endif
