#include "Parser.h"
#include "ExpObjOp.h"
#include "ExpObBin.h"
#include "ExpStore.h"
#include <algorithm>
#include "../FormulaPainter/InEdit.h"

extern int count_funct_name;
extern ExpStore s_ExpStore;

bool NIsConst( PNode Expr )
  {
  std::function<void( PNode )> PreOrder = [&] ( PNode p )
    {
    if( p == nullptr ) return;
    if( p->m_OpSign == 'v' ) throw 1;
    PreOrder( p->m_pLeft );
    PreOrder( p->m_pRight );
    };
  try
    {
    PreOrder( Expr );
    }
  catch( int ix )
    {
    return false;
    }
  return true;
  };

QByteArray Parser::PackUnAscii( const QByteArray& Expr )
  {
  const uchar *pExpr = ( const uchar* ) Expr.data();
  const uchar *pChar = pExpr;
  for( ; *pChar != 0 && *pChar < 128; pChar++ );
  if( *pChar == 0 ) return Expr;
  QByteArray Result, Prefix( "!0x" );
  do
    {
    Result.append( ( const char* ) pExpr, pChar - pExpr );
    char Buf[3];
    Result += Prefix + QByteArray(itoa( *pChar, Buf, 16 )) + '!';
    for( pExpr = ++pChar; *pChar != 0 && *pChar < 128; pChar++ );
    } while( *pChar != 0 );
  return Result + ( const char* ) pExpr;
  }

QByteArray Parser::UnpackUnAscii( const QByteArray& ASource )
  {
  QByteArray S;
  int iRestPos = 0, iStartSymb;
  bool bErr = false;
  while( ( iStartSymb = ASource.indexOf( "!0x", iRestPos ) ) != -1 )
    {
    int iSymbEnd = ASource.indexOf( '!', iStartSymb + 4 );
    bErr = iSymbEnd == -1;
    if( bErr ) break;
    bool bOk;
    uchar Symb = ASource.mid( iStartSymb + 1, iSymbEnd - iStartSymb - 1 ).toInt( &bOk, 16 );
    bErr = !bOk;
    if( bErr ) break;
    S += ASource.mid( iRestPos, iStartSymb - iRestPos ) + QByteArray( 1, Symb );
    iRestPos = iSymbEnd + 1;
    }
  if( bErr ) return ASource;
  return  S + ASource.mid( iRestPos );
  }

ErrParser::ErrParser( const QString& Message, ParserErr Error ) : m_ErrStatus( Error ), m_Message( Message )
  {
  QVector <QByteArray> MsgErrParserName = { "MsyntaxErr", "MIncrctName", "MNullPtr", "MNoOpnBrckt",
    "MNoClsBrckt", "MIncrctDegree", "MUnknwnMult",
    "MInadmOper", "MInadmFunc", "MNoSolutions", "MNoSolvType", "MUnaptMethod", "MCanNotExpand", "MNewErr" };
  s_TruePolynom = false;
  s_Precision = s_OldPrecision;
  if( s_bSuppressErrorMsg ) return;
  if( Error < MsgErrParserName.length() )
   {
    m_MsgName = MsgErrParserName[Error];
    this->Message();
   }
    else
   {
   m_MsgName = "MNotDefinePrsrErr";
   this->ShowMessage();
   }
  }

QByteArray ErrParser::Message()
  {
  return FromLang(m_Message);
  }

TNode::TNode( Parser *pOwner, const PNode Parent ) : m_pParent( Parent ), m_IsLeft( false ), m_IsRight( false ), m_pLeft( nullptr ),
m_pRight(nullptr), m_pOwner(pOwner), m_Priority(0)
  {
  pOwner->m_NodeCreated++; m_pPrev = pOwner->m_pTree; pOwner->m_pTree = this;
  }


TNode::~TNode()
  {
  m_pOwner->m_NodeDeleted++;
  }

void ErrParser::ShowMessage()
  {
  QMessageBox::critical( nullptr, "ParserError", "Error Name: " + m_MsgName + " Message: " + m_Message );
  }

void Parser::FreeTree()
  {
  while( m_pTree != m_pFirst )
    {
    PNode P = m_pTree;
    m_pTree = P->m_pPrev;
    delete P;
    }
  }

/*
void Parser::FreeTree( PNode &P )
  {
  if( P.isNull() ) return;
  FreeTree( P->m_pLeft );
  FreeTree( P->m_pRight );
  PNode Empty;
  P.swap( Empty );
  P.clear();
  }
  */

QByteArray Parser::Number()
  {
  QByteArray S;
  while( isdigit(m_Char) )
    {
    S += m_Char;
    m_Char = GetChar();
    }
  return S;
  }

QByteArray Parser::GetWord()
  {
  static SetOfChars SetChar( SetOfChars( "A..Za..z0..9_?&" ) +
  SetOfChars { msImUnit, msBigAlpha, '.', '.', msOmega, 210, 221, 222, 224, 225, 226, 229, 232, 233, 239, 240 } );
  QByteArray S;
  while( SetChar.contains( m_Char ) )
    {
    S += m_Char;
    m_Char = GetChar();
    }
  return S;
  }

void Parser::GetToken()
  {
  while( m_Char <= ' ' && m_Char != 0 ) m_Char = GetChar();
  bool Next = true;
  m_CurrChar = m_Char;
  switch( m_Char )
    {
    case '(': m_Token = toLeftParenthesis; break;
    case ')': m_Token = toRightParenthesis; break;
    case '{': m_Token = toLeftCurly; break;
    case '}': m_Token = toRightCurly; break;
    case '+': m_Token = toPlus; break;
    case '-': m_Token = toMinus; break;
    case '*':
    case msMultSign2:
      m_Token = toMultiply;
      break;
    case '|':
    case '/': m_Token = toDivide; break;
    case ';': m_Token = toSemicolon; break;
    case ',': m_Token = toComma; break;
    case 0: m_Token = toEndOfText; break;
    case '[': m_Token = toLeftBracket; break;
    case ']': m_Token = toRightBracket; break;
    case '^':
    case msHighReg:
      m_Token = toPower;
      break;
    case '~': m_Token = toRoot; break;
    case '@': m_Token = toMetaSign; break;
    case 39: m_Token = toApostrophe; break;
    case '`': m_Token = toBackApostrophe; break;
    case ':': m_Token = toColon; break;
    case '!': m_Token = toExclMark; break;
    case '&': m_Token = toAmpersand; break;
    case '=': m_Token = toEqual; break;
    case '<':
    case '>':
    case msMinequal:
    case msMaxequal:
    case msNotequal:
    case msApprox:
    case msRound:
    case msAround:
    case msParallel:
    case msIdentity:
    case msCross:
    case msAddition:
    case msArrowToRight:
    case msDoubleArrow:
    case msIntersection:
    case msStrictInclusion:
    case msUnstrictInclusion:
    case msArrowsUp:
    case msArrowsUpDown:
    case msNotBelong:
    case msUnion:
      m_Token = toBinary;
      break;
    case msPlusMinus:
    case 177:
      m_Token = toPlusMinus;
      break;
    case '.':
      m_Char = GetChar();
      if( m_Char == '.' )
        m_Token = toTwoPoints;
      else
        {
        m_Token = toPoint; // .
        Next = false;
        }
      break;
    case '%':
      m_Char = GetChar();
      if( m_Char == '*' )
        m_Token = toPercents;
      else
        {
        m_Name = "%";
        m_Token = toIdentifier;
        Next = false;
        }
      break;
    case '"':
      m_Token = toQuote;
      m_Char = GetChar();
      m_Name = "";
      do
        {
        m_Name += m_Char;
        m_Char = GetChar();
        if( m_Char == 0 )
          throw ErrParser( "Incorrect string", peName);
        } while( m_Char != '"' );
      break;
    default:
      if( isdigit(m_Char) )
        {
        m_Name = Number();
        if( m_Char == s_DecimalSeparator )
          {
          m_Char = GetChar();
          m_Name += s_DecimalSeparator;
          m_Name += Number();
          m_Token = toFloat;
          }
        else
          m_Token = toInteger;
        uchar c = m_Pos >= m_Source.length() ? 0 : m_Source.at( m_Pos );
        if( m_Char == msDegree || m_Char == msMinute || ( m_Char == '`' &&
          ( c == msDegree || c == msMinute ) ) )
          {
          if( m_Char == '`' )
            m_Char = GetChar();
          if( m_Char == msDegree )
            m_Token = toDegree;
          else
            if( m_Char == msMinute )
              m_Token = toMinute;
          m_Char = GetChar();
          if( m_Char == 39 )
            m_Char = GetChar();
          }
        Next = false;
        break;
        }
      m_Name = GetWord();
      if( m_Name == "" || ( m_Name.length() > 1 && !_litera( m_Name[0] ) ) )
        throw ErrParser( "Incorrect m_Name", peName );
      int fnum;
      if ( m_Name == "S" && m_Char != '[' )
        m_Token = toFunction;
      else
        if( m_Name != "S" && IsFunctName(m_Name, fnum))
          m_Token = toFunction;
        else
          {
          m_Token = toIdentifier;
          if (m_DerivativeWithBrackets && m_FixCount != -1 ) m_FixCount++;
          }
      Next = false;
    }
  if (Next)
    {
    m_Char = GetChar();
    if (m_FixCount == 2)
      {
      m_DerivativeWithBrackets = m_Char == '(';
      m_FixCount = -1;
      }
    }
  while( m_Char <= ' ' &&  m_Char != 0 )
    m_Char = GetChar();
  m_NewLine = false;
  if( m_Char == msCharNewLine )
    {
    m_Char = GetChar();
    if( m_Token != toComma || m_Char == ',' )
      m_Char = GetChar();
    m_NewLine = true;
    }
  }

