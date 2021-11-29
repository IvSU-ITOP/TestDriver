#ifndef MATH_INEDIT
#define MATH_INEDIT
#include <QtWidgets>
#include "XPTypes.h"
#include <QStaticText>
#include "XPSymbol.h"
#include "YClasses.h"
#include "formulapainter_global.h"
#include <QTextCodec>

#define ToLang(x) EdStr::sm_pCodec->toUnicode(x)
#define FromLang(x) EdStr::sm_pCodec->fromUnicode(x)
#define IsConstEdType(Type, Val) dynamic_cast< const Type* >( Val ) != nullptr
#define NumberToStr(x) QByteArray::number(x)

using namespace std;
const int StepString = 8;
FORMULAPAINTER_EXPORT void CreateRecode();

class MapColor : public QMap<QByteArray, QRgb>
  {
  public:
    FORMULAPAINTER_EXPORT static MapColor s_MapColor;
    MapColor();
  };

FORMULAPAINTER_EXPORT QChar ToUnicode( uchar Smb );
FORMULAPAINTER_EXPORT QByteArray FromUnicode(const QString& S);

class TXPGrEl
  {
  public:
#ifdef LEAK_DEBUG
    FORMULAPAINTER_EXPORT static int sm_CreatedCount;
    FORMULAPAINTER_EXPORT static int sm_DeletedCount;
    TXPGrEl() { sm_CreatedCount++; }
    virtual ~TXPGrEl() {sm_DeletedCount++;}
#endif
    QSize m_Size;
    int m_Axis;
    TPoint m_Start;
    FORMULAPAINTER_EXPORT static TLanguages sm_Language;
    FORMULAPAINTER_EXPORT static bool sm_EditKeyPress;
    QPoint Start() { return QPoint( m_Start.X, m_Start.Y ); }
  };

class EdList;
class EdElm;
class EdTable;
class EdChar;

enum EdAction { edNone, edCursor, edRefresh, edBeep, edInvalid };
enum FontKind { fkMain, fkIndex, fkPower, fkExtern };
enum IndReg { irIndex = -1, irNormal, irPower, irShape };

class PEdElm
  {
  EdElm *m_pEdElm;
  public:
    PEdElm() : m_pEdElm( nullptr ) {}
    PEdElm( const PEdElm& E );
    PEdElm( EdElm* pE );
    ~PEdElm();
    bool IsEmpty() const { return m_pEdElm == nullptr; }
    EdElm* operator ->( ) const { return m_pEdElm; }
    EdElm& operator * ( ) const { return *m_pEdElm; }
    PEdElm&  operator = (const PEdElm& E );
    PEdElm& operator = ( EdElm* pE );
    EdElm* data() const { return m_pEdElm; }
    bool operator ==( const PEdElm& E ) const { return E.m_pEdElm == m_pEdElm; }
    bool operator !=( const PEdElm& E ) const { return E.m_pEdElm != m_pEdElm; }
    void clear();
  };

class XPInEdit : public TXPGrEl //    {       Main object-EDITOR of the expression    }
    {
    friend class EdBigChar;
    int m_CharHeight[4];
    int m_TopChar[4];
    int m_SpaceChar[4];
    FontKind m_FontKind;
    FontKind m_SavedKind;
    QRect m_SelectedRect;
    QPen m_OldPen;
    QColor m_OldPenColor;
    QColor m_OldBrushColor;
    class EdStr *m_pActiveStr;
    public:
    static QString ToUnicode( const QString &Text  );
    FORMULAPAINTER_EXPORT static int sm_AddHeight;
    FORMULAPAINTER_EXPORT static XPInEdit* sm_pEditor;
    FORMULAPAINTER_EXPORT static bool sm_IsRetryEdit;
    FORMULAPAINTER_EXPORT static bool sm_EditString;
    FORMULAPAINTER_EXPORT static void( *sm_Message )( const QString& );
    FORMULAPAINTER_EXPORT static bool sm_TextFont;
    static class EdMatr* sm_pMatr;
    FORMULAPAINTER_EXPORT static QByteArray sm_Text;
    FORMULAPAINTER_EXPORT static QString sm_BasePath;
    EdList *m_pL;               //   {       List of the inputing elements   }
    bool m_Active;              //  {       If True - refresh changes               }
    bool m_Visible;             //{       Current Char added as Visible   }
    IndReg  m_IndReg;
    bool m_RecalcSpaces;
    bool m_IsInterval;
    bool m_CreateSystem;
    PEdElm m_pWForMeasure;
    int m_DrawingPower;             //{ if >0 set "power`s" chars_size & chars_color }
                                 //{ if <0 set "idex`s"  chars_size & chars_color }
    EditSets m_EditSets;
    ViewSettings m_ViewSettings;
    QPainter *m_pCanvas;
    QImage *m_pMyImage;
    Qt::PenStyle m_OldPenStyle;
    bool m_InsideEditor;
    void SetPen( const QPen& Pen );
    void RestorePen();
    void SetPaintColor( QColor C );
    void RestorePaintColor();
    void SetBrushColor( QColor C );
    void RestoreBrushColor();
    void SetPenStyle( Qt::PenStyle );
    void RestorePenStyle();
    void FreeContents();
    void SetFont( const QFont &Font );
    void RestoreFont();
    void SetCharFont();
    int TextHeight( const QString &Text );
    int TextWidth( const QString &Text );
    int CharWidth( QChar Char, int Kern = 0 );
    int CharHeight();
    int CharHeight( QChar Char );
    void PaintChar( int X, int Y, QChar Char );
    QRect CharRect( QChar C ) { return m_pCanvas->fontMetrics().boundingRect( C ); }
    int ETextHeight( const QString &Text );
    int ETextWidth( const QString &Text );
    void SetPowerSize( int x, bool allow );
    void SetRegSize( int ind );
    void SetEditSets( EditSets &Value ) { m_EditSets = Value; }
    void SizeEmptySign( QSize &Size, int &Axis );
    void TextOut( int X, int Y, const QString &Text, bool Shift = false );
    void Line( int X1, int Y1, int X2, int Y2 );
    void DrawAsEmptySign( TXPGrEl *pEl );
    FORMULAPAINTER_EXPORT void SetRegister(IndReg SymReg);
    EdList *GetCurrentList();
    void SetCurrentList( EdList *pList );
    FORMULAPAINTER_EXPORT TPoint GetCursorPosition();
    FORMULAPAINTER_EXPORT int GetCursorSize();
    void ConditCalcCursorMeasures( int &X, int &Y, int &SizeY );
    EdTable *m_pCurrentTable;
    FORMULAPAINTER_EXPORT void RestoreFormula( const QByteArray& Formula, bool InsideEditor = false );
    FORMULAPAINTER_EXPORT XPInEdit( const TPoint &P, QPainter *pCanvas, EditSets &AEditSets, const ViewSettings&  );
    FORMULAPAINTER_EXPORT XPInEdit( const QByteArray&, EditSets &AEditSets, const ViewSettings& );
    FORMULAPAINTER_EXPORT virtual ~XPInEdit();
    //       MAIN procedure of edit
    FORMULAPAINTER_EXPORT EdAction EditAction( U_A_T Uact );
    FORMULAPAINTER_EXPORT void PreCalc( TPoint P, QSize &S, int &A );
    QSize CalcSize() { PreCalc( m_Start, m_Size, m_Axis ); return m_Size; }
    FORMULAPAINTER_EXPORT void EditDraw();
    void EditDraw( TPoint NewStart );

    FORMULAPAINTER_EXPORT QByteArray Write();
      //       Generation of List_of_editor`s_elements ( for re-Edition )
    FORMULAPAINTER_EXPORT void UnRead( const QByteArray &S );
    void UnRead( char c ) { char C[] = "0"; C[0] = c; UnRead( C ); }
    FORMULAPAINTER_EXPORT void Clear(bool SaveTemplate = false);
    void SetVisible( bool Visib ) { m_Visible = Visib; }
    FORMULAPAINTER_EXPORT bool GlobalSetCurrent( int x, int y );
    void AddElement( EdElm *pSpacer );
    QByteArray GetFragment();
    void SelectFragment( QRect &FRect );
    FORMULAPAINTER_EXPORT void ClearSelection();
    FORMULAPAINTER_EXPORT QRect GetRect();
    FORMULAPAINTER_EXPORT virtual QImage PWrite();
    FORMULAPAINTER_EXPORT virtual QByteArray SWrite();
    FORMULAPAINTER_EXPORT QImage* GetImage();
    FORMULAPAINTER_EXPORT QPixmap GetPixmap();
    void SelectRect( const QRect& R );
    void ClearSurface();
    FORMULAPAINTER_EXPORT bool IsEmpty();
    FORMULAPAINTER_EXPORT void Select();
    FORMULAPAINTER_EXPORT bool SelectString(int);
    FORMULAPAINTER_EXPORT bool Selected();
    FORMULAPAINTER_EXPORT void SetTextFont();
    FORMULAPAINTER_EXPORT void SetMathFont();
    void ResetIndex();
  };

