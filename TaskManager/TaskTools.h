#ifndef MATH_TASKTOOLS
#define MATH_TASKTOOLS

#include <QSharedPointer>
#include <qfile.h>
#include <qsize.h>
#include "../Mathematics/ExpOb.h"
#include <qframe.h>
#include <qtextedit.h>
#include <qscrollarea.h>
#include "../TaskFileManager/Globa.h"
#include <QPushButton>
#include <QBoxLayout>
#include <qlabel.h>
#include <qgroupbox.h>
#include "../TaskFileManager/taskfilemanager.h"

bool GetString( QByteArray& line, QByteArray& Name );
bool XPkeyword( const QByteArray& S );

class HelpButtonsWindow;
class HelpStepWindow;
class TXPTask;

class RichTextWindow : public QTextEdit, public RichText
  {
  Q_OBJECT
  ViewSettings m_ViewSettings;
  ViewSettings m_OldViewSettings;
  virtual void closeEvent( QCloseEvent *event );
  void showEvent( QShowEvent * );
  virtual void contextMenuEvent( QContextMenuEvent *event );
  bool m_ContentLoading;
protected:
  virtual void dropEvent(QDropEvent *event) override;
//  virtual void dragEnterEvent( QDragEnterEvent *event ) override;
  public:
    RichTextWindow( QWidget *pParent );
    ~RichTextWindow();
    QSize GetMinSize();
    void ResetSize();
    QByteArray GetText();
    void ResetLanguage();
    void SetFixedWidth( int Width ); 
    public slots:
    void SetContent( PDescrList, bool SetDocument = false );
    void ContentChanged();
    void AddRow();
    void DeleteRow();
  };

class HelpButton : public QLabel
  {
  Q_OBJECT
  friend class HelpTaskWindow;
  friend class HelpButtonsWindow;
  PStepMemb m_pStepMemb;
  class BoundedButton *m_pEdit;
  BoundedButton *m_pDelete;
  BoundedButton *m_pAddBefore;
  virtual void mousePressEvent( QMouseEvent *e );
  public:
    HelpButton( class HelpButtonsWindow *pParent, PStepMemb pStepMemb );
    PStepMemb GetStep() { return m_pStepMemb; }
    int GetPercent();
  signals:
    void clicked();
  };

class HelpButtonsWindow : public QGroupBox
  {
  Q_OBJECT
  friend HelpButton;
  QVBoxLayout *m_pLayout;
  class StepEdit *m_pStepEdit;
  class HelpTaskWindow *m_pTaskWindow;
  virtual void	resizeEvent( QResizeEvent *pE );
  public:
    bool m_TrueSummPercent;
    HelpButtonsWindow( HelpTaskWindow *pParent, PStepList pStepList );
    int GetMinHeight();
  public slots:
  void StartEditStep();
  void StopEditStep();
  void RestartEditStep();
  void AddBefore();
  void DeleteStep();
  void AddStep();
  };

class HelpPanelWindow : public QWidget
  {
  public:
    HelpPanelWindow( QWidget *pParent, QWidget *pWindow, const QString& Title ) : QWidget( pParent ) { Create( pWindow, Title ); }
    HelpPanelWindow( QWidget *pParent ) : QWidget( pParent ) {}
  protected:
    void Create( QWidget *pWindow, const QString& Title );
  };

class HelpStepWindow : public HelpPanelWindow
  {
  friend class TXPTask;
  PStepMemb m_pStepMemb;
  virtual void closeEvent( QCloseEvent *event );
  public:
    HelpStepWindow( QWidget *pParent, PStepMemb pStepMemb );
  };

class HintWindow : public HelpPanelWindow
  {
  public:
    HintWindow( QWidget *pParent, TXPStepPromptDescr *pStepPrompt );
    HintWindow( QWidget *pParent, TXPSOptPromptDescr *pOptPrompt );
    HintWindow(QWidget *pParent, PDescrList AnswerPrompt);
  };

class HelpTaskWindow : public QDialog
  {
  Q_OBJECT
  friend HelpStepWindow;
  friend HelpButtonsWindow;
  friend class WinTesting;
  friend TXPTask;
  RichTextWindow *m_pRichWindow;
  HelpButtonsWindow *m_pButtonsWindow;
  HelpStepWindow *m_pStepWindow;
  PStepList m_pStepList;
  QHBoxLayout *m_pPanelLayout;
  HelpPanelWindow *m_pButtonPanel;
  QPushButton *m_pBtnOK;
  QLabel *m_pInvalidSumm;
  virtual void keyPressEvent( QKeyEvent *event );
  virtual void closeEvent( QCloseEvent *event );
  public:
    HelpTaskWindow(  QWidget *pParent, const QString& Label, PStepList pStepList, PDescrList pDescrList );
    public slots:
    void ShowStep();
    void RefreshButtons();
  };

