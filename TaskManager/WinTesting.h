#ifndef MATH_TESTING
#define MATH_TESTING
#include <qapplication.h>
#include <qnetworkaccessmanager.h>
#include <QNetworkReply>
#ifdef Q_OS_ANDROID
#include "../CommonWidgets/KeyBoard.h"
#endif
#include "../CommonWidgets/XPGedit.h"
#include "../CommonWidgets/Panel.h"
#include "../TaskFileManager/taskfilemanager.h"
#include "../Mathematics/Algebra.h"
#include "../Mathematics/ExpOb.h"
#include "QuestWindow.h"
#include "OutWindow.h"
#include <qlistwidget.h>
#include <QListWidgetItem> 
//#include <qcheckbox.h>
//#include <Plotter.h>
#include "MultiPlot.h"

class TranslateObjects : public QHash<QObject*, bool>
  {
  public:
    void AddObject( QObject *pObject ) { insert( pObject, false ); }
    void Translate( QObject *pObject ) { ( *this )[pObject] = true; }
    bool CanSave() { return count() > 2 && key( false ) == nullptr; }
  };

class CalcButton : public QPushButton
  {
  QByteArray m_Hint;
  QByteArray m_DefaultHint;
  public:
    static double sm_ButtonHeight;
    CalcButton( const QByteArray& Hint, const QByteArray& DefaultHint, const QString& Icon = QString(), bool bToTitle = false );
    void LangSwitch();
  };

class CalcWidget : public QWidget
  {
  QGridLayout  *m_pButtons;
  static QTextStream sm_Result;
  static QTextStream sm_Test;
  static void CloseFiles();
  protected:
    static void Solve( Solver* pSolver );
  public:
    static void ResetTestMode();
    static void AutoTest();
    CalcWidget();
    CalcButton *AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint );
    void LangSwitch();
    QPushButton *GetButton( int row, int col );
  };

class WinTesting : public QMainWindow
  {
  Q_OBJECT
    friend HelpTaskWindow;
  friend class TXPTask;
  virtual void	resizeEvent( QResizeEvent *pE );
  static void ClearXPWindows();
  static void StartXPRESSTask( const QString& FName = "", bool Edit = false );
  static void NewXPRESSTask( const QString& TaskFileName, bool Edit = false );
  static QString SelectTask();
  static QAction *sm_English;
  static QAction *sm_Hebrev;
  static QAction *sm_Russian;
  static QAction *sm_Bulgarian;
  static QAction *sm_Ukrainian;
  static QAction *sm_pSaveTaskFile;
  void HideHelps() {}
  QAction *m_pOpenTask;
  QAction *m_pEditTask;
  QAction *m_pQuit;
  QAction *m_pCreateTask;
  QAction *m_pOpenTaskBtn;
  QAction *m_pOpenStack;
  QAction *m_pShowFunctions;
  QAction *m_pShowExpression;
  QAction *m_pHyperlink;
  QAction *m_pEditTracks;
  QAction *m_pSelectTrack;
  QAction *m_pWebTask;
  QMenu *m_pFile;
  QNetworkReply *m_pReply;
  QString m_TasksUrl;
  QString m_RootAppUrl;
  QString m_TopicId;
  QString m_UserId;
  QLabel *m_pWaitMessage;
  QFile m_PictFile;
 // Plotter *m_pPlotter;
  MultiPlotter *m_pMultiPlotter;

  bool m_Review;
  virtual bool event( QEvent *event );
  void ReturnResult();
  void SetBtnShowUnar();
  void SetBtnShowUnarByAdd();
  void SetBtnShowMultSign();
  protected:
    void closeEvent(QCloseEvent *event) override;
  public:
    static Panel *sm_pPanel;
    static QuestWindow *sm_pQuestWindow;
    static OutWindow *sm_pOutWindow;
    static class BottomWindow *sm_pBottomWindow;
    static QMenuBar *sm_pMenuBar;
    static QToolBar *sm_pToolBar;
    static QString sm_TaskPath;
    static QAction *sm_pShowGreek;
    static QAction *sm_pShowBigGreek;
    static QAction *sm_pShowMathSymbols;
    static QAction *sm_pShowUnar;
    static QAction *sm_pShowUnarByAdd;
    static QAction *sm_pShowMultSign;
    static QAction *sm_pEditHeader;
    static QAction *sm_pEditCalc;
    static QAction *sm_pAddPicture;
    static QAction *sm_pShowCalcualtor;
    static QAction *sm_pMultiPlot;
    static QAction *sm_pCalcDeriv;
    static WinTesting* sm_pMainWindow;
    static QNetworkAccessManager sm_NetworkManager;
    static QStringList sm_ApplicationArguments;
    static TranslateObjects sm_TranslateObjects;
    static QMenu* sm_pEditor;
    QAction *m_pSaveFormula;
    QAction *m_pRestoreFormula;
    QVector <QPointF>* m_point_graph=nullptr;
    static bool Translation();
    WinTesting();
    static void NewData();
    static void SaveEnable( QObject* );
    QVector <QPointF> CalculatePoint();
    void ReCalculate();
public slots :
    void slotOpenTaskFile(bool Edit = false);
    void slotChangeLanguage( TLanguages L = lngHebrew );
    void slotSaveTaskFile(bool bNewName = true);
    void slotEditTaskFile() { slotOpenTaskFile( true ); }
    void slotOpenStack();
    void slotCreateTask();
    void slotReplyTopic();
    void slotReplyTask();
    void ReadyRead();
    void ResultRead();
    void slotShowGreek();
    void slotShowBigGreek();
    void slotShowMathSymbols();
    void btnShowUnarClick();
    void btnShowUnarByAddClick();
    void btnShowMultSignClick();
    void SaveExpression();
    void RestoreExpression();
    void EditTaskHeader();
    void EditTaskCalc();
    void ShowFunctions();
    void ShowExpression();
    void AddPictures();
    void AddHyperlink();
    void EditTracks();
    void SelectTrack();
    void BrowseTask();
    void Calc();
    void Quit();
    void MultiPlot();
    void CalcDeriv();
  };