QByteArray Parser::PreProcessor( const QByteArray& Source, const QByteArray& ASelectName )
  {
  if( Source.isEmpty() ) return "";
  QByteArray ASource = Source.trimmed(), sHexa;
  QByteArray Result = ASource.left(1);

  uchar c1 = ASource[0], c2;
  bool func = c1 == '$';
  bool quotes = false;
  bool isHexa = false;
  bool isHexaValue = false;
  int isName = 0;
  bool isMinus = false;
  bool isComma = false;
  bool wasLasti = false;
  int i = 0, k = 0, j = 0, iBracket = -1;

//  static SetOfChars SetChar( ")*+/(" );
  static SetOfChars SetChar( ")(" );
  static SetOfChars SetChar1( SetOfChars( "*+/(,;=" ) +
    SetOfChars { 255, msMinequal, msNotequal, msMaxequal, msApprox, msRound, msAround, msParallel,
    msIdentity, msCross, msAddition, msArrowToRight, msDoubleArrow, msIntersection, msStrictInclusion,
    msUnstrictInclusion, msArrowsUp, msArrowsUpDown, msNotBelong, msUnion} );
  static SetOfChars SetChar2( SetOfChars( "A..Za..z$" ) + SetOfChars { 184, '.', '.', 189, 192, 200, 201, 206, 208, 209, 230, 231, 232 } );
  static SetOfChars SetChar3(SetOfChars("A..Za..z") + SetOfChars{ msBigAlpha, '.', '.', msOmega, 221, 222, 230, 231, 232 });
//  static SetOfChars SetChar3( SetOfChars( "A..Za..z" ) + SetOfChars { 184, '.', '.', 189, 192, 200, 201, 206, 208, 209, 221, 222, 230, 231, 232 } );
  static SetOfChars SetChar31( SetOfChars( "A..Za..z@" ) + SetOfChars { 184, '.', '.', 189, 192, 200, 201, 206, 208, 209, 221, 222, 231, 232 } );
  static SetOfChars SetChar4( ")}!" );
  static SetOfChars SetChar5( SetOfChars( "A..Za..z$0..9({" ) + SetOfChars { msBigAlpha, '.', '.', msOmega, 230, 231, 232 } );
  static SetOfChars SetChar6( SetOfChars( "A..Za..z$0..9({" ) + SetOfChars { msBigAlpha, '.', '.', msOmega } );

  auto CommonAnalyse = [&]()
    {
    isMinus = false;
    isComma = false;
    isName = false;
    for( j = i + 1; !SetChar.contains( ASource[j] ) && j < ASource.length(); j++ )
      {
      isMinus = isMinus || ( ASource[j] == '-' && (  ( c1 != 'f' && c1 != 'y'  ) || ( i > 2 &&  isalpha( ASource[i - 2] ) ) ) );
      isComma = isComma || ASource[j] == ',';
      }
    if( ASource[j] == ')' && ( isComma || !isMinus ) )
      {
      for( j = Result.length() - 1; j >= 0 && !SetChar1.contains( Result[j] ); j-- );
      Result = Result.left( j + 1) + "func(" + Result.mid( j + 1 ) + ',';
      }
    else
      Result = Result + '*' + (char) c2;
    };

  bool bNotEndFor;
  auto EndFor = [&] ()
    {
    if( bNotEndFor ) return;
    if( c2 != ' ' || quotes ) c1 = c2;
    if( c1 == '$' )
      func = true;
    else
      if( c1 == 255 ) func = false;
    };

  for( i = 1; i < ASource.length(); i++, EndFor() )
    {
    bNotEndFor = true;
    if( quotes )
      {
      Result += ASource[i];
      if( ASource[i] == '"' )
        {
        if( i == ASource.length() - 1 ) return Result;
        c1 = '"';
        continue;
        }
      if( c1 == '"' )
        {
        c1 = ASource[i];
        quotes = false;
        }
      continue;
      }
    if( isHexa || isHexaValue )
      {
      if( isxdigit( ASource[i] ) )
        {
        sHexa += ASource[i];
        if( i == ASource.length() - 1 )
          return Result + ( isHexa ? QByteArray::fromHex( sHexa ) : QByteArray::number( sHexa.toUInt(nullptr, 16) ) );
        continue;
        }
      Result += ( isHexa ? QByteArray::fromHex( sHexa ) : QByteArray::number( sHexa.toUInt( nullptr, 16 ) ) );
      if( i == ASource.length() - 1 )
        return Result + ASource[i];
      c1 = ASource[i];
      Result += c1;
      isHexa = false;
      isHexaValue = false;
      sHexa.clear();
      continue;
      }
    bNotEndFor = false;
    c2 = ASource[i];
    if(wasLasti )
      {
      wasLasti = false;
      if( c2 == ')' || c2 == '-' || c2 == '@' || SetChar1.contains( c2 ))
        {
        Result = Result.left( Result.length() - 1 ) + "*i";
        Result += c2;
        continue;
        }
      }
    if( isdigit( c1 ) )
      {
      if( c1 == '0' && c2 == 'x' && ( Result.length() == 1 || !isdigit( Result[Result.length() - 2] ) ) )
        {
        isHexa = true;
        Result = Result.left( Result.length() - 1 );
        }
      else
        if( c1 == '0' && c2 == 'X' && i < ASource.length() && isxdigit( ASource[i + 1] ) )
          {
          isHexaValue = true;
          Result = Result.left( Result.length() - 1 );
          }
        else
          if( SetChar2.contains( c2 ) && ( c2 != '@' || ASource[i + 1] != ';') )
            Result = Result + '*' + ( char ) c2;
          else
            if( c2 == '(' )
              if( isName )
                CommonAnalyse();
              else
                Result = Result + '*' + ( char ) c2;
            else
              Result += c2;
      continue;
      }
    if( SetChar3.contains(c1) )
      {
      if( SetChar31.contains( c2 ) && ( c2 != '@' || ASource[i + 1] != ';' ) )
        {
        if( !func && ( c1 == 230 || c2 == '@' || ( ASelectName.length() == 1 &&
          ( toupper( c1 ) == toupper( ASelectName[0] ) || ( toupper( c2 ) == toupper( ASelectName[0] ) && c1 != 'd' ) ) ) ) )
          {
          Result = Result + '*' + ( char ) c2;
          isName = false;
          }
        else
          {
          Result += c2;
          isName++;
          wasLasti = c2 == 'i' && isName > 1;
          }
        }
      else
        if( c2 == '(' || c2 == '$' )
          CommonAnalyse();
        else
          if( c2 == 230 )
            {
            Result  = Result + '*' + (char) c2;
            isName = false;
            }
          else
            if( isdigit( c2 ) )
              {
              if( c1 == 230 )
                {
                Result = Result + '*' + ( char ) c2;
                isName = false;
                }
              else
                {
                Result += c2;
                isName++;
                wasLasti = c2 == 'i' && isName > 1;
                }
              }
          else
            {
            Result += c2;
            isName++;
            wasLasti = c2 == 'i' && isName > 1;
            }
      continue;
      }
    if( SetChar4.contains(c1) )
      {
      if( SetChar5.contains( c2 ) )
        Result = Result + '*' + ( char ) c2;
      else
        if( c2 == '@' )
          if( ASource[i + 1] == ';' )
            Result += c2;
          else
            Result = Result + '*' + (char) c2;
        else
          if( c2 == msMinute )
            {
            for( iBracket = 1, j = Result.length() - 2; j >= 0 && iBracket > 0; j-- )
              {
              if( Result[j] == '(' || ( uchar ) Result[j] == 255 )
                iBracket--;
              else
                if( Result[j] == ')' )
                  iBracket++;
              }
            if( iBracket == 0 )
              {
              Result = Result.left( j + 1) + "(DrStroke" + Result.mid( j + 1, Result.length() - (j + 2) ) + ",x))";
              c2 = ' ';
              }
            }
        else
          Result += c2;
      continue;
      }
    if( c1 == 39 )
      {
      if( SetChar6.contains(c2) )
        {
        for( j = i; j < ASource.length() - 1 && ASource[j] != '`'; j++ );
        if( j < ASource.length() - 1 && ( uchar ) ASource[j + 1] != msMinute )
          Result = Result + '*' + ( char ) c2;
        else
          Result += c2;
        }
      else
        Result += c2;
      continue;
      }
    if( c1 == '"' )
      {
      Result += c2;
      quotes = c2 != '"';
      if( !quotes ) c2  = '#';
      }
    else
      Result += c2;
   }
   return Result;
 }

