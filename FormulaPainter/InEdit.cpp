#include "InEdit.h"
#include <ctype.h>
#include "StrHebEng.h"
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include <qhash.h>
#include <qmap.h>
#include <qinputdialog.h>
#include "FuncName.h"

#define TypeConvert(Type) ( dynamic_cast <Type*> ( m_pL->m_pSub_L->m_pMother->m_pMember.data() ) )

#ifdef LEAK_DEBUG
int TXPGrEl::sm_CreatedCount;
int TXPGrEl::sm_DeletedCount;
#endif

QRect XPInEdit::GetRect() { return QRect( m_Start.X, m_Start.Y, m_Size.width(), m_Size.height() ); }
QImage* XPInEdit::GetImage() { return m_pMyImage; }
QPixmap XPInEdit::GetPixmap() { return QPixmap::fromImage(*m_pMyImage); }
bool XPInEdit::Selected()
  {
  return m_SelectedRect.isValid();
  }

bool TXPGrEl::sm_EditKeyPress = false;
QByteArray XPInEdit::sm_Text;
bool XPInEdit::sm_TextFont = false;
void( *XPInEdit::sm_Message )( const QString& ) = nullptr;
TLanguages TXPGrEl::sm_Language = lngHebrew;
QByteArray SWriteMode;
QTextCodec* EdStr::sm_pCodec = QTextCodec::codecForName( "Windows-1255" );
QTextCodec* EdBaseChar::sm_pCodec = QTextCodec::codecForName("Windows-1253");
EdMatr* XPInEdit::sm_pMatr = nullptr;
bool EdTable::sm_WasTemplate = false;
bool EdTable::sm_RowTableAsExpression = false;
PEdElm EdTable::sm_pTemplate;
XPInEdit* XPInEdit::sm_pEditor = nullptr;
bool XPInEdit::sm_IsRetryEdit = false;
bool XPInEdit::sm_EditString = false;
bool EdStr::sm_PureText = false;
int XPInEdit::sm_AddHeight = 10;
int EdIntegr::sm_SignSize = 0;
int EdIntervals::sm_Width = 400;
int EdIntervals::sm_Height = 25;
QString XPInEdit::sm_BasePath;
bool EdList::sm_PartlySelected = false;

FORMULAPAINTER_EXPORT MapColor MapColor::s_MapColor;

QHash<uchar, wchar_t> Recode;

void CreateRecode()
  {
  Recode[msMultSign2] = 0xd7;
  Recode[msMultSign1] = 0x22c5; // 0x2022
  Recode[msBigAlpha] = 0x0391;
  Recode[msBigBetta] = 0x0392;
  Recode[msBigGamma] = 0x0393;
  Recode[msBigDelta] = 0x0394;
  Recode[msBigEpsilon] = 0x0395;
  Recode[msBigDZeta] = 0x0396;
  Recode[msBigEta] = 0x0397;
  Recode[msBigTeta] = 0x0398;
  Recode[msBigIota] = 0x0399;
  Recode[msBigKappa] = 0x039a;
  Recode[msBigLambda] = 0x039b;
  Recode[msBigMu] = 0x039c;
  Recode[msBigNu] = 0x039d;
  Recode[msBigKsi] = 0x039e;
  Recode[msBigOmicron] = 0x039f;
  Recode[msBigPi] = 0x03a0;
  Recode[msBigRo] = 0x03a1;
  Recode[msBigSigma] = 0x03a3;
  Recode[msBigTau] = 0x03a4;
  Recode[msBigYpsilon] = 0x03a5;
  Recode[msBigFi] = 0x03a6;
  Recode[msBigHi] = 0x03a7;
  Recode[msBigPsi] = 0x03a8;
  Recode[msBigOmega] = 0x03a9;
  Recode[msAlpha] = 0x03b1;
  Recode[msBetta] = 0x03b2;
  Recode[msGamma] = 0x03b3;
  Recode[msDelta] = 0x03b4;
  Recode[msEpsilon] = 0x03b5;
  Recode[msDZeta] = 0x03b6;
  Recode[msEta] = 0x03b7;
  Recode[msTeta] = 0x03b8;
  Recode[msIota] = 0x03b9;
  Recode[msKappa] = 0x03ba;
  Recode[msLambda] = 0x03bb;
  Recode[msMu] = 0x03bc;
  Recode[msNu] = 0x03bd;
  Recode[msKsi] = 0x03be;
  Recode[msOmicron] = 0x03bf;
  Recode[msPi] = 0x03c0;
  Recode[msRo] = 0x03c1;
  Recode[msSigmaX] = 0x03c2;
  Recode[msSigma] = 0x03c3;
  Recode[msTau] = 0x03c4;
  Recode[msYpsilon] = 0x03c5;
  Recode[msFi] = 0x03c6;
  Recode[msHi] = 0x03c7;
  Recode[msPsi] = 0x03c8;
  Recode[msOmega] = 0x03c9;
  Recode[msBird] = 0x2222;
  Recode[msApprox] = 0x2248;
  Recode[msRound] = 0x223c;
  Recode[msAround] = 0x2245;
  Recode[msParallel] = 0x2225;
  Recode[msNotequal] = 0x2260;
  Recode[msIdentity] = 0x2261;
  Recode[msMinequal] = 0x2264;
  Recode[msMaxequal] = 0x2265;
  Recode[msPlusMinus] = 0x00b1;
  Recode[msMinusPlus] = 0x2213;
  Recode[msCross] = 0x22a5;
  Recode[msTriangle] = 0x25b3;
  Recode[ms3Points] = 0x2026;
  Recode[msAddition] = 0x2208;
  Recode[msInfinity] = 0x221e;
  Recode[msImUnit] = 0x2139;
  Recode[msCConjugation] = 0x0305;
  Recode[msMean] = 0x0305;
  Recode[msArrowToRight] = 0x2192;
  Recode[msDoubleArrow] = 0x2194;
  Recode[msIntersection] = 0x2229;
  Recode[msStrictInclusion] = 0x2282;
  Recode[msUnstrictInclusion] = 0x2286;
  Recode[msGenQuantifier] = 0x2200;
  Recode[msExQuantifier] = 0x2203;
  Recode[msArrowsUp] = 0x21c8;
  Recode[msArrowsUpDown] = 0x21c5;
  Recode[msNotBelong] = 0x2209;
  Recode[msIntegral] = 0x222b;
  Recode[msDoubleIntegral] = 0x222C;
  Recode[msTripleIntegral] = 0x222D;
  Recode[msContourIntegral] = 0x222E;
  Recode[msUnion] = 0x222a;
  Recode[msDegree] = 0xb0;
  Recode[msMinute] = 0xb4;
  Recode[msMean] = 0x305;
  Recode[msPartialDiff] = 0x2202;
  Recode[msPrime] = 0x2032;
  Recode[msDoublePrime] = 0x2033;
  Recode[msTriplePrime] = 0x2034;
  Recode[msInvisible] = msInvisible;
  Recode[msCConjugation] = 'z';
  Recode[msLongMinus] = '-';
  Recode[msLongPlus] = '+';
  Recode[msIdle] = 0x2021;
  Recode[msBrackLeftLower] = '{';
  Recode[msBrackRightLower] = '}';
  }

MapColor::MapColor()
  {
  ( *this )["Aqua"] = 0x00FFFF;
  ( *this )["Black"] = 0;
  ( *this )["Blue"] = 0x0000ff;
  ( *this )["DarkGray"] = 0x808080;
  ( *this )["Fuchsia"] = 0xff00ff;
  ( *this )["Gray"] = 0xa0a0a4;
  ( *this )["Green"] = 0x00ff00;
  ( *this )["Lime"] = 0x00FF00;
  ( *this )["LightGray"] = 0xc0c0c0;
  ( *this )["Maroon"] = 0x800000;
  ( *this )["Navy"] = 0x000080;
  ( *this )["Olive"] = 0x808000;
  ( *this )["Purple"] = 0x800080;
  ( *this )["Red"] = 0xff0000;
  ( *this )["Silver"] = 0xC0C0C0;
  ( *this )["Teal"] = 0x008080;
  ( *this )["White"] = 0xFFFFFF;
  ( *this )["Yellow"] = 0xffff00;
  }

QChar ToUnicode(uchar Smb)
  {
  if( XPInEdit::sm_TextFont || Smb < 127 && Smb != msMultSign2 && Smb != msMultSign1 && Smb != msIdle) return Smb;
//  if (XPInEdit::sm_EditKeyPress)
//    return EdBaseChar::sm_pCodec->toUnicode( (const char*) &Smb, 1)[0];
  return Recode[Smb];
  }

QByteArray FromUnicode(const QString& S)
  {
  QByteArray Result;
  for (int i = 0; i < S.count(); i++)
    {
    QChar C = S[i];
    uchar c = C.toLatin1();
    if(c == 'z' || c == '-' || c == '+')
      Result += c;
    else
      {
      QList<uchar> L = Recode.keys(C.unicode());
      if (L.isEmpty())
        Result += c;
      else
        Result += L[0];
      }
    }
  return Result;
  }

long long Round( double Val)
  {
  return floor( Val + 0.5 );
  }
enum Param {arVect, arLim};

bool IsNumber( QByteArray &V )
  {
  bool Result;
  V.toInt( &Result );
  return Result;
  }

void DefineArrow (int X1, int Y1, int X2, int Y2, int TypArr, int &EndX1, int &EndY1, int &EndX2, int &EndY2)
  {
  double FirstAngle, SecondAngle, VectorLength;

  int ArrowLength;
  int ArrowLengthMin [] = {9,5};
  int ArrowLengthMax [] = {24, 20};
  int VectorLengthMin [] = {50, 10};
  int VectorLengthMax [] = {120, 60};

  VectorLength = X2-X1;
  ArrowLength = Round( ArrowLengthMax[TypArr] - ( ( VectorLengthMax[TypArr] - VectorLength ) *
    ( ArrowLengthMax[TypArr] - ArrowLengthMin[TypArr] ) ) / ( VectorLengthMax[TypArr] - VectorLengthMin[TypArr] ) );

  if( ArrowLength > Round( ArrowLengthMax[TypArr] ) )
    ArrowLength = Round( ArrowLengthMax[TypArr] );

  if( ArrowLength < Round( ArrowLengthMin[TypArr] ) )
    ArrowLength = Round( ArrowLengthMin[TypArr] );

  FirstAngle = M_PI/4;
   SecondAngle = - M_PI/4;
  EndX1 = X2 - Round (ArrowLength * sin (FirstAngle));
  EndY1 = Y2 - Round (ArrowLength * cos (FirstAngle));
  EndX2 = X2 - Round (ArrowLength * cos (SecondAngle));
  EndY2 = Y2 - Round (ArrowLength * sin (SecondAngle));
  }

//#define DebugBorder

XPInEdit::XPInEdit( const TPoint &P, QPainter *pCanvas, EditSets &AEditSets, const ViewSettings& VS ) : m_pCanvas( pCanvas ), m_Active( true ), m_EditSets( AEditSets ),
m_Visible(true), m_RecalcSpaces(false), m_pCurrentTable(NULL), m_IsInterval(false), m_IndReg(irNormal), m_CreateSystem(false),
  m_pMyImage( nullptr ), m_SavedKind( fkExtern ), m_FontKind( fkMain ), m_ViewSettings( VS ), m_InsideEditor(false), m_pActiveStr(nullptr)
  {
  for( int i = 0; i < 4; m_CharHeight[i++] = 0 );
  SetFont( m_EditSets.m_MainFont );
  m_Start = P;
  m_pL = new EdList( this );
  m_pL->m_Start = P;
  m_pL->m_pSub_L = m_pL;
  m_pWForMeasure = new EdChar( msBetta, this );
  /*
  QFile Fout( "Recode.js" );
  Fout.open( QIODevice::WriteOnly );
  Fout.write( "var Recode = {\r\n" );
  bool Comma = false;
  for( auto pPair = Recode.begin(); pPair != Recode.end(); pPair++ )
    {
    if( pPair.value() < 128 ) continue;
    if( Comma ) Fout.write( ",\r\n" );
    Comma = true;
    Fout.write( QByteArray::number( pPair.value() ) + ":'!0x" + QByteArray::number( pPair.key(), 16 ) + "!'" );
    }
  Fout.write( "\r\n};\r\n" );
  */
  }

XPInEdit::XPInEdit(const QByteArray& Formula, EditSets &AEditSets, const ViewSettings& VS) : m_pCanvas(new QPainter), m_pActiveStr(nullptr),
  m_Active(true), m_EditSets(AEditSets), m_ViewSettings(VS), m_Visible(true), m_RecalcSpaces(false), m_CreateSystem(false),
  m_pCurrentTable(nullptr), m_IsInterval(false), m_IndReg(irNormal), m_SavedKind(fkExtern), m_FontKind(fkMain), m_InsideEditor(false)
  {
  for( int i = 0; i < 4; m_CharHeight[i++] = 0 );
  m_pMyImage = new QImage( 100, 50, QImage::Format_ARGB32 );
  m_pCanvas->begin( m_pMyImage );
  SetFont( m_EditSets.m_MainFont );
  m_pL = new EdList( this );
  m_pL->m_Start = m_Start;
  m_pL->m_pSub_L = m_pL;
  m_pWForMeasure = new EdChar( msBetta, this );
  RestoreFormula( Formula );
  PreCalc( m_Start, m_Size, m_Axis );
  int dY = m_Size.height() / 2 - m_Axis;
  int dSize = sm_AddHeight;
  if( dY > 0 )
    {
    m_Start.Y += dY;
    dSize += dY / 2;
    }
  else
    dSize -= dY * 2;
  PreCalc( m_Start, m_Size, m_Axis );
  m_pCanvas->end();
  delete m_pMyImage;
  m_pMyImage = new QImage( m_Size.width() + 2, m_Size.height() + dSize, QImage::Format_ARGB32 );
  m_pCanvas->begin( m_pMyImage );
  m_pMyImage->fill( m_EditSets.m_BkgrColor );
  m_pL->Draw( m_Start );
  m_pCanvas->end();
  }

void XPInEdit::FreeContents()
  {
  delete m_pL;
  }

XPInEdit::~XPInEdit()
  {
  FreeContents();
  delete m_pCanvas;
  delete m_pMyImage;
  }

void XPInEdit::SetFont( const QFont &Font )
  {
  if( sm_TextFont ) return;
  m_SavedKind = m_FontKind;
  if( m_EditSets.m_MainFont == Font )
    m_FontKind = fkMain;
  else
    if( m_EditSets.m_PowrFont == Font )
      m_FontKind = fkPower;
    else
      if( m_EditSets.m_IndxFont == Font )
        m_FontKind = fkIndex;
      else
        m_FontKind = fkExtern;
  m_pCanvas->setFont( Font );
  m_CharHeight[fkExtern] = 0;
  if( m_FontKind != fkExtern && m_CharHeight[m_FontKind] > 0 ) return;
  QRect R = CharRect( ::ToUnicode( msMinusPlus ) );
  m_TopChar[m_FontKind] = R.top() - 1;
  R = CharRect( ::ToUnicode( msBetta ) );
  m_CharHeight[m_FontKind] = R.bottom() - m_TopChar[m_FontKind];
  m_SpaceChar[m_FontKind] = max(R.width() / 5, 2 );
  }

void XPInEdit::RestoreFont()
  {
  switch( m_SavedKind )
    {
    case fkMain:
      m_pCanvas->setFont( m_EditSets.m_MainFont );
      break;
    case fkIndex:
      m_pCanvas->setFont( m_EditSets.m_IndxFont );
      break;
    case fkPower:
      m_pCanvas->setFont( m_EditSets.m_PowrFont );
      break;
    case fkExtern:
      return;
    }
  m_FontKind = m_SavedKind;
  }

void XPInEdit::SetCharFont()
  {
  switch( m_IndReg )
    {
    case irIndex:
      SetFont( m_EditSets.m_IndxFont );
      return;
    case irPower:
      SetFont( m_EditSets.m_PowrFont );
      return;
    }
  SetFont( m_EditSets.m_MainFont );
  }

void XPInEdit::SetTextFont()
  {
  m_pCanvas->setFont( m_ViewSettings.m_SimpCmFont );
  QString Name = m_ViewSettings.m_SimpCmFont.family();
  sm_TextFont = true;
  }

void XPInEdit::SetMathFont()
  {
  sm_TextFont = false;
  SetFont( m_ViewSettings.m_TaskCmFont );
  }

void XPInEdit::SetPen( const QPen& Pen )
  {
  m_OldPen = m_pCanvas->pen();
  m_pCanvas->setPen( Pen );
  }

void XPInEdit::RestorePen()
  {
  m_pCanvas->setPen( m_OldPen );
  }

void XPInEdit::SetPaintColor( QColor C )
  {
  QPen Pen( m_pCanvas->pen() );
  m_OldPenColor = Pen.color();
  Pen.setColor( C );
  m_pCanvas->setPen( Pen );
  }

void XPInEdit::RestorePaintColor()
  {
  QPen Pen( m_pCanvas->pen() );
  Pen.setColor( m_OldPenColor );
  m_pCanvas->setPen( Pen );
  }

void XPInEdit::SetBrushColor( QColor C )
  {
  QBrush Brush( m_pCanvas->brush() );
  m_OldBrushColor = Brush.color();
  Brush.setColor( C );
  Brush.setStyle( Qt::SolidPattern );
  m_pCanvas->setBrush( Brush );
  }

void XPInEdit::RestoreBrushColor()
  {
  QBrush Brush( m_pCanvas->brush() );
  Brush.setColor( m_OldBrushColor );
  m_pCanvas->setBrush( Brush );
  }

void XPInEdit::SetPenStyle( Qt::PenStyle S )
  {
  QPen Pen( m_pCanvas->pen() );
  m_OldPenStyle = Pen.style();
  Pen.setStyle( S );
  m_pCanvas->setPen( Pen );
  }

void XPInEdit::RestorePenStyle()
  {
  QPen Pen( m_pCanvas->pen() );
  Pen.setStyle( m_OldPenStyle );
  m_pCanvas->setPen( Pen );
  }

QString XPInEdit::ToUnicode( const QString &Text  )
  {
  QString UString;
  for( int iChar = 0; iChar < Text.length(); UString += ::ToUnicode( Text[iChar++].toLatin1() ) );
  return UString;
  }

int XPInEdit::CharWidth( QChar Char, int Kern )
  {
  if( Char == msInvisible ) return 0;
  int Width = m_pCanvas->fontMetrics().boundingRect( Char ).width();
  if( m_FontKind == fkExtern )
    Width -= m_pCanvas->fontMetrics().rightBearing( Char ) * Kern;
  else
    Width += m_SpaceChar[m_FontKind];
  return Width;
  }

int XPInEdit::TextHeight( const QString &Text )
  {
  return m_pCanvas->fontMetrics().boundingRect( ToUnicode(Text) ).height();
  }

int XPInEdit::ETextHeight( const QString &Text )
  {
  return m_pCanvas->fontMetrics().tightBoundingRect( ToUnicode( Text ) ).height();
  }

int XPInEdit::ETextWidth( const QString &Text )
  {
  return m_pCanvas->fontMetrics().tightBoundingRect( ToUnicode( Text ) ).width();
  }

int XPInEdit::TextWidth( const QString &Text )
  {
  return m_pCanvas->fontMetrics().boundingRect( Text ).width();
  }
/*
int XPInEdit::TextWidth( const QString &Text )
{
int Result = m_pCanvas->fontMetrics().width(Text);
if( !m_pCanvas->fontInfo().italic() ) return Result;
Result = Round( Result * 1.3 );
if( Text.length() > 1 ) return Result;
QChar T = Text[0];
switch( T.cell() )
{
case 'f' : return Round( Result * 1.3 );
case '/' : return Round( Result * 1.2 );
case 'I' :
case 'J' :
return Round( Result * 1.1 );
}
return Result;
}
*/

int XPInEdit::CharHeight( QChar Char )
  {
  if( m_CharHeight[fkExtern] > 0 ) return m_CharHeight[fkExtern];
  QRect R = CharRect( Char );
  m_CharHeight[fkExtern] = R.height();
  m_TopChar[fkExtern] = R.top();
  return m_CharHeight[fkExtern];
  }

int XPInEdit::CharHeight()
  {
  return m_CharHeight[m_FontKind];
  }

bool XPInEdit::GlobalSetCurrent(int x, int y)
  {
  if( m_pL->m_pFirst == nullptr ) return false;
  EdList* pSL;
  EdMemb* pCr;
  if( IsConstEdType( EdChartEditor, m_pL->m_pFirst->m_pMember.data() ) )
    return m_pL->m_pFirst->SetCurrent( TPoint( x, y ), pSL, pCr );
  return true;
  }
/*
function TXPInEdit.GlobalSetCurrent(x,y: integer): boolean; // 26.01.01
var
  EdList: TEdList;
  EdMemb: TEdMemb;
{
  CurrentTable := nil;
  Result := L.SetCurrent(Point(x,y),EdList,EdMemb)
};

Destructor TXPInEdit.Destroy;
{
        FEditSets.Free;
        FViewSets.Free;
        FreeContents;
        Inherited Destroy;
};

*/

void XPInEdit::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  m_pL->PreCalc( P, m_Size, m_Axis );
  if( m_pL->Splitted() )
    m_Axis = m_Size.height() / 2;
  S = m_Size;
  A = m_Axis;
  }

void XPInEdit::EditDraw()
  {
  if( m_SelectedRect.isValid() )
    m_pCanvas->fillRect( m_SelectedRect, ~m_EditSets.m_BkgrColor.rgb() );
  if(m_pCurrentTable != nullptr)
     m_pL = m_pCurrentTable->m_pOwnerList;
  m_pL->PreCalc( m_Start, m_Size, m_Axis );
  m_pL->Draw( m_Start );
  }

void XPInEdit::EditDraw( TPoint NewStart )
  {
  m_pL->PreCalc( NewStart, m_Size, m_Axis );
  m_pL->Draw( NewStart );
  }

void XPInEdit::SelectRect( const QRect& R )
  {
  if( m_SelectedRect.isValid() )
    m_SelectedRect |= R;
  else
    m_SelectedRect = R;
  }

void XPInEdit::Select()
  {
  m_pL->Select();
  EditDraw();
  }

bool XPInEdit::SelectString(int Y)
  {
  bool Result = m_pL->SelectString(Y);
  EditDraw();
  return Result;
  }

void XPInEdit::SetRegister(IndReg SymReg)
  {
  if (m_pL->m_pSub_L->m_pFirst == nullptr) return;
  m_IndReg = SymReg;
  if (SymReg != irNormal)
    {
    EdMemb *pE = m_pL->m_pSub_L->Append_Before(new EdChar(msIdle, this));
    m_pL->m_pSub_L->m_pCurr = pE;
    }
  ClearSurface();
  EditDraw();
  }

void XPInEdit::ResetIndex()
  {
  EdChar *pEch = nullptr;
  EdMemb *pMemb = m_pL->m_pSub_L->m_pCurr;
  if (pMemb == nullptr) pMemb = m_pL->m_pSub_L->m_pLast;
  if(pMemb != nullptr ) pEch = dynamic_cast<EdChar*>(pMemb->m_pMember.data());
  if (pEch == nullptr)
    m_IndReg = irNormal;
  else
    m_IndReg = pEch->m_ind;
  }

EdAction XPInEdit::EditAction( U_A_T Uact  )
  {
  int MaxVarLen = 1, VarLen;
  bool Simplevar= false;
  if ( m_pL->m_pSub_L->m_pMother != nullptr )
    {
    if( TypeConvert( EdVect ) != NULL )
      {
      Simplevar= true;
      MaxVarLen = 2;
      }
    }
  if( Uact.act == actPrintable ) // { create & append text`s element }
    {
    bool IsMother = m_pL->m_pSub_L->m_pMother != nullptr;
    m_CreateSystem = IsMother && TypeConvert( EdSyst ) != nullptr;
    bool AllowedChar = ( m_pL->m_pSub_L->m_pMother == nullptr || Uact.c() != ';' && Uact.c() != msCharNewLine ) ||
      ( IsMother && ( m_CreateSystem || TypeConvert( EdMatr ) != nullptr ) && Uact.c() == ';' );
    //  ( L.Sub_L.mother.member is TEdDetMult ) or   // NP 03.01.2000
    //  ( L.Sub_L.mother.member is TEdDetRMult ) or  // NP 03.01.2000
    //  ( L.Sub_L.mother.member is TEdDetDiv ) or    // NP 06.01.2000
    //  ( L.Sub_L.mother.member is TEdDetRDiv ) or   // NP 21.01.2000
    //  ( L.Sub_L.mother.member is TEdDetRSumm ) or  // NP 16.01.2000
    //  ( L.Sub_L.mother.member is TEdDetSumm ) or   // DvsL 30.11.99
    //    AllowedChar := True;
//    if( m_pL->m_pSub_L->m_pMother != NULL  &&
//      TypeConvert( EdSyst ) != NULL && ( Uact.c() == ';' ) )
//        AllowedChar = true;

    if( Simplevar)
      {
      VarLen = 0;
      EdMemb *pElementM = m_pL->m_pSub_L->m_pFirst;
      while( pElementM != NULL )
        {
        EdChar *pEch = dynamic_cast<EdChar*>(pElementM->m_pMember.data());
        if (pEch == nullptr)
          VarLen = 3;
        else
          if(pEch->c() != msIdle )
            VarLen++;
        pElementM = pElementM->m_pNext;
        }
      if( VarLen >= MaxVarLen )
        AllowedChar = false;
      }

    if( m_pL->m_pSub_L->m_pMother != nullptr && sm_pMatr != nullptr && !m_Active && Uact.c() == ',' )
      Uact = ' ';

    if( AllowedChar )
      {
      EdTable *pTable = nullptr;
      if(m_pL->m_pSub_L->m_pMother != nullptr ) pTable = TypeConvert( EdTable );
      if( pTable != nullptr && pTable->m_GridState != TGRVisible )
          {
          EdTable& elementT = *pTable;
//          if( elementT.m_GridState == TGRUnvisible && m_pL->m_pSub_L->m_pFirst != nullptr ) elementT.InsertColumn();
//          if( elementT.m_Col < elementT.m_ColCount - 1 && m_pL->m_pSub_L->m_pFirst != nullptr )
//            elementT.MoveToNext( m_pL->m_pSub_L );
          if( elementT.m_GridState == TGRUnvisible && m_pL->m_pSub_L->m_pFirst != nullptr )
            m_pL->m_pSub_L->MemberDelete(m_pL->m_pSub_L->m_pFirst);
          if( m_pL->m_pSub_L->m_pFirst == nullptr )
            {
            uchar S[] = { '+', '-', msMultSign2, ':', '=', '(', ')' };
            if( In( Uact.c(), S ) )
              m_pL->m_pSub_L->Append_Before( new EdStr( this, SimpleChar( Uact.c() ) ) );
            else
              m_pL->m_pSub_L->Append_Before( new EdChar( Uact.c(), this ) );
//          while( elementT.m_Col < elementT.m_ColCount - 1 && m_pL->m_pSub_L->m_pFirst != nullptr &&
//            elementT.MoveToNext( m_pL->m_pSub_L ) );
            }
          }
        else
          m_pL->m_pSub_L->Append_Before(new EdChar(Uact.c(), this));
      return edRefresh;
      }
    return edNone;
    }
  if( Uact.act == actCtrlKey )
    {
    if( Uact == "vk_Back" )
      {
      if( m_pCurrentTable != nullptr && m_pCurrentTable->m_GridState == TGRUnvisible )
        {
        if( m_pCurrentTable->m_Col != 0 )
          {
          m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
          if( m_pCurrentTable->CanCellEmpty() )
            m_pL->m_pSub_L->NextDelete();
          }
        }
      else
        m_pL->m_pSub_L->PrevDelete();
      ResetIndex();
      return edRefresh;
      }
    if( Uact == "vk_Delete" )
      {
      if(sm_EditString)
        {
        Clear();
        return edRefresh;
        }
      if( m_pCurrentTable != nullptr && m_pCurrentTable->m_ColCount > 1)
        if( m_pL->m_pSub_L->m_pFirst == nullptr && m_pCurrentTable->m_GridState == TGRUnvisible )
          m_pCurrentTable->DelColumn();
        else
          {
          if( m_pCurrentTable->CanCellEmpty())
            m_pL->m_pSub_L->NextDelete();
          }
        else
         m_pL->m_pSub_L->NextDelete();
      ResetIndex();
      return edRefresh;
      }
    if( Uact == "vk_Up" )
      {
      m_IndReg = irNormal;
      m_pL->m_pSub_L->MoveUp( m_pL->m_pSub_L );
      return edCursor;
      }
    if( Uact == "vk_Down" )
      {
      m_IndReg = irNormal;
      m_pL->m_pSub_L->MoveDown( m_pL->m_pSub_L );
      return edCursor;
      }
    if( Uact == "vk_Left" )
      {
      if (m_IndReg != irNormal && m_pL->m_pSub_L->m_pCurr != 0)
        {
        EdChar *pEch = nullptr;
        if(m_pL->m_pSub_L->m_pCurr->m_pPrev != nullptr )  pEch = dynamic_cast<EdChar*>(m_pL->m_pSub_L->m_pCurr->m_pPrev->m_pMember.data());;
        if (pEch == nullptr || pEch->m_ind == irNormal)
          {
          m_IndReg = irNormal;
          return edCursor;
          }
        }
      if( m_pCurrentTable != nullptr && m_pCurrentTable->m_GridState == TGRUnvisible )
        m_pCurrentTable->MoveToPrev( m_pL->m_pSub_L );
      else
        m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
      ResetIndex();
      return edCursor;
      }
    if( Uact == "vk_Right" )
      {
      if (m_pL->m_pSub_L->m_pCurr == nullptr && m_IndReg != irNormal)
        {
        m_IndReg = irNormal;
        return edCursor;
        }
//      if( m_pCurrentTable == nullptr || m_pCurrentTable->m_GridState != TGRUnvisible ||
      if( m_pCurrentTable == nullptr || !m_pCurrentTable->MoveToNext( m_pL->m_pSub_L ) )
        {
        m_pL->m_pSub_L->MoveRight( m_pL->m_pSub_L );
        m_pCurrentTable = nullptr;
        }
      ResetIndex();
      return edCursor;
      }
    if( Simplevar) return edBeep;
    }

  if( Uact == "ROOT" )
    {
    EdTwo *pElement = new  EdRoot( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElement );
    pElement->m_pAA->m_pMother = pAppending;
    pElement->m_pBB->m_pMother = pAppending;
        m_pL->m_pSub_L = pElement->m_pBB;
    return edRefresh;
    }

  if (Uact == "SQROOT")
    {
    EdRoot *pElement = new  EdSqRoot( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElement );
    pElement->m_pAA->m_pMother = pAppending;
    pElement->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pElement->m_pAA;
    return edRefresh;
    }

  if (Uact=="ABS")
    {
    EdAbs *pElementA = new EdAbs( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementA);
    pElementA->m_pAA->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementA->m_pAA;
    return edRefresh;
    }

  if (Uact=="FRACTION")
    {
    EdTwo *pElement = new EdFrac (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElement );
    pElement->m_pAA->m_pMother=pAppending;
    pElement->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L=pElement->m_pAA;
    return edRefresh;
    }

  if (Uact == "INTEGRAL")
    {
    EdIntegr *pElementI = new EdIntegr (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElementI);
    pElementI->m_pAA->m_pMother = pAppending;
    pElementI->m_pBB->m_pMother = pAppending;
    pElementI->m_pDD->m_pMother = pAppending;
    m_pL->m_pSub_L=pElementI->m_pAA;
    if (m_Active)
      {
      m_pL->m_pSub_L->Append_Before(new EdChar ('(', this));
      m_pL->m_pSub_L->Append_Before(new EdChar (')', this));
      m_pL->m_pSub_L->MoveLeft(m_pL->m_pSub_L);
      }
    return edRefresh;
    }

  if (Uact == "LG")
    {
    EdLg *pElementLg = new EdLg (this, "lg");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }

  if (Uact == "LN")
    {
    EdLg *pElementLg = new EdLg (this, "ln");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElementLg);
    pElementLg->m_pAA->m_pMother = pAppending;
    pElementLg->m_pNN->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementLg->m_pAA;
    return edRefresh;
    }

  if (Uact == "POWER")
    {
    EdTwo *pElement = new EdPowr (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElement);
    pElement->m_pAA->m_pMother = pAppending;
    pElement->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L=pElement->m_pAA;
    return edRefresh;
    }

  if (Uact == "EXP")
    {
    EdLg *pElementLg = new EdLg (this, "exp");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "SIN")
    {
    EdLg *pElementLg = new EdLg (this, "sin");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "COS")
    {
    EdLg *pElementLg = new EdLg (this, "cos");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "TAN")
    {
    EdLg *pElementLg = new EdLg (this, "tan");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "COT")
    {
    EdLg *pElementLg = new EdLg (this, "cot");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "ARCSIN")
    {
    EdLg *pElementLg = new EdLg (this, "arcsin");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "ARCCOS")
    {
    EdLg *pElementLg = new EdLg (this, "arccos");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "ARCTAN")
    {
    EdLg *pElementLg = new EdLg (this, "arctan");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }
  if (Uact == "ARCCOT")
    {
    EdLg *pElementLg = new EdLg (this, "arccot");
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementLg);
    pElementLg->m_pAA->m_pMother=pAppending;
    pElementLg->m_pNN->m_pMother=pAppending;
    m_pL->m_pSub_L=pElementLg->m_pAA;
    return edRefresh;
    }

  if( Uact == "TRIGOCOMP" )
    {
    EdAbs *pElementA = new TEdTrigoComp( this, sm_Text );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementA );
//    elementA.AA.Mother = appending;
//    L.Sub_L = elementA.AA;
    return edRefresh;
    }

  if (Uact == "MATRIX")
    {
    EdAbs *pElementA = new EdMatr( sm_Text, this );
    ( dynamic_cast <EdMatr *> (pElementA) )->m_IsVisible=true;
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementA);
    pElementA->m_pAA->m_pMother = pAppending;
    m_pL->m_pSub_L=pElementA->m_pAA;
    return edRefresh;
    }

  if( Uact == "MATRIX_UNV" )
    {
    EdAbs *pElementA = new EdMatr( sm_Text, this );
    ( dynamic_cast <EdMatr *> ( pElementA ) )->m_IsVisible = false;
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementA );
    pElementA->m_pAA->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementA->m_pAA;
    return edRefresh;
    }
  if( Uact == "SPACER" )
    {
    if(sm_pMatr != nullptr) sm_pMatr->AddSpacer();
    return edRefresh;
    }

  if( Uact == "SETVISIBLE" )
    {
    SetVisible(true);
    return edRefresh;
    }

  if( Uact == "SETUNVISIBLE" )
    {
    SetVisible( false );
    return edRefresh;
    }

  if( Uact == "TABLE" )
    {
    m_pCurrentTable = new EdTable( this, sm_Text );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( m_pCurrentTable );
    m_pCurrentTable->SetAppending( *pAppending );
    return edRefresh;
    }

  if( Uact == "CREATETABLE" )
    {
    CreateTableDialog Dlg;
    if( Dlg.exec() == QDialog::Rejected ) return edNone;
    m_pCurrentTable = new EdTable( this, Dlg.GetResult() );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( m_pCurrentTable );
    m_pCurrentTable->SetAppending( *pAppending );
    m_pCurrentTable->m_pOwnerList->m_pSub_L->MoveLeft( m_pCurrentTable->m_pOwnerList->m_pSub_L );
//    m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
    return edRefresh;
    }

  if (Uact == "CREATEMATRIX")
    {
    CreateMatrixDialog Dlg;
    if (Dlg.exec() == QDialog::Rejected) return edNone;
    QByteArray Result(Dlg.GetResult());
    EdMatr *pMatr = new EdMatr(Result.mid(1), this, true);
    if (Result[0] == 'D')
      {
      EdAbs *pElementA = new EdAbs(this);
      EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementA);
      pElementA->m_pAA->m_pMother = pAppending;
      m_pL->m_pSub_L = pElementA->m_pAA;
      pMatr->m_IsVisible = false;
      }
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pMatr);
    pMatr->m_pAA->m_pMother = pAppending;
    m_pL->m_pSub_L = pMatr->m_pAA;
    return edRefresh;
    }

  if( Uact == "CELL" )
    {
    m_pL = m_pCurrentTable->GetCell();
    return edRefresh;
    }

  if( Uact == "ENDTABLE" )
    {
    if( !m_pCurrentTable->m_NoFreeze ) m_pCurrentTable->Freeze();
    if( !sm_IsRetryEdit && sm_pEditor == this )
      {
      m_pL = m_pCurrentTable->m_pOwnerList;
      return edRefresh;
      }
    m_pCurrentTable->m_Row = m_pCurrentTable->m_RowCount - 1;
    m_pCurrentTable->m_Col = m_pCurrentTable->m_ColCount - 1;
    m_pL = m_pCurrentTable->m_pOwnerList;
    m_pL->m_pSub_L->MoveRight( m_pL->m_pSub_L );
    return edCursor;
    }

  if( Uact == "CHART" )
    {
    EdChart *pChart = new EdChart( this, sm_Text, sm_pEditor != this );
    if( pChart->m_Labels.isEmpty() )
      delete pChart;
    else
      if( sm_pEditor == this )
        new EdChartEditor( this, pChart );
      else
        m_pL->m_pSub_L->Append_Before( pChart );
    return edRefresh;
    }

  if( Uact == "CHARTEDITOR" )
    {
    m_pCurrentTable = new EdChartEditor( this );
    return edRefresh;
    }

  if( Uact == "TEXT" )
    {
    if( m_pCurrentTable != nullptr && sm_Text == " " ) return edRefresh;
    if (m_pL->m_pSub_L->m_pMother == nullptr && m_pL->m_pSub_L->m_pFirst == nullptr && m_InsideEditor)
      {
/*
      EditString Dlg(ToLang(sm_Text), true);
      if (Dlg.exec() == QDialog::Rejected) return edNone;
      QString Result = Dlg.GetString();
      if (Result.isEmpty()) return edNone;
      QByteArray Text(FromLang(Result));
      for (int i = Text.length() - 1; i >= 0; i--)
        {
        sm_EditKeyPress = true;
        m_pL->m_pSub_L->Append_Before(new EdChar(Text[i], this));
        }
      sm_EditKeyPress = false;
      }
      */
      sm_Language == lngEnglish;
      QByteArray Text(1, '\"');
      Text += sm_Text + '\"';
      for (auto pChar = Text.begin(); pChar != Text.end(); pChar++)
        {
        m_pL->m_pSub_L->Append_Before(new EdChar(*pChar, this));
        }
      return edRefresh;
      }
    EdStr *pElementStr = new EdStr( this, sm_Text );
    m_pL->m_pSub_L->Append_Before( pElementStr );
    return edRefresh;
    }

  if (Uact == "PICTURE")
    {
    EdPicture *pPicture = new EdPicture(this, sm_Text);
    m_pL->m_pSub_L->Append_Before(pPicture);
    return edRefresh;
    }

  if (Uact == "LIMIT")
    {
    EdLimit *pElementL = new EdLimit (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementL);
    pElementL->m_pAA->m_pMother = pAppending;
    pElementL->m_pVL->m_pMother = pAppending;
    pElementL->m_pEE->m_pMother = pAppending;
    pElementL->m_pNN->m_pMother = pAppending;
    pElementL->m_pRR->m_pMother = pAppending;
    m_pL->m_pSub_L=pElementL->m_pAA;
    if (m_Active)
      {
       m_pL->m_pSub_L->Append_Before(new EdChar ('(', this));
       m_pL->m_pSub_L->Append_Before(new EdChar (')', this));
       m_pL->m_pSub_L->MoveLeft(m_pL->m_pSub_L);
      }
    return edRefresh;
    }

  if( Uact == "INTERVAL" )
    {
    EdInterval *pInterval = new EdInterval( this, sm_Text );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pInterval );
    pInterval->m_pAA->m_pMother = pAppending;
    pInterval->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pInterval->m_pAA;
    return edRefresh;
    }

  if( Uact == "IMUNIT" )
    {
    EdImUnit *pElementIm = new EdImUnit (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElementIm);
    pElementIm->m_pAA->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementIm->m_pAA;
    m_pL->m_pSub_L->MoveRight (m_pL->m_pSub_L);
    return edRefresh;
    }

  if (Uact == "DERIVATIVE")
    {
    EdDeriv *pElementD = new EdDeriv( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementD);
    pElementD->m_pAA->m_pMother = pAppending;
    pElementD->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L= pElementD->m_pAA;
    return edRefresh;
    }

  if( Uact == "STROKEDERIV" )
    {
    EdStrokeDeriv *pElementD = new EdStrokeDeriv( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementD );
    pElementD->m_pAA->m_pMother = pAppending;
    pElementD->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementD->m_pAA;
    return edRefresh;
    }

  if( Uact == "STROKEPARTDERIV" )
    {
    EdStrokePartialDeriv *pElementD = new EdStrokePartialDeriv( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementD );
    pElementD->m_pAA->m_pMother = pAppending;
    pElementD->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementD->m_pAA;
    return edRefresh;
    }

  if( Uact == "PARTDERIV" )
    {
    EdPartialDeriv *pElementD = new EdPartialDeriv( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementD );
    pElementD->m_pAA->m_pMother = pAppending;
    pElementD->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementD->m_pAA;
    return edRefresh;
    }


  if (Uact == "VECTOR")
    {
    EdVect *pElementA = new EdVect (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementA);
    pElementA->m_pAA->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementA->m_pAA;
    return edRefresh;
    }

  if (Uact == "PerCount")
    {
    EdPerCount *pElementPerCount = new EdPerCount (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementPerCount);
    pElementPerCount->m_pAA->m_pMother = pAppending;
    pElementPerCount->m_pB1->m_pMother = pAppending;
    pElementPerCount->m_pCC->m_pMother = pAppending;
    pElementPerCount->m_pB2->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementPerCount->m_pCC;
    return edRefresh;
    }

  if (Uact == "BinomCoeff")
    {
    EdBCoeff *pElementBCoeff = new EdBCoeff (this, 'C');
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementBCoeff);
    pElementBCoeff->m_pAA->m_pMother = pAppending;
    pElementBCoeff->m_pBB->m_pMother = pAppending;
    pElementBCoeff->m_pCC->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementBCoeff->m_pCC;
    return edRefresh;
    }

  if (Uact == "ABinomCoeff")
    {
    EdBCoeff *pElementBCoeff = new EdBCoeff (this, 'A');
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementBCoeff);
    pElementBCoeff->m_pAA->m_pMother = pAppending;
    pElementBCoeff->m_pBB->m_pMother = pAppending;
    pElementBCoeff->m_pCC->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementBCoeff->m_pCC;
    return edRefresh;
    }

  if (Uact == "FUNC")
    {
    EdFunc *pElementFunc = new EdFunc (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementFunc);
    pElementFunc->m_pAA->m_pMother = pAppending;
    pElementFunc->m_pB1->m_pMother = pAppending;
    pElementFunc->m_pBB->m_pMother = pAppending;
    pElementFunc->m_pB2->m_pMother = pAppending;
    m_pL->m_pSub_L->MoveLeft (m_pL->m_pSub_L);
    m_pL->m_pSub_L->MoveLeft (m_pL->m_pSub_L);
    return edRefresh;
    }

  if (Uact == "SYSTEM")
    {
    EdAbs *pElementA = new EdSyst (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before(pElementA);
    pElementA->m_pAA->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementA->m_pAA;
    return edRefresh;
    }

  if (Uact == "INDEX")
    {
    EdTwo *pElement = new EdIndx (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElement);
    pElement->m_pAA->m_pMother = pAppending;
    pElement->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pElement->m_pAA;
/*
    if (m_Active)
      {
      m_pL->m_pSub_L->Append_Before (new EdChar ('(', this));
      m_pL->m_pSub_L->Append_Before (new EdChar (')', this));
      m_pL->m_pSub_L->MoveLeft (m_pL->m_pSub_L);
      }
      */
    return edRefresh;
    }

  if (Uact == "MEASURED")
    {
    EdMeas *pElementR = new EdMeas (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElementR);
    pElementR->m_pAA->m_pMother = pAppending;
    pElementR->m_pBB->m_pMother = pAppending;
    pElementR->m_pOB->m_pMother = pAppending;
    pElementR->m_pCB->m_pMother = pAppending;
    m_pL->m_pSub_L=pElementR->m_pAA;
    if (m_Active)
      {
      m_pL->m_pSub_L->Append_Before(new EdChar ('(', this));
      m_pL->m_pSub_L->Append_Before(new EdChar(')', this));
      m_pL->m_pSub_L->MoveLeft(m_pL->m_pSub_L);
      }
    return edRefresh;
    }

  if (Uact == "LOG")
    {
    EdLog *pElementLog = new EdLog (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before (pElementLog);
    pElementLog->m_pAA->m_pMother = pAppending;
    pElementLog->m_pBB->m_pMother = pAppending;
    pElementLog->m_pB1->m_pMother = pAppending;
    pElementLog->m_pCC->m_pMother = pAppending;
    pElementLog->m_pB2->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementLog->m_pCC;
    return edRefresh;
    }

  if ( Uact == "DEFINTEGRAL" )
    {
    EdDfIntegr *pElementI = new EdDfIntegr (this);
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before ( pElementI );
    pElementI->m_pAA->m_pMother = pAppending;
    pElementI->m_pBB->m_pMother = pAppending;
    pElementI->m_pLL->m_pMother = pAppending;
    pElementI->m_pHL->m_pMother = pAppending;
    pElementI->m_pDD->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementI->m_pAA;
    if ( m_Active )
      {
      m_pL->m_pSub_L->Append_Before ( new EdChar ( '(', this ) );
      m_pL->m_pSub_L->Append_Before ( new EdChar ( ')', this ) );
      m_pL->m_pSub_L->MoveLeft ( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if( Uact == "DBLINTEGRAL" )
    {
    EdDoubleIntegr *pElementI = new EdDoubleIntegr( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementI );
    pElementI->m_pAA->m_pMother = pAppending;
    pElementI->m_pBB->m_pMother = pAppending;
    pElementI->m_pVaR2->m_pMother = pAppending;
    pElementI->m_pRegion->m_pMother = pAppending;
    pElementI->m_pD2->m_pMother = pAppending;
    pElementI->m_pDD->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementI->m_pAA;
    if( m_Active )
      {
      m_pL->m_pSub_L->Append_Before( new EdChar( '(', this ) );
      m_pL->m_pSub_L->Append_Before( new EdChar( ')', this ) );
      m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if( Uact == "TRPLINTEGRAL" )
    {
    EdTripleIntegr *pElementI = new EdTripleIntegr( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementI );
    pElementI->m_pAA->m_pMother = pAppending;
    pElementI->m_pBB->m_pMother = pAppending;
    pElementI->m_pVaR2->m_pMother = pAppending;
    pElementI->m_pRegion->m_pMother = pAppending;
    pElementI->m_pD2->m_pMother = pAppending;
    pElementI->m_pDD->m_pMother = pAppending;
    pElementI->m_pVaR3->m_pMother = pAppending;
    pElementI->m_pD3->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementI->m_pAA;
    if( m_Active )
      {
      m_pL->m_pSub_L->Append_Before( new EdChar( '(', this ) );
      m_pL->m_pSub_L->Append_Before( new EdChar( ')', this ) );
      m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if( Uact == "CRVINTEGRAL" )
    {
    EdCurveIntegr *pElementI = new EdCurveIntegr( this, sm_Text[0]  );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementI );
    pElementI->m_pAA->m_pMother = pAppending;
    pElementI->m_pBB->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementI->m_pAA;
    if( m_Active )
      {
      m_pL->m_pSub_L->Append_Before( new EdChar( '(', this ) );
      m_pL->m_pSub_L->Append_Before( new EdChar( ')', this ) );
      m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if ( Uact == "GSUMMA" )
    {
    sm_EditKeyPress = false;
    EdGSumm *pElementS = new EdGSumm ( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before ( pElementS );
    pElementS->m_pAA->m_pMother = pAppending;
    pElementS->m_pLL->m_pMother = pAppending;
    pElementS->m_pHL->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementS->m_pAA;
    if ( m_Active )
      {
      m_pL->m_pSub_L->Append_Before ( new EdChar ( '(', this ));
      m_pL->m_pSub_L->Append_Before ( new EdChar ( ')', this ));
      m_pL->m_pSub_L->MoveLeft ( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if ( Uact == "GMULT" )
    {
    EdGSumm *pElementS = new EdGMult ( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before ( pElementS );
    pElementS->m_pAA->m_pMother = pAppending;
    pElementS->m_pLL->m_pMother = pAppending;
    pElementS->m_pHL->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementS->m_pAA;
    if ( m_Active )
      {
      m_pL->m_pSub_L->Append_Before ( new EdChar ( '(', this ));
      m_pL->m_pSub_L->Append_Before ( new EdChar ( ')', this ));
      m_pL->m_pSub_L->MoveLeft ( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if ( Uact == "SUBST" )
    {
    EdSubst *pElementS = new EdSubst( this );
    EdMemb *pAppending = m_pL->m_pSub_L->Append_Before( pElementS );
    pElementS->m_pAA->m_pMother = pAppending;
    pElementS->m_pLL->m_pMother = pAppending;
    pElementS->m_pHL->m_pMother = pAppending;
    m_pL->m_pSub_L = pElementS->m_pAA;
    if ( m_Active )
      {
      m_pL->m_pSub_L->Append_Before ( new EdChar ( '(', this ));
      m_pL->m_pSub_L->Append_Before ( new EdChar ( ')', this ));
      m_pL->m_pSub_L->MoveLeft ( m_pL->m_pSub_L );
      }
    return edRefresh;
    }

  if( Uact == "NEWLINE" )
    {
    EdMemb *pElementM;
    char PrevChar;
    if( m_pL->m_pSub_L->m_pMother != nullptr ) return edInvalid;
    if( m_pL->m_pSub_L->m_pCurr != nullptr )
      pElementM = m_pL->m_pSub_L->m_pCurr->m_pPrev;
    else
      pElementM = m_pL->m_pSub_L->m_pLast;
    if( pElementM == nullptr || !pElementM->ElChar( PrevChar ) || !SplitOperSign( PrevChar ) ) return edInvalid;
      {
      m_pL->m_pSub_L->Append_Before( new EdChar( msCharNewLine, this ) );
      if( PrevChar != ',' && PrevChar != '=' )
        m_pL->m_pSub_L->Append_Before( new EdChar( PrevChar, this ) );
      return edRefresh;
      }
    }

  if( Uact == "INTERVALS" )
    {
    m_pL->m_pSub_L->Append_Before( new EdIntervals( this, sm_Text ) );
    return edRefresh;
    }

  if( Uact == "POLYGON" )
    {
    m_pL->m_pSub_L->Append_Before( new EdPolygon( this, sm_Text ) );
    return edRefresh;
    }

  if ( Uact.act == actMouseButton )
    {
    m_pL->m_pSub_L = m_pL;
    m_pL->m_pCurr = NULL;
    if( !m_pL->m_pSub_L->SetCurrent( Uact.coord(), m_pL->m_pSub_L, m_pL->m_pCurr ) )
      m_pL->m_pSub_L = m_pL;
    return edRefresh;
    }

  return edNone;
  }

void XPInEdit::UnRead( const QByteArray &S )
  {
  U_A_T Uact;
  if( S.length() == 1 )
    {
    Uact.act = actPrintable;
    Uact = S[0];
    }
  else
    {
    Uact = S.data();
    Uact.act = actCtrlKey;
    }
  m_Active = false;
  EditAction( Uact );
  m_Active = true;
  }

void XPInEdit::Clear( bool SaveTemplate)
  {
  EdList::sm_PartlySelected = false;
  if( SaveTemplate )
  for( EdMemb* pMemb = m_pL->m_pFirst; pMemb != NULL; pMemb = pMemb->m_pNext )
    if( IsConstEdType( EdTable, pMemb->m_pMember.data() ) )
      {
      EdTable::sm_pTemplate = dynamic_cast< EdTable* >( pMemb->m_pMember.data() );
      pMemb->m_pMember.clear();
      break;
      }
  FreeContents();
  m_pL = new EdList( this );
  m_pL->m_Start = m_Start;
  m_pL->m_pSub_L = m_pL;
  m_pCanvas->setLayoutDirection(Qt::LeftToRight);
  }

void XPInEdit::ClearSurface()
  {
  dynamic_cast< QImage* >( m_pCanvas->device() )->fill( m_EditSets.m_BkgrColor );
  }


void XPInEdit::TextOut( int X, int Y, const QString &Text, bool Shift )
  {
  if( Shift ) Y -= m_TopChar[m_FontKind];
  m_pCanvas->drawText( X, Y, Text );
  }

void XPInEdit::PaintChar( int X, int Y, QChar Char )
  {
  bool bSelected = m_SelectedRect.contains( X, Y );
  if( bSelected )
      SetPaintColor( ~m_pCanvas->pen().color().rgb() );
  m_pCanvas->drawText( X, Y - m_TopChar[m_FontKind], Char );
  if( bSelected )
    RestorePaintColor();
  }

void XPInEdit::SetPowerSize( int x, bool allow )
  {
  if( !allow ) return;
  if( x == 0 )
    m_DrawingPower = 0;
  else
    m_DrawingPower += x;
  if( m_DrawingPower == 0 )
    {
    if( m_FontKind != fkExtern ) m_FontKind = fkMain;
    SetFont( m_EditSets.m_MainFont );
    }
  if( m_DrawingPower > 0 )
    {
    if( m_FontKind != fkExtern ) m_FontKind = fkPower;
    SetFont( m_EditSets.m_PowrFont );
    }
  if( m_DrawingPower < 0 )
    {
    if( m_FontKind != fkExtern ) m_FontKind = fkIndex;
    SetFont( m_EditSets.m_IndxFont );
    }
  }

void XPInEdit::SetRegSize( int ind )
  {
  if( m_DrawingPower > 0 )
    {
    SetFont( m_EditSets.m_PowrFont );
    return;
    }
  if( m_DrawingPower < 0 )
    {
    SetFont( m_EditSets.m_IndxFont );
    return;
    }
  SetFont( m_EditSets.m_MainFont );
  if( ind == 0 || ind == 2 ) return;
  if( ind > 0 )
    SetFont( m_EditSets.m_PowrFont );
  else
    SetFont( m_EditSets.m_IndxFont );
  }

void XPInEdit::SizeEmptySign( QSize &Size, int &Axis )
  {
  Size.setWidth( CharWidth( 'X' ) );
  Size.setHeight( CharHeight() );
  Axis = Size.height() / 2;
  }

void XPInEdit::Line( int X1, int Y1, int X2, int Y2 )
  {
  const QPen &Pen = m_pCanvas->pen();
  m_pCanvas->drawLine( X1, Y1, X2, Y2 );
  if( Pen.isSolid() || Pen.width() == 1 ) return;
  int DotLen = Pen.width() + 2;
  if( X1 == X2 )
    {
    int y = Y1;
    if( Y2 < Y1 )
      {
      y = Y2;
      Y2 = Y1;
      }
    while( y < Y2 )
      {
      m_pCanvas->drawLine( X1, y, X1, min(y + DotLen, Y2) );
      y += DotLen * 2;
      }
    return;
    }
  if( Y1 == Y2 )
    {
    int x = Y1;
    if( X2 < X1 )
      {
      x = X2;
      X2 = X1;
      }
    while( x < X2 )
      {
      m_pCanvas->drawLine( x, Y1, min( x + DotLen, X2 ), Y1 );
      x += DotLen * 2;
      }
    return;
    }
  if( X2 < X1 )
    {
    int x = X2;
    X2 = X1;
    X1 = x;
    int y = Y2;
    Y2 = Y1;
    Y1 = y;
    }
  double k = (Y2 - Y1)/(X2 - X1);
  double a = Y2 - k * X2;
  DotLen = max( 1LL, Round( abs( 1/sqrt( 1 + k*k ) ) * DotLen ) );
  int x = X1;
  int y = Y1;
  while( x < X2 )
    {
    m_pCanvas->drawLine( x, y, min( x + DotLen, X2 ) - 1, Round( a + k * x ) );
    x += DotLen + 1;
    y = Round( a + k * x );
    }
  }

void XPInEdit::DrawAsEmptySign( TXPGrEl *pEl )
  {
  int dH = Round( ( pEl->m_Size.height() * 2 ) / 3 );
  int dT = ( pEl->m_Size.height() - dH ) / 2;
  int StartX = pEl->m_Start.X;
  int StartY = pEl->m_Start.Y;
  int SizecX = pEl->m_Size.width();
  int SizecY = pEl->m_Size.height();
  Line( StartX, StartY + dT, StartX + SizecX, StartY + SizecY / 2 );
  Line( StartX + SizecX, StartY + SizecY / 2, StartX, StartY + dT + dH );
  Line( StartX + SizecX, StartY + dT, StartX, StartY + SizecY / 2 );
  Line( StartX, StartY + SizecY / 2, StartX + SizecX, StartY + dT + dH );
#ifdef DebugBorder
  SavePaintState();
  m_Pen.setColor( m_EditSets.m_SignColor );
  m_pCanvas->setPen( m_Pen );
  Line( StartX, StartY, StartX + SizecX, StartY );
  Line( StartX + SizecX, StartY, StartX + SizecX, StartY + SizecY );
  Line( StartX, StartY + SizecY, StartX + SizecX, StartY + SizecY );
  Line( StartX, StartY, StartX, StartY + SizecY );
#endif
  }

void XPInEdit::ConditCalcCursorMeasures( int &X, int &Y, int &SizeY )
  {
  if( sm_Language == lngHebrew && m_pL->m_pSub_L->m_IsCell && !sm_EditKeyPress )
    {
    X = m_pL->m_pSub_L->Curr_positionX();
    Y = m_pL->m_pSub_L->Curr_positionY();
    SizeY = m_pL->m_pSub_L->Curr_SizeY();
    return;
    }
  EdMemb *pLast = m_pL->m_pSub_L->m_pLast;
  if( pLast != nullptr )
    {
    SizeY = m_pL->m_pSub_L->Curr_SizeY();
    if(SizeY == 0 ) SizeY = CharHeight();
    if (pLast->StrSeparat())
      {
      X = m_pL->m_pSub_L->m_Start.X;
      Y = 0;
      for (EdMemb *pM = pLast; pM != nullptr; pM = pM->m_pPrev)
        Y = max(Y, pM->m_Size.height() + pM->m_Start.Y );
      Y += StepString;
      }
    else
      {
      X = pLast->m_Start.X + pLast->m_Size.width();
      Y = pLast->Curr_positionY();
      //      Y = pLast->m_Start.Y + max(0, pLast->m_Axis - SizeY / 2);
      }
    return;
    }
  if( m_pL->m_pSub_L->m_Size.isValid() )
    {
    X = m_pL->m_pSub_L->m_Start.X;
    SizeY = CharHeight();
    Y = m_pL->m_pSub_L->m_Start.Y;
    return;
    }
  bool bActive = m_Active;
  bool bVisible = m_Visible;
  IndReg bFIndReg = m_IndReg;
  bool bDrawingPower = m_DrawingPower;
  if( !sm_EditKeyPress )
    m_pL->m_pSub_L->Append_Before( m_pWForMeasure );
  PreCalc( m_Start, m_Size, m_Axis );
  m_pL->m_pSub_L->MoveLeft( m_pL->m_pSub_L );
  X = m_pL->m_pSub_L->Curr_positionX();
  Y = m_pL->m_pSub_L->Curr_positionY();
  SizeY = m_pL->m_pSub_L->Curr_SizeY();
  if( !sm_EditKeyPress )
    m_pL->m_pSub_L->NextDelete();
  m_Active = bActive;
  m_Visible = bVisible;
  m_IndReg = bFIndReg;
  m_DrawingPower = bDrawingPower;
  if( !sm_EditKeyPress )
    PreCalc( m_Start, m_Size, m_Axis );
  sm_EditKeyPress = false;
  }

TPoint XPInEdit::GetCursorPosition()
  {
  EdList &LSub_L = *m_pL->m_pSub_L;
  TPoint Result;
  if( LSub_L.m_pCurr != NULL )
    {
    Result.X = LSub_L.Curr_positionX();
    Result.Y = LSub_L.Curr_positionY();
    return Result;
    }
 int SizeY;
 int X, Y;
 ConditCalcCursorMeasures( X, Y, SizeY );
 Result.X = X;
 Result.Y = Y;

 if ( m_pL->m_pSub_L->m_pMother != 0 )
     {
   if( TypeConvert( EdFrac ) != NULL && ( m_pL->m_pSub_L->m_pFirst != NULL ) )
     Result.X = m_pL->m_pSub_L->Curr_positionX();

   EdDfIntegr *pElIntegr = TypeConvert( EdDfIntegr );
   if( pElIntegr != NULL && ( (m_pL->m_pSub_L == pElIntegr->m_pLL) || ( m_pL->m_pSub_L == pElIntegr->m_pHL) ) && ( m_pL->m_pSub_L->m_pFirst != 0) )
        Result.X = m_pL->m_pSub_L->Curr_positionX();

   EdGSumm *pElGSumm = TypeConvert( EdGSumm );
   if( pElGSumm != NULL && ( (m_pL->m_pSub_L == pElGSumm->m_pLL) || (m_pL->m_pSub_L == pElGSumm->m_pHL) ) && (m_pL->m_pSub_L->m_pFirst) )
            Result.X = m_pL->m_pSub_L->Curr_positionX();

   EdLimit *ElLimit = TypeConvert( EdLimit );
   if( ElLimit != NULL && ( ( m_pL->m_pSub_L == ElLimit->m_pVL ) || ( m_pL->m_pSub_L == ElLimit->m_pEE ) ) && ( m_pL->m_pSub_L->m_pFirst ) )
            Result.X = m_pL->m_pSub_L->Curr_positionX();

   EdMeas *ElMeas = TypeConvert( EdMeas );
   if( ElMeas != NULL && ( ElMeas->m_DegMin ) && ( m_pL->m_pSub_L == ElMeas->m_pBB ) )
            Result.X = m_pL->m_pSub_L->Curr_positionX();
     }

  return Result;
  }

int XPInEdit::GetCursorSize()
  {
  int Result = m_pL->m_pSub_L->Curr_SizeY();
  //if (m_pL->m_pSub_L->m_pCurr == NULL)
    if( Result == 0  )
    {
    int X, Y, SizeY;
    ConditCalcCursorMeasures( X, Y, SizeY );
    Result = SizeY;
    }
  if( Result != 0 ) return Result;
  return CharHeight( '|' );
//  return TextHeight( "W(|?[01" );
  }

void XPInEdit::ClearSelection()
  {
  EdMemb *pMemb = m_pL->m_pFirst;
  while( pMemb != NULL )
    {
    pMemb->m_pMember->ClearSelection();
    pMemb = pMemb->m_pNext;
    }
  m_pCanvas->fillRect( m_SelectedRect, m_EditSets.m_BkgrColor.rgb() );
  m_SelectedRect.setSize( QSize( 0, 0 ) );
  EditDraw();
  }

void XPInEdit::SelectFragment( QRect &FRect )
  {
  EdMemb *pMemb = m_pL->m_pFirst;
  while( pMemb != NULL )
    {
    pMemb->m_pMember->SelectFragment(FRect);
    pMemb = pMemb->m_pNext;
    }
  }

QByteArray XPInEdit::GetFragment()
  {
  QByteArray Result;
  EdMemb *pMemb = m_pL->m_pFirst;
  while( pMemb != NULL )
    {
    Result += pMemb->m_pMember->GetFragment();
    pMemb = pMemb->m_pNext;
    }
  return Result;
  }

QImage XPInEdit::PWrite()
  {
  QByteArray Text = m_pL->SWrite();
  if( Text.isEmpty() ) return QImage();
  QImage *pImage = reinterpret_cast< QImage * >( m_pCanvas->device() );
  QSize S = CalcSize();
  QImage Image( pImage->copy( 0, 0, S.width(), S.height() ) );
  Image.setText( "F1", Text.data() );
  return Image;
  }

QByteArray XPInEdit::SWrite()
  {
  return m_pL->SWrite();
  }

bool EdElm::InRect( QRect &FRect)
  {
  return FRect.contains( m_Start.X, m_Start.Y ) ||
    FRect.contains( m_Start.X, m_Start.Y + m_Size.height() ) ||
    FRect.contains( m_Start.X + m_Size.width() , m_Start.Y ) ||
    FRect.contains( m_Start.X + m_Size.width(), m_Start.Y + m_Size.height() );
  }

void EdElm::SelectRect()
  {
  if( !m_Selected ) return;
  m_pOwner->SelectRect( QRect( QPoint( m_Start.X, m_Start.Y ), m_Size ) );
  }

EdMemb::EdMemb(const PEdElm& pE, EdMemb *pP, EdMemb *pN, EdList *pEL) : EdElm(pEL->m_pOwner), m_IsProtected(false)
  {
  m_pMother = pEL;
  m_pMember = pE;
  m_pPrev = pP;
  m_pNext = pN;
  m_Start = pE->m_Start;
  if (XPInEdit::sm_pMatr == nullptr) return;
  EdChar *pEch = dynamic_cast<EdChar*>(pE.data());
  m_IsProtected = IsConstEdType(EdSpace, pE.data()) || (pEch != nullptr && (pEch->c() == ' ' || pEch->c() == msIdle || pEch->c() == ';' ) );
  }

void EdMemb::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  m_pMember->PreCalc( m_Start, m_Size, m_Axis );
  S = m_Size;
  A = m_Axis;
  }

int EdMemb::Curr_SizeY()
  {
  for (EdMemb *pEM = this; pEM != nullptr; pEM = pEM->m_pPrev)
    {
    EdChar *pEch = dynamic_cast<EdChar*>(pEM->m_pMember.data());
    if (pEch == nullptr || pEch->m_ind == m_pOwner->m_IndReg || pEch->m_ind == irNormal) return pEM->m_Size.height();
    }
  return m_Size.height();
  }

int EdMemb::Curr_positionY()
  {
  for (EdMemb *pEM = this; pEM != nullptr; pEM = pEM->m_pPrev)
    {
    EdChar *pEch = dynamic_cast<EdChar*>(pEM->m_pMember.data());
    if (pEch == nullptr || pEch->m_ind == m_pOwner->m_IndReg || pEch->m_ind == irNormal) return pEM->m_Start.Y;
    }
  return m_Start.Y;
  }

EdMemb* EdMemb::ParentCalc( EdMemb *pNext )
  {
  char ch;
  if( !m_pMember->ElChar( ch ) ) return m_pNext;
  m_pMember->m_Start = m_Start;
  EdMemb *pResult = m_pMember->ParentCalc( m_pNext );
  m_Start = m_pMember->m_Start;
  m_Size = m_pMember->m_Size;
  m_Axis = m_pMember->m_Axis;
  return pResult;
  }

void EdMemb::Draw( TPoint P )
  {
  if( m_Start.X != P.X  || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  m_pMember->Draw( P );
  }

bool EdMemb::HasIntegral()
  {
  return ( !m_pMember.IsEmpty() && IsConstEdType( EdIntegr, m_pMember.data() ) ) || ( m_pNext != nullptr && m_pNext->HasIntegral() );
  }

EdDfIntegr* EdMemb::GetDefIntegral()
  {
  EdDfIntegr *pResult = nullptr;
  if(!m_pMember.IsEmpty())
    {
    pResult = dynamic_cast< EdDfIntegr* >(m_pMember.data());
    if(pResult != nullptr) return pResult;
    }
  if(m_pNext == nullptr) return pResult;
  return m_pNext->GetDefIntegral();
  }

int EdMemb::IntegralDPos()
  {
  if( m_pMember.IsEmpty() ) return -1;
  const EdIntegr *pInteglal = dynamic_cast< const EdIntegr* >( m_pMember.data() );
  if( pInteglal != nullptr ) return pInteglal->m_pDD->m_Start.Y;
  if( m_pNext == nullptr ) return -1;
  return m_pNext->IntegralDPos();
  }

int EdMemb::IntegralSignPos()
  {
  if( m_pMember.IsEmpty() ) return -1;
  const EdIntegr *pInteglal = dynamic_cast< const EdIntegr* >( m_pMember.data() );
  if( pInteglal != nullptr ) return pInteglal->m_pIntegralSign->m_Start.Y;
  if( m_pNext == nullptr ) return -1;
  return m_pNext->IntegralSignPos();
  }

bool EdMemb::IdleChar()
  {
  if (m_pMember.IsEmpty()) return false;
  EdChar *pEch = dynamic_cast<EdChar*>(m_pMember.data());
  return pEch != nullptr && pEch->c() == msIdle;
  }

EdList::EdList( EdList *pList ) : EdElm( pList->m_pOwner ), m_Primary( pList->m_Primary ), m_ReCalc( pList->m_ReCalc ),
  m_IsCell( pList->m_IsCell ), m_Hebrew( pList->m_Hebrew ), m_pMother( pList->m_pMother ), m_pFirst( NULL ),
  m_pLast( NULL ), m_pCurr( NULL ), m_pSub_L( NULL ), m_pFirstInLine( NULL )
  {
  Copy( pList );
  }

EdList::~EdList()
  {
  while( m_pFirst != NULL )
    {
    EdMemb *pMfree = m_pFirst;
    m_pFirst = m_pFirst->m_pNext;
    delete pMfree;
    }
  }

bool EdList::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  pCr = nullptr;
  if (m_Start.Y <= C.Y && m_Start.Y + m_Size.height() >= C.Y)
    if (C.X < 0)
      {
      pCr = m_pFirst;
      return true;
      }
    else
      {
      if (m_Start.X > C.X || m_Start.X + m_Size.width() < C.X) return false;
      if (m_pFirst == nullptr)
        {
        pSL = this;
        return true;
        }
      }
  for (EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext)
    if (pIndex->m_Start.X <= C.X  &&
      pIndex->m_Start.X + pIndex->m_Size.width() >= C.X &&
      pIndex->m_Start.Y <= C.Y &&
      pIndex->m_Start.Y + pIndex->m_Size.height() >= C.Y)
      {
      if (pIndex->Protected() && !pIndex->IdleChar()) return false;
      pSL = this;
      if (IsConstEdType(EdChar, pIndex->m_pMember.data()) && C.X > pIndex->m_Start.X + pIndex->m_Size.width() / 2)
        {
        if (pIndex == m_pLast)
          pCr = nullptr;
        else
          if (pIndex->m_pNext->StrSeparat())
            pCr = pIndex->m_pNext;
          else
            pCr = pIndex;
        return true;
        }
      else
        pCr = pIndex;
      return pIndex->m_pMember->SetCurrent(C, pSL, pCr);
      }
    else
      if (C.X < 0 && pIndex->m_Start.Y <= C.Y &&
        pIndex->m_Start.Y + pIndex->m_Size.height() >= C.Y)
        {
        pCr = pIndex;
        return true;
        }
  return false;
  }

void EdList::TailSize()
  {
  m_Start = m_pFirst->m_Start;
  m_Size.setHeight(0);
  m_Size.setWidth(0);
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    {
    m_Size.setWidth( max(  m_Size.width(), pIndex->m_Start.X + pIndex->m_Size.width() - m_Start.X ) );
    m_Size.setHeight( max( m_Size.height(), pIndex->m_Start.Y + pIndex->m_Size.height() - m_Start.Y ) );
    if ( pIndex->m_pNext == NULL && pIndex->StrSeparat() )
       m_Size.setHeight( max( m_Size.height(), pIndex->m_Start.Y + pIndex->m_Size.height() +
        pIndex->m_Size.height() + StepString - m_Start.Y ) );
    }
  }

void EdList::PreCalc(TPoint P, QSize &S, int &A)
  {
  if(P.Y < 0) P.Y = 0;
  m_Start = P;
  if (m_pMother == NULL)
    {
    m_pOwner->SetPowerSize(0, true);
    if (m_Hebrew && m_pFirst != nullptr  )
      {
      int H = 0;
      for (EdMemb *pIndex = m_pLast; pIndex != NULL; pIndex = pIndex->m_pPrev)
        {
        pIndex->PreCalc(P, pIndex->m_Size, pIndex->m_Axis);
        P.X += pIndex->m_Size.width();
        H = max(H, pIndex->m_Size.height());
        }
      m_Size.setWidth(P.X);
      m_Size.setHeight(H);
      S = m_Size;
      A = m_Axis = m_pFirst->m_Axis;
      return;
      }
    }
  if(m_Primary )
    {
    m_PrevSize.setWidth( 0 );
    m_PrevSize.setHeight( 0 );
    m_PrevAxis = 0;
    }
  if( m_pFirst == NULL )
    {
    m_Size.setWidth( 0 );
    m_Size.setHeight( 0 );
    m_Axis = 0;
    if( m_pMother != NULL ) m_pOwner->SizeEmptySign( m_Size, m_Axis ); //Warning to javascript
    }
  else
    {
    m_pFirst->PreCalc( P, m_pFirst->m_Size, m_pFirst->m_Axis );
    int LStartY = m_Start.Y;
    int RStartY = m_Start.Y;
    if ( LStartY + m_PrevAxis < RStartY + m_pFirst->m_Axis )
      LStartY = RStartY + m_pFirst->m_Axis - m_PrevAxis;
    if( LStartY + m_PrevAxis > RStartY + m_pFirst->m_Axis && m_pFirst->m_Size.height() != 0 )
      RStartY = LStartY + m_PrevAxis - m_pFirst->m_Axis;
    m_PrevSize.setHeight( max( LStartY + m_PrevSize.height(), RStartY + m_pFirst->m_Size.height() ) - m_Start.Y );
    m_PrevAxis = m_pFirst->m_Axis + RStartY - m_Start.Y;
//{#$             PrevSize.X not used (may be used as X-coord of List's Start)    }
    if( m_pFirst->m_pNext != NULL )
      {
      EdListTail Tai( this );
      if( !m_ReCalc || !m_pFirst->StrSeparat() )
        {
        if( !m_pFirst->StrSeparat() )
          P.X = P.X + m_pFirst->m_Size.width();
        else
          {
          if( m_pMother == NULL ) //      Something such as this, if ";"
            P.X = m_pOwner->m_Start.X;            //      will be available not only
          else                                                    //      in main (root) List
            //      Start.X when Primary = True ]
            P.X = dynamic_cast<EdAbs*>( m_pMother->m_pMember.data())->m_pAA->m_Start.X;
//                                              "as ( TEdSyst or TEdMatr )"
          P.Y += m_PrevSize.height() + StepString;
//{#$     Only on first (left-to-right) pass - therefore P.Y = Start.Y of whole List      }
          Tai.m_Primary = true;
          }
        Tai.PreCalc( P, Tai.m_Size, Tai.m_Axis );
        if( !m_pFirst->StrSeparat() )
          {
          if( m_pFirst->m_Start.Y + m_pFirst->m_Axis < Tai.m_Start.Y + Tai.m_Axis )
            {
            m_pFirst->m_Start.Y = Tai.m_Start.Y + Tai.m_Axis - m_pFirst->m_Axis;
            m_pFirst->PreCalc( m_pFirst->m_Start, m_pFirst->m_Size, m_pFirst->m_Axis );
            }
          if( m_pFirst->m_Start.Y + m_pFirst->m_Axis > Tai.m_Start.Y + Tai.m_Axis && Tai.m_Size.height() != 0 )
            {
            Tai.m_Start.Y = m_pFirst->m_Start.Y + m_pFirst->m_Axis - Tai.m_Axis;
            Tai.m_ReCalc = true;
            Tai.PreCalc( Tai.m_Start, Tai.m_Size, Tai.m_Axis );
            }
          m_Size.setWidth( m_pFirst->m_Size.width() + Tai.m_Size.width() );
          }
        else
          m_Size.setWidth( max(  m_pFirst->m_Size.width(), Tai.m_Start.X + Tai.m_Size.width() - m_pFirst->m_Start.X ) );
        }
      else
        {
        Tai.TailSize();
        m_Size.setWidth( max( m_pFirst->m_Size.width(), Tai.m_Start.X + Tai.m_Size.width() - m_pFirst->m_Start.X ) );
        }
      m_Size.setHeight( max( m_pFirst->m_Start.Y + m_pFirst->m_Size.height(), Tai.m_Start.Y +
        Tai.m_Size.height() ) - m_Start.Y );
      m_Axis = m_pFirst->m_Axis + m_pFirst->m_Start.Y - m_Start.Y;
      }
    else
      {
      m_Size = m_pFirst->m_Size;
      m_Axis = m_pFirst->m_Axis;
      if( m_pFirst->StrSeparat() )
        m_Size.setHeight( m_Size.height() + StepString + m_Size.height() );
      }
    }
  S = m_Size;
  A = m_Axis;
  if( m_Primary ) ParentCalc( NULL );
  }

EdMemb* EdList::ParentCalc( EdMemb *pNext )
  {
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->ParentCalc(pIndex->m_pNext) );
  return pNext;
  }

bool EdList::Splitted()
  {
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    if( pIndex->StrSeparat() ) return true;
  return false;
  }

void EdList::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  if( m_pFirst == NULL )
    {
    m_pOwner->DrawAsEmptySign( this );
    return;
    }
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    pIndex->Draw( pIndex->m_Start );
  }

QByteArray EdList::Write()
  {
  QByteArray W;
  bool IsText = false;
  bool WasMetaSign = false;
  bool bSyst = m_pMother != nullptr && (IsConstEdType( EdSyst, m_pMother->m_pMember.data()));
  bool bSelected = sm_PartlySelected && m_pOwner->Selected();
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    {
    if( Protected(pIndex) && !bSyst || bSelected && !pIndex->Selected() ) continue;
    char C;
    if( (bSyst || (m_pMother != nullptr && ( IsConstEdType( EdDetSumm, m_pMother->m_pMember.data() ) ||
      IsConstEdType( EdMatr, m_pMother->m_pMember.data() ) ) ) ) && pIndex->m_pMember->ElChar( C ) && C == ';' &&
      dynamic_cast<EdChar*>( pIndex->m_pMember.data() )->m_vis && !WasMetaSign )
       W += msMetaSign;
    WasMetaSign = WasMetaSign || pIndex->m_pMember->ElChar( C ) && C == msMetaSign;
    if( m_pMother != nullptr && ( IsConstEdType( EdMatr, m_pMother->m_pMember.data() ) && pIndex->m_pMember->ElChar( C ) && C == ' ' &&
      pIndex->m_pPrev != nullptr && ( !pIndex->m_pPrev->ElChar( C ) || ( C != ' ' && C != ';' ) ) ) )
      W += ',';
    else
      if( pIndex->m_pMember->ElChar( C ) )
        if( ( C == msDegree || C == msMinute ) &&( W.length() == 0 || isdigit( W[W.length() - 1] ) ) )
          {
          W = W + '`' + C + '\'';
          continue;
          }
        else
          IsText = IsText || (C < 0 && dynamic_cast<EdChar*>(pIndex->m_pMember.data())->m_EditKeyPress);
    W += pIndex->m_pMember->Write();
    }
  if (IsText && W[0] != '"')
    return '"' + W + '"';
  return W;
  }

void EdList::MoveLeft( EdList* &pEL )
  {
  if( pEL->m_pCurr == NULL )
    if( pEL->m_pLast == NULL )
      {
      if( pEL->m_pMother != NULL )
        if( !pEL->m_pMother->m_pMember->MoveToPrev( pEL ) )
          {
          EdMemb *pIndCurr = pEL->m_pMother;
          pEL = pEL->m_pMother->m_pMother;
          pEL->m_pCurr = pIndCurr;
          }
      }
    else
      {
      if( !pEL->m_pLast->m_pMember->MoveInLeft( pEL ) )
         pEL->m_pCurr = pEL->m_pLast;
      }
  else
    if( pEL->m_pCurr->m_pPrev == NULL )
      {
      if( pEL->m_pMother != NULL )
        if( !pEL->m_pMother->m_pMember->MoveToPrev( pEL ) )
          {
          EdMemb *pIndCurr = pEL->m_pMother;
          pEL = pEL->m_pMother->m_pMother;
          pEL->m_pCurr = pIndCurr;
          }
      }
    else
      {
      if (pEL->m_pCurr->m_pPrev->Protected() && pEL->m_pMother->m_pMember->MoveToPrev(pEL) ) return;
      if (!pEL->m_pCurr->m_pPrev->m_pMember->MoveInLeft(pEL))
        pEL->m_pCurr = pEL->m_pCurr->m_pPrev;
      }
  }

void EdList::MoveRight(EdList* &pEL)
  {
  if (pEL->m_pCurr == NULL)
    {
    if (pEL->m_pMother != NULL && !pEL->m_pMother->m_pMember->MoveToNext(pEL))
      {
      EdMemb *pIndCurr = pEL->m_pMother->m_pNext;
      pEL = pEL->m_pMother->m_pMother;
      pEL->m_pCurr = pIndCurr;
      }
    return;
    }
  if (pEL->m_pCurr->IdleChar() && (pEL->m_pMother == nullptr || pEL->m_pMother->m_pMember->MoveToNext(pEL))) return;
//  if (pEL->m_pMother != nullptr && pEL->m_pMother->m_pMember->m_pParent != nullptr)
//    pEL->m_pMother->m_pMember->m_pParent->MoveToNext(pEL);
//  else
    if (!pEL->m_pCurr->m_pMember->MoveInRight(pEL))
      pEL->m_pCurr = pEL->m_pCurr->m_pNext;
  }

void EdList::MoveUp( EdList* &pEl )
  {
  for( EdList *pIndList = pEl; pIndList->m_pMother != NULL; pIndList = pIndList->m_pMother->m_pMother )
    if( pIndList->m_pMother->m_pMember->MoveToUp( pIndList ) )
      {
      pEl = pIndList;
      return;
      }
  }

void EdList::MoveDown( EdList* &pEl )
  {
  for( EdList *pIndList = pEl; pIndList->m_pMother != NULL; pIndList = pIndList->m_pMother->m_pMother )
    if( pIndList->m_pMother->m_pMember->MoveToDown( pIndList ) )
      {
      pEl = pIndList;
      return;
      }
  }

int EdList::Curr_positionX()
  {
  if( m_pCurr == NULL )
    {
    if( m_pLast != NULL ) return m_pLast->m_Start.X + m_pLast->m_Size.width();
    return m_Start.X + m_Size.width();
    }
  if( !m_pCurr->m_pMember.IsEmpty() )
    return m_pCurr->m_pMember->m_Start.X;
  return 0;
  }

int EdList::Curr_positionY()
  {
  if( m_pCurr == NULL )
    {
    if( m_pLast != NULL ) return m_pLast->Curr_positionY();
    return m_Start.Y;
    }
  return m_pCurr->Curr_positionY();
//  return m_pCurr->m_pMember->m_Start.Y;
  }

int EdList::Curr_SizeX()
  {
  if( m_pCurr == NULL ) return 0;
  return m_pCurr->m_pMember->m_Size.width();
  }

int  EdList::Curr_SizeY()
  {
  if( m_pCurr == NULL )
    {
    if( m_pLast != NULL ) return m_pLast->Curr_SizeY();
    return m_Size.height();
    }
  return m_pCurr->Curr_SizeY();
  }

QByteArray EdList::SWrite()
    {
    EdMemb *index = m_pFirst;
    QByteArray W = "";
    bool WasMetaSign = false;
    char C;
    for (EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext)
        {
    if( pIndex->Protected() ) continue;
        if ((m_pMother != NULL) && (typeid (m_pMother->m_pMember) == typeid(EdSyst)) ||
            /*(typeid(m_pMother->m_pMember) == typeid(EdDetSumm)) ||*/ (typeid(m_pMother->m_pMember) == typeid(EdMatr)))
            {
      if( index->m_pMember->ElChar( C ) && ( C == ';' ) && ( dynamic_cast<EdChar*>( index->m_pMember.data() )->m_vis ) && !WasMetaSign )
                {
                WasMetaSign = index->m_pMember->ElChar(C) && (C == msMetaSign);
                }
            }
        if ((m_pMother != NULL) && (typeid(m_pMother->m_pMember) == typeid(EdMatr)) &&
                index->m_pMember->ElChar(C) && (C == ' ') && (index->m_pPrev != NULL) &&
                index->m_pPrev->m_pMember->ElChar(C) && (C != ' '))
                    {
                    if (SWriteMode != "Matrix")
                    W = W + ",  ";
                    }
                else
                    {
                    if (index->m_pMember->ElChar(C) && ((C == msDegree) || (C == msMinute)))
                        {
                      W = W + charToTex(C);
                        index = index->m_pNext;
                        continue;
                        }
                    }
                W = W + index->m_pMember->SWrite();
                index = index->m_pNext;
        }
    return W;
    }

EdMemb* EdList::Append_Before( const PEdElm& pE )
  {
  EdChar *pC = dynamic_cast<EdChar*>(pE.data());
  EdMemb* Result = nullptr;
  if (m_pCurr != NULL)
    {
    if (sm_EditKeyPress && pC->c() == ';')
      {
      EdChar *pCurrC = dynamic_cast<EdChar*>(m_pCurr->m_pMember.data());
      if (pCurrC != nullptr && pCurrC->c() == ';')
        {
        Result = new EdMemb(pE, m_pCurr, m_pCurr->m_pNext, this);
        m_pCurr->m_pNext = Result;
        if (Result->m_pNext != nullptr) Result->m_pNext->m_pPrev = Result;
        }
      else
        {
        Result = new EdMemb(pE, m_pLast, NULL, this);
        m_pLast->m_pNext = Result;
        m_pLast = Result;
        EdChar *pRevC = dynamic_cast<EdChar*>(Result->m_pPrev->m_pMember.data());
        if (pRevC == NULL || pRevC->c() != ';')
          {
          Result = new EdMemb(new EdChar(';', m_pOwner), m_pLast, NULL, this);
          m_pLast->m_pNext = Result;
          m_pLast = Result;
          }
        }
      m_pCurr = Result;
      m_pFirstInLine = Result;
      }
    else
      {
      EdChar *pCurrChar = dynamic_cast<EdChar*>(m_pCurr->m_pMember.data());
      if (pCurrChar != nullptr && pCurrChar->c() == msIdle)
        {
        m_pCurr->m_pMember = pE;
        Result = dynamic_cast<EdMemb*>(m_pCurr);
        m_pCurr = m_pCurr->m_pNext;
        return Result;
        }
      if (m_pCurr->m_pMother != nullptr && m_pCurr->m_pMother->m_pMother != nullptr && m_pCurr->m_pMother->m_pMother->m_pMember->m_pParent != nullptr)
        return m_pCurr->m_pMother->m_pMother->m_pMember->m_pParent->ReplaceParentMemb(this, pE);
      Result = new EdMemb(pE, m_pCurr->m_pPrev, m_pCurr, this);
      if (m_pCurr->m_pPrev != NULL)
        {
        EdChar *pRevC = dynamic_cast<EdChar*>(m_pCurr->m_pPrev->m_pMember.data());
        if (pRevC != NULL && pRevC->c() == ';')
          {
          m_pCurr->m_pPrev->m_pNext = Result;
          m_pCurr->m_pPrev = Result;
          m_pFirstInLine = Result;
          if (!IsNoHebrew(pC->c())) m_pCurr = Result;
          }
        else
          {
          m_pCurr->m_pPrev->m_pNext = Result;
          m_pCurr->m_pPrev = Result;
          }
        }
      else
        {
        m_pFirst = Result;
        m_pFirstInLine = Result;
        m_pCurr->m_pPrev = Result;
        EdChar *pChar = dynamic_cast<EdChar*>(m_pCurr->m_pMember.data());
        if (pChar != NULL && pChar->c() == ';') m_pCurr = Result;
        }
      }
    sm_EditKeyPress = false;
    return Result;
    }
  if (m_pMother != nullptr && m_pLast != nullptr && m_pMother->m_pMember->m_pParent != nullptr ) return m_pMother->m_pMember->m_pParent->ReplaceParentMemb(this, pE);
  /*
  if( !sm_EditKeyPress && pC != nullptr && IsHebChar( pC->c() ) )
    if( m_pFirst == nullptr )
      {
      m_Hebrew = true;
      if(m_IsCell)
      Result = new EdMemb( new EdStr(m_pOwner, QByteArray(1, pC->c())), nullptr, nullptr, this );
      }
    else
      {
      EdStr *pStr = dynamic_cast<EdStr*>(m_pLast->m_pMember.data());
      if(pStr != nullptr)
        {
        pStr->AddChar(pC->c());
        sm_EditKeyPress = false;
        m_pCurr = NULL;
        return m_pLast;
        }
      }
*/
  if( Result == nullptr )
    Result = new EdMemb( pE, m_pLast, nullptr, this );
  if( m_pFirst == nullptr)
    {
    m_pFirst = Result;
    m_pFirstInLine = Result;
    sm_EditKeyPress = false;
//    sm_EditKeyPress = m_Hebrew;
    }
  else
    sm_EditKeyPress = false;
  if( m_pLast != NULL ) m_pLast->m_pNext = Result;
  m_pLast = Result;
  m_pCurr = NULL;
  return Result;
  }

void EdList::MemberDelete( EdMemb *pM )
  {
  if( pM == NULL || pM->Protected() && pM->m_pNext != nullptr  ) return;
  if( pM->m_pPrev != NULL ) pM->m_pPrev->m_pNext = pM->m_pNext;
  if( pM->m_pNext != NULL ) pM->m_pNext->m_pPrev = pM->m_pPrev;
  if( m_pFirst == pM ) m_pFirst = pM->m_pNext;
  if( m_pFirstInLine == pM ) m_pFirstInLine = pM->m_pNext;
  if( m_pLast == pM ) m_pLast = pM->m_pPrev;
  if( pM == m_pCurr ) m_pCurr = m_pCurr->m_pNext;
  delete pM;
  }

void EdList::PrevDelete()
  {
  if( m_pCurr != NULL)
    MemberDelete( m_pCurr->m_pPrev );
  else
    MemberDelete( m_pLast );
  }

void EdList::ClearSelection()
  {
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    pIndex->m_pMember->ClearSelection();
  m_Selected = false;
  }

void EdList::SelectFragment( QRect &FRect )
  {
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    {
    pIndex->m_pMember->SelectFragment(FRect);
    m_Selected = m_Selected || pIndex->m_pMember->m_Selected;
    }
  }

QByteArray EdList::GetFragment()
  {
  QByteArray Result;
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    Result += pIndex->m_pMember->GetFragment();
  return Result;
  }

EdMemb *EdList::GetMemb(const EdElm* pEl)
  {
  for (EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext)
    if (pIndex->m_pMember.data() == pEl) return pIndex;
  return nullptr;
  }

void EdList::Select()
  {
  m_Selected = true;
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    pIndex->m_pMember->m_Selected = true;
  }

bool EdList::SelectString(int Y)
  {
  bool Result = false;
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext )
    {
    if(pIndex->Protected()) continue;
    EdBaseChar *pBC = dynamic_cast<EdBaseChar*>(pIndex->m_pMember.data() );
    if(pBC == nullptr) return false;
    int Start = pBC->Start().y();
    if( Y < Start ) return Result;
    if( Y > Start + pBC->Height()) continue;
    pIndex->m_pMember->m_Selected = true;
    Result = true;
    }
  return Result;
  }

bool EdList::HasIntegral()
  {
  return m_pFirst != nullptr && m_pFirst->HasIntegral();
  }

EdDfIntegr* EdList::GetDefIntegral()
  {
  if(m_pFirst == nullptr) return nullptr;
  return m_pFirst->GetDefIntegral();
  }

int EdList::IntegralDPos()
  {
  int Result = m_pFirst == nullptr ? -1 : m_pFirst->IntegralDPos();
//  qDebug() << "IntegralDPos" << Result;
  return Result;
  }

int EdList::IntegralSignPos()
  {
  int Result = m_pFirst == nullptr ? -1 : m_pFirst->IntegralSignPos();
//  qDebug() << "IntegralSignPos" << Result;
  return Result;
  }

int EdList::Count()
  {
  int iCount = 0;
  for( EdMemb *pIndex = m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext, iCount++ );
  return iCount;
  }

void EdList::Clear( int Count )
  {
  EdList::sm_PartlySelected = false;
  ClearSelection();
  for( ; m_pLast != NULL && !m_pLast->Protected() && Count != 0; MemberDelete( m_pLast), Count-- );
  }

void EdList::Copy( EdList *pList )
  {
  Clear();
  for( EdMemb *pMemb = pList->m_pFirst; pMemb != nullptr; pMemb = pMemb->m_pNext )
    Append_Before( pMemb->m_pMember );
  }

EdListTail::EdListTail( EdList *pParent ) : EdList( pParent->m_pOwner )
  {
  m_pMother = pParent->m_pMother;
  m_Primary = false;
  if( pParent->m_pFirst != NULL ) m_pFirst = pParent->m_pFirst->m_pNext;
  if( m_pFirst != nullptr ) m_pLast = pParent->m_pLast;
  m_PrevSize = pParent->m_PrevSize;
  m_PrevAxis = pParent->m_PrevAxis;
  m_ReCalc = pParent->m_ReCalc;
  }

bool EdMatrixBody::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (!EdList::SetCurrent(C, pSL, pCr)) return false;
  EdMemb *pIndex = pCr;
  if (pIndex != nullptr && IsConstEdType(EdSpace, pIndex->m_pMember.data())) pIndex = pIndex->m_pNext;
  pCr = pIndex;
  return pIndex != nullptr;
  }

void EdMatrixBody::MoveLeft(EdList* &pEl)
  {
  do
    EdList::MoveLeft(pEl);
  while (m_pCurr != m_pFirst && m_pCurr->Protected() && !m_pCurr->IdleChar());
  if (m_pCurr == m_pFirst)
    {
    EdList::MoveLeft(pEl);
    if( !dynamic_cast<EdMatr*>(m_pMother->m_pMember.data())->m_IsVisible ) EdList::MoveLeft(pEl);
    }
  }

void EdMatrixBody::MoveRight(EdList* &pEl)
  {
  do
    EdList::MoveRight(pEl);
  while (m_pCurr != nullptr && m_pCurr->Protected() && !m_pCurr->IdleChar() && m_pCurr->m_pPrev->Protected());
  if (m_pCurr == nullptr && pEl->m_pMother != nullptr && IsConstEdType(EdMatr, pEl->m_pMother->m_pMember.data()))
    {
    EdList::MoveRight(pEl);
    if (!dynamic_cast<EdMatr*>(m_pMother->m_pMember.data())->m_IsVisible) EdList::MoveRight(pEl);
    }
  }

bool EdMatrixBody::Protected(EdMemb *pIndex)
  {
  if (!pIndex->m_IsProtected) return false;
  return IsConstEdType(EdSpace, pIndex->m_pMember.data());
  }

EdMemb *EdMatrixBody::Append_Before(const PEdElm& pE)
  {
  if( XPInEdit::sm_pMatr != nullptr ) return EdList::Append_Before(pE);
  EdList* pEl = this;
  if(m_pCurr != nullptr && m_pCurr->m_pPrev == nullptr ) EdList::MoveRight(pEl);
  EdChar *pEch = dynamic_cast<EdChar*>(pE.data());
  if (pEch != nullptr && (pEch->c() == ' ' || pEch->c() == ',' || pEch->c() == ';')) return m_pCurr;
  if (m_pCurr == nullptr)
    if (m_pLast->m_IsProtected)
      m_pCurr = m_pLast;
    else
      {
      EdMemb* Result = new EdMemb(pE, m_pLast, nullptr, this);
      m_pLast->m_pNext = Result;
      m_pLast = Result;
      return m_pLast;
      }
  if( m_pCurr == nullptr) return EdList::Append_Before(pE);
  if ( m_pCurr->m_IsProtected  && !m_pCurr->IdleChar() && (m_pCurr->m_pPrev == nullptr || m_pCurr->m_pPrev->IdleChar() ) ) m_pCurr = m_pCurr->m_pPrev;
  if (m_pCurr == nullptr) return EdList::Append_Before(pE);
  pEch = dynamic_cast<EdChar*>(m_pCurr->m_pMember.data());
  if (pEch == nullptr || pEch->c() != msIdle ) return EdList::Append_Before(pE);
  m_pCurr->m_IsProtected = false;
  m_pCurr->m_pMember = pE;
  MoveRight(pEl);
  return m_pCurr;
  }

void EdMatrixBody::MemberDelete(EdMemb *pM)
  {
  if (pM->Protected()) return;
  bool bIdleSign = false;
  if (pM->m_pPrev == nullptr)
    bIdleSign = pM->m_pNext == nullptr || pM->m_pNext->Protected();
  else
    if (pM->m_pNext == nullptr)
      bIdleSign = pM->m_pPrev->Protected();
    else
      bIdleSign = pM->m_pNext->Protected() && pM->m_pPrev->Protected();
  if (bIdleSign)
    {
    pM->m_pMember = new EdChar(msIdle, m_pOwner);
    pM->m_IsProtected = true;
    return;
    }
  EdList::MemberDelete(pM);
  }

EdBaseChar::EdBaseChar( uchar C, IndReg ind, QColor Color, XPInEdit *pOwn ) :
  EdElm( pOwn), m_ind( ind ), m_Color( Color ), m_EditKeyPress(TXPGrEl::sm_EditKeyPress)
  {
  if(m_EditKeyPress)
    if( C == '{')
      C = msBrackLeftLower;
    else
      if( C == '}')
        C = msBrackRightLower;
  m_ch = C;
  m_Qch = ToUnicode( C );
  }

void EdBaseChar::ResetChar( uchar C )
  {
  m_ch = C;
  m_Qch = ToUnicode( m_ch );
  m_Width = -1;
  m_Height = -1;
  }

EdBigChar::EdBigChar(uchar C, IndReg ind, QColor Color, XPInEdit *pOwn, const QFont& Font, int Kern) : EdBaseChar(C, ind, Color, pOwn),
  m_Font(Font), m_Kern(Kern)
  {
  CalcSize();
  }

void EdBigChar::CalcSize()
  {
  m_pOwner->SetFont( m_Font );
  m_Width = m_pOwner->CharWidth( m_Qch, m_Kern );
  QRect R = m_pOwner->m_pCanvas->fontMetrics().boundingRect( m_Qch );
  m_Height = R.height() + 3;
  m_Top = R.top();
  m_pOwner->RestoreFont();
  }

void EdBigChar::RecalcSize( double K )
  {
  m_Font.setPointSize( m_Font.pointSize() * K );
  CalcSize();
  }

void EdBigChar::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  m_Size.setWidth( m_Width );
  m_Size.setHeight( m_Height );
  m_Axis = m_Height / 2;
  S = m_Size;
  A = m_Axis;
  }

void EdBigChar::Draw( TPoint P )
  {
  m_pOwner->SetFont( m_Font );
  if( ( m_ch == '{' || m_ch == '}' ) && m_Size.height() > m_Height )
    {
    QPainter &Canvas = *m_pOwner->m_pCanvas;
    QPen Pen( Canvas.pen() );
    QRect R = m_pOwner->CharRect( m_Qch );
    QSize BrckSize = R.size();
    QImage BrckIm( BrckSize.width() + 1, BrckSize.height(), QImage::Format_ARGB32_Premultiplied );
    QPainter BrckPainter( &BrckIm );
    BrckPainter.setFont( Canvas.font() );
    QColor BackgroundColor = m_pOwner->m_EditSets.m_BkgrColor;
    if( m_Selected )
      {
      Pen.setColor( Qt::white );
      BackgroundColor = Qt::black;
      }
    BrckPainter.setPen( Pen );
    BrckPainter.fillRect( 0, 0, BrckIm.width(), BrckIm.height(), BackgroundColor );
    BrckPainter.drawText( 0, -R.top(), m_Qch );
    QRgb BrackColors[sc_MaxBracketWidth];
    double QuarterBracket = BrckSize.height() / 4.0;
    double InsertionLen = ( m_Size.height() - BrckSize.height() ) / 2.0;
    QRgb OldColor = Pen.color().rgb();
    int iImgBottom;
    int iCharBottom;
    auto Insertion = [&] ( int iCharTop )
      {
      for( int iPixel = 0; iPixel < BrckIm.width() && iPixel < sc_MaxBracketWidth; iPixel++ )
        BrackColors[iPixel] = BrckIm.pixel( iPixel, iImgBottom );
      for( int iPixel = 0; iPixel < BrckIm.width() && iPixel < sc_MaxBracketWidth; iPixel++ )
        {
        Pen.setColor( BrackColors[iPixel] );
        Canvas.setPen( Pen );
        Canvas.drawLine( P.X + iPixel, P.Y + iCharTop, P.X + iPixel, P.Y + iCharBottom );
        }
      };
    iImgBottom = Round( QuarterBracket );
    Canvas.drawImage( P.X, P.Y, BrckIm, 0, 0, BrckIm.width(), iImgBottom );
    iCharBottom = Round( QuarterBracket + InsertionLen );
    Insertion( iImgBottom );
    int iCenterLen = Round( BrckSize.height() - QuarterBracket * 2 );
    Canvas.drawImage( P.X, P.Y + iCharBottom, BrckIm, 0, iImgBottom, BrckIm.width(), iCenterLen );
    iImgBottom = Round( BrckSize.height() - QuarterBracket );
    int iCharTop = iCharBottom + iCenterLen;
    iCharBottom = Round( iCharTop + InsertionLen );
    Insertion( iCharTop );
    Canvas.drawImage( P.X, P.Y + iCharBottom, BrckIm, 0, iImgBottom, BrckIm.width(), Round( QuarterBracket ) );
    Pen.setColor( OldColor );
    Canvas.setPen( Pen );
    }
  else
    m_pOwner->m_pCanvas->drawText( P.X, P.Y - m_Top, m_Qch );
  m_pOwner->RestoreFont();
  }

QByteArray EdBigChar::Write()
  {
  return QByteArray() + ( char ) m_ch;
  }

QByteArray EdBigChar::SWrite()
  {
  return charToTex( m_ch );
  }

EdChar::EdChar( uchar C, XPInEdit *pOwn, QRgb Color ) : EdBaseChar( C, C == msCConjugation && !TXPGrEl::sm_EditKeyPress ? irShape : pOwn->m_IndReg, Color, pOwn ),
  m_Color(Color), m_IsInterval(pOwn->m_IsInterval)
  {
  m_vis = pOwn->m_Visible && m_ch != ';';
  if( m_ch == ',' && m_IsInterval ) m_ch = ';';
  m_IsSeparat = m_ch == ';' && pOwn->m_CreateSystem;
//  if( m_ch == msMultSign1 ) m_vis = sm_ShowMultSign;
//  if( m_ch == msMultSign2 && !sm_ShowMultSign )
//    if( m_ch == msMultSign2  )
//    {
//    m_ch = msMultSign1;
//    m_Qch = ToUnicode( m_ch );
//    }
//  if( !m_vis && m_ch == msMultSign2 || m_ch == msMultSign1 ) pOwn->m_Visible = true;
  m_Width = -1;
  if (m_ch == ' ' && XPInEdit::sm_pMatr != nullptr) m_Width = 6;
  m_Height = -1;
  if( pOwn->m_IndReg == irShape ) pOwn->m_IndReg = irNormal;
  }

int EdChar::GetAddHeight(char C)
  {
  if( m_ind != 2 ) return 0;
  int add = 0;
  if( C >= 'A' && C <= 'Z' || In( C, "idfhjklb" ) ) add = 3;
  return add;
  }

bool EdChar::StrSeparat()
  {
  return m_IsSeparat || m_ch == msCharNewLine || m_ch == ';' && !m_IsInterval;
  }

void EdChar::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  m_pOwner->SetRegSize( m_ind );
  if(m_Width == -1)  m_Width = m_pOwner->CharWidth( m_Qch );
  if(m_Height == -1)
    {
    m_Height = m_pOwner->CharHeight();
    if( m_ch >= msPrime && m_ch <= msTriplePrime ) m_Height /= 2;
    }
  m_Size.setWidth( m_Width );
  m_Size.setHeight( m_Height );
  switch( m_ch )
    {
    case')':
      m_Size.setWidth( m_Width + 2 );
    case '(':
      m_Size.setHeight( m_Height );
      break;
    case '-':
    case '.':
    case ',':
    case 'i':
    case '+':
      m_Size.setWidth( m_Width + 4 );
      break;
    case msMultSign1:
    case ':':
    case '=':
    case msArrowToRight:
    case msLongMinus:
    case msLongPlus:
    case ';':
      m_Size.setWidth( m_Width + 8 );
    }
  if( m_ch == msCharNewLine || NotVisible() )
    {
    m_Size.setWidth( m_ch == msMultSign2 || m_ch == msMultSign1 ? 2 : 0 );
    if( m_ch != '1' ) m_Size.setHeight( m_pOwner->CharHeight( 'W' ) );
    }
  if( m_ind == 0 || m_ind == 2 )
    if( m_ch == msCharNewLine || NotVisible() )
      m_Axis = m_pOwner->CharHeight( 'W' ) / 2;
    else
      m_Axis = m_Height / 2;
  if( m_ind == 1 ) m_Axis = m_Size.height();
  if( m_ind == -1 ) m_Axis = 0;
  if( m_ind == 2 )
    {
    int add = GetAddHeight( m_ch );
    m_Size.setHeight( m_Size.height() + add );
    m_Axis += add;
    }
  S = m_Size;
  A = m_Axis;
  m_pOwner->SetRegSize( 0 );
  }

void EdChar::SelectFragment( QRect &FRect )
  {
  EdElm::SelectFragment( FRect );
  m_Selected = m_Selected || !( ( FRect.top() > m_Start.Y + m_Size.height() ) || ( FRect.bottom() < m_Start.Y ) ||
    ( FRect.left() > m_Start.X + m_Size.width() ) || ( FRect.right() < m_Start.X ) );
  }

EdMemb* EdChar::ParentCalc( EdMemb *pNext )
  {
  while( m_ch == '(' && pNext != NULL && !pNext->StrSeparat() )
    {
    EdMemb *pIndex = pNext;
    pNext = pIndex->ParentCalc( pIndex->m_pNext );
    char NextChar;
    if( pIndex->ElChar( NextChar ) && NextChar == ')' )
      {
      pIndex->m_pMember->m_Start.Y = m_Start.Y;
      pIndex->m_pMember->m_Size.setHeight( m_Size.height() );
      pIndex->m_pMember->m_Axis = m_Axis;
      pIndex->m_Start.Y = m_Start.Y;
      pIndex->m_Size.setHeight( m_Size.height() );
      pIndex->m_Axis = m_Axis;
      break;
      }
    int dY = m_Start.Y - pIndex->m_Start.Y;
    if( dY > 0 )
      {
      m_Start.Y = pIndex->m_Start.Y;
      m_Size.setHeight( m_Size.height() + dY );
      m_Axis += dY;
      }
    dY = ( pIndex->m_Start.Y + pIndex->m_Size.height() ) - ( m_Start.Y + m_Size.height() );
    if( dY > 0 )
      m_Size.setHeight( m_Size.height() + dY );
    }
  return pNext;
  }

void EdChar::TextOut( TPoint &P )
  {
  if( m_ind == 2 )
    {
    int add = GetAddHeight( m_ch );
      m_pOwner->TextOut( P.X, P.Y + add, m_Qch + QString( ToUnicode( msMean ) ), true );
    }
  else
    {
    int add = 0;
    switch( m_ch )
      {
      case msIdle:
        m_pOwner->DrawAsEmptySign(this);
        return;
      case '=':
      case msMultSign1:
      case ':':
      case msArrowToRight:
      case msLongMinus:
      case msLongPlus:
        add = 4;
        break;
      case '.':
      case ',':
      case '-':
      case '+':
      case 'i':
        add = 2;
      }
    if( XPInEdit::sm_TextFont )
      m_pOwner->TextOut( P.X, P.Y + add, EdStr::sm_pCodec->toUnicode( (char*) &m_ch, 1 ), true );
    else
      m_pOwner->PaintChar( P.X + add, P.Y, m_Qch );
    }
  }

void EdChar::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y ) m_Start = P;
  if (m_ch == msCharNewLine || m_ch == msInvisible || NotVisible()) return;
  SelectRect();
  m_pOwner->SetRegSize( m_ind );
  QPainter &Canvas = *m_pOwner->m_pCanvas;
  QRgb OldFontColor = Canvas.pen().color().rgb();
  QFont Font = Canvas.font();
  int OldFontWeight = Font.weight();
  QPen Pen( Canvas.pen() );
  if( ( m_ch == '(' || m_ch == ')' ) && m_Size.height() > m_Height )
    {
    QRect R = m_pOwner->CharRect( m_Qch );
    QSize BrckSize = R.size();
    int HalfBracket = BrckSize.height() / 2;
    int EndLine = m_Size.height() - HalfBracket;
    QImage BrckIm( BrckSize.width() + 1, BrckSize.height(), QImage::Format_ARGB32_Premultiplied );
    QPainter BrckPainter( &BrckIm );
    BrckPainter.setFont( Canvas.font() );
    QColor BackgroundColor = m_pOwner->m_EditSets.m_BkgrColor;
    if( m_Selected )
      {
      Pen.setColor( Qt::white );
      BackgroundColor = Qt::black;
      }
    BrckPainter.setPen( Pen );
    BrckPainter.fillRect( 0, 0, BrckIm.width(), BrckIm.height(), BackgroundColor );
    BrckPainter.drawText( 0, -R.top(), m_Qch );
    QRgb BrackColors[sc_MaxBracketWidth];
    for( int iPixel = 0; iPixel < BrckIm.width() && iPixel < sc_MaxBracketWidth; iPixel++ )
      BrackColors[iPixel] = BrckIm.pixel( iPixel, HalfBracket );
    Canvas.drawImage( P.X, P.Y, BrckIm, 0, 0, BrckIm.width(), HalfBracket );
    QRgb OldColor = Pen.color().rgb();
    for( int iPixel = 0; iPixel < BrckIm.width() && iPixel < sc_MaxBracketWidth; iPixel++ )
      {
      Pen.setColor( BrackColors[iPixel] );
      Canvas.setPen( Pen );
      Canvas.drawLine( P.X + iPixel, P.Y + HalfBracket, P.X + iPixel, P.Y + EndLine );
      }
    Canvas.drawImage( P.X, P.Y + EndLine, BrckIm, 0, HalfBracket, BrckIm.width(), HalfBracket );
    Pen.setColor( OldColor );
    Canvas.setPen( Pen );
    }
  else
    {
    if( m_Color != Qt::black )
      {
      Font.setWeight( QFont::Bold );
      Pen.setColor( m_Color );
      Canvas.setFont( Font );
      Canvas.setPen( Pen );
      TextOut( P );
      }
    else
      TextOut( P );
    }

#ifdef DebugBorder
  Pen.setColor( m_pOwner->m_EditSets.m_SignColor );
  Canvas.setPen( Pen );
  m_pOwner->Line( P.X, P.Y, P.X + m_Size.width(), P.Y );
  m_pOwner->Line( P.X + m_Size.width(), P.Y, P.X + m_Size.width(), P.Y + m_Size.height() );
  m_pOwner->Line( P.X, P.Y + m_Size.height(), P.X + m_Size.width(), P.Y + m_Size.height() );
  m_pOwner->Line( P.X, P.Y, P.X, P.Y + m_Size.height() );
#endif

  Pen.setColor( OldFontColor );
  Canvas.setPen( Pen );
  Font.setWeight( OldFontWeight );
  Canvas.setFont( Font );
  }

QByteArray EdChar::Write()
  {
  if( m_ch == msInvisible ) return "sp";
  char S = 0;
  if( m_ind == 1 ) S = msHighReg;
  if( m_ind == -1 ) S = msLowReg;
  if( m_ind == 2 && m_ch != msCConjugation) S = msMean;
  if( !m_vis && m_ch != msMultSign1 && m_ch != msMultSign2 && m_ch != '1' && m_ch != ';' ) S += msMetaSign;
  char C = m_ch == msLongMinus || m_ch == msLongPlus ? m_Qch.toLatin1() : m_ch;
  if( S == 0 )
    return QByteArray() + C;
  return SimpleChar( S ) + C;
  }

QByteArray EdChar::SWrite()
  {
  QByteArray S = charToTex( m_ch );
  if( !m_vis && ( m_ch == msMultSign2 || m_ch == msMultSign1 || m_ch == '1' ) )
    S = "\\setunvisible\n" + S;
    if( m_ind == 1 )
    return "\\superscript{" + S + "}";
  if( m_ind == -1 )
    return "\\subscript{" + S + "}";
  if( m_ind == 2 )
    return "\\mean{" + S + "}";
  return S;
  }

bool EdAbs::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pAA->SetCurrent(C, pSL, pCr))
    pSL->m_pCurr = pCr;
  else
    if (m_pAA->m_Start.X > C.X)
      {
      pCr = pSL->GetMemb(this);
      return true;
      }
    else
      if (m_pAA->m_Start.X + m_pAA->m_Size.width() < C.X)
        return false;
      else
        pSL = m_pAA;
  return true;
  }

bool EdAbs::MoveInRight(EdList* &pL)
  {
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  if (m_pAA->m_pFirst == nullptr) return true;
  EdMatr *pMatr = dynamic_cast<EdMatr*>(pL->m_pCurr->m_pMember.data());
  if (pMatr != nullptr)
    {
    pL = pMatr->m_pAA;
    pMatr->m_pAA->m_pCurr = pMatr->m_pAA->m_pFirst->m_pNext;
    }
  return true;
  }

bool EdAbs::MoveInLeft(EdList* &pL)
  {
  pL = m_pAA;
  pL->m_pCurr = NULL;
  if (m_pAA->m_pFirst == nullptr) return true;
  EdMatr *pMatr = dynamic_cast<EdMatr*>(m_pAA->m_pFirst->m_pMember.data());
  if (pMatr != nullptr)
    {
    pL = pMatr->m_pAA;
    pMatr->m_pAA->m_pCurr = pMatr->m_pAA->m_pLast->m_pPrev;
    }
  return true;
  }

void EdAbs::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  TPoint P1;
  QSize S1;
  if( m_IsVisible )
    {
    P1.X = m_Start.X + 8;
    P1.Y = m_Start.Y + 4;
    }
  else
    {
    P1.X = m_Start.X;
    P1.Y = m_Start.Y;
    }
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
  if( m_IsVisible )
    {
    m_Size.setWidth( S1.width() + 16 );
    m_Size.setHeight( S1.height() + 8 );
    }
  else
    {
    m_Size.setWidth( S1.width() );
    m_Size.setHeight( S1.height() );
    }
  m_Axis = P1.Y - m_Start.Y + m_pAA->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdAbs::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  m_pAA->Draw( m_pAA->m_Start );
  if( !m_IsVisible ) return;
  QPen Pen = m_pOwner->m_pCanvas->pen();
 // Pen.setColor( m_pOwner->m_EditSets.m_SignColor );
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line( m_Start.X + 4, m_Start.Y, m_Start.X + 4, m_Start.Y + m_Size.height() );
  m_pOwner->Line( m_pAA->m_Start.X + m_pAA->m_Size.width() + 4, m_Start.Y, m_pAA->m_Start.X + m_pAA->m_Size.width() + 4, m_Start.Y + m_Size.height() );
  }

QByteArray EdAbs::Write()
  {
  return  '(' + QByteArray(AbsName) + '(' + m_pAA->Write() + "))";
  }

QByteArray EdAbs::SWrite()
    {
    return "\\abs{" + m_pAA->SWrite() + '}';
    }

QByteArray EdTwo::GetFragment()
  {
  if( m_Selected ) return Write() + '&';
  return m_pAA->GetFragment() + m_pBB->GetFragment();
  }

bool EdTwo::SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr )
  {
  if (m_pAA->SetCurrent(C, pSL, pCr) || m_pBB->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

EdInterval::EdInterval( XPInEdit *pOwn, const QByteArray& Signs ) : EdTwo( pOwn ), m_pLeftBracket( new EdChar( Signs[0], pOwn ) ),
  m_pRightBracket(new EdChar(Signs[1], pOwn)), m_pSemicolon(new EdList(pOwn))
  {
  m_pSemicolon->Append_Before( new EdChar( ' ', pOwn ) );
  pOwn->m_IsInterval = true;
  EdChar *pC = new EdChar(';', pOwn);
  pC->m_vis = true;
  m_pSemicolon->Append_Before( pC );
  pOwn->m_IsInterval = false;
  m_pSemicolon->Append_Before( new EdChar( ' ', pOwn ) );
  }

void EdInterval::PreCalc( TPoint P, QSize &S, int &A )
  {
  QSize S1, S2;
  int A1, A2;
  m_Start = P;
  TPoint PA( P ), PB( P ), PSM;
  m_pAA->PreCalc( PA, S1, A1 );
  m_pBB->PreCalc( PB, S2, A2 );
  if( S1.height() > S2.height() )
    {
    m_Size.setHeight( S1.height() );
    m_Axis = A1;
    PB.Y += A1 - A2;
    PSM = PB;
    }
  else
    {
    m_Size.setHeight( S2.height() );
    m_Axis = A2;
    PA.Y += A2 - A1;
    PSM = PA;
    }
  m_pLeftBracket->PreCalc( P, S1, A1 );
  m_pLeftBracket->m_Size.setHeight( m_Size.height() );
  PA.X += m_pLeftBracket->Width();
  m_pAA->PreCalc( PA, S1, A1 );
  PSM.X = PA.X + S1.width();
  m_pSemicolon->PreCalc( PSM, S2, A2 );
  PB.X = PSM.X + S2.width() - 8;
  m_pBB->PreCalc( PB, S2, A2 );
  P.X = PB.X + S2.width();
  m_pRightBracket->PreCalc( P, S2, A2 );
  m_pRightBracket->m_Size.setHeight( m_Size.height() );
  m_Size.setWidth( P.X + m_pRightBracket->Width() - m_Start.X );
  S = m_Size;
  A = m_Axis;
  }

void EdInterval::Draw( TPoint P )
  {
  if( ( m_Start.X != P.X ) || ( m_Start.Y != P.Y ) )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  m_pLeftBracket->Draw( m_pLeftBracket->m_Start );
  m_pAA->Draw( m_pAA->m_Start );
  m_pSemicolon->Draw( m_pSemicolon->m_Start );
  m_pBB->Draw( m_pBB->m_Start );
  m_pRightBracket->Draw( m_pRightBracket->m_Start );
  }

/*
bool EdInterval::MoveInRight( EdList* &pL ) { pL = m_pAA; pL->m_pCurr = m_pAA->m_pFirst; return true; }
bool EdInterval::MoveInLeft( EdList* &pL ) { pL = m_pBB; pL->m_pCurr = NULL; return true; }
*/

bool EdInterval::MoveToNext( EdList* &pL )
  {
  if( pL != m_pAA ) return false;
  pL = m_pBB;
  pL->m_pCurr = m_pBB->m_pFirst;
  return true;
  }

bool EdInterval::MoveToPrev( EdList* &pL )
  {
  if( pL != m_pBB ) return false;
  pL = m_pAA;
  pL->m_pCurr = NULL;
  return true;
  }

QByteArray EdInterval::Write()
  {
  return m_pLeftBracket->Write() + m_pAA->Write() + ';' + m_pBB->Write() + m_pRightBracket->Write();
  }

QByteArray EdInterval::SWrite()
  {
  return "\\interval{" + m_pLeftBracket->SWrite() + m_pRightBracket->SWrite() + "}{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + '}';
  }

EdIntervals::IPoint::IPoint( const QByteArray& V, const QByteArray& S ) :
  m_sValue(V), m_Sign(S[0] == '1')
  {
  QByteArrayList VList = m_sValue.split( '/' );
  if( VList.count() == 1 )
    {
    m_Value = m_sValue.toDouble();
    return;
    }
  m_Value = VList[0].toDouble() / VList[1].toDouble();
  }

EdIntervals::EdIntervals( XPInEdit *pOwn, const QByteArray& Intervals ) : EdElm( pOwn )
  {
  QByteArrayList List = Intervals.split( ',' );
  m_Ax = List[0].toDouble();
  m_Len = List[1].toDouble();
  for( int iPoint = 2; iPoint < List.count() - 1; iPoint += 3 )
    {
    m_Points.append( IPoint( List[iPoint], List[iPoint + 1] ) );
    m_Intervals.append( List[iPoint + 2].toInt() );
    IPoint &P = m_Points.last();
    P.m_X = Round( ( P.m_Value - m_Ax ) / m_Len * sm_Width );
    }
  m_Intervals.append( List.last().toInt() );
  }

void EdIntervals::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  m_Size.setWidth( sm_Width );
  m_Size.setHeight(sm_Height);
  m_Axis = 10;
  S = m_Size;
  A = m_Axis;
  }

void EdIntervals::Draw( TPoint P )
  {
  if( ( m_Start.X != P.X ) || ( m_Start.Y != P.Y ) )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  QPen Pen = m_pOwner->m_pCanvas->pen();
  Pen.setColor( "black" );
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line( m_Start.X, m_Start.Y + 5, m_Start.X + m_Size.width(), m_Start.Y + 5 );
  for( int i = 0; i < m_Points.count(); i++ )
    {
    int x = m_Points[i].m_X;
    if( !m_Points[i].m_Sign )
      m_pOwner->m_pCanvas->drawEllipse( m_Start.X + x - 3, m_Start.Y + 2, 8, 8 );
    else
      m_pOwner->m_pCanvas->fillRect( m_Start.X + x - 2, m_Start.Y + 3, 6, 6, "black" );
    int Shift = m_Points[i].m_sValue.count() * 3;
    m_pOwner->m_pCanvas->drawText( m_Start.X + x - Shift, m_Start.Y + 32, m_Points[i].m_sValue );
    }
  int j = 1;
  int i = 0;
  do
    {
    if( m_Intervals[i] )
      m_pOwner->Line( m_Start.X + j, m_Start.Y, m_Start.X + j, m_Start.Y + 7 );
    j += 4;
    if( i < m_Points.count() && j > m_Points[i].m_X ) i++;
    } while( j < m_Size.width() - 1 );
  }


EdPolygon::EdPolygon( XPInEdit *pOwn, const QByteArray& Polygon ) : EdAbs( pOwn ), m_ZLP( false )
  {
  QByteArrayList Points = Polygon.split( ',' );
  m_FullSize = Points[0].toInt() * 300;
  m_MinX = Points[1].toDouble();
  m_MinY = Points[2].toDouble();
  m_MaxX = Points[3].toDouble();
  m_MaxY = Points[4].toDouble();
  m_Count = Points[5].toInt();
  double Dx = m_MaxX - m_MinX;
  double Dy = m_MaxY - m_MinY;
  double Delta = max( Dx, Dy );
  m_Size.setWidth( Round( ( ceil( m_MaxX ) - floor( m_MinX ) ) / Delta * m_FullSize ) + 120 );
  m_Size.setHeight( Round( m_FullSize *Dy / Delta ) + 40 );
  const int Shift = -20;
  int I = 6;
  for( int i = 1; i <= m_Count; i++ )
    {
    m_PosString[i] = Points[I++].toDouble();
    m_X1[i] = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_Y1[i] = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    m_X2[i] = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_Y2[i] = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    if( m_X1[i] != m_X2[i] )
      {
      if( m_Y1[i] == m_Y2[i] )
        {
        m_X1[i] = 0;
        m_X2[i] = m_Size.width();
        }
      else
        {
        if( m_X1[i] > m_X2[i] )
          {
          int iTmp = m_X1[i];
          m_X1[i] = m_X2[i];
          m_X2[i] = iTmp;
          iTmp = m_Y1[i];
          m_Y1[i] = m_Y2[i];
          m_Y2[i] = iTmp;
          }
        double k = ( (double) m_Y2[i] - m_Y1[i] ) / ( m_X2[i] - m_X1[i] );
        double rY0 = m_Y1[i] - k * m_X1[i];
        m_Y2[i] = Round( rY0 + k * m_Size.width() );
        if( m_Y2[i] > m_Size.height() )
          m_Y2[i] = m_Size.height();
        else
          if( m_Y2[i] < Shift ) m_Y2[i] = Shift;
        m_X2[i] = Round( ( m_Y2[i] - rY0 ) / k );
        if( m_X1[i] > 0 )
          {
          if( k > 0 )
            m_Y1[i] = Shift;
          else
            m_Y1[i] = m_Size.height();
          m_X1[i] = Round( ( m_Y1[i] - rY0 ) / k );
          }
        }
      }
    else
      {
      m_Y1[i] = Shift;
      m_Y2[i] = m_Size.height();
      }
    double v = Points[I++].toDouble();
    double v1 = Points[I++].toDouble();
    if( v == 0 && v1 == 0 )
      {
      m_Gx[i] = 0;
      m_Gy[i] = 0;
      }
    else
      {
      m_Gx[i] = Round( 2 * v / sqrt( v*v + v1*v1 ) );
      m_Gy[i] = Round( 2 * v1 / sqrt( v* v + v1*v1 ) );
      }
    m_Decent[i] = Points[I++].toDouble() != 0;
    }
  m_Count1 = Points[I++].toInt();
  for( int i = 1; i <= abs( m_Count1 ); i++ )
    {
    m_X[i] = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_Y[i] = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    }
  m_ColorX = 0;
  m_ColorY = 0;
  if( m_Count1 != 0 )
    {
    m_ColorX = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_ColorY = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    }
  for( int i = 1; i <= m_Count; i++ )
    {
    QByteArray Name = Points[I++];
    m_Inscriptions[i] = Name.mid( 1, Name.length() - 2);
    }
  m_X0 = Round( -m_MinX / Delta*m_FullSize );
  m_Y0 = Round( m_MaxY / Delta*m_FullSize );

  m_NamedPointCount = Points[I++].toInt();
  for( int i = 1; i <= m_NamedPointCount; i++ )
    {
    m_NamedPoints[i].m_X = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_NamedPoints[i].m_Y = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    m_NamedPoints[i].m_Name = Points[I++];
    }

  if( I < Points.count() )
    {
    m_ZLP = true;
    m_X1[0] = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_X1[0] = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    m_X2[0] = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_Y2[0] = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    m_Gxr = Points[I++].toDouble();
    m_Gyr = Points[I++].toDouble();
    m_Gx[0] = Round( 2 * m_Gxr / sqrt( m_Gxr * m_Gxr + m_Gyr*m_Gyr ) );
    m_Gy[0] = Round( 2 * m_Gyr / sqrt( m_Gxr * m_Gxr + m_Gyr*m_Gyr ) );
    m_X[0] = Round( ( Points[I++].toDouble() - m_MinX ) / Delta*m_FullSize );
    m_Y[0] = Round( ( m_MaxY - Points[I++].toDouble() ) / Delta*m_FullSize );
    }
  }

void EdPolygon::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  m_Axis = 10;
  S = m_Size;
  A = m_Axis;
  }

void EdPolygon::Draw( TPoint P )
  {
  enum TypeDraw { DrawLine, DrawConstraint, DrawString, DrawPattern };
  const int Shift = 20, Len = 20, step = 20;

  auto CalcStyle = [&]( int i )
    {
    for( int j = 1; j <= m_Count; j++ )
      if( m_X1[j] == m_X2[j] )
        if( m_X[i] == m_X1[j] && m_X[i - 1] == m_X1[j] )
          {
          i = j;
          break;
          }
        else
          continue;
      else
        if( m_X1[j] == m_Y2[j] )
          if( ( m_Y[i] == m_X1[j] ) && ( m_Y[i - 1] == m_X1[j] ) )
            {
            i = j;
            break;
            }
          else
            continue;
      else
          {
          double k = ( (double) m_Y2[j] - m_X1[j] ) / ( m_X2[j] - m_X1[j] );
          double a = m_Y2[j] - k * m_X2[j];
          if( abs( m_Y[i] - Round( a + k * m_X[i] ) ) > 2 || abs( m_Y[i - 1] - Round( a + k * m_X[i - 1] ) ) > 2 ) continue;
          i = j;
          break;
          }
    if( m_Decent[i] ) return Qt::SolidLine;
    return Qt::DotLine;
    };

  auto PutLines = [&]( TypeDraw DrawType )
    {
    for( int i = 1; i <= m_Count; i++ )
      {
      double t = m_PosString[i];
      double gLen = sqrt( m_Gx[i] * m_Gx[i] + m_Gy[i] * m_Gy[i] );
      int a1 = m_Start.X + m_X1[i];
      int b1 = m_Start.Y + m_Y1[i] + Shift;
      int a2 = m_Start.X + m_X2[i];
      int b2 = m_Start.Y + m_Y2[i] + Shift;
      if( gLen != 0 )
        {
        if( DrawType == DrawLine )
          continue;
        else
          if( DrawType == DrawConstraint )
            {
            m_pOwner->Line( a1, b1, a2, b2 );
            continue;
            }
        }
      else
        if( DrawType == DrawConstraint )
          continue;
        else
        if( DrawType == DrawLine )
          {
          m_pOwner->Line( a1, b1, a2, b2 );
          continue;
          }
      double Angle = 180.0 / M_PI*qAtan2( b1 - b2, a2 - a1 );
      if( Angle > 90 ) Angle -= 180;
      if( Angle < -90 ) Angle += 180;

      int LL = 1;
//      if( m_Gy[i]<0 || m_Gy[i] == 0 && m_Gx[i] >= 0 )
      LL += m_pOwner->CharHeight( '9' ) * 1.2;
      int tx, ty;
      if( gLen == 0 )
        {
        Angle = Angle / 180 * M_PI;
        tx = Round( a1 + t*( a2 - a1 ) + 5 * sin( Angle ) );
        ty = Round( b1 + t*( b2 - b1 ) + 5 * cos( Angle ) );
        }
      else
        {
        tx = Round( a1 + t*( a2 - a1 ) - LL*m_Gx[i] / gLen );
        ty = Round( b1 + t*( b2 - b1 ) + LL*m_Gy[i] / gLen );
        }
      m_pOwner->m_pCanvas->save();
      m_pOwner->m_pCanvas->translate( tx, ty );
      m_pOwner->m_pCanvas->rotate( -Angle );
      m_pOwner->TextOut( 0, 0, m_Inscriptions[i] );
      m_pOwner->m_pCanvas->restore();
      if( gLen == 0 ) continue;

      if( abs( a1 - a2 )>abs( b1 - b2 ) )
        {
        if( a1 > a2 )
          {
          int k = a1; a1 = a2; a2 = k;
          k = b1; b1 = b2; b2 = k;
          }
        int xx0 = a1;
        while( xx0 < a2 )
          {
          int yy0 = Round( b1 + ( (double) b2 - b1 ) / ( a2 - a1 )*( xx0 - a1 ) );
          m_pOwner->Line( xx0, yy0, Round( xx0 + Len*m_Gx[i] / gLen ), Round( yy0 - Len*m_Gy[i] / gLen ) );
          xx0 += step;
          }
        }
      else
        {
        if( b1 > b2 )
          {
          int k = a1; a1 = a2; a2 = k;
          k = b1; b1 = b2; b2 = k;
          }
        int yy0 = b1;
        while( yy0 < b2 )
          {
          int xx0 = Round( a1 + ( ( double ) a2 - a1 ) / ( b2 - b1 )*( yy0 - b1 ) );
          m_pOwner->Line( xx0, yy0, Round( xx0 + Len*m_Gx[i] / gLen ), Round( yy0 - Len*m_Gy[i] / gLen ) );
          yy0 += step;
          }
        }
      }
    };

  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }

  double Delta = max( m_MaxX - m_MinX, m_MaxY - m_MinY );
  m_pOwner->TextOut( m_Start.X + m_Size.width() - 20, m_Start.Y + m_Y0 + 15, "x" );
  m_pOwner->TextOut( m_Start.X + m_X0 + 5, m_Start.Y + 18, "y" );
   if( m_ZLP )
    {
    int a1 = m_Start.X + m_X1[0];
    int b1 = m_Start.Y + m_X1[0] + Shift;
    int a2 = m_Start.X + m_X2[0];
    int b2 = m_Start.Y + m_Y2[0] + Shift;
    m_pOwner->Line( a1, b1, a2, b2 );
    double gLen = sqrt( m_Gx[0] * m_Gx[0] + m_Gy[0] * m_Gy[0] );
    int xx0 = ( a1 + a2 ) / 2;
    int yy0 = ( b1 + b2 ) / 2;
    int tx = Round( xx0 + 3 * Len*m_Gx[0] / gLen );
    int ty = Round( yy0 - 3 * Len*m_Gy[0] / gLen );
    m_pOwner->Line( xx0, yy0, tx, ty );
    int xx = Round( xx0 + 2 * Len*m_Gx[0] / gLen );
    int yy = Round( yy0 - 2 * Len*m_Gy[0] / gLen );
    m_pOwner->Line( tx, ty, Round( xx + 0.5*Len*m_Gy[0] / gLen ), Round( yy + 0.5*Len*m_Gx[0] / gLen ) );
    m_pOwner->Line( tx, ty, Round( xx - 0.5*Len*m_Gy[0] / gLen ), Round( yy - 0.5*Len*m_Gx[0] / gLen ) );
    m_pOwner->TextOut( tx + 3, ty + 3, "(" + QByteArray::number( m_Gxr ) + "," + QByteArray::number( m_Gyr ) + ")" );
    }
  QPen OldPen = m_pOwner->m_pCanvas->pen();
  QPen Pen = OldPen;
  Pen.setWidth( 1 );
  Pen.setColor( "black" );
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->m_pCanvas->setRenderHint( QPainter::Antialiasing );
  PutLines( DrawConstraint );
  Pen.setWidth( 3 );
  if( m_Count1 != 0 )
    if( m_Count1 > 0 )
      {
      int xx = m_X[1];
      int yy = m_Y[1];
      for( int i = 2; i <= m_Count1; i++ )
        {
        Pen.setStyle( CalcStyle( i ) );
        m_pOwner->m_pCanvas->setPen( Pen );
        m_pOwner->Line( m_Start.X + xx, m_Start.Y + yy + Shift, m_Start.X + m_X[i], m_Start.Y + m_Y[i] + Shift );
        xx = m_X[i];
        yy = m_Y[i];
        }
      }
    else
      {
      Pen.setStyle( CalcStyle( 2 ) );
      m_pOwner->m_pCanvas->setPen( Pen );
      m_pOwner->Line( m_Start.X + m_X[1], m_Start.Y + m_Y[1] + Shift, m_Start.X + m_X[2], m_Start.Y + m_Y[2] + Shift );
      Pen.setStyle( CalcStyle( 4 ) );
      m_pOwner->m_pCanvas->setPen( Pen );
      m_pOwner->Line( m_Start.X + m_X[3], m_Start.Y + m_Y[3] + Shift, m_Start.X + m_X[4], m_Start.Y + m_Y[4] + Shift );
      }
  m_pOwner->m_pCanvas->setPen( OldPen );
  int i = 1;
  m_pOwner->Line( m_Start.X + i, m_Start.Y + i, m_Start.X + m_Size.width() - i, m_Start.Y + i );
  m_pOwner->Line( m_Start.X + i, m_Start.Y + m_Size.height() - i, m_Start.X + m_Size.width() - i, m_Start.Y + m_Size.height() - i );
  m_pOwner->Line( m_Start.X + i, m_Start.Y + i, m_Start.X + i, m_Start.Y + m_Size.height() - i );
  m_pOwner->Line( m_Start.X + m_Size.width() - i, m_Start.Y + i, m_Start.X + m_Size.width() - i, m_Start.Y + m_Size.height() - i );

  if( m_ColorX != 0 || m_ColorY != 0 )
    {
    QImage *pImage = dynamic_cast< QImage* >( m_pOwner->m_pCanvas->device() );
    QColor Color( 0xBFFF00 );
    QPoint QP( QPoint( m_Start.X + m_ColorX, m_Start.Y + m_ColorY + Shift ) );
    FloodFill( QRect( m_Start.X, m_Start.Y, 1000, 1000 ), pImage, QP, Color );
    FloodFill( QRect( m_Start.X + m_ColorX, m_Start.Y + m_ColorY + Shift + 5, 500, 500 ), pImage, QP, Color );
    FloodFill( QRect( m_Start.X + m_ColorX - 5, m_Start.Y + m_ColorY + Shift, 500, 500 ), pImage, QP, Color );
    FloodFill( QRect( m_Start.X + m_ColorX, m_Start.Y + m_ColorY + Shift - 5, 500, 500 ), pImage, QP, Color );
    }

  PutLines( DrawLine );

  PutLines( DrawPattern );

  Pen = OldPen;
  Pen.setColor( QColor( 0x00FF0000 ) );
  m_pOwner->m_pCanvas->setPen( Pen );

  m_pOwner->Line( m_Start.X, m_Start.Y + m_Y0 + Shift, m_Start.X + m_Size.width(), m_Start.Y + m_Y0 + Shift );
  m_pOwner->Line( m_Start.X + m_Size.width(), m_Start.Y + m_Y0 + Shift, m_Start.X + m_Size.width() - 10, m_Start.Y + m_Y0 - 5 + Shift );
  m_pOwner->Line( m_Start.X + m_Size.width(), m_Start.Y + m_Y0 + Shift, m_Start.X + m_Size.width() - 10, m_Start.Y + m_Y0 + 5 + Shift );

  int iStep = EdChart::CalculateStep( round( m_MaxX - m_MinX ), 1 );
  QFont OldFont = m_pOwner->m_pCanvas->font();
  QFont Font = OldFont;
  Font.setPointSizeF( 2 * Font.pointSizeF() / 3 );
  m_pOwner->m_pCanvas->setFont( Font );
  for( i = Round( m_MinX / iStep ) * iStep; i <= Round( m_MaxX ); i += iStep )
    {
    int k = Round( ( i - m_MinX ) / Delta * m_FullSize );
    m_pOwner->Line( m_Start.X + k, m_Start.Y + m_Y0 + Shift, m_Start.X + k, m_Start.Y + m_Y0 - 5 + Shift );
    m_pOwner->TextOut( m_Start.X + k, m_Start.Y + m_Y0 + Shift - 3, QByteArray::number( i ) );
    }

  m_pOwner->Line( m_Start.X + m_X0, m_Start.Y, m_Start.X + m_X0, m_Start.Y + m_Size.height() );
  m_pOwner->Line( m_Start.X + m_X0, m_Start.Y, m_Start.X + m_X0 - 5, m_Start.Y + 10 );
  m_pOwner->Line( m_Start.X + m_X0, m_Start.Y, m_Start.X + m_X0 + 5, m_Start.Y + 10 );

  iStep = EdChart::CalculateStep( Round( m_MaxY - m_MinY ), 2 );
  i = Round( m_MinY / iStep ) * iStep;
  int iShift = max( m_pOwner->TextWidth( QString::number( round( m_MinY ) ) ), m_pOwner->TextWidth( QString::number( round( m_MaxY ) ) ) );
  if( iShift > 0 )
    iShift++;
  else
  iShift = 10;
  if( m_X0 > 10 )
    iShift = -iShift;
  else
  iShift = 3;
  for( ; i <= Round( m_MaxY ); i += iStep )
    {
    int k = Round( ( m_MaxY - i ) / Delta * m_FullSize );
    if( k + Shift < m_Size.height() )
      {
      m_pOwner->Line( m_Start.X + m_X0, m_Start.Y + k + Shift, m_Start.X + m_X0 + 5, m_Start.Y + k + Shift );
      if( i != 0 )
        m_pOwner->TextOut( m_Start.X + m_X0 + iShift, m_Start.Y + k + Shift, QString::number( i ) );
      }
    }
  m_pOwner->m_pCanvas->setFont( OldFont );

  if( m_ZLP )
    {
    QPen Pen = OldPen;
    Pen.setWidth( 3 );
    Pen.setColor( QColor( 0x000000FF ) );
    m_pOwner->m_pCanvas->setPen( Pen );
    m_pOwner->m_pCanvas->drawEllipse( m_Start.X + m_X[0] - 3, m_Start.Y + m_Y[0] + Shift - 3, m_Start.X + m_X[0] + 3, m_Start.Y + m_Y[0] + Shift + 3 );
    m_pOwner->m_pCanvas->setPen( OldPen );
    }

  m_pOwner->m_pCanvas->setFont( m_pOwner->m_ViewSettings.m_CalcCmFont );
  int FontHeight = m_pOwner->m_pCanvas->font().pixelSize();
  for( i = 1; i <= m_NamedPointCount; i++ )
    m_pOwner->TextOut( m_NamedPoints[i].m_X + 3, m_NamedPoints[i].m_Y - FontHeight, m_NamedPoints[i].m_Name );
  m_pOwner->m_pCanvas->setFont( OldFont );
  }

TEdTrigoComp::TEdTrigoComp( XPInEdit *pOwn, const QByteArray& Parms ) : EdAbs( pOwn )
  {
  int iPos = Parms.indexOf( ',' );
  m_Re = Parms.left( iPos++ ).toDouble();
  int iNext = Parms.indexOf( ',', iPos );
  m_Im = Parms.mid( iPos, iNext - iPos ).toDouble();
  m_rScale = Parms.mid( iNext + 1 ).toDouble();
  if( m_Re > 0 )
    {
    m_MaxX = m_Re + 1;
    m_MinX = -1;
    }
  else
    {
    m_MinX = m_Re - 1;
    m_MaxX = 1;
    }
  if( m_Im > 0 )
    {
    m_MaxY = m_Im + 1;
    m_MinY = -1;
    }
  else
    {
    m_MinY = m_Im - 1;
    m_MaxY = 1;
    }
  }

void TEdTrigoComp::PreCalc( TPoint P, QSize &S, int &A )
  {
  double Dx = m_MaxX - m_MinX;
  double Dy = m_MaxY - m_MinY;

  m_Scale = 300;
  m_Size.setHeight( Round( 300 * Dy / Dx ) );
  int MaxScale = Round( 350 * m_rScale );
  if( m_Size.height() > MaxScale )
    {
    m_Scale = max( 40, min( ( int ) Round( ( double ) MaxScale / m_Size.height() * 300 ), 100 ) );
    if( m_Scale % 2 == 1 ) m_Scale++;
    m_Size.setHeight( Round( m_Scale * Dy / Dx ) );
    }
  if( m_Re < 0 )
    m_Size.setWidth( m_Scale + cm_Shift * 2 );
  else
    m_Size.setWidth( m_Scale + cm_Shift );
  m_x0 = Round( -m_MinX / Dx*m_Scale );
  m_y0 = Round( m_MaxY / Dx*m_Scale );
  int iStepX = EdChart::CalculateStep( round( Dx ), 1 );
  int iStepY = EdChart::CalculateStep( round( Dy ), 1 );
  int iStep = max( iStepX, iStepY );
  int k = m_pOwner->TextWidth( NumberToStr( Round( m_MaxX ) ) ) * 0.67;
  m_VerticalLabelX = k + 5 > Round( iStep / Dx * m_Scale );
  int AddY = 3;
  if( m_Im > 0 && m_Re < 0 ) AddY = 5;
  if( m_VerticalLabelX && m_MaxY != 1 )
    m_Size.setHeight( m_y0 + k + 5 + cm_Shift );
  else
    if( m_MaxY == 1 )
      m_Size.setHeight( m_y0 + Round( abs( m_MinY ) / Dx * m_Scale ) + cm_Shift * AddY );
    else
      m_Size.setHeight( m_y0 + cm_Shift * AddY );
  m_x1 = Round( ( m_Re - m_MinX ) / Dx*m_Scale );
  m_y1 = Round( ( m_MaxY - m_Im ) / Dx*m_Scale );
  m_Start = P;
  m_Axis = m_Size.height() / 2;
  S = m_Size;
  A = m_Axis;
  }

void TEdTrigoComp::Draw( TPoint P )
  {
  const int Len = 20, step = 20;
  const double t = 0.5;
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  QPainter &Canvas = *m_pOwner->m_pCanvas;
  QColor OldFontColor = Canvas.pen().color();
  QFont Font = Canvas.font();
  int OldFontWeight = Font.weight();
  QPen Pen( Canvas.pen() );
  Canvas.setPen( Qt::black );
  int OldFontHeight = Canvas.font().pointSize();
  Font.setPointSize( Round( 2 * OldFontHeight / 3 ) );
  Canvas.setFont( Font );
  m_pOwner->Line( m_Start.X, m_Start.Y + m_y0 + cm_Shift, m_Start.X + m_Size.width(), m_Start.Y + m_y0 + cm_Shift );
  m_pOwner->Line( m_Start.X + m_Size.width(), m_Start.Y + m_y0 + cm_Shift, m_Start.X + m_Size.width() - 10, m_Start.Y + m_y0 - 5 + cm_Shift );
  m_pOwner->Line( m_Start.X + m_Size.width(), m_Start.Y + m_y0 + cm_Shift, m_Start.X + m_Size.width() - 10, m_Start.Y + m_y0 + 5 + cm_Shift );
  m_pOwner->TextOut( m_Start.X + m_Size.width() - 10, m_Start.Y + m_y0 + cm_Shift - 9, "x" );

  double Dx = m_MaxX - m_MinX;
  int iStepX = EdChart::CalculateStep( round( Dx ), 1 );
  int iStepY = EdChart::CalculateStep( round( m_MaxY - m_MinY ), 1 );
  int iStep = max( iStepX, iStepY );
  int iX0;
  if( m_MinX > 0 )
    iX0 = floor( ( m_MinX - iStep ) / iStep )*iStep;
  else
    iX0 = ceil( ( m_MinX ) / iStep )*iStep;
  int iH = m_pOwner->ETextHeight( "0" );
  if( m_VerticalLabelX )
    {
    int i = iX0;
    int iy = m_Start.Y + m_y0 + 5 + cm_Shift;
    while( i + iStep <= Round( m_MaxX ) )
      {
      int k = Round( i / Dx * m_Scale );
      m_pOwner->Line( m_Start.X + m_x0 + k, m_Start.Y + m_y0 + cm_Shift, m_Start.X + m_x0 + k, m_Start.Y + m_y0 - 5 + cm_Shift );
      if( i != 0 )
        {
        QByteArray sLabel = NumberToStr( i );
        Canvas.save();
        Canvas.translate( m_Start.X + m_x0 + k - iH / 2.0, iy );
        Canvas.rotate( 90 );
        m_pOwner->TextOut( 0,  0, sLabel );
        Canvas.restore();
        }
      i = i + iStep;
      }
    }
  else
    {
    int i = iX0;
    while( i + iStep <= Round( m_MaxX ) )
      {
      int k = Round( i / Dx * m_Scale );
      m_pOwner->Line( m_Start.X + m_x0 + k, m_Start.Y + m_y0 + cm_Shift, m_Start.X + m_x0 + k, m_Start.Y + m_y0 - 5 + cm_Shift );
      if( i != 0 )
        m_pOwner->TextOut( m_Start.X + m_x0 + k - m_pOwner->TextWidth( NumberToStr( i ) ) / 2,
          m_Start.Y + m_y0 + cm_Shift + 5 + iH, NumberToStr( i ) );
      i = i + iStep;
      }
    }

  m_pOwner->Line( m_Start.X + m_x0, m_Start.Y, m_Start.X + m_x0, m_Start.Y + m_Size.height() );
  m_pOwner->Line( m_Start.X + m_x0, m_Start.Y, m_Start.X + m_x0 - 5, m_Start.Y + 10 );
  m_pOwner->Line( m_Start.X + m_x0, m_Start.Y, m_Start.X + m_x0 + 5, m_Start.Y + 10 );
  m_pOwner->TextOut( m_Start.X + m_x0 + 5, m_Start.Y, "y" );

  int Fmt = NumberToStr( Round( m_MaxY ) ).length();
  int ix = max( m_pOwner->TextWidth( NumberToStr( Round( m_MaxY ) ) ), m_pOwner->TextWidth( NumberToStr( Round( m_MinY ) ) ) ) + 3;
  for( int i = Round( m_MinY ); i <= Round( m_MaxY ); i += iStep )
    {
    int k = Round( ( m_MaxY - i ) / ( m_MaxX - m_MinX )*m_Scale );
    m_pOwner->Line( m_Start.X + m_x0, m_Start.Y + k + cm_Shift, m_Start.X + m_x0 + 5, m_Start.Y + k + cm_Shift );
    if( i != 0 )
      m_pOwner->TextOut( m_Start.X + m_x0 - ix, m_Start.Y + k + cm_Shift, QString( "%1" ).arg( i, Fmt ) );
    }
  Font.setPointSize( OldFontHeight );
  Canvas.setFont( Font );
  Canvas.setPen( Pen );
  int tx = m_Start.X + m_x1;
  int ty = m_Start.Y + m_y1 + cm_Shift;
  int gx = m_x1 - m_x0;
  int gy = m_y0 - m_y1;
  int gLen = sqrt( gx * gx + gy * gy );
  Pen.setStyle( Qt::DotLine );
  Canvas.setPen( Pen );
  m_pOwner->Line( tx, ty, tx, m_Start.Y + m_y0 + cm_Shift );
  m_pOwner->Line( tx, ty, m_Start.X + m_x0, ty );
  Pen.setStyle( Qt::SolidLine );
  Canvas.setPen( Pen );

  m_pOwner->Line( m_Start.X + m_x0, m_Start.Y + m_y0 + cm_Shift, tx, ty );
  m_pOwner->Line( tx, ty, Round( tx - ( 10 * gx - 5 * gy ) / gLen ), Round( ty + ( 10 * gy + 5 * gx ) / gLen ) );
  m_pOwner->Line( tx, ty, Round( tx - ( 10 * gx + 5 * gy ) / gLen ), Round( ty + ( 10 * gy - 5 * gx ) / gLen ) );
  int g2 = min( (int) Round( gLen / 2.0 ), m_y0 + cm_Shift );
  double Angle = qRadiansToDegrees( qAtan2( m_Im, m_Re ) );
  if( Angle < 0 ) Angle += 360;
  Canvas.drawArc( m_Start.X + m_x0 - g2 / 2, m_Start.Y + m_y0 + cm_Shift - g2 / 2, g2, g2, 0.0, Angle * 16 );
  }

QByteArray TEdTrigoComp::Write()
  {
  return "@TrigoComplex(" + NumberToStr( m_Re ) + "+" + NumberToStr( m_Im ) + "*i," +
    NumberToStr( m_rScale ) + ",false)";
  }

QByteArray TEdTrigoComp::SWrite()
  {
  return "\\trigocomp {" + NumberToStr( m_Re ) + ',' + NumberToStr( m_Im ) + ',' + NumberToStr( m_Scale ) + '}';
  }

void EdPowr::PreCalc( TPoint P, QSize &S, int &A )
  {
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_Start = P;
  TPoint P1 = m_Start, P2;
  QSize S1, S2;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
  P2.X = m_pAA->m_Start.X + S1.width();
  P2.Y = P1.Y;
  if( m_AsIndex ) m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pBB->PreCalc( P2, S2, m_pBB->m_Axis );
  if( m_AsIndex ) m_pOwner->SetPowerSize( -1, BaseLevel );
  if( m_AsIndex && S1.height() <= S2.height() )
    m_pAA->m_Start.Y = m_pAA->m_Start.Y + S2.height() - S1.height() / 2;
  if( m_AsIndex && S1.height() > S2.height() )
    m_pAA->m_Start.Y = m_pAA->m_Start.Y + S2.height() / 2;
  if( m_pAA->m_Start.Y != P1.Y )
    m_pAA->PreCalc( m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis );
  m_Size.setWidth( S1.width() + S2.width() + m_pOwner->CharWidth( '-' ) / 3 );
  m_Size.setHeight( m_pAA->m_Start.Y + m_pAA->m_Size.height() - m_Start.Y );
  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdPowr::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pAA->Draw( m_pAA->m_Start );
  if( m_AsIndex ) m_pOwner->SetPowerSize( 1, BaseLevel );
  m_pBB->Draw( m_pBB->m_Start );
  if( m_AsIndex ) m_pOwner->SetPowerSize( -1, BaseLevel );
  }

bool EdPowr::MoveToNext( EdList* &pL )
  {
  if (pL != m_pAA)
    {
    if(m_pParent == nullptr) return false;
    return m_pParent->MoveToNext(pL);
    }
  pL = m_pBB;
  pL->m_pCurr = m_pBB->m_pFirst;
  return true;
  }

bool EdPowr::MoveToPrev ( EdList* &pL )
  {
  if (m_pParent != nullptr) return m_pParent->MoveToPrev(pL);
  if( pL != m_pBB ) return false;
  pL = m_pAA;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdPowr::MoveToUp( EdList* &pL )
  {
  if( pL != m_pAA ) return false;
  pL = m_pBB;
  pL->m_pCurr = m_pBB->m_pFirst;;
  return true;
  }

bool EdPowr::MoveToDown ( EdList* &pL )
  {
  if( pL != m_pBB ) return false;
  pL = m_pAA;
  pL->m_pCurr = NULL;
  return true;
  }

QByteArray EdPowr::SWrite()
    {
    return "\\power{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + "}";
    }

QByteArray EdPowr::Write()
  {
  return "((" + m_pAA->Write() + ") ^ (" + m_pBB->Write() + "))";
  }

void EdRoot::PreCalc( TPoint P, QSize &S, int &A )//  Procedure TEdRoot.PreCalc;
  {
  bool BaseLevel = m_pOwner->m_DrawingPower == 0; //BaseLevel : Boolean; BaseLevel := Owner.PowerSize = 0;
  m_Start = P;//Start := P;
  TPoint P2 = m_Start, P1; //Recode[    P1, P2 : TPoint; P2 := Start;
  QSize S1, S2; //S1, S2 : TSize;
  m_pOwner->SetPowerSize( +1, BaseLevel );//Owner.SetPowerSize( +1, BaseLevel );
  m_pBB->PreCalc( P2, S2,m_pBB->m_Axis );//BB.PreCalc( P2, S2, BB.Axis );
  m_pOwner->SetPowerSize( -1, BaseLevel );//Owner.SetPowerSize( -1, BaseLevel );
  P1.X = P2.X + S2.width() + 6;//P1.X := P2.X + S2.width() + 6;
  P1.Y = P2.Y + 2;//P1.Y := P2.Y + 2;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );//AA.PreCalc( P1, S1, AA.Axis );
  if( S1.height() / 2 < S2.height() + 2 )// if ( S1.cY div 2 < S2.cY + 2 ) then  {
    {
    m_pAA->m_Start.Y = m_pAA->m_Start.Y + S2.height() + 2 - S1.height() / 2;
    //AA.Start.Y := AA.Start.Y + S2.cY + 2 - S1.cY div 2;
    m_pAA->PreCalc( m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis );
    //AA.PreCalc( AA.Start, AA.Size, AA.Axis );
    }
  m_Size.setWidth( S1.width() + S2.width() + 8 );//Size.cX := S1.cX + S2.cX + 8;
  m_Size.setHeight( m_pAA->m_Start.Y + m_pAA->m_Size.height() - m_Start.Y );
  //Size.cY := AA.Start.Y + AA.Size.cY - Start.Y;
  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;
  //Axis   := AA.Start.Y - Start.Y + AA.Axis;
  S = m_Size;//S := Size;
  A = m_Axis;//A := Axis;
  }

void EdRoot::Draw( TPoint P )//  Procedure TEdRoot.Draw ( P : TPoint );
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )//IF ( Start.X <> P.X ) or ( Start.Y <> P.Y )
    {
    m_Start = P;//Start := P;
    PreCalc( m_Start, m_Size, m_Axis );//PreCalc( Start, Size, Axis );
    }
  SelectRect();
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;// BaseLevel : Boolean; BaseLevel := Owner.PowerSize = 0;
  m_pAA->Draw( m_pAA->m_Start );//AA.Draw( AA.Start );
  m_pOwner->SetPowerSize( 1, BaseLevel );//Owner.SetPowerSize( +1, BaseLevel );
  m_pBB->Draw( m_pBB->m_Start );//BB.Draw( BB.Start );
  m_pOwner->SetPowerSize( -1, BaseLevel );//Owner.SetPowerSize( -1, BaseLevel );
  QPen Pen = m_pOwner->m_pCanvas->pen();// Owner.Canvas.Pen := Owner.EditSets.SignPen;
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line( m_pAA->m_Start.X - 1, m_pAA->m_Start.Y - 2,
    m_pAA->m_Start.X + m_pAA->m_Size.width() + 1, m_pAA->m_Start.Y - 2 );
  m_pOwner->Line( m_pAA->m_Start.X - 1, m_pAA->m_Start.Y - 1,
    m_pBB->m_Start.X + 4, m_pAA->m_Start.Y + m_pAA->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height(),
    m_pBB->m_Start.X + 3, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X, m_pBB->m_Start.Y + 3 + m_pBB->m_Size.height(),
    m_pBB->m_Start.X + 3, m_pBB->m_Start.Y + 3 + m_pBB->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X + 3, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height(),
    m_pBB->m_Start.X + 3, m_pAA->m_Start.Y + m_pAA->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X + 4, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height(),
    m_pBB->m_Start.X + 4, m_pAA->m_Start.Y + m_pAA->m_Size.height() );
  }

bool EdRoot::MoveToNext( EdList* &pL )
  {
  if( pL != m_pBB ) return false;
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdRoot::MoveToPrev ( EdList* &pL )
  {
  if( pL != m_pAA ) return false;
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdRoot::MoveToUp( EdList* &pL )
  {
  if( pL != m_pAA ) return false;
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdRoot::MoveToDown ( EdList* &pL )
  {
  if( pL != m_pBB ) return false;
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

QByteArray EdRoot::SWrite()
    {
    return "\\root{" + m_pBB->SWrite() + "}{" + m_pAA->SWrite() + "}";
    }

void EdSqRoot::PreCalc(TPoint P, QSize &S, int &A)//Procedure TEdSqRoot.PreCalc;
  {
   bool BaseLevel = m_pOwner->m_DrawingPower == 0; //BaseLevel : Boolean; BaseLevel := Owner.PowerSize = 0;
  TPoint P2 = m_Start, P1; //Recode[    P1, P2 : TPoint; P2 := Start;
  QSize S1, S2; //S1, S2 : TSize;
  m_pOwner->SetPowerSize(+1, BaseLevel);//Owner.SetPowerSize( +1, BaseLevel );
  m_pBB->PreCalc(P2, S2, m_pBB->m_Axis);//BB.PreCalc( P2, S2, BB.Axis ); ???
  m_pOwner->SetPowerSize(-1, BaseLevel);// Owner.SetPowerSize( -1, BaseLevel );
  P1.X = P2.X + m_pBB->m_Size.width() + 6;//P1.X := P2.X + BB.Size.cX + 6; ???
  P1.Y = P2.Y + 2;//P1.Y := P2.Y + 2;
  m_pAA->PreCalc(P1, S1, m_pAA->m_Axis);//AA.PreCalc( P1, S1, AA.Axis );
  if (S1.height() / 2 < S2.height() + 2)//if ( S1.cY div 2 < S2.cY + 2 ) then
    {
    m_pAA->m_Start.Y = m_pAA->m_Start.Y + S2.height() + 2 - S1.height()/2; //???
    m_pAA->PreCalc(m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis);
    }
  m_Size.setWidth( S1.width() + S2.width() + 8);
  m_Size.setHeight(m_pAA->m_Start.Y + m_pAA->m_Size.height() - m_Start.Y);
  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;
  S = m_Size;//S := Size;
  A = m_Axis;//A := Axis;
  }

void EdSqRoot::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  m_pAA->Draw( m_pAA->m_Start );
  QPen Pen = m_pOwner->m_pCanvas->pen();// Owner.Canvas.Pen := Owner.EditSets.SignPen;
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line( m_pAA->m_Start.X - 1, m_pAA->m_Start.Y - 2, m_pAA->m_Start.X + m_pAA->m_Size.width() + 1, m_pAA->m_Start.Y - 2 );
  m_pOwner->Line( m_pAA->m_Start.X - 1, m_pAA->m_Start.Y - 1, m_pBB->m_Start.X + 4, m_pAA->m_Start.Y + m_pAA->m_Size.height() ); //???
  m_pOwner->Line( m_pBB->m_Start.X, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height(), m_pBB->m_Start.X + 3, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X, m_pBB->m_Start.Y + 3 + m_pBB->m_Size.height(), m_pBB->m_Start.X + 3, m_pBB->m_Start.Y + 3 + m_pBB->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X + 3, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height(), m_pBB->m_Start.X + 3, m_pAA->m_Start.Y + m_pAA->m_Size.height() );
  m_pOwner->Line( m_pBB->m_Start.X + 4, m_pBB->m_Start.Y + 2 + m_pBB->m_Size.height(), m_pBB->m_Start.X + 4, m_pAA->m_Start.Y + m_pAA->m_Size.height() );
  }

bool EdSqRoot::MoveInRight(EdList* &pL)
  {
  pL=m_pAA;
  pL->m_pCurr=m_pAA->m_pFirst;
  return true;
  }

bool EdSqRoot::MoveInLeft(EdList* &pL)
  {
  pL=m_pAA;
  pL->m_pCurr=NULL;
  return true;
  }

bool EdSqRoot::MoveToDown(EdList* &pL)
  {
  return false;
  }

bool EdSqRoot::MoveToUp(EdList* &pL)
  {
  return false;
  }

bool EdSqRoot::MoveToPrev(EdList* &pL)
  {
  return false;
  }

bool EdSqRoot::MoveToNext(EdList* &pL)
  {
  return false;
  }

QByteArray EdSqRoot::SWrite()
    {
    return "\\sqrt{" + m_pAA->SWrite() + "}";
    }

void EdVect::PreCalc( TPoint P, QSize &S, int&A )
  {
  TPoint P1;
  QSize S1;

  m_Start = P;

  P1.X = m_Start.X + 2;
  P1.Y = m_Start.Y + 8;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );

  m_Size.setWidth( S1.width() + 4 );
  DefineArrow( m_Start.X, m_Start.Y, m_Start.X + m_Size.width(), m_Start.Y, arVect, arX1,
    arY1, arX2, arY2 );

  dy = m_Start.Y - arY1;
  arY1 += dy;
  arY2 += dy;

  if( dy != 3 )
    {
    P1.Y += 2 * dy - 6;
    m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
    }
  m_Size.setHeight( S1.height() + max( 2 * dy, 6 ) + 2 );
  m_Axis = P1.Y - m_Start.Y + m_pAA->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdVect::Draw(TPoint P)
  {
  if ((m_Start.X != P.X) || (m_Start.Y != P.Y))
    {
    m_Start=P;
    PreCalc(m_Start, m_Size, m_Axis);
    }
  SelectRect();
  m_pAA->Draw(m_pAA->m_Start);
  QPen Pen = m_pOwner->m_pCanvas->pen();// Owner.Canvas.Pen := Owner.EditSets.SignPen;
 // Pen.setColor( m_pOwner->m_EditSets.m_SignColor );// Owner.Canvas.Pen.Color := Owner.EditSets.SignColor;
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line(m_Start.X, m_Start.Y+dy, m_Start.X+m_Size.width(), m_Start.Y+dy);
  m_pOwner->Line(arX1, arY1, m_Start.X+m_Size.width(), m_Start.Y+dy);
  m_pOwner->Line(arX2, arY2, m_Start.X+m_Size.width(), m_Start.Y+dy);
  }

QByteArray EdVect::Write()
  {
//  return '(' + QByteArray(VectName) + '(' + m_pAA->Write() + "))";
  return QByteArray(VectName) + '(' + m_pAA->Write() + ')';
  }

QByteArray EdVect::SWrite()
    {
    return "\\vector{" + m_pAA->SWrite() + '}';
    }

bool EdFrac::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pAA->SetCurrent(C, pSL, pCr) || m_pBB->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2 ) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdFrac::PreCalc( TPoint P, QSize &S, int &A )
  {
  m_Start = P;
  TPoint P1, P2;
  QSize S1, S2;
  P1 = m_Start;
  P1.X = m_Start.X;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
  P2.X = m_pAA->m_Start.X;
  P2.Y = m_pAA->m_Start.Y + S1.height() + 4;
  m_pBB->PreCalc( P2, S2, m_pBB->m_Axis );
  if( S1.width() < S2.width() )
    {
    m_pAA->m_Start.X = m_pAA->m_Start.X + ( S2.width() - S1.width() ) / 2;
    m_pAA->PreCalc( m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis );
    }
  if( S1.width() > S2.width() )
    {
    m_pBB->m_Start.X = m_pBB->m_Start.X + ( S1.width() - S2.width() ) / 2;
    m_pBB->PreCalc( m_pBB->m_Start, m_pBB->m_Size, m_pBB->m_Axis );
    }
  m_Axis = m_pAA->m_Size.height() + 2;
  m_Size.setWidth( max( m_pAA->m_Size.width(), m_pBB->m_Size.width() ) );
  m_Size.setHeight( m_pAA->m_Size.height() + m_pBB->m_Size.height() + 6 );
  S = m_Size;
  A = m_Axis;
  }

void EdFrac::Draw( TPoint P )
  {
  SelectRect();
  if( ( m_Start.X != P.X ) || ( m_Start.Y != P.Y ) )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  m_pAA->Draw( m_pAA->m_Start );
  m_pBB->Draw( m_pBB->m_Start );
  QPen Pen = m_pOwner->m_pCanvas->pen();
  //Pen.setColor(m_pOwner->m_EditSets.m_SignColor);
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line( min( m_pAA->m_Start.X, m_pBB->m_Start.X ), m_pAA->m_Start.Y + m_Axis,
    max( m_pAA->m_Start.X + m_pAA->m_Size.width(), m_pBB->m_Start.X + m_pBB->m_Size.width() ), m_pAA->m_Start.Y + m_Axis );
  }

bool EdFrac::MoveInRight(EdList* &pL)
  {
  if (m_pAA->m_Start.X <= m_pBB->m_Start.X)
    {
    pL=m_pAA;
    pL->m_pCurr=m_pAA->m_pFirst;
    }
  else
    {
    pL=m_pBB;
    pL->m_pCurr=m_pBB->m_pFirst;
    }
  return true;
  }

bool EdFrac::MoveInLeft(EdList* &pL)
  {
  if (m_pAA->m_Start.X <= m_pBB->m_Start.X)
    {
    pL=m_pAA;
    }
  else
    {
    pL=m_pBB;
    }
  pL->m_pCurr=NULL;
  return true;
  }

bool EdFrac::MoveToUp(EdList* &pL)
  {
  if (pL=m_pBB)
    {
    pL=m_pAA;
    pL->m_pCurr=m_pAA->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdFrac::MoveToDown(EdList* &pL)
  {
  if (pL=m_pAA)
    {
    pL=m_pBB;
    pL->m_pCurr=m_pBB->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

QByteArray EdFrac::SWrite()
    {
    QByteArray NOMstr, DENstr;
    NOMstr = m_pAA->SWrite();
    DENstr = m_pBB->SWrite();
    return "\\frac{" + NOMstr + "}{" + DENstr + "}";
    }

QByteArray EdFrac::Write()
  {
  QByteArray NOMstr, DENstr;
  NOMstr = m_pAA->Write();
  DENstr = m_pBB->Write();
  if( IsNumber( NOMstr ) && IsNumber( DENstr ) )
    return  "{(" + NOMstr + ")/(" + DENstr + ")}";
  return "((" + NOMstr + ")/(" + DENstr + "))";
  }

EdIntegr::EdIntegr( XPInEdit *pOwn, uchar Sign ) : EdTwo( pOwn ), m_pDD( new EdList( m_pOwner ) )
  {
  m_pDD->Append_Before( new EdChar( 'd', m_pOwner ) );
  QFont IntFont( pOwn->m_EditSets.m_MainFont );
  IntFont.setPointSize( IntFont.pointSize() * 1.5 );
  m_pIntegralSign = new EdBigChar( Sign, irNormal, "Black", pOwn, IntFont, 0 );
  }

void EdIntegr::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint PAA, PBB( P ), PD;
  QSize SAA, SBB, SD;
  int IntAxis;
  bool AHasIntegral = m_pAA->HasIntegral();
  bool BHasIntegral = m_pBB->HasIntegral();
  if( dynamic_cast< const EdDfIntegr * >( this ) == nullptr ) m_Start = P;
  m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
  if( AHasIntegral )
    m_pAA->PreCalc( m_Start, SAA, m_pAA->m_Axis );
  else
    m_pAA->PreCalc( P, SAA, m_pAA->m_Axis );
  if( BHasIntegral )
    m_pBB->PreCalc( m_Start, SBB, m_pBB->m_Axis );
  else
    m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  double MaxH = sm_SignSize == 0 ? max( SBB.height(), SAA.height() ) : sm_SignSize;
  if( Round( MaxH ) - m_Size.height() > 2 )
    {
    m_pIntegralSign->RecalcSize( MaxH / m_Size.height() );
    m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
    }
  int AAY = P.Y + IntAxis - SAA.height() / 2;
  PAA.X = P.X + m_pIntegralSign->Width() + 2;
  PAA.Y = AAY;
  if( AHasIntegral )
    {
    PAA.Y = m_Start.Y;
    AAY = m_pAA->IntegralDPos();
    }
  m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
  PD.X = PAA.X + SAA.width() + 5;
  PD.Y = AAY;
  m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
  int ax = max( AHasIntegral ? 0 : m_pAA->m_Axis, max( m_pDD->m_Axis, BHasIntegral ? 0 : m_pBB->m_Axis ) );
  m_Axis = AAY - m_Start.Y + ax;
  PBB.X = PD.X + SD.width();
  PBB.Y = AAY;
  if( BHasIntegral )
    PBB.Y = m_Start.Y + max( 0, m_Axis - m_pBB->m_Axis);
  m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  if( ax > m_pAA->m_Axis )
    {
    PAA.Y = AAY + ax - m_pAA->m_Axis;
    m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
    }
  if( ax > m_pDD->m_Axis )
    {
    PD.Y = AAY + ax - m_pDD->m_Axis;
    m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
    }
  if( ax > m_pBB->m_Axis )
    {
    PBB.Y = AAY + ax - m_pBB->m_Axis;
    m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
    }

  m_Size.setWidth( PBB.X + SBB.width() - P.X + 4 );
  if( AHasIntegral )
    m_Axis = m_pAA->m_Axis;
  if( BHasIntegral && m_pBB->m_Axis > m_Axis )
    {
    int dAxis = m_pBB->m_Axis - m_Axis;
    P.Y += dAxis;
    m_pIntegralSign->PreCalc( P, SD, IntAxis );
    PD.Y += dAxis;
    m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
    PAA.Y += dAxis;
    m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
    m_Axis = m_pBB->m_Axis;
    }
  if( sm_SignSize == 0 ) sm_SignSize = m_pIntegralSign->m_Size.height();
  S = m_Size;
  A = m_Axis;
  }

void EdIntegr::Draw( TPoint P )
  {
  if( ( m_Start.X != P.X ) || ( m_Start.Y != P.Y ) )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  m_pIntegralSign->Draw( m_pIntegralSign->m_Start );
  m_pAA->Draw( m_pAA->m_Start );
  m_pDD->Draw( m_pDD->m_Start );
  m_pBB->Draw( m_pBB->m_Start );
  sm_SignSize = 0;
  }

QByteArray EdIntegr::GetFragment()
  {
  if (m_Selected) return Write() + '&';
  return m_pAA->GetFragment() + m_pDD->GetFragment() + m_pBB->GetFragment();
  }

void EdIntegr::SelectFragment(QRect &FRect)
  {
  if (InRect(FRect) && ((m_Start.X+3)>FRect.left()))
    {
    m_Selected=true;
    m_pAA->Select();
    m_pBB->Select();
    m_pDD->Select();
    return;
    }
  EdTwo::SelectFragment(FRect);
  m_pDD->SelectFragment(FRect);
  }

bool EdIntegr::MoveToNext (EdList* &pL)
  {
  if (pL==m_pAA)
    {
    pL=m_pBB;
    pL->m_pCurr=m_pBB->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdIntegr::MoveToPrev (EdList* &pL)
  {
  if (pL==m_pBB)
    {
    pL=m_pAA;
    pL->m_pCurr=NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdIntegr::MoveInLeft(EdList* &pL)
  {
  pL=m_pBB;
  pL->m_pCurr=NULL;
  return true;
  }

bool EdIntegr::MoveInRight(EdList* &pL)
  {
  pL=m_pAA;
  pL->m_pCurr=m_pAA->m_pFirst;
  return true;
  }

QByteArray EdIntegr::SWrite()
    {
    return "\\int{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + '}';
    }

QByteArray EdIntegr::Write()
  {
  return '(' + QByteArray(IntegrName) + '(' + m_pAA->Write() + ',' + m_pBB->Write() + "))";
  }

EdDoubleIntegr::EdDoubleIntegr( XPInEdit *pOwn, uchar Sign ) : EdIntegr( pOwn, Sign ), m_pD2( new EdList( m_pOwner ) ),
  m_pRegion( new EdList( m_pOwner ) ), m_pVaR2( new EdList( m_pOwner ) )
  {
  m_pD2->Append_Before( new EdChar( 'd', m_pOwner ) );
  }

bool EdDoubleIntegr::SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr )
  {
  if (m_pVaR2->SetCurrent(C, pSL, pSL->m_pCurr) || m_pRegion->SetCurrent(C, pSL, pSL->m_pCurr))
    {
    pCr = pSL->m_pCurr;
    return true;
    }
  return EdIntegr::SetCurrent( C, pSL, pCr );
  }

void EdDoubleIntegr::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint PAA, PBB(P), PVar2(P), PD, PD2;
  QSize SAA, SBB, SVar2, SD, SD2;
  m_Start = P;
  m_pIntegralSign->PreCalc( P, m_Size, m_Axis );
  m_pAA->PreCalc( P, SAA, m_pAA->m_Axis );
  int ASignY = m_pAA->IntegralSignPos();
  m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  int BSignY = m_pBB->IntegralSignPos();
  m_pVaR2->PreCalc( PVar2, SVar2, m_pVaR2->m_Axis );
  int Var2SignY = m_pVaR2->IntegralSignPos();
  double MaxH = sm_SignSize == 0 ? max( max( SAA.height(), SBB.height() ), SVar2.height() ) : sm_SignSize;
  if( Round( MaxH ) - m_Size.height() > 2 )
    {
    m_pIntegralSign->RecalcSize( MaxH / m_Size.height() );
    m_pIntegralSign->PreCalc( P, m_Size, m_Axis );
    }
  int SignY = max( BSignY, max( ASignY, Var2SignY ) );
  PAA.Y = -1;
  PBB.Y = -1;
  PVar2.Y = -1;
  if( SignY > 0 )
    {
    P.Y = SignY;
    int IntAxis;
    m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
    m_Axis = IntAxis + SignY - m_Start.Y;
    if( ASignY > 0 ) PAA.Y = m_Start.Y + SignY - ASignY;
    if( BSignY > 0 ) PBB.Y = m_Start.Y + SignY - BSignY;
    if( Var2SignY > 0 ) PVar2.Y = m_Start.Y + SignY - Var2SignY;
    }
  PAA.X = P.X + m_pIntegralSign->Width() + 2;
  if( PAA.Y == -1 ) PAA.Y = m_Start.Y + m_Axis - SAA.height() / 2;
  m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
  PD.X = PAA.X + SAA.width() + 5;
  PD.Y = m_Axis;
  m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
  PD.Y = m_Start.Y + m_Axis - SD.height() / 2;
  m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
  PBB.X = PD.X + SD.width();
  if( PBB.Y == -1 ) PBB.Y = m_Start.Y + m_Axis - SBB.height() / 2;
  m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  PD2.X = PBB.X + SBB.width() + 5;
  PD2.Y = PD.Y;
  m_pD2->PreCalc( PD2, SD2, m_pD2->m_Axis );
  PVar2.X = PD2.X + SD2.width();
  if( PVar2.Y == -1 )  PVar2.Y = m_Start.Y + m_Axis - SVar2.height() / 2;
  m_pVaR2->PreCalc( PVar2, SVar2, m_pVaR2->m_Axis );
  /*
  int ax = max( max( m_pAA->m_Axis, max( m_pDD->m_Axis, m_pBB->m_Axis ) ), m_pVaR2->m_Axis);
  if( ax > m_pAA->m_Axis )
    {
    PAA.Y = AAY + ax - m_pAA->m_Axis;
    m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
    }
  if( ax > m_pDD->m_Axis )
    {
    PD.Y = AAY + ax - m_pDD->m_Axis;
    m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
    }
  if( ax > m_pBB->m_Axis )
    {
    PBB.Y = AAY + ax - m_pBB->m_Axis;
    m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
    }
  if( ax > m_pD2->m_Axis )
    {
    PD2.Y = AAY + ax - m_pD2->m_Axis;
    m_pD2->PreCalc( PD2, SD2, m_pD2->m_Axis );
    }
  if( ax > m_pVaR2->m_Axis )
    {
    PVar2.Y = AAY + ax - m_pVaR2->m_Axis;
    m_pVaR2->PreCalc( PVar2, SVar2, m_pVaR2->m_Axis );
    }
    */
  TPoint PR;
  QSize SR;
  PR.X = P.X;
  PR.Y = P.Y + m_Size.height() + 2;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pRegion->PreCalc( PR, SR, m_pRegion->m_Axis );
  if( m_pIntegralSign->m_Size.width() > SR.width() )
    {
    PR.X += ( m_pIntegralSign->m_Size.width() - SR.width() ) / 2;
    m_pRegion->PreCalc( PR, SR, m_pRegion->m_Axis );
    }
  m_pOwner->SetPowerSize( +1, BaseLevel );

  m_Size.setWidth( PVar2.X + SVar2.width() - m_Start.X + 4 );
  m_Size.setHeight( PR.Y + SR.height() - m_Start.Y );
  if( sm_SignSize == 0 ) sm_SignSize = m_pIntegralSign->m_Size.height();
  S = m_Size;
  A = m_Axis;
  }

void EdDoubleIntegr::Draw( TPoint P )
  {
  EdIntegr::Draw( P );
  m_pD2->Draw( m_pD2->m_Start );
  m_pVaR2->Draw( m_pVaR2->m_Start );
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pRegion->Draw( m_pRegion->m_Start );
  m_pOwner->SetPowerSize( -1, BaseLevel );
  }

QByteArray EdDoubleIntegr::GetFragment()
  {
  if( m_Selected ) return Write() + '&';
  return m_pAA->GetFragment() + m_pDD->GetFragment() + m_pBB->GetFragment() +
    m_pD2->GetFragment() + m_pVaR2->GetFragment() + m_pRegion->GetFragment();
  }

void EdDoubleIntegr::SelectFragment( QRect &FRect )
  {
  if( InRect( FRect ) && ( ( m_Start.X + 3 )>FRect.left() ) )
    {
    m_Selected = true;
    m_pAA->Select();
    m_pBB->Select();
    m_pDD->Select();
    m_pD2->Select();
    m_pVaR2->Select();
    m_pRegion->Select();
    return;
    }
  EdIntegr::SelectFragment( FRect );
  m_pD2->Select();
  m_pVaR2->Select();
  m_pRegion->Select();
  }

bool EdDoubleIntegr::MoveToNext( EdList* &pL )
  {
  if( EdIntegr::MoveToNext( pL ) ) return true;
  if( pL == m_pBB )
    {
    pL = m_pVaR2;
    pL->m_pCurr = m_pVaR2->m_pFirst;
    return true;
    }
  return false;
  }

bool EdDoubleIntegr::MoveToPrev( EdList* &pL )
  {
  if( pL == m_pVaR2 )
    {
    pL = m_pBB;
    pL->m_pCurr = NULL;
    return true;
    }
  return EdIntegr::MoveToPrev( pL );
  }

bool EdDoubleIntegr::MoveInLeft( EdList* &pL )
  {
  pL = m_pVaR2;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdDoubleIntegr::MoveToUp( EdList* &pL )
  {
  if( pL != m_pRegion ) return false;
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdDoubleIntegr::MoveToDown( EdList* &pL )
  {
  if( pL == m_pRegion ) return false;
  pL = m_pRegion;
  pL->m_pCurr = NULL;
  return true;
  }

QByteArray EdDoubleIntegr::SWrite()
  {
  return "\\dblint{" + m_pRegion->SWrite() + "}{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + "}{" + m_pVaR2->SWrite() + '}';
  }

QByteArray EdDoubleIntegr::Write()
  {
  return '(' + QByteArray( DoubleIntegrName ) + '(' + m_pAA->Write() + ',' + m_pRegion->Write() + ',' + m_pBB->Write() +
    ',' + m_pVaR2->Write() + "))";
  }

EdTripleIntegr::EdTripleIntegr( XPInEdit *pOwn ) : EdDoubleIntegr( pOwn, msTripleIntegral ), m_pD3( new EdList( m_pOwner ) ),
 m_pVaR3( new EdList( m_pOwner ) )
  {

  m_pD3->Append_Before( new EdChar( 'd', m_pOwner ) );
  }

bool EdTripleIntegr::SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr )
  {
  pSL = m_pVaR3;
  if( pSL->SetCurrent( C, pSL, pSL->m_pCurr ) )
    {
    pCr = pSL->m_pCurr;
    return true;
    }
  if( ( m_pVaR3->m_pFirst == NULL ) &&
    ( m_pVaR3->m_Start.X <= C.X ) &&
    ( m_pVaR3->m_Start.X + m_pBB->m_Size.width() >= C.X ) &&
    ( m_pVaR3->m_Start.Y <= C.Y ) &&
    ( m_pVaR3->m_Start.Y + m_pBB->m_Size.height() >= C.Y ) )
    {
    pCr = NULL;
    return true;
    }
  return EdDoubleIntegr::SetCurrent( C, pSL, pCr );
  }

void EdTripleIntegr::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint PAA, PBB( P ), PVar2( P ), PD, PD2, PVar3( P ), PD3;
  QSize SAA, SBB, SVar2, SVar3, SD, SD2, SD3;
  int IntAxis;
  m_Start = P;
  m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
  m_pAA->PreCalc( P, SAA, m_pAA->m_Axis );
  m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  m_pVaR2->PreCalc( PVar2, SVar2, m_pVaR2->m_Axis );
  m_pVaR3->PreCalc( PVar3, SVar3, m_pVaR3->m_Axis );
  double MaxH = sm_SignSize == 0 ? max( max( max( SAA.height(), SBB.height() ), SVar2.height() ), SVar3.height() ) : sm_SignSize;
  if( Round( MaxH ) - m_Size.height() > 2 )
    {
    m_pIntegralSign->RecalcSize( MaxH / m_Size.height() );
    m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
    }
  int AAY = m_Start.Y + IntAxis - SAA.height() / 2;
  PAA.X = P.X + m_pIntegralSign->Width() + 2;
  PAA.Y = AAY;
  m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
  PD.X = PAA.X + SAA.width() + 5;
  PD.Y = AAY;
  m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
  PBB.X = PD.X + SD.width();
  PBB.Y = AAY;
  m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  PD2.X = PBB.X + SBB.width() + 5;
  PD2.Y = AAY;
  m_pD2->PreCalc( PD2, SD2, m_pD2->m_Axis );
  PVar2.X = PD2.X + SD2.width();
  PVar2.Y = AAY;
  m_pVaR2->PreCalc( PVar2, SVar2, m_pVaR2->m_Axis );
  PD3.X = PVar2.X + SVar2.width() + 5;
  PD3.Y = AAY;
  m_pD3->PreCalc( PD3, SD3, m_pD3->m_Axis );
  PVar3.X = PD3.X + SD3.width();
  PVar3.Y = AAY;
  m_pVaR3->PreCalc( PVar3, SVar3, m_pVaR3->m_Axis );

  int ax = max( max( max( m_pAA->m_Axis, max( m_pDD->m_Axis, m_pBB->m_Axis ) ), m_pVaR2->m_Axis ), m_pVaR3->m_Axis );
  if( ax > m_pAA->m_Axis )
    {
    PAA.Y = AAY + ax - m_pAA->m_Axis;
    m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
    }
  if( ax > m_pDD->m_Axis )
    {
    PD.Y = AAY + ax - m_pDD->m_Axis;
    m_pDD->PreCalc( PD, SD, m_pDD->m_Axis );
    }
  if( ax > m_pBB->m_Axis )
    {
    PBB.Y = AAY + ax - m_pBB->m_Axis;
    m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
    }
  if( ax > m_pD2->m_Axis )
    {
    PD2.Y = AAY + ax - m_pD2->m_Axis;
    m_pD2->PreCalc( PD2, SD2, m_pD2->m_Axis );
    }
  if( ax > m_pVaR2->m_Axis )
    {
    PVar2.Y = AAY + ax - m_pVaR2->m_Axis;
    m_pVaR2->PreCalc( PVar2, SVar2, m_pVaR2->m_Axis );
    }
  if( ax > m_pD3->m_Axis )
    {
    PD3.Y = AAY + ax - m_pD3->m_Axis;
    m_pD3->PreCalc( PD3, SD3, m_pD3->m_Axis );
    }
  if( ax > m_pVaR3->m_Axis )
    {
    PVar3.Y = AAY + ax - m_pVaR3->m_Axis;
    m_pVaR3->PreCalc( PVar3, SVar3, m_pVaR3->m_Axis );
    }

  TPoint PR;
  QSize SR;
  PR.X = P.X;
  PR.Y = P.Y + m_Size.height() + 2;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pRegion->PreCalc( PR, SR, m_pRegion->m_Axis );
  if( m_pIntegralSign->m_Size.width() > SR.width() )
    {
    PR.X += ( m_pIntegralSign->m_Size.width() - SR.width() ) / 2;
    m_pRegion->PreCalc( PR, SR, m_pRegion->m_Axis );
    }
  m_pOwner->SetPowerSize( +1, BaseLevel );

  m_Size.setWidth( PVar3.X + SVar3.width() - m_Start.X + 4 );
  m_Size.setHeight( PR.Y + SR.height() - P.Y );
  m_Axis = AAY - m_Start.Y + ax;
  if( sm_SignSize == 0 ) sm_SignSize = m_pIntegralSign->m_Size.height();
  S = m_Size;
  A = m_Axis;
  }

void EdTripleIntegr::Draw( TPoint P )
  {
  EdDoubleIntegr::Draw( P );
  m_pD3->Draw( m_pD3->m_Start );
  m_pVaR3->Draw( m_pVaR3->m_Start );
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pRegion->Draw( m_pRegion->m_Start );
  m_pOwner->SetPowerSize( -1, BaseLevel );
  }

QByteArray EdTripleIntegr::GetFragment()
  {
  if( m_Selected ) return Write() + '&';
  return m_pAA->GetFragment() + m_pDD->GetFragment() + m_pBB->GetFragment() +
    m_pD2->GetFragment() + m_pVaR2->GetFragment() + m_pD3->GetFragment() + m_pVaR3->GetFragment() + m_pRegion->GetFragment();
  }

void EdTripleIntegr::SelectFragment( QRect &FRect )
  {
  if( InRect( FRect ) && ( ( m_Start.X + 3 )>FRect.left() ) )
    {
    m_Selected = true;
    m_pAA->Select();
    m_pBB->Select();
    m_pDD->Select();
    m_pD2->Select();
    m_pVaR2->Select();
    m_pD3->Select();
    m_pVaR3->Select();
    m_pRegion->Select();
    return;
    }
  EdDoubleIntegr::SelectFragment( FRect );
  m_pD3->Select();
  m_pVaR3->Select();
  }

bool EdTripleIntegr::MoveToNext( EdList* &pL )
  {
  if( EdDoubleIntegr::MoveToNext( pL ) ) return true;
  if( pL == m_pVaR2 )
    {
    pL = m_pVaR3;
    pL->m_pCurr = m_pVaR3->m_pFirst;
    return true;
    }
  return false;
  }

bool EdTripleIntegr::MoveToPrev( EdList* &pL )
  {
  if( pL == m_pVaR3 )
    {
    pL = m_pVaR2;
    pL->m_pCurr = NULL;
    return true;
    }
  return EdDoubleIntegr::MoveToPrev( pL );
  }

bool EdTripleIntegr::MoveInLeft( EdList* &pL )
  {
  pL = m_pVaR3;
  pL->m_pCurr = NULL;
  return true;
  }

QByteArray EdTripleIntegr::SWrite()
  {
  return "\\trplint{" + m_pRegion->SWrite() + "}{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + "}{" + m_pVaR2->SWrite() + "}{" + m_pVaR3->SWrite() + '}';
  }

QByteArray EdTripleIntegr::Write()
  {
  return '(' + QByteArray( TripleIntegrName ) + '(' + m_pAA->Write() + ',' + m_pRegion->Write() + ',' + m_pBB->Write() +
    ',' + m_pVaR2->Write() + ',' + m_pVaR3->Write() + "))";
  }

EdCurveIntegr::EdCurveIntegr( XPInEdit *pOwn, uchar Sign ) : EdTwo( pOwn )
  {
  QFont IntFont( pOwn->m_EditSets.m_MainFont );
  IntFont.setPointSize( IntFont.pointSize() * 1.5 );
  m_pIntegralSign = new EdBigChar( Sign, irNormal, "Black", pOwn, IntFont, Sign == msContourIntegral ? -1 : 0 );
  }

void EdCurveIntegr::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint PAA(P), PBB;
  QSize SAA, SBB;
  int IntAxis;
  m_Start = P;
  m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
  m_pAA->PreCalc( P, SAA, m_pAA->m_Axis );
  double MaxH = EdIntegr::sm_SignSize == 0 ? SAA.height() : EdIntegr::sm_SignSize;
  if( Round( MaxH ) - m_Size.height() > 2 )
    {
    m_pIntegralSign->RecalcSize( MaxH / m_Size.height() );
    m_pIntegralSign->PreCalc( P, m_Size, IntAxis );
    }
  int dH = max(0, m_pAA->m_Axis - IntAxis);
  if(dH > 0)
    {
    TPoint IntP(P);
    IntP.Y += dH;
    m_pIntegralSign->PreCalc( IntP, m_Size, IntAxis );
    m_Size.setHeight(m_Size.height() + dH);
    }
  PBB.X = P.X;
  PBB.Y = P.Y + m_Size.height() + 2;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
  if( m_Size.width() > SBB.width() )
    {
    PBB.X += ( m_Size.width() - SBB.width() ) / 2;
    m_pBB->PreCalc( PBB, SBB, m_pBB->m_Axis );
    }
  m_pOwner->SetPowerSize( +1, BaseLevel );
  if( m_Size.width() < SBB.width() )
    {
    PAA.X += ( SBB.width() - m_Size.width() ) / 2;
    m_pIntegralSign->PreCalc( PAA, m_Size, IntAxis );
    }
  int AAY = m_Start.Y + IntAxis - SAA.height() / 2 + dH;
  PAA.X += m_pIntegralSign->Width() + 2;
  PAA.Y = AAY;
  m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
  m_Size.setWidth( max( SBB.width(), ( PAA.X + SAA.width() - P.X ) ) + 4 );
  m_Size.setHeight( PBB.Y + SBB.height() - P.Y );
  m_Axis = AAY - m_Start.Y + m_pAA->m_Axis;
  if( EdIntegr::sm_SignSize == 0 ) EdIntegr::sm_SignSize = m_pIntegralSign->m_Size.height();
  S = m_Size;
  A = m_Axis;
  }

void EdCurveIntegr::Draw( TPoint P )
  {
  if( ( m_Start.X != P.X ) || ( m_Start.Y != P.Y ) )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  m_pIntegralSign->Draw( m_pIntegralSign->m_Start );
  m_pAA->Draw( m_pAA->m_Start );
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pBB->Draw( m_pBB->m_Start );
  m_pOwner->SetPowerSize( -1, BaseLevel );
  EdIntegr::sm_SignSize = 0;
  }

QByteArray EdCurveIntegr::GetFragment()
  {
  if( m_Selected ) return Write() + '&';
  return m_pAA->GetFragment() + m_pBB->GetFragment();
  }

void EdCurveIntegr::SelectFragment( QRect &FRect )
  {
  if( InRect( FRect ) && ( ( m_Start.X + 3 )>FRect.left() ) )
    {
    m_Selected = true;
    m_pAA->Select();
    m_pBB->Select();
    return;
    }
  EdTwo::SelectFragment( FRect );
  }

bool EdCurveIntegr::MoveInLeft( EdList* &pL )
  {
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdCurveIntegr::MoveInRight( EdList* &pL )
  {
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdCurveIntegr::MoveToUp( EdList* &pL )
  {
  if( pL != m_pBB ) return false;
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdCurveIntegr::MoveToDown( EdList* &pL )
  {
  if( pL == m_pBB ) return false;
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

QByteArray EdCurveIntegr::SWrite()
  {
  return "\\crvint{" + m_pIntegralSign->SWrite() + "}{" + m_pBB->SWrite() + "}{" + m_pAA->SWrite() + '}';
  }

QByteArray EdCurveIntegr::Write()
  {
  QByteArray Result = '(' + m_pAA->Write() + ',' + m_pBB->Write() + ')';
  switch( m_pIntegralSign->c() )
    {
    case msIntegral:
      return CurveIntegrName + Result;
    case msDoubleIntegral:
      return SurfaceIntegrName + Result;
    }
  return ContourIntegrName + Result;
  }

bool EdLg::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pNN->SetCurrent(C, pSL, pCr) || m_pAA->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdLg::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint P1, P2, PN;
  QSize S1, SN;
  m_Start = P;
  PN = P;
  m_pNN->PreCalc( PN, SN, m_pNN->m_Axis );
  P1.X = PN.X + SN.width() + 4;
  P1.Y = PN.Y;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
  if( ( m_pNN->m_Axis ) < ( m_pAA->m_Axis ) )
    {
    PN.Y += m_pAA->m_Axis - m_pNN->m_Axis;
    m_pNN->PreCalc( PN, SN, m_pNN->m_Axis );
    }
  if( m_pAA->m_Axis < m_pNN->m_Axis )
    {
    P1.Y += m_pNN->m_Axis - m_pAA->m_Axis;
    m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
    }
  m_Size.setHeight( P1.Y + S1.height() - m_Start.Y );
  P2 = m_pAA->m_Start;
  m_pB1->PreCalc( P2, m_pB1->m_Size, m_pB1->m_Axis );
  m_pB1->m_pFirst->m_pMember->m_Size.setHeight( m_pAA->m_Size.rheight() );

  int BrackWidth = m_pOwner->CharWidth( '(' );
  m_pAA->m_Start.X += BrackWidth;
  m_pAA->PreCalc( m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis );

  P2.Y = m_pAA->m_Start.Y;
  P2.X = m_pAA->m_Start.X + m_pAA->m_Size.width();
  m_pB2->PreCalc( P2, m_pB2->m_Size, m_pB2->m_Axis );
  m_pB2->m_pFirst->m_pMember->m_Size.setHeight( m_pAA->m_Size.height() );
  m_Size.setWidth( P2.X + BrackWidth - m_Start.X );

  m_Axis = P1.Y - m_Start.Y + m_pAA->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdLg::Draw (TPoint P)
  {
  if ((m_Start.X != P.X) || (m_Start.Y != P.Y))
    {
    m_Start=P;
    PreCalc (m_Start, m_Size, m_Axis);
    }
  SelectRect();
  m_pNN->Draw( m_pNN->m_Start );
  m_pB1->m_pFirst->m_pMember->Draw(m_pB1->m_Start);
  m_pAA->Draw(m_pAA->m_Start);
  m_pB2->m_pFirst->m_pMember->Draw(m_pB2->m_Start);
  }

void EdLg::ClearSelection()
  {
  m_pAA->ClearSelection();
  m_pNN->ClearSelection();
  m_pB1->ClearSelection();
  m_pB2->ClearSelection();
  m_Selected=false;
  }

void EdLg::SelectFragment(QRect &FRect)
  {
  m_pAA->SelectFragment(FRect);
  m_pNN->SelectFragment(FRect);
  m_pB1->SelectFragment(FRect);
  m_pB2->SelectFragment(FRect);
  m_Selected=(m_pNN->m_Selected && m_pB1->m_Selected && m_pAA->m_Selected);
  }

  QByteArray EdLg::GetFragment()
    {
    if (m_Selected)  return Write() + '&';
    return m_pNN->GetFragment() + m_pB1->GetFragment() + m_pB2->GetFragment();
    }

  QByteArray EdLg::Write()
    {
    return m_pNN->Write() + '(' + m_pAA->Write() + ')';
    }

  bool EdLg::MoveInRight(EdList* &pL)
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }

  bool EdLg::MoveInLeft(EdList* &pL)
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }

  bool EdLg::MoveToNext(EdList* &pL)
    {
    if (pL == m_pNN)
      {
      pL = m_pAA;
      pL->m_pCurr = m_pAA->m_pFirst;
      return true;
      }
    else
      {
      return false;
      }
    }

  bool EdLg::MoveToPrev(EdList* &pL)
    {
    if (pL == m_pAA)
      {
      pL = m_pNN;
      pL->m_pCurr = m_pNN->m_pLast;
      return true;
      }
    else
      {
      return false;
      }
    }

  QByteArray EdLg::SWrite()
    {
    return "\\" + m_pNN->SWrite() + '{' + m_pAA->SWrite() + '}';//???? ?????? 28.10.2015
    }

  EdMatr::EdMatr(const QByteArray& Size, XPInEdit *pOwn, bool CreateNew) : m_Row(0), m_Col(0)
    {
    m_pOwner = pOwn;
    m_IsVisible = true;

    m_RowCount = Size.left(Size.indexOf(',')).toInt();
    m_ColCount = Size.mid(Size.indexOf(',') + 1).toInt();
    m_pAA = new EdMatrixBody(pOwn);
//    if (m_RowCount < 2) return;
    XPInEdit::sm_pMatr = this;
    m_Spaces.resize(m_RowCount);
    for (int i = m_RowCount - 1; i > -1; i--)
      {
      m_Spaces[i].resize(m_ColCount + 1);
      m_Spaces[i][m_ColCount] = new EdSpace(false, nullptr, nullptr, pOwn);
      for (int j = m_ColCount - 1; j > -1; j--)
        if (i == m_RowCount - 1)
          m_Spaces[i][j] = new EdSpace(false, nullptr, m_Spaces[i][j + 1], pOwn);
        else
          m_Spaces[i][j] = new EdSpace(i == 0, m_Spaces[i + 1][j], m_Spaces[i][j + 1], pOwn);
      }
    m_Spaces[0][m_ColCount]->m_Last = true;
    if (!CreateNew) return;
    for (; m_Row < m_RowCount; m_Row++)
      {
      for (m_Col = 0; m_Col < m_ColCount; m_Col++)
        {
        m_pAA->Append_Before(m_Spaces[m_Row][m_Col]);
        m_pAA->Append_Before(new EdChar(msIdle, pOwn));
        if(m_Col < m_ColCount - 1) m_pAA->Append_Before(new EdChar(' ', pOwn));
        }
      m_pAA->Append_Before(m_Spaces[m_Row][m_Col]);
      if (m_Row < m_RowCount - 1)
        {
        m_pOwner->SetVisible(false);
        m_pAA->Append_Before(new EdChar(';', pOwn));
        m_pOwner->SetVisible(true);
        }
      }
    XPInEdit::sm_pMatr = nullptr;
    m_pAA->m_pCurr = m_pAA->m_pFirst->m_pNext;
    }

  void EdMatr::AddSpacer()
    {
    m_pOwner->AddElement(m_Spaces[m_Row][m_Col]);
    if (++m_Col > m_ColCount)
      {
      if (++m_Row == m_RowCount)
        {
        XPInEdit::sm_pMatr = nullptr;
        return;
        }
      m_Col = 0;
      }
    }

#define EDCHAR (pEch = dynamic_cast<EdChar*>(pIndex->m_pMember.data())) != nullptr && pEch->c() != ' ' && pEch->c() != ','

  bool EdMatr::MoveToUp(EdList* &pL)
    {
    EdMemb *pIndex = pL->m_pCurr;
    if (pIndex == nullptr) pIndex = pL->m_pLast;
    int iPos = 0;
    EdChar *pEch;
    for (bool Nochar, StartElement = true; pIndex != nullptr && ((Nochar = !(EDCHAR)) || pEch->c() != ';'); pIndex = pIndex->m_pPrev)
      if (Nochar)
        StartElement = true;
      else
        if (StartElement)
          {
          StartElement = false;
          iPos++;
          }
    if (pIndex == nullptr) return false;
    pIndex = pIndex->m_pPrev;
    iPos = m_ColCount - iPos;
    for (bool Nochar, StartElement = false; pIndex != nullptr && iPos != 0 && ((Nochar = !(EDCHAR)) || pEch->c() != ';'); pIndex = pIndex->m_pPrev)
      if (Nochar)
        StartElement = true;
      else
        if (StartElement)
          {
          StartElement = false;
          if (--iPos == 0) break;
          }
    if (pIndex == nullptr) return false;
    pL->m_pCurr = pIndex;
    return true;
    }

bool EdMatr::MoveToDown(EdList* &pL)
  {
  EdMemb *pIndex = pL->m_pCurr;
  if (pIndex == nullptr) return false;;
  int iPos = 0;
  EdChar *pEch;
  for (bool Nochar, StartElement = true; pIndex != nullptr && ((Nochar = !(EDCHAR)) || pEch->c() != ';'); pIndex = pIndex->m_pNext)
    if (Nochar)
      StartElement = true;
    else
      if (StartElement)
        {
        StartElement = false;
        iPos++;
        }
  if (pIndex == nullptr) return false;
  iPos = m_ColCount - iPos;
  pIndex = pIndex->m_pNext;
  for (bool Nochar, StartElement = false; pIndex != nullptr && iPos != 0 && ((Nochar = !(EDCHAR)) || pEch->c() != ';'); pIndex = pIndex->m_pNext)
    if (Nochar)
      StartElement = true;
    else
      if (StartElement)
        {
        StartElement = false;
        if (--iPos == 0) break;
        }
  pL->m_pCurr = pIndex;
  if (pIndex->Protected() && !pIndex->IdleChar() && pIndex->m_pNext->Protected()) pL->MoveRight(pL);
  return true;
  }

void EdMatr::PreCalc(TPoint P, QSize &S, int &A)
  {
  EdMemb *index;
  EdAbs::PreCalc (P, S, A);
  m_pOwner->m_RecalcSpaces=true;
  EdAbs::PreCalc (P, S, A);
  index = m_pAA->m_pFirst;
  while ( index )
    {
    if (index->StrSeparat())
      {
      m_Axis=m_Size.height()/2;
      break;
      }
    index=index->m_pNext;
    }
  S = m_Size;
  A = m_Axis;
  }

void EdMatr::Draw (TPoint P)
  {
  SelectRect();
  EdAbs::Draw( P );
  m_pAA->Draw( m_pAA->m_Start );

   if( !m_IsVisible ) return;
   m_pOwner->Line(m_Start.X + 4, m_Start.Y, m_Start.X + 8, m_Start.Y);
   m_pOwner->Line(m_Start.X + 4, m_Start.Y + m_Size.height() - 1, m_Start.X + 8, m_Start.Y + m_Size.height() - 1 );
   m_pOwner->Line(m_pAA->m_Start.X + m_pAA->m_Size.width() + 4, m_Start.Y, m_pAA->m_Start.X + m_pAA->m_Size.width(), m_Start.Y);
   m_pOwner->Line(m_pAA->m_Start.X + m_pAA->m_Size.width() + 4, m_Start.Y + m_Size.height() -1, m_pAA->m_Start.X + m_pAA->m_Size.width(), m_Start.Y + m_Size.height() -1);
  }

QByteArray EdMatr::Write()
  {
  return '(' + QByteArray(MatrName) + '(' + m_pAA->Write() + "))";
  }

QByteArray EdMatr::SWrite()
    {
  QByteArray Result = "\\matrix{" + QByteArray::number(m_RowCount) + ',' + QByteArray::number(m_ColCount) + "}{\\spacer\n";
  for (EdMemb *pIndex = m_pAA->m_pFirst; pIndex != nullptr; pIndex = pIndex->m_pNext)
    {
    EdChar *pEch = dynamic_cast<EdChar*>(pIndex->m_pMember.data());
    if (pEch == nullptr) continue;
    char c = pEch->c();
    switch (c)
      {
        case ' ':
          Result += "\\setvisible\n,\\spacer\n";
          continue;
        case ';':
          Result += "\\spacer\n\\setunvisible\n;\\setvisible\n\\spacer\n";
          continue;
      }
    Result += c;
    }
  return Result += "\\spacer\n\\setvisible\n}";
    }

bool EdLimit::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pAA->SetCurrent(C, pSL, pCr) || m_pVL->SetCurrent(C, pSL, pCr) || m_pEE->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdLimit::PreCalc(TPoint P, QSize &S, int &A)
  {
  TPoint P1,PL,PV,PN,PR;
  QSize S1,SL,SV,SN,SR;
  int dx,by,ax;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_Start = P;
  PN = P;
  m_pNN->PreCalc(PN, SN, m_pNN->m_Axis);

  P1.X = PN.X +SN.width() + 4;
  P1.Y = PN.Y;
  m_pAA->PreCalc (P1, S1, m_pAA->m_Axis);

  if( m_pNN->m_Axis > m_pAA->m_Axis )
    {
    P1.Y += m_pNN->m_Axis - m_pAA->m_Axis;
    m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
    }

  if( m_pNN->m_Axis < m_pAA->m_Axis )
    {
    PN.Y += m_pAA->m_Axis - m_pNN->m_Axis;
    m_pNN->PreCalc( PN, SN, m_pNN->m_Axis );
    }

  by = PN.Y + SN.height() + 3;

  m_pOwner->SetPowerSize(-1, BaseLevel);

  PV.X = P.X;
  PV.Y = by;
  m_pVL->PreCalc (PV, SV, m_pVL->m_Axis);

  PR.X = PV.X + SV.width();
  PR.Y = by;
  m_pRR->PreCalc (PR, SR, m_pRR->m_Axis);

  PL.X = PR.X + SR.width();
  PL.Y = by;
  m_pEE->PreCalc (PL, SL, m_pEE->m_Axis);

  ax = max ( (max ( m_pVL->m_Axis, m_pRR->m_Axis )), m_pEE->m_Axis );
 if (m_pVL->m_Axis < ax)
    {
    PV.Y+=ax-m_pVL->m_Axis;
    m_pVL->PreCalc(PV, SV, m_pVL->m_Axis);
    }
  if (m_pRR->m_Axis < ax)
    {
    PR.Y+=ax-m_pRR->m_Axis;
    m_pRR->PreCalc(PR, SR, m_pRR->m_Axis);
    }
  if( m_pEE->m_Axis < ax )
    {
    PL.Y += ax - m_pEE->m_Axis;
    m_pEE->PreCalc( PL, SL, m_pEE->m_Axis );
    }

  dx = PL.X + SL.width() - PV.X;

  if( dx < SN.width() )
    {
    PV.X += ( SN.width() - dx ) / 2;
    PR.X += ( SN.width() - dx ) / 2;
    PL.X += ( SN.width() - dx ) / 2;
    m_pVL->PreCalc( PV, SV, m_pVL->m_Axis );
    m_pRR->PreCalc( PR, SR, m_pRR->m_Axis );
    m_pEE->PreCalc( PL, SL, m_pEE->m_Axis );
    }

  m_pOwner->SetPowerSize (+1, BaseLevel);

  if( dx > SN.width() )
    {
    PN.X += ( dx - SN.width() ) / 2;
    P1.X += dx - SN.width();
    m_pNN->PreCalc( PN, SN, m_pNN->m_Axis );
    m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );
    }

  m_Size.setWidth(max(P1.X+S1.width(), PL.X +SL.width()));
  m_Size.setWidth(m_Size.width()-m_Start.X);
  m_Size.setWidth(m_Size.width() + 4);
  //m_Size.setHeight(max (PV.Y + SV.height(), PR.Y + SR.height()));
  //m_Size.setHeight(max ( max (PV.Y + SV.height(), PR.Y + SR.height()) , PL.Y + SL.height()));
  m_Size.setHeight (max ((max ( max (PV.Y + SV.height(), PR.Y + SR.height()) , PL.Y + SL.height())) , P1.Y + S1.height()));
  m_Size.setHeight(m_Size.height() - m_Start.Y);
  m_Axis = P1.Y - m_Start.Y + m_pAA->m_Axis;
  S =  m_Size;
  A = m_Axis;
  }

void EdLimit::Draw (TPoint P)
  {
  int arX1, arY1, arX2, arY2;

  if (m_Start.X != P.X || m_Start.Y != P.Y)
    {
    m_Start=P;
    PreCalc (m_Start, m_Size, m_Axis);
    }
  SelectRect();
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pNN->Draw (m_pNN->m_Start);
  m_pAA->Draw (m_pAA->m_Start);

  m_pOwner->SetPowerSize (-1, BaseLevel);
  m_pVL->Draw (m_pVL->m_Start);
  m_pEE->Draw (m_pEE->m_Start);
  m_pOwner->SetPowerSize (+1, BaseLevel);

  DefineArrow (m_pRR->m_Start.X + 2, m_pRR->m_Start.Y + m_pRR->m_Axis, m_pRR->m_Start.X + m_pRR->m_Size.width() - 2,
    m_pRR->m_Start.Y + m_pRR->m_Axis, arLim, arX1, arY1, arX2, arY2);

  QPen Pen = m_pOwner->m_pCanvas->pen();
   //Pen.setColor( m_pOwner->m_EditSets.m_SignColor );
   m_pOwner->m_pCanvas->setPen( Pen );

   m_pOwner->Line(m_pRR->m_Start.X + 2, m_pRR->m_Start.Y + m_pRR->m_Axis, m_pRR->m_Start.X + m_pRR->m_Size.width() - 2, m_pRR->m_Start.Y + m_pRR->m_Axis);
   m_pOwner->Line(arX1, arY1, m_pRR->m_Start.X + m_pRR->m_Size.width() - 2, m_pRR->m_Start.Y + m_pRR->m_Axis );
   m_pOwner->Line(arX2, arY2, m_pRR->m_Start.X + m_pRR->m_Size.width() - 2, m_pRR->m_Start.Y + m_pRR->m_Axis );
  }

QByteArray EdLimit::GetFragment ()
  {
  if (m_Selected) return Write() + '&';
  return m_pVL->GetFragment() + m_pEE->GetFragment() + m_pAA->GetFragment();
  }

void EdLimit::SelectFragment (QRect &FRect)
  {
  m_pNN->SelectFragment(FRect);
  m_pAA->SelectFragment(FRect);
  m_pVL->SelectFragment(FRect);
  m_pEE->SelectFragment(FRect);
  m_Selected = m_pNN->m_Selected;
  if (m_Selected)
    {
    m_pAA->Select();
    m_pVL->Select();
    m_pEE->Select();
    }
  }

void EdLimit::ClearSelection ()
  {
  m_pAA->ClearSelection();
  m_pVL->ClearSelection();
  m_pEE->ClearSelection();
  m_pNN->ClearSelection();
  m_Selected = false;
  }

QByteArray EdLimit::Write()
  {
  return '(' + QByteArray( LimitName ) + '(' + m_pAA->Write() + ',' + m_pVL->Write() + ',' + m_pEE->Write() + "))";
  }

bool EdLimit::MoveInRight (EdList* &pL)
  {
  pL = m_pVL;
  pL->m_pCurr = m_pVL->m_pFirst;
  return true;
  }

bool EdLimit::MoveInLeft (EdList* &pL)
  {
  pL=m_pAA;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdLimit::MoveToNext (EdList* &pL)
  {
  if (pL == m_pVL)
    {
    pL=m_pEE;
    pL->m_pCurr = m_pEE->m_pFirst;
    return true;
    }
  else
    {
    if (pL == m_pEE)
      {
       pL=m_pAA;
       pL->m_pCurr = m_pAA->m_pFirst;
       return true;
      }
    else
      {
      return false;
      }
    }
  }

bool EdLimit::MoveToPrev (EdList* &pL)
  {
  if (pL == m_pAA)
    {
    pL=m_pEE;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    if (pL == m_pEE)
      {
       pL=m_pVL;
       pL->m_pCurr = NULL;
       return true;
      }
    else
      {
      return false;
      }
    }
  }

bool EdLimit::MoveToUp (EdList* &pL)
  {
  if (pL == m_pVL || pL == m_pEE)
    {
    pL=m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdLimit::MoveToDown (EdList* &pL)
  {
  if (pL == m_pAA)
    {
    pL = m_pEE;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

QByteArray EdLimit::SWrite()
    {
    return "\\lim{" + m_pVL->SWrite() + "}{" + m_pEE->SWrite() + "}{" + m_pAA->SWrite() + '}';
    }

void EdImUnit::PreCalc (TPoint P, QSize &S, int &A)
  {
  m_pAA->PreCalc (m_Start, m_Size, m_pAA->m_Axis);
  m_Axis = m_pAA->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdImUnit::Draw (TPoint P)
  {
  if (m_Start.X != P.X || m_Start.Y != P.Y)
    {
    m_Start = P;
    }
  SelectRect();
  m_pAA->Draw(m_Start);
  }

QByteArray EdImUnit::Write()
  {
  return QByteArray(1, msImUnit);
  }

QByteArray EdImUnit::SWrite()
    {
    return "\\im";
    }

bool EdImUnit::MoveInRight (EdList* &pL)
  {
  return false;
  }
bool EdImUnit::MoveInLeft (EdList* &pL)
  {
  return false;
  }

EdStrokeDeriv::EdStrokeDeriv(XPInEdit *pOwn) : EdPowr(pOwn), m_Order(-1)
  {
  m_AsIndex = false;
  }

void EdStrokeDeriv::PreCalc( TPoint P, QSize &S, int &A )
  {
  if( m_pBB->m_pFirst == nullptr )
    {
    if( m_Order > 0 ) m_Order--;
    if( m_Order == -1 ) m_Order = 0;
    }
  else
    if( m_Order == -1 )
      {
      m_Order = m_pBB->Count() - 1;
      m_pBB->Clear();
      }
    else
      if( m_pBB->m_pFirst->m_pNext != nullptr && m_pBB->m_pFirst->m_pNext->m_pMember != m_pOwner->m_pWForMeasure )
        {
        m_pBB->Clear();
        if( m_Order < 2 ) m_Order++;
        }
  if( m_pBB->m_pFirst == nullptr )
    m_pBB->Append_Before( new EdChar( msPrime + m_Order, m_pOwner ) );
  EdPowr::PreCalc( P, S, A );
  }

QByteArray EdStrokeDeriv::Write()
  {
  QByteArray AA(m_pAA->Write());
  if (AA.endsWith(')') )
    {
    int iParCount = 0;
    for (int iChar = 0; iChar < AA.length(); iChar++)
      if (AA[iChar] == '(')
        iParCount++;
      else
        if (AA[iChar] == ')')
          iParCount--;
    if(iParCount != 0 ) AA = '(' + AA;
    }
  QByteArray Result = QByteArray( DerivName ) + "(TRUE,FALSE," + AA;
  for( int iVar = 0; iVar <= m_Order; iVar++ )
    Result += ",x";
  return Result + ')';
  }

QByteArray EdStrokeDeriv::SWrite()
  {
  QByteArray Result = "\\strkder{" + m_pAA->SWrite() + "}{";
  for( int iVar = 0; iVar < m_Order; iVar++ )
    Result += 'x';
  return Result + '}';
  }

EdStrokePartialDeriv::EdStrokePartialDeriv( XPInEdit *pOwn ) : EdIndx( pOwn ), m_pOrder( nullptr ), m_pSavedDeNom( nullptr )
  {}

void EdStrokePartialDeriv::PreCalc( TPoint P, QSize &S, int &A )
  {
  if( m_pBB->m_pFirst == nullptr )
    {
    if( m_pOrder != nullptr )
      {
      if( m_pSavedDeNom != nullptr ) m_pSavedDeNom->m_IsCell = true;
      delete m_pOrder;
      }
    m_pOrder = new EdChar( msPrime, m_pOwner );
    }
  else
    {
    int iOrder = 0;
    auto SetVarList = [&] ()
      {
      for( EdMemb *pMember = m_pBB->m_pFirst->m_pNext; pMember != nullptr; pMember = pMember->m_pNext, iOrder++ )
        {
        m_pBB->m_pCurr = pMember;
        m_pBB->Append_Before( new EdChar( ',', m_pOwner ) );
        }
      };
    if( m_pSavedDeNom == nullptr )
      {
      m_pSavedDeNom = new EdList( m_pBB );
      SetVarList();
      }
    else
      {
      EdMemb *pMember = m_pBB->m_pFirst;
      bool Comma = false;
      for( ; pMember != nullptr; pMember = pMember->m_pNext, Comma = !Comma )
        {
        EdChar *pChar = dynamic_cast< EdChar* >( pMember->m_pMember.data() );
        if( pChar == nullptr ) break;
        uchar C = pChar->c();
        if( Comma )
          if( C == ',' )
            continue;
          else
            break;
        if( !( C >= 'a' && C <= 'z' || C >= msAlpha && C <= msOmega ) ) break;
        iOrder++;
        }
      if( pMember == nullptr )
        {
        if( Comma )
          if( m_pSavedDeNom->m_IsCell )
            {
            delete m_pSavedDeNom;
            m_pSavedDeNom = new EdList( m_pBB );
            }
          else
            {
            int iOldCount = m_pSavedDeNom->Count();
            if( iOrder > iOldCount ) m_pSavedDeNom->Append_Before( m_pBB->m_pLast->m_pMember );
            if( iOrder < iOldCount ) m_pSavedDeNom->MemberDelete( m_pSavedDeNom->m_pLast );
            }
        }
      else
        {
        m_pBB->Clear();
        if( m_pSavedDeNom != nullptr )
          {
          m_pBB->Copy( m_pSavedDeNom );
          SetVarList();
          }
        }
      }
    m_pOrder = new EdChar( msPrime + min( iOrder - 1, 2 ), m_pOwner );
    }
  EdIndx::PreCalc( P, S, A );
  TPoint POrder( P );
  QSize SOrder;
  int  OderAxis;
  POrder.X += m_pAA->m_Size.width() + 1;
  m_pOrder->PreCalc( POrder, SOrder, OderAxis );
  }

void EdStrokePartialDeriv::Draw( TPoint P )
  {
  EdIndx::Draw( P );
  m_pOrder->Draw( m_pOrder->m_Start );
  }

QByteArray EdStrokePartialDeriv::Write()
  {
  return QByteArray( DerivName ) + "(TRUE,TRUE," + m_pAA->Write() + ',' + m_pBB->Write() + ')';;
  }

QByteArray EdStrokePartialDeriv::SWrite()
  {
  return "\\strkprtder{" + m_pAA->SWrite() + "}{" + m_pSavedDeNom->SWrite() + '}';
  }

EdDeriv::EdDeriv(XPInEdit *pOwn) : EdFrac(pOwn), m_pSavedDeNom(nullptr), m_Sign('d'), m_HighOrder(false)
  {}

void EdDeriv::RestoreDenom()
  {
  m_pBB->Clear();
  if( m_pBB->m_pFirst == nullptr )
    {
    m_pBB->Append_Before( new EdChar( m_Sign, m_pOwner ) );
    m_pBB->m_pFirst->SetProtected( true );
    }
  int iOrder = m_pSavedDeNom->Count();
  if( iOrder == 0 ) return;
  if( iOrder == 1 )
    {
    m_pBB->Append_Before( m_pSavedDeNom->m_pFirst->m_pMember );
    return;
    }
  m_HighOrder = true;
  if( m_Sign != 'd' ) return;
  EdPowr *pPower = new EdPowr( m_pOwner );
  pPower->m_pParent = this;
  EdMemb *pAppending = m_pBB->Append_Before( pPower );
  pPower->m_pAA->m_pMother = pAppending;
  pPower->m_pBB->m_pMother = pAppending;
  pPower->m_pAA->Append_Before( m_pSavedDeNom->m_pFirst->m_pMember );
  pPower->m_pBB->Append_Before( new EdChar( '0' + iOrder, m_pOwner ) );
  }

bool EdDeriv::TestVar( uchar C )
  {
  return C == msIdle || ((C >= 'a' && C <= 'z' || C >= msAlpha && C <= msOmega) && C != 'd');
  }

int EdDeriv::TestDenom()
  {
  int iCount = m_pBB->Count();
  if( iCount == 1 ) return drSignOnly;
  if (iCount == 3)
    {
    EdChar *pNewChar = dynamic_cast<EdChar*>(m_pBB->m_pLast->m_pPrev->m_pMember.data());
    if (!(IsConstEdType(EdChar, m_pBB->m_pLast->m_pMember.data())) || pNewChar == nullptr || !TestVar(pNewChar->c()) ) return drError;
    m_pBB->MemberDelete(m_pBB->m_pLast);
    dynamic_cast< EdChar* >(m_pSavedDeNom->m_pLast->m_pMember.data())->ResetChar(pNewChar->c());
    return 1;
    }
  if (iCount > 3) return drError;
  if( !m_pBB->m_pFirst->Protected() )
    {
    m_pBB->MemberDelete( m_pBB->m_pFirst );
    return drNoOperation;
    }
  EdChar *pChar = dynamic_cast< EdChar* >( m_pBB->m_pLast->m_pMember.data() );
  if( pChar != nullptr )
    {
    if( !TestVar( pChar->c() ) ) return drError;
    if( m_pSavedDeNom->m_pFirst == nullptr )
      {
      m_pSavedDeNom->Append_Before( pChar );
      return 1;
      }
    while( m_pSavedDeNom->m_pFirst != m_pSavedDeNom->m_pLast )
      m_pSavedDeNom->MemberDelete( m_pSavedDeNom->m_pLast );
    dynamic_cast< EdChar* >( m_pSavedDeNom->m_pLast->m_pMember.data() )->ResetChar( pChar->c() );
    return 1;
    }
  EdPowr *pPower = dynamic_cast< EdPowr* >( m_pBB->m_pLast->m_pMember.data() );
  if( pPower == nullptr ) return drError;
  if( pPower->m_pAA->m_pFirst == nullptr || pPower->m_pBB->m_pFirst == nullptr ) return drNoOperation;
  if( pPower->m_pAA->Count() != 1 ) return drError;
  if( pPower->m_pBB->Count() != 1 ) return drError;
  pChar = dynamic_cast< EdChar* >( pPower->m_pBB->m_pFirst->m_pMember.data() );
  if( pChar == nullptr ) return drError;
  if( !isdigit( pChar->c() ) ) return drError;
  if( pChar->c() < '2' ) return drError;
  int iOrder = pChar->c() - '0';
  pChar = dynamic_cast< EdChar* >( pPower->m_pAA->m_pFirst->m_pMember.data() );
  if( pChar == nullptr || !TestVar( pChar->c()) ) return drError;
  iCount = m_pSavedDeNom->Count();
  if( iCount > iOrder )
    for( ; iCount > iOrder; m_pSavedDeNom->MemberDelete( m_pSavedDeNom->m_pLast ), iCount-- );
  else
    for( ; iCount < iOrder; m_pSavedDeNom->Append_Before( pChar ), iCount++ );
  for( EdMemb *pMember = m_pSavedDeNom->m_pFirst; pMember != nullptr; pMember = pMember->m_pNext )
    dynamic_cast< EdChar* >( pMember->m_pMember.data() )->ResetChar( pChar->c() );
  return iCount;
  }

void EdDeriv::PreCalc( TPoint P, QSize &S, int &A )
  {
  auto AddDiff = [&] ( EdElm *pNewElm )
    {
    if( m_pAA->m_pFirst->Protected() )
      {
      m_pAA->m_pFirst->SetProtected( false );
      m_pAA->MemberDelete( m_pAA->m_pFirst );
      }
    m_pAA->m_pCurr = m_pAA->m_pFirst;
    m_pAA->Append_Before( pNewElm );
    m_pAA->m_pFirst->SetProtected( true );
    return m_pAA->m_pFirst;
    };

  auto ChangeNom = [&] ()
    {
    EdChar *pChar = nullptr;
    if( m_pAA->m_pFirst->Protected() )
      pChar = dynamic_cast< EdChar* >( m_pAA->m_pFirst->m_pMember.data() );
    int iOrder = max( 1, m_pSavedDeNom->Count() );
    if( iOrder == 1 )
      {
      if( pChar == nullptr )
        AddDiff( new EdChar( m_Sign, m_pOwner ) );
      return;
      }
    if( pChar == nullptr && m_pAA->m_pFirst->Protected() )
        {
        EdPowr *pPower = dynamic_cast< EdPowr* >( m_pAA->m_pFirst->m_pMember.data() );
        pChar = dynamic_cast< EdChar* >( pPower->m_pBB->m_pFirst->m_pMember.data() );
        pChar->ResetChar('0' + iOrder);
        return;
        }
      EdPowr *pPower = new EdPowr( m_pOwner );
      EdMemb *pAppending = AddDiff( pPower );
      pPower->m_pAA->m_pMother = pAppending;
      pPower->m_pBB->m_pMother = pAppending;
      pPower->m_pAA->Append_Before( new EdChar( m_Sign, m_pOwner ) );
      pPower->m_pAA->m_pFirst->SetProtected( true );
      pPower->m_pBB->Append_Before( new EdChar( '0' + iOrder, m_pOwner ) );
      pPower->m_pBB->m_pFirst->SetProtected( true );
    };
  if( m_pSavedDeNom == nullptr )
    {
    EdPowr *pPower = nullptr;
    if(m_pBB->m_pLast != nullptr) pPower = dynamic_cast<EdPowr*>(m_pBB->m_pLast->m_pMember.data());
    m_HighOrder = pPower != nullptr;
    if (m_HighOrder)
      m_pSavedDeNom = new EdList(m_pOwner);
    else
      m_pSavedDeNom = new EdList( m_pBB );
    if( m_pAA->m_pFirst == nullptr )
      {
      m_pAA->Append_Before( new EdChar( m_Sign, m_pOwner ) );
      m_pAA->m_pFirst->SetProtected( true );
      m_pAA->Append_Before(new EdChar(msIdle, m_pOwner));
      m_pBB->m_pCurr = m_pBB->m_pFirst;
      m_pBB->Append_Before( new EdChar( m_Sign, m_pOwner ) );
      m_pBB->m_pFirst->SetProtected( true );
      if(m_pBB->Count() == 1) m_pBB->Append_Before(new EdChar(msIdle, m_pOwner));
      if (m_HighOrder) pPower->m_pParent = this;
      EdList *pEl = m_pAA;
      m_pAA->MoveLeft(pEl);
      EdFrac::PreCalc( P, S, A );
      return;
      }
    RestoreDenom();
    ChangeNom();
    EdFrac::PreCalc( P, S, A );
    return;
    }

  if (m_pAA->m_pLast->Protected())
    {
    m_pAA->Append_Before(new EdChar(msIdle, m_pOwner));
    EdList *pEl = m_pAA;
    m_pAA->MoveLeft(pEl);
    EdFrac::PreCalc(P, S, A);
    return;
    }

  if (m_pBB->m_pLast->Protected())
    {
    m_pBB->Append_Before(new EdChar(msIdle, m_pOwner));
    EdList *pEl = m_pBB;
    m_pBB->MoveLeft(pEl);
    EdFrac::PreCalc(P, S, A);
    return;
    }

  if( !m_pAA->m_pFirst->Protected() )
    {
    m_pAA->MemberDelete( m_pAA->m_pFirst );
    EdFrac::PreCalc( P, S, A );
    return;
    }

  int iOrder = TestDenom();
  if( iOrder == drError )
    {
    RestoreDenom();
    EdFrac::PreCalc( P, S, A );
    return;
    }

  if( iOrder == drSignOnly ) m_pSavedDeNom->Clear();
  if( iOrder != drNoOperation ) ChangeNom();
  EdFrac::PreCalc( P, S, A );
  }

bool EdDeriv::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pAA->m_Start.X <= C.X && m_pAA->m_Start.X + m_pAA->m_Size.width() >= C.X &&
    m_pAA->m_Start.Y <= C.Y && m_pAA->m_Start.Y + m_pAA->m_Size.height() >= C.Y)
    {
    pSL = m_pAA;
    if (!pSL->SetCurrent(C, pSL, pSL->m_pCurr)) MoveInRight(pSL);
    pCr = pSL->m_pCurr;
    return true;
    }
  if (m_pBB->m_Start.X > C.X || m_pBB->m_Start.X + m_pBB->m_Size.width() < C.X || m_pBB->m_Start.Y > C.Y || m_pBB->m_Start.Y + m_pBB->m_Size.height() < C.Y) return false;
  pSL = m_pBB;
  m_pBB->m_pCurr = m_pBB->m_pFirst;
  do
    m_pBB->MoveRight(pSL);
  while (m_pBB->m_pCurr->Protected());
  EdPowr *pPower = dynamic_cast<EdPowr*>(m_pBB->m_pCurr->m_pMember.data());
  if (pPower != nullptr)
    {
    if (m_pBB->m_pCurr->m_pMember->SetCurrent(C, pSL, pCr)) return true;
    pSL = pPower->m_pAA;
    pSL->MoveLeft(pSL);
    }
  pCr = pSL->m_pCurr;
  return true;
  }

bool EdDeriv::MoveInRight(EdList* &pL)
  {
  pL = m_pAA;
  m_pAA->m_pCurr = m_pAA->m_pFirst;
  do
    m_pAA->MoveRight(pL);
    while (m_pAA->m_pCurr->Protected());
    return true;
  }

bool EdDeriv::MoveInLeft(EdList* &pL)
  {
  return MoveInRight(pL);
  }

bool EdDeriv::MoveToUp(EdList* &pL)
  {
  if( !EdFrac::MoveToUp(pL) ) return false;
  return MoveInRight(pL);
  }

bool EdDeriv::MoveToDown(EdList* &pL)
  {
  if (!EdFrac::MoveToDown(pL)) return false;
  m_pBB->m_pCurr = m_pBB->m_pFirst;
  if (m_pBB->m_pCurr == nullptr) return true;
  do
    m_pBB->MoveRight(pL);
  while (m_pBB->m_pCurr->Protected());
  if (m_HighOrder)m_pBB->MoveRight(pL);
  return true;
  }

bool EdDeriv::MoveToPrev(EdList* &pL)
  {
  if (IsConstEdType(EdPowr, pL->m_pMother->m_pMember.data()) ) pL = m_pBB;
  EdMemb *pIndCurr = pL->m_pMother;
  pL = pL->m_pMother->m_pMother;
  pL->m_pCurr = pIndCurr;
  return true;
  }

bool EdDeriv::MoveToNext(EdList* &pL)
  {
  if (IsConstEdType(EdPowr, pL->m_pMother->m_pMember.data())) pL = m_pBB;
  EdMemb *pIndCurr = pL->m_pMother->m_pNext;
  pL = pL->m_pMother->m_pMother;
  pL->m_pCurr = pIndCurr;
  return true;
  }

QByteArray EdDeriv::Write()
  {
  QByteArray Result = QByteArray( DerivName ) + "(FALSE," + ( m_Sign == 'd' ? "FALSE," : "TRUE," ) + m_pAA->Write();
  for( EdMemb *pMember = m_pSavedDeNom->m_pFirst; pMember != nullptr; pMember = pMember->m_pNext )
    Result += ',' + pMember->Write();
  return Result + ')';
  }

QByteArray EdDeriv::SWrite()
    {
  QByteArray Result = m_Sign == 'd' ? "\\der{" : "\\prtder{";
  for (EdMemb *pIndex = m_pAA->m_pFirst->m_pNext; pIndex != NULL; pIndex = pIndex->m_pNext)
    Result += pIndex->m_pMember->SWrite();
  return Result + "}{" + m_pSavedDeNom->SWrite() + '}';
    }

EdPartialDeriv::EdPartialDeriv( XPInEdit *pOwn ) : EdDeriv( pOwn )
  {
  m_Sign = msPartialDiff;
  }

EdMemb* EdDeriv::ReplaceParentMemb(EdList *pL, const PEdElm& pE)
  {
  static PEdElm Saved;
  EdChar *pNewChar = dynamic_cast<EdChar*>(pE.data());
  auto TestReplace = [&]()
    {
    bool Replace = false;
    if (pNewChar != nullptr)
      {
      EdChar *pOldChar = dynamic_cast<EdChar*>(pL->m_pCurr->m_pMember.data());
      if (isdigit(pOldChar->c()))
        Replace = isdigit(pNewChar->c());
      else
        Replace = TestVar(pNewChar->c());
      }
    if( Replace)
      pL->m_pCurr->m_pMember = pE;
    else
      Saved = pE;
    };

  if (pL->m_pCurr == nullptr)
    {
    pL->m_pCurr = pL->m_pLast;
    TestReplace();
    return pL->m_pCurr;
    }
  TestReplace();
  EdMemb  *Result = pL->m_pCurr;
  pL->m_pCurr = nullptr;
  return Result;
  }

void EdPartialDeriv::RestoreDenom()
  {
  EdDeriv::RestoreDenom();
  int iOrder = m_pSavedDeNom->Count();
  if( iOrder < 2 ) return;
  EdMemb *pMember = m_pSavedDeNom->m_pFirst;
  int iPartOrder = 1;
  EdChar *pD = new EdChar( m_Sign, m_pOwner );
  EdChar *pChar = dynamic_cast< EdChar* >( pMember->m_pMember.data() );
  bool NeedD = false;
  for( pMember = pMember->m_pNext; pMember != nullptr; pMember = pMember->m_pNext )
    {
    EdChar *pNewChar = dynamic_cast< EdChar* >( pMember->m_pMember.data() );
    if( pNewChar == nullptr ) break;
    if( pChar->c() == pNewChar->c() )
      {
      iPartOrder++;
      if( pMember->m_pNext != nullptr ) continue;
      }
    if( NeedD ) m_pBB->Append_Before( pD );
    NeedD = true;
    if( iPartOrder == 1 )
      m_pBB->Append_Before( pChar );
    else
      {
      EdPowr *pPower = new EdPowr( m_pOwner );
      pPower->m_pParent = this;
      EdMemb *pAppending = m_pBB->Append_Before( pPower );
      pPower->m_pAA->m_pMother = pAppending;
      pPower->m_pBB->m_pMother = pAppending;
      pPower->m_pAA->Append_Before( pChar );
      pPower->m_pBB->Append_Before( new EdChar( '0' + iPartOrder, m_pOwner ) );
      }
    if( pMember->m_pNext == nullptr )
      {
      if( pChar->c() == pNewChar->c() ) break;
      if( NeedD ) m_pBB->Append_Before( pD );
      m_pBB->Append_Before( pNewChar );
      break;
      }
    iPartOrder = 1;
    pChar = pNewChar;
    }
  }

int EdPartialDeriv::TestDenom()
  {
  if( !m_pBB->m_pFirst->Protected() )
    {
    m_pBB->MemberDelete( m_pBB->m_pFirst );
    return drNoOperation;
    }
  QByteArray Variables;
  bool MustSign = false;
  for( EdMemb *pMember = m_pBB->m_pFirst->m_pNext; pMember != nullptr; pMember = pMember->m_pNext, MustSign = !MustSign )
    {
    EdChar *pChar = dynamic_cast< EdChar* >( pMember->m_pMember.data() );
    if( pChar == nullptr )
      {
      if( MustSign ) return drError;
      EdPowr *pPower = dynamic_cast< EdPowr* >( pMember->m_pMember.data() );
      if (pPower->m_pAA->m_pFirst == nullptr && pPower->m_pBB->m_pFirst == nullptr)
        {
        pPower->m_pParent = this;
        if(pMember->m_pNext == nullptr) return drNoOperation;
        pChar = dynamic_cast< EdChar* >(pMember->m_pNext->m_pMember.data());
        if (pChar == nullptr) return drError;;
        if (pChar->c() != msPartialDiff) m_pBB->MemberDelete(pMember->m_pNext);
        return drNoOperation;
        }
      if( pPower == nullptr ) return drError;
      if( pPower->m_pAA->m_pFirst == nullptr || pPower->m_pBB->m_pFirst == nullptr ) return drNoOperation;
      if( pPower->m_pAA->Count() != 1 ) return drError;
      if( pPower->m_pBB->Count() != 1 ) return drError;
      pChar = dynamic_cast< EdChar* >( pPower->m_pBB->m_pFirst->m_pMember.data() );
      if( pChar == nullptr ) return drError;
      if( !isdigit( pChar->c() ) ) return drError;
      if( pChar->c() < '2' ) return drError;
      EdChar *pVar = dynamic_cast< EdChar* >( pPower->m_pAA->m_pFirst->m_pMember.data() );
      if( pVar == nullptr ) return drError;
      uchar C = pVar->c();
      if( !TestVar(C) ) return drError;
      Variables.append( pChar->c() - '0', C );
      continue;
      }
    uchar C = pChar->c();
    if( MustSign )
      {
      if( C == 'd' )
        pChar->ResetChar( msPartialDiff );
      else
        if (pChar->c() != msPartialDiff)
          {
          if (!TestVar(pChar->c())) return drError;
          MustSign = false;
          EdMemb *pForDelete = pMember;
          pMember = pMember->m_pPrev;
          m_pBB->MemberDelete(pForDelete);
          continue;
          }
      if( pMember->m_pNext == nullptr ) return drNoOperation;
      continue;
      }
    if( !TestVar( C ) ) return drError;
    Variables.append( 1, C );
    }
  int iOrder = Variables.length();
  if( iOrder == 0 ) return drSignOnly;
  int iCount = m_pSavedDeNom->Count();
  if( iCount > iOrder )
    for( ; iCount > iOrder; m_pSavedDeNom->MemberDelete( m_pSavedDeNom->m_pLast ), iCount-- );
  else
    for( ; iCount < iOrder; m_pSavedDeNom->Append_Before( new EdChar('x', m_pOwner ) ), iCount++ );
  int iChar = 0;
  for( EdMemb *pMember = m_pSavedDeNom->m_pFirst; pMember != nullptr; pMember = pMember->m_pNext, iChar++ )
    dynamic_cast< EdChar* >( pMember->m_pMember.data() )->ResetChar( Variables[iChar] );
  return iCount;
  }

bool EdPartialDeriv::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pBB->m_Start.X > C.X || m_pBB->m_Start.X + m_pBB->m_Size.width() < C.X || m_pBB->m_Start.Y > C.Y || m_pBB->m_Start.Y + m_pBB->m_Size.height() < C.Y)
    return EdDeriv::SetCurrent(C, pSL, pCr);
  pSL = m_pBB;
  for (EdMemb *pIndex = m_pBB->m_pFirst; pIndex != NULL; pIndex = pIndex->m_pNext)
    if (pIndex->m_Start.X <= C.X  &&
      pIndex->m_Start.X + pIndex->m_Size.width() >= C.X &&
      pIndex->m_Start.Y <= C.Y &&
      pIndex->m_Start.Y + pIndex->m_Size.height() >= C.Y)
      {
      EdChar *pChar = dynamic_cast<EdChar*>(pIndex->m_pMember.data());
      if (pChar != nullptr && pChar->c() == msPartialDiff ) pIndex = pIndex->m_pNext;
      m_pBB->m_pCurr = pIndex;
      EdPowr *pPower = dynamic_cast<EdPowr*>(pIndex->m_pMember.data());
      if (pPower == nullptr) return true;
      return pIndex->m_pMember->SetCurrent(C, pSL, m_pBB->m_pCurr);
      }
  return false;
  }

EdMemb* EdPartialDeriv::ReplaceParentMemb(EdList *pL, const PEdElm& pE)
  {
  return EdDeriv::ReplaceParentMemb(pL, pE);
  }

bool EdPartialDeriv::MoveToNext(EdList* &pL)
  {
//  if (IsConstEdType(EdPowr, pL->m_pMother->m_pMember.data())) pL = m_pBB;
  EdMemb *pIndCurr = pL->m_pMother->m_pNext;
  pL = pL->m_pMother->m_pMother;
  pL->m_pCurr = pIndCurr;
  return true;
  }

bool EdPerCount::SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  pSL = m_pCC;
  if (pSL->SetCurrent (C, pSL, pSL->m_pCurr))
    {
    pCr = pSL->m_pCurr;
    }
  else
    {
    if ((m_pCC->m_pFirst == NULL) &&
      (m_pCC->m_Start.X <= C.X) &&
      (m_pCC->m_Start.X + m_pCC->m_Size.width() >= C.X) &&
      (m_pCC->m_Start.Y <= C.Y) &&
      (m_pCC->m_Start.Y + m_pCC->m_Size.width() >= C.Y))
      {
      pCr = NULL;
      }
    }
  return true;
  }

void EdPerCount::PreCalc (TPoint P, QSize &S, int &A)
  {
  TPoint P1, P2;
  QSize S1, S2;

  m_Start = P;

  P1 = m_Start;
  m_pAA->PreCalc (P1, S1, m_pAA->m_Axis);

  P2.X = m_pAA->m_Start.X + S1.width();
  P2.Y = P1.Y;
  m_pCC->PreCalc (P2, S2, m_pCC->m_Axis);

  m_Size.setWidth (S1.rwidth() + S2.rwidth() + m_pOwner->CharWidth('-'));
  m_Size.setHeight (m_pCC->m_Start.Y + m_pCC->m_Size.height() - m_Start.Y);

  if (m_pCC->m_Axis > m_pAA->m_Axis)
    {
    m_pAA->m_Start.Y = m_Start.Y + m_pCC->m_Axis - m_pAA->m_Axis;
    m_pAA->PreCalc (m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis);
    }
  P2 = m_pCC->m_Start;
  m_pB1->PreCalc (P2, m_pB1->m_Size, m_pB1->m_Axis);
  m_pB1->m_pFirst->m_pMember->m_Size.setHeight(m_pCC->m_Size.height());
  m_Size.setWidth(m_Size.width() + m_pOwner->CharWidth('('));

  m_pCC->m_Start.X = m_pCC->m_Start.X + m_pOwner->CharWidth('(');
  m_pCC->PreCalc (m_pCC->m_Start, m_pCC->m_Size, m_pCC->m_Axis);

  P2.Y = m_pCC->m_Start.Y;
  P2.X = m_pCC->m_Start.X + m_pCC->m_Size.width();
  m_pB2->PreCalc (P2, m_pB2->m_Size, m_pB2->m_Axis);
  m_pB2->m_pFirst->m_pMember->m_Size.setHeight(m_pCC->m_Size.height());
  m_Size.setWidth(m_Size.width() + m_pOwner->CharWidth(')'));

  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;

  S = m_Size;
  A = m_Axis;
  }

void EdPerCount::Draw (TPoint P)
  {
  if (m_Start.X != P.X || m_Start.Y != P.Y)
    {
    m_Start = P;
    PreCalc (m_Start, m_Size, m_Axis);
    }
  SelectRect();
  m_pAA->Draw (m_pAA->m_Start);
  m_pB1->Draw (m_pB1->m_Start);
  m_pCC->Draw (m_pCC->m_Start);
  m_pB2->Draw (m_pB2->m_Start);
  }

void EdPerCount::SelectFragment (QRect &FRect)
  {
  m_pAA->SelectFragment(FRect);
  m_pB1->SelectFragment(FRect);
  m_pCC->SelectFragment(FRect);
  m_pB2->SelectFragment(FRect);
  m_Selected = (m_pAA->m_Selected && m_pB1->m_Selected && m_pCC->m_Selected && m_pB2->m_Selected);
  }

void EdPerCount::ClearSelection ()
  {
  m_pAA->ClearSelection();
  m_pB1->ClearSelection();
  m_pCC->ClearSelection();
  m_pB2->ClearSelection();
  m_Selected = false;
  }

QByteArray EdPerCount::GetFragment()
  {
  if (m_Selected) return Write() + '&';
  return m_pAA->GetFragment() + m_pB1->GetFragment() + m_pCC->GetFragment() + m_pB2->GetFragment();
  }

QByteArray EdPerCount::Write()
  {
  return  "PerCount" + '('+ m_pCC->Write() + ")";
  }

bool EdPerCount::MoveInRight (EdList* &pL)
  {
  pL=m_pCC;
  pL->m_pCurr = m_pCC->m_pFirst;
  return true;
  }

bool EdPerCount::MoveInLeft (EdList* &pL)
  {
  pL=m_pCC;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdPerCount::MoveToNext (EdList* &pL)
  {
  return false;
  }

bool EdPerCount::MoveToPrev (EdList* &pL)
  {
  return false;
  }

bool EdPerCount::MoveToUp (EdList* &pL)
  {
  return false;
  }

bool EdPerCount::MoveToDown (EdList* &pL)
  {
  return false;
  }

QByteArray EdPerCount::SWrite()
    {
    return "\\percount{" + m_pCC->SWrite() + '}';
    }

bool EdBCoeff::SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pBB->SetCurrent(C, pSL, pCr) || m_pCC->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdBCoeff::PreCalc (TPoint P, QSize &S, int &A)
  {
  TPoint P1, P2, P3;
  QSize S1, S2, S3;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;

  m_Start = P;

  P1 = m_Start;
  m_pAA->PreCalc (P1, S1, m_pAA->m_Axis);

  P2.X = m_pAA->m_Start.X + S1.width();
  P2.Y = P1.Y;
  m_pOwner->SetPowerSize (+1, BaseLevel);
  m_pBB->PreCalc (P2, S2, m_pBB->m_Axis);

  if (S1.height() <= S2.height())
    {
    m_pAA->m_Start.Y += S2.height() - (S1.height()/2);
    }

  if (S1.height() > S2.height())
    {
    m_pAA->m_Start.Y += S1.height()/2;
    }

  if (m_pAA->m_Start.Y != P1.Y)
    {
    m_pAA->PreCalc (m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis);
    P1 = m_pAA->m_Start;
    S1 = m_pAA->m_Size;
    }

  m_pOwner->SetPowerSize (-1, BaseLevel);

  P3.X = P2.X;
  P3.Y = P1.Y;
  m_pOwner->SetPowerSize (-1, BaseLevel);
  m_pCC->PreCalc (P3, S3, m_pCC->m_Axis);

  if (S1.height() <= S3.height())
    {
    m_pCC->m_Start.Y += S1.height()/2;
    }

  if (S1.height() > S3.height())
    {
    m_pCC->m_Start.Y += S1.height() - (S3.height()/2);
    }

  if (m_pCC->m_Start.Y != P3.Y)
    {
    m_pCC->PreCalc (m_pCC->m_Start, m_pCC->m_Size, m_pCC->m_Axis);
    }

  m_pOwner->SetPowerSize (+1, BaseLevel);

  m_Size.setWidth(S1.width() + max (S2.width(), S3.width()) + m_pOwner->CharWidth('-')+2);//CharWidth('-') ????
  m_Size.setHeight(m_pAA->m_Start.Y + m_pAA->m_Size.height() - m_Start.Y+20);
  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;

  S = m_Size;
  A = m_Axis;
  }

void EdBCoeff::Draw (TPoint P)
  {
  if ((m_Start.X != P.X) || (m_Start.Y != P.Y))
    {
    m_Start = P;
    PreCalc (m_Start, m_Size, m_Axis);
    }
  SelectRect();
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pAA->Draw (m_pAA->m_Start);
  m_pOwner->SetPowerSize(-1, BaseLevel);
  m_pBB->Draw(m_pBB->m_Start);
  m_pCC->Draw(m_pCC->m_Start);
  m_pOwner->SetPowerSize(+1, BaseLevel);
  }

QByteArray EdBCoeff::GetFragment()
  {
  if (m_pAA->m_Selected && (m_pBB->m_Selected || m_pCC->m_Selected)) return Write() + '&';
  return m_pAA->GetFragment() + m_pBB->GetFragment() + m_pCC->GetFragment();
  }

QByteArray EdBCoeff::Write()
  {
  if (m_pAA->Write() == "C")
    return "BinomCoeff(" + m_pBB->Write() + ',' + m_pCC->Write() + ')';
  return "ACoeff(" + m_pBB->Write() + ',' + m_pCC->Write() + ')';
  }

bool EdBCoeff::MoveInRight (EdList* &pL)
  {
  pL = m_pCC;
  pL->m_pCurr = m_pCC->m_pFirst;
  return true;
  }

bool EdBCoeff::MoveInLeft (EdList* &pL)
  {
  pL = m_pCC;
  pL->m_pCurr = NULL;
  return true;
  }
bool EdBCoeff::MoveToNext (EdList* &pL)
  {
  return false;
  }
bool EdBCoeff::MoveToPrev (EdList* &pL)
  {
  return false;
  }
bool EdBCoeff::MoveToUp (EdList* &pL)
  {
  if ((pL == m_pAA) || (pL == m_pCC))
    {
    pL = m_pBB;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }
bool EdBCoeff::MoveToDown (EdList* &pL)
  {
  if ((pL == m_pBB) || (pL == m_pAA))
    {
    pL = m_pCC;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

QByteArray EdBCoeff::SWrite()
    {
    if (m_pAA->SWrite() == "C")
        return "\\bcoeff{" + m_pBB->SWrite() + "}{" + m_pCC->SWrite() + '}';
  return "\\acoeff{" + m_pBB->SWrite() + "}{" + m_pCC->SWrite() + '}';
    }

void EdFunc::PreCalc (TPoint P, QSize &S, int &A)
  {
  TPoint P1, P2, P3, P4;
  QSize S1, S2, S3, S4;

  m_Start = P;

  P1 = m_Start;
  m_pAA->PreCalc (P1, S1, m_pAA->m_Axis);

  P2.X = m_pAA->m_Start.X + S1.width();
  P2.Y = P1.Y;
  m_pB1->PreCalc (P2, S2, m_pB1->m_Axis);

  P3.X = m_pB1->m_Start.X + S2.width();
  P3.Y = P2.Y;
  m_pBB->PreCalc (P3, S3, m_pBB->m_Axis);

  P4.X = m_pBB->m_Start.X + S3.width();
  P4.Y = P3.Y;
  m_pB2->PreCalc (P4, S4, m_pB2->m_Axis);

  m_Size.setWidth (S1.width() + S2.width() + S3.width() + S4.width());
  if (m_pAA->m_Size.height() >= m_pBB->m_Size.height())
    {
    m_Size.setHeight (m_pAA->m_Size.height());
    m_Axis = m_pAA->m_Axis;
    }
  else
    {
    m_Size.setHeight(m_pBB->m_Size.height());
    m_Axis = m_pBB->m_Axis;
    m_pB1->m_Size.setHeight (m_Size.height());
    if (m_pB1->m_pFirst != NULL)
      {
      m_pB1->m_pFirst->m_Size.setHeight (m_Size.height());
      m_pB1->m_pFirst->m_pMember->m_Size.setHeight (m_Size.height());
      }
    m_pB2->m_Size.setHeight (m_Size.height());
    if (m_pB2->m_pFirst != NULL)
      {
      m_pB2->m_pFirst->m_Size.setHeight (m_Size.height());
      m_pB2->m_pFirst->m_pMember->m_Size.setHeight (m_Size.height());
      }
    P1.Y += m_Axis - m_pAA->m_Axis;
    m_pAA->PreCalc( P1, m_pAA->m_Size, m_pAA->m_Axis );
    }
  S = m_Size;
  A = m_Axis;
  }

void EdFunc::Draw (TPoint P)
  {
   if ((m_Start.X != P.X) || (m_Start.Y != P.Y))
    {
    m_Start = P;
    PreCalc (m_Start, m_Size, m_Axis);
    }
   SelectRect();
   m_pAA->Draw( m_pAA->m_Start );
   m_pB1->Draw (m_pB1->m_Start);
   m_pBB->Draw (m_pBB->m_Start);
   m_pB2->Draw (m_pB2->m_Start);
  }

QByteArray EdFunc::Write()
  {
  return "func(" + m_pAA->Write() +  ',' + m_pBB->Write() + ')' ;
  }

bool EdFunc::MoveInRight (EdList* &pL)
  {
  pL = m_pBB;
  pL->m_pCurr = m_pBB->m_pFirst;
  return true;
  }

bool EdFunc::MoveInLeft (EdList* &pL)
  {
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdFunc::MoveToNext (EdList* &pL)
  {
  if (pL == m_pAA)
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdFunc::MoveToPrev (EdList* &pL)
  {
  if (pL == m_pBB)
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

void EdFunc::ClearSelection ()
  {
  EdTwo::ClearSelection();
  m_pB1->ClearSelection();
  m_pB2->ClearSelection();
  m_Selected = false;
  }

void EdFunc::SelectFragment (QRect &FRect)
  {
  EdTwo::SelectFragment(FRect);
  m_pB1->SelectFragment(FRect);
  m_pB2->SelectFragment(FRect);
  m_Selected = m_pAA->m_Selected && m_pBB->m_Selected && m_pB2->m_Selected;
  }

QByteArray EdFunc::GetFragment ()
  {
  if (m_Selected) return Write() + '&';
  return m_pAA->GetFragment() + m_pBB->GetFragment() + m_pB1->GetFragment() + m_pB2->GetFragment();
  }

EdSyst::EdSyst( XPInEdit *pOwn ) : EdAbs( pOwn )
  {
  QFont SystFont( pOwn->m_EditSets.m_MainFont );
  SystFont.setPointSize( SystFont.pointSize() * 1.5 );
  SystFont.setWeight( QFont::Thin );
  m_pSign = new EdBigChar( msSystem, irNormal, "Black", pOwn, SystFont, -1 );
  }

void EdSyst::PreCalc ( TPoint P, QSize &S, int &A )
  {
  QSize S1, SSyst;
  m_Start = P;
  m_pSign->PreCalc( P, SSyst, m_Axis );
  m_pAA->PreCalc( P, S1, m_pAA->m_Axis );
  int SignHeight = S1.height() * 1.25;
  if( SignHeight - SSyst.height() > 2 )
    m_pSign->m_Size.setHeight( SignHeight );
  else
    SignHeight = SSyst.height();
  m_Size.setHeight( SignHeight );
  m_Axis = m_Size.height() / 2;
  m_pAA->PreCalc( TPoint( P.X + SSyst.width() + 2, m_Start.Y + ( m_Size.height() - S1.height() ) / 2 ), S1, m_pAA->m_Axis );
  m_Size.setWidth( SSyst.width() + S1.width() + 2);
  S = m_Size;
  A = m_Axis;
  }

void EdSyst::Draw ( TPoint P )
  {
  if ((m_Start.X != P.X) || (m_Start.Y != P.Y))
    {
    m_Start=P;
    PreCalc(m_Start, m_Size, m_Axis);
    }
  SelectRect();
  m_pSign->Draw( P );
  m_pAA->Draw( m_pAA->m_Start );
  }

QByteArray EdSyst::Write()
  {
  return '(' + QByteArray(SystName) + '(' + m_pAA->Write() + "))";
  }

QByteArray EdSyst::SWrite()
    {
    return "\\system{" + m_pAA->SWrite() + '}';
    }

void EdIndx::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint P1, P2;
  QSize S1, S2;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;

  m_Start = P;

  P1 = m_Start;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );

  m_pOwner->SetPowerSize( -1, BaseLevel );
  P2.X = m_pAA->m_Start.X + S1.width();
  P2.Y = P1.Y;
  m_pBB->PreCalc( P2, S2, m_pBB->m_Axis );

  if( S1.height() <= S2.height() )
    {
    m_pBB->m_Start.Y += S1.height() / 2;
    }
  if( S1.height() > S2.height() )
    {
    m_pBB->m_Start.Y += S1.height() - ( S2.height() / 2 );
    }
  if( m_pBB->m_Start.Y != P2.Y )
    {
    m_pBB->PreCalc( m_pBB->m_Start, m_pBB->m_Size, m_pBB->m_Axis );
    }
  m_pOwner->SetPowerSize( +1, BaseLevel );

  m_Size.setWidth( S1.width() + S2.width() + m_pOwner->CharWidth( '-' ) / 3 );
  m_Size.setHeight( m_pBB->m_Start.Y + m_pBB->m_Size.height() - m_Start.Y );
  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;

  S = m_Size;
  A = m_Axis;
  }

void EdIndx::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pAA->Draw( m_pAA->m_Start );
  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pOwner->m_pCanvas->setPen( m_pOwner->m_EditSets.m_SignColor );
  m_pBB->Draw( m_pBB->m_Start );
  m_pOwner->m_pCanvas->setPen( m_pOwner->m_EditSets.m_CursColor );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  }

QByteArray EdIndx::Write()
  {
  return m_pAA->Write() + '['  + m_pBB->Write() +  ']' ;
  }

bool EdIndx::MoveInRight (EdList* &pL)
  {
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdIndx::MoveInLeft (EdList* &pL)
  {
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdIndx::MoveToNext (EdList* &pL)
  {
  if (pL == m_pAA)
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdIndx::MoveToPrev (EdList* &pL)
  {
  if (pL == m_pBB)
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdIndx::MoveToUp (EdList* &pL)
  {
  if (pL == m_pBB)
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdIndx::MoveToDown (EdList* &pL)
  {
  if (pL == m_pAA)
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

QByteArray EdIndx::SWrite()
    {
    return "\\index{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + '}';
    }

bool EdMeas::CinMass ()
  {
  return m_C == ( char ) msDegree || m_C == ( char ) msMinute || m_C == '%';
  }

void EdMeas::CheckDegMin()
  {
  EdMemb *pIndex;
  if (m_pBB->m_pFirst != NULL)
    m_DegMin = true;
  else
    m_DegMin = false;
  pIndex = m_pBB->m_pFirst;
  while ( pIndex && m_DegMin )
    {
    m_DegMin = pIndex->ElChar ( m_C ) && CinMass ();
    pIndex = pIndex->m_pNext;
    }
  }

void EdMeas::PreCalc (TPoint P, QSize &S, int &A)
  {
  TPoint P1, P2, PO, PC;
  QSize S1, S2, SO, SC;

  m_Start = P;
  CheckDegMin ();
  P1 = m_Start;
  m_pAA->PreCalc (P1, S1, m_pAA->m_Axis);

  if (!m_DegMin)
    {
    PO.X = m_pAA->m_Start.X + S1.width();
    PO.Y = P1.Y;
    m_pOB->PreCalc (PO, SO, m_pOB->m_Axis);

    P2.X = m_pOB->m_Start.X + SO.width();
    }
  else
    {
    P2.X = m_pAA->m_Start.X +S1.width();
    }
  P2.Y = P1.Y;
  m_pBB->PreCalc (P2, S2, m_pBB->m_Axis);

  if (!m_DegMin)
    {
    PC.X = m_pBB->m_Start.X + S2.width();
    PC.Y = P1.Y;
    m_pCB->PreCalc (PC, SC, m_pCB->m_Axis);
    }
  m_Axis = max(m_pAA->m_Axis, m_pBB->m_Axis);
  if (!m_DegMin)
    {
    m_Axis = max (m_Axis, m_pOB->m_Axis);
    m_Axis = max (m_Axis, m_pCB->m_Axis);
    }
  if (m_pAA->m_Axis < m_Axis)
    {
    P1.Y += m_Axis - m_pAA->m_Axis;
    m_pAA->PreCalc(P1, S1, m_pAA->m_Axis);
    }
  if (m_pBB->m_Axis < m_Axis)
    {
    P2.Y += m_Axis - m_pBB->m_Axis;
    m_pBB->PreCalc (P2, S2, m_pBB->m_Axis);
    }
  if (!m_DegMin)
    {
    if (m_pOB->m_Axis < m_Axis)
      {
      PO.Y += m_Axis - m_pOB->m_Axis;
      m_pOB->PreCalc (PO, SO, m_pOB->m_Axis);
      }
    if (m_pCB->m_Axis < m_Axis)
      {
      PC.Y += m_Axis - m_pCB->m_Axis;
      m_pCB->PreCalc (PC, SC, m_pCB->m_Axis);
      }
    }
  if (!m_DegMin)
    {
    m_Size.setWidth (m_pCB->m_Start.X + m_pCB->m_Size.width() - m_Start.X +3);
    }
  else
    {
    m_Size.setWidth (m_pBB->m_Start.X + m_pBB->m_Size.width() - m_Start.X + 3);
    }
  m_Size.setHeight (max ( m_pAA->m_Start.Y + m_pAA->m_Size.height(), m_pBB->m_Start.Y + m_pBB->m_Size.height() ) );
  if (!m_DegMin)
    {
    m_Size.setHeight (max (max (m_pAA->m_Start.Y + m_pAA->m_Size.height(), m_pBB->m_Start.Y + m_pBB->m_Size.height()), m_pOB->m_Start.Y + m_pOB->m_Size.height()));
    m_Size.setHeight (max (max (m_pAA->m_Start.Y + m_pAA->m_Size.height(), m_pBB->m_Start.Y + m_pBB->m_Size.height()), m_pCB->m_Start.Y + m_pCB->m_Size.height()));
    }
  m_Size.setHeight (m_Size.height() - m_Start.Y + 1);

  S = m_Size;
  A = m_Axis;
  }

void EdMeas::Draw (TPoint P)
  {
  int x1, x2, y1, y2;
  EdElm *pOldElement;

  if (m_Start.X != P.X || m_Start.Y != P.Y)
    {
    m_Start = P;
    PreCalc (m_Start, m_Size, m_Axis);
    }
  SelectRect();
  CheckDegMin ();

  m_pAA->Draw (m_pAA->m_Start);
  m_pBB->Draw (m_pBB->m_Start);
  if (m_DegMin) return;

  QPen Pen = m_pOwner->m_pCanvas->pen();
  //Pen.setColor( m_pOwner->m_EditSets.m_SignColor );
  m_pOwner->m_pCanvas->setPen( Pen );

  x1 = m_pBB->m_Start.X - 5;
  y1 = m_pBB->m_Start.Y;
  x2 = m_pBB->m_Start.X + m_pBB->m_Size.width() + 5;
  y2 = m_pBB->m_Start.Y + m_pBB->m_Size.height();
  m_pOwner->Line (x1, y1, x1, y2);
  m_pOwner->Line (x1, y1, x1+5, y1);
  m_pOwner->Line (x1, y2, x1+5, y2);
  m_pOwner->Line (x2, y1, x2, y2);
  m_pOwner->Line (x2, y1, x2-5, y1);
  m_pOwner->Line (x2, y2, x2-5, y2);
  }

QByteArray EdMeas::Write()
  {
  QByteArray M(m_pBB->Write());
  if ( M == "`?'" || M == "`?'" )
    return m_pAA->Write() + m_pBB->Write();
  QByteArray A = m_pAA->Write();
  int EqPos = A.indexOf('=');
  if(EqPos == -1 )
    return  "((" + A + ")`" + m_pBB->Write() + "')";
  QByteArray Result = '(' + A.left(EqPos + 1) +  "((" + A.mid(EqPos + 2) + ")`" + m_pBB->Write() + "')";
  return Result;
  }

bool EdMeas::MoveInRight (EdList* &pL)
  {
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdMeas::MoveInLeft (EdList* &pL)
  {
  pL = m_pBB;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdMeas::MoveToNext( EdList* &pL )
  {
  if( pL == m_pAA )
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdMeas::MoveToPrev (EdList* &pL)
  {
  if (pL == m_pBB)
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

QByteArray EdMeas::SWrite()
  {
  if ( ( m_pBB->SWrite() == "`?'" ) || ( m_pBB->SWrite() == "`?'" ) )//2.11.2015
    return m_pAA->SWrite() + m_pBB->SWrite();
  return "\\units{" + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + '}';//2.11.2015
  }

void EdLog::ClearSelection ()
  {
  m_pAA->ClearSelection();
  m_pBB->ClearSelection();
  m_pB1->ClearSelection();
  m_pB2->ClearSelection();
  m_pCC->ClearSelection();
  m_Selected = false;
  }

void EdLog::SelectFragment (QRect &FRect)
  {
  m_Selected = m_pBB->InRect (FRect) && (m_pAA->InRect (FRect) || m_pB1->InRect (FRect));
  if (m_Selected)
    {
    m_pAA->m_Selected;
    m_pBB->m_Selected;
    m_pB1->m_Selected;
    m_pB2->m_Selected;
    m_pCC->m_Selected;
    return;
    }
  m_pAA->SelectFragment (FRect);
  m_pBB->SelectFragment (FRect);
  m_pB1->SelectFragment (FRect);
  m_pB2->SelectFragment (FRect);
  m_pCC->SelectFragment (FRect);
  }

QByteArray EdLog::GetFragment()
  {
  if (m_Selected) return Write() + '&';
  return m_pAA->GetFragment() + m_pBB->GetFragment () + m_pB1->GetFragment() + m_pCC->GetFragment() + m_pB2->GetFragment();
  }

bool EdLog::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if ( m_pBB->SetCurrent(C, pSL, pCr) || m_pCC->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdLog::PreCalc( TPoint P, QSize &S, int &A )
  {
  TPoint P1, P2, P3;
  QSize S1, S2, S3;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  unsigned short int ax;

  m_Start = P;

  P1 = m_Start;
  m_pAA->PreCalc( P1, S1, m_pAA->m_Axis );

  m_pOwner->SetPowerSize( -1, BaseLevel );
  P2.X = m_pAA->m_Start.X + S1.width();
  P2.Y = P1.Y;
  m_pBB->PreCalc( P2, S2, m_pBB->m_Axis );

  if( S1.height() <= S2.height() )
    {
    m_pBB->m_Start.Y += S1.height() / 2;
    }
  if( S1.height() > S2.height() )
    {
    m_pBB->m_Start.Y += S1.height() - S1.height() / 2;
    }

  if( m_pBB->m_Start.Y != P2.Y )
    {
    m_pBB->PreCalc( m_pBB->m_Start, m_pBB->m_Size, m_pBB->m_Axis );
    }

  P3.X = m_pBB->m_Start.X + m_pBB->m_Size.width();
  P3.Y = m_Start.Y;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pCC->PreCalc( P3, S3, m_pCC->m_Axis );

  m_Size.setWidth( S1.width() + S2.width() + S3.width() + m_pOwner->CharWidth( '-' ) / 3 );
  m_Size.setHeight( m_pBB->m_Start.Y + m_pBB->m_Size.height() - m_Start.Y );

  ax = max( m_pBB->m_Axis, m_pCC->m_Axis );
  if( ax > m_pAA->m_Axis )
    {
    m_pAA->m_Start.Y = m_Start.Y + ax - m_pAA->m_Axis;
    m_pAA->PreCalc( m_pAA->m_Start, m_pAA->m_Size, m_pAA->m_Axis );
    if( m_pCC->m_Axis > m_pBB->m_Axis )
      {
      m_pOwner->SetPowerSize( -1, BaseLevel );
      m_pBB->m_Start.Y += ax - m_pAA->m_Axis;
      m_pBB->PreCalc( m_pBB->m_Start, m_pBB->m_Size, m_pBB->m_Axis );
      m_pOwner->SetPowerSize( +1, BaseLevel );
      }
    else
      {
      m_pCC->m_Start.Y += ax - m_pAA->m_Axis;
      m_pCC->PreCalc( m_pCC->m_Start, m_pCC->m_Size, m_pCC->m_Axis );
      }
    m_Size.setHeight( max( m_pBB->m_Start.Y + m_pBB->m_Size.height(), m_pCC->m_Start.Y + m_pCC->m_Size.height() ) - m_Start.Y );
    }
  P2 = m_pCC->m_Start;
  m_pB1->PreCalc( P2, m_pB1->m_Size, m_pB1->m_Axis );
  m_pB1->m_pFirst->m_pMember->m_Size.setHeight( m_pCC->m_Size.height() );
  m_Size.setWidth( m_Size.width() + m_pOwner->CharWidth( '(' ) );

  m_pCC->m_Start.X = m_pCC->m_Start.X + m_pOwner->CharWidth( '(' );
  m_pCC->PreCalc( m_pCC->m_Start, m_pCC->m_Size, m_pCC->m_Axis );

  P2.Y = m_pCC->m_Start.Y;
  P2.X = m_pCC->m_Start.X + m_pCC->m_Size.width();
  m_pB2->PreCalc( P2, m_pB2->m_Size, m_pB2->m_Axis );
  m_pB2->m_pFirst->m_pMember->m_Size.setHeight( m_pCC->m_Size.height() );
  m_Size.setWidth( m_Size.width() + m_pOwner->CharWidth( ')' ) );

  m_Axis = m_pAA->m_Start.Y - m_Start.Y + m_pAA->m_Axis;

  S = m_Size;
  A = m_Axis;
  }

void EdLog::Draw( TPoint P )
  {
  bool BaseLevel;
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pAA->Draw( m_pAA->m_Start );

  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pBB->Draw( m_pBB->m_Start );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pB1->m_pFirst->m_pMember->Draw( m_pB1->m_Start );
  m_pCC->Draw( m_pCC->m_Start );
  m_pB2->m_pFirst->m_pMember->Draw( m_pB2->m_Start );
  }

QByteArray EdLog::Write()
  {
  return m_pAA->Write() + '(' + m_pBB->Write() + ',' + m_pCC->Write() + ')';
  }

bool EdLog::MoveInRight( EdList* &pL )
  {
  pL = m_pCC;
  pL->m_pCurr = m_pCC->m_pFirst;
  return true;
  }

bool EdLog::MoveInLeft( EdList* &pL )
  {
  pL = m_pCC;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdLog::MoveToNext( EdList* &pL )
  {
  if( pL == m_pAA )
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pFirst;
    return true;
    }
  if( pL == m_pBB )
    {
    pL = m_pCC;
    pL->m_pCurr = m_pCC->m_pFirst;
    return true;
    }
  return false;
  }

bool EdLog::MoveToPrev( EdList* &pL )
  {
  if( pL == m_pBB )
    {
    pL->m_pCurr = m_pAA->m_pMother;
    return true;
    }
  if( pL == m_pCC )
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pLast;
    return true;
    }
  return false;
  }

bool EdLog::MoveToDown( EdList* &pL )
  {
  if( pL == m_pCC )
    {
    pL = m_pBB;
    pL->m_pCurr = m_pBB->m_pLast;
    return true;
    }
  return false;
  }

bool EdLog::MoveToUp( EdList* &pL )
  {
  if( pL == m_pBB )
    {
    pL = m_pCC;
    pL->m_pCurr = m_pCC->m_pLast;
    return true;
    }
  return false;
  }

QByteArray EdLog::SWrite()
    {
    return "\\log{" + m_pBB->SWrite() + "}{" + m_pCC->SWrite() + '}';
    }

bool EdDfIntegr::SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr )
  {
  if (m_pAA->SetCurrent(C, pSL, pCr) || m_pBB->SetCurrent(C, pSL, pCr) || m_pHL->SetCurrent(C, pSL, pCr) || m_pLL->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdDfIntegr::PreCalc ( TPoint P, QSize &S, int &A )
  {
  TPoint PL, PH;
  QSize SL, SH, SInt;
  int Shift = 0;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
  m_pOwner->SetPowerSize( 0, BaseLevel );
  int MyHHeight = SH.height();
  if(m_Shift == -1)
    {
    EdDfIntegr *pAA = m_pAA->GetDefIntegral();
    if(pAA != nullptr )
      {
      int HHeight = MyHHeight;
      pAA->DefineHHeight(HHeight);
      Shift = HHeight - MyHHeight;
      }
    }
  else
    Shift = m_Shift - MyHHeight;
  P.Y += Shift;
  TPoint PInt = P;
  int dX, dY, AInt;
  m_Start = P;
  EdIntegr::PreCalc( PInt, SInt, AInt );
  SInt = m_pIntegralSign->m_Size;
  PH = P;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
  PL = P;
  m_pOwner->SetPowerSize( -2, BaseLevel );
  m_pLL->PreCalc( PL, SL, m_pLL->m_Axis );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  dX = max( SH.width(), max( SInt.width(), SL.width() ) );
  if( SH.width() < dX )
    {
    PH.X += ( dX - SH.width() ) / 2;
    m_pOwner->SetPowerSize( +1, BaseLevel );
    m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
    m_pOwner->SetPowerSize( -1, BaseLevel );
    }
  if( SInt.width() < dX ) PInt.X += ( dX - SInt.width() ) / 2;
  PInt.Y += SH.height() + 2 ;
  EdIntegr::PreCalc( PInt, SInt, AInt );
  int dSign = m_pIntegralSign->m_Start.Y - PInt.Y;
  if( dSign > 0 )
    {
    PH.Y += dSign;
    m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
    PInt.Y += dSign;
    }
  if( SL.width() < dX ) PL.X += ( dX - SL.width() ) / 2;
  PL.Y = PInt.Y + SInt.height();
  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pLL->PreCalc( PL, SL, m_pLL->m_Axis );
  m_pOwner->SetPowerSize( +1, BaseLevel );
//  dX = max( SH.width(), max( SInt.width(), SL.width() ) );
//  if( dX == SInt.width() )
  m_Size.setWidth( SInt.width() + PInt.X - m_Start.X );
  m_Size.setHeight( PL.Y + SL.height() - m_Start.Y );
  dSign = m_pAA->IntegralSignPos() - m_pIntegralSign->m_Start.Y;
  if( dSign > 0 )
    {
    PH.Y += dSign;
    m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
    PL.Y += dSign;
    m_pLL->PreCalc( PL, SL, m_pLL->m_Axis );
    PInt = m_pIntegralSign->m_Start;
    PInt.Y += dSign;
    m_pIntegralSign->PreCalc( PInt, SInt, AInt );
    }
  m_Size.setHeight( max( m_Size.height(), m_pAA->m_Size.height() ) );
  S = m_Size;
  m_Axis += Shift;
  A = m_Axis;
  }

void EdDfIntegr::Draw( TPoint P )
  {
  SelectRect();
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  EdIntegr::Draw( P );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pHL->Draw( m_pHL->m_Start );
  m_pOwner->SetPowerSize( -2, BaseLevel );
  m_pLL->Draw( m_pLL->m_Start );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  }

void EdDfIntegr::ClearSelection ()
  {
  EdIntegr::ClearSelection ();
  m_pHL->ClearSelection ();
  m_pLL->ClearSelection ();
  m_Selected = false;
  }

void EdDfIntegr::SelectFragment ( QRect &FRect )
  {
  EdIntegr::SelectFragment ( FRect );
  if (m_Selected)
    {
    m_pLL->Select ();
    m_pHL->Select ();
    return;
    }
  m_pLL->SelectFragment ( FRect );
  m_pHL->SelectFragment ( FRect );
  m_Selected = ( m_pLL->m_Selected && m_pHL->m_Selected ) || ( ( m_pLL->m_Selected || m_pHL->m_Selected ) &&
    ( m_pAA->m_Selected || m_pBB->m_Selected || m_pDD->m_Selected ) );
  if ( m_Selected )
    {
    m_pLL->Select();
    m_pHL->Select();
    m_pAA->Select();
    m_pBB->Select();
    m_pDD->Select();
    }
  }

QByteArray EdDfIntegr::GetFragment ()
  {
  if ( m_Selected ) return Write() + '&';
  return m_pAA->GetFragment () + m_pDD->GetFragment () + m_pBB->GetFragment ();
  }

QByteArray EdDfIntegr::Write ()
  {
  return '(' + QByteArray(IntegrName) + '(' + m_pAA->Write () + ',' + m_pLL->Write () + ',' + m_pHL->Write () + ','+
    m_pBB->Write () + "))";
  }

bool EdDfIntegr::MoveInRight ( EdList* &pL )
  {
  pL = m_pHL;
  pL->m_pCurr = m_pHL->m_pFirst;
  return true;
  }

bool EdDfIntegr::MoveToNext ( EdList* &pL )
  {
  if ( ( pL == m_pLL ) || ( pL == m_pHL ) )
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    if ( pL == m_pAA )
      {
      pL = m_pBB;
      pL->m_pCurr = m_pBB->m_pFirst;
      return true;
      }
    else
      {
      return false;
      }
    }
  }

bool EdDfIntegr::MoveToPrev ( EdList* &pL )
  {
  if ( pL == m_pBB)
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    if ( pL == m_pAA )
      {
      pL = m_pHL;
      pL->m_pCurr = NULL;
      return true;
      }
    else
      {
      return false;
      }
    }
  }

bool EdDfIntegr::MoveToUp ( EdList* &pL )
  {
  if ( pL == m_pLL )
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    if ( ( pL== m_pAA ) || ( pL == m_pBB ) )
      {
      pL = m_pHL;
      pL->m_pCurr = NULL;
      return true;
      }
    else
      {
      return false;
      }
    }
  }

bool EdDfIntegr::MoveToDown ( EdList* &pL )
  {
  if ( pL == m_pHL )
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    if ( ( pL == m_pAA ) || ( pL == m_pBB ) )
      {
      pL = m_pLL;
      pL->m_pCurr = NULL;
      return true;
      }
    else
      {
      return false;
      }
    }
  }

QByteArray EdDfIntegr::SWrite()
    {
  QByteArray Result = "\\dint{" + m_pLL->SWrite() + "}{" + m_pHL->SWrite() + "}{";
  /*
  if( m_pBB->HasIntegral() )
    {
    QByteArray BB = m_pBB->SWrite();
    return Result + BB.mid( BB.indexOf( '*' ) + 1 ) + "}{" + BB.left( BB.indexOf( '\\' ) ) + '}';
    }
    */
  return Result + m_pAA->SWrite() + "}{" + m_pBB->SWrite() + '}';
    }

void EdDfIntegr::DefineHHeight( int &Shift )
  {
  TPoint PH;
  QSize SH;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
  m_pOwner->SetPowerSize( 0, BaseLevel );
  m_Shift = max(Shift, SH.height());
  EdDfIntegr *pAA = m_pAA->GetDefIntegral();
  if(pAA != nullptr ) pAA->DefineHHeight(m_Shift);
  }

EdGSumm::EdGSumm( XPInEdit *pOwn, uchar Sign ) : EdElm( pOwn ), m_pAA( new EdList( m_pOwner ) ), m_pLL( new EdList( m_pOwner ) ),
  m_pHL( new EdList( m_pOwner ) )
  {
  QFont SigmaFont( pOwn->m_EditSets.m_MainFont );
  SigmaFont.setPointSize( SigmaFont.pointSize() * 1.5 );
  m_pSign = new EdBigChar( Sign, irNormal, "Black", pOwn, SigmaFont, -1 );
  }

bool EdGSumm::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pAA->SetCurrent(C, pSL, pCr) || m_pHL->SetCurrent(C, pSL, pCr) || m_pLL->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdGSumm::PreCalc ( TPoint P, QSize &S, int &A )
  {
  TPoint PAA, PL, PH, PSum = P;
  QSize SAA, SL, SH, SSum;
  int dX, dY, Axis;
  m_Start = P;
  m_pSign->PreCalc( P, SSum, Axis );
  m_pAA->PreCalc( P, SAA, m_pAA->m_Axis );
  double MaxH = SAA.height();
  if( Round( MaxH ) - SSum.height() > 2 )
    {
    m_pSign->RecalcSize( MaxH / SSum.height() );
    m_pSign->PreCalc( P, SSum, Axis );
    }
  bool BaseLevel;
  BaseLevel = m_pOwner->m_DrawingPower == 0;
  PH = P;
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
  PL = P;
  m_pOwner->SetPowerSize( -2, BaseLevel );
  m_pLL->PreCalc( PL, SL, m_pLL->m_Axis );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  dX = max( SH.width(), max( SSum.width(), SL.width() ) );
  if( SH.width() < dX )
    {
    PH.X += ( dX - SH.width() ) / 2;
    m_pOwner->SetPowerSize( +1, BaseLevel );
    m_pHL->PreCalc( PH, SH, m_pHL->m_Axis );
    m_pOwner->SetPowerSize( -1, BaseLevel );
    }
  if( SSum.width() < dX ) PSum.X += ( dX - SSum.width() ) / 2;
  PSum.Y += SH.height() + 2;
  m_pSign->PreCalc( PSum, SSum, Axis );
  int AAY = PSum.Y + Axis - SAA.height() / 2;
  PAA.X = PSum.X + m_pSign->Width() + 2;
  PAA.Y = AAY;
  m_pAA->PreCalc( PAA, SAA, m_pAA->m_Axis );
  m_Axis = AAY + m_pAA->m_Axis;
  if( SL.width() < dX ) PL.X += ( dX - SL.width() ) / 2;
  PL.Y = PSum.Y + SSum.height() + 2;
  m_pOwner->SetPowerSize( -1, BaseLevel );
  m_pLL->PreCalc( PL, SL, m_pLL->m_Axis );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_Size.setWidth( max( PH.X + SH.width(), max( PAA.X + SAA.width(), PL.X + SL.width() ) ) - P.X );
  m_Size.setHeight( PL.Y + SL.height() - P.Y );
  S = m_Size;
  A = m_Axis;
  }

void EdGSumm::Draw( TPoint P )
  {
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  if( ( m_Start.X != P.X ) || ( m_Start.Y != P.Y ) )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  m_pSign->Draw( m_pSign->m_Start );
  m_pAA->Draw( m_pAA->m_Start );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  m_pHL->Draw( m_pHL->m_Start );
  m_pOwner->SetPowerSize( -2, BaseLevel );
  m_pLL->Draw( m_pLL->m_Start );
  m_pOwner->SetPowerSize( +1, BaseLevel );
  }

QByteArray EdGSumm::Write ()
  {
  return QByteArray(GSummName) + '(' + m_pAA->Write() + ',' + m_pLL->Write() + ',' + m_pHL->Write() + ')';
  }

void EdGSumm::SelectFragment ( QRect &FRect )
  {
  m_pAA->SelectFragment ( FRect );
  m_pLL->SelectFragment ( FRect );
  m_pHL->SelectFragment ( FRect );
  m_Selected = ( m_pAA->m_Selected && ( m_pLL->m_Selected || m_pHL->m_Selected ) ) || ( m_pLL->m_Selected && m_pHL->m_Selected );
  if ( m_Selected )
    {
    m_pAA->Select();
    m_pHL->Select();
    m_pLL->Select();
    }
  }

QByteArray EdGSumm::GetFragment ()
  {
  if ( m_Selected ) return Write() + '&';
    return m_pAA->GetFragment() + m_pHL->GetFragment() + m_pLL->GetFragment();
  }

void EdGSumm::ClearSelection ()
  {
  m_pAA->ClearSelection();
  m_pHL->ClearSelection();
  m_pLL->ClearSelection();
  m_Selected = false;
  }

bool EdGSumm::MoveInRight ( EdList* &pL )
  {
  pL = m_pHL;
  pL->m_pCurr = m_pHL->m_pFirst;
  return true;
  }

bool EdGSumm::MoveInLeft ( EdList* &pL )
  {
  pL = m_pAA;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdGSumm::MoveToNext ( EdList* &pL )
  {
  if ( ( pL == m_pLL ) || ( pL == m_pHL ) )
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdGSumm::MoveToPrev ( EdList* &pL )
  {
  if ( pL == m_pAA )
    {
    pL = m_pHL;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdGSumm::MoveToUp ( EdList* &pL )
  {
  if ( pL == m_pLL )
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    if ( pL == m_pAA )
      {
      pL = m_pHL;
      pL->m_pCurr = NULL;
      return true;
      }
    else
      {
      return false;
      }
    }
  }

bool EdGSumm::MoveToDown ( EdList* &pL )
  {
  if ( pL == m_pHL )
    {
    pL = m_pAA;
    pL->m_pCurr = m_pAA->m_pFirst;
    return true;
    }
  else
    {
    if ( pL == m_pAA )
      {
      pL = m_pLL;
      pL->m_pCurr = NULL;
      return true;
      }
    else
      {
      return false;
      }
    }
  }

QByteArray EdGSumm::SWrite()
    {
    return "\\summa{" + m_pLL->SWrite() + "}{" + m_pHL->SWrite() + "}{" + m_pAA->SWrite() + '}';
    }

EdGMult::EdGMult( XPInEdit *pOwn ) : EdGSumm( pOwn, msBigPi ) {}

QByteArray EdGMult::Write ()
  {
  return '(' + QByteArray(GMultName) + '(' + m_pAA->Write() + ',' + m_pLL->Write() + ',' + m_pHL->Write() + "))";
  }

QByteArray EdGMult::SWrite()
    {
    return "\\product{" + m_pLL->SWrite() + "}{" + m_pHL->SWrite() + "}{" + m_pAA->SWrite() + '}';
    }

EdSubst::EdSubst( XPInEdit *pOwn ) : EdElm( pOwn ), m_pAA( new EdList( m_pOwner ) ), m_pLL( new EdList( m_pOwner ) ),
  m_pHL( new EdList( m_pOwner ) )
  {}

bool EdSubst::SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr)
  {
  if (m_pAA->SetCurrent(C, pSL, pCr) || m_pHL->SetCurrent(C, pSL, pCr) || m_pLL->SetCurrent(C, pSL, pCr))
    {
    pSL->m_pCurr = pCr;
    return true;
    }
  if (C.X > m_Start.X + m_Size.width() / 2) return false;
  pCr = pSL->GetMemb(this);
  return true;
  }

void EdSubst::PreCalc ( TPoint P, QSize &S, int &A )
  {
  TPoint P1, PL, PH;
  QSize S1, SL, SH;
  int dY;
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  m_Start = P;

  P1.X = m_Start.X;
  P1.Y = m_Start.Y + 2; //space in top
  m_pAA->PreCalc ( P1, S1, m_pAA->m_Axis );

  m_PS.X = P1.X + S1.width() + 3; // space after expression
  m_PS.Y = m_Start.Y;
  m_SS.setWidth(1);
  m_SS.setHeight ( S1.height() + 4 );//space in top & bottom

  PH.X = m_PS.X + 3;
  PH.Y = m_Start.Y;
  m_pOwner->SetPowerSize ( +1, BaseLevel );
  m_pHL->PreCalc ( PH, SH, m_pHL->m_Axis );

  PL.X = PH.X;
  PL.Y = PH.Y + SH.height() + 2;
  m_pOwner->SetPowerSize ( -2, BaseLevel );
  m_pLL->PreCalc ( PL, SL, m_pLL->m_Axis );
  m_pOwner->SetPowerSize ( +1, BaseLevel );

  dY = ( PL.Y + SL.height() ) - ( m_PS.Y + m_SS.height() );

  if( dY < 0 )
    {
    PL.Y -= dY;
    m_pOwner->SetPowerSize( -1, BaseLevel );
    m_pLL->PreCalc( PL, SL, m_pLL->m_Axis );
    m_pOwner->SetPowerSize( +1, BaseLevel );
    }

  if ( dY > 0 )
    {
    m_SS.setHeight ( m_SS.height() + dY );
    P1.Y += dY / 2;
    m_pAA->PreCalc ( P1, S1, m_pAA->m_Axis );
    }

  m_Size.setWidth ( max ( PH.X + SH.width() , PL.X + SL.width() ) );
  m_Size.setWidth ( m_Size.width() - m_Start.X );
  m_Size.setWidth ( m_Size.width() + 4 ); // space after Subst
  m_Size.setHeight ( PL.Y + SL.height() - m_Start.Y );
  m_Axis = P1.Y - m_Start.Y + m_pAA->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdSubst::Draw ( TPoint P )
  {
  bool BaseLevel = m_pOwner->m_DrawingPower == 0;
  if ((m_Start.X != P.X) || (m_Start.Y != P.Y))
    {
    m_Start = P;
    PreCalc(m_Start, m_Size, m_Axis);
    }
  SelectRect();
  m_pAA->Draw ( m_pAA->m_Start );
  QPen Pen = m_pOwner->m_pCanvas->pen();// Owner.Canvas.Pen := Owner.EditSets.SignPen;
  Pen.setWidth (1);
  m_pOwner->m_pCanvas->setPen( Pen );
  m_pOwner->Line ( m_PS.X, m_PS.Y, m_PS.X, m_PS.Y + m_SS.height() );
  m_pOwner->SetPowerSize ( +1, BaseLevel );
  m_pHL->Draw ( m_pHL->m_Start );
  m_pOwner->SetPowerSize ( -2, BaseLevel );
  m_pLL->Draw ( m_pLL->m_Start );
  m_pOwner->SetPowerSize ( +1, BaseLevel );
  }

QByteArray EdSubst::Write()
  {
  return '(' + QByteArray(SubstName) + '(' + m_pAA->Write() + ',' + m_pLL->Write() + ',' + m_pHL->Write() + "))";
  }

bool EdSubst::MoveInRight( EdList* &pL )
  {
  pL = m_pAA;
  pL->m_pCurr = m_pAA->m_pFirst;
  return true;
  }

bool EdSubst::MoveInLeft( EdList* &pL )
  {
  pL = m_pHL;
  pL->m_pCurr = NULL;
  return true;
  }

bool EdSubst::MoveToNext( EdList* &pL )
  {
  if ( pL == m_pAA )
    {
    pL = m_pHL;
    pL->m_pCurr = m_pHL->m_pFirst;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdSubst::MoveToPrev( EdList* &pL )
  {
  if ( ( pL == m_pLL ) || ( pL == m_pHL ) )
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    return false;
    }
  }

bool EdSubst::MoveToUp( EdList* &pL )
  {
  pL = m_pHL;
  return true;
  /*
  if ( pL == m_pLL )
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    if ( pL == m_pAA )
      {
      pL = m_pHL;
      pL->m_pCurr = m_pHL->m_pFirst;
      return true;
      }
    else
      {
      return false;
      }
    }
*/
  }

bool EdSubst::MoveToDown( EdList* &pL )
  {
  pL = m_pLL;
  return true;
/*
  if ( pL == m_pHL )
    {
    pL = m_pAA;
    pL->m_pCurr = NULL;
    return true;
    }
  else
    {
    if ( pL == m_pAA )
      {
      pL = m_pLL;
      pL->m_pCurr = m_pLL->m_pFirst;
      return true;
      }
    else
      {
      return false;
      }
    }
    */
  }

QByteArray EdSubst::SWrite()
    {
    return "\\subst{" + m_pAA->SWrite() + "}{" + m_pLL->SWrite() + "}{" + m_pHL->SWrite() + '}';
    }

EdStr::EdStr(XPInEdit *pOwn, QByteArray Text, bool NoSelectFont) : EdElm(pOwn), m_Value(Text), m_SelStart(-1), m_SelEnd(-1), m_NoSelectFont(NoSelectFont)
  {
  if(Text[0] != (char) msBaseLang)
    {
    QString UString;
    for( int iChar = 0; iChar < Text.length(); UString += ::ToUnicode( Text[iChar++] ) );
    m_SValue = UString.split('\n');
    return;
    }
  m_SValue = ToLang(Text.mid(1).replace(msPrime, '"').replace(msDoublePrime, '{').replace(msTriplePrime, '}').replace(msCharNewLine, '\n')).split('\n');
  }

EdStr::EdStr(XPInEdit *pOwn, QString text, bool NoSelectFont) : EdElm(pOwn), m_SelStart(-1),m_SelEnd(-1), m_NoSelectFont(NoSelectFont)
  {
  m_Value = sm_pCodec->fromUnicode(text).replace('\n', msCharNewLine);
  m_SValue = text.split('\n');
  }

void EdStr::SelectFont()
  {
  if( m_NoSelectFont ) return;
  if( m_pOwner->m_pCurrentTable == nullptr )
    m_pOwner->SetFont( m_pOwner->m_ViewSettings.m_TaskCmFont );
  else
    {
    TableGrid TGState = m_pOwner->m_pCurrentTable->m_GridState;
    if( TGState == TGRVisible || TGState == TGRPartlyVisible)
      m_pOwner->SetFont( m_pOwner->m_ViewSettings.m_SimpCmFont );
    }
  }

void EdStr::PreCalc ( TPoint P, QSize &S, int &A )
  {
  SelectFont();
  m_Start = P;
  QFontMetrics FM = m_pOwner->m_pCanvas->fontMetrics();
  m_Size *= 0.0;
  for (auto pStr = m_SValue.begin(); pStr != m_SValue.end(); pStr++)
    if( pStr->length() == 1)
      {
      m_Size.setHeight(max(m_Size.height(), m_pOwner->CharHeight()));
      m_Size.setWidth(max(m_Size.width(), m_pOwner->CharWidth(pStr->front())));
      }
    else
      {
      QRect RLine(FM.boundingRect(*pStr));
      m_Size.setHeight(max(m_Size.height(), RLine.height()));
      m_Size.setWidth(max(m_Size.width(), RLine.width() + 10));
      }
  m_Axis = m_Size.height() / 2 + FM.strikeOutPos();
  S = m_Size;
  A = m_Axis;
  if( !m_NoSelectFont ) m_pOwner->RestoreFont();
  }

void EdStr::Draw( TPoint P )
  {
  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }
  SelectRect();
  SelectFont();
  int StrHeight = m_Size.height() / m_SValue.count();
  int StartY = m_Start.Y;
//  m_pOwner->m_pCanvas->setLayoutDirection(Qt::RightToLeft);
  m_pOwner->m_pCanvas->setLayoutDirection(Qt::LeftToRight);
  for (auto pStr = m_SValue.begin(); pStr != m_SValue.end(); pStr++, StartY += StrHeight)
    if(pStr->length() == 1)
      {
      QByteArray Text(pStr->toUtf8());
      if(Text.length() == 2 )
        m_pOwner->PaintChar( m_Start.X, StartY, ToUnicode(msMultSign2 ));
      else
        m_pOwner->PaintChar(m_Start.X, StartY, pStr->front());
      }
    else
      {
      QString Text(*pStr);
      m_pOwner->m_pCanvas->drawText(QRectF(m_Start.X, StartY, m_Size.width(), StrHeight), *pStr, QTextOption(Qt::AlignRight));
      }
  m_pOwner->m_pCanvas->setLayoutDirection(Qt::LeftToRight);
  if( m_SelStart == -1 || m_Selected )
    {
    if( !m_NoSelectFont ) m_pOwner->RestoreFont();
    return;
    }
/*
  int X0 = 0;
  if( m_SelStart > 0 )
    X0 = m_pOwner->TextWidth( m_SValue.left( m_SelStart ) );
  int XEnd = m_Size.width();
  if( m_SelEnd < m_SValue.length() )
     XEnd -= m_pOwner->TextWidth( m_SValue.mid( m_SelEnd + 1 ) );
  QRect R( QPoint(m_Start.X, m_Start.Y), m_Size );
  R.moveRight( X0 );
  R.setWidth( XEnd - X0 );
  m_pOwner->SelectRect( R );
  */
  }

void EdStr::SelectFragment( QRect &FRect )
  {
/*
  m_Selected = false;
  if( !( ( m_Start.Y >= FRect.top() && m_Start.Y < FRect.bottom() ) ||
    ( m_Start.Y + m_Size.height() <= FRect.bottom() && m_Start.Y + m_Size.height() > FRect.top() ) ||
    ( m_Start.Y <= FRect.top() && m_Start.Y + m_Size.height() >= FRect.bottom() ) ) )
    return;
  if( m_Start.X + 2 >= FRect.left() && m_Start.X + m_Size.width() - 2 <= FRect.right() )
    {
    m_SelStart = 0;
    m_SelEnd = m_Value.length() - 1;
    m_Selected = true;
    return;
    }
  SelectFont();
  int i;
  QString sLeft;
  for( int i = 0; i < m_SValue.length(); i++ )
    {
    if( m_SelStart == -1 && m_Start.X + m_pOwner->TextWidth( sLeft ) >= FRect.left() )
      m_SelStart = i;
    if( m_SelStart != -1 && m_Start.X + m_Size.width() - m_pOwner->TextWidth( m_SValue.mid( i + 1 ) ) > FRect.right() )
      {
      m_SelEnd = i - 1;
      m_pOwner->RestoreFont();
      return;
      }
    sLeft += m_SValue[i];
    }
  if( m_SelStart != -1 ) m_SelEnd = m_SValue.length() - 1;
  m_Selected = m_SelStart == 0;
  m_pOwner->RestoreFont();
  */
  }

 void EdStr::ClearSelection()
    {
    m_SelStart = -1;
    m_SelEnd = -1;
    m_Selected = false;
    }

QByteArray EdStr::GetFragment()
  {
  if( m_SelStart == -1 ) return "";
  return m_Value.mid( m_SelStart, m_SelEnd - m_SelStart + 1 );
  }

QByteArray EdStr::Write()
    {
  if( sm_PureText ) return m_Value;
    return '"'+ QByteArray(m_Value).replace('"', msPrime).replace('{', msDoublePrime).replace('}', msTriplePrime) + '"';
    }

QByteArray EdStr::SWrite()
    {
    QByteArray Text(m_Value);
    return "\\comment{" + Text.replace('{', msBrackLeftUpper ).replace('}', msBrackRightUpper ) + '}';
    }

void EdStr::AddChar(char c)
  {
  m_Value += c;
  m_SValue.last() += sm_pCodec->toUnicode(QByteArray(1, c));
  }

EdTable::EdTable( XPInEdit* pOwn, const QByteArray& Parms ) : EdElm( pOwn ), m_IsTEdChartEditor( false ), m_WasTemplate( sm_WasTemplate ),
  m_Row(0), m_Col(0), m_ColCount(0), m_pCurrentCell(nullptr), m_pOwnerList(nullptr)
  {
  QByteArrayList ParmList( Parms.split( ',' ) );
  m_GridState = (TableGrid) ParmList[2].toInt();
  m_RowCount = ParmList[0].toInt();
  m_Body.resize( m_RowCount );
  m_SizeRows.resize( m_RowCount );
  m_FrozenCells.resize( m_RowCount );
  m_SelectedCount.resize( m_RowCount );
  SetColumnNumber( ParmList[1].toInt() );
  m_NoFreeze = ParmList[3].toInt();
  int UnvisibleCount = ParmList[4].toInt();
  for( int i = 0; i < UnvisibleCount; i++ )
    m_UnvisibleColumns[ParmList[i + 5].toInt()] = true;
  }

void EdTable::SetColumnNumber( int NewNumber )
  {
  m_SizeCols.resize( NewNumber );
  m_UnvisibleColumns.resize( NewNumber );
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_FrozenCells[i].resize( NewNumber );
    if( m_ColCount < NewNumber )
      for( int j = m_ColCount; j < NewNumber; j++ )
        {
        EdList *pList = new EdList( m_pOwner );
        pList->m_IsCell = m_GridState != TGRUnvisible && m_GridState != TGRPartlyVisible;
        pList->m_pSub_L = new EdList( m_pOwner );
        m_Body[i].push_back( pList );
        }
    else
      m_Body[i].resize( NewNumber );
    }
  m_ColCount = NewNumber;
  }

bool EdTable::SetCurrent( const TPoint& C, EdList*& SL, EdMemb* &Cr )
  {
  Cr = nullptr;
  int y = m_Start.Y + m_BorderSize;
  int i = -1;
  for( int k = 0; i < m_RowCount; k++ )
    {
    if( C.Y >= y  &&  C.Y <= y + m_SizeRows[k] )
      {
      i = k;
      break;
      }
    y += m_SizeRows[k];
    }
  if( i == -1 ) return false;
  int j = -1;
  int x = m_Start.X + m_BorderSize;
  for( int k = 0; k < m_ColCount; k++ )
    {
    if( C.X >= x && C.X <= x + m_SizeCols[k] )
      {
      if( m_FrozenCells[i][k] ) return false;
      j = k;
      break;
      }
    x += m_SizeCols[k];
    }
  if( j == -1 ) return false;
  m_pOwner->m_pCurrentTable = this;
  SL = m_Body[i][j]->m_pSub_L;
  Cr = SL->m_pCurr;
  m_Row = i;
  m_Col = j;
  if( !XPInEdit::sm_EditString ) return true;
  EdStr::sm_PureText = true;
  EditString Dlg( ToLang( SL->Write() ) );
  EdStr::sm_PureText = false;
  if( Dlg.exec() == QDialog::Rejected ) return true;
  if( Dlg.IsString() )
    {
    QString Result = Dlg.GetString();
    SL->m_pSub_L->Clear();
    if( !Result.isEmpty() )
      SL->m_pSub_L->Append_Before( new EdStr( m_pOwner, Result ) );
    return true;
    }
  QByteArray Result = Dlg.GetFormula();
  SL->m_pSub_L->Clear();
  if( !Result.isEmpty() )
  for( int i = 0; i < Result.count(); i++ )
    SL->m_pSub_L->Append_Before( new EdChar( Result[i], m_pOwner ) );
  return true;
  }

void EdTable::PreCalc( TPoint P, QSize& S, int& A )
  {
  m_pOwner->m_pCurrentTable = this;
  m_Start = P;
  for( int j = 0; j < m_ColCount; j++ )
    m_SizeCols[j] = 0;
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_SizeRows[i] = 0;
    for( int j = 0; j < m_ColCount; j++ )
      if( m_UnvisibleColumns[j] )
        m_FrozenCells[i][j] = true;
      else
        {
        m_Body[i][j]->m_pSub_L->PreCalc( P, m_Size, A );
        m_SizeRows[i] = max( m_SizeRows[i], m_Size.height() );
        m_SizeCols[j] = max( m_SizeCols[j], m_Size.width() );
        }
    }

  m_Size.setHeight( m_BorderSize * 2 );
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_SizeRows[i] += 2 * m_BorderSize;
    m_Size.setHeight( m_Size.height() + m_SizeRows[i] );
    }

  m_Size.setWidth( m_BorderSize * 2 );
  for( int j = 0; j < m_ColCount; j++ )
    {
    if( !m_UnvisibleColumns[j] && ( m_GridState != TGRUnvisible || m_Body[0][j]->m_pSub_L->m_Size.width() == 0 ) )
      m_SizeCols[j] += 2 * m_BorderSize;
    m_Size.setWidth( m_Size.width() + m_SizeCols[j] );
    }

  int y = m_Start.Y + m_BorderSize;
  for( int i = 0; i < m_RowCount; i++ )
    {
    int x = m_Start.X + m_BorderSize;
    for( int j = 0; j < m_ColCount; j++ )
      {
      if(  m_Body[i][j]->m_pSub_L->m_Size.width() == 0 ||
        ( m_Body[i][j]->m_pSub_L->m_pFirst != nullptr && IsConstEdType( EdChart, m_Body[i][j]->m_pSub_L->m_pFirst->m_pMember.data() ) ) )
        P.X = x + m_BorderSize;
      else
        P.X = x + ( m_SizeCols[j] - m_Body[i][j]->m_pSub_L->m_Size.width() ) / 2;
      P.Y = y + ( m_SizeRows[i] - m_Body[i][j]->m_pSub_L->m_Size.height() ) / 2;
      QSize S1;
      m_Body[i][j]->m_pSub_L->PreCalc( P, S1, A );
      x += m_SizeCols[j];
      }
    y += m_SizeRows[i];
    }
  m_Axis = m_Size.height() / 2;
  S = m_Size;
  A = m_Axis;
  }

void EdTable::Draw( TPoint P )
  {
  auto CanDrawBorder = [] ( EdList* pCell )
    {
    EdMemb *pFirst = pCell->m_pSub_L->m_pFirst;
    if(pFirst == nullptr) return true;
    EdElm *pData = pFirst->m_pMember.data();
    return !( IsConstEdType( EdStr, pData ) ) && (!( IsConstEdType( EdChar, pData ) ) || ( dynamic_cast< EdChar* >( pData )->c() != '"' ) );
    };
  SelectRect();
  m_pOwner->m_pCurrentTable = this;
  switch( m_GridState )
    {
    case TGRVisible:
      {
      m_pOwner->SetPaintColor( m_pOwner->m_EditSets.m_SignColor );
      int XStart = m_Start.X + m_BorderSize, XEnd = m_Start.X + m_Size.width() - m_BorderSize;
      int YStart = m_Start.Y + m_BorderSize, YEnd = m_Start.Y + m_Size.height() - m_BorderSize;
      m_pOwner->Line( XStart, m_Start.Y + m_BorderSize, XEnd, m_Start.Y + m_BorderSize );
      m_pOwner->Line( XStart, YStart, XStart, m_Start.Y + YEnd );
      int y = YStart;
      for( int i = 0; i < m_RowCount; i++ )
        {
        y += m_SizeRows[i];
        m_pOwner->Line( XStart, y, XEnd, y );
        }
      int x = XStart;
      for( int j = 0; j < m_ColCount; j++ )
        {
        x += m_SizeCols[j];
        m_pOwner->Line( x, YStart, x, YEnd );
        }
      for( int i = 0, Top = m_BorderSize; i < m_RowCount; Top += m_SizeRows[i++] )
        for( int j = 0, Left = m_BorderSize; j < m_ColCount; Left += m_SizeCols[j++] )
          {
          if( m_UnvisibleColumns[j] ) continue;
          EdList &Body = *m_Body[i][j]->m_pSub_L;
          if( m_FrozenCells[i][j] && ( !m_IsTEdChartEditor || m_RowCount == 2 ) )
            FloodFill( QRect( Left, Top, m_SizeCols[j], m_SizeRows[i] ),
              dynamic_cast< QImage* >( m_pOwner->m_pCanvas->device() ), Body.Start(), QColor( 0x00ffff ) );
          Body.Draw( Body.m_Start );
          }
      m_pOwner->RestorePaintColor();
      }
      break;
    case TGRUnvisible:
      for( int i = 0; i < m_RowCount; i++ )
        for( int j = 0; j < m_ColCount; j++ )
          {
          EdList *pCell = m_Body[i][j]->m_pSub_L;
          pCell->Draw( pCell->m_Start );
          }
      break;
    case TGRPartlyVisible:
      {
      m_pOwner->SetPaintColor( m_pOwner->m_EditSets.m_SignColor );
      int y = m_Start.Y + m_BorderSize;
      for( int i = 0; i < m_RowCount; i++ )
        {
        int YNext = y + m_SizeRows[i];
        int x = m_Start.X + m_BorderSize;
        if( CanDrawBorder( m_Body[i][0] ) )
          m_pOwner->Line( x, y, x, YNext );
        for( int j = 0; j < m_ColCount; j++ )
          {
          int XNext = x + m_SizeCols[j];
          if( CanDrawBorder( m_Body[i][j] ) )
            {
            m_pOwner->Line( x, y, XNext, y );
            m_pOwner->Line( x, YNext, XNext, YNext );
            x = XNext;
            }
          x = XNext;
          if( j < m_ColCount - 1 && CanDrawBorder( m_Body[i][j + 1] ) || CanDrawBorder( m_Body[i][j] ) )
            m_pOwner->Line( x, y, x, YNext );
          }
        y = YNext;
        }
      for( int i = 0; i < m_RowCount; i++ )
        for( int j = 0; j < m_ColCount; j++ )
          m_Body[i][j]->m_pSub_L->Draw( m_Body[i][j]->m_pSub_L->m_Start );
      }
    }
  }

QByteArray EdTable::GetFragment()
  {
  if( m_Selected ) return Write() + "&";
  bool bFragment = false;
  QByteArray Result;
  for( int i = 0; i < m_RowCount; i++ )
    if( m_SelectedCount[i] > 0 )
      {
      if( m_SelectedCount[i] == m_ColCount )
        {
        bFragment = true;
        break;
        }
      for( int j = 0; j < m_ColCount; j++ )
        Result += m_Body[i][j]->m_pSub_L->GetFragment();
      return Result;
      }
  if( !bFragment ) return Result;
  Result = "Table(";
  bool SelectionStarted = false;
  for( int i = 0; i < m_RowCount; i++ )
    {
    if( m_SelectedCount[i] > 0 )
      {
      if( SelectionStarted ) Result += ';';
      SelectionStarted = true;
      for( int j = 0; j < m_ColCount; j++ )
        {
        QByteArray S = m_Body[i][j]->m_pSub_L->Write();
        if( S.isEmpty() ) S = " ";
        if( S.indexOf( ',' ) > -1 && IsListEx( S ) )
          S = "@ListOrd(" + S + ')';
        Result += S;
        if( j < m_ColCount - 1 ) Result += ',';
        }
      }
    else
      if( SelectionStarted ) break;
    }
  Result += ')';
  return Result;
  }

void EdTable::SelectFragment( QRect &FRect )
  {
  int SelectedRows = 0;
  for( int i = 0; i < m_RowCount; i++ )
    {
    for( int j = 0; j < m_ColCount; j++ )
      {
      m_Body[i][j]->SelectFragment( FRect );
      if( m_Body[i][j]->m_pSub_L->m_Selected )
        m_SelectedCount[i]++;
      }
    if( m_SelectedCount[i] > 0 )
      SelectedRows++;
    }
  if( SelectedRows == 0 ) return;
  if( SelectedRows == m_RowCount )
    {
    m_Selected = true;
    for( int i = 0; i < m_RowCount; i++ )
      for( int j = 0; j < m_ColCount; j++ )
        m_Body[i][j]->m_pSub_L->Select();
    return;
    }
  if( SelectedRows == 1 ) return;
  for( int i = 0; i < m_RowCount; i++ )
    if( m_SelectedCount[i] > 0 )
      for( int j = 0; j < m_ColCount; j++ )
        m_Body[i][j]->m_pSub_L->Select();
  }

void EdTable::ClearSelection()
  {
  for( int i = 0; i < m_RowCount; i++ )
    if( m_SelectedCount[i] > 0 )
      {
      m_SelectedCount[i] = 0;
      for( int j = 0; j < m_ColCount; j++ )
        m_Body[i][j]->m_pSub_L->ClearSelection();
      }
  m_Selected = false;
  }

QByteArray EdTable::Write()
  {
  QByteArray Result;
  if( sm_RowTableAsExpression )
    {
    for( int j = 0; j < m_ColCount; j++ )
      if( m_Body[0][j]->m_pFirst != nullptr )
        {
        EdStr *pStr = dynamic_cast< EdStr* >( m_Body[0][j]->m_pSub_L->m_pFirst->m_pMember.data() );
        if( pStr != nullptr )
          {
          if( pStr->m_Value == "=" ) return Result;
          Result += pStr->m_Value;
          }
        else
          Result += m_Body[0][j]->m_pSub_L->m_pFirst->m_pMember->Write();
        }
    return Result;
    }
  switch( m_GridState )
    {
    case TGRVisible:
      Result = "Table(";
      break;
    case TGRUnvisible:
      Result = "UnvisibleTable(";
      break;
    case TGRPartlyVisible:
      Result = "PartlyVisibleTable(";
    }
  if( m_NoFreeze ) Result += "NoFreeze;";
  QByteArray S;
  for( int j = 0; j < m_ColCount; j++ )
    if( m_UnvisibleColumns[j] ) S += ';' + QByteArray::number( j );
  if( !S.isEmpty() ) Result += "Unvisible" + S + ";";
  for( int i = 0; i < m_RowCount; i++ )
    {
    for( int j = 0; j < m_ColCount; j++ )
      {
      S = m_Body[i][j]->m_pSub_L->Write();
      if( S.isEmpty() ) S = "\" \"";
      if( S.indexOf( ',' ) > -1 && IsListEx( S ) )
        S = "@ListOrd(" + S + ')';
      Result += S;
      if( j < m_ColCount - 1 ) Result += ',';
      }
    if( i < m_RowCount - 1 ) Result += ';';
    }
  Result += ')';
  return Result;
  }

QByteArray EdTable::SWrite()
  {
  QByteArray Result( "\\table{" + QByteArray::number( m_RowCount ) + ',' + QByteArray::number( m_ColCount ) + ',' +
    QByteArray::number( m_GridState ) + ',' + QByteArray::number( m_NoFreeze ) + ',' );
  int UnvisibleCount = 0;
  for( int i = 0; i < m_UnvisibleColumns.count(); i++ )
    if( m_UnvisibleColumns[i] ) UnvisibleCount++;
  Result += QByteArray::number( UnvisibleCount );
  if( UnvisibleCount > 0 )
    for( int i = 0; i < m_UnvisibleColumns.count(); i++ )
      if( m_UnvisibleColumns[i] ) Result += ',' + QByteArray::number( i );
  Result += '}';
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      Result += "\\cell{" + m_Body[i][j]->m_pSub_L->SWrite() + '}';
  return Result + "\\endtable";
  }

EdChart::EdChart( XPInEdit *pOwn, const QByteArray& Parms, bool NoStrSelectFont ) : EdElm( pOwn )
  {
  int iComma = Parms.indexOf( ',' );
  m_Type = Parms.left( iComma ) == "ThinCol" ? ThinCol : ThickCol;
  int iNext = Parms.indexOf( ',', iComma + 1 );
  int N = Parms.mid( iComma + 1, iNext - iComma - 1 ).toInt();
  m_Colors.reserve( N );
  m_Labels.reserve( N );
  m_Y.reserve( N );
  iComma = iNext;
  iNext = Parms.indexOf( ',', iComma + 1 );
  m_pLabelX = new EdStr( pOwn, Parms.mid( iComma + 1, iNext - iComma - 1 ), NoStrSelectFont );
  iComma = iNext;
  iNext = Parms.indexOf( ',', iComma + 1 );
  m_pLabelY = new EdStr( pOwn, Parms.mid( iComma + 1, iNext - iComma - 1 ), NoStrSelectFont );
  iComma = iNext;
  iNext = Parms.indexOf( ',', iComma + 1 );
  m_pNameX = new EdStr( pOwn, Parms.mid( iComma + 1, iNext - iComma - 1 ), NoStrSelectFont );
  iComma = iNext;
  iNext = Parms.indexOf( ',', iComma + 1 );
  m_pNameY = new EdStr( pOwn, Parms.mid( iComma + 1, iNext - iComma - 1 ), NoStrSelectFont );
  iComma = iNext;
  iNext = Parms.indexOf( ',', iComma + 1 );
  m_Scale = Parms.mid( iComma + 1, iNext - iComma - 1).toDouble();
  iComma = iNext;
  iNext = Parms.indexOf( ',', iComma + 1 );
  m_FromTemplate = Parms.mid( iComma + 1, iNext - iComma - 1 ).toInt();
  int i = 0;
  for( iComma = iNext; i < N; i++, iComma = iNext )
    {
    iNext = Parms.indexOf( ',', iComma + 1 );
    QColor C( QColor::fromRgb( Parms.mid( iComma + 1, iNext - iComma - 1 ).toUInt( nullptr, 16 ) ) );
    m_Colors.push_back( C );
    }
  for( i = 0; i < N; i++, iComma = iNext )
    {
    iNext = Parms.indexOf( ',', iComma + 1 );
    m_Labels.push_back( Parms.mid( iComma + 1, iNext - iComma - 1 ) );
    }
  for( i = 0; i < N; i++, iComma = iNext )
    {
    iNext = Parms.indexOf( ',', iComma + 1 );
    m_Y.push_back( Parms.mid( iComma + 1, iNext - iComma - 1 ).toInt() );
    }
  QString sTmp = dynamic_cast<EdStr*>( &*m_pNameX )->m_SValue[0];
  while( !sTmp.isEmpty() )
    {
    int k = sTmp.indexOf( ' ' );
    if( k == -1 )
      {
      m_Words.push_back( sTmp );
      break;
      }
    m_Words.push_back( sTmp.left( k ) );
    sTmp = sTmp.mid( k + 1 );
    }
  }

EdChart::EdChart( XPInEdit *pOwn ) : EdElm( pOwn ), m_Type( ThinCol ), m_pLabelX( new EdStr( pOwn, "X" ) ),
m_pLabelY( new EdStr( pOwn, "Y" ) ), m_pNameX( new EdStr( pOwn, "Name X" ) ), m_pNameY( new EdStr( pOwn, "Name Y" ) ),
m_Scale( 1.0 ), m_FromTemplate( false )
  {
  m_Colors.reserve( 2 );
  m_Labels.reserve( 2 );
  m_Y.reserve( 2 );
  for( int i = 0; i < 2; i++ )
    {
    m_Colors.push_back( QColor( Qt::white ) );
    m_Labels.push_back( QByteArray::number( i ) );
    m_Y.push_back( i );
    }
  }

void EdChart::SetColumnNumber( int NewNumber )
  {
  if( NewNumber == m_Labels.count() ) return;
  m_Y.resize( NewNumber);
  m_Labels.resize( NewNumber );
  m_Colors.resize( NewNumber );
  }

QByteArray EdChart::SWrite()
  {
  QByteArray Result = "\\chart{" + QByteArray( m_Type == ThinCol ? "ThinCol" : "ThickCol" ) + ','
    + QByteArray::number( m_Colors.count() ) + ',' + dynamic_cast<EdStr*>(&*m_pLabelX)->m_Value + ',' +
    dynamic_cast<EdStr*>( &*m_pLabelY )->m_Value + ',' + dynamic_cast<EdStr*>(&*m_pNameX)->m_Value + ',' +
    dynamic_cast<EdStr*>(&*m_pNameY)->m_Value + ',' + QByteArray::number( m_Scale ) + ',' + QByteArray::number( m_FromTemplate );
  for( int i = 0; i < m_Colors.count(); i++ )
    Result += ',' + QByteArray::number( m_Colors[i].rgb(), 16 );
  for( int i = 0; i < m_Labels.count(); i++ )
    Result += ',' + m_Labels[i];
  for( int i = 0; i < m_Y.count(); i++ )
    Result += ',' + QByteArray::number( m_Y[i] );
  return Result + '}';
  }

QByteArray EdChart::Write()
  {
  QByteArray Result = "Chart(m=" + QByteArray::number( m_Scale ) + ';' + ( m_Type == ThinCol ? "ThinCol" : "ThickCol" );
  for( int i = 0; i < m_Colors.count(); i++ )
    Result += ",0X" + QByteArray::number( m_Colors[i].rgb(), 16 ).mid(2);
  QByteArray Label( dynamic_cast< EdStr* >( &*m_pLabelX )->m_Value );
  QByteArray Name( dynamic_cast< EdStr* >( &*m_pNameX )->m_Value );
  if( !Label.isEmpty() ) Name += ':';
  Result += ";\"" + Name + Label + '"';
  for( int i = 0; i < m_Labels.count(); i++ )
    Result += ',' + m_Labels[i];
  Name = dynamic_cast< EdStr* >( &*m_pNameY )->m_Value;
  Label = dynamic_cast< EdStr* >( &*m_pLabelY )->m_Value;
  if( !Label.isEmpty() ) Name += ':';
  Result += ";\"" + Name + Label + '"';
  for( int i = 0; i < m_Y.count(); i++ )
    Result += ',' + QByteArray::number( m_Y[i] );
  return Result + ')';
  }

int EdChart::CalculateStep( int N, int KX )
  {
  int iStep = 1;
  if( N > 10 )
    {
    int k = Round( pow( 10, floor( log10( N ) ) ) );
    iStep = ceil( (double) N / k ) * KX;
    if( iStep > 5 )
      iStep = 10;
    else
      if( iStep > 2 )
        iStep = 5;
    iStep = iStep * k / 10;
    }
  return iStep;
  }

double EdChart::GetX( int j, bool bRight )
  {
  int iPos = m_Labels[j].indexOf( '-' );
  if( iPos == -1 ) return m_Labels[j].toDouble();
  if( bRight ) return m_Labels[j].mid( iPos + 1 ).toDouble();
  return m_Labels[j].left( iPos ).toDouble();
  }

void EdChart::SelectFont()
  {
  QFont F( m_pOwner->m_ViewSettings.m_TaskCmFont );
  F.setPointSize( 10 );
  m_pOwner->SetFont( F );
  }

void EdChart::PreCalc( TPoint P, QSize& S, int& A )
  {
  m_FullSize = Round( 300 * m_Scale );
  m_XMin = GetX( 0 );
  m_MaxX = GetX( m_Labels.count() - 1, true );
  m_MaxY = m_Y.back();
  double dxMin = m_MaxX, dxMax = 0;
  double xPrev;
  for( int i = 0; i < m_Labels.count(); i++ )
    {
    double Value = m_Y[i];
    if( Value > m_MaxY ) m_MaxY = Value;
    if( m_Type == ThickCol )
      {
      Value = GetX( i, true ) - GetX( i );
      dxMin = min( dxMin, Value );
      dxMax = max( dxMax, Value );
      }
    else
      {
      Value = GetX( i );
      if( i > 0 )
        {
        dxMin = min( dxMin, Value - xPrev );
        dxMax = max( dxMax, Value - xPrev );
        }
      xPrev = Value;
      }
    }
  m_Dx = m_MaxX - m_XMin + 1.5 * dxMax;
  SelectFont();
  m_X0 = m_pOwner->TextWidth( QByteArray::number( ceil( m_MaxY ) ) ) + 2;
  m_FullSize = max( m_FullSize, ( int ) Round( m_X0 / dxMin * m_Dx ) );
  int iStep = CalculateStep( round( m_Dx ), 1 );
  int k = m_pOwner->TextWidth( QByteArray::number( Round( m_MaxX ) ) );
  m_VerticalLabelX = k + 5 > Round( iStep / m_Dx * m_FullSize );
  m_Y0 = m_FullSize + dynamic_cast<EdStr*>(&*m_pNameX)->Height() + sc_Shift;
  if( m_VerticalLabelX )
    m_Size.setHeight( m_Y0 + k + 5 );
  else
    m_Size.setHeight( m_Y0 + m_pOwner->CharHeight() + 5 );
  int iMaxWordLen = 0;
  for( int i = 0; i < m_Words.count(); i++ )
    iMaxWordLen = max( iMaxWordLen, m_pOwner->TextWidth( m_Words[i] ) );
  m_Size.setWidth( m_FullSize + m_X0 + iMaxWordLen + 20 );
  m_Start = P;
  m_Axis = m_Size.height() / 2;
  S = m_Size;
  A = m_Axis;
  m_pOwner->RestoreFont();
  }

void EdChart::Draw( TPoint P )
  {
  const int HalfCol = 2, HalfShelf = 4;
  auto PutLabel = [&]( int ColCenter, int iX, int iY )
    {
    QByteArray sLabel( QByteArray::number( m_Y[iX] ) );
    m_pOwner->TextOut( m_Start.X + ColCenter - m_pOwner->TextWidth( sLabel ) / 2, m_Start.Y + iY - 2, sLabel );
    };

  if( m_Start.X != P.X || m_Start.Y != P.Y )
    {
    m_Start = P;
    PreCalc( m_Start, m_Size, m_Axis );
    }

  SelectFont();
  int HarHeight = m_pOwner->CharHeight();
  m_pLabelY->Draw( TPoint( m_Start.X + m_X0 - m_pOwner->CharWidth( 'f' ) - 10, m_Start.Y ) );
  m_pLabelX->Draw( TPoint( m_Start.X + m_Size.width() - 20, m_Start.Y + m_Y0 ) );

  int iy = m_Start.Y + m_Y0 - dynamic_cast<EdStr*>(&*m_pNameX)->Height() * m_Words.count();
  for( int i = 0; i < m_Words.count(); i++ )
    {
    m_pOwner->TextOut( m_Start.X + m_X0 + m_FullSize + 5, iy, m_Words[i] );
    iy = iy + m_pOwner->TextHeight( m_Words[i] );
    }
  m_pNameY->Draw( TPoint(m_Start.X + m_X0 + 10, m_Start.Y) );

  m_pOwner->Line( m_Start.X, m_Start.Y + m_Y0, m_Start.X + m_Size.width(), m_Start.Y + m_Y0 );
  m_pOwner->Line( m_Start.X + m_Size.width(), m_Start.Y + m_Y0, m_Start.X + m_Size.width() - 10, m_Start.Y + m_Y0 - 5 );
  m_pOwner->Line( m_Start.X + m_Size.width(), m_Start.Y + m_Y0, m_Start.X + m_Size.width() - 10, m_Start.Y + m_Y0 + 5 );

  int iStep = CalculateStep( Round( m_Dx ), 1 );
  int iX0 = floor( ( m_XMin - iStep ) / iStep )*iStep;
  if( m_VerticalLabelX )
    {
    for( int i = 0; i + iX0 <= Round( m_MaxX ); i = i + iStep )
      {
      int k = Round( i / m_Dx * m_FullSize );
      m_pOwner->Line( m_Start.X + m_X0 + k, m_Start.Y + m_Y0, m_Start.X + m_X0 + k, m_Start.Y + m_Y0 - 5 );
      if( i != 0 )
        {
        QPainter &Canvas = *m_pOwner->m_pCanvas;
        QByteArray sLabel = QByteArray::number( i + iX0 );
        int dx = m_pOwner->TextHeight( sLabel );
        int dy = m_pOwner->TextWidth( sLabel );
        QImage Im( dx + 1, dy, QImage::Format_ARGB32_Premultiplied );
        QPainter Painter( &Im );
        Painter.setFont( Canvas.font() );
        QColor BackgroundColor = m_pOwner->m_EditSets.m_BkgrColor;
        Painter.setPen( Canvas.pen() );
        Painter.fillRect( 0, 0, Im.width(), Im.height(), BackgroundColor );
        Painter.translate( dx, dy );
        Painter.rotate( 270.0);
        Painter.drawText( 0, 0, sLabel );
        int ix = m_X0 + k - dx / 2;
        Canvas.drawImage( m_Start.X + ix, m_Start.Y + m_Y0 + 3, Im, 0, 0, Im.width(), Im.height() );
        }
      }
    }
  else
    {
    for( int i = 0; i + iX0 <= Round( m_MaxX ); i = i + iStep )
      {
      int k = Round( i / m_Dx * m_FullSize );
      m_pOwner->Line( m_Start.X + m_X0 + k, m_Start.Y + m_Y0, m_Start.X + m_X0 + k, m_Start.Y + m_Y0 - 5 );
      if( i != 0 )
        {
        QByteArray sLabel = QByteArray::number( i + iX0 );
        int ix = m_X0 + k - m_pOwner->TextWidth( sLabel ) / 2;
        m_pOwner->TextOut( m_Start.X + ix, m_Start.Y + m_Y0 + HarHeight + 3, sLabel );
        }
      }
    }

  m_pOwner->Line( m_Start.X + m_X0, m_Start.Y, m_Start.X + m_X0, m_Start.Y + m_Size.height() );
  m_pOwner->Line( m_Start.X + m_X0, m_Start.Y, m_Start.X + m_X0 - 5, m_Start.Y + 10 );
  m_pOwner->Line( m_Start.X + m_X0, m_Start.Y, m_Start.X + m_X0 + 5, m_Start.Y + 10 );

  int iMaxY = Round( m_MaxY );
  iStep = CalculateStep( iMaxY, 2 );
  int iLen = QByteArray::number( iMaxY ).length();
  for( int i = 0; i <= iMaxY; i = i + iStep )
    {
    int k = m_Y0 - Round( i / m_MaxY * m_FullSize );
    if( k < m_Size.height() && k > 0 )
      {
      m_pOwner->Line( m_Start.X + m_X0, m_Start.Y + k, m_Start.X + m_X0 + 5, m_Start.Y + k );
      m_pOwner->TextOut( m_Start.X + 1, m_Start.Y + k + HarHeight + 2, QString( "%1" ).arg( i, iLen ) );
      }
    }

  for( int i = 0; i < m_Y.count(); i++ )
    {
    double Value = GetX( i );
    int ix = Round( ( Value - iX0 ) / m_Dx * m_FullSize ) + m_X0;
    int h = Round( m_Y[i] / m_MaxY * m_FullSize );
    iy = m_Y0 - h;
    m_pOwner->SetBrushColor( m_Colors[i] );
    m_pOwner->SetPaintColor( m_Colors[i] );
    if( m_Type == ThickCol )
      {
      int iXNext = Round( ( GetX( i, true ) - iX0 ) / m_Dx * m_FullSize ) + m_X0;
      m_pOwner->m_pCanvas->drawRect( m_Start.X + ix, m_Start.Y + iy, iXNext - ix, h );
      PutLabel( ( ix + iXNext ) / 2, i, iy );
      }
    if( m_Type == ThinCol )
      {
      m_pOwner->m_pCanvas->drawRect( m_Start.X + ix - HalfCol, m_Start.Y + iy, HalfCol * 2, h );
      m_pOwner->Line( m_Start.X + ix - HalfShelf, m_Start.Y + iy, m_Start.X + ix + HalfShelf, m_Start.Y + iy );
      PutLabel( ix, i, iy );
      }
    }
  m_pOwner->RestoreFont();
  m_pOwner->RestoreBrushColor();
  m_pOwner->RestorePaintColor();
  }

EdList* EdTable::GetCell()
  {
  m_pCurrentCell = m_Body[m_Row][m_Col];
  if( ++m_Col < m_ColCount ) return m_pCurrentCell;
  m_Col = 0;
  if( ++m_Row == m_RowCount ) m_Row = 0;
  return m_pCurrentCell;
  }

bool EdTable::MoveToNext( EdList*& L )
  {
  m_pOwner->m_pCurrentTable = this;
  if( m_pCurrentCell->m_pSub_L->m_pMother != nullptr &&
    m_pCurrentCell->m_pSub_L->m_pMother != m_pCurrentCell->m_pMother )
    {
    L->MoveRight( L );
    return true;
    }
  int iNewCol = m_Col + 1;
  int iNewRow = m_Row;
  while( iNewRow < m_RowCount && iNewRow >= 0 )
    {
    for( ; iNewCol < m_ColCount && m_FrozenCells[iNewRow][iNewCol] && m_GridState != TGRUnvisible; iNewCol++ );
    if( iNewCol < m_ColCount ) break;
    if( XPInEdit::sm_Language == lngHebrew )
      iNewRow--;
    else
      iNewRow++;
    iNewCol = 0;
    }
  if( iNewRow == m_RowCount || iNewRow == -1 ) return false;
  m_Row = iNewRow;
  m_Col = iNewCol;
  L = m_Body[m_Row][m_Col]->m_pSub_L;
  L->m_pCurr = L->m_pFirst;
  return true;
  }

bool EdTable::MoveToPrev( EdList*& L )
  {
  int iNewCol = m_Col - 1;
  int iNewRow = m_Row;
  while( iNewRow < m_RowCount && iNewRow >= 0 )
    {
    for( ; iNewCol >= 0 && m_FrozenCells[iNewRow][iNewCol] && m_GridState != TGRUnvisible; iNewCol-- );
    if( iNewCol >= 0 ) break;
    if( XPInEdit::sm_Language == lngHebrew )
      iNewRow++;
    else
      iNewRow--;
    iNewCol = m_ColCount - 1;
    }
  m_pOwner->m_pCurrentTable = nullptr;
  if( iNewRow == m_RowCount || iNewRow == -1 ) return false;
  m_Row = iNewRow;
  m_Col = iNewCol;
  L = m_Body[m_Row][m_Col]->m_pSub_L;
  L->m_pCurr = L->m_pFirst;
  m_pOwner->m_pCurrentTable = this;
  return true;
  }

bool EdTable::MoveInRight( EdList*& L )
  {
  int iNewRow = 0, iNewCol = 0;
  while( iNewRow < m_RowCount )
    {
    while( iNewCol < m_ColCount )
      {
      if( !m_FrozenCells[iNewRow][iNewCol] ) break;
      iNewCol++;
      }
    if( iNewCol < m_ColCount ) break;
    iNewRow++;
    iNewCol = 0;
    }
  if( iNewRow == m_RowCount ) return false;
  m_Row = iNewRow;
  m_Col = iNewCol;
  L = m_Body[m_Row][m_Col]->m_pSub_L;
  L->m_pCurr = L->m_pFirst;
  m_pOwner->m_pCurrentTable = this;
  return true;
  }

bool EdTable::MoveInLeft( EdList*& L )
  {
  int iNewCol = m_ColCount - 1, iNewRow = m_RowCount - 1;
  while( iNewRow >= 0 )
    {
    while( iNewCol >= 0 )
      {
      if( !m_FrozenCells[iNewRow][iNewCol] ) break;
      iNewCol--;
      }
    if( iNewCol >= 0 ) break;
    iNewRow--;
    iNewCol = m_ColCount - 1;
    }
  if( iNewRow == -1 ) return false;
  m_Row = iNewRow;
  m_Col = iNewCol;
  L = m_Body[m_Row][m_Col]->m_pSub_L;
  L->m_pCurr = L->m_pFirst;
  m_pOwner->m_pCurrentTable = this;
  return true;
  }

bool EdTable::MoveToUp( EdList*& L )
  {
  int iNewRow = m_Row - 1;
  int iNewCol = m_Col;
  while( iNewRow >= 0 )
    {
    if( m_FrozenCells[iNewRow][iNewCol] && iNewRow == m_Row - 1 )
      iNewCol = m_ColCount - 1;
    while( iNewCol >= 0 )
      {
      if( !m_FrozenCells[iNewRow][iNewCol] ) break;
      iNewCol--;
      }
    if( iNewCol >= 0 ) break;
    iNewRow--;
    iNewCol = m_ColCount - 1;
    }
  if( iNewRow == -1 ) return false;
  m_Row = iNewRow;
  m_Col = iNewCol;
  L = m_Body[m_Row][m_Col]->m_pSub_L;
  L->m_pCurr = L->m_pFirst;
  return true;
  }

bool EdTable::MoveToDown( EdList*& L )
  {
  int iNewRow = m_Row + 1;
  int iNewCol = m_Col;
  while( iNewRow < m_RowCount )
    {
    if( m_FrozenCells[iNewRow][iNewCol] && iNewRow == m_Row + 1 )
      iNewCol = 0;
    while( iNewCol < m_ColCount )
      {
      if( !m_FrozenCells[iNewRow][iNewCol] ) break;
      iNewCol = iNewCol + 1;
      }
    if( iNewCol < m_ColCount ) break;
    iNewRow++;
    iNewCol = 0;
    }
  if( iNewRow == m_RowCount ) return false;
  m_Row = iNewRow;
  m_Col = iNewCol;
  L = m_Body[m_Row][m_Col]->m_pSub_L;
  L->m_pCurr = L->m_pFirst;
  return true;
  }

EdTable::~EdTable()
  {
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      delete m_Body[i][j];
  if( m_pOwner->m_pCurrentTable == this ) m_pOwner->m_pCurrentTable = nullptr;
  }

void EdTable::SetAppending( EdMemb& appending )
  {
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = 0; j < m_ColCount; j++ )
      m_Body[i][j]->m_pSub_L->m_pMother = m_Body[i][j]->m_pMother = &appending;
  m_pOwnerList = m_pOwner->m_pL;
  m_pCurrentCell = m_Body[0][0];
  m_pOwner->m_pL = m_Body[0][0];
  }

void EdTable::DelRow()
  {
  for( int j = 0; j < m_ColCount; j++ )
    delete m_Body[m_Row][j];
  for( int i = m_Row + 1; i < m_RowCount; i++ )
    {
    m_SizeRows[i - 1] = m_SizeRows[i];
    for( int j = 0; j < m_ColCount; j++ )
      {
      m_Body[i - 1][j] = m_Body[i][j];
      m_FrozenCells[i - 1][j] = m_FrozenCells[i][j];
      }
    }
  m_RowCount--;
  if( m_Row == m_RowCount ) m_Row--;
  m_pOwner->m_pL->m_pSub_L = m_Body[m_Row][m_Col];
  }

bool EdTable::CanCellEmpty()
  {
  return !m_FrozenCells[m_Row][m_Col];
  }

void EdTable::DelColumn()
  {
  if( ( m_GridState == TGRUnvisible ) && m_FrozenCells[m_Row][m_Col] ) return;
  for( int i = 0; i < m_RowCount; i++ )
    for( int j = m_Col + 1; j < m_ColCount; j++ )
      {
      m_SizeCols[j - 1] = m_SizeCols[j];
      for( int i = 0; i < m_RowCount; i++ )
        {
        m_Body[i][j - 1] = m_Body[i][j];
        m_FrozenCells[i][j - 1] = m_FrozenCells[i][j];
        }
      }
  m_ColCount--;
  if( m_Col == m_ColCount ) m_Col--;
  int i = m_Col;
  while( i < m_ColCount && m_FrozenCells[m_Row][i] ) i++;
  if( i < m_ColCount ) m_Col = i;
  m_pOwner->m_pL->m_pSub_L = m_Body[m_Row][m_Col];
  }

void EdTable::InsertRow()
  {
  m_Body.resize( m_RowCount + 1 );
  m_Body[m_RowCount++].resize( m_ColCount );
  m_SizeRows.resize( m_RowCount );
  m_FrozenCells.resize( m_RowCount );
  m_FrozenCells[m_RowCount - 1].resize( m_ColCount );
  EdMemb *Appending = m_Body[0][0]->m_pMother;
  int k = m_RowCount - 1;
  for( int i = m_Row; i < m_RowCount - 1; i++, k-- )
    {
    m_SizeRows[k] = m_SizeRows[k - 1];
    for( int j = 0; j < m_ColCount; j++ )
      {
      m_Body[k][j] = m_Body[k - 1][j];
      m_FrozenCells[k][j] = m_FrozenCells[k - 1][j];
      }
    }
  for( int j = 0; j < m_ColCount; j++ )
    {
    m_Body[m_Row][j] = new EdList( m_pOwner );
    m_Body[m_Row][j]->m_pMother = Appending;
    m_FrozenCells[m_Row][j] = false;
    }
  m_pOwner->m_pL->m_pSub_L = m_Body[m_Row][m_Col];
  }

void EdTable::InsertColumn()
  {
  int k = m_ColCount;
  m_ColCount++;
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_Body[i].resize( m_ColCount );
    m_FrozenCells[i].resize( m_ColCount );
    }
  m_SizeCols.resize( m_ColCount );
  m_UnvisibleColumns.resize( m_ColCount );
  m_UnvisibleColumns[k] = false;
  EdMemb *Appending = m_Body[0][0]->m_pMother;
  for( int j = ++m_Col; j < m_ColCount - 1; j++, k-- )
    {
    m_SizeCols[k] = m_SizeCols[k - 1];
    for( int i = 0; i < m_RowCount; i++ )
      {
      m_Body[i][k] = m_Body[i][k - 1];
      m_FrozenCells[i][k] = m_FrozenCells[i][k - 1];
      }
    }
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_Body[i][m_Col] = new EdList( m_pOwner );
    m_Body[i][m_Col]->m_pMother = Appending;
    m_FrozenCells[i][m_Col] = false;
    }
  m_pOwner->m_pL->m_pSub_L = m_Body[m_Row][m_Col];
  }

void EdTable::AddRow()
  {
  m_RowCount++;
  m_Body.resize( m_RowCount );
  m_Body.back().resize( m_ColCount );
  m_SizeRows.resize( m_RowCount );
  m_FrozenCells.resize( m_RowCount );
  m_FrozenCells.back().resize( m_ColCount );
  EdMemb *Appending = m_Body[0][0]->m_pMother;
  m_Row = m_RowCount - 1;
  for( int j = 0; j < m_ColCount; j++ )
    {
    m_Body[m_Row][j] = new EdList( m_pOwner );
    m_Body[m_Row][j]->m_pMother = Appending;
    m_FrozenCells[m_Row][j] = false;
    }
  m_pOwner->m_pL->m_pSub_L = m_Body[m_Row][m_Col];
  }

void EdTable::AddColumn()
  {
  m_Col = m_ColCount;
  m_ColCount++;
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_Body[i].resize( m_ColCount );
    m_FrozenCells[i].resize( m_ColCount );
    }
  m_SizeCols.resize( m_ColCount );
  m_UnvisibleColumns.resize( m_ColCount );
  m_UnvisibleColumns[m_Col] = false;
  EdMemb *Appending = m_Body[0][0]->m_pMother;
  for( int i = 0; i < m_RowCount; i++ )
    {
    m_Body[i][m_Col] = new EdList( m_pOwner );
    m_Body[i][m_Col]->m_pMother = Appending;
    m_FrozenCells[i][m_Col] = false;
    }
  m_pOwner->m_pL->m_pSub_L = m_Body[m_Row][m_Col];
  }

void EdTable::Freeze()
  {
  if( sm_pTemplate.IsEmpty() )
    {
    if( m_UnvisibleColumns[0] )
      for( int i = 0; i < m_RowCount; i++ )
        m_FrozenCells[i][2] = true;
    else
      for( int i = 0; i < m_RowCount; i++ )
        for( int j = 0; j < m_ColCount; j++ )
          m_FrozenCells[i][j] = sm_WasTemplate && m_Body[i][j]->m_pFirst != nullptr;
    }
  else
    {
    for( int i = 0; i < m_RowCount; i++ )
      {
      EdTable *pTmpTable = dynamic_cast<EdTable*>(&*sm_pTemplate);
      int ColCount = min(m_ColCount, pTmpTable->m_ColCount);
      for( int j = 0; j < ColCount; j++ )
        m_FrozenCells[i][j] = pTmpTable->m_FrozenCells[i][j];
      }
    sm_pTemplate.clear();
    }
  m_Row = 0;
  while( m_Row < m_RowCount )
    {
    if( XPInEdit::sm_Language == lngHebrew )
      {
      m_Col = m_ColCount - 1;
      while( m_Col >= 0 )
        {
        if( !m_FrozenCells[m_Row][m_Col] ) return;
        m_Col--;
        }
      }
    else
      {
      m_Col = 0;
      while( m_Col < m_ColCount )
        {
        if( !m_FrozenCells[m_Row][m_Col] && !m_UnvisibleColumns[m_Col] ) return;
        m_Col++;
        }
      }
    m_Row++;
    }
  m_Col = 0;
  m_Row = 0;
  }

EdChartType::EdChartType( EdChartEditor *pEd ) : EdElm( pEd->m_pOwner ), m_pEditor( pEd ), m_Type( pEd->ChartType() )
  {
  m_Pictures.append( QPixmap( QString( ":/Resources/ThinChart.png" ) ) );
  m_Pictures.append( QPixmap( QString( ":/Resources/ThickChart.png" ) ) );
  }

void EdChartType::PreCalc( TPoint P, QSize& S, int& A )
  {
  m_Start = P;
  m_Size.setWidth( m_Pictures[m_Type].width() + 6 );
  m_Size.setHeight( m_Pictures[m_Type].height() + 6 );
  m_Axis = 0;
  S = m_Size;
  A = m_Axis;
  }

void EdChartType::Draw( TPoint P )
  {
  m_pOwner->m_pCanvas->fillRect( P.X, P.Y, m_Size.width(), m_Size.height(), Qt::white );
  m_pOwner->m_pCanvas->drawPixmap( P.X + 3, P.Y + 3, m_Pictures[m_Type] );
  }

QByteArray EdChartType::Write()
  {
  return m_Type == ThinCol ? "ThinCol" : "ThickCol";
  }

void EdChartType::ChangeType()
  {
  CreateChartDialog Dlg( m_pEditor );
  if( Dlg.exec() == QDialog::Rejected || !Dlg.m_WasChanged ) return;
  m_Type = m_pEditor->ChartType();
  m_pEditor->m_pOwner->ClearSurface();
  m_pEditor->m_pOwner->EditDraw();
  }

EdColumnColor::EdColumnColor(QColor Color, EdChartEditor *pEd) : EdElm(pEd->m_pOwner), m_Color(Color), m_WasChanged(false), m_pEditor(pEd)
  {}

void EdColumnColor::PreCalc( TPoint P, QSize& S, int& A )
  {
  m_Start = P;
  m_Size.setWidth( 0 );
  EdList *pCell = m_pEditor->m_Body[0][0];
  m_Size.setHeight( pCell->m_Size.height() );
  m_Axis = pCell->m_Axis;
  S = m_Size;
  A = m_Axis;
  }

void EdColumnColor::Draw( TPoint P )
  {
  FloodFill( QRect( P.X - 3, P.Y - 3, 100, 100 ),
    dynamic_cast< QImage* >( m_pOwner->m_pCanvas->device() ), QPoint( P.X, P.Y ), QColor( m_Color ) );
  }

QByteArray EdColumnColor::Write()
  {
  return "0X" + QByteArray::number( m_Color.rgb(), 16 ).mid(2);
  }

void EdColumnColor::ChangeColor()
  {
  m_WasChanged = !m_WasChanged;
  if( !m_WasChanged ) return;
  QColor C = QColorDialog::getColor( m_Color );
  if( !C.isValid() ) return;
  m_Color = C;
  Draw( m_Start );
  }

void EdChartEditor::CreateBody()
  {
  m_IsTEdChartEditor = true;
  m_GridState = TGRVisible;
  int iFirstRow = 0;
  m_pParentMemb = m_pOwner->m_pL->m_pSub_L->Append_Before( this );
  SetAppending( *m_pParentMemb );
  m_Row = m_RowCount - 2;
  m_Body[m_Row][0]->m_pSub_L->Append_Before( m_pTemplChart->m_pNameX );
  m_Body[m_Row][0]->m_pSub_L->Append_Before( new EdChar( ':', m_pOwner ) );
  m_Body[m_Row][0]->m_pSub_L->Append_Before( m_pTemplChart->m_pLabelX );
  m_Body[m_Row + 1][0]->m_pSub_L->Append_Before( m_pTemplChart->m_pNameY );
  m_Body[m_Row + 1][0]->m_pSub_L->Append_Before( new EdChar( ':', m_pOwner ) );
  m_Body[m_Row + 1][0]->m_pSub_L->Append_Before( m_pTemplChart->m_pLabelY );
  for( int j = 1; j < m_ColCount; j++ )
    {
    QByteArray &Labels = m_pTemplChart->m_Labels[j - 1];
    for( int k = 0; k < Labels.count(); m_Body[m_Row][j]->m_pSub_L->Append_Before( new EdChar( Labels[k++], m_pOwner ) ) );
    }
  if( !m_WasTemplate )
    {
    m_Body[0][0]->m_pSub_L->Append_Before( new EdChartType( this ) );
    for( int j = 1; j < m_ColCount; j++ )
      {
      m_Body[0][j]->m_pSub_L->Append_Before( new EdColumnColor( m_pTemplChart->m_Colors[j - 1], this ) );
      const QByteArray &Values = QByteArray::number( m_pTemplChart->m_Y[j - 1] );
      for( int k = 0; k < Values.count(); m_Body[2][j]->m_pSub_L->Append_Before( new EdChar( Values[k++], m_pOwner ) ) );
      m_FrozenCells[0][j] = true;
      }
    m_FrozenCells[0][0] = true;
    }
  else
    Freeze();
  m_Col = 1;
  m_Row = m_RowCount - 1;
  m_pOwnerList->m_pSub_L = m_Body[m_Row][m_Col]->m_pSub_L;
  }

EdChartEditor::EdChartEditor( XPInEdit *pOwn, EdChart *pChart ) : EdTable( pOwn, ( sm_WasTemplate ? "2," : "3," ) +
  QByteArray::number( pChart->m_Labels.count() + 1 ) + ",1,0,0" ), m_pTemplChart( pChart )
  {
  CreateBody();
  }

EdChartEditor::EdChartEditor( XPInEdit *pOwn ) : EdTable( pOwn, "3,3,1,0,0" ), m_pTemplChart( new EdChart( pOwn ) )
  {
  CreateBody();
  }

EdChartEditor::~EdChartEditor()
  {
  delete m_pTemplChart;
  }

void EdChartEditor::SetColumnNumber( int NewNumber )
  {
  m_pTemplChart->SetColumnNumber( NewNumber++ );
  if( NewNumber == m_ColCount ) return;
  int OldColCount = m_ColCount;
  EdTable::SetColumnNumber( NewNumber );
  if( OldColCount > m_ColCount ) return;
  SetAppending( *m_pParentMemb );
  for( int j = OldColCount; j < m_ColCount; j++ )
    {
    m_Body[0][j]->m_pSub_L->Append_Before( new EdColumnColor( m_pTemplChart->m_Colors[j - 1], this ) );
    QByteArray &Labels = m_pTemplChart->m_Labels[j - 1];
    for( int k = 0; k < Labels.count(); m_Body[1][j]->m_pSub_L->Append_Before( new EdChar( Labels[k++], m_pOwner ) ) );
    const QByteArray &Values = QByteArray::number( m_pTemplChart->m_Y[j - 1] );
    for( int k = 0; k < Values.count(); m_Body[2][j]->m_pSub_L->Append_Before( new EdChar( Values[k++], m_pOwner ) ) );
    m_FrozenCells[0][j] = true;
    }
  }

bool EdChartEditor::SetCurrent( const TPoint& C, EdList *& SL, EdMemb *& Cr )
  {
  int y = m_Start.Y + m_BorderSize;
  if( m_RowCount == 3 && C.Y >= y && C.Y <= y + m_SizeRows[0] )
    {
    for( int k = 0, x = m_Start.X + m_BorderSize; k < m_ColCount; x += m_SizeCols[k++] )
      {
      if( C.X >= x && C.X <= x + m_SizeCols[k] )
        {
        if( k > 0 )
          dynamic_cast< EdColumnColor* >( m_Body[0][k]->m_pSub_L->m_pFirst->m_pMember.data() )->ChangeColor();
        else
          dynamic_cast< EdChartType* >( m_Body[0][0]->m_pSub_L->m_pFirst->m_pMember.data() )->ChangeType();
        SL = m_Body[1][k];
        Cr = SL->m_pCurr;
        m_Row = 1;
        m_Col = k;
        return false;
        }
      }
    }
  EdTable::SetCurrent( C, SL, Cr );
  return true;
  }

bool EdChartEditor::ResetChart()
  {
  bool Ok;
  if( m_RowCount == 2 )
    {
    for( int j = 1; j < m_ColCount; j++ )
      {
      m_pTemplChart->m_Y[j - 1] = m_Body[1][j]->m_pSub_L->Write().toInt( &Ok );
      if( !Ok ) return false;
      }
    return true;
    }
  if( m_pTemplChart->m_Type == ThinCol )
    for( int j = 1; j < m_ColCount; j++ )
      {
      m_pTemplChart->m_Labels[j - 1] = m_Body[1][j]->m_pSub_L->Write();
      m_pTemplChart->m_Labels[j - 1].toInt( &Ok );
      if( !Ok ) return false;
      }
  else
    for( int j = 1; j < m_ColCount; j++ )
      {
      QByteArray Label( m_Body[1][j]->m_pSub_L->Write() );
      int iPos = Label.indexOf( '-' );
      if( iPos < 1 || iPos == Label.length() - 1 ) return false;
      Label.left( iPos ).toInt( &Ok );
      if( !Ok ) return false;
      Label.mid( iPos ).toInt( &Ok );
      if( !Ok ) return false;
      m_pTemplChart->m_Labels[j - 1] = Label;
      }
  for( int j = 1; j < m_ColCount; j++ )
    {
    m_pTemplChart->m_Y[j - 1] = m_Body[2][j]->m_pSub_L->Write().toInt( &Ok );
    if( !Ok ) return false;
    m_pTemplChart->m_Colors[j - 1] = dynamic_cast< EdColumnColor* >( m_Body[0][j]->m_pSub_L->m_pFirst->m_pMember.data() )->m_Color;
    }
  QByteArray LabName = m_Body[1][0]->m_pSub_L->Write().replace('"', "" );
  int iColon = LabName.indexOf( ':' );
  if( iColon == -1 ) return false;
  m_pTemplChart->m_pNameX = new EdStr( m_pOwner, LabName.left(iColon) );
  m_pTemplChart->m_pLabelX = new EdStr( m_pOwner, LabName.mid( iColon + 1 ) );
  LabName = m_Body[2][0]->m_pSub_L->Write().replace( '"', "" );;
  iColon = LabName.indexOf( ':' );
  if( iColon == -1 ) return false;
  m_pTemplChart->m_pNameY = new EdStr( m_pOwner, LabName.left( iColon ) );
  m_pTemplChart->m_pLabelY = new EdStr( m_pOwner, LabName.mid( iColon + 1 ) );
  return true;
  }

QByteArray EdChartEditor::Write()
  {
  if( ResetChart() ) return m_pTemplChart->Write();
  return "";
  }

QByteArray EdChartEditor::SWrite()
  {
  if( ResetChart() ) return m_pTemplChart->SWrite();
  return "";
  }

bool EdChartEditor::MoveToUp( EdList*& L )
  {
  if( m_Row == 1 && m_Col == m_ColCount - 1 ) return false;
  return EdTable::MoveToUp( L );
  }

bool EdChartEditor::MoveToDown( EdList*& L )
  {
  if( m_Row == 1 && m_Col == m_ColCount - 1 ) return false;
  return EdTable::MoveToDown( L );
  }

EdSpace::EdSpace( bool F, EdSpace* pN, EdSpace* pR, XPInEdit *pOwn ) : EdElm( pOwn ), m_First( F ), m_Last( false ),
  m_pNext( pN ), m_pRight( pR )
  {
  m_Size.setWidth( 0 );
  m_Size.setHeight( m_pOwner->CharHeight( 'W' ) );
  m_Axis = m_Size.height() / 2;
  }

void EdSpace::PreCalc( TPoint P, QSize &S, int &A )
  {
  if( !m_First || !m_pOwner->m_RecalcSpaces )
    {
    m_Start = P;
    S = m_Size;
    A = m_Axis;
    if( m_Last && m_pOwner->m_RecalcSpaces )
      m_pOwner->m_RecalcSpaces = false;
    return;
    }
  EdSpace *pElem = this;
  int MaxColWeight = 0;
  while( pElem != NULL )
    {
    MaxColWeight = max( MaxColWeight, pElem->m_pRight->m_Start.X - pElem->m_Start.X - pElem->m_Size.width() );
    pElem = pElem->m_pNext;
    }
  pElem = this;
  while( pElem != NULL )
    {
    pElem->m_Size.setWidth( MaxColWeight - pElem->m_pRight->m_Start.X + pElem->m_Start.X + pElem->m_Size.width() );
    pElem = pElem->m_pNext;
    }
  m_Start = P;
  S = m_Size;
  A = m_Axis;
  }

EdPicture::EdPicture(XPInEdit *pOwn, const QByteArray& Path) : EdElm(pOwn), m_Path(Path)
  {
  QString FullPath = XPInEdit::sm_BasePath + '/' + Path;
  if (QFile::exists(FullPath))
    {
    m_Picture = QPixmap(FullPath);
    m_Size.setWidth(m_Picture.width());
    m_Size.setHeight(m_Picture.height());
    }
  else
    {
    m_NotFound = "Picture not found";
    m_pOwner->SetFont(m_pOwner->m_ViewSettings.m_TaskCmFont);
    QFontMetrics FM = m_pOwner->m_pCanvas->fontMetrics();
    m_Size = FM.boundingRect(m_NotFound).size();
    }
  m_Axis = m_Size.height() / 2;
  }

void EdPicture::PreCalc(TPoint P, QSize& S, int& A)
  {
  m_Start = P;
  S = m_Size;
  A = m_Axis;
  }

void EdPicture::Draw(TPoint P)
  {
  SelectRect();
  if(m_NotFound.isEmpty())
    if (m_Selected)
      {
      QPixmap PM(m_Picture);
      PM.setMask(PM.createHeuristicMask());
      m_pOwner->m_pCanvas->drawPixmap(P.X, P.Y, PM);
      }
    else
      m_pOwner->m_pCanvas->drawPixmap(P.X, P.Y, m_Picture);
  else
    m_pOwner->TextOut(m_Start.X, m_Start.Y + m_Size.height(), m_NotFound);
  }

QByteArray EdPicture::Write()
  {
  return "Picture(\"" + m_Path + "\")";
  }

QByteArray EdPicture::SWrite()
  {
  return "\\picture{" + m_Path + '}';
  }

struct OneArgSubst
  {
  QByteArray m_Name;
  QByteArray m_Value;
  OneArgSubst( QByteArray Name, QByteArray Value ) : m_Name( Name ), m_Value( Value ) {}
  };

OneArgSubst OneArgs[] = {
  OneArgSubst( "exp", "EXP" ),
  OneArgSubst( "lg", "LG" ),
  OneArgSubst( "ln", "LN" ),
  OneArgSubst( "sin", "SIN" ),
  OneArgSubst( "cos", "COS" ),
  OneArgSubst( "tan", "TAN" ),
  OneArgSubst( "cot", "COT" ),
  OneArgSubst( "arcsin", "ARCSIN" ),
  OneArgSubst( "arccos", "ARCCOS" ),
  OneArgSubst( "arctan", "ARCTAN" ),
  OneArgSubst( "arccot", "ARCCOT" )
  };
int NOneArgs = sizeof( OneArgs ) / sizeof( OneArgSubst );

class StringStream : public QByteArray
  {
  int m_Pos;
  public:
    StringStream( const QByteArray &String ) : QByteArray( String ), m_Pos( 0 ) {}
    StringStream& operator >> ( char& Res )
      {
      if( m_Pos >= length() )
        Res = 0;
      else
        Res = at( m_Pos++ );
      return *this;
      }
    void Skip() { m_Pos++; }
  };

QByteArray  Convert( StringStream& Stream );

QByteArray  Convert( const QByteArray &Formula )
  {
  StringStream SS( Formula );
  return Convert( SS );
  }

QByteArray  Convert( StringStream& Stream )
  {
  QByteArray sResult, sCommand;
  char sChar;
  int iState = 0;
  bool bDone = false, bRead = true, bContinue;
  static bool bWasNewLine = false;
  static bool bText = false;
  int I;
  while( !bDone )
    {
    if( bRead ) Stream >> sChar;
    bDone = sChar == 0 || sChar == '}';
    if( bDone && bText )
      {
      bText = false;
      sResult += '\n';
      }
    if( iState == 0 )
      {
      if( sChar == '{' )
        {
        sResult += Convert( Stream );
        if( sCommand.isEmpty() ) return sResult;
        iState = 0;
        sCommand.clear();
        bRead = true;
        continue;
        }
      if( sChar != '\\' && sChar != '}' && sChar != 0 )
        {
        sResult += sChar;
        if( !bText ) sResult += '\n';
        bRead = true;
        continue;
        }
      if( sChar == '\\' )
        {
        iState = 1; // '\'
        sCommand.clear();
        bRead = true;
        continue;
        }
      }
    else
      if( iState == 1 )
        {
        if( sChar >= 'A' && sChar <= 'Z' || sChar >= 'a' && sChar <= 'z' || sChar == '_')
          {
          sCommand += sChar;
          bRead = true;
          continue;
          }
        else
          {
          if( sCommand.isEmpty() )
            {
            iState = 0;
            sResult = sResult + sChar + '\n';
            bRead = true;
            continue;
            }
          char Code = TexNames::Symbols[sCommand];
          if( Code != 0 )
            {
            iState = 0;
            sResult = sResult + Code + '\n';
            bRead = false;
            continue;
            }
          if( sCommand == "newline" )
            {
            iState = 0;
            if (bText)
              {
              sResult += msCharNewLine;
              continue;
              }
            sResult += "NEWLINE\n";
            bRead = true;
            bWasNewLine = true;
            continue;
            };

          if( sCommand == "subscript" )
            {
            iState = 0;
            sResult += "subscript\n"; // 0xE4
            bRead = false;
            bText = true;
            continue;
            }

          if( sCommand == "mean" )
            {
            iState = 0;
            sResult += "mean\n"; // 0xE4
            bRead = false;
            bText = true;
            continue;
            }

          if( sCommand == "superscript" )
            {
            iState = 0;
            sResult += "superscript\n"; // 0xE4
            bRead = false;
            bText = true;
            continue;
            }

          if( sCommand == "comment" )
            {
            iState = 0;
            sResult += "TEXT\n";
            bRead = false;
            bText = true;
            continue;
            }

          if (sCommand == "picture")
            {
            iState = 0;
            sResult += "PICTURE\n";
            bRead = false;
            bText = true;
            continue;
            }

          if( sCommand == "frac" )
            {
            sResult += "FRACTION\n";
            QByteArray Value( Convert( Stream ) );
            sResult += Value;
            sResult += "vk_Down\n";
            Stream.Skip();
            Value = Convert( Stream );
            sResult += Value;
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "sqrt" )
            {
            sResult += "SQROOT\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "power" )
            {
            sResult += "POWER\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "root" )
            {
            sResult += "ROOT\n";
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "index" )
            {
            sResult += "INDEX\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "log" )
            {
            sResult += "LOG\n";
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "int" )
            {
            sResult += "INTEGRAL\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "dint" )
            {
            sResult += "DEFINTEGRAL\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "dblint" )
            {
            sResult += "DBLINTEGRAL\n";
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "trplint" )
            {
            sResult += "TRPLINTEGRAL\n";
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "crvint" )
            {
            sResult += "CRVINTEGRAL\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "subst" )
            {
            sResult += "SUBST\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "interval" )
            {
            sResult += "INTERVAL\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "abs" )
            {
            sResult += "ABS\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "list" )
            {
            Stream.Skip();
            do
              {
              bWasNewLine = false;
              sResult += Convert( Stream );
              Stream >> sChar;
              if( sChar == '}' ) break;
              if( !bWasNewLine )
                sResult += ",\n \n";
              } while( true );
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "func" )
            {
            sResult += "FUNC\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "matrix" )
            {
            sResult += "MATRIX\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "matrix_unv" )
            {
            sResult += "MATRIX_UNV\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "setvisible" )
            {
            iState = 0;
            sResult += "SETVISIBLE\n";
            sCommand.clear();
            bRead = true;
            continue;
            };

          if( sCommand == "setunvisible" )
            {
            iState = 0;
            sResult += "SETUNVISIBLE\n";
            sCommand.clear();
            bRead = true;
            continue;
            };

          if( sCommand == "invisible" )
            {
            iState = 0;
            sResult += msInvisible;
            sCommand.clear();
            bRead = true;
            continue;
            };

          if( sCommand == "spacer" )
            {
            iState = 0;
            sResult += "SPACER\n";
            sCommand.clear();
            bRead = true;
            continue;
            };

          if( sCommand == "table" )
            {
            sResult += "TABLE\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "cell" )
            {
            iState = 0;
            sResult += "CELL\n";
            sResult += Convert( Stream );
            sCommand.clear();
            bRead = true;
            continue;
            };

          if( sCommand == "endtable" )
            {
            iState = 0;
            sResult += "ENDTABLE\n";
            sCommand.clear();
            bRead = true;
            continue;
            };

          if( sCommand == "chart" )
            {
            sResult += "CHART\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "bcoeff" )
            {
            sResult += "BinomCoeff\n";
            sResult += "vk_Up\n";
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "acoeff" )
            {
            sResult += "ABinomCoeff\n";
            sResult += "vk_Up\n";
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "units" )
            {
            sResult += "MEASURED\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "trigocomp" )
            {
            sResult += "TRIGOCOMP\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sResult += Convert( Stream );
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "summa" )
            {
            sResult += "GSUMMA\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "lim" )
            {
            sResult += "LIMIT\n";
            sResult += "vk_Down\n";
            sResult += "vk_Left\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "product" )
            {
            sResult += "GMULT\n";
            sResult += "vk_Back\n";
            sResult += "vk_Delete\n";
            sResult += "vk_Down\n";
            sResult += Convert( Stream );
            sResult += "vk_Up\n";
            sResult += "vk_Up\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "der" )
            {
            sResult += "DERIVATIVE\n";
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "prtder" )
            {
            sResult += "PARTDERIV\n";
            sResult += Convert( Stream );
            sResult += "vk_Down\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "strkder" )
            {
            sResult += "STROKEDERIV\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "strkprtder" )
            {
            sResult += "STROKEPARTDERIV\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            Stream.Skip();
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "system" )
            {
            sResult += "SYSTEM\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "percount" )
            {
            sResult += "PerCount\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "im" )
            {
            sResult += "IMUNIT\n";
            if( sChar != 0 && sChar != '}' && sChar != '\\' ) sResult = sResult + sChar + '\n';
            iState = 0;
            sCommand.clear();
            bRead = sChar != '\\';
            continue;
            }

          if( sCommand == "vector" )
            {
            sResult += "VECTOR\n";
            sResult += Convert( Stream );
            sResult += "vk_Right\n";
            iState = 0;
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "intervals" )
            {
            sResult += "INTERVALS\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sCommand.clear();
            bRead = true;
            continue;
            }

          if( sCommand == "polygon" )
            {
            sResult += "POLYGON\n";
            bText = true;
            iState = 0;
            sResult += Convert( Stream );
            sCommand.clear();
            bRead = true;
            continue;
            }

          // Graph
          // Search for one arg function.
          bContinue = false;
          for( I = 0; I < NOneArgs; I++ )
            {
            if( sCommand == OneArgs[I].m_Name )
              {
              sResult = sResult + OneArgs[I].m_Value + '\n';
              sResult += Convert( Stream );
              sResult += "vk_Right\n";
              iState = 0;
              sCommand.clear();
              bRead = true;
              continue;
              }
            }

          if( bContinue ) continue;
          //
          iState = 0;
          //sResult := sResult + '\\' + sCommand + '';
          continue;
          }
        }
      }
  return sResult;
  }

  void XPInEdit::RestoreFormula( const QByteArray& Formula, bool InsideEditor )
    {
    m_InsideEditor = InsideEditor;
    QList<QByteArray> List = Convert( Formula ).split( '\n' );
    bool bText = false;
    QByteArray Command;
    IndReg Reg = m_IndReg = irNormal;
    for( int i = 0; i < List.size(); i++ )
      {
      QByteArray sCommand = List[i];
      if( sCommand == "subscript" )
        Reg = irIndex;
      else
        if( sCommand == "superscript" )
          Reg = irPower;
        else
          if( sCommand == "mean" )
            Reg = irShape;
          else
            {
            if( Reg != irNormal )
              {
              for( int iChar = 0; iChar < sCommand.length(); iChar++ )
                {
                m_IndReg = Reg;
                UnRead( sCommand[iChar] );
                }
              m_IndReg = Reg = irNormal;
              continue;
              }
            if( bText )
              {
              XPInEdit::sm_Text = sCommand;
              UnRead( Command );
              bText = false;
              continue;
              }
            if( sCommand == "TEXT" || sCommand == "MATRIX" || sCommand == "MATRIX_UNV" || sCommand == "TABLE" || sCommand == "CRVINTEGRAL" ||
              sCommand == "CHART" || sCommand == "INTERVAL" || sCommand == "INTERVALS" || sCommand == "POLYGON" || sCommand == "TRIGOCOMP" ||
              sCommand == "PICTURE" )
              {
              Command = sCommand;
              bText = true;
              continue;
              }
            UnRead( sCommand );
            }
      }
    }

  bool XPInEdit::IsEmpty() { return m_pL == nullptr || m_pL->IsEmpty(); }