class TypeSelector : public QDialog
  {
  friend WinTesting;
  QRadioButton *m_pLearn;
  QRadioButton *m_pTrain;
  QRadioButton *m_pExam;
  public:
    TypeSelector( QWidget *pParent );
  };

class Algebra1 : public CalcWidget
  {
  Q_OBJECT
  public:
    Algebra1();
    void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Algebra1::*Slot )( ) );
    public slots:
    void FactoringExpression();
    void SquaresDifference();
    void OpeningBrackets();
    void SumSquare();
    void SumCubes();
    void Trinomial();
    void SumCube();
    void DifferenceCubes();
    void Calculator();
  };

class Algebra2 : public CalcWidget
  {
  Q_OBJECT
  public:
    Algebra2();
    void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Algebra2::*Slot )( ) );
    public slots:
    void CalculateLg();
    void LogEquations();
    void Inequalities();
    void SystemInequalities();
    void GraphPresent();
    void ExpEquations();
    void MethodSubstitution();
    void LinearSystem();
    void IntroductingNewVars();
  };

class Equations1 : public CalcWidget
  {
  Q_OBJECT
  public:
    Equations1();
    void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Equations1::*Slot )( ) );
    public slots:
    void LinearEq();
    void RootsQuadEq();
    void FormulaQuadEq();
    void CalcDiscrim();
    void VietteTheorem();
    void BiquadraticEq();
    void AlgFracionsEq();
    void RadicalEq();
    void CalcQuadEq();
  };

class Equations2 : public CalcWidget
  {
  Q_OBJECT
  public:
    Equations2();
    void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Equations2::*Slot )( ) );
    public slots:
    void BasicSinEq();
    void BasicTanEq();
    void SinCosEq();
    void BasiCosEq();
    void BasicCotEq();
    void SquareTrigoEq();
    void AlgFracionsEq();
    void HomogenEq();
    void Calculator();
  };

class SciCalc : public CalcWidget
  {
  Q_OBJECT
  public:
    SciCalc();
    void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( SciCalc::*Slot )( ) );
    public slots:
    void Sin();
    void Cos();
    void Tan();
    void Lg10();
    void Ln();
    void ToRad();
    void ToDeg();
    void Pi();
    void Calculator();
  };

class Calculus : public CalcWidget
  {
  Q_OBJECT
  public:
    Calculus();
    void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Calculus::*Slot )( ) );
  public slots:
    void Derivative();
    void IndefIntegr();
    void Limit();
    void DerivFunc();
    void DefIntegr();
    void EigenValues();
    void Determinant();
    void Transposition();
    void MatrixInv();
  };

class Trigo : public CalcWidget
  {
  Q_OBJECT
  public:
     Trigo();
     void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Trigo::*Slot )( ) );
  public slots:
     void Angle2();
     void Angle3();
     void Tan2();
     void Alpha2();
     void Priv();
     void TrigOfSumma();
     void Summ_Mult();
     void Trigo_Summ();
     void CalcTrigo();
  };

class Advanced : public CalcWidget
  {
  Q_OBJECT
  public:
     Advanced();
     void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Advanced::*Slot )( ) );
  public slots:
     void Permut();
     void BinomCoef();
     void Accomod();
     void Statistic();
     void Correl();
     void LineProgr();
     void AlgToTrigon();
     void ComplexOp();
     void Present();
  };