QByteArray Parser::FullPreProcessor( const QByteArray& ASource, const QByteArray& ASelectName )
  {
  if( ASource.isEmpty() ) return "";
  m_DerivativeWithBrackets = false;
  m_FixCount = -1;
  QByteArray s, s1;

  auto InsideQuotes = [&] ( int pos )
    {
    bool Result = false;
    for( int i = 0; i <= pos; i++ )
      if( s[i] == '"' ) Result = !Result;
    return Result;
    };

  auto InsideFunc = [&] ( int p )
    {
    int i = 0;
    for( ; i <= 23 && p + i < s.length() && s[p + i] != '$' && ( uchar ) s[p + i] != 255; i++ );
    return p + i < s.length() && ( uchar ) s[p + i] == 255;
    };

  s_GlobalInvalid = false;
  s_CheckError = false;

  static SetOfChars SetChar( SetOfChars( "+-:=()" ) + SetOfChars { msMultSign2 } );
  static SetOfChars SetChar1( SetOfChars( "0..9_" ) + SetOfChars { 233 } );
  static SetOfChars SetChar2 { '`', 39, msDegree, msMinute };
  s = UnpackUnAscii( ASource );
//  if (s.left(4) == "(Der") s = s.mid(1);
  int p = s.indexOf( "max" );
  if( p >= 0 && !InsideQuotes( p ) )
    {
    s.remove( p, 3 );
    s.insert( p, 231 );
    }



  int n = 0;
  p = s.indexOf( "UnvisibleTable" );
  if( p >= 0 )
    {
    s.replace("UnvisibleTableX", "UnvisibleTable");
    n = p + 15;
    }
  else
    {
    p = s.indexOf( "PartlyVisibleTable" );
    if( p >= 0 ) n = p + 19;
    }
  if( n > 0 && ( s.indexOf( '"' ) == -1 || s.indexOf( "\" " ) > -1 ) )
    {
    s1 = s.left( n );
    for( int i = n; i < s.length() - 1; i++ )
      if( SetChar.contains( s[i] ) && s[i - 1] != '"' )
        s1 = s1 + '"' + s[i] + '"';
      else
        s1 += s[i];
    s = s1 + ')';
    }

  int CountName = count_funct_name - 1;
//  int CountName = count_funct_name;
//  if( !s_TaskEditorOn ) CountName--;
  for( int i = 0; i < CountName; i++ )
    {
    s1.clear();
    p = s.indexOf( InputFunctName( i ) );
    if( p >= 0 && ( s[p] == 'S' && (s[p+1] == '_' || s[p+1] == '=' || s[p+1] == '[' ) || s.mid( p, 4 ) == "Lime" ) )
      p = -1;
    while( p >= 0 )
      {
      n = p + InputFunctName( i ).length();
      if( InsideQuotes( p ) )
        {
        s1 += s.left( n );
        do
          s1 += s[n++];
        while( s[n] != '"' );
        s = s.mid( n + 1 );
        }
      else
        {
        int k = p;
        for( ; k >= 0 && s[k] != '`' && ( uchar ) s[k] != 39; k-- );
        if( k >= 0 && s[k] == '`' )
          {
          s1 += s.left( n + 1 );
          s = s.mid( n + 1 );
          }
        else
          {
          if( InsideFunc( p ) || ( InputFunctName( i ) == "S" && s.mid( p - 3, 5 ).toUpper() == "FALSE" ) )
            {
            s1 += s.left( n + 1 );
            s = s.mid( n + 1 );
            }
          else
            {
            s.insert( p, '$' );
            s1 += s.left( n + 1 ) + ( char ) 255;
            int j = n;
            while( s[++j] == ' ' );
            if( s[j] == '(' )
              s = s.mid( j + 1 );
            else
              {
              if( s[j] == '@' )  j++;
              if( isdigit( (uchar) s[j] ) || s[j] == s_DecimalSeparator )
                do j++; while( j < s.length() && ( isdigit( ( uchar ) s[j] ) || s[j] == s_DecimalSeparator ) );
              else
                if( s[j++] == '{' )
                  while( s[j++] != '}' );
                else
                  while( j < s.length() && SetChar1.contains( s[j] ) ) j++;
              while( j < s.length() && SetChar2.contains( s[j] ) ) j++;
              while( j < s.length() && isdigit( ( uchar ) s[j] ) ) j++;
              while( j < s.length() && SetChar2.contains( s[j] ) ) j++;
              s.insert( j, ')' );
              s = s.mid( n + 1 );
              }
            }
          }
        }
      p = s.indexOf( InputFunctName( i ) );
      }
    if( !s1.isEmpty() ) s = s1 + s;
    }
  s = PreProcessor( s, ASelectName );
  p = s.indexOf( 231 );
  if( p >= 0 && !InsideQuotes( p ) )
    {
    s.remove( p, 1 );
    s.insert( p, "max" );
    }
  QByteArray Result;
  for( int i = 0; i < s.length(); i++ )
    if( ( uchar ) s[i] == 255 )
      Result += '(';
    else
      if( s[i] == '$' )
        continue;
      else
        Result += s[i];
  return Result;
  }

PNode Parser::MonadicMinus( bool &IsName, PNode AParent )
  {
  PNode p;
  PNode pResult;
  if( m_Token == toPlus || m_Token == toMinus || m_Token == toPlusMinus || m_Token == toMinusPlus )
    {
    if( m_Token == toMinus )
      {
      GetToken();
      p = new TNode(this);
      p->m_pRight = MonadicMinus( p->m_IsRight, p );
      p->m_IsLeft = false;
      if( p->m_pRight->m_OpSign == msInfinity )
        {
        p->m_OpSign = msInfinity;
        p->m_Info = "-";
        p->m_pRight = nullptr;
        p->m_IsRight = false;
        }
      else
        p->m_OpSign = 'u';
      p->m_pParent = AParent;
      p->m_Priority = 2;
      pResult = p;
      IsName = p->m_IsRight;
      }
    else
      if( m_Token == toPlusMinus )
        {
        GetToken();
        p = new TNode(this);
        p->m_pRight = MonadicMinus( p->m_IsRight, p );
        p->m_IsLeft = false;
        if( p->m_pRight->m_OpSign == msInfinity )
          {
          p->m_OpSign = msInfinity;
          p->m_Info = QByteArray( 1, msPlusMinus );
          p->m_pRight = nullptr;
          p->m_IsRight = false;
          }
        else
          p->m_OpSign = msPlusMinus;
        p->m_pParent = AParent;
        p->m_Priority = 2;
        pResult = p;
        IsName = p->m_IsRight;
        }
      else
        {
        GetToken();
        pResult = MonadicMinus( IsName, AParent );
        }
    }
  else
    pResult = Power( IsName, AParent );
  return pResult;
  }

PNode Parser::Power( bool &IsName, PNode AParent )
  {
  PNode pResult = Factor( IsName, AParent );
  while( m_Token == toPower || m_Token == toRoot )
    {
    PNode p = new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    p->m_IsLeft = IsName;
    uchar op = m_Token;
    GetToken();
    p->m_pRight = Factor( p->m_IsRight, p );
    switch( op )
      {
      case toPower:
        p->m_OpSign = '^';
        p->m_Info = "^";
        break;
      case toRoot:
        p->m_OpSign = '~';
        p->m_Info = "~";
      }
    p->m_pParent = AParent;
    p->m_Priority = 1;
    //    pResult= p;
    IsName = p->m_IsLeft || p->m_IsRight;
    m_MultUnknown = m_MultUnknown || ( p->m_IsLeft && p->m_IsRight );
    return p;
    }
  return pResult;
  }

