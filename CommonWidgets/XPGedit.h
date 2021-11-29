#ifndef MATH_XPGEDIT
#define MATH_XPGEDIT
#include <QtWidgets>
#include "XPcursor.h"
#include "../FormulaPainter/InEdit.h"
#include "../Mathematics/ExpOb.h"
#include "commonwidgets.h"

COMMONWIDGETS_EXPORT extern int iFontSize;
COMMONWIDGETS_EXPORT extern int iPowDecrease;
COMMONWIDGETS_EXPORT extern int iPenWidth;

class XPGedit : public QWidget
  {
  static int const
    cm_LeftWGEdMargin = 4,
    cm_TopWGEdMargin = 3;

  XPCursor *m_pCursor;
  int EEditWinHeight();
  int EEditWinWidth();
  int m_VShift;
  int m_HShift;
  bool m_Refreshing;
  bool m_IsCopy;
  bool m_KeyPressed;
  QSize m_MinSize;
  QImage *m_pImage;
  EditSets m_EditSets;
  QPainter m_Painter;
  QByteArray m_FormulaForCopy;
  QByteArray m_FormulaForPaste;
  MathExpr m_ExprForCopy;
  QScrollArea *m_pScrollArea;
  bool CanCopy();
  bool CanPaste();
  void Copy(bool ToWord = false);
  void Paste();
  protected:
    virtual void keyPressEvent( QKeyEvent* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void	resizeEvent( QResizeEvent * );
    virtual void dropEvent( QDropEvent *event );
    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void mouseDoubleClickEvent( QMouseEvent *event );
  public:
    COMMONWIDGETS_EXPORT static bool sm_ShowMultSignInQWindow;
    COMMONWIDGETS_EXPORT static bool sm_SelectPictures;
    XPInEdit *m_pInEdit;
    COMMONWIDGETS_EXPORT static ViewSettings sm_ViewSettings;
    COMMONWIDGETS_EXPORT static void ( *sm_ChangeState )( bool IsEmpty );
    COMMONWIDGETS_EXPORT static void( *sm_ResetTestMode )();
    COMMONWIDGETS_EXPORT static void( *sm_AutoTest )( );
    COMMONWIDGETS_EXPORT QByteArray static &ShrinkFuncName( QByteArray& FuncName );
    COMMONWIDGETS_EXPORT QByteArray static &ExpandFuncName( QByteArray& FuncName );
    COMMONWIDGETS_EXPORT XPGedit( QWidget *parent = nullptr, EditSets *pEditSets = nullptr );
    ~XPGedit() 
      { 
      delete m_pCursor; 
      delete m_pInEdit; 
      }
    virtual void paintEvent( QPaintEvent* );
    void DisplayCursor() { m_pCursor->Show(); }
    void HideCursor() { m_pCursor->Hide(); }
    void MoveCursor();
    COMMONWIDGETS_EXPORT void RefreshXPE();
    COMMONWIDGETS_EXPORT void Editor( U_A_T &Uact );
    virtual bool EdKeyPress( QKeyEvent *pMessage );
    COMMONWIDGETS_EXPORT void RestoreFormula( const QByteArray& Formula );
    COMMONWIDGETS_EXPORT void RestoreFromFile();
    COMMONWIDGETS_EXPORT void SaveFormula();
    COMMONWIDGETS_EXPORT void UnReadExpr( const MathExpr& Exp );
    COMMONWIDGETS_EXPORT void Clear( bool SaveTemplate = false );
    COMMONWIDGETS_EXPORT QByteArray Write() { return m_pInEdit->Write(); }
    COMMONWIDGETS_EXPORT void ResetView( bool ShowFunctions );
    COMMONWIDGETS_EXPORT QScrollArea* SetSize( QSize Size );
  };

class DockWithoutTitle : public QDockWidget
  {
  public:
    COMMONWIDGETS_EXPORT DockWithoutTitle( QWidget *pWidget );
  };
#endif