class AlgFrct : public CalcWidget
  {
  Q_OBJECT
  public:
     AlgFrct();
     void AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( AlgFrct::*Slot )( ) );
  public slots:
     void Mltpl();
     void Div();
     void Calculator();
     void DivPolynoms();
     void Cancel();
     void CmnDenom();
     void Sum();
     void Dif();
     void Polynom();
  };

class ListCalculators : public QListWidget
  {
  public:
  void LangSwitch();
  void AddItem( const QByteArray& Name, const QByteArray& DefaultName );
  };

class DetailedCalculator : public QWidget
  {
  Q_OBJECT
  ListCalculators *m_pCalculators;
  QStackedLayout *m_pCalcsLayout;
  public:
    DetailedCalculator();
    void LangSwitch();
    CalcWidget* GetCalcWidget( int );
    public slots:
    void SelectPanel( int );
  };

class DoubleButton : public QPushButton
  {
  Q_OBJECT
    QPushButton* m_pBaseButton;
  public:
  DoubleButton();
  void SetButton( QPushButton* pButton );
  void ClearButton();
  public slots:
  void ClickBase();
  };

class ButtonBox : public QWidget
  {
  friend class SolverWidget;
  QGridLayout *m_pGrid;
  QLabel *m_pPrompt;
  int m_ButtonCount;
  public:
    ButtonBox(QWidget*);
    void SetButton( QPushButton* );
    void Clear();
    bool SolveDefault();
  };

class SolverWidget : public QWidget
  {
  Q_OBJECT
  QPushButton *m_pSolve;
  QPushButton *m_pSelfTest;
  QPushButton *m_pInpAnswer;
  QPushButton *m_pUndoSelfTest;
  QPushButton *m_pMoveToWorkSheet;
  ButtonBox *m_pButtonBox;
  MathExpr m_Expression;
  virtual void resizeEvent( QResizeEvent *event );
  public:
    SolverWidget();
    void LangSwitch();
    public slots:
    void SearchSolve();
    void Solve( Solver*, QPushButton* );
    void ClearButtons() { m_pButtonBox->Clear(); }
    void MoveToWorkSheet();
    void SelfTest();
    void InpAnswer();
    void UndoSelfTest();
  };

class BottomWindow : public QWidget
  {
  Q_OBJECT
    QLabel *m_pTitle;
  QLabel *m_pCalcTitle;
  QLabel *m_pPrecision;
  CalcButton *m_pPrecisionIncr;
  CalcButton *m_pPrecisionDecr;
  CalcButton *m_pAngleMeas;
  DetailedCalculator *m_pDetailedCalc;
  SolverWidget *m_pSolverWidet;
  CalcButton *m_pSWitchCalc;
  QVBoxLayout *m_pVCalcLayout;
  QGroupBox *m_pCalculator;
  QHBoxLayout *m_pMainLayout;
  TExpr::TrigonomSystem m_TrigonomSystem;
  int m_EditorWidth;
  int m_FullEditorWidth;
  public:
    BottomWindow();
    void LangSwitch();
    void ShowPrecision();
    void ShowAngleMeas();
    public slots:
    void MainTaskClick();
    void PrecIncr();
    void PrecDecr();
    void ChangeAngleMeas();
    void SwitchCalc();
    QPushButton* GetCalcButton( int Widget, int row, int col );
    void ClearButtons() { m_pSolverWidet->ClearButtons(); }
    void ShowCalculator(bool Show);
    void RestoreTrigonomSystem() { TExpr::sm_TrigonomSystem = m_TrigonomSystem; }
    public slots:
    void ShowCalculator();
  };

class CentralWindow : public QWidget
  {
  QLabel *m_pTitle;
  public:
    CentralWindow();
    void SetTitle( const QString& Title ) { m_pTitle->setText( Title ); }
  };

class LogSpinBox : public QDoubleSpinBox
  {
  bool m_ByMouse;
  void stepBy( int steps );
  QValidator::State validate( QString &text, int &pos ) const;
  };

class TaskHeaderEditor : public QDialog
  {
  Q_OBJECT
    BaseTask::TaskHeader& m_TaskHeader;
  QCheckBox *m_pShowUnarMinus;
  QCheckBox *m_pHideUnarMinus;
  QCheckBox *m_pShowMinusByAddition;
  QCheckBox *m_pHideMinusByAddition;
  QCheckBox *m_pShowMultSign;
  QCheckBox *m_pHideMultSign;
  QCheckBox *m_pShowRad;
  QCheckBox *m_pShowDeg;
  QCheckBox *m_pShowMinute;
  QCheckBox *m_pMultiTask;
  QCheckBox *m_pNoHint;
  LogSpinBox *m_pAccuracy;
  QLineEdit *m_pName;
  public:
    TaskHeaderEditor( QWidget *pParent, BaseTask *pTask );
    bool Cancelled() { return m_pName->text().isEmpty();}
    public slots:
    void accept();
    void ChangeUnarm(int);
    void ChangeMinusByAdd(int);
    void ChangeMult(int);
    void ChangeRad(int);
    void ChangeShowMinute(int);
  };