PNode  Parser::Factor( bool &IsName, PNode AParent )
  {
  PNode  pResult, p, q;
  if( m_Token == toMetaSign )
    {
    GetToken();
    m_Name = ( char ) msMetaSign + m_Name;
    }
  switch( m_Token )
    {
    case toLeftParenthesis:
      GetToken();
      pResult = Expression( IsName, AParent );
      if( m_Token != toRightParenthesis )
        throw ErrParser( "Syntactical error", peSyntacs );
      GetToken();
      break;
    case toLeftCurly:
      GetToken();
      pResult = Expression( IsName, AParent );
      if( m_Token != toRightCurly )
        throw ErrParser( "Syntactical error", peSyntacs );
      GetToken();
      break;
    case toFunction:
      pResult= new TNode(this);
      pResult->m_pLeft = nullptr;
      pResult->m_IsLeft = false;
      pResult->m_OpSign = 'F';
      pResult->m_Info = m_Name;
      m_DerivativeWithBrackets = m_Name == "Der";
      if (m_DerivativeWithBrackets) m_FixCount = 0;
      pResult->m_pParent = AParent;
      pResult->m_Priority = 0;
      GetToken();
      if( m_Token != toLeftParenthesis )
        throw ErrParser( "Syntactical error", peSyntacs );
      q = List2( IsName, pResult );
      if( pResult->m_Info == "Matric" || pResult->m_Info == "syst" )
        IsName = false;
      if( m_Token != toRightParenthesis )
        throw ErrParser( "Syntactical error", peSyntacs );
      pResult->m_pRight = q;
      pResult->m_IsRight = IsName;
      GetToken();
      break;
    case toIdentifier:
    case toFloat:
    case toInteger:
    case toQuote:
      p= new TNode(this);
      switch( m_Token )
        {
        case toIdentifier: p->m_OpSign = 'v'; break;
        case toFloat: p->m_OpSign = 'f'; break;
        case toInteger: p->m_OpSign = 'i'; break;
        case toQuote: p->m_OpSign = 's';
        }
      if( m_Name == "TRUE" || m_Name == "FALSE" ) p->m_OpSign = 'b';
      if( m_Name == QByteArray( 1, msInfinity ) )
        {
        p->m_OpSign = msInfinity;
        p->m_Info = "+";
        }
      else
        if( m_Name == QByteArray( 1, msImUnit ) )
          {
          p->m_OpSign = 'c';
          p->m_Info = m_Name;
          }
        else
          p->m_Info = m_Name;
      p->m_pParent = AParent;
      p->m_Priority = 0;
      pResult = p;
      IsName = m_Name.toUpper() == m_SelectName.toUpper();
      GetToken();
      break;
    case toMinute:
      p=new TNode(this);
      p->m_OpSign = 'd';
      p->m_Info = ';' + m_Name;
      p->m_pParent = AParent;
      p->m_Priority = 0;
      pResult = p;
      IsName = false;
      GetToken();
      break;
    case toDegree:
      p=new TNode(this);
      p->m_OpSign = 'd';
      p->m_Info = m_Name + ';';
      p->m_pParent = AParent;
      p->m_Priority = 0;
      pResult = p;
      IsName = m_Name.toUpper() == m_SelectName.toUpper();
      GetToken();
      if( m_Token == toMinute )
        {
        if( m_Name.toFloat() >= 60 )
          throw ErrParser( "Syntactical error", peSyntacs );
        pResult->m_Info += m_Name;
        GetToken();
        }
      break;
    default:
      throw ErrParser( "Syntactical error", peSyntacs );
    }

  if( m_Token == toLeftCurly )
    {
    GetToken();
    p = Expression( IsName, pResult );
    pResult->m_OpSign = 'M';
    pResult->m_pRight = p;
    if( m_Token != toRightCurly )
      throw ErrParser( "Syntactical error", peSyntacs );
    GetToken();
    }
  if( m_Token == toBackApostrophe )
    {
    p = pResult;
    pResult=new TNode(this);
    p->m_pParent = pResult;
    pResult->m_pParent = AParent;
    pResult->m_OpSign = 'm';
    pResult->m_pLeft = p;
    pResult->m_IsLeft = false;
    GetToken();
    m_Constan = false;
    pResult->m_pRight = Measure( pResult );
    pResult->m_IsRight = false;
    if( m_Token != toApostrophe )
      throw ErrParser( "Syntactical error", peSyntacs );
    GetToken();
    IsName = false;
    }
  if( m_Token == toIdentifier && m_Name == "%" )
    {
    p = pResult;
    pResult=new TNode(this);
    p->m_pParent = pResult;
    pResult->m_pParent = AParent;
    pResult->m_OpSign = 'm';
    pResult->m_pLeft = p;
    pResult->m_IsLeft = false;
    m_Constan = false;
    pResult->m_pRight = Measure( pResult );
    pResult->m_IsRight = false;
    IsName = false;
    }
  if( m_Token == toIdentifier && m_Name == QByteArray( 1, msDegree ) )
    {
    p = pResult;
    pResult=new TNode(this);
    p->m_pParent = pResult;
    pResult->m_pParent = AParent;
    pResult->m_OpSign = 'm';
    pResult->m_pLeft = p;
    pResult->m_IsLeft = false;
    m_Constan = false;
    pResult->m_pRight = Measure( pResult );
    pResult->m_IsRight = false;
    IsName = false;
    }
  if( m_Token == toExclMark )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = nullptr;
    p->m_IsLeft = false;
    p->m_IsRight = pResult->m_IsLeft || pResult->m_IsRight;
    p->m_pRight = pResult;
    p->m_OpSign = 'F';
    p->m_Info = "factorial";
    pResult = p;
    GetToken();
    }
  if( m_Token == toLeftBracket )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    p->m_IsLeft = pResult->m_IsLeft || pResult->m_IsRight;
    GetToken();
    p->m_pRight = Expression( p->m_IsRight, p );
    p->m_OpSign = 'I';
    p->m_Info = QByteArray( 1, 'I' );
    pResult = p;
    if( m_Token != toRightBracket )
      throw ErrParser( "Syntactical error", peSyntacs );
    GetToken();
    }
  return pResult;
  }

PNode  Parser::Term( bool &IsName, PNode AParent )
  {
  PNode p;
  PNode pResult = MonadicMinus( IsName, AParent );
  while( m_Token == toMultiply || m_Token == toDivide || m_Token == toColon || m_Token == toDiviEv || m_Token == toPercents )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    p->m_IsLeft = IsName;
    switch( m_Token )
      {
      case toMultiply: p->m_OpSign = m_CurrChar; break;
      case toDivide: p->m_OpSign = '/'; break;
      case toColon: p->m_OpSign = ':'; break;
//      case toDiviEv: p->m_OpSign = '|'; break;
      case toPercents: p->m_OpSign = '%';
      }
    if( m_NewLine )
      p->m_Info = "NL";
    else
      p->m_Info.clear();
    GetToken();
    p->m_pRight = MonadicMinus( p->m_IsRight, p );
    p->m_pParent = AParent;
    p->m_Priority = 3;
    pResult = p;
    IsName = p->m_IsLeft || p->m_IsRight;
    m_MultUnknown = m_MultUnknown || ( p->m_IsLeft && p->m_IsRight );
    }
  return pResult;
  }

PNode  Parser::Expression( bool &IsName, PNode AParent )
  {
  PNode p;
  PNode pResult = Term( IsName, AParent );
  while( m_Token == toPlus || m_Token == toMinus || m_Token == toPlusMinus || m_Token == toMinusPlus )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    p->m_IsLeft = IsName;
    switch( m_Token )
      {
      case toPlus: p->m_OpSign = '+'; break;
      case toMinus: p->m_OpSign = '-'; break;
      case toPlusMinus: p->m_OpSign = msPlusMinus; break;
      case toMinusPlus: p->m_OpSign = msMinusPlus;
      }
    if( m_NewLine )
      p->m_Info = "NL";
    else
      p->m_Info.clear();
    GetToken();
    p->m_pRight = Term( p->m_IsRight, p );
    p->m_pParent = AParent;
    p->m_Priority = 4;
    pResult = p;
    IsName = p->m_IsLeft || p->m_IsRight;
    m_MultUnknown = m_MultUnknown || ( p->m_IsLeft && p->m_IsRight );
    }

  if( m_Token == toLeftBracket )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    p->m_IsLeft = pResult->m_IsLeft || pResult->m_IsRight;
    GetToken();
    p->m_pRight = Expression( p->m_IsRight, p );
    p->m_OpSign = 'I';
    p->m_Info = "I";
    pResult = p;
    if( m_Token != toRightBracket )
      throw ErrParser( "Syntactical error", peSyntacs );
    GetToken();
    }
  return pResult;
  }