class ExprPanel : public QLabel
  {
  friend class NationalTextButton;
  MathExpr m_Expr;
  QByteArray *m_pExpr;
  QColor m_BackColor;
  void CreateContent();
  virtual void mouseDoubleClickEvent( QMouseEvent *event );
  virtual void dropEvent( QDropEvent *event );
  virtual void dragEnterEvent( QDragEnterEvent *event );
  virtual void mousePressEvent( QMouseEvent* );
  bool CanPaste();
  public:
    ExprPanel( QWidget *pParent, MathExpr Exp, const QString& BackColor );
    ExprPanel( QByteArray *pExp, const QString& BackColor );
  };


class NationalTextButton : public QPushButton
  {
  ExprPanel *m_pExprPanel;
  virtual void mouseReleaseEvent(QMouseEvent *e);
  public:
    NationalTextButton(ExprPanel *pPanel) : QPushButton("Text"), m_pExprPanel(pPanel) {}
  };

class SelectTrackButton : public QPushButton
  {
  friend class TrackSelectionWindow;
  int m_TrackNumber;
  public:
    SelectTrackButton( QWidget *pParent, int TrackNumber, const QString& Text );
  };

class TrackSelectionWindow : public QDialog
  {
  Q_OBJECT
  virtual void keyPressEvent( QKeyEvent *event );
  bool m_IsHelp;
  public:
    TrackSelectionWindow( QWidget *pParent, bool bIsHelp );
  public slots:
  virtual void accept();
  };

class CommentEd : public QLineEdit
  {
  Q_OBJECT
  QByteArray *m_pHead;
  public:
    CommentEd( QByteArray *pHead );
    public slots:
    void Edit();
  };

class MarkEd : public QSpinBox
  {
  Q_OBJECT
  friend class StepEdit;
  int *m_pMark;
  public:
    MarkEd( int *pMark );
  public slots:
    void Edit(int i);
  };

class TemplEdit : public QLineEdit
  {
  Q_OBJECT
    PDescrList m_pTemplate;
  virtual void mouseDoubleClickEvent( QMouseEvent *event );
  virtual void dropEvent( QDropEvent *event );
  virtual void dragEnterEvent( QDragEnterEvent *event );
  public:
    TemplEdit( PStepMemb pStep );
  public slots:
  void Edit();
  };

class PromptEditor : public QDialog
  {
  public:
    PromptEditor(QWidget *pParent, PStepMemb pStep);
  };

class FalseCommentEditor : public QDialog
  {
  public:
    FalseCommentEditor(QWidget *pParent, PDescrList pFalse);
  };

class StepEdit : public QDialog
  {
  Q_OBJECT
    friend HelpButtonsWindow;
  HelpButton *m_pButton;
  CommentEd *m_pStepName;
  QCheckBox *m_pShowUnarMinus;
  QCheckBox *m_pHideUnarMinus;
  QCheckBox *m_pShowMinusByAddition;
  QCheckBox *m_pHideMinusByAddition;
  QCheckBox *m_pShowMultSign;
  QCheckBox *m_pHideMultSign;
  QCheckBox *m_pOnExactCompare;
  QCheckBox *m_pShowRad;
  QCheckBox *m_pShowDeg;
  QCheckBox *m_pNoHint;
  QPushButton *m_pShowPromptEdit;
  PromptEditor *m_pPromptEditor;
  FalseCommentEditor *m_pFalseCommentEditor;
  QPushButton *m_pBtnEdCommentF1;
  QPushButton *m_pBtnEdCommentF2;
  QPushButton *m_pBtnEdCommentF3;
  MarkEd *m_pMarkEdit;
  QDoubleSpinBox *m_pHeightEditorWindow;
  public:
    StepEdit( HelpButton *pButton );
    public slots:
    void AddAnswer();
    void RemoveAnswer();
    void Recalc();
    void ChangeUnarm( int );
    void ChangeMinusByAdd(int);
    void ChangeMult( int );
    void ChangeRad( int );
    void ChangeHint(int);
    void PromptEdit();
    void FalseCommentEdit();
    PStepMemb GetStep() { return m_pButton->GetStep(); }
    void accept();
  };

class BoundedButton : public QPushButton
  {
  public:
  HelpButton *m_pOwner;
  PDescrMemb m_pData;
  BoundedButton( HelpButton *pOwner, const QByteArray& Icon, const QByteArray& ToolTip );
  BoundedButton( PDescrMemb pData, const QByteArray& Icon, const QByteArray& ToolTip );
  };

class NationalTextEditor : public QDialog
  {
    QTextEdit *m_TextEditor;
  public:
  NationalTextEditor(const QString& Text);
  QByteArray GetText();
  };

#endif