class EdMemb;

class EdElm : public TXPGrEl  //{       Graphics element of the input text/expression   }
  {
  friend PEdElm;
  int m_Counter;
  public:
    XPInEdit *m_pOwner;
    bool  m_Selected;
    EdElm *m_pParent;
    EdElm() : m_pOwner( nullptr ), m_Selected( false ), m_Counter(0), m_pParent(nullptr) {}
    EdElm( XPInEdit *pOwn ) : m_pOwner( pOwn ), m_Selected( false ), m_Counter( 0 ), m_pParent(nullptr) {}
    virtual ~EdElm() {}
//        {       Set current element & list for edition (if it selected by mouse)        }
    virtual bool SetCurrent( const TPoint &C, EdList* &pSL,  EdMemb* &pCr ) { return true; }
//        {       Evaluation of the element`s coordinates and size        }
    virtual void PreCalc ( TPoint P, QSize &S, int &A ) = 0;
    virtual EdMemb *ParentCalc( EdMemb *pNext ) { return pNext; }
  //      {       Draw element  (If element was moved, executes PreCalc)  }
    virtual void Draw ( TPoint P ) = 0;
    virtual QByteArray Write() = 0;
    virtual QByteArray GetFragment() { if( m_Selected ) return Write() + '&'; return ""; }
    virtual void SelectFragment( QRect &FRect) { m_Selected = InRect( FRect ); }
    virtual void ClearSelection() { m_Selected = false; }
    bool InRect( QRect &FRect);
    bool virtual MoveInRight( EdList* &pL ) { return false; }
    bool virtual MoveInLeft ( EdList* &pL ) { return false; }
    bool virtual MoveToNext ( EdList* &pL ) { return false; }
    bool virtual MoveToPrev ( EdList* &pL ) { return false; }
    bool virtual MoveToUp   ( EdList* &pL ) { return false; }
    bool virtual MoveToDown ( EdList* &pL ) { return false; }
    bool virtual StrSeparat() { return false; }
    bool virtual ElChar( char &C ) { return false; }
    virtual QByteArray SWrite() { return Write(); }
    virtual void SelectRect();
    virtual EdMemb* ReplaceParentMemb(EdList *pL, const PEdElm& pE) { return nullptr; }
  };

inline PEdElm::PEdElm( const PEdElm& E ) : m_pEdElm( E.m_pEdElm ) { if( m_pEdElm != nullptr ) m_pEdElm->m_Counter++; }
inline PEdElm::PEdElm( EdElm* pE ) : m_pEdElm( pE ) { if( m_pEdElm != nullptr ) m_pEdElm->m_Counter++; }
inline PEdElm::~PEdElm()
  {
  if( m_pEdElm != nullptr && --m_pEdElm->m_Counter == 0 )
  delete m_pEdElm;
  }

inline PEdElm&  PEdElm::operator = ( const PEdElm& E )
  {
  if( m_pEdElm != nullptr && --m_pEdElm->m_Counter == 0 ) delete m_pEdElm;
  m_pEdElm = E.m_pEdElm;
  if(m_pEdElm != nullptr ) m_pEdElm->m_Counter++;
  return *this;
  }

inline PEdElm& PEdElm::operator = ( EdElm* pE )
  {
  if( m_pEdElm != nullptr && --m_pEdElm->m_Counter == 0 ) delete m_pEdElm;
  m_pEdElm = pE;
  if (m_pEdElm != nullptr) m_pEdElm->m_Counter++;
  return *this;
  }

inline void PEdElm::clear()
  {
  if( m_pEdElm != nullptr && --m_pEdElm->m_Counter == 0 ) delete m_pEdElm;
  m_pEdElm = nullptr;
  }

class EdDfIntegr;

class EdMemb : public EdElm //               {       element with list`s indexes             }
  {
  friend class EdMatrixBody;
  bool m_IsProtected;
  public:
    EdList  *m_pMother;         //               {       reference on mother list
                              //                                 (that contain this element)             }
    PEdElm m_pMember;    //                     {       contens of this list`s meber    }
    EdMemb *m_pPrev;     //                   {       index on previous element               }
    EdMemb *m_pNext;    //                    {       index on next element                   }
    EdMemb( const PEdElm& pE, EdMemb *pP, EdMemb *pN, EdList *pEL );
    virtual void PreCalc( TPoint P, QSize &S, int &A);
    virtual EdMemb* ParentCalc( EdMemb *pNext );
    virtual void Draw ( TPoint P);
    virtual QByteArray Write() { return m_pMember->Write(); }
    virtual bool StrSeparat()  { return m_pMember->StrSeparat(); }
    virtual bool ElChar( char &C ) { return m_pMember->ElChar(C); }
    bool Protected() { return m_IsProtected; }
    void SetProtected( bool Val ) { m_IsProtected = Val; }
    bool HasIntegral();
    EdDfIntegr *GetDefIntegral();
    int IntegralDPos();
    int IntegralSignPos();
    bool IdleChar();
    int Curr_SizeY();
    int Curr_positionY();
    bool Selected() { return !m_pMember.IsEmpty() && m_pMember->m_Selected; }
  };