PNode  Parser::List( bool &IsName, PNode AParent )
  {
  PNode p;
  PNode pResult = Expression( IsName, AParent );
  while( m_Token == toComma || m_Token == toSemicolon )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pRight = pResult;
    GetToken();
    bool IsName1 = false;
    p->m_pLeft = Expression( IsName1, p );
    p->m_OpSign = ',';
    if( m_NewLine )
      p->m_Info = "NL";
    else
      p->m_Info.clear();
    pResult = p;
    IsName = IsName || IsName1;
    }
  return pResult;
  }

PNode  Parser::System()
  {
  PNode p;
  PNode pResult = OneRel( PNode() );
  while( m_Token == toSemicolon || m_Token == toMetaSign )
    {
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    if( m_Token == toMetaSign )
      {
      GetToken();
      p->m_Info = "@";
      }
    else
      p->m_Info.clear();
    GetToken();
    p->m_pRight = OneRel( p );
    p->m_OpSign = ';';
    pResult = p;
    }
  return pResult;
  }

PNode  Parser::Factor1( PNode AParent )
  {
  PNode p;
  PNode pResult;
  switch( m_Token )
    {
    case toLeftParenthesis:
      GetToken();
      pResult = Measure( AParent );
      if( m_Token != toRightParenthesis )
        throw ErrParser( "Syntactical error", peSyntacs );
      GetToken();
      break;
    case toFunction:
    case toIdentifier:
      p=new TNode(this);
      p->m_OpSign = 'v';
      p->m_Info = m_Name;
      p->m_pParent = AParent;
      pResult = p;
      GetToken();
      break;
    case toInteger:
      if( m_Name != "1" )
        throw ErrParser( "Syntactical error", peSyntacs );
      m_Constan = true;
      p=new TNode(this);
      p->m_OpSign = 'i';
      p->m_Info = m_Name;
      p->m_pParent = AParent;
      p->m_Priority = 0;
      pResult = p;
      GetToken();
      break;
    default:
      throw ErrParser( "Syntactical error", peSyntacs );
    }
  return pResult;
  }

PNode  Parser::Constant( PNode AParent )
  {
  PNode p;
  PNode pResult;
  switch( m_Token )
    {
    case toLeftParenthesis:
      GetToken();
      pResult = Constant( AParent );
      if( m_Token != toRightParenthesis )
        throw ErrParser( "Syntactical error", peSyntacs );
      GetToken();
      break;
    case toInteger:
      p=new TNode(this);
      p->m_OpSign = 'i';
      p->m_Info = m_Name;
      p->m_pParent = AParent;
      pResult = p;
      GetToken();
      break;
    default:
      throw ErrParser( "Syntactical error", peSyntacs );
    }
  return pResult;
  }

PNode  Parser::Power1( PNode AParent )
  {
  PNode pResult = Factor1( AParent );
  if( m_Token == toPower )
    {
    PNode p = new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    GetToken();
    try
      {
      p->m_pRight = Constant(p);
      }
    catch (ErrParser E)
      {
      if (m_Token != toMinus) throw;
      p->m_pRight = MonadicMinus(p->m_IsRight, p);
      if(m_Token == toRightParenthesis ) GetToken();
      }
    p->m_OpSign = '^';
    p->m_Info = "^";
    p->m_pParent = AParent;
    pResult = p;
    }
  return pResult;
  }

PNode  Parser::Measure( PNode AParent )
  {
  PNode p;
  PNode pResult = Power1( AParent );
  while( m_Token == toMultiply || m_Token == toDivide )
    {
    if( m_Constan && m_Token != toDivide )
      throw ErrParser( "Syntactical error", peSyntacs );
    m_Constan = false;
    p=new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    uchar op = m_Token;
    GetToken();
    p->m_pRight = Power1( p );
    switch( op )
      {
      case toMultiply:
        p->m_OpSign = '*';
        p->m_Info = "*";
        break;
      case toDivide:
        p->m_OpSign = '/';
        p->m_Info = "/";
      }
    p->m_pParent = AParent;
    pResult = p;
    }
  return pResult;
  }

PNode  Parser::GetMeasure( const QByteArray& ASource )
  {
  m_Source = ASource;
  m_SelectName = "x";
  m_Pos = 0;
  m_Char = GetChar();
  GetToken();
  PNode pResult = Measure( PNode() );
  if( m_Token != toEndOfText )
    throw ErrParser( "Syntactical error", peSyntacs );
  return pResult;
  }

PNode  Parser::GetExpression( const QByteArray& ASource )
  {
  m_Source = ASource;
  m_SelectName = "x";
  m_Pos = 0;
  m_Char = GetChar();
  GetToken();
  bool b;
  PNode pResult = Expression( b, PNode() );
  if( m_Token != toEndOfText )
    throw ErrParser( "Syntactical error", peSyntacs );
  return pResult;
  }

bool Parser::IsUnknown( QByteArray& ASource )
  {
  m_Source = ASource;
  m_SelectName = "x";
  m_Pos = 0;
  m_Char = GetChar();
  GetToken();
  bool Result;
  PNode pResult = Expression( Result, PNode() );
  if( m_Token != toEndOfText )
    throw ErrParser( "Syntactical error", peSyntacs );
  return Result;
  }

PNode  Parser::Equation( const QByteArray& ASource, const QByteArray& ASelectName, bool &IsName, bool &AMultUnknown, bool FullProc )
  {
  if( FullProc )
    m_Source = FullPreProcessor( ASource, ASelectName);
  else
    m_Source = ASource;
  m_SelectName = ASelectName;
  m_Pos = 0;
  m_Char = GetChar();
  GetToken();
  m_MultUnknown = false;
  PNode pResult = new TNode(this);
  pResult->m_pLeft = Expression( pResult->m_IsLeft, pResult );
  if( m_Token == toEqual )
    {
    if( m_NewLine )
      pResult->m_Info = "NL";
    else
      pResult->m_Info.clear();
    pResult->m_OpSign = '=';
    pResult->m_Priority = 5;
    GetToken();
    }
  else
    throw ErrParser( "Syntactical error", peSyntacs );
  pResult->m_pRight = Expression( pResult->m_IsRight, pResult );
  if( m_Token != toEndOfText )
    throw ErrParser( "Syntactical error", peSyntacs );
  IsName = pResult->m_IsLeft || pResult->m_IsRight;
  m_MultUnknown = m_MultUnknown || ( pResult->m_IsLeft && pResult->m_IsRight );
  AMultUnknown = m_MultUnknown;
  return pResult;
  }

PNode  Parser::OneRel( PNode Parent )
  {
  PNode pResult = new TNode(this);
  pResult->m_pParent = Parent;
  pResult->m_pLeft = Expression( pResult->m_IsLeft, pResult );
  if( m_Token == toEqual || m_Token == toBinary )
    {
    pResult->m_Info = QByteArray( 1, m_CurrChar );
    pResult->m_OpSign = 'B';
    pResult->m_Priority = 5;
    if( m_NewLine )
      pResult->m_Info += "NL";
    GetToken();
    }
  else
    throw ErrParser( "Syntactical error", peSyntacs );
  pResult->m_pRight = Expression( pResult->m_IsRight, pResult );
  return pResult;
  }

PNode  Parser::SystemEq( QByteArray& ASource )
  {
  m_Source = ASource;
  m_Pos = 0;
  m_Char = GetChar();
  GetToken();
  PNode pResult = OneRel( PNode() );
  while( m_Token == toSemicolon )
    {
    PNode p = new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    GetToken();
    p->m_pRight = OneRel( p );
    p->m_OpSign = ';';
    p->m_pParent = nullptr;
    p->m_Priority = 5;
    pResult = p;
    }
  return pResult;
  }

PNode  Parser::Interval( PNode AParent )
  {
  PNode pResult = new TNode(this);
  pResult->m_pParent = AParent;
  pResult->m_OpSign = '[';
  pResult->m_Priority = 0;
  if( m_Token == toLeftBracket )
    pResult->m_Info = "[";
  else
    pResult->m_Info = "(";
  GetToken();
  bool b;
  pResult->m_pLeft = Expression( b, pResult );
  if( m_Token != toSemicolon )
    throw ErrParser( "Syntactical error", peSyntacs );
  GetToken();
  pResult->m_pRight = Expression( b, pResult );
  if( m_Token == toRightBracket )
    pResult->m_Info += "]";
  else
    if( m_Token == toRightParenthesis )
      pResult->m_Info += ")";
    else
      throw ErrParser( "Syntactical error", peSyntacs );
  return pResult;
  }