class CaseEditor : public QDialog
  {
  Q_OBJECT
    QTextEdit *m_pEditor;
  public:
    QByteArray m_EditionResult;
    CaseEditor( QWidget *pParent, const QByteArray& VarName, const QByteArray& Text );
    public slots:
    virtual void accept();
  };

class CellEditor : public QLineEdit
  {
  protected:
    virtual void showEvent( QShowEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void mouseDoubleClickEvent( QMouseEvent *event );
    virtual void dragEnterEvent( QDragEnterEvent *event );
  public:
    CellEditor(const QString& Formula ) : QLineEdit(Formula) {}
    static QString ToUnicode(const QByteArray& B);
  };

class RowEditor : public CellEditor
  {
  Q_OBJECT
    int m_RowNumber;
  class TaskCalcEditor *m_pReceiver;
  void focusInEvent( QFocusEvent * );
  virtual void mouseDoubleClickEvent( QMouseEvent *event );
  virtual void mousePressEvent( QMouseEvent *event );
  virtual void changeEvent( QEvent *ev );
  public:
    RowEditor( int RowNumber, TaskCalcEditor *pReceiver );
    public slots:
    void EditCase();
    void paste();
  };

class CaseRow
  {
  friend class TableCaseEditor;
  QLabel *m_pValue;
  CellEditor *m_pExpression;
  public:
    CaseRow( QGridLayout *pGrid, const QByteArray& Value, const QByteArray& Expression, int iRow  );
  };

class TableCaseEditor : public QDialog
  {
  Q_OBJECT
    QComboBox *m_pProbVarName;
    RowEditor *m_pCase;
    QGridLayout *m_pGrid;
    int RowCount() { return m_pGrid->count() / 2; }
  public:
    QByteArray m_EditionResult;
    TableCaseEditor( QWidget *pParent, const QStringList& VarNames, RowEditor* Case );
    public slots:
    virtual void accept();
    virtual void SetVariable();
  };

class CalcRow  
  {
  friend class TaskCalcEditor;
  friend RowEditor;
  RowEditor *m_pVariable;
  RowEditor *m_pExpression;
  QComboBox *m_pParmFunctions;
  QLabel *m_pResult;
  CalcPair m_Pair;
  TaskCalcEditor *m_pEditor;
  public:
    CalcRow() {}
    CalcRow( QGridLayout *pGrid, class TaskCalcEditor*, int iRow  );
    void Recalc();
    void SaveText();
  };

class TaskCalcEditor : public QDialog
  {
  friend RowEditor;
  friend CalcRow;
  Q_OBJECT
    CalcList *m_pCalc;
  static CalcList sm_Calc;
  QVector<CalcRow> m_Rows;
  QGridLayout *m_pGrid;
  QPushButton *m_pTableCaseEdButton;
  QTimer *m_pTestTableCaseButton;
  int m_LastFocused;
  RowEditor *m_pLastFocused;
  void SaveText(int iRow = -1);
  public:
    static bool sm_Restart;
    TaskCalcEditor( QWidget *pParent, BaseTask *pTask );
    int LastFocused() { return m_LastFocused; }
    void SetLastFocused(int);
    public slots:
    void accept();
    void Recalc();
    void AddRow();
    void InsertRow();
    void DelRow();
    void TableCaseEdit();
    void TestTableCaseButton();
  };

class HyperLinkEditor : public QDialog
  {
  Q_OBJECT
  QLineEdit *m_pFile;
  QLineEdit *m_pText;
  QLineEdit *m_pBaseUrl;
  QPushButton *m_pPutHyperlink;
  static QByteArray sm_BaseUrl;
  public:
    HyperLinkEditor();
  public slots:
  void accept();
  void TextChanged(const QString&);
  };

class TraksEditor : public QDialog
  {
  Q_OBJECT
    PTrack m_pTrack;
  QListWidget *m_pListNames;
  RichTextWindow *m_pRichWindow;
  QPushButton *m_pRemoveTrack;
  public:
    TraksEditor();
    public slots:
    void accept();
    void reject();
    void AddTrack();
    void RemoveTrack();
    void EditTrack( QListWidgetItem *item );
  };

#endif