class EdList : public EdElm //               {       List of editor`s elements                       }
  {
  protected:
    virtual bool Protected(EdMemb *pIndex) { return pIndex->Protected(); }
public:
  FORMULAPAINTER_EXPORT static bool sm_PartlySelected;
  EdMemb *m_pMother;   //             {       reference on mother element
                     // (that contain this list)                        }
  EdMemb *m_pFirst;    //            {       Index on the first   element            }
  EdMemb *m_pLast;     //           {       Index on the last    element            }
  EdMemb *m_pCurr;     //           {       Current element
                     //   Cursor set before current
                     //  If current=nil - input in position
                     // after last element (into tail LIST)     }
  EdList *m_pSub_L;
  EdMemb *m_pFirstInLine;
  QSize m_PrevSize; // {       size of the image on left-to-right pass }
  int m_PrevAxis;    //  {       Math/Ligic axis of expression   - " -   }
  bool m_Primary;
  bool m_ReCalc;
  bool m_IsCell;
  bool m_Hebrew;
  EdList( XPInEdit *pOwn ) : EdElm( pOwn ), m_Primary( true ), m_ReCalc( false ), m_IsCell( false ), m_Hebrew(false),
    m_pMother(NULL), m_pFirst(NULL), m_pLast(NULL), m_pCurr(NULL), m_pSub_L(NULL), m_pFirstInLine(NULL)    {}
  EdList( EdList *pList );
  virtual ~EdList();
  virtual bool SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr );
  void TailSize();
  virtual EdMemb *ParentCalc( EdMemb *pNext );
  virtual QByteArray Write();
  virtual QByteArray SWrite();
  void virtual MoveLeft ( EdList* &pEl );
  void virtual MoveRight( EdList* &pEl );
  void MoveUp   ( EdList* &pEl );
  void MoveDown ( EdList* &pEl );
  int Curr_positionX();  //  { Position of input }
  int Curr_positionY();
  int Curr_SizeX(); //  { Size of current              }
  int Curr_SizeY();
    //    { Append E to List before current element If current=nil  -  append into tail of EL }
  virtual EdMemb *Append_Before( const PEdElm& pE );
  virtual void MemberDelete( EdMemb *pM );
        //  BACK SPACE processing : deleting of the element, preposition to current. If current = nil - deletating of the last element.
        // If current = HEAD of LIST - has no effect.                      }
  void PrevDelete();
       // DELETE_KEY processing : deleting of the current. If current = nil - has no effect.                                       }
  void NextDelete() { MemberDelete( m_pCurr ); }
  bool Splitted();
  virtual QByteArray GetFragment();
  virtual void SelectFragment( QRect &FRect );
  virtual void ClearSelection();
  void Select();
  virtual void PreCalc ( TPoint P, QSize &S, int &A);
  virtual void Draw ( TPoint P );
  bool HasIntegral();
  EdDfIntegr *GetDefIntegral();
  int IntegralDPos();
  int IntegralSignPos();
  int Count();
  void Clear( int Count = -1 );
  void Copy( EdList *pList );
  bool IsEmpty() { return m_pFirst == nullptr; }
  EdMemb *GetMemb(const EdElm*);
  bool SelectString(int Y);
  };

inline EdList* XPInEdit::GetCurrentList() { return m_pL->m_pSub_L; }
inline void XPInEdit::SetCurrentList( EdList *pList ) { m_pL->m_pSub_L = pList; }
inline QByteArray XPInEdit::Write() { m_pActiveStr = nullptr; return m_pL->Write(); }
inline void XPInEdit::AddElement( EdElm *pSpacer ) { m_pL->m_pSub_L->Append_Before( pSpacer ); }

class EdListTail : public EdList
  {
  public:
    EdListTail( EdList *pParent );
    ~EdListTail() { m_pFirst = NULL; }
  };

class EdMatrixBody : public EdList
  {
  public:
    EdMatrixBody(XPInEdit *pOwn) : EdList(pOwn) {}
    virtual bool SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual EdMemb *Append_Before(const PEdElm& pE);
    void MoveLeft(EdList* &pEl);
    void MoveRight(EdList* &pEl);
    void MemberDelete(EdMemb *pM);
    bool Protected(EdMemb *pIndex);
  };

class EdBaseChar : public EdElm
  {
  protected:
  static int const sc_MaxBracketWidth = 50;
  uchar m_ch;
  QChar m_Qch;
  int m_Height;
  int m_Width;
  public:
    FORMULAPAINTER_EXPORT static QTextCodec *sm_pCodec;
    bool m_vis;
    IndReg m_ind;
    QColor m_Color;
    bool m_EditKeyPress;
    EdBaseChar( uchar C, IndReg ind, QColor Color, XPInEdit *pOwn );
    void ResetChar( uchar C );
    uchar c() { return m_ch; };
    QChar C() { return m_Qch; }
    int Width() { return m_Width; }
    int Height() { return m_Height; }
  };

class EdBigChar : public EdBaseChar
  {
  QFont m_Font;
  int m_Top;
  int m_Kern;
  void CalcSize();
  public:
    EdBigChar( uchar C, IndReg ind, QColor Color, XPInEdit *pOwn, const QFont& Font, int Kern = 0 );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    void RecalcSize( double K );
    virtual QByteArray SWrite();
  };