PNode  Parser::RelSeq( bool &IsName, PNode AParent )
  {
  int pos1 = m_Pos;
  char c1 = m_Char;
  GetToken();
  int Token1 = m_Token;
  PNode  pResult;
  try
    {
    m_pFirst = m_pTree;
    pResult = Expression( IsName, AParent );
    m_pFirst = nullptr;
    }
  catch( ErrParser E )
    {
    m_FirstListMemberIsList = false;
    if( !s_bSuppressErrorMsg )
      {
      m_pFirst = nullptr;
      m_Pos = pos1;
      m_Char = c1;
      if( Token1 == toLeftBracket )
        {
        GetToken();
        pResult = Interval( AParent );
        GetToken();
        }
      else
        {
        if( Token1 != toLeftParenthesis ) throw;
        GetToken();
        pResult = RelSeq( IsName, AParent );
        if( m_Token != toRightParenthesis )
          if( m_Token == toSemicolon )
            {
            m_Pos = pos1;
            m_Char = c1;
            GetToken();
            pResult = Interval( AParent );
            }
          else
            {
 //           FreeTree();
            throw;
            }
          GetToken();
        }
      }
    else
      FreeTree();
    }
  if( m_Token == toEqual || m_Token == toBinary )
    {
    PNode p = new TNode(this);
    pResult->m_pParent = p;
    p->m_pLeft = pResult;
    p->m_IsLeft = IsName;
    p->m_OpSign = 'B';
    p->m_Info = QByteArray( 1, m_CurrChar );
    if( m_NewLine )
      p->m_Info += "NL";
    p->m_pRight = RelSeq( p->m_IsRight, p );
    p->m_pParent = AParent;
    p->m_Priority = 5;
    pResult = p;
    IsName = p->m_IsLeft || p->m_IsRight;
    }
  return pResult;
  }

PNode  Parser::List1( bool &IsName, PNode AParent )
  {
  PNode  pResult;
  int pos1 = m_Pos;
  char c1 = m_Char;
  int Second = 0;
  do
    {
    m_Pos = pos1;
    m_Char = c1;
    try
      {
      if( Second > 0 )
        GetToken();
      pResult = RelSeq( IsName, AParent );
      while( m_Token == toComma )
        {
//        PNode p = nullptr;
        PNode p = new TNode(this);
        pResult->m_pParent = p;
        p->m_OpSign = ',';
        if( m_NewLine )
          p->m_Info = "NL";
        else
          p->m_Info.clear();
        p->m_pRight = pResult;
        bool IsName1;
        p->m_pLeft = RelSeq( IsName1, p );
        p->m_pParent = AParent;
        p->m_Priority = 5;
        pResult = p;
        IsName = IsName || IsName1;
        }
      if( Second > 0 )
        if( m_Token != toRightParenthesis )
          throw ErrParser( "Syntactical error", peSyntacs );
        else
          GetToken();
      if( Second > 0 ) Second = 0;
      }
    catch( ErrParser E )
      {
      m_FirstListMemberIsList = false;
      Second++;
      if( m_Token == toEndOfText ) throw;
      }
    } while( Second != 0 && Second <= 1 );
  if( Second > 1 )
    throw ErrParser( "Syntactical error", peSyntacs );
  return pResult;
  }

PNode  Parser::List2( bool &IsName, PNode AParent )
  {
  PNode  pResult;
  int pos1 = m_Pos;
  char c1 = m_Char;
  int Second = 0;
  do
    {
    m_Pos = pos1;
    m_Char = c1;
    try
      {
      if( Second > 0 )
        GetToken();
      pResult = List1( IsName, AParent );
      while( m_Token == toSemicolon || m_Token == toMetaSign )
        {
        PNode p = new TNode(this);
        p->m_OpSign = ';';
        pResult->m_pParent = p;
        p->m_pLeft = pResult;
        if( m_Token == toMetaSign )
          {
          GetToken();
          p->m_Info = "@";
          }
        else
          p->m_Info.clear();
        bool IsName1;
        p->m_pRight = List1( IsName1, p );
        p->m_pParent = nullptr;
        p->m_Priority = 5;
        pResult = p;
        IsName = IsName || IsName1;
        }
      if( Second > 0 )
        if( m_Token != toRightParenthesis )
          throw ErrParser( "Syntactical error", peSyntacs );
        else
          GetToken();
      if( Second > 0 ) Second = 0;
      }
    catch( ErrParser E )
      {
      m_FirstListMemberIsList = false;
      Second++;
      if( m_Token == toEndOfText ) throw;
      }
    } while( Second != 0 && Second <= 1 );
  return pResult;
  }

PNode  Parser::AnyExpr( const QByteArray& ASource, const QByteArray& UncnownName )
  {
  PNode pResult;
  if( ASource.isEmpty() ) return PNode();
  m_SelectName = UncnownName;
  m_Source = ASource;
  m_Pos = 0;
  m_Char = GetChar();
  bool b;
  try
    {
    pResult = List2( b, PNode() );
    if( m_Token != toEndOfText )
      throw ErrParser( "Syntactical error", peSyntacs );
    }
  catch (ErrParser& ErrMsg)
    {
    s_GlobalInvalid = true;
    return PNode();
    }
  return pResult;
  }

PNode  Parser::TestAnyExpr( bool &AllOk, QByteArray& ASource, const QByteArray& UncnownName )
  {
  AllOk = true;
  if( ASource.isEmpty() ) return PNode();
  m_SelectName = UncnownName;
  m_Source = ASource;
  m_Pos = 0;
  m_Char = GetChar();
  bool b;
  PNode  pResult = List2( b, PNode() );
  AllOk = m_Token == toEndOfText;
  return pResult;
  }

MathExpr Parser::OutPut( PNode p ) // The tree of solution will be transformed to expression
  {
  if( p == nullptr ) return nullptr;

  MathExpr Result;
  typedef QVector<MathExpr> TArgs;
  MathExpr op1, op2, op11, op22, Exp;
  int Degree, N;
  double d, V;
  int Int, Nom, Den;
  Lexp SysEq; // TL2exp
  Lexp List; // TLexp
  PExMemb f;
  TArgs op( 10 );
  int ArgCount;
  QByteArray fname;
  MathExpr body;
  uchar IntegralType;

  auto GetArg = [&] ( TLexp* pList, TArgs& Args, int& Count )
    {
    Count = 0;
    f = pList->m_pFirst;
    while( !f.isNull() )
      {
      Args[Count++] = f->m_Memb;
      f = f->m_pNext;
      }
    };

  if( p->m_OpSign == 'F' )
    s_FuncArgListDepth = s_FuncArgListDepth + 1;
  op1 = OutPut( p->m_pLeft );
  op2 = OutPut( p->m_pRight );
  switch( p->m_OpSign )
    {
    case ';':
      m_FirstListMemberIsList = false;
      if( op1.List2ex( f ) )
        SysEq = op1;
      else
        {
        SysEq = new TL2exp;
        SysEq.Addexp( op1 );
        if( p->m_Info == "@" )
          SysEq.Last()->m_Visi = false;
        else
          SysEq.Last()->m_Visi = true;
        }
      SysEq.Addexp( op2 );
      if( p->m_Info == "@" )
        SysEq.Last()->m_Visi = false;
      else
        SysEq.Last()->m_Visi = true;
      Result = SysEq;
      break;
    case ',':
      if( op2.Listex( f ) && !m_FirstListMemberIsList && (!op1.Listex( f ) || s_FuncArgListDepth > 0 ) )
        List = op2;
      else
        {
        List = new TLexp;
        List.Addexp( op2 );
        }
      m_FirstListMemberIsList = false;
      if( p->m_Info == "NL" )                // 2000-09-21
        List.Addexp( new TNewLin );
      List.Addexp( op1 );
      Result = List;
      break;
    case '+':
      if( p->m_Info == "NL" )
        Result = new TSumm( new TSumm( op1, new TNewLin ), op2 );
      else
        {
        if( op2.Complex( op11, op22 ) && op11.Cons_int( Int ) && ( Int == 0 ) && !op1.HasComplex() )
          {
          Cast( TComplexExpr, op2.Ptr() )->m_Re = op1;
          Result = op2;
          }
        else
          if( op1.Complex( op11, op22 ) && op11.Cons_int( Int ) && ( Int == 0 ) && !op2.HasComplex() )
            {
            Cast( TComplexExpr, op1.Ptr() )->m_Re = op2;
            Result = op1;
            }
          else
            Result = new TSumm( op1, op2 );
        }
      break;
    case '-':
      if( p->m_Info == "NL" )
        Result = new TSubt( new TSubt( op1, new TNewLin ), op2 );
      else
        if( op2.Complex( op11, op22 ) && op11.Cons_int( Int ) && ( Int == 0 ) && !op1.HasComplex() )
          {
          Cast( TComplexExpr, op2.Ptr() )->m_Re = op1;
          Cast( TComplexExpr, op2.Ptr() )->m_Im = new TUnar( op22 );
          Result = op2;
          }
        else
          if( op1.Complex( op11, op22 ) && op11.Cons_int( Int ) && ( Int == 0 ) && !op2.HasComplex() )
            {
            Cast( TComplexExpr, op1.Ptr() )->m_Re = new TUnar( op2 );
            Result = op1;
            }
          else
            Result = op1 - op2;
      break;
    case msPlusMinus:
      if( op1 == nullptr )
        Result = new TUnapm( op2 );
      else
        Result = new TBinar( msPlusMinus, op1, op2 );
      break;
    case  '*':
    case msMultSign2:
      if( p->m_Info == "NL" )
        Result = new TMult( new TMult( op1, new TNewLin, p->m_OpSign ), op2, p->m_OpSign );
      else
        if( op2.Complex( op11, op22 ) && op11.Cons_int( Int ) && ( Int == 0 ) && op22.Cons_int( Int ) && ( Int == 1 ) &&
          ( IsType( TConstant, op1 ) || IsType( TVariable, op1 ) || IsType( TSimpleFrac, op1 ) ) )
          Result = new TComplexExpr( op11, op1 );
        else
          if( op1.Complex( op11, op22 ) && op11.Cons_int( Int ) && ( Int == 0 ) && op22.Cons_int( Int ) && ( Int == 1 ) &&
            ( IsType( TConstant, op2 ) || IsType( TVariable, op2 ) || IsType( TSimpleFrac, op2 ) ) )
            Result = new TComplexExpr( op11, op2 );
          else
            Result = new TMult( op1, op2, p->m_OpSign );
      break;
    case '%':
      Result = new TBinar( ' ', new TStr( op1.WriteE() + "%*" ), new TStr( op2.WriteE() ) );
      break;
    case ':':
      Result = new TDivi( op1, op2, false, ':' );
      break;
    case '/':
    case '|':
      if( m_pCond == nullptr )
        if( op1.Cons_int( Nom ) && op2.Cons_int( Den ) )
          Result = new TSimpleFrac( Nom, Den );
        else
          if( op1.Unarminus( op11 ) && op11.Cons_int( Nom ) && op2.Cons_int( Den ) )
            Result = new TSimpleFrac( -Nom, Den );
          else
            Result = new TDivi( op1, op2, p->m_OpSign );
      else
        if( op2.Reduce().Constan( d ) && abs( d ) < 0.0000001 )
          throw  ErrParser( "No Solutions!", peNoSolv );
        else
          {
          Result = op1 / op2;
          PNode q = GetExpression( op2.WriteE() );
          if( !NIsConst( q ) )
            {
            MathExpr ex = new TBinar( msNotequal, op2, ::Constant( 0 ) );
            if( !m_pCond->FindEqual( ex, f ) ) m_pCond->Addexp( ex );
            }
          }
      break;
      /*
    case '|':
      if( m_pCond != nullptr && op2.Reduce().Constan( d ) && abs( d ) < 0.0000001 )
        throw  ErrParser( "No Solutions!", peNoSolv );
      if( p->m_Info == "NL" )
        Result = new TDiviEv( new TDiviEv( op1, new TNewLin ), op2 );
      else
        if( op1.Cons_int( Nom ) && op2.Cons_int( Den ) )
          Result = new TSimpleFrac( Nom, Den );
        else
          if( op1.Unarminus( op11 ) && op11.Cons_int( Nom ) && op2.Cons_int( Den ) )
            Result = new TSimpleFrac( -Nom, Den );
          else
            Result = new TDiviEv( op1, op2 );
      break;
      */
    case '^':
      if( op1.Variab( fname ) && ( fname == "e" ) )
        Result = new TFunc( false, "exp", op2 );
      else
        Result = new TPowr( op1, op2 );
      break;
    case '~':
      if( op2.Cons_int( Degree ) )
        Result = new TRoot( op1, Degree );
      else
        Result = new TRoot1( op1, op2 );
      break;
    case '=':
      if( p->m_Info == "NL" )
        Result = new TBinar( '=', new TBinar( '=', op1, new TNewLin ), op2 );
      else
        Result = new TBinar( '=', op1, op2 );
      break;
    case  'B':
      if( p->m_Info.mid( 1, 2 ) == "NL" )
        Result = new TBinar( p->m_Info[0], new TBinar( p->m_Info[0], op1, new TNewLin ), op2 );
      else
        Result = new TBinar( p->m_Info[0], op1, op2 );
      break;
    case  'b':
      Result = new TBool( p->m_Info == "TRUE" );
      break;
    case msInfinity:
      Result = new TInfinity( p->m_Info == "-" );
      break;
    case 'c':
      Result = new TComplexExpr( new TConstant( 0 ), new TConstant( 1 ) );
      break;
    case  'F':
      s_FuncArgListDepth = std::max( s_FuncArgListDepth - 1, 0 );
      IntegralType = 0;
      if( p->m_Info == "CurveIntegral" )
        IntegralType = msIntegral;
      else if( p->m_Info == "SurfaceIntegral" )
        IntegralType = msDoubleIntegral;
      else if( p->m_Info == "ContourIntegral" )
        IntegralType = msContourIntegral;
      if( IntegralType != 0 )
        {
        TLexp *pLext = CastPtr( TLexp, op2 );
        if( pLext == nullptr || pLext->m_Count != 2 ) throw ErrParser( "Syntax error!", peSyntacs );
        GetArg( pLext, op, ArgCount );
        Result = new TCurveIntegral( false, IntegralType, op[0], op[1] );
        }
      else
      if( p->m_Info == "arcsinM" )
        if( TExpr::sm_TrigonomSystem == TExpr::tsRad )
          Result = new TSumm( new TMult( new TPowr( new TUnar( new TConstant( 1 ) ), new TVariable( false, "k" ) ),
          new TFunc( false, "arcsin", op2 ) ), new TMult( new TVariable( false, msPi ), new TVariable( false, "k" ) ) );
        else
          {
          Exp = new TSumm( new TMult( new TPowr( new TUnar( new TConstant( 1 ) ), new TVariable( false, "k" ) ),
            new TFunc( false, "arcsin", op2 ) ), new TMult( new TDegExpr( "180;" ), new TVariable( false, "k" ) ) );
          Result = Exp.Reduce();
          }
      else
        if( p->m_Info == "arccosM" )
          if( TExpr::sm_TrigonomSystem == TExpr::tsRad )
            Result = new TBinar( msPlusMinus, new TMult( new TConstant( 2 ), new TMult( new TVariable( false, msPi ),
            new TVariable( false, "k" ) ) ), new TFunc( false, "arccos", op2 ) );
          else
            {
            Exp = new TBinar( msPlusMinus, new TMult( new TDegExpr( "360;" ), new TVariable( false, "k" ) ),
              new TFunc( false, "arccos", op2 ) );
            Result = Exp.Reduce();
            }
        else if( p->m_Info == "arctanM" )
          if( TExpr::sm_TrigonomSystem == TExpr::tsRad )
            Result = new TSumm( new TFunc( false, "arctan", op2 ), new TMult( new TVariable( false, msPi ), new TVariable( false, "k" ) ) );
          else
            {
            Exp = new TSumm( new TFunc( false, "arctan", op2 ), new TMult( new TDegExpr( "180;" ), new TVariable( false, "k" ) ) );
            Result = Exp.Reduce();
            }
        else  if( p->m_Info == "arccotM" )
          if( TExpr::sm_TrigonomSystem == TExpr::tsRad )
            Result = new TSumm( new TFunc( false, "arccot", op2 ), new TMult( new TVariable( false, msPi ), new TVariable( false, "k" ) ) );
          else
            {
            Exp = new TSumm( new TFunc( false, "arccot", op2 ), new TMult( new TDegExpr( "180;" ), new TVariable( false, "k" ) ) );
            Result = Exp.Reduce();
            }
        else  if( p->m_Info == "log" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          Result = new TLog( CastPtr( TLexp, op2 )->m_pFirst->m_Memb, CastPtr( TLexp, op2 )->m_pLast->m_Memb );
          }
        else  if( p->m_Info == "Integral" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          switch( ArgCount )
            {
            case 2:
              Result = new TIntegral( false, op[0], op[1] );
              break;
            case 4:
                Result = new TDefIntegral( false, op[0], op[1], op[2], op[3] );
              break;
            default:
              throw  ErrParser( "Syntax error!", peSyntacs );
            }
          }

        else  if( p->m_Info == "DoubleIntegral" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          if( ArgCount != 4 ) throw  ErrParser( "Syntax error!", peSyntacs );
          Result = new TMultIntegral( false, op[0], op[1], op[2], op[3] );
          }

        else  if( p->m_Info == "TripleIntegral" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          if( ArgCount != 5 ) throw  ErrParser( "Syntax error!", peSyntacs );
          Result = new TMultIntegral( false, op[0], op[1], op[2], op[3], op[4] );
          }

        else  if( p->m_Info == "Der" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          TDeriv *pDeriv = new TDeriv(op2);
          pDeriv->m_HasParenthesis = m_DerivativeWithBrackets;
          m_DerivativeWithBrackets = false;
          m_FixCount = 0;
          Result = pDeriv;
          }

        else  if( p->m_Info == "DrStroke" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          TLexp *pLexp = CastPtr( TLexp, op2 );
          Result = new TDeriv( new TBool( true ), new TBool( false ), pLexp->m_pFirst->m_Memb, pLexp->m_pLast->m_Memb );
          }

        else  if( p->m_Info == "Lim" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          if( ArgCount < 3 ) throw ErrParser( "Syntax error!", peSyntacs );
          Result = new TLimit( false, op[0], op[1], op[2] );
          }

        else  if( p->m_Info == "Zumma" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          if( ArgCount < 3 ) throw ErrParser( "Syntax error!", peSyntacs );
          Result = new TGSumm( false, op[0], op[1], op[2] );
          }

        else  if( p->m_Info == "subst" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          if( ArgCount < 3 ) throw ErrParser( "Syntax error!", peSyntacs );
          Result = new TSubst( false, op[0], op[1], op[2] );
          }

        else  if( p->m_Info == "Product" )
          {
          if( !( IsType( TLexp, op2 ) ) ) throw ErrParser( "Syntax error!", peSyntacs );
          GetArg( CastPtr( TLexp, op2 ), op, ArgCount );
          if( ArgCount < 3 ) throw ErrParser( "Syntax error!", peSyntacs );
          Result = new TGMult( false, op[0], op[1], op[2] );
          }

        else if( p->m_Info == "abs" )
          Result = new TAbs( false, op2 );
        else  if( p->m_Info == "Vector" )
          Result = new TVect( op2 );
        else  if( p->m_Info == "Matric" )
          Result = new TMatr( op2 );
        else  if( p->m_Info == "Table" )
          Result = new TTable( op2 );
        else  if( p->m_Info == "Picture" )
          Result = new TExprPict( op2 );
        else if( p->m_Info == "UnvisibleTable" )
          {
          Result = new TTable( op2 );
          CastPtr( TTable, Result )->m_GridState = TGRUnvisible;
          }
        else  if( p->m_Info == "PartlyVisibleTable" )
          {
          Result = new TTable( op2 );
          CastPtr( TTable, Result )->m_GridState = TGRPartlyVisible;
          }
//        else  if( p->m_Info == "CreateObject" )
//          Result = Lexp( op2 ).CreateObject();
        else  if( p->m_Info == "syst" )
          Result = new TSyst( op2 );
        else  if( p->m_Info == "Polygon" )
          Result = new TPolygon( op2 );
        else  if( p->m_Info == "Chart" )
          Result = new TChart( op2 );
        else if( ( char ) p->m_Info[0] == msMetaSign )
          {
          fname = p->m_Info.mid( 1 );
          if( s_TaskEditorOn )
            {
            if( s_iDogOption > 0 )
              Result = new TFunc( false, InterNameByInputName( p->m_Info ), op2 );
            else
              {
              Exp = new TFunc( true, InterNameByInputName( fname ), op2 );
              try
                {
                Result = Exp.Perform().Ptr();
                }
              catch (ErrParser E)
                {
                Result = Exp;
                }
              }
            }
          else
            {
            Exp = new TFunc( true, InterNameByInputName( fname ), op2 );
            Result = Exp.Perform();
            m_FirstListMemberIsList = fname == "Simplify" && IsType( TLexp, op2 );
            }
          }
        else
          Result = new TFunc( false, InterNameByInputName( p->m_Info ), op2 );
        break;
    case  'u':
      if( op2.SimpleFrac_( Nom, Den ) && !( IsType( TMixedFrac, op2 ) ) )
        Result = new TSimpleFrac( -Nom, Den );
      else
        if( op2.Constan( d ) && ( d >= 0.0 ) )
          Result = new TConstant( -d );
        else
          Result = new TUnar( op2 );
      break;
    case 'i':
    case  'f':
      Result = new TConstant( p->m_Info.toDouble() );
      break;
    case 'd':
      Result = new TDegExpr( p->m_Info );
      break;
    case 's':
      Result = new TCommStr( p->m_Info );
      break;
    case 'v':
      if( !s_TaskEditorOn )
        {
        if( ( char ) p->m_Info[0] == msMetaSign )
          if( s_ExpStore.Known_var_expr( p->m_Info.mid( 1 ), body ) )
            Result = body;
          else
            Result = new TVariable( false, p->m_Info.mid( 1 ) );
        else if( ( p->m_Info == "i" ) && ( p->m_pParent == nullptr || ( p->m_pParent->m_Info != "I" &&  p->m_pParent->m_Info != "Vector") ) )
          Result = new TComplexExpr( new TConstant( 0 ), new TConstant( 1 ) );
        else if( p->m_Info == "true" )
          Result = new TBool( true );
        else if( p->m_Info == "false" )
          Result = new TBool( false );
        else
          Result = new TVariable( false, p->m_Info );
        }
      else
        {
        if( ( char ) p->m_Info[0] == msMetaSign )
          if( s_iDogOption == 0 && s_ExpStore.Known_var_expr( p->m_Info.mid( 1 ), body ) )
            Result = body;
          else
            Result = new TVariable( false, p->m_Info );
        else if( ( p->m_Info == "i" ) && ( p->m_pParent == nullptr || ( p->m_pParent->m_Info != "I" &&  p->m_pParent->m_Info != "Vector") ) )
          Result = new TComplexExpr( new TConstant( 0 ), new TConstant( 1 ) );
        else if( p->m_Info == "true" )
          Result = new TBool( true );
        else if( p->m_Info == "false" )
          Result = new TBool( false );
        else
          Result = new TVariable( false, p->m_Info );
        }
      break;
    case  'M':
      Int = s_ExpStore.GetValue( p->m_Info );
      if( op2.Divis( op11, op22 ) )
        {
        op11.Cons_int( Nom );
        op22.Cons_int( Den );
        }
      else
        op2.SimpleFrac_( Nom, Den );
      Result = new TMixedFrac( Int, Nom, Den );
      break;
    case 'm':
      op22 = op1.Reduce();
      if( !op22.Unarminus( op11 ) ) op11 = op22;
      if( s_iDogOption > 0 || s_TaskEditorOn || op11.Variab( fname ) || op11.ConstExpr() )
        Result = new TMeaExpr( op1, op2 );
      else
        throw  ErrParser( "Syntax error!", peSyntacs );
      break;
    case  'I':
      Result = new TIndx( op1, op2 );
      break;
    case  '[':
      Result = new TSimpleInterval( p->m_Info, op1, op2 );
      break;
    default:
      throw  ErrParser( "Wrong operation!", peOpertn );
    }
  return Result;
  }