class EdChar : public EdBaseChar  //       { CHAR as element of the input text/expression }
  {
  void TextOut( TPoint &P );
  int GetAddHeight(char C);
  bool NotVisible() { return !m_vis && (m_pOwner != XPInEdit::sm_pEditor || (m_ch != msMultSign1 && m_ch != msMultSign2)); }
  public:
    bool m_IsInterval;
    bool m_IsSeparat;
    QColor m_Color;
    EdChar( uchar C, XPInEdit *pOwn, QRgb Color = 0 );
      EdChar( EdChar &EC ) : EdBaseChar( EC.m_ch, EC.m_ind, EC.m_Color, EC.m_pOwner ), m_IsInterval( EC.m_IsInterval ),
        m_IsSeparat(EC.m_IsSeparat) { m_Height = EC.m_Height; m_Width = EC.m_Width; m_vis = EC.m_vis; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual EdMemb *ParentCalc( EdMemb *pNext );
    virtual void Draw ( TPoint P );
    virtual QByteArray Write();
    virtual bool StrSeparat();
    virtual bool ElChar( char &C) { C = m_ch; return true; }
    virtual void SetColor( QColor Color ) { m_Color = Color; }
    virtual void SelectFragment( QRect &FRect );
    virtual QByteArray SWrite();
  };

class EdAbs : public EdElm
  {
  public:
    EdList *m_pAA;
    bool m_IsVisible;
    EdAbs() {}
    EdAbs( XPInEdit *pOwn ) : EdElm( pOwn ), m_IsVisible(true), m_pAA( new EdList( pOwn ) ) {}
    virtual ~EdAbs() { delete m_pAA; }
    virtual bool SetCurrent( const TPoint &C, EdList* &pSL,  EdMemb* &pCr );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw ( TPoint P );
    virtual QByteArray Write();
    bool virtual MoveInRight(EdList* &pL);
    bool virtual MoveInLeft(EdList* &pL);
    virtual void SelectFragment( QRect &Rect ) { m_pAA->SelectFragment( Rect ); m_Selected = m_pAA->m_Selected; }
    virtual void ClearSelection() { m_pAA->ClearSelection(); m_Selected = false; }
  virtual QByteArray SWrite();
  };

class EdDetSumm : EdAbs
  {
  bool m_Carry;
  uchar m_CarryOfs;
  QByteArray m_CarryStr;
  int m_Sgnsz, m_Dx, m_Carryx, m_Carrycx, m_Carrycy, m_Chrw;
  char m_SignChr;
  int m_Signum;
  bool m_Normalized;
  public:
    EdDetSumm( XPInEdit *pOwn, char ASignChr, bool ACarry, byte ACarryOfs, const QByteArray& ACarryStr ) : EdAbs( pOwn ) {}
    void Normalize() {}
    void PreCalc( TPoint P, QSize& S, int& A ) {}
    void Draw( TPoint P ) {}
    QByteArray Write() { return ""; }
  };

class EdChart : public EdElm
  {
  friend XPInEdit;
  friend class EdChartEditor;
  friend class CreateChartDialog;
  ChartType m_Type;
  double m_MaxX, m_MaxY, m_Dx, m_XMin;
  int m_X0, m_Y0, m_FullSize;
  bool m_FromTemplate;
  QVector<int> m_Y;
  QVector<QByteArray> m_Labels;
  QVector<QColor> m_Colors;
  bool m_VerticalLabelX;
  double m_Scale;
  PEdElm m_pLabelX, m_pLabelY, m_pNameY, m_pNameX;
  QStringList m_Words;
  static const int sc_Shift = 20;
  public:
    EdChart( XPInEdit *pOwn, const QByteArray& Parms, bool NoStrSelectFont = true );
    EdChart( XPInEdit *pOwn );
    static int CalculateStep( int N, int KX );
    virtual void PreCalc( TPoint P, QSize& S, int& A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
    double GetX( int j, bool bRight = false );
    void SelectFont();
    void SetColumnNumber( int );
  };

class EdTwo : public EdElm
  {
  public:
    EdList *m_pAA;
    EdList *m_pBB;
    EdTwo( XPInEdit *pOwn ) : EdElm( pOwn ), m_pAA( new EdList( m_pOwner ) ), m_pBB( new EdList( m_pOwner ) ) {}
    virtual ~EdTwo() { delete m_pAA; delete m_pBB; }
    virtual QByteArray GetFragment();
    virtual void SelectFragment( QRect &Rect )
      { m_pAA->SelectFragment( Rect ); m_pBB->SelectFragment( Rect ); m_Selected = m_pAA->m_Selected && m_pBB->m_Selected;; }
    virtual void ClearSelection() { m_pAA->ClearSelection(); m_pBB->ClearSelection(); m_Selected = false; }
    virtual bool SetCurrent( const TPoint &C, EdList* &SL, EdMemb* &Cr );
  };

class EdPowr : public EdTwo
  {
  protected:
    bool m_AsIndex;
  public:
    EdPowr( XPInEdit *pOwn ) : EdTwo( pOwn ), m_AsIndex(true) {}
    virtual void PreCalc ( TPoint P, QSize &S, int &A );
    virtual void Draw ( TPoint P );
    virtual QByteArray Write();
    virtual bool MoveInRight( EdList* &pL ) { pL = m_pAA; pL->m_pCurr = m_pAA->m_pFirst; return true; }
    virtual bool MoveInLeft ( EdList* &pL ) { pL = m_pBB; pL->m_pCurr = NULL; return true; }
    virtual bool MoveToNext ( EdList* &pL );
    virtual bool MoveToPrev ( EdList* &pL );
    virtual bool MoveToUp   ( EdList* &pL );
    virtual bool MoveToDown ( EdList* &pL );
    virtual QByteArray SWrite();
  };

class EdRoot : public EdPowr    //TEdRoot = class( TEdPowr )
  {
  public: //private
    EdRoot( XPInEdit *pOwn ) : EdPowr( pOwn ) {}
    virtual void PreCalc ( TPoint P, QSize &S, int &A );//procedure PreCalc ( P : TPoint; Recode[S : TSize; Recode[A : Word ); override;
    virtual void Draw ( TPoint P );//procedure Draw ( P : TPoint );  override;
    virtual QByteArray Write() { return "((" + m_pAA->Write() + ")~(" + m_pBB->Write() + "))"; }//function Write : QByteArray; override;
    virtual bool MoveInRight( EdList* &pL ) { pL = m_pBB; pL->m_pCurr = m_pBB->m_pFirst; return true; } //function MoveInRight( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveInLeft ( EdList* &pL ) { pL = m_pAA; pL->m_pCurr = NULL; return true; }//function MoveInLeft ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToNext ( EdList* &pL ); //function MoveToNext ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToPrev ( EdList* &pL );//function MoveToPrev ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToUp   ( EdList* &pL );//function MoveToUp   ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToDown ( EdList* &pL );//function MoveToDown ( Recode[L : TEdList ) : Boolean; override;
    virtual QByteArray SWrite();
  };

class EdSqRoot : public EdRoot //TEdSqRoot = class( TEdRoot )
  {
  public: //private
    EdSqRoot (XPInEdit *pOwn) : EdRoot ( pOwn ) {}
    virtual void PreCalc ( TPoint P, QSize &S, int &A ); //procedure PreCalc ( P : TPoint; Recode[S : TSize; Recode[A : Word ); override;
    virtual void Draw ( TPoint P );//procedure Draw ( P : TPoint );  override;
    virtual QByteArray Write() { return "((" + m_pAA->Write() + ")~(2))"; }//function Write : QByteArray; override;
    virtual bool MoveInLeft ( EdList* &pL );//function MoveInLeft ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveInRight( EdList* &pL );
    virtual bool MoveToNext ( EdList* &pL ); //function MoveToNext ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToPrev ( EdList* &pL );//function MoveToPrev ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToUp   ( EdList* &pL );//function MoveToUp   ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToDown ( EdList* &pL );//function MoveToDown ( Recode[L : TEdList ) : Boolean; override;
    virtual QByteArray SWrite();
  };

class EdIntegr : public EdTwo
  {
  public:
    EdBigChar *m_pIntegralSign;
    EdList *m_pDD;
    static int sm_SignSize;
    EdIntegr( XPInEdit *pOwn, uchar Sign = msIntegral );
    virtual ~EdIntegr() { delete m_pDD; delete m_pIntegralSign; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual bool MoveInRight( EdList* &pL );
    virtual bool MoveInLeft( EdList* &pL );
    virtual bool MoveToNext( EdList* &pL );
    virtual bool MoveToPrev( EdList* &pL );
    virtual QByteArray GetFragment();
    virtual void SelectFragment( QRect &FRect );
    virtual void ClearSelection() { m_pDD->ClearSelection(); m_Selected = false; }
    virtual QByteArray SWrite();
  };

class EdDoubleIntegr : public EdIntegr
  {
  public:
    EdList *m_pD2;
    EdList *m_pVaR2;
    EdList *m_pRegion;
    EdDoubleIntegr( XPInEdit *pOwn, uchar Sign = msDoubleIntegral );
    virtual ~EdDoubleIntegr() { delete m_pD2; delete m_pRegion; delete m_pVaR2; }
    virtual bool SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual bool MoveInLeft( EdList* &pL );
    virtual bool MoveToNext( EdList* &pL );
    virtual bool MoveToPrev( EdList* &pL );
    virtual bool MoveToUp( EdList* &pL );
    virtual bool MoveToDown( EdList* &pL );
    virtual QByteArray GetFragment();
    virtual void SelectFragment( QRect &FRect );
    virtual void ClearSelection() { m_pDD->ClearSelection(); m_Selected = false; }
    virtual QByteArray SWrite();
  };

class EdTripleIntegr : public EdDoubleIntegr
  {
  public:
    EdList *m_pD3;
    EdList *m_pVaR3;
    EdTripleIntegr( XPInEdit *pOwn );
    virtual ~EdTripleIntegr() { delete m_pVaR3; delete m_pD3; }
    virtual bool SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual bool MoveInLeft( EdList* &pL );
    virtual bool MoveToNext( EdList* &pL );
    virtual bool MoveToPrev( EdList* &pL );
    virtual QByteArray GetFragment();
    virtual void SelectFragment( QRect &FRect );
    virtual void ClearSelection() { m_pDD->ClearSelection(); m_Selected = false; }
    virtual QByteArray SWrite();
  };

class EdCurveIntegr : public EdTwo
  {
  EdBigChar *m_pIntegralSign;
  public:
    EdCurveIntegr( XPInEdit *pOwn, uchar Sign );
    virtual ~EdCurveIntegr() { delete m_pIntegralSign; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual bool MoveInLeft( EdList* &pL );
    virtual bool MoveInRight( EdList* &pL );
    virtual bool MoveToUp( EdList* &pL );
    virtual bool MoveToDown( EdList* &pL );
    virtual QByteArray GetFragment();
    virtual void SelectFragment( QRect &FRect );
    virtual QByteArray SWrite();
  };

class EdVect: public EdAbs
{
public:
int arX1, arY1, arX2, arY2, dy;
EdVect (XPInEdit *pOwn) : EdAbs (pOwn) {}
virtual void PreCalc (TPoint P, QSize &S, int &A);
virtual void Draw (TPoint P);
virtual QByteArray Write ();
virtual QByteArray SWrite();
};

class EdFrac: public EdTwo //TEdFrac = class( TEdTwo )  expression "fraction" : AA/BB
  {
  public: //private
    EdFrac (XPInEdit *pOwn): EdTwo(pOwn) {}
     virtual bool SetCurrent( const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    //function  SetCurrent( C : TPoint; Recode[SL : TEdList; Recode[Cr : EdMemb ) : Boolean;override;
    virtual void PreCalc ( TPoint P, QSize &S, int &A );
    //procedure PreCalc ( P : TPoint; Recode[S : TSize; Recode[A : Word ); override;
    virtual void Draw ( TPoint P );//procedure Draw ( P : TPoint );  override;
    virtual QByteArray Write();
    virtual bool MoveInRight( EdList* &pL );  //function MoveInRight( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveInLeft ( EdList* &pL ); //function MoveInLeft ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToUp   ( EdList* &pL );//function MoveToUp   ( Recode[L : TEdList ) : Boolean; override;
    virtual bool MoveToDown ( EdList* &pL );//function MoveToDown ( Recode[L : TEdList ) : Boolean; override;
    virtual QByteArray SWrite();
  };

class EdLg : public EdElm
  {
  public:
    EdList *m_pAA;
    EdList *m_pNN;
    EdList *m_pB1;
    EdList *m_pB2;
    EdLg (XPInEdit *pOwn, QByteArray Str): EdElm (pOwn), m_pAA(new EdList (m_pOwner) ),
      m_pNN (new EdList (m_pOwner)), m_pB1 (new EdList (m_pOwner)), m_pB2 (new EdList (m_pOwner))
      {
       for (int i=0; i<Str.length(); i++) //отвечает за появление строки "lg"
          {
          m_pNN->Append_Before(new EdChar (Str[i], m_pOwner));
          }
       m_pB1->Append_Before(new EdChar ('(', m_pOwner));
       m_pB2->Append_Before(new EdChar (')', m_pOwner));
      }
    virtual ~EdLg() {delete m_pAA; delete m_pNN; delete m_pB1; delete m_pB2;}
    virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
    virtual QByteArray GetFragment ();
    virtual void SelectFragment (QRect &FRect);
    virtual void ClearSelection ();
    virtual QByteArray SWrite();
  };

class EdSpace : public EdElm
  {
  public:
    bool m_First; //First element in column
    bool m_Last; //following after last column
    EdSpace* m_pNext; //Next element in column
    EdSpace* m_pRight; //Next element in row
    EdSpace() : m_First( false ), m_Last( false ), m_pNext( nullptr ), m_pRight( nullptr ) {}
    EdSpace( bool F, EdSpace* pN, EdSpace* pR, XPInEdit *pOwn );
    void PreCalc( TPoint P, QSize &S, int &A );
    void Draw( TPoint P ) {}
    QByteArray Write() { return ""; }
  };

class EdMatr : public EdAbs
  {
  QVector<QVector<EdSpace*>> m_Spaces;
  int m_RowCount;
  int m_ColCount;
  int m_Row;
  int m_Col;
  public:
    EdMatr(const QByteArray& Size, XPInEdit *pOwn, bool CreateNew = false);
    void AddSpacer();
    virtual void PreCalc(TPoint P, QSize &S, int &A);
    virtual void Draw(TPoint P);
    virtual QByteArray Write();
    virtual QByteArray SWrite();
    virtual bool MoveToUp(EdList* &pL);
    virtual bool MoveToDown(EdList* &pL);
  };

class EdLimit : public EdElm
  {
  public:
    EdList *m_pAA;
    EdList *m_pVL;
    EdList *m_pEE;
    EdList *m_pNN;
    EdList *m_pRR;
    EdLimit (XPInEdit *pOwn) : EdElm (pOwn), m_pAA (new EdList (m_pOwner)), m_pVL (new EdList (m_pOwner)),
      m_pEE (new EdList (m_pOwner)), m_pNN (new EdList (m_pOwner)), m_pRR (new EdList (m_pOwner))
      {
      m_pNN->Append_Before(new EdChar ('l', m_pOwner));
      m_pNN->Append_Before(new EdChar ('i', m_pOwner));
      m_pNN->Append_Before(new EdChar ('m', m_pOwner));
      m_pRR->Append_Before(new EdChar ('-', m_pOwner));
      m_pRR->Append_Before(new EdChar ('>', m_pOwner));
      }
    virtual ~EdLimit() {delete m_pAA; delete m_pVL; delete m_pEE; delete m_pNN; delete m_pRR;}
    virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
    virtual bool MoveToUp (EdList* &pL);
    virtual bool MoveToDown (EdList* &pL);
    virtual QByteArray GetFragment ();
    virtual void SelectFragment (QRect &FRect);
    virtual void ClearSelection ();
    virtual QByteArray SWrite();
  };

class EdImUnit : public EdAbs
  {
  public:
//    EdList *m_pAA;
//    EdImUnit (XPInEdit *pOwn) : EdAbs (pOwn), m_pAA (new EdList (m_pOwner))
//      {m_pAA->Append_Before(new EdChar ('i', m_pOwner));}
//    ~EdImUnit () {delete m_pAA;}
    EdImUnit (XPInEdit *pOwn) : EdAbs (pOwn)
      {
      m_pAA = new EdList( m_pOwner ); m_pAA->Append_Before( new EdChar( 'i', m_pOwner ) );
      }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw (TPoint P);
    virtual QByteArray Write ();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual QByteArray SWrite();
  };

class EdIndx : public EdTwo
  {
  public:
    EdIndx( XPInEdit *pOwn ) : EdTwo( pOwn ) {}
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual bool MoveInRight( EdList* &pL );
    virtual bool MoveInLeft( EdList* &pL );
    virtual bool MoveToPrev( EdList* &pL );
    virtual bool MoveToNext( EdList* &pL );
    virtual bool MoveToUp( EdList* &pL );
    virtual bool MoveToDown( EdList* &pL );
    virtual QByteArray SWrite();
  };

class EdStrokeDeriv : public EdPowr
  {
  int m_Order;
  public:
    EdStrokeDeriv( XPInEdit *pOwn );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
  };

class EdStrokePartialDeriv : public EdIndx
  {
  EdChar *m_pOrder;
  EdList *m_pSavedDeNom;
  public:
    EdStrokePartialDeriv( XPInEdit *pOwn );
    ~EdStrokePartialDeriv() { delete m_pOrder; delete m_pSavedDeNom; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
  };

class EdDeriv : public EdFrac
  {
  protected:
    EdList *m_pSavedDeNom;
    uchar m_Sign;
    bool m_HighOrder;
    static bool TestVar( uchar C );
  public:
    enum { drError = -2, drNoOperation, drSignOnly };
    EdDeriv( XPInEdit *pOwn );
    ~EdDeriv() { delete m_pSavedDeNom; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
    virtual void RestoreDenom();
    virtual int TestDenom();
    virtual bool SetCurrent(const TPoint &C, EdList* &SL, EdMemb* &Cr);
    virtual bool MoveInRight(EdList* &pL);
    virtual bool MoveInLeft(EdList* &pL);
    virtual bool MoveToUp(EdList* &pL);
    virtual bool MoveToDown(EdList* &pL);
    virtual bool MoveToPrev(EdList* &pL);
    virtual bool MoveToNext(EdList* &pL);
    virtual EdMemb* ReplaceParentMemb(EdList *pL, const PEdElm& pE);
  };

class EdPartialDeriv : public EdDeriv
  {
  public:
    EdPartialDeriv( XPInEdit *pOwn );
    virtual void RestoreDenom();
    virtual int TestDenom();
    bool SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual EdMemb* ReplaceParentMemb(EdList *pL, const PEdElm& pE);
    virtual bool MoveToNext(EdList* &pL);
  };

class EdPerCount : public EdElm
  {
  public:
    EdList *m_pAA;
    EdList *m_pB1;
    EdList *m_pB2;
    EdList *m_pCC;
    EdPerCount (XPInEdit *pOwn) : EdElm (pOwn), m_pAA (new EdList (m_pOwner)),
      m_pCC (new EdList (m_pOwner)), m_pB1 (new EdList (m_pOwner)), m_pB2 (new EdList (m_pOwner))
      {
      m_pAA->Append_Before(new EdChar ('P', m_pOwner));
      m_pB1->Append_Before(new EdChar ('(', m_pOwner));
      m_pB2->Append_Before(new EdChar (')', m_pOwner));
      }
    ~EdPerCount() {delete m_pAA; delete m_pB1; delete m_pB2; delete m_pCC;}
    virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
    virtual bool MoveToUp (EdList* &pL);
    virtual bool MoveToDown (EdList* &pL);
    virtual QByteArray GetFragment ();
    virtual void SelectFragment (QRect &FRect);
    virtual void ClearSelection ();
    virtual QByteArray SWrite();
  };

class EdBCoeff : public EdElm
  {
  public:
    EdList *m_pAA;
    EdList *m_pBB;
    EdList *m_pCC;
    EdBCoeff (XPInEdit *pOwn, char Name) : EdElm (pOwn), m_pAA (new EdList (m_pOwner)), m_pBB (new EdList (m_pOwner)),
      m_pCC (new EdList (m_pOwner))
      {
      m_pAA->Append_Before(new EdChar (Name, m_pOwner));
      }
    virtual ~EdBCoeff() { delete m_pAA; delete m_pBB; delete m_pCC; }
    virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
    virtual bool MoveToUp (EdList* &pL);
    virtual bool MoveToDown (EdList* &pL);
    virtual QByteArray GetFragment ();
    virtual QByteArray SWrite();
  };

class EdFunc : public EdTwo
  {
  public:
    EdList *m_pB1;
    EdList *m_pB2;
    EdFunc (XPInEdit *pOwn) : EdTwo (pOwn), m_pB1 (new EdList (m_pOwner)), m_pB2 (new EdList (m_pOwner))
      {
      m_pB1->Append_Before(new EdChar ('(', m_pOwner));
      m_pB2->Append_Before(new EdChar (')', m_pOwner));
      }
    virtual ~EdFunc() { delete m_pB1; delete m_pB2; }
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
    virtual QByteArray GetFragment ();
    virtual void SelectFragment (QRect &FRect);
    virtual void ClearSelection ();
  };

class EdSyst : public EdAbs
  {
  EdBigChar *m_pSign;
  public:
    EdSyst( XPInEdit *pOwn );
    virtual ~EdSyst() { delete m_pSign; }
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual QByteArray SWrite();
  };

class EdInterval : public EdTwo
  {
  EdChar *m_pLeftBracket;
  EdChar *m_pRightBracket;
  EdList *m_pSemicolon;
  public:
    EdInterval( XPInEdit *pOwn, const QByteArray& Signs );
    virtual ~EdInterval() { delete m_pLeftBracket, delete m_pRightBracket; delete m_pSemicolon; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual bool MoveInRight( EdList* &pL ) { pL = m_pAA; pL->m_pCurr = m_pAA->m_pFirst; return true; }
    virtual bool MoveInLeft( EdList* &pL ) { pL = m_pBB; pL->m_pCurr = NULL; return true; }
    virtual bool MoveToNext( EdList* &pL );
    virtual bool MoveToPrev( EdList* &pL );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
  };

class EdIntervals : public EdElm
  {
  struct IPoint
    {
    int m_X;
    double m_Value;
    QByteArray m_sValue;
    bool m_Sign;
    IPoint() {}
    IPoint( const QByteArray& V, const QByteArray& S );
    };
  double m_Ax;
  double m_Len;
  QVector<IPoint> m_Points;
  QVector<bool> m_Intervals;
  public:
    static int sm_Width;
    static int sm_Height;
    EdIntervals( XPInEdit *pOwn, const QByteArray& Intervals );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write() { return ""; }
  };

class EdPolygon : public EdAbs
  {
  double m_MaxX, m_MinX, m_MaxY, m_MinY, m_Gxr, m_Gyr;
  int m_X1[21], m_X2[21], m_Y1[21], m_Y2[21], m_X[21], m_Y[21], m_Gx[21], m_Gy[21];
  int m_X0, m_Y0, m_FullSize;
  int m_Count, m_Count1, m_ColorX, m_ColorY, m_NamedPointCount;
  bool m_ZLP;
  PascArray<QByteArray> m_Inscriptions;
  PascArray<double> m_PosString;
  PascArray< bool > m_Decent;
  struct NamedPoint
    {
    int m_X;
    int m_Y;
    QByteArray m_Name;
    NamedPoint() {}
    NamedPoint( int x, int y, QByteArray Name ) : m_X( x ), m_Y( y ), m_Name( Name ) {}
    };
  PascArray<NamedPoint> m_NamedPoints;
  public:
    EdPolygon( XPInEdit *pOwn, const QByteArray& Polygon );
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write() { return ""; }
  };

class TEdTrigoComp : public EdAbs
  {
  const int cm_Shift = 20;
  double m_Re, m_Im, m_MaxX, m_MinX, m_MaxY, m_MinY, m_rScale;
  int m_x0, m_y0, m_x1, m_y1, m_Scale;
  boolean m_VerticalLabelX;
  public:
  TEdTrigoComp( XPInEdit *pOwn, const QByteArray& Parms );
  virtual void PreCalc( TPoint P, QSize &S, int &A );
  virtual void Draw( TPoint P );
  virtual QByteArray Write();
  virtual QByteArray SWrite();
  };

class EdGMult;

class EdSubst;

class EdGSumm : public EdElm
  {
  protected:
  EdBigChar *m_pSign;
  public:
    EdList *m_pAA;
    EdList *m_pLL;
    EdList *m_pHL;
    EdGSumm( XPInEdit *pOwn, uchar Sign = msBigSigma );
    virtual ~EdGSumm() { delete m_pAA; delete m_pHL; delete m_pLL; delete m_pSign; }
   virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr );
   virtual void PreCalc ( TPoint P, QSize &S, int &A );
   virtual void Draw ( TPoint P );
   virtual QByteArray Write ();
   virtual bool MoveInRight( EdList* &pL );
   virtual bool MoveInLeft ( EdList* &pL );
   virtual bool MoveToNext ( EdList* &pL );
   virtual bool MoveToPrev ( EdList* &pL );
   virtual bool MoveToUp ( EdList* &pL );
   virtual bool MoveToDown ( EdList* &pL );
   virtual QByteArray GetFragment();
   virtual void SelectFragment( QRect &FRect );
   virtual void ClearSelection ();
   virtual QByteArray SWrite();
  };

class EdMeas : public EdTwo
  {
  public:
    EdList *m_pOB;
    EdList *m_pCB;
    bool m_DegMin;
    char m_C;
    EdMeas (XPInEdit *pOwn) : EdTwo (pOwn), m_pOB (new EdList (m_pOwner)), m_pCB (new EdList (m_pOwner))
      {
      m_pOB->Append_Before (new EdChar (' ', m_pOwner));
      m_pOB->Append_Before (new EdChar ('[', m_pOwner));
      m_pCB->Append_Before (new EdChar (']', m_pOwner));
      }
    virtual ~EdMeas() { delete m_pOB; delete m_pCB; }
    virtual void CheckDegMin();
    virtual bool CinMass (); //для проверки принадлежности к массиву
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual QByteArray SWrite();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
  };

class EdLog : public EdElm
  {
  public:
    EdList *m_pAA;
    EdList *m_pBB;
    EdList *m_pB1;
    EdList *m_pCC;
    EdList *m_pB2;
    EdLog (XPInEdit *pOwn) : EdElm (pOwn), m_pAA (new EdList (m_pOwner)), m_pBB (new EdList (m_pOwner)),
      m_pB1 (new EdList (m_pOwner)), m_pCC (new EdList (m_pOwner)), m_pB2 (new EdList (m_pOwner))
      {
      m_pAA->Append_Before (new EdChar ('l', m_pOwner));
      m_pAA->Append_Before (new EdChar ('o', m_pOwner));
      m_pAA->Append_Before (new EdChar ('g', m_pOwner));
      m_pB1->Append_Before (new EdChar ('(', m_pOwner));
      m_pB2->Append_Before (new EdChar (')', m_pOwner));
      }
    ~EdLog() {delete m_pAA; delete m_pBB; delete m_pB1; delete m_pB2; delete m_pCC;}
    virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr);
    virtual void PreCalc (TPoint P, QSize &S, int &A);
    virtual void Draw (TPoint P);
    virtual QByteArray Write();
    virtual bool MoveInRight (EdList* &pL);
    virtual bool MoveInLeft (EdList* &pL);
    virtual bool MoveToPrev (EdList* &pL);
    virtual bool MoveToNext (EdList* &pL);
    virtual bool MoveToUp (EdList* &pL);
    virtual bool MoveToDown (EdList* &pL);
    virtual QByteArray GetFragment ();
    virtual void SelectFragment (QRect &FRect);
    virtual void ClearSelection ();
    virtual QByteArray SWrite();
  };

class EdDfIntegr : public EdIntegr
  {
  int m_Shift;
  public:
    EdList *m_pLL;
    EdList *m_pHL;
    EdDfIntegr ( XPInEdit *pOwn ) :  EdIntegr (pOwn), m_pLL ( new EdList (m_pOwner)),
      m_pHL ( new EdList (m_pOwner)), m_Shift(-1) {}
    ~EdDfIntegr () {delete m_pLL; delete m_pHL;}
   virtual bool SetCurrent (const TPoint &C, EdList* &pSL, EdMemb* &pCr);
   virtual void PreCalc (TPoint P, QSize &S, int &A );
   virtual void Draw (TPoint P);
   virtual QByteArray Write ();
   virtual bool MoveInRight(EdList* &pL);
   virtual bool MoveToNext (EdList* &pL);
   virtual bool MoveToPrev (EdList* &pL);
   virtual bool MoveToUp (EdList* &pL);
   virtual bool MoveToDown (EdList* &pL);
   virtual QByteArray GetFragment();
   virtual void SelectFragment(QRect &FRect);
   virtual void ClearSelection ();
   virtual QByteArray SWrite();
   void DefineHHeight( int &Shift );
  };

class EdGMult : public EdGSumm
  {
  public:
    EdGMult( XPInEdit *pOwn );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
  };

class EdSubst : public EdElm
  {
  TPoint m_PS;
  QSize m_SS;
  public:
    EdList *m_pAA;
    EdList *m_pLL;
    EdList *m_pHL;
    EdSubst( XPInEdit *pOwn );
    virtual ~EdSubst() { delete m_pAA; delete m_pLL; delete m_pHL; }
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw ( TPoint P );
    virtual QByteArray Write ();
    virtual bool MoveInRight( EdList* &pL );
    virtual bool MoveInLeft( EdList* &pL );
    virtual bool MoveToNext( EdList* &pL );
    virtual bool MoveToPrev( EdList* &pL );
    virtual bool MoveToUp( EdList* &pL );
    virtual bool MoveToDown( EdList* &pL );
    virtual QByteArray SWrite();
    virtual bool SetCurrent(const TPoint &C, EdList* &pSL, EdMemb* &pCr);
  };

class EdStr : public EdElm
  {
  friend class EdChart;
  friend class EdTable;
  QStringList m_SValue;
  QByteArray m_Value;
  int m_SelStart, m_SelEnd;
  void SelectFont();
  bool m_NoSelectFont;
  public:
    static bool sm_PureText;
    FORMULAPAINTER_EXPORT static QTextCodec *sm_pCodec;
    EdStr( XPInEdit *pOwn, QByteArray text, bool NoSelectFont = false );
    EdStr( XPInEdit *pOwn, QString text, bool NoSelectFont = false );
    EdStr( XPInEdit *pOwn, const char* text, bool NoSelectFont = false ) : EdStr( pOwn, QByteArray(text), NoSelectFont )  {}
    virtual void PreCalc( TPoint P, QSize &S, int &A );
    virtual void Draw( TPoint P );
    virtual QByteArray Write();
    virtual QByteArray GetFragment();
    virtual void SelectFragment( QRect &FRect );
    virtual void ClearSelection();
    virtual QByteArray SWrite();
    int Width() { return m_Size.width(); }
    int Height() { return m_Size.height(); }
    void AddChar(char c);
  };

class EdTable : public EdElm
  {
  friend class EdChartEditor;
  const int m_BorderSize = 4;
  QVector< int> m_SizeRows;
  QVector< int> m_SizeCols;
  QVector< int> m_SelectedCount;
  QVector< QVector<bool> > m_FrozenCells;
  bool m_WasTemplate;
  bool IsListEx(const QByteArray&) { return false; }
  public:
    int m_Row, m_Col, m_RowCount, m_ColCount;
    FORMULAPAINTER_EXPORT static bool sm_WasTemplate;
    FORMULAPAINTER_EXPORT static bool sm_RowTableAsExpression;
    static PEdElm sm_pTemplate;
    QVector< QVector <EdList*> > m_Body;
    EdList *m_pCurrentCell;
    EdList *m_pOwnerList;
  bool m_IsTEdChartEditor;
  TableGrid m_GridState;
  bool m_NoFreeze;
  QVector< bool > m_UnvisibleColumns;
  EdTable( XPInEdit *pOwn, const QByteArray& Size );
  virtual ~EdTable();
  virtual bool SetCurrent( const TPoint& C, EdList *& SL, EdMemb *& Cr );
  virtual void PreCalc( TPoint P, QSize& S, int& A );
  virtual void Draw( TPoint P );
  void SetAppending( EdMemb& appending );
  virtual QByteArray Write();
  virtual QByteArray SWrite();
  virtual bool MoveInRight( EdList*& L );
  virtual bool MoveInLeft( EdList*& L );
  virtual bool MoveToUp( EdList*& L );
  virtual bool MoveToDown( EdList*& L );
  virtual bool MoveToNext( EdList*& L );
  virtual bool MoveToPrev( EdList*& L );
  bool CanCellEmpty();
  virtual QByteArray GetFragment();
  virtual void SelectFragment( QRect &Rect );
  void ClearSelection();
  void DelRow();
  void DelColumn();
  void InsertRow();
  virtual void InsertColumn();
  void AddRow();
  virtual void AddColumn();
  void Freeze();
  EdList* GetCell();
  virtual void SetColumnNumber( int NewNumber );
  };

class EdChartEditor : public EdTable
  {
  friend class CreateChartDialog;
  EdChart *m_pTemplChart;
  EdMemb *m_pParentMemb;
  void CreateBody();
  public:
    EdChartEditor( XPInEdit *pOwn, EdChart *pChart );
    EdChartEditor( XPInEdit *pOwn );
    ~EdChartEditor();
    virtual bool SetCurrent( const TPoint& C, EdList *& SL, EdMemb *& Cr );
    virtual QByteArray Write();
    virtual QByteArray SWrite();
    virtual bool MoveToUp( EdList*& L );
    virtual bool MoveToDown( EdList*& L );
    bool ResetChart();
    ChartType ChartType() { return m_pTemplChart->m_Type; }
    void SetColumnNumber( int NewNumber );
  };

class EdChartType : public EdElm
  {
  EdChartEditor *m_pEditor;
  ChartType m_Type;
  QVector<QPixmap> m_Pictures;
  public:
    EdChartType( EdChartEditor *pEd );
    virtual void PreCalc( TPoint P, QSize& S, int& A );
    virtual void Draw( TPoint P );
    QByteArray Write();
    void ChangeType();
  };

class EdColumnColor : public EdElm
  {
  friend EdChartEditor;
  QColor m_Color;
  bool m_WasChanged;
  EdChartEditor *m_pEditor;
  public:
    EdColumnColor( QColor Color, EdChartEditor *pEd );
    virtual void PreCalc( TPoint P, QSize& S, int& A );
    virtual void Draw( TPoint P );
    QByteArray Write();
    void ChangeColor();
  };

class EdPicture : public EdElm
  {
  QPixmap m_Picture;
  QByteArray m_Path;
  QString m_NotFound;
  public:
    EdPicture(XPInEdit *pOwn, const QByteArray& Path);
  void PreCalc(TPoint P, QSize& S, int& A);
  void Draw(TPoint P);
  virtual QByteArray Write();
  virtual QByteArray SWrite();
  };

#endif