MathExpr Parser::FullStrToExpr( const QByteArray& ASource, const QByteArray& Uncnown )
  {
  s_GlobalInvalid = false;
  s_CheckError = false;
  if( ASource.isEmpty() )
    {
    s_GlobalInvalid = true;
    return nullptr;
    }
  Parser P;
  MathExpr Result;
  try
    {
    Result = P.OutPut( P.AnyExpr( P.FullPreProcessor( ASource, Uncnown ) ) );
    }
  catch( ... )
    {
    s_GlobalInvalid = true;
    return nullptr;
    }
  if( Result.IsEmpty() ) return nullptr;
  return Result;
  }


MathExpr Parser::StrToExpr( const QByteArray& ASource )
  {
//  return FullStrToExpr( ASource, "x" );
  return FullStrToExpr(ASource, "");
  }

MathExpr Parser::PureStrToExpr( const QByteArray& ASource )
  {
  return FullStrToExpr( ASource, "" );
  }

MathExpr Parser::StringToExpr( const QByteArray& ASource )
  {
  s_GlobalInvalid = false;
  s_CheckError = false;
  Parser P;
  MathExpr Result = P.OutPut( P.AnyExpr( P.FullPreProcessor( ASource, "x" ) ) );
  if( s_GlobalInvalid ) throw  ErrParser( "Argument error", peFunctn );
  return Result;
  }

bool IsListExpression( const QByteArray& Text )
  {
  MathExpr Exp( Parser::StrToExpr( Text ) );
  if( Exp.IsEmpty() ) return false;
  PExMemb F;
  return Exp.Listex( F );
  }
