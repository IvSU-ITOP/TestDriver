#include "WinTesting.h"
#include "../Mathematics/ExpStore.h"
#include "../Mathematics/Parser.h"
#include "../Mathematics/SolChain.h"
#include "../Mathematics/Analysis.h"
#include "../Mathematics/GlobalVars.h"
#include "../FormulaPainter/InEdit.h"
#include "TaskWork.h"
#include <qcoreapplication.h>
#include <QUrl>
#include <qtextstream.h>
#include "SelectTask.h"

extern TXPTask s_Task;
TXPTask s_MainTask;
extern QString s_AppPath;

Panel* WinTesting::sm_pPanel = nullptr;
QuestWindow* WinTesting::sm_pQuestWindow = nullptr;
OutWindow* WinTesting::sm_pOutWindow = nullptr;
BottomWindow* WinTesting::sm_pBottomWindow = nullptr;
QMenuBar* WinTesting::sm_pMenuBar;
QMenu* WinTesting::sm_pEditor;
QToolBar* WinTesting::sm_pToolBar;
QString WinTesting::sm_TaskPath;
WinTesting* WinTesting::sm_pMainWindow;
double CalcButton::sm_ButtonHeight = 50.0;
QNetworkAccessManager WinTesting::sm_NetworkManager;
QAction* WinTesting::sm_English;
QAction* WinTesting::sm_Hebrev;
QAction* WinTesting::sm_Russian;
QAction* WinTesting::sm_Bulgarian;
QAction* WinTesting::sm_Ukrainian;
QAction* WinTesting::sm_pSaveTaskFile;
QAction* WinTesting::sm_pShowGreek;
QAction* WinTesting::sm_pShowBigGreek;
QAction* WinTesting::sm_pShowMathSymbols;
QAction* WinTesting::sm_pShowUnar;
QAction* WinTesting::sm_pShowUnarByAdd;
QAction* WinTesting::sm_pShowMultSign;
QAction* WinTesting::sm_pAddPicture;
QAction* WinTesting::sm_pEditHeader;
QAction* WinTesting::sm_pEditCalc;
QAction* WinTesting::sm_pShowCalcualtor;
QAction* WinTesting::sm_pMultiPlot;
QAction* WinTesting::sm_pCalcDeriv;
TranslateObjects WinTesting::sm_TranslateObjects;

QTextStream CalcWidget::sm_Result(new QFile );
QTextStream CalcWidget::sm_Test( new QFile );

QStringList WinTesting::sm_ApplicationArguments;

bool TaskCalcEditor::sm_Restart = false;
CalcList TaskCalcEditor::sm_Calc;
QByteArray HyperLinkEditor::sm_BaseUrl("https://halomda.org/Download/");

void TaskHelpShow() { s_Task.HelpShow(); }
void TaskHintShow() { s_Task.HintShow(); }

void AnsverEnter() 
  {
  bool ShowMSignOld = BaseTask::sm_GlobalShowMultSign;
  s_Task.EntBtnClick(); 
  if (ShowMSignOld != BaseTask::sm_GlobalShowMultSign) WinTesting::sm_pMainWindow->btnShowMultSignClick();
  }

void NewData() { WinTesting::NewData(); }
void ChangeEditor( bool IsEmpty ) { WinTesting::sm_pMainWindow->m_pSaveFormula->setDisabled( IsEmpty ); }
void ResetTestMode() { CalcWidget::ResetTestMode(); }
void AutoTest() { CalcWidget::AutoTest(); }

DetailedCalculator::DetailedCalculator() : m_pCalculators( new ListCalculators ), 
  m_pCalcsLayout( new QStackedLayout )
  {
  QVBoxLayout *pVCalcLayout = new QVBoxLayout;
  Algebra1 *pAl1 = new Algebra1;
  Algebra2 *pAl2 = new Algebra2;
  Equations1 *pE1 = new Equations1;
  Equations2 *pE2 = new Equations2;
  SciCalc *pSciCalc = new SciCalc;
  Calculus *pCalculus = new Calculus;
  Trigo *pTrigo = new Trigo;
  Advanced *pAdvanced = new Advanced;
  AlgFrct *pAlgFrct = new AlgFrct;
  m_pCalcsLayout->addWidget( pAl1 );
  m_pCalcsLayout->addWidget( pAl2 );
  m_pCalcsLayout->addWidget( pE1 );
  m_pCalcsLayout->addWidget( pE2 );
  m_pCalcsLayout->addWidget( pSciCalc );
  m_pCalcsLayout->addWidget( pCalculus );
  m_pCalcsLayout->addWidget( pTrigo );
  m_pCalcsLayout->addWidget( pAdvanced );
  m_pCalcsLayout->addWidget( pAlgFrct );
  QHBoxLayout *pHLayout = new QHBoxLayout;
  m_pCalculators->AddItem( "MAlgebra", "Algebra 1" );
  m_pCalculators->AddItem( "MAlgebra_2", "Algebra 2" );
  m_pCalculators->AddItem( "MEquations", "Equations1" );
  m_pCalculators->AddItem( "MEquations2", "Equations2" );
  m_pCalculators->AddItem( "MSciCalc", "Sci. Calc." );
  m_pCalculators->AddItem( "MCalculus", "Calculus" );
  m_pCalculators->AddItem( "MTrigo", "Trigo" );
  m_pCalculators->AddItem( "MAdvanced", "Advanced" );
  m_pCalculators->AddItem( "MAlgFractLong", "Algebraic Fractions" );
  connect( m_pCalculators, SIGNAL( currentRowChanged( int ) ), SLOT( SelectPanel( int ) ) );
  pHLayout->addWidget( m_pCalculators );
  pHLayout->addLayout( m_pCalcsLayout );
  pVCalcLayout->addLayout( pHLayout );
  setLayout( pVCalcLayout );
  }

void DetailedCalculator::SelectPanel( int Panel )
  {
  Panel::sm_pEditor->setFocus();
  m_pCalcsLayout->setCurrentIndex( Panel );
  }

void DetailedCalculator::LangSwitch()
  {
  m_pCalculators->LangSwitch();
  for( int Widget = 0; Widget < m_pCalcsLayout->count(); Widget++ )
    dynamic_cast< CalcWidget* >( m_pCalcsLayout->widget( Widget ) )->LangSwitch();
  }

CalcWidget* DetailedCalculator::GetCalcWidget( int Index )
  {
  return dynamic_cast< CalcWidget* >( m_pCalcsLayout->widget( Index ) );
  }

DoubleButton::DoubleButton() : m_pBaseButton( nullptr )
  {
  setEnabled( false );
  QPixmap Pixmap( ":/Resources/Calculator/BtnCalcCalc.jpg" );
  QPixmap NewPix( Pixmap.scaledToHeight( CalcButton::sm_ButtonHeight - 10 ).size() );
  NewPix.fill( palette().button().color() );
  setIcon( NewPix );
  setIconSize( NewPix.size() );
  setFixedWidth(NewPix.width());
  setFlat( true );
  connect( this, SIGNAL( clicked() ), SLOT( ClickBase() ) );
  }

void DoubleButton::SetButton( QPushButton* pButton )
  {
  m_pBaseButton = pButton;
  setIcon( pButton->icon() );
  setEnabled( true );
  setToolTip( pButton->toolTip() );
  setGeometry(pButton->geometry());
  }

void DoubleButton::ClearButton()
  {
  setEnabled( false );
  QPixmap NewPix( iconSize() );
  NewPix.fill( palette().button().color() );
  setIcon( NewPix );
  setToolTip( "" );
  }

void DoubleButton::ClickBase()
  {
  m_pBaseButton->click();
  }

ButtonBox::ButtonBox(QWidget *pParent) : QWidget(pParent), m_pGrid(new QGridLayout),
  m_ButtonCount(0), m_pPrompt(new QLabel(" ") )
  {
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  pMainLayout->addWidget( m_pPrompt );
  m_pGrid->setMargin( 0 );
  setStyleSheet( "QPushButton { margin:0 }" );

  auto AddButton = [&] ( QPushButton* pBtn )
    {
    m_pGrid->addWidget( pBtn, m_pGrid->count() / 3, m_pGrid->count() % 3 );
    };

  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  AddButton( new DoubleButton );
  pMainLayout->addLayout( m_pGrid );
  setLayout( pMainLayout );
  adjustSize();
  }

void ButtonBox::SetButton( QPushButton* pBtn )
  {
  dynamic_cast< DoubleButton* >( m_pGrid->itemAtPosition( m_ButtonCount / 3, m_ButtonCount % 3 )->widget() )->SetButton( pBtn );
  m_ButtonCount++;
  }

void ButtonBox::Clear()
  {
  for( int iButton = 0; iButton < m_ButtonCount; iButton++ )
    dynamic_cast< DoubleButton* >( m_pGrid->itemAtPosition( iButton / 3, iButton % 3 )->widget() )->ClearButton();
  m_ButtonCount = 0;
  m_pPrompt->clear();
  }

bool ButtonBox::SolveDefault()
  {
  if( m_ButtonCount == 0 ) return false;
  int iButton = m_ButtonCount - 1;
  DoubleButton *pBtn = dynamic_cast< DoubleButton* >( m_pGrid->itemAtPosition( iButton / 3, iButton % 3 )->widget() );
  pBtn->click();
  pBtn->ClearButton();
  if( m_ButtonCount == 1 ) m_pPrompt->clear();
  return true;
  }

SolverWidget::SolverWidget() : m_pSolve( new QPushButton(this) ), m_pButtonBox( new ButtonBox(this) ),
  m_pMoveToWorkSheet(new QPushButton(this))
  {
  QPixmap Pixmap = QPixmap( ":/Resources/Calculator.jpg" ).scaledToHeight( CalcButton::sm_ButtonHeight * 2.5 );
  m_pSolve->setIcon( Pixmap );
  m_pSolve->setIconSize( Pixmap.size() );
  m_pSolve->setFlat( false );
  connect( m_pSolve, SIGNAL( clicked() ), SLOT( SearchSolve() ) );
  m_pSolve->adjustSize();
  Pixmap = QPixmap( ":/Resources/MoveToWorksheet.png" );
  m_pMoveToWorkSheet->setIcon(Pixmap);
  m_pMoveToWorkSheet->setIconSize( Pixmap.size() );
  m_pMoveToWorkSheet->setFixedWidth(Pixmap.width());
  m_pMoveToWorkSheet->setFlat( false );
  connect( m_pMoveToWorkSheet, SIGNAL( clicked() ), SLOT( MoveToWorkSheet() ) );
  m_pMoveToWorkSheet->adjustSize();
  }

void SolverWidget::resizeEvent( QResizeEvent *event )
  {
  int Wthis = width(), WSolve = m_pSolve->width(), WBox = m_pButtonBox->width(), WMove = m_pMoveToWorkSheet->width();
  int HThis = height(), HSolve = max(m_pSolve->height(), m_pButtonBox->height()), HTitle = m_pMoveToWorkSheet->height();
  int HSP = (HThis - HSolve - HTitle) / 4;
  int WSp = (Wthis - WSolve - WBox) / 15;
  QRect RMove = m_pMoveToWorkSheet->geometry();
  RMove.moveTopLeft(QPoint((Wthis - WMove) /3, HSP ) );
  m_pMoveToWorkSheet->setGeometry(RMove);
  HSP += HSP + HTitle;
  QRect RSolve = m_pSolve->geometry();
  RSolve.moveTopLeft(QPoint(WSp, HSP));
  m_pSolve->setGeometry(RSolve);
  QRect RBox = m_pButtonBox->geometry();
  RBox.moveTopLeft(QPoint(WSp + WSolve + WSp, HSP) );
  m_pButtonBox->setGeometry(RBox);
  }

void SolverWidget::LangSwitch()
  {
  m_pSolve->setToolTip( X_Str( "MSolveAllTasks", "Solve" ) );
  m_pButtonBox->setToolTip( X_Str( "MPossibleTasks", "Possible tasks" ) );
  m_pMoveToWorkSheet->setToolTip( X_Str( "MBtnEnterHint", "Move expression to output window" ) );
  }

void SolverWidget::MoveToWorkSheet()
  {
  MainTab::sm_Enter();
  }

void SolverWidget::SelfTest()
  {

  }

void SolverWidget::InpAnswer()
  {

  }

void SolverWidget::UndoSelfTest()
  {

  }

void SolverWidget::Solve( Solver *pS, QPushButton* pBtn)
  {
  auto Final = [&]()
    {
    delete pS;
    return;
    };
  try
    {
    TExpr::sm_CalcOnly = true;
    ExpStore::sm_pExpStore->Init_var();
    pS->SetExpression( m_Expression );
    TExpr::sm_CalcOnly = false;
    if (s_GlobalInvalid) return Final();
    }
  catch( ErrParser )
    {
    TExpr::sm_CalcOnly = false;
    return Final();
    }
  MathExpr Result = pS->Result();
  if( Result.IsEmpty() ) return Final();
  bool V;
  if( Result.Boolean_( V ) )
    {
    if( !V ) return Final();
    }
  else
    if( !pS->Success() ) return Final();
  m_pButtonBox->SetButton( pBtn );
  return Final();
  }

void SolverWidget::SearchSolve()
  {
  Panel::sm_pEditor->setFocus();
  m_pButtonBox->Clear();
  QByteArray Formula( Panel::sm_pEditor->Write() );
  if( Formula.isEmpty() ) return;
  s_GlobalInvalid = false;
  m_Expression = Parser::StrToExpr( Formula );
  if( m_Expression.IsEmpty() || s_GlobalInvalid )
    {
    WinTesting::sm_pOutWindow->AddComm( X_Str( "MsyntaxErr", "Syntax error!" ) );
    return;
    }
  TSolutionChain::sm_SolutionChain.Clear();
  TSolutionChain::sm_SolutionChain.m_Accumulate = false;

  QString Prompt;
  auto Final = [&] ( bool Solve )
    {
    TSolutionChain::sm_SolutionChain.m_Accumulate = true;
    if (m_pButtonBox->m_ButtonCount > 0) m_pButtonBox->m_pPrompt->setText( Prompt );
    if( !Solve ) return;
    if( !m_pButtonBox->SolveDefault() )
      WinTesting::sm_pOutWindow->AddComm( X_Str( "MNotSuitableExpr", "Not suitable expression!" ) );
    };

  if( m_Expression.ConstExpr() || m_Expression.HasComplex() || m_Expression.HasMatrix() )
    {
    if( IsType( TConstant, m_Expression ) )
      {
      Solve( new TSin, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 0, 0 ) );
      Solve( new TCos, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 0, 1 ) );
      Solve( new TTan, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 0, 2 ) );
      Solve( new TLg, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 1, 0 ) );
      Solve( new TLn, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 1, 1 ) );
      Solve( new TDegRad, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 1, 2 ) );
      Solve( new TRadDeg, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 2, 0 ) );
      Solve( new TSciCalc, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 2, 2 ) );
      Prompt = X_Str( "MCanEvaluate", "For this value you can calculate:" );
      return Final( false );
      }
    Solve( new TSciCalc, WinTesting::sm_pBottomWindow->GetCalcButton( 4, 2, 2 ) );
    return Final( true );
    }

  bool TestNumerical = false;
  auto SearchSystem = [&] ( const MathExpr& Ex )
    {
    PExMemb pMemb;
    if( !Ex.Listex( pMemb ) ) return false;
    MathExpr Left, Right;
    uchar Sign;
    if( !pMemb->m_Memb.Binar_( Sign, Left, Right ) ) return true;
    if( Sign != '=' )
      {
      Solve( new SysInEqXY, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 1, 0 ) );
      Solve( new SysInEq, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 0, 2 ) );
      Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
      return true;
      }
    PExMemb pNext = pMemb->m_pNext;
    if( pNext.isNull() ) return true;
    if( TestNumerical && pNext->m_Memb.IsNumerical() )
      {
      pNext = pNext->m_pNext;
      if( pNext.isNull() || !pNext->m_Memb.IsNumerical() ) return true;
      Solve( new TSolvCalcEquation, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 2, 2 ) );
      return true;
      }
    if( !pNext->m_Memb.Binar( '=', Left, Right ) ) return true;
    Solve( new MakeSubstitution, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 2, 0 ) );
    Solve( new MakeExchange, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 2, 2 ) );
    Solve( new SolveLinear, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 2, 1 ) );
    Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
    return true;
    };
  
  MathExpr SysExpr;
  if( m_Expression.Syst_( SysExpr ) )
    {
    SearchSystem( SysExpr );
    return Final(true);
    }
  
  TestNumerical = true;
  if( SearchSystem( m_Expression ) ) return Final(true);

  MathExpr Left, Right;
  uchar Sign;
  if( m_Expression.Binar_( Sign, Left, Right ) )
    {
    if( Sign != '=' )
      {
      Solve( new RatInEq, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 1, 1 ) );
      Solve( new SysInEq, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 0, 2 ) );
      Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
      return Final(true);
      }
    Solve( new TSolvDetLinEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 0, 0 ) );
    if( m_pButtonBox->m_ButtonCount == 1 ) return Final( true );

    Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
    Solve( new TSolvDisQuaEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 1, 0 ) );
    Solve( new TSolvQuaEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 0, 1 ) );
    Solve( new TSolvDetVieEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 1, 1 ) );
    Solve( new TSolvDetQuaEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 0, 2 ) );
    if( m_pButtonBox->m_ButtonCount > 0 ) return Final( true );

    Solve( new TSolvCalcPolinomEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 2, 2 ) );
    Solve( new TSolvCalcEquation, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 2, 2 ) );
    Solve( new Log1Eq, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 0, 1 ) );
    Solve( new ExpEq, WinTesting::sm_pBottomWindow->GetCalcButton( 1, 1, 2 ) );
    Solve( new TSolvCalcDetBiQuEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 1, 2 ) );
    Solve( new TSolvFractRatEq, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 2, 0 ) );
    Solve( new TSolvCalcIrratEq, WinTesting::sm_pBottomWindow->GetCalcButton( 2, 2, 1 ) );
    if( Formula.indexOf( "sin" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 0, 0 ) );
    if( Formula.indexOf( "tan" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 0, 1 ) );
    if( Formula.indexOf( "cos" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 1, 0 ) );
    if( Formula.indexOf( "cot" ) != -1 )
      Solve( new TSolvCalcSimpleTrigoEq, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 1, 1 ) );
    Solve( new TSolvCalcSinCosEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 0, 2 ) );
    Solve( new TSolvCalcTrigoEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 1, 2 ) );
//    Solve( new TSolvFractRatEq, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 2, 0 ) );
    Solve( new TSolvCalcHomogenTrigoEqu, WinTesting::sm_pBottomWindow->GetCalcButton( 3, 2, 1 ) );
    return Final(true);
    }
  Prompt = X_Str( "MCanAlsoCalculate", "You can also calculate:" );
  int PosPow = Formula.indexOf( '^' );
  if( PosPow != -1 && Formula.indexOf( '^', PosPow + 1 ) != -1 )
    {
    int iPos2 = Formula.indexOf( '2' );
    if( iPos2 != -1 && Formula.indexOf( '2', iPos2 + 1 ) != -1 )
      {
      Solve( new TSolvSumCub, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 1, 1 ) );
      Solve( new TSolvSubCub, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 2, 1 ) );
      Solve( new TSolvSqrSubSum, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 2, 0 ) );
      Solve( new TSolvSqrSubSum, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 1, 0 ) );
      Solve( new TSolvSubSqr, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 0, 1 ) );
      if( m_pButtonBox->m_ButtonCount > 0 ) return Final( true );
      }
    }
  Solve( new TSolvSumCub, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 1, 1 ) );
  Solve( new TSolvSqrSubSum, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 2, 0 ) );
  Solve( new TSolvSqrSubSum, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 1, 0 ) );
  Solve( new TSolvSubSqr, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 0, 1 ) );
  Solve( new TSolvSubCub, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 2, 1 ) );
  if( m_pButtonBox->m_ButtonCount > 0 ) return Final( true );
  Solve( new TSolvReToMult, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 0, 0 ) );
  Solve( new TSolvExpand, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 0, 2 ) );
  Solve( new TTrinom, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 1, 2 ) );
  if( m_pButtonBox->m_ButtonCount > 0 ) return Final( true );
  Solve( new Alg1Calculator, WinTesting::sm_pBottomWindow->GetCalcButton( 0, 2, 2 ) );
  Final(true);
  }

BottomWindow::BottomWindow() : m_pTitle(new QLabel), m_pPrecision(new QLabel), m_pDetailedCalc( new DetailedCalculator), 
  m_pPrecisionIncr( new CalcButton( "MPrecPlusBtnHint", "Increase precision", ":/Resources/Extras-Forward-icon.png", true ) ),
  m_pPrecisionDecr( new CalcButton( "MPrecMinusBtnHint", "Decrease precision", ":/Resources/Extras-Backward-icon.png", true ) ),
  m_pAngleMeas( new CalcButton( "MRadDegHint", "Angle Measure", "", true ) ), m_pSolverWidet( new SolverWidget ), m_pVCalcLayout( new QVBoxLayout ),
  m_pSWitchCalc( new CalcButton( "MDisplayKeys", "Keys", "", true ) ), m_pCalculator( new QGroupBox), m_pMainLayout( new QHBoxLayout),
  m_pCalcTitle( new QLabel ), m_TrigonomSystem(TExpr::tsRad)
  {
  WinTesting::sm_pBottomWindow = this;
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  XPGedit::sm_ResetTestMode = ResetTestMode;
  XPGedit::sm_AutoTest = AutoTest;
  QGroupBox *pGroupBox = new QGroupBox();
  PanelButton::sm_ButtonHeight = WinTesting::sm_pPanel->width() / 3;
  QPushButton *pMainTaskBtn = new PanelButton( ":/Resources/MainHelp.png" );
  connect( pMainTaskBtn, SIGNAL( clicked() ), SLOT( MainTaskClick() ) );
  QVBoxLayout *pVLayout = new QVBoxLayout;
  pVLayout->addWidget( pMainTaskBtn );
  pVLayout->setAlignment( pMainTaskBtn, Qt::AlignHCenter | Qt::AlignCenter );
  pGroupBox->setLayout( pVLayout );
  pGroupBox->setFixedWidth( WinTesting::sm_pPanel->width() );
  QHBoxLayout *pCalcLayout = new QHBoxLayout;
  QVBoxLayout *pEdLayout = new QVBoxLayout;
  m_pTitle->setAlignment( Qt::AlignCenter );
  m_pTitle->setText( X_Str( "EditWindowCaption", "Edit Window   press Enter to save or use calculator to evaluate" ) );
  m_pTitle->setStyleSheet( "QLabel {font-size:16px}" );
  pEdLayout->addWidget( m_pTitle );
  m_pTitle->adjustSize();
  m_pMainLayout->setMargin( 0 );
  m_pMainLayout->addWidget( pGroupBox );
  m_pMainLayout->addLayout( pCalcLayout );
  m_pCalculator->setStyleSheet( "QWidget {font-size:16px}" );
  QHBoxLayout *pHLayout = new QHBoxLayout;
  pHLayout->addWidget( m_pAngleMeas );
  m_pAngleMeas->setStyleSheet( "QWidget {margin-left:16px}" );
  connect( m_pAngleMeas, SIGNAL( clicked() ), SLOT( ChangeAngleMeas() ) );
  pHLayout->addWidget( m_pSWitchCalc );
  connect( m_pSWitchCalc, SIGNAL( clicked() ), SLOT( SwitchCalc() ) );
  pHLayout->addWidget( m_pPrecisionDecr );
  connect( m_pPrecisionDecr, SIGNAL( clicked() ), SLOT( PrecDecr() ) );
  pHLayout->addWidget( m_pPrecisionIncr );
  connect( m_pPrecisionIncr, SIGNAL( clicked() ), SLOT( PrecIncr() ) );
  m_pPrecision->setStyleSheet( "QWidget {margin-right:10px}" );
  pHLayout->addWidget( m_pPrecision );
  m_pCalcTitle->setAlignment( Qt::AlignCenter );
  m_pVCalcLayout->addWidget( m_pCalcTitle );
  m_pVCalcLayout->addLayout( pHLayout );
  m_pVCalcLayout->setMargin( 0 );
  m_pDetailedCalc->adjustSize();
//  QSize G = m_pSolverWidet->size();
  m_pSolverWidet->setFixedSize( m_pDetailedCalc->size() );
//  G = m_pSolverWidet->size();
  m_pVCalcLayout->addWidget( m_pDetailedCalc );
//  m_pVCalcLayout->addWidget( m_pSolverWidet );
  m_pCalculator->setLayout( m_pVCalcLayout );
  m_pCalculator->setFixedWidth( WinTesting::sm_pPanel->width() * 2.2 );
  m_FullEditorWidth = ScreenSize.width() - pGroupBox->width() - 20;
  m_EditorWidth = m_FullEditorWidth - m_pCalculator->width();
  m_pCalculator->show();
  m_pDetailedCalc->show();
  Panel::sm_pEditor = new XPGedit( this, BaseTask::sm_pEditSets );
  QScrollArea *pArea = Panel::sm_pEditor->SetSize(QSize(m_EditorWidth, ScreenSize.height() / 4 - m_pTitle->height()));
  pEdLayout->addWidget( pArea );
  pCalcLayout->addLayout(pEdLayout);
  pCalcLayout->addWidget(m_pCalculator);
  setLayout(m_pMainLayout);
  ShowPrecision();
  ShowAngleMeas();
  LangSwitch();
  m_pAngleMeas->adjustSize();
  m_pSWitchCalc->setFixedHeight( m_pAngleMeas->height() );
//  G = m_pSolverWidet->size();
  }

QPushButton* BottomWindow::GetCalcButton( int Widget, int row, int col )
  {
  return m_pDetailedCalc->GetCalcWidget( Widget )->GetButton( row, col );
  }

void BottomWindow::ShowCalculator(bool Show)
  {
  if( Show )
    {
    m_pCalculator->show();
    Panel::sm_pEditor->SetSize(QSize(m_EditorWidth, Panel::sm_pEditor->height()) );
    Panel::sm_pEditor->setFixedWidth(m_EditorWidth);
    WinTesting::sm_pShowCalcualtor->setText("Hide &Calculator");
    return;
    }
  m_pCalculator->hide();
  WinTesting::sm_pShowCalcualtor->setText("Show &Calculator");
  Panel::sm_pEditor->SetSize(QSize(m_FullEditorWidth, Panel::sm_pEditor->height()) );
  Panel::sm_pEditor->setFixedWidth(m_FullEditorWidth);
  }

void BottomWindow::ShowCalculator()
  {
  ShowCalculator( !m_pCalculator->isVisible() );
  }

void BottomWindow::SwitchCalc()
  {
  Panel::sm_pEditor->setFocus();
  if( m_pDetailedCalc->isVisible() )
    {
    m_pSolverWidet->show();
    m_pVCalcLayout->replaceWidget( m_pDetailedCalc, m_pSolverWidet );
    m_pDetailedCalc->hide();
    m_pSWitchCalc->setToolTip( X_Str( "MDisplayCalculatorKeys", "Display Calculator Keys" ) );
    return;
    }
  m_pDetailedCalc->show();
  m_pVCalcLayout->replaceWidget( m_pSolverWidet, m_pDetailedCalc );
  m_pSolverWidet->hide();
  m_pSWitchCalc->setToolTip( X_Str( "MHideCalculatorKeys", "Hide Calculator Key" ) );
  }

void BottomWindow::MainTaskClick()
  {
  Panel::sm_pEditor->setFocus();
  s_MainTask.HelpShow();
  }

void BottomWindow::LangSwitch()
  {
  m_pTitle->setText( X_Str( "EditWindowCaption", "Edit Window   press Enter to save or use calculator to evaluate" ) );
  m_pCalcTitle->setText( X_Str( "MCalculator", "Calculator" ) );
  m_pPrecision->setToolTip( X_Str( "MPanelPrecHint", "Precision of Calculations" ) );
  m_pAngleMeas->setToolTip( X_Str( "MRadDegHint", "Angle Measure" ) );
  ShowAngleMeas();
  m_pPrecisionIncr->LangSwitch();
  m_pPrecisionDecr->LangSwitch();
  m_pDetailedCalc->LangSwitch();
  m_pSolverWidet->LangSwitch();
  m_pSWitchCalc->setText( X_Str( "MDisplayKeys", "Keys" ) );
  if( m_pDetailedCalc->isVisible() )
    m_pSWitchCalc->setToolTip( X_Str( "MHideCalculatorKeys", "Hide Calculator Key" ) );
  else
    m_pSWitchCalc->setToolTip( X_Str( "MDisplayCalculatorKeys", "Display Calculator Keys" ) );
  }

void BottomWindow::ShowPrecision()
  {
  m_pPrecision->setText( QString::number( s_Precision, 'f', 9 ) );
  Panel::sm_pEditor->setFocus();
  }

void BottomWindow::ShowAngleMeas()
  {
  if( TExpr::sm_TrigonomSystem == TExpr::tsRad )
    m_pAngleMeas->setText( X_Str( "MRad", "Rad" ) );
  else
    m_pAngleMeas->setText( X_Str( "MDeg", "Deg" ) );
  m_TrigonomSystem = TExpr::sm_TrigonomSystem;
  Panel::sm_pEditor->setFocus();
  }

void BottomWindow::PrecIncr()
  {
  if( s_Precision < 1e-8 ) return;
  s_OldPrecision = s_Precision /= 10.0;
  TExpr::sm_Accuracy = s_Precision;
  ShowPrecision();
  }

void BottomWindow::PrecDecr()
  {
  if( s_Precision > 0.1 ) return;
  s_OldPrecision = s_Precision *= 10.0;
  TExpr::sm_Accuracy = s_Precision;
  ShowPrecision();
  }

void BottomWindow::ChangeAngleMeas()
  {
  if( TExpr::sm_TrigonomSystem == TExpr::tsRad )
    TExpr::sm_TrigonomSystem = TExpr::tsDeg;
  else
    TExpr::sm_TrigonomSystem = TExpr::tsRad;
  ShowAngleMeas();
  }

CentralWindow::CentralWindow() : m_pTitle( new QLabel )
  {
  QVBoxLayout *pVLayout = new QVBoxLayout;
  m_pTitle->setAlignment( Qt::AlignCenter );
  m_pTitle->setText( X_Str( "OutWin_Caption", "WorkSheet                             use mouse keys to select text or to change settings" ) );
  pVLayout->addWidget( m_pTitle );
  m_pTitle->setStyleSheet( "QLabel {font-size:16px}" );
  pVLayout->addWidget( WinTesting::sm_pOutWindow = new OutWindow(this) );
  pVLayout->setMargin( 0 );
  setLayout( pVLayout );
  }

void AfterMsg() { Panel::sm_pEditor->setFocus(); }

//WinTesting::WinTesting() : m_Review(false), m_pPlotter(nullptr), m_pMultiPlotter(nullptr)
WinTesting::WinTesting() : m_Review(false), m_pMultiPlotter(nullptr)
  {
#ifdef Q_OS_ANDROID
  iFontSize = 64;
  iPowDecrease = 24;
  iPenWidth = 2;
#else
  iFontSize = 16;
  iPowDecrease = 5;
  iPenWidth = 1;
#endif
  sm_pMainWindow = this;
  s_AfterMsg = AfterMsg;
  RichTextDocument::sm_TempPath = QDir::tempPath();
  XPGedit::sm_ChangeState = ChangeEditor;
  setWindowIcon( QIcon( ":/Resources/xpress99.ico" ) );
  QString FontMath( "Cambria Math" );
  QFont MainFont( FontMath, iFontSize, QFont::Normal );
  QFont PowerFont( MainFont );
  PowerFont.setPointSize( MainFont.pointSize() - iPowDecrease );
  BaseTask::sm_pEditSets = new EditSets( MainFont, PowerFont, PowerFont, QString( "white" ), QString( "red" ), QString( "black" ) );
  sm_pPanel = new Panel( this );
  sm_pPanel->adjustSize();
  connect( sm_pPanel, SIGNAL( ExitBtn() ), SLOT( close() ) );
  sm_pMenuBar = new QMenuBar;
  m_pFile = new QMenu( "&File" );
  m_pOpenTask = m_pFile->addAction( QIcon( ":/Resources/fileopen.png" ), "&Open Task File", this, SLOT( slotOpenTaskFile() ), QKeySequence( "CTRL+O" ) );
  m_pEditTask = m_pFile->addAction( QIcon( ":/Resources/fileopen.png" ), "&Edit Task File", this, SLOT( slotEditTaskFile() ), QKeySequence( "CTRL+E" ) );
  m_pCreateTask = m_pFile->addAction( QIcon( ":/Resources/NewTask.jpg" ), "&Create Task", this, SLOT( slotCreateTask() ), QKeySequence( "CTRL+N" ) );
  m_pOpenStack = m_pFile->addAction( QIcon( ":/Resources/NewTask.jpg" ), "&Create Task from Stack", this, SLOT( slotOpenStack() ), QKeySequence( "CTRL+M" ) );
  sm_pSaveTaskFile = m_pFile->addAction( QIcon( ":/Resources/filesave.png" ), "&Save Task File", this, SLOT( slotSaveTaskFile() ), QKeySequence( "CTRL+S" ) );
//  m_pWebTask = m_pFile->addAction( QIcon( ":/Resources/WebOpen.png" ), "Open Task from &Web", this, SLOT( BrowseTask() ), QKeySequence( "CTRL+W" ) );
  sm_pSaveTaskFile->setEnabled( false );
  m_pFile->addSeparator();
  m_pSaveFormula = m_pFile->addAction( QIcon( ":/Resources/btnSaveHistory.jpg" ), "Save E&xpression", this, SLOT( SaveExpression() ), QKeySequence( "CTRL+X" ) );
  m_pRestoreFormula = m_pFile->addAction( QIcon( ":/Resources/btnRestoreHistory.jpg" ), "&Restore Expression", this, SLOT( RestoreExpression() ), QKeySequence( "CTRL+R" ) );;
  m_pFile->addSeparator();
  m_pQuit = m_pFile->addAction( "&Quit", this, SLOT( Quit() ), QKeySequence( "CTRL+Q" ) );
  sm_pMenuBar->addMenu( m_pFile );
  QMenu* pLanguage = new QMenu( "&Language" );
  sm_English = pLanguage->addAction( "&English", this, [&] () { slotChangeLanguage( lngEnglish ); }, QKeySequence( "CTRL+L" ) );
  sm_English->setCheckable( true );
  sm_English->setChecked( false );
  sm_Hebrev = pLanguage->addAction( "&Hebrew", this, [&] () { slotChangeLanguage( lngHebrew ); }, QKeySequence( "CTRL+H" ) );
  sm_Hebrev->setCheckable( true );
  sm_Hebrev->setChecked( true );
  sm_Hebrev->setEnabled( false );
  sm_Bulgarian = pLanguage->addAction( "&Bulgarian", this, [&] () { slotChangeLanguage( lngBulgarian ); }, QKeySequence( "CTRL+B" ) );
  sm_Bulgarian->setCheckable( true );
  sm_Bulgarian->setChecked( false );
  sm_Russian = pLanguage->addAction( "&Russian", this, [&] () { slotChangeLanguage( lngRussian ); }, QKeySequence( "CTRL+R" ) );
  sm_Russian->setCheckable( true );
  sm_Russian->setChecked( false );
  sm_Ukrainian = pLanguage->addAction( "&Ukrainian", this, [&] () { slotChangeLanguage( lngUkrainian ); }, QKeySequence( "CTRL+U" ) );
  sm_Ukrainian->setCheckable( true );
  sm_Ukrainian->setChecked( false );

  sm_pMenuBar->addMenu( pLanguage );
  sm_pEditor = new QMenu( "&Task Editor" );
  sm_pEditHeader = sm_pEditor->addAction( "Edit &Header", this, SLOT( EditTaskHeader() ) );
  sm_pEditCalc = sm_pEditor->addAction( "Edit &Calculation", this, SLOT( EditTaskCalc() ) );
  sm_pAddPicture = sm_pEditor->addAction( "Add &Pictures", this, SLOT( AddPictures() ) );
  m_pShowFunctions = sm_pEditor->addAction( "Show &Functions", this, SLOT( ShowFunctions() ) );
  m_pShowExpression = sm_pEditor->addAction( "Show &Expression", this, SLOT( ShowExpression() ) );
  m_pHyperlink = sm_pEditor->addAction( "Insert H&yperlink", this, SLOT( AddHyperlink() ) );
  m_pEditTracks = sm_pEditor->addAction( "Edit &Tracks", this, SLOT( EditTracks() ) );
  m_pSelectTrack = sm_pEditor->addAction( "&Select Track", this, SLOT( SelectTrack() ) );
  m_pSelectTrack->setEnabled( false );
  sm_pMenuBar->addMenu( sm_pEditor );
  sm_pEditor->setEnabled( false );
  sm_pToolBar = new QToolBar;
  sm_pToolBar->setStyleSheet( "QToolBar {border-style:inset;border-width:1px;border-color:#999999}" );
  m_pOpenTaskBtn = sm_pToolBar->addAction( QIcon( ":/Resources/fileopen.png" ), "&Open Task File", this, SLOT( slotOpenTaskFile() ) );
  sm_pShowGreek = sm_pToolBar->addAction( QIcon( ":/Resources/alpha.jpg" ), "Show &Greek Symbols", this, SLOT( slotShowGreek() ) );
  sm_pShowBigGreek = sm_pToolBar->addAction( QIcon( ":/Resources/Alpha-Large.jpg" ), "Show &Big Greek Symbols", this, SLOT( slotShowBigGreek() ) );
  sm_pShowMathSymbols = sm_pToolBar->addAction( QIcon( ":/Resources/Math.jpg" ), "Show &Math Symbols", this, SLOT( slotShowMathSymbols() ) );
  sm_pShowUnar = sm_pToolBar->addAction( QIcon( ":/Resources/ShowUnar.jpg" ), "Show &Unar brackets", this, SLOT( btnShowUnarClick() ) );
  sm_pShowUnarByAdd = sm_pToolBar->addAction(QIcon(":/Resources/ShowUnarByAdd.jpg"), "Show Unar brackets by &Addition", this, SLOT(btnShowUnarByAddClick()));
  sm_pShowMultSign = sm_pToolBar->addAction( QIcon( ":/Resources/ShowMult.jpg" ), "Show Multiplication &Sign", this, SLOT( btnShowMultSignClick() ) );
  addDockWidget( Qt::LeftDockWidgetArea, new DockWithoutTitle( sm_pPanel ) );
  setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
  addDockWidget( Qt::TopDockWidgetArea, new DockWithoutTitle( new QuestWindow( this ) ) );
  addDockWidget( Qt::BottomDockWidgetArea, new DockWithoutTitle( new BottomWindow ) );
  sm_pShowCalcualtor = sm_pToolBar->addAction(QIcon(":/Resources/abacus.png"), "Show &Calculator", sm_pBottomWindow, SLOT(ShowCalculator()));
  sm_pMultiPlot = sm_pToolBar->addAction( QIcon(":/Resources/plotter.png"), "Plot Graph", this ,SLOT(MultiPlot()));
  sm_pCalcDeriv = sm_pToolBar->addAction( QIcon(":/Resources/GraphBtnDerive.png"), "Calculate Derivative", this ,SLOT(CalcDeriv()));

  setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );
  setCentralWidget( new CentralWindow );
  m_pWaitMessage = new QLabel( WinTesting::sm_pOutWindow );
  m_pWaitMessage->setStyleSheet(
    "QLabel {  background:lightgray;border-width:10px;border-style:solid;border-color:blue;border-radius:25;font-weight:bold;font-size:20pt;color:red }" );
  m_pWaitMessage->hide();
  XPInEdit::sm_Language = lngEnglish;
  MainTab::sm_Help = TaskHelpShow; 
  MainTab::sm_Hint = TaskHintShow; 
  MainTab::sm_Enter = AnsverEnter; 
  MainTab::sm_NewData = NewData;
  slotChangeLanguage();
  }

void WinTesting::Quit()
  {
  sm_pPanel->EmitExit();
  }

void	WinTesting::resizeEvent( QResizeEvent *pE )
  {
  }

void WinTesting::closeEvent(QCloseEvent *event)
  {
  Panel::sm_pPanel->HideSymbols();
  if(sm_pSaveTaskFile->isEnabled())
    switch( QMessageBox::question( nullptr, "Task Not Saved",
      "Task " + s_Task.m_pFile->fileName() + " was not saved, save or ignore exit?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore  ) )
      {
      case QMessageBox::Yes:
        slotSaveTaskFile();
        return;
      case QMessageBox::Ignore:
        event->ignore();
      case QMessageBox::No:
        return;
      }
  if( s_Task.m_WorkMode == wrkExam && QMessageBox::question( nullptr, X_Str( "MCancelSelfTest", "Cancel" ),
    X_Str( "MCancelAnswer", "Do you cancel the answer ?" ) ) != QMessageBox::Yes )
    event->ignore();
  }

void WinTesting::slotChangeLanguage( TLanguages Lang )
  {
  QAction* LangActions[] = { sm_Hebrev, sm_English, sm_Russian, sm_Bulgarian, sm_Ukrainian };
  for( TLanguages L = lngHebrew; L <= lngUkrainian; L = ( TLanguages ) ( L + 1 ) )
    {
    LangActions[L]->setEnabled( true );
    LangActions[L]->setChecked( false );
    }
  LangActions[Lang]->setEnabled( false );
  LangActions[Lang]->setChecked( true );
  XPInEdit::sm_Language = Lang;
//#ifndef DEBUG_TASK
  if( Lang == lngHebrew )
    s_MainTask.SetFileName( s_AppPath + "/Main/MAIN.heb" );
//    s_MainTask.SetFileName( ":/Resources/Main/MAIN.heb" );
  else
    s_MainTask.SetFileName( s_AppPath + "/Main/main.tsk" );
//    s_MainTask.SetFileName( ":/Resources/Main/main.tsk" );
//#endif
  EdStr::sm_pCodec = QTextCodec::codecForName( Lang == lngHebrew ? "Windows-1255" : "Windows-1251" );
  m_pEditTask->setText( X_Str( "ActionEditTask", "Edit Task File" ) );
  m_pOpenTask->setText( X_Str( "ActionOpenTask", "Open Task File" ) );
  sm_pSaveTaskFile->setText( X_Str( "ActionSaveTask", "Save Task File" ) );
  m_pFile->setTitle( X_Str( "MFile_Caption", "File" ) );
  m_pQuit->setText( X_Str( "MExit_Caption", "Exit" ) );
  m_pOpenTaskBtn->setText( X_Str( "ActionOpenTask", "Open Task File" ) );
  sm_pBottomWindow->LangSwitch();
  sm_pOutWindow->SetTitle( X_Str( "OutWin_Caption", "WorkSheet                             use mouse keys to select text or to change settings" ) );
  sm_pQuestWindow->m_pRTWindow->ResetLanguage();
  sm_pShowGreek->setToolTip( X_Str( "TabSheetSmallGreekCaption", "Greek" ) );
  sm_pShowBigGreek->setToolTip( X_Str( "TabSheetBigGreekCaption", "Capital Greek" ) );
  sm_pShowMathSymbols->setToolTip( X_Str( "TabSheetSymbolCaption", "Symbols" ) );
  if( BaseTask::sm_GlobalShowMultSign )
    sm_pShowMultSign->setToolTip( X_Str( "MHideMultSign", "Hide Multiplication Sign" ) );
  else
    sm_pShowMultSign->setToolTip( X_Str( "MShowMultSign", "Show Multiplication Sign" ) );
  if( BaseTask::sm_GlobalShowUnarMinus )
    sm_pShowUnar->setToolTip( X_Str( "MHideUnar", "Hide Unar Minus" ) );
  else
    sm_pShowUnar->setToolTip( X_Str( "MShowUnar", "Show Unar Minus" ) );
  if (BaseTask::sm_GlobalShowMinusByAddition)
    sm_pShowUnarByAdd->setToolTip(X_Str("MHideUnarByAdd", "Hide Minus By Addition"));
  else
    sm_pShowUnarByAdd->setToolTip(X_Str("MShowUnarByAdd", "Show Minus By Addition"));
  sm_pPanel->LangSwitch();
  }

void WinTesting::slotShowGreek()
  {
  Panel::sm_pGreek->show();
  }

void WinTesting::slotShowBigGreek()
  {
  Panel::sm_pBigGreek->show();
  }

void WinTesting::slotShowMathSymbols()
  {
  Panel::sm_pMathSymbol->show();
  }

void WinTesting::btnShowUnarClick()
  {
  BaseTask::sm_GlobalShowUnarMinus = !BaseTask::sm_GlobalShowUnarMinus;
  SetBtnShowUnar();
  }

void WinTesting::btnShowUnarByAddClick()
  {
  BaseTask::sm_GlobalShowMinusByAddition = !BaseTask::sm_GlobalShowMinusByAddition;
  SetBtnShowUnarByAdd();
  }

void WinTesting::btnShowMultSignClick()
  {
  BaseTask::sm_GlobalShowMultSign = !BaseTask::sm_GlobalShowMultSign;
  SetBtnShowMultSign();
  }

void WinTesting::SetBtnShowUnar()
  {
  if (BaseTask::sm_GlobalShowUnarMinus)
    sm_pShowUnar->setToolTip(X_Str("MHideUnar", "Hide Unar Minus"));
  else
    sm_pShowUnar->setToolTip(X_Str("MShowUnar", "Show Unar Minus"));
  }

void WinTesting::SetBtnShowUnarByAdd()
  {
  if (BaseTask::sm_GlobalShowMinusByAddition)
    sm_pShowUnarByAdd->setToolTip(X_Str("MHideUnarByAdd", "Hide Minus by Addition"));
  else
    sm_pShowUnarByAdd->setToolTip(X_Str("MShowUnarByAdd", "Show Minus by Addition"));
  }

void WinTesting::SetBtnShowMultSign()
  {
  if (BaseTask::sm_GlobalShowMultSign)
    sm_pShowMultSign->setToolTip(X_Str("MHideMultSign", "Hide Multiplication Sign"));
  else
    sm_pShowMultSign->setToolTip(X_Str("MShowMultSign", "Show Multiplication Sign"));
  }

void WinTesting::ClearXPWindows()
  {
  Panel::sm_pEditor->Clear();
  Panel::sm_pEditor->RefreshXPE();
  sm_pOutWindow->Clear();
  sm_pQuestWindow->Clear();
  sm_pBottomWindow->ClearButtons();
  }

void WinTesting::NewData() 
  {
  if( sm_pSaveTaskFile->isEnabled() && QMessageBox::question( nullptr, "Task Not Saved",
    "Task " + s_Task.m_pFile->fileName() + " was not saved, save?" ) == QMessageBox::Yes ) WinTesting::sm_pMainWindow->slotSaveTaskFile();
  /*
  if( s_TaskEditorOn )
    {
    s_Task.m_pCalc->Calculate();
    sm_pQuestWindow->SetContent( s_Task.m_pQuestion );
    }
  else
  */
    NewXPRESSTask( sm_TaskPath ); 
  }

void WinTesting::NewXPRESSTask( const QString& TaskFileName, bool Edit )
  {
  ClearXPWindows();
  s_Task.Clear();
  s_Task.sm_EditTask = Edit;
  sm_pSaveTaskFile->setEnabled( false );
  sm_pBottomWindow->ShowCalculator(false);
  StartXPRESSTask( TaskFileName, Edit );
  //      if( Assigned( GraphEdit ) && ( ( GraphEdit.TrainingType == LinInEq ) || GeoActivate ) )
  //        ActivateForm( GraphEdit );
  Panel::sm_GeoActivate = false;
  //        XPStatus.StMessage = "";
  }

void WinTesting::StartXPRESSTask( const QString& FName, bool Edit )
  {
  s_TaskEditorOn = Edit;
  s_ShowMinute = false;
  sm_pEditor->setEnabled( Edit );
  TLanguages L = BaseTask::GetLangByFileName( FName );
  if( L != XPInEdit::sm_Language )
    sm_pMainWindow->slotChangeLanguage(L);
  ExpStore::sm_pExpStore->Init_var();
  if( !FName.isEmpty() )
    s_Task.SetFileName( FName );
  else
    s_Task.LoadFromFile();

  //    if( XPRESSCalculator != nullptr ) XPRESSCalculator.PanelPrecision.Caption = CalcStrPrecision;

  /*
    if( XPMonitorMode )
    {
    QuestWin.Visible = false;
    HFormMan.LabelSize = HFormMan.ExpLabelSize;
    };
    */
  try
    {
    s_Task.CalcRead();
    TXPTask::sm_NewHebFormat = TXPTask::sm_NewHebFormat || s_Task.GetLanguage() != lngHebrew;
    s_Task.SetGlobalSWhowRad( TExpr::sm_TrigonomSystem == TExpr::tsRad );
   sm_pQuestWindow->SetContent( s_Task.m_pQuestion );
    if( s_Task.m_WorkMode == wrkExam )
      s_Task.StartExam();
    }
  catch( QString Err )
    {
    QMessageBox::critical( nullptr, "Task Error", Err );
    }
  catch( ErrParser Err )
    {
    QMessageBox::critical( nullptr, "Task Error", Err.Message() );
    }
  //    WasDefinition = false;

  //    XPStatus.DfMessage = X_str( "XPStatMess", "MReadyToEnter", "Ready" );
  }

QString WinTesting::SelectTask()
  {
  QString Extends( "Hebrew(*.heb)\nEnglish(*.tsk)\nBulgarian(*.tbg)\nRussian(*.tru)\nUkrainian(*.ukr)" );
  if( XPInEdit::sm_Language == lngEnglish )
    Extends = "English(*.tsk)\nHebrew(*.heb)\nBulgarian(*.tbg)\nRussian(*.tru)\nUkrainian(*.ukr)";
  if( XPInEdit::sm_Language == lngRussian )
    Extends = "Russian(*.tru)\nEnglish(*.tsk)\nHebrew(*.heb)\nBulgarian(*.tbg)\nUkrainian(*.ukr)";
  if( XPInEdit::sm_Language ==lngBulgarian )
    Extends = "Bulgarian(*.tbg)\nEnglish(*.tsk)\nHebrew(*.heb)\nRussian(*.tru)\nUkrainian(*.ukr)";
  if( XPInEdit::sm_Language ==lngUkrainian )
    Extends = "Ukrainian(*.ukr)\nEnglish(*.tsk)\nHebrew(*.heb)\nRussian(*.tru)\nBulgarian(*.tbg)";
  QString FileName = QFileDialog::getOpenFileName( nullptr, X_Str( "LangSwitch_Title", "Task Selecting Dialog" ),
    sm_TaskPath, Extends, nullptr, QFileDialog::ReadOnly );
  if( !FileName.isEmpty() ) sm_TaskPath = FileName;
  return FileName;
  }

void WinTesting::slotOpenTaskFile(bool Edit)
  {
  ExpStore::sm_pExpStore->Clear();
#ifdef LEAK_DEBUG
  qDebug() << "GrEl Created: " << TXPGrEl::sm_CreatedCount << ", GrElDeleted: " << TXPGrEl::sm_DeletedCount;
  TXPGrEl::sm_CreatedCount = TXPGrEl::sm_DeletedCount = 0;
  qDebug() << "Expr Created: " << TExpr::sm_CreatedCount << ", deleted: " << TExpr::sm_DeletedCount << " Created List count: " << TExpr::sm_CreatedList.count();
  if( TExpr::sm_CreatedCount > TExpr::sm_DeletedCount )
    {
    qDebug() << "Unremoved expressions";
    for( auto pExpr = TExpr::sm_CreatedList.begin(); pExpr != TExpr::sm_CreatedList.end(); pExpr++ )
      qDebug() << ( *pExpr )->m_Contents;
    }
#endif
  if( sm_pSaveTaskFile->isEnabled() && QMessageBox::question( nullptr, "Task Not Saved",
    "Task " + s_Task.m_pFile->fileName() + " was not saved, save?" ) == QMessageBox::Yes ) slotSaveTaskFile();
  QString FileName = SelectTask();
  if( FileName.isEmpty() ) return;
  QDir::setCurrent(FileName.left(FileName.lastIndexOf('/')) );
  sm_TranslateObjects.clear();
  HideHelps();
  MainTab::sm_NewData = NewData;
  s_TaskEditorOn = Edit;
  sm_pSaveTaskFile->setEnabled( Edit );
  try
    {
    NewXPRESSTask( FileName, Edit );
    m_pSelectTrack->setEnabled( Edit && s_Task.m_pTrack->m_MultyTrack );
    }
  catch( ErrParser& ErrMsg )
    {
    QMessageBox::critical( nullptr, "Task Error", "Error Name: " + ErrMsg.Name() + " Message: " + ErrMsg.Message() );
    }
  }

void WinTesting::slotCreateTask()
  {
  HideHelps();
  s_Task.sm_EditTask = true;
  ClearXPWindows();
  s_Task.Clear();
  sm_pEditor->setEnabled( false );
  TaskHeaderEditor HE( this, &s_Task );
  if(HE.Cancelled() || HE.exec() == QDialog::Rejected )
    {
    sm_pEditor->setEnabled( true );
    return;
    }
  s_TaskEditorOn = true;
  sm_pQuestWindow->SetContent( s_Task.m_pQuestion );
  EditTaskCalc();
  EditTracks();
  sm_pEditor->setEnabled( true );
  ExpStore::sm_pExpStore->Init_var();
  }

void WinTesting::slotSaveTaskFile( bool bNewName )
  {
  QString FileName = s_Task.m_pFile->fileName();
  if( bNewName && !s_Task.m_NewTask )
    {
    QString FileExt = BaseTask::GetFileExtByLang( XPInEdit::sm_Language );
    FileName = QFileDialog::getSaveFileName( nullptr, X_Str( "LangSwitch_Title", "Task Selecting Dialog" ),
      sm_TaskPath, "Task file (*" + FileExt + ')' );
    if( FileName.isEmpty() ) return;
    }
  try
    {
    QByteAStream Stream;
    s_Task.Save( Stream );
    s_Task.m_pFile->Save( Stream, FileName );
    }
  catch( ErrParser& ErrMsg )
    {
    QMessageBox::critical( nullptr, "Error", ErrMsg.Message() );
    }
  sm_pSaveTaskFile->setEnabled( false );
  }

void WinTesting::slotOpenStack()
  {
  QString FileName = QFileDialog::getOpenFileName( nullptr, X_Str( "LangSwitch_Stack_Question", "Selecting STACK File Dialog" ),
    sm_TaskPath, "*.xml", nullptr, QFileDialog::ReadOnly );
  if( FileName.isEmpty() ) return;
  try
    {
    HideHelps();
    s_Task.sm_EditTask = true;
    ClearXPWindows();
    sm_pEditor->setEnabled( false );
    TaskHeaderEditor HE( this, &s_Task );
    if(HE.Cancelled() || HE.exec() == QDialog::Rejected )
      {
      sm_pEditor->setEnabled( true );
      return;
      }
    s_TaskEditorOn = true;
    sm_pQuestWindow->SetContent( s_Task.m_pQuestion );
    s_Task.LoadFromStack(FileName);
    EditTaskCalc();
    EditTracks();
    sm_pEditor->setEnabled( true );
    ExpStore::sm_pExpStore->Init_var();
    }
  catch( ErrParser& ErrMsg )
    {
    QMessageBox::critical( nullptr, "Error", ErrMsg.Message() );
    }
  }

bool WinTesting::event( QEvent *event )
  {
  if( event->type() == QEvent::ActivationChange )
    {
    static bool NoChanged = true;
    if( NoChanged ) SetWindowPos( ( HWND ) winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
    NoChanged = false;
    return QMainWindow::event( event );
    }
  if( event->type() == QEvent::Hide )
    {
//    if( m_pPlotter != nullptr ) m_pPlotter->close();
//    delete m_pPlotter;
//    m_pPlotter = nullptr;
    return QMainWindow::event( event );
    }
  static bool SplitOutWindow = false;
  if( SplitOutWindow && event->type() == QEvent::MouseButtonPress )
    {
    QMouseEvent *pMEvent = dynamic_cast< QMouseEvent* >( event );
    if( pMEvent->button() == Qt::LeftButton )
      {
      sm_pQuestWindow->m_pRTWindow->setMinimumHeight( 100 );
      SplitOutWindow = false;
      }
    }
  if( event->type() == QEvent::CursorChange )
    {
    QCursor Cursor = cursor();
    if( Cursor.shape() == Qt::SplitVCursor )
      {
      QRect Geometry = sm_pOutWindow->geometry();
      QPoint P = sm_pOutWindow->mapToGlobal( Geometry.topLeft() );
      if( Cursor.pos().y() < P.y() )
        {
        SplitOutWindow = true;
        return QMainWindow::event( event );
        }
      }
    SplitOutWindow = false;
    }
  if( event->type() == QEvent::KeyboardLayoutChange)
    {
    m_Review = true;
    }
  if( event->type() == QEvent::Show )
    {
//    s_Task.SetWorkMode("wrkExam");
    if( m_Review || sm_ApplicationArguments.count() != 7 )
      {
      sm_pQuestWindow->SetContentsWidth();
      sm_pBottomWindow->ShowCalculator(false);
      return true;
      }
    m_Review = true;
    m_TasksUrl = sm_ApplicationArguments[2];
    m_RootAppUrl = m_TasksUrl.left( m_TasksUrl.lastIndexOf( '/' ) + 1 );
    m_TasksUrl += "/TASKS/";
    m_TopicId = sm_ApplicationArguments[4];
    m_UserId = sm_ApplicationArguments[6];
    s_Task.SetWorkMode( sm_ApplicationArguments[5] );
    QNetworkRequest Request( QUrl( m_RootAppUrl + "GetTask.asp?tpc=" + m_TopicId + "&usr=" + m_UserId + "&mode=" + sm_ApplicationArguments[5] ) );
    m_pReply = sm_NetworkManager.get( Request );
    connect( m_pReply, SIGNAL( finished() ), SLOT( slotReplyTopic() ) );
    return true;
    }
    return QMainWindow::event( event );
  }

void WinTesting::slotReplyTopic()
  {
  QByteArray R = m_pReply->readAll();
  disconnect( m_pReply, SIGNAL( finished() ), this, SLOT( slotReplyTopic() ) );
  m_pReply->deleteLater();
  if( R.left( 4 ) != "NAME" )
    {
    QMessageBox::critical( nullptr, X_Str( "HMMsgDlg_dlgInfoE", "Error" ), X_Str( "LoadTaskNameError", "Load Task Name Error" ) );
    return;
    }
  int iEnd = R.length() - 1;
  for( ; R[iEnd] == ( char ) 10 || R[iEnd] == ( char ) 13; iEnd-- );
  int iStart = iEnd;
  for( ; R[iStart] != ( char ) 10 && R[iStart] != ( char ) 13; iStart-- );
  QByteArray TaskName = R.mid( iStart + 1, iEnd - iStart );
  m_pWaitMessage->setText( X_Str( "WaitTaskLoading", "Please wait while the task loads" ) );
  m_pWaitMessage->show();
  QNetworkRequest Request( QUrl( m_TasksUrl + TaskName ) );
  m_pReply = sm_NetworkManager.get( Request );
  connect( m_pReply, SIGNAL( finished() ), SLOT( slotReplyTask() ) );
  }

void WinTesting::slotReplyTask()
  {
  QByteArray Task = m_pReply->readAll();
  QString FName = m_pReply->url().fileName();
  disconnect( m_pReply, SIGNAL( finished() ), this, SLOT( slotReplyTask() ) );
  m_pReply->deleteLater();
  if( Task.left( 4 ) != "TASK" )
    {
    QMessageBox::critical( nullptr, X_Str( "HMMsgDlg_dlgInfoE", "Error" ), X_Str( "CantLoadTask", "Can't Load Task: " ) + m_pReply->url().toString() );
    return;
    }
  static int NumTsk = 0;
  QString TmpPath( QDir::tempPath() + '/' );
  QFile FTsk( TmpPath + "Task" + QString::number( NumTsk++ ) + FName.right( FName.length() - FName.lastIndexOf( '.' ) ) );
  if( !FTsk.open( QIODevice::WriteOnly ) )
    {
    QMessageBox::critical( nullptr, X_Str( "HMMsgDlg_dlgInfoE", "Error" ), X_Str( "CantCreateTaskFile", "Can't create Task File: " ) + FTsk.fileName() );
    return;
    }
  FTsk.write( Task );
  FTsk.close();
  QByteArrayList Pictures;
  for( int iEnd, iPict = Task.indexOf( "PICT(" ); iPict != -1; iPict = Task.indexOf( "PICT(", iEnd ) )
    {
    iEnd = Task.indexOf( ')', iPict += 5 );
    QByteArray PictName( Task.mid( iPict, iEnd - iPict ) );
    if( Pictures.indexOf( PictName ) != -1 ) continue;
    Pictures.push_back( PictName );
    PictName += ".jpg";
    m_PictFile.setFileName( TmpPath + PictName );
    if( !m_PictFile.open( QIODevice::WriteOnly ) )
      {
      QMessageBox::critical( nullptr, X_Str( "HMMsgDlg_dlgInfoE", "Error" ), X_Str( "CantCreateFilePicture", "Can't Create File Picture: " ) +
        m_PictFile.fileName() );
      break;
      }
    QEventLoop Loop;
    QNetworkRequest Request( QUrl( m_TasksUrl + PictName ) );
    m_pReply = sm_NetworkManager.get( Request );
    connect( m_pReply, SIGNAL( readyRead() ), SLOT( ReadyRead() ) );
    connect( m_pReply, SIGNAL( finished() ), &Loop, SLOT( quit() ) );
    Loop.exec();
    if( m_PictFile.size() == 0 )
      QMessageBox::critical( nullptr, X_Str( "HMMsgDlg_dlgInfoE", "Error" ), X_Str( "CantLoadPicture", "Can't Load Picture: " ) +
      m_pReply->url().toString() );
    m_PictFile.close();
    disconnect( m_pReply, SIGNAL( finished() ), &Loop, SLOT( quit() ) );
    disconnect( m_pReply, SIGNAL( readyRead() ), this, SLOT( ReadyRead() ) );
    }
  m_pWaitMessage->hide();
  sm_TaskPath = FTsk.fileName();
  HideHelps();
  NewXPRESSTask( sm_TaskPath );
  }

void  WinTesting::ReadyRead()
  {
  m_PictFile.write( m_pReply->readAll() );
  }

void WinTesting::ReturnResult()
  {
  QEventLoop Loop;
  QNetworkRequest Request( QUrl( m_RootAppUrl + "GetResult.asp?tpc=" + m_TopicId + "&usr=" + m_UserId + "&done=" +
    QByteArray::number( s_Task.m_SummMarks ) + "&TSum=" + QByteArray::number( s_Task.m_pStepsL->m_SummMarksTotal ) ) );
  m_pReply = sm_NetworkManager.get( Request );
  connect( m_pReply, SIGNAL( readyRead() ), SLOT( ResultRead() ) );
  connect( m_pReply, SIGNAL( finished() ), &Loop, SLOT( quit() ) );
  Loop.exec();
  }

void WinTesting::ResultRead()
  {
  QString Result = m_pReply->readAll();
  if( Result.isEmpty() )
    {
    QMessageBox::critical( nullptr, "Site Error", "There was no response from server" );
    QCoreApplication::quit();
    }
  QString AddMsg;
  if( Result == "Second" )
    AddMsg = X_Str( "MRepeatAnswer", "but the answer was repeated and therefore was not accepted" );
  else
    AddMsg = X_Str( "MTotalResult", "Total Result: " ) + Result;
  QString TotalSum = QString::number( max( s_Task.m_pStepsL->m_SummMarksTotal, 1 ) );
  QString sTaskResult = X_Str( "PlgSingExamResHeadMark", "Mark" ) + ": (" + TotalSum + ')' + QString::number(s_Task.m_SummMarks);
  QMessageBox::information( nullptr, X_Str( "ResLabel1Caption", "Results" ), sTaskResult + "\r\n" + AddMsg );
  QCoreApplication::quit();
  }

void WinTesting::SaveEnable( QObject* pO )
  {
  if( Translation() )
    {
    sm_TranslateObjects.Translate( pO );
    sm_pSaveTaskFile->setEnabled( sm_TranslateObjects.CanSave() );
    return;
    }
  if( s_TaskEditorOn ) sm_pSaveTaskFile->setEnabled( true );
  }

bool WinTesting::Translation() 
  { 
  return s_Task.GetLanguage() != XPInEdit::sm_Language; 
  }

void WinTesting::SaveExpression()
  {
  Panel::sm_pEditor->SaveFormula();
  }

void WinTesting::RestoreExpression()
  {
  Panel::sm_pEditor->RestoreFromFile();
  }

void WinTesting::EditTaskHeader()
  {
  sm_pEditHeader->setEnabled( false );
  m_pCreateTask->setEnabled( false );
  m_pEditTask->setEnabled( false );
  TaskHeaderEditor HE( this, &s_Task );
  HE.exec();
  sm_pEditHeader->setEnabled( true );
  m_pCreateTask->setEnabled( true );
  m_pEditTask->setEnabled( true );
  }

void WinTesting::EditTaskCalc()
  {
  sm_pEditCalc->setEnabled( false );
  m_pCreateTask->setEnabled( false );
  m_pEditTask->setEnabled( false );
  int LastFocused = -1;
  do
    {
    TaskCalcEditor CE( this, &s_Task );
    CE.show();
    CE.SetLastFocused(LastFocused);
    CE.exec();
    LastFocused = CE.LastFocused();
    } while( TaskCalcEditor::sm_Restart );
  sm_pEditCalc->setEnabled( true );
  m_pCreateTask->setEnabled( true );
  m_pEditTask->setEnabled( true );
  }

void WinTesting::ShowFunctions()
  {
  Panel::sm_pEditor->ResetView( true );
  }

void WinTesting::ShowExpression()
  {
  Panel::sm_pEditor->ResetView( false );
  }

void WinTesting::AddPictures()
  {
  Panel::sm_pEditor->Clear();
  QStringList Paths = QFileDialog::getOpenFileNames( nullptr, "Selection of pictures" );
  for( int i = 0; i < Paths.count(); i++ )
    {
    QByteArray P = Paths.at(i).toLocal8Bit();
    P = P.mid(P.lastIndexOf('/') + 1);
    if( i < Paths.count() - 1 ) P += ';';
    Panel::sm_pEditor->RestoreFormula(P);
    }
  XPGedit::sm_SelectPictures = EdList::sm_PartlySelected = !Paths.isEmpty();
  }

void WinTesting::AddHyperlink()
  {
  HyperLinkEditor HEd;
  HEd.exec();
  }

void WinTesting::EditTracks()
  {
  TraksEditor TE;
  TE.exec();
  if( !s_Task.m_pTrack->m_MultyTrack ) return;
  s_Task.m_pTrack->m_TrackSelected = true;
  s_Task.m_pTrack->m_SelectedTrack = 1;
  m_pSelectTrack->setEnabled( true );
  }

void WinTesting::SelectTrack()
  {
  if( sm_pSaveTaskFile->isEnabled() ) slotSaveTaskFile(false);
  s_Task.m_pTrack->m_TrackSelected = false;
  s_Task.SelectTrack();
  }

void WinTesting::BrowseTask()
  {
  static QString Chrome;
  if( Chrome.isEmpty() )
    {
    HKEY hkChrome;
    if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Google\\Update", 0, KEY_READ, &hkChrome ) == ERROR_SUCCESS )
      {
      unsigned long iLen = _MAX_PATH;
      unsigned long iType = REG_SZ;
      char cPath[_MAX_PATH];
      if( RegQueryValueExA( hkChrome, "LastInstallerSuccessLaunchCmdLine", 0, &iType, ( unsigned char* ) cPath, &iLen ) == ERROR_SUCCESS )
        Chrome = cPath;
      RegCloseKey( hkChrome );
      }
    if( Chrome.isEmpty() )
      {
      QMessageBox::critical( nullptr, "Error", "Google Chrome there was not installed");
      return;
      }
    }
  QString FileName = SelectTask();
  if( FileName.isEmpty() ) return;
  TypeSelector TS( this );
  if( TS.exec() == QDialog::Rejected ) return;
  QFile Fin( FileName );
  Fin.open( QIODevice::ReadOnly );
  QByteArray Task = Fin.readAll();
  QByteArray TaskName = FileName.toLocal8Bit();
  QByteArray Ext = TaskName.mid( TaskName.lastIndexOf( '.' ) );
  TaskName = "Local" + Ext;
  auto Send = [] ( const QByteArray& Arg )
    {
    Connector C( "https://halomda.org/WebTestManager/CreateLocalTest.php", Arg );
    QByteArray R = C.Connect();
    if( R.trimmed() != "OK" )
      {
      QMessageBox::critical( nullptr, "Error", "Can't UpLoad Task " + R );
      return false;
      }
    return true;
    };
  if( !Send( "File=" + Task.replace( '+', "%2B" ) + "&Name=" + TaskName ) ) return;
  QSet<QByteArray> Picts;
  for( int iEnd, iStart = Task.indexOf( "PICT(" ); iStart != -1; iStart = Task.indexOf( "PICT(", iEnd ) )
    {
    iEnd = Task.indexOf( ')', iStart += 5  );
    QByteArray PictName = Task.mid( iStart, iEnd - iStart ).trimmed();
    if( Picts.find( PictName ) != Picts.end() ) continue;
    Picts.insert( PictName );
    QFile FPic( FileName.left( FileName.lastIndexOf( '/' ) + 1 ) + ( PictName += ".jpg" ) );
    FPic.open( QIODevice::ReadOnly );
    QByteArray Body = FPic.readAll().toBase64();
    if( !Send( "File=" + Body.replace( '+', "%2B" ) + "&Name=" + PictName ) ) return;
    }
  QByteArray Mode( "wrkLearn" );
  if( TS.m_pExam->isChecked() ) Mode = "wrkExam";
  if( TS.m_pTrain->isChecked() ) Mode = "wrkTrain";
  QByteArray Topic = "4824";
  if( Ext == ".tsk" ) Topic = "4825";
  if( Ext == ".tru" ) Topic = "4826";
  if( Ext == ".tbg" ) Topic = "4827";
  static QProcess Browser;
  if( Browser.state() != QProcess::NotRunning )
    {
    Browser.terminate();
    Browser.waitForFinished();
    }
  Browser.start( Chrome + " https://halomda.org/WebTestManager/StartTest.php?tpc=" + Topic + "&usr=17187&HID=0&chpId=519&mode=" + Mode );
  }

void WinTesting::Calc()
  {
  QByteArray Formula( Panel::sm_pEditor->Write() );
  if( Formula.isEmpty() ) return;
  MathExpr Expr = MathExpr( Parser::StrToExpr( Formula ) );
  if( s_GlobalInvalid || Expr.IsEmpty() ) return;
  Expr = Expr.Reduce();
  if( Expr.IsEmpty() )
    {
    QMessageBox::critical( nullptr, "Error", "Can't Reduce Formula " + Formula );
    return;
    }
  WinTesting::sm_pOutWindow->AddExp( Expr );
  }

/*
void WinTesting::PlotGraph()
  {
  if(m_pPlotter != nullptr) delete m_pPlotter;
  m_pPlotter = new Plotter;
  m_pPlotter->setFixedSize(m_pPlotter->size());
  if( m_pPlotter->Plot(Panel::sm_pEditor->Write()) )  m_pPlotter->show();
  }
*/

void WinTesting::MultiPlot()
  {
  if(m_pMultiPlotter != nullptr) delete m_pMultiPlotter;
  try
    {
    m_pMultiPlotter = new MultiPlotter(Panel::sm_pEditor->Write());
    m_pMultiPlotter->setFixedSize(m_pMultiPlotter->size());
    if( m_pMultiPlotter->Plot() ) m_pMultiPlotter->show();
    }
  catch( QString Err )
    {
    QMessageBox::information( NULL, "Error", Err );
    }
  }

void WinTesting::CalcDeriv()
  {
  QByteArray Formula = Panel::sm_pEditor->Write();
  if(Formula.isEmpty()) return;
  Solver *pSolv = new TDerivative;
  ExpStore::sm_pExpStore->Init_var();
  TSolutionChain::sm_SolutionChain.Clear();
  pSolv->SetExpression(Formula);
  MathExpr EResult = pSolv->Result();
  delete pSolv;
  bool V;
  if( EResult.Boolean_( V ) && V)
    {
    EResult = TSolutionChain::sm_SolutionChain.GetLastExpr();
    Formula = Formula + ";" + EResult->SWrite();
    Panel::sm_pEditor->Clear();
    Panel::sm_pEditor->RestoreFormula(Formula);
    }
  }

TypeSelector::TypeSelector( QWidget *pParent ) : QDialog( pParent, Qt::WindowSystemMenuHint ), m_pLearn( new QRadioButton( "Learn", this ) ),
  m_pTrain( new QRadioButton( "Train", this ) ), m_pExam( new QRadioButton( "Exam", this ) )
  {
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Select Work Mode" );
  m_pTrain->setChecked( true );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget( m_pLearn );
  pVBox->addWidget( m_pTrain );
  pVBox->addWidget( m_pExam );
  QHBoxLayout *pHBox = new  QHBoxLayout;
  QPushButton *cmdOK = new QPushButton( "OK" );
  connect( cmdOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pHBox->addWidget( cmdOK );
  QPushButton *cmdCancel = new QPushButton( "Cancel" );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHBox->addWidget( cmdCancel );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  }

QValidator::State LogSpinBox::validate( QString &text, int &pos ) const
  {
  if( !m_ByMouse ) return QValidator::Invalid;
  return QDoubleSpinBox::validate( text, pos );
  }

void LogSpinBox::stepBy( int steps )
  {
  m_ByMouse = true;
  if( steps > 0 ) 
    setValue( value() * 10 );
  else
    setValue( value() / 10 );
  m_ByMouse = false;
  }

TaskHeaderEditor::TaskHeaderEditor( QWidget *pParent, BaseTask *pTask ) : QDialog( pParent, Qt::WindowSystemMenuHint ), m_TaskHeader( pTask->GetHeader() ),
  m_pShowUnarMinus( new QCheckBox ), m_pHideUnarMinus( new QCheckBox ), m_pShowMultSign( new QCheckBox ), m_pHideMultSign( new QCheckBox ),
  m_pShowRad(new QCheckBox), m_pShowDeg(new QCheckBox), m_pMultiTask(new QCheckBox), m_pAccuracy(new LogSpinBox), m_pName(new QLineEdit),
  m_pShowMinusByAddition(new QCheckBox), m_pHideMinusByAddition(new QCheckBox), m_pShowMinute( new QCheckBox )
  {
  if ( m_TaskHeader.m_pName == nullptr || m_TaskHeader.m_pName->isEmpty()) return;
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Edit header of task" );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  QHBoxLayout *pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Task Name" ) );
  m_pName->setText(EdStr::sm_pCodec->toUnicode(*m_TaskHeader.m_pName) );
  pHBox->addWidget( m_pName );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Accuracy, for default type 0.0" ) );
  m_pAccuracy->setRange( 0.0000001, 0.1 );
  m_pAccuracy->setSingleStep( 0.0000001 );
  m_pAccuracy->setDecimals( 7 );
  m_pAccuracy->setValue(m_TaskHeader.m_Accuracy);
  pHBox->addWidget( m_pAccuracy );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Show unar minus" ) );
  m_pShowUnarMinus->setChecked( m_TaskHeader.m_ShowUnarMinus );
  pHBox->addWidget( m_pShowUnarMinus );
  connect( m_pShowUnarMinus, SIGNAL( stateChanged( int ) ), SLOT( ChangeUnarm( int ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Hide unar minus" ) );
  m_pHideUnarMinus->setChecked( m_TaskHeader.m_HideUnarMinus );
  pHBox->addWidget( m_pHideUnarMinus );
  connect( m_pHideUnarMinus, SIGNAL( stateChanged( int ) ), SLOT( ChangeUnarm( int ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget(new QLabel("Show minus by addition"));
  m_pShowMinusByAddition->setChecked(m_TaskHeader.m_ShowMinusByAddition);
  pHBox->addWidget(m_pShowMinusByAddition);
  connect(m_pShowMinusByAddition, SIGNAL(stateChanged(int)), SLOT(ChangeMinusByAdd(int)));
  pVBox->addLayout(pHBox);
  pHBox = new  QHBoxLayout;
  pHBox->addWidget(new QLabel("Hide minus by addition"));
  m_pHideMinusByAddition->setChecked(m_TaskHeader.m_HideMinusByAddition);
  pHBox->addWidget(m_pHideMinusByAddition);
  connect(m_pHideMinusByAddition, SIGNAL(stateChanged(int)), SLOT(ChangeMinusByAdd(int)));
  pVBox->addLayout(pHBox);
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Show sign of multiplication" ) );
  m_pShowMultSign->setChecked( m_TaskHeader.m_ShowMultSign );
  pHBox->addWidget( m_pShowMultSign );
  connect( m_pShowMultSign, SIGNAL( stateChanged( int ) ), SLOT( ChangeMult( int ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Hide sign of multiplication" ) );
  m_pHideMultSign->setChecked( m_TaskHeader.m_HideMultSign );
  pHBox->addWidget( m_pHideMultSign );
  connect( m_pHideMultSign, SIGNAL( stateChanged( int ) ), SLOT( ChangeMult( int ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Angle in radians" ) );
  m_pShowRad->setChecked( m_TaskHeader.m_ShowRad );
  pHBox->addWidget( m_pShowRad );
  connect( m_pShowRad, SIGNAL( stateChanged( int ) ), SLOT( ChangeRad( int ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Angle in degrees" ) );
  m_pShowDeg->setChecked( m_TaskHeader.m_ShowDeg );
  pHBox->addWidget( m_pShowDeg );
  connect( m_pShowDeg, SIGNAL( stateChanged( int ) ), SLOT( ChangeRad( int ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget(new QLabel("Show minutes"));
  m_pShowMinute->setChecked(s_ShowMinute);
  pHBox->addWidget(m_pShowMinute);
  connect(m_pShowMinute, SIGNAL(stateChanged(int)), SLOT(ChangeShowMinute(int)));
  pVBox->addLayout(pHBox);
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "This task is multitask" ) );
  m_pMultiTask->setChecked( *m_TaskHeader.m_pMultiTask );
  pHBox->addWidget( m_pMultiTask );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  QPushButton *cmdOK = new QPushButton( "Next" );
  connect( cmdOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pHBox->addWidget( cmdOK );
  QPushButton *cmdCancel = new QPushButton( "Cancel" );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHBox->addWidget( cmdCancel );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  }

void TaskHeaderEditor::accept()
  {
  TMult::sm_ShowUnarMinus = m_TaskHeader.m_ShowUnarMinus = m_pShowUnarMinus->isChecked();
  m_TaskHeader.m_HideUnarMinus = m_pHideUnarMinus->isChecked();
  TSumm::sm_ShowMinusByAddition = m_TaskHeader.m_ShowMinusByAddition = m_pShowMinusByAddition->isChecked();
  m_TaskHeader.m_HideMinusByAddition = !m_TaskHeader.m_ShowMinusByAddition;
  TMult::sm_ShowMultSign = m_TaskHeader.m_ShowMultSign = m_pShowMultSign->isChecked();
  if (m_TaskHeader.m_ShowMultSign) XPGedit::sm_ShowMultSignInQWindow = true;
  m_TaskHeader.m_HideMultSign = m_pHideMultSign->isChecked();
  if (m_TaskHeader.m_HideMultSign) XPGedit::sm_ShowMultSignInQWindow = false;
  m_TaskHeader.m_ShowRad = m_pShowRad->isChecked();
  m_TaskHeader.m_ShowDeg = m_pShowDeg->isChecked();
  TExpr::sm_Accuracy = m_TaskHeader.m_Accuracy = m_pAccuracy->value();
  *m_TaskHeader.m_pName = EdStr::sm_pCodec->fromUnicode( m_pName->text() );
  *m_TaskHeader.m_pMultiTask = m_pMultiTask->isChecked();
  WinTesting::SaveEnable( this );
  QDialog::accept();
  }

void TaskHeaderEditor::ChangeUnarm(int State)
  {
  if( State == 0 ) return;
  QObject *pSender = sender();
  if( pSender == m_pShowUnarMinus )
    m_pHideUnarMinus->setChecked( false );
  else
    m_pShowUnarMinus->setChecked( false );
  }

void TaskHeaderEditor::ChangeMinusByAdd(int State)
  {
  if (State == 0) return;
  QObject *pSender = sender();
  if (pSender == m_pShowMinusByAddition)
    m_pHideMinusByAddition->setChecked(false);
  else
    m_pShowMinusByAddition->setChecked(false);
  }

void TaskHeaderEditor::ChangeMult( int State )
  {
  if( State == 0 ) return;
  QObject *pSender = sender();
  if( pSender == m_pShowMultSign )
    m_pHideMultSign->setChecked( false );
  else
    m_pShowMultSign->setChecked( false );
  }

void TaskHeaderEditor::ChangeRad( int State )
  {
  if( State == 0 ) return;
  QObject *pSender = sender();
  if( pSender == m_pShowDeg )
    m_pShowRad->setChecked( false );
  else
    m_pShowDeg->setChecked( false );
  }

void TaskHeaderEditor::ChangeShowMinute(int State)
  {
  s_ShowMinute = State;
  }

CaseEditor::CaseEditor( QWidget *pParent, const QByteArray& VarName, const QByteArray& Text ) : 
  QDialog( pParent, Qt::WindowSystemMenuHint ), m_pEditor( new QTextEdit )
  {
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Edit case instruction for variable " + VarName );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget( m_pEditor );
  QHBoxLayout *pHBox = new  QHBoxLayout;
  QPushButton *pSave = new QPushButton( "Save" );
  pHBox->addWidget( pSave );
  connect( pSave, SIGNAL( clicked() ), SLOT( accept() ) );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  pHBox->addWidget( pCancel );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  resize( 500, 500 );
  if( Text.isEmpty() ) return;

  int iPos = 0;
  int iCaseCount = 1;
  auto GetArg = [&] ()
    {
    int iEnd = iPos, Bracket = 0;
    for( ; iEnd < Text.count() && ( Bracket != 0 || Text[iEnd] != ',' ); iEnd++ )
      {
      if( Text[iEnd] == '(' )
        {
        if( Text.mid( iPos, 6 ) == "@case(" )
          {
          iPos = iEnd + 1;
          iCaseCount++;
          return QByteArray( "@case(" );
          }
        Bracket++;
        continue;
        }
      if( Text[iEnd] == ')' )
        {
        Bracket--;
        if( Bracket < 0 )
          if( iEnd == Text.count() - iCaseCount )
            break;
          else
            throw QString( "exists superfluous closing bracket" );
        }
      }
    if( iEnd == Text.count() ) throw QString( "not enough closing brackets" );
    QByteArray Result( Text.mid( iPos, iEnd - iPos ) );
    iPos = iEnd + 1;
    return Result.trimmed();
    };

  try
    {
    QByteArray FirstArg, Result;
    bool CanEnclosure;
    int iBraceCount = 0;
    int iSp = 0;
#define SP QByteArray(iSp,' ') + 
    for( int iArg = 0; iArg < 5; iArg++ )
      {
      QByteArray Arg( GetArg() );
      if( iPos >= Text.count() - 1 && iArg != 4 ) throw QString( "too small number arguments of function case" );
      switch( iArg )
        {
        case 0:
          if( FirstArg.isEmpty() )
            Result = "case=" + Arg + ";\n";
          else
            if( !CanEnclosure )
              {
              Result += SP "{\n" + SP "case=" + Arg + ";\n";
              iSp += 2;
              iBraceCount++;
              }
          FirstArg = Arg;
          continue;
        case 1:
          Result += SP "if( case == " + Arg + ")\n";
          iSp += 2;
          continue;
        case 2:
          Result += SP "return " + Arg + ";\n";
          iSp -= 2;
          continue;
        case 3:
          Result += SP "else\n";
          iSp += 2;
          CanEnclosure = FirstArg == Arg;
          if( !CanEnclosure ) Result += SP "if( case == " + Arg + ")\n";
          continue;
        case 4:
          if( Arg == "@case(" )
            {
            iArg = -1;
            continue;
            }
          iSp += 2;
          Result += SP "return " + Arg + ";\n";
        }
      }
    for( int i = 0; i < iBraceCount; i++, iSp -= 2)
      Result += SP "}\n";
    m_pEditor->setText( Result );
#undef SP
    }
  catch( const QString &Msg )
    {
    QMessageBox::critical( nullptr, "Case error", Msg );
    return;
    }
  }
 
void CaseEditor::accept()
  {
  QStringList Rows( m_pEditor->toPlainText().split( '\n' ) );
  if( Rows.count() == 1 )
    {
    QDialog::accept();
    return;
    }
  try
    {
    while( Rows.last().isEmpty() ) Rows.takeLast();
    if( Rows.count() < 5 ) throw QString( "Too Too few lines" );
    auto TestExpression = [] ( const QByteArray& ExprText )
      {
      MathExpr Expr = MathExpr( Parser::StrToExpr( ExprText ) );
      if( s_GlobalInvalid || Expr.IsEmpty() ) throw QString( "Expression error in " ) + ExprText;
      };

    m_EditionResult.clear();
    int iCaseCount = 0;
    bool CanEnclosure = false;
    QByteArray FirstArg, IfArg;
    bool Brace = false;
    for( int iArg = 0, iRow = 0, iBraceCount = 0, iPos, iEnd; iArg < 5 && iRow < Rows.count() - iBraceCount; iArg++, iRow++ )
      {
      QByteArrayList RowList( Rows[iRow].toLocal8Bit().split( ' ' ) );
      QByteArray Row;
      for( int i = 0; i < RowList.count(); Row += RowList[i++] );
      switch( iArg )
        {
        case 0:
          if( Row.left( 4 ) == "case" )
            {
            iPos = Row.indexOf( '=' ) + 1;
            iEnd = Row.indexOf( ';' );
            if( iPos == 0 || iEnd == -1 ) throw QString( "Error in line " ) + Row;
            FirstArg = Row.mid( iPos, iEnd - iPos );
            TestExpression( FirstArg );
            }
          else
            if( !CanEnclosure ) throw QString( "Error in line " ) + Row;
          m_EditionResult += "@case(" + FirstArg + ',';
          if( CanEnclosure && !Brace ) iArg++;
          CanEnclosure = false;
          Brace = false;
          iCaseCount++;
          continue;
        case 1:
          iPos = Row.indexOf( "==" ) + 2;
          iEnd = Row.lastIndexOf( ')' );
          if( Row.left( 3 ) != "if(" || iPos == 1 || iEnd != Row.count() - 1 ) throw QString( "Error in line " ) + Row;
          Row = Row.mid( iPos, iEnd - iPos );
          TestExpression( Row );
          IfArg = Row;
          continue;
        case 2:
          if( Row == "{" )
            {
            if( !( CanEnclosure && iRow < Rows.count() - 2 ) ) throw QString( "Error in line " ) + Row;
            Brace = true;
            iBraceCount++;
            FirstArg = IfArg;
            iArg = -1;
            m_EditionResult += IfArg + ',';
            continue;
            }
          if( CanEnclosure && iRow < Rows.count() - 1 )
            {
            iRow -= 2;
            m_EditionResult += FirstArg + ',';
            iArg = -1;
            continue;
            }
          iPos = Row.indexOf( "return" ) + 6;
          iEnd = Row.indexOf( ';' );
          if( iPos == 5 || iEnd == -1 ) throw QString( "Error in line " ) + Row;
          Row = Row.mid( iPos, iEnd - iPos );
          TestExpression( Row );
          m_EditionResult += IfArg + ',' + Row;
          continue;
        case 3:
          if( Row != "else" ) throw QString( "Error in line " ) + Row;
          m_EditionResult += ',';
          continue;
        case 4:
          CanEnclosure = ( iPos = Row.indexOf( "return" ) + 6 ) == 5;
          if( CanEnclosure )
            {
            iArg = 0;
            iRow--;
            continue;
            }
          iEnd = Row.indexOf( ';' );
          if( iEnd == -1 ) throw QString( "Error in line " ) + Row;
          Row = Row.mid( iPos, iEnd - iPos );
          TestExpression( Row );
          m_EditionResult += FirstArg + ',' + Row;
        }
      }
    m_EditionResult += QByteArray( iCaseCount, ')' );
    TestExpression( m_EditionResult );
    QDialog::accept();
    }
  catch( const QString& Msg )
    {
    QMessageBox::critical( nullptr, "Text error", Msg );
    return;
    }
  }

void CellEditor::showEvent( QShowEvent *event )
  {
  home( false );
  }

void CellEditor::dropEvent( QDropEvent *event )
  {
  qDebug() << "Drop RE Start:" << event->mimeData()->html();
  if( event->source() == this ) return;
  event->acceptProposedAction();
  QString Text = event->mimeData()->html();
  int Img = Text.indexOf( "<img" );
  if( Img == -1 ) return;
  Img = Text.indexOf( "src", Img );
  Img = Text.indexOf( '"', Img ) + 1;
  QString Path = Text.mid( Img, Text.indexOf( '"', Img ) - Img );
  QImage Image( Path );
  if( Image.isNull() ) return;
  QString Formula = ToUnicode(Parser::UnpackUnAscii(Image.text("F1").toLatin1()));
  if( Formula.indexOf("Picture(") > -1)
    {
    int iStart = Formula.indexOf('"');
    Formula = Formula.mid(iStart, Formula.lastIndexOf('"') - iStart + 1);
    }
  setText( Formula );
  WinTesting::SaveEnable( this );
  qDebug() << "Drop RE Stop:";
  }

void CellEditor::mouseDoubleClickEvent( QMouseEvent *event )
  {
  Panel::sm_pEditor->Clear( true );
  WinTesting::sm_pMainWindow->ShowExpression();
  s_iDogOption = 1;
  QByteArray BaseChar(FromUnicode( text() ));
  if(BaseChar.indexOf("@case") != - 1) s_iDogOption = 2;
  MathExpr Expr = MathExpr( Parser::StrToExpr( XPGedit::ExpandFuncName(BaseChar) ) );
  s_iDogOption = 0;
  if( s_GlobalInvalid || Expr.IsEmpty() ) return;
  QByteArray Formula;
  if(Expr->Variab(Formula))
    Formula = "@" + Formula;
  else
    Formula = Expr.SWrite();
  Panel::sm_pEditor->RestoreFormula( Formula );
  Panel::sm_pEditor->activateWindow();
  }

QString CellEditor::ToUnicode(const QByteArray& B)
  {
  QString Formula;
  for (int i = 0; i < B.count(); i++)
    Formula += ::ToUnicode(B[i]);
  return Formula;
  }

void CellEditor::dragEnterEvent( QDragEnterEvent *event )
  {
  qDebug() << "Drag RE Start:" << event->mimeData()->html();
  if( event->mimeData()->hasHtml() ) event->acceptProposedAction();
  qDebug() << "Drag RE Stop:";
  }

RowEditor::RowEditor( int RowNumber, TaskCalcEditor *pReceiver ) :
  CellEditor(""), m_RowNumber( RowNumber ), m_pReceiver( pReceiver )
  {
  setAcceptDrops( true );
  }

void RowEditor::mousePressEvent( QMouseEvent *event )
  {
  if( event->button() == Qt::RightButton && ( text().isEmpty() || text().left( 6 ) == "@case(" ) )
    {
    QMenu *pMenu = createStandardContextMenu();
    pMenu->addAction( "Edit case instruction", this, SLOT( EditCase() ) );
    pMenu->exec( event->globalPos() );
    delete pMenu;
    return;
    }
  if(hasSelectedText())
    {
    CalcRow &Row = m_pReceiver->m_Rows[m_RowNumber];
    if(Row.m_pVariable == this && Row.m_pExpression->text().contains(".jpg"))
      {
      Row.Recalc();
      QByteArray Formula("Picture(@" + text().trimmed().toLocal8Bit() + ')');
      MathExpr Expr = MathExpr( Parser::StrToExpr( Formula) );
      if( s_GlobalInvalid || Expr.IsEmpty() ) return;
      XPInEdit InEd( Expr.SWrite(), *BaseTask::sm_pEditSets, XPGedit::sm_ViewSettings );
      QImage *pImage = InEd.GetImage();
      pImage->setText( "F1", Parser::PackUnAscii( Formula ) );
      QString Path( RichTextDocument::GetTempPath());
      pImage->save( Path );
      QString Html = "<img src=\"" + Path + "\" style=\"vertical-align:middle;\" />";
      QDrag *pDrag = new QDrag( this );
      pDrag->setPixmap( QPixmap( ":/Resources/Drag.png" ) );
      QMimeData *pMimeData = new QMimeData;
      pMimeData->setHtml( Html );
      pDrag->setMimeData( pMimeData );
      pDrag->exec();
      return;
      }
    }
  QLineEdit::mousePressEvent( event );
  }

void RowEditor::EditCase()
  {
  CaseEditor CE( this, m_pReceiver->m_Rows[m_RowNumber].m_pVariable->text().toLocal8Bit(), FromUnicode(text()).mid( 6 ) );
  CE.show();
  if( CE.exec() == QDialog::Accepted ) setText( CE.m_EditionResult );
  }

void RowEditor::paste()
  {
  QString Formula = ToUnicode(Parser::UnpackUnAscii(QApplication::clipboard()->text().toLatin1()));
  setText(Formula);
  }

void RowEditor::mouseDoubleClickEvent( QMouseEvent *event )
  {
  m_pReceiver->SaveText(m_RowNumber);
  if( TaskCalcEditor::sm_Calc[m_RowNumber].m_Kind != tXDexpress || text().isEmpty() ) return;
  CellEditor::mouseDoubleClickEvent(event);
  }

QTextCodec* s_pCodec = QTextCodec::codecForName("Windows-1253");

void RowEditor::focusInEvent( QFocusEvent * pE )
  {
  m_pReceiver->m_LastFocused = m_RowNumber;
  QLineEdit::focusInEvent( pE );
  if(m_pReceiver->m_pLastFocused == this) return;
  m_pReceiver->m_pLastFocused = this;
  m_pReceiver->m_pTestTableCaseButton->start(100);
  }

void RowEditor::changeEvent( QEvent *ev )
  {
  TaskCalcEditor::sm_Calc[m_RowNumber].m_Kind = text().indexOf('[' ) == -1 ? tXDexpress : tXDrandom;
  QLineEdit::changeEvent( ev );
  }

CalcRow::CalcRow( QGridLayout *pVBox, TaskCalcEditor* pEditor, int iRow ) : m_pEditor(pEditor),
  m_pVariable( new RowEditor( iRow, pEditor ) ), m_pExpression( new RowEditor( iRow, pEditor ) ),
  m_pParmFunctions( new QComboBox ),
  m_pResult( new QLabel ), m_Pair( pEditor->sm_Calc[iRow++] )
  {
  pVBox->addWidget( m_pVariable, iRow, 0 );
  pVBox->addWidget( m_pExpression, iRow, 1 );
  pVBox->addWidget( m_pResult, iRow, 2 );
  m_pVariable->setText(RowEditor::ToUnicode(m_Pair.m_Variable ));
  m_pExpression->setText(RowEditor::ToUnicode(m_Pair.m_Expression ));
  }

void CalcRow::Recalc()
  {
  try
    {
    SaveText();
    m_Pair.Calculate();
    MathExpr VarValue;
    ExpStore::sm_pExpStore->Known_var_expr( m_Pair.m_Variable, VarValue );
    if(!VarValue.IsEmpty() ) m_pResult->setText( VarValue.WriteE() );
    }
  catch( ErrParser& ErrMsg )
    {
    QMessageBox::critical( nullptr, "Error", ErrMsg.Message() );
    }
  }

void CalcRow::SaveText()
  {
  m_Pair.m_Expression = FromUnicode(m_pExpression->text());
  m_Pair.m_Variable = FromUnicode(m_pVariable->text()).trimmed();
  if( m_Pair.m_Expression.indexOf( '[' ) == -1 )
    m_Pair.m_Kind = tXDexpress;
  else
    m_Pair.m_Kind = tXDrandom;
  }

CaseRow::CaseRow( QGridLayout *pGrid, const QByteArray& Value, const QByteArray& Expression, int iRow ) :
   m_pValue( new QLabel(Value) ), m_pExpression( new CellEditor( Expression ) )
  {
  pGrid->addWidget( m_pValue, iRow, 0 );
  pGrid->addWidget( m_pExpression, iRow, 1 );
  }

TableCaseEditor::TableCaseEditor( QWidget *pParent, const QStringList& VarNames, RowEditor* pCase ) :
  QDialog( pParent, Qt::WindowSystemMenuHint ), m_pCase(pCase), m_pGrid(new QGridLayout), m_pProbVarName(new QComboBox)
  {
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Edit Case" );
  m_pProbVarName->addItems(VarNames);
  connect( m_pProbVarName, SIGNAL( currentIndexChanged(int) ), SLOT( SetVariable() ) );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget(m_pProbVarName);
  m_pGrid->setColumnStretch( 1, 2 );
  m_pGrid->addWidget( new QLabel( "Value" ), 0, 0 );
  m_pGrid->addWidget( new QLabel( "Expression" ), 0, 1 );
  QWidget *pScrollWidget = new QWidget;
  pScrollWidget->setLayout( m_pGrid );
  QScrollArea *pPanelArea = new QScrollArea;
  pPanelArea->setWidgetResizable( true );
  pPanelArea->setWidget( pScrollWidget );
  pPanelArea->setAlignment( Qt::AlignCenter );
  pVBox->addWidget( pPanelArea );
  QHBoxLayout *pRowBox = new QHBoxLayout;
  QPushButton *pSave = new QPushButton( "Save Case" );
  connect( pSave, SIGNAL( clicked() ), SLOT( accept() ) );
  pRowBox->addWidget( pSave );
  QPushButton *cmdCancel = new QPushButton( "Cancel" );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pRowBox->addWidget( cmdCancel );
  pVBox->addLayout( pRowBox );
  setLayout( pVBox );
  QString Case(m_pCase->text());
  if(Case.isEmpty())
    {
    SetVariable();
    return;
    }
  Case = Case.mid(Case.indexOf("(@") + 2);
  QString VarName(Case.left(Case.indexOf(',')));
  int Index = m_pProbVarName->findText(VarName + '=', Qt::MatchStartsWith );
  if(Index == 0)
    SetVariable();
  else
    m_pProbVarName->setCurrentIndex(Index);
  }

void TableCaseEditor::SetVariable()
  {
  for(int Item = m_pGrid->count() - 1; Item > 1; Item--)
    {
    QLayoutItem *pItem = m_pGrid->takeAt(Item);
    delete pItem->widget();
    delete pItem;
    }
  QString Row(m_pProbVarName->currentText());
  QString Variable = Row.left(Row.indexOf('='));
  Row = Row.mid(Row.indexOf('[') + 1);
  QStringList Values;
  int iPoints = Row.indexOf("..");
  if(iPoints == -1)
    Values = Row.left(Row.lastIndexOf(']')).split(';');
  else
    {
    double dStart = Row.left(iPoints).toDouble();
    int iClose = Row.indexOf(']');
    double dEnd = Row.mid(iPoints + 2, iClose - iPoints - 2 ).toDouble();
    QString sStep = Row.mid(iClose + 1).trimmed();
    double dStep = sStep.toDouble();
    int iPecision = 0;
    int iPoint = sStep.indexOf('.');
    if( iPoint != -1 ) iPecision = sStep.length() - iPoint - 1;
    for(double dValue = dStart; dValue < dEnd + 0.1 * dStep; dValue += dStep)
      Values.append(QString::number(dValue, 'f', iPecision ));
    }
  QStringList Expressions = m_pCase->text().split('@' + Variable + ',');
  for(int iVar = 0, iExpr = 0; iVar < Values.count(); iVar++, iExpr++ )
    {
    m_pGrid->addWidget(new QLabel(Values[iVar]), iVar + 1, 0);
    QString sExpr;
    if(Expressions.count() > 1)
      {
      sExpr = Expressions[iExpr];
      if(sExpr.left(5) == "@case") sExpr =  Expressions[++iExpr];
      int iComma = sExpr.indexOf(',');
      if( iComma == -1 )
        {
        int BrCount = sExpr.count(")") - sExpr.count("(");
        while( BrCount-- )
          sExpr = sExpr.left(sExpr.length() - 1);
        }
      else
        sExpr = sExpr.mid(iComma + 1, sExpr.length() - iComma - 2);
      }
    m_pGrid->addWidget(new CellEditor(sExpr), iVar + 1, 1);
    }
  }

void TableCaseEditor::accept()
  {
  QString Variable(m_pProbVarName->currentText());
  Variable = Variable.left(Variable.indexOf('='));
  QString sCase;
  for(int iValue = 1; iValue < RowCount() ; iValue++)
    if(iValue < RowCount() - 1)
      sCase += "@case(@" + Variable + ',' + ( (QLabel*) m_pGrid->itemAtPosition(iValue, 0)->widget())->text() +
         ',' + ( (CellEditor*) m_pGrid->itemAtPosition(iValue, 1)->widget())->text() + ",@" + Variable + ',';
    else
      sCase += ( (CellEditor*) m_pGrid->itemAtPosition(iValue, 1)->widget())->text();
  m_pCase->setText(sCase + QString( RowCount() - 2, ')'));
  QDialog::accept();
  }

TaskCalcEditor::TaskCalcEditor( QWidget *pParent, BaseTask *pTask ) : QDialog( pParent, Qt::WindowSystemMenuHint ), m_pCalc( pTask->m_pCalc ),
  m_pGrid(new QGridLayout), m_pTestTableCaseButton(new QTimer(this)), m_LastFocused(-1), m_pLastFocused(nullptr)
  {
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Edit section CALC" );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  m_pGrid->setColumnStretch( 1, 2 );
  m_pGrid->addWidget( new QLabel( "Variable" ), 0, 0 );
  m_pGrid->addWidget( new QLabel( "Expression" ), 0, 1 );
  m_pGrid->addWidget( new QLabel( "Result" ), 0, 2 );
  if( !sm_Restart ) sm_Calc = *m_pCalc;
  sm_Restart = false;
  if(sm_Calc.count() == 0)
    {
    sm_Calc.append( CalcPair((char) msPi + QByteArray("=3.1415926535"), &sm_Calc, tXDexpress ) );
    sm_Calc.append( CalcPair( "e=2.7182818285", &sm_Calc, tXDexpress ) );
    }
  for( int iRow = 0; iRow < sm_Calc.count(); iRow++ )
    m_Rows.push_back( CalcRow( m_pGrid, this, m_Rows.count() ) );
  QWidget *pScrollWidget = new QWidget;
  pScrollWidget->setLayout( m_pGrid );
  QScrollArea *pPanelArea = new QScrollArea;
  pPanelArea->setWidgetResizable( true );
  pPanelArea->setWidget( pScrollWidget );
  pPanelArea->setAlignment( Qt::AlignCenter );
  pVBox->addWidget( pPanelArea );
  QHBoxLayout *pRowBox = new QHBoxLayout;
  QPushButton *pDelRow = new QPushButton( "Delete row" );
  connect( pDelRow, SIGNAL( clicked() ), SLOT( DelRow() ) );
  pRowBox->addWidget( pDelRow );
  QPushButton *pAddRow = new QPushButton( "Add row" );
  connect( pAddRow, SIGNAL( clicked() ), SLOT( AddRow() ) );
  pRowBox->addWidget( pAddRow );
  QPushButton *pInsertRow = new QPushButton( "Insert row" );
  connect( pInsertRow, SIGNAL( clicked() ), SLOT( InsertRow() ) );
  pRowBox->addWidget( pInsertRow );
  QPushButton *pRecalc = new QPushButton( "Recalc" );
  connect( pRecalc, SIGNAL( clicked() ), SLOT( Recalc() ) );
  pRowBox->addWidget( pRecalc);
  pVBox->addLayout( pRowBox );
  QHBoxLayout *pHBox = new  QHBoxLayout;
  m_pTableCaseEdButton = new QPushButton( "Edit case by table" );
  connect( m_pTableCaseEdButton, SIGNAL( clicked() ), SLOT( TableCaseEdit() ) );
  m_pTableCaseEdButton->setEnabled(false);
  pHBox->addWidget( m_pTableCaseEdButton );
  connect(m_pTestTableCaseButton, SIGNAL(timeout()), SLOT( TestTableCaseButton() ));
  QPushButton *cmdOK = new QPushButton( "Next" );
  connect( cmdOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pHBox->addWidget( cmdOK );
  QPushButton *cmdCancel = new QPushButton( "Cancel" );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHBox->addWidget( cmdCancel );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  resize( 1000, min(700, (sm_Calc.count() + 5) * 35 ) );
  }

void TaskCalcEditor::SaveText( int iRow )
  {
  if (iRow == -1)
    {
    for (int iRow = 0; iRow < m_Rows.count(); iRow++)
      {
      m_Rows[iRow].SaveText();
      sm_Calc[iRow] = m_Rows[iRow].m_Pair;
      }
    return;
    }
  m_Rows[iRow].SaveText();
  sm_Calc[iRow] = m_Rows[iRow].m_Pair;
  }

void TaskCalcEditor::accept()
  {
  SaveText();
  *m_pCalc = sm_Calc;
  WinTesting::SaveEnable( this );
  QDialog::accept();
  }

void TaskCalcEditor::Recalc()
  {
  ExpStore::sm_pExpStore->Clear();
  for( int iRow = 0; iRow < m_Rows.count(); iRow++ )
    m_Rows[iRow].Recalc();
  }

void TaskCalcEditor::TableCaseEdit()
  {
  QStringList VarRandNames;
  for(int iRow = 0; iRow < m_Rows.count(); iRow++)
    {
    QString sExpression(m_Rows[iRow].m_pExpression->text());
    if(sExpression.front() == '[')
      VarRandNames.append(m_Rows[iRow].m_pVariable->text() + '=' + sExpression);
    }
  if(VarRandNames.count() == 0) return;
  m_pLastFocused = m_Rows[m_LastFocused].m_pExpression;
  m_pLastFocused->selectAll();
  TableCaseEditor Editor(parentWidget(), VarRandNames, m_Rows[m_LastFocused].m_pExpression );
  Editor.show();
  Editor.exec();
  m_pLastFocused->deselect();
  m_pLastFocused->setFocus();
  m_pLastFocused = nullptr;
  }

void TaskCalcEditor::AddRow()
  {
  SaveText();
  sm_Calc.append( CalcPair( &sm_Calc ) );
  m_LastFocused = sm_Calc.count() - 1;
  sm_Restart = true;
  reject();
  }

void TaskCalcEditor::InsertRow()
  {
  if( m_LastFocused == -1 ) AddRow();
  if( !sm_Calc.isEmpty() && QMessageBox::question( nullptr, "Add row to CALC", "You add row before " +
    sm_Calc[m_LastFocused].m_Variable + " to section CALC?" ) != QMessageBox::Yes ) return;
  SaveText();
  sm_Calc.insert( m_LastFocused, CalcPair( &sm_Calc ) );
  sm_Restart = true;
  reject();
  }

void TaskCalcEditor::DelRow()
  {
  if( sm_Calc.isEmpty() ) return;
  if( QMessageBox::question( nullptr, "Delete row from CALC", "You delete row " +
    sm_Calc[m_LastFocused ].m_Variable + " from section CALC?" ) != QMessageBox::Yes ) return;
  SaveText();
  sm_Calc.removeAt( m_LastFocused );
  if( m_LastFocused >= sm_Calc.count() ) m_LastFocused = sm_Calc.count() - 1;
  sm_Restart = true;
  reject();
  }

void TaskCalcEditor::TestTableCaseButton()
  {
  m_pTestTableCaseButton->stop();
  QString sExpression(m_Rows[m_LastFocused].m_pExpression->text());
  m_pTableCaseEdButton->setEnabled(sExpression.isEmpty() || sExpression[0] != '[');
  if(m_pLastFocused == nullptr)
    {
    m_Rows[m_LastFocused].m_pVariable->setFocus();
    return;
    }
  m_pLastFocused->setFocus();
  m_pLastFocused = nullptr;
  }

void TaskCalcEditor::SetLastFocused(int LastFocused)
  {
  m_LastFocused = LastFocused;
  if(LastFocused != -1) TestTableCaseButton();
  }


HyperLinkEditor::HyperLinkEditor() : QDialog( nullptr, Qt::WindowSystemMenuHint ), m_pFile( new QLineEdit ),
  m_pText( new QLineEdit ), m_pBaseUrl(new QLineEdit)
  {
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Create Hyperlink" );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  QHBoxLayout *pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Name of File" ) );
  pHBox->addWidget(m_pFile);
  connect(m_pFile, SIGNAL( textChanged( const QString& ) ), SLOT( TextChanged( const QString& ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Hyperlink Label" ) );
  pHBox->addWidget( m_pText );
  connect( m_pText, SIGNAL( textChanged( const QString& ) ), SLOT( TextChanged( const QString& ) ) );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget(new QLabel("Base URL"));
  pHBox->addWidget(m_pBaseUrl);
  m_pBaseUrl->setText(sm_BaseUrl);
  connect(m_pBaseUrl, SIGNAL(textChanged(const QString&)), SLOT(TextChanged(const QString&)));
  pVBox->addLayout(pHBox);
  pHBox = new  QHBoxLayout;
  m_pPutHyperlink = new QPushButton( "Put to Clipboard" );
  pHBox->addWidget( m_pPutHyperlink );
  connect( m_pPutHyperlink, SIGNAL( clicked() ), SLOT( accept() ) );
  m_pPutHyperlink->setEnabled( false );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHBox->addWidget( pCancel );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  }

void HyperLinkEditor::accept()
  {
  QString BaseUrl(m_pBaseUrl->text());
  if (!BaseUrl.endsWith('/')) BaseUrl += '/';
  QString Html = "<a href=\"" + BaseUrl + m_pFile->text() + "\">" + m_pText->text() + "</a>";
  QMimeData *pMimeData = new QMimeData;
  pMimeData->setHtml( Html );
  QClipboard *pClipboard = QApplication::clipboard();
  pClipboard->setMimeData( pMimeData );
  QDialog::accept();
  }

void HyperLinkEditor::TextChanged( const QString& )
  {
  if( m_pText->text().isEmpty() || m_pFile->text().isEmpty() ) return;
  QString Url = m_pBaseUrl->text();
  m_pPutHyperlink->setEnabled( Url.left( 8 ) == "https://" || Url.left( 7 ) == "http://" );
  }

TraksEditor::TraksEditor() : QDialog( nullptr, Qt::WindowSystemMenuHint ), m_pListNames( new QListWidget ),
m_pTrack(s_Task.m_pTrack), m_pRichWindow(new RichTextWindow(this)), m_pRemoveTrack(new QPushButton("Remove track"))
  {
  setStyleSheet( "QWidget {font-size:12pt;}" );
  setWindowTitle( "Edit Tracks" );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget( new QLabel( "Track Names" ) );
  for( auto pName = m_pTrack->m_NameOfTrack.begin(); pName != m_pTrack->m_NameOfTrack.end(); pName++ )
    m_pListNames->addItem( ToLang( *pName ) );
  connect( m_pListNames, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ), SLOT( EditTrack( QListWidgetItem * ) ) );
  pVBox->addWidget( m_pListNames );
  QHBoxLayout *pHBox = new  QHBoxLayout;
  QPushButton *pButton = new QPushButton( "Add Track" );
  pHBox->addWidget( pButton );
  connect( pButton, SIGNAL( clicked() ), SLOT( AddTrack() ) );
  pHBox->addWidget( m_pRemoveTrack );
  connect( m_pRemoveTrack, SIGNAL( clicked() ), SLOT( RemoveTrack() ) );
  m_pRemoveTrack->setEnabled( m_pListNames->count() > 0 );
  pVBox->addLayout( pHBox );
  pVBox->addWidget( new QLabel( "Description of Tracks" ) );
  m_pRichWindow->SetContent( m_pTrack->m_TracksDescription );
  m_pRichWindow->ResetSize();
  pVBox->addWidget( m_pRichWindow );
  pHBox = new  QHBoxLayout;
  pButton = new QPushButton( "Next" );
  pHBox->addWidget( pButton );
  connect( pButton, SIGNAL( clicked() ), SLOT( accept() ) );
  pButton = new QPushButton( "Cancel" );
  pHBox->addWidget( pButton );
  connect( pButton, SIGNAL( clicked() ), SLOT( reject() ) );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  }

void TraksEditor::accept()
  {
  QDialog::reject();
  return;
  m_pTrack->m_NameOfTrack.clear();
  m_pTrack->m_MultyTrack = m_pListNames->count() > 0;
  for( int iTrack = 0; iTrack < m_pListNames->count(); iTrack++ )
    m_pTrack->m_NameOfTrack.append(EdStr::sm_pCodec->fromUnicode( m_pListNames->item(iTrack)->text() ) );
  WinTesting::sm_pMainWindow->slotSaveTaskFile( false );
  s_Task.LoadFromFile();
  QDialog::accept();
  }

void TraksEditor::reject()
  {
  dynamic_cast< RichTextDocument* >( m_pRichWindow->document() )->SetContent( m_pTrack->m_TracksDescription );
  QDialog::reject();
  }

void TraksEditor::AddTrack()
  {
  m_pListNames->addItem( "New Track" );
  m_pRemoveTrack->setEnabled( true );
  }

void TraksEditor::RemoveTrack()
  {
  QListWidgetItem *pItem = m_pListNames->takeItem( m_pListNames->count() - 1 );
  delete pItem;
  m_pRemoveTrack->setEnabled( m_pListNames->count() > 0 );
  }

void TraksEditor::EditTrack( QListWidgetItem *pItem )
  {
  pItem->setFlags( pItem->flags() | Qt::ItemIsEditable );
  m_pListNames->editItem( pItem );
  }

Algebra1::Algebra1()
  {
  AddButton( ":/Resources/Calculator/RMultBtnAl.jpg", "MMultAlHint", "Factoring Expression", &Algebra1::FactoringExpression );
  AddButton( ":/Resources/Calculator/SubtSqrBtnAl.jpg", "MSubtSqrAlHint", "Difference of Squares", &Algebra1::SquaresDifference );
  AddButton( ":/Resources/Calculator/ExpandBtnAl.jpg", "MExpandAlHint", "Opening Brackets", &Algebra1::OpeningBrackets );
  AddButton( ":/Resources/Calculator/SqrSubSumBtnAl.jpg", "MSqrSubSumAlHint", "Square of a Sum or a Difference", &Algebra1::SumSquare );
  AddButton( ":/Resources/Calculator/SumCubBtnAl.jpg", "MSumCubAlHint", "Sum of Cubes", &Algebra1::SumCubes );
  AddButton( ":/Resources/Calculator/TrinomBtnAl.jpg", "MTrinomAlHint", "Factoring Trinomial", &Algebra1::Trinomial );
  AddButton( ":/Resources/Calculator/CubSubSumBtnAl.jpg", "MCubSumAlHint", "Cube of Sum or Difference", &Algebra1::SumCube );
  AddButton( ":/Resources/Calculator/SubCubBtnAl.jpg", "MSubCubAlHint", "Difference of Cubes", &Algebra1::DifferenceCubes );
  AddButton( ":/Resources/Calculator/CalculateBtnAl.jpg", "MCalculateAlHint", "Calculator", &Algebra1::Calculator );
  }

void Algebra1::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Algebra1::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Algebra1::FactoringExpression()
  {
  Solve( new TSolvReToMult );
  }

void Algebra1::SquaresDifference()
  {
  Solve( new TSolvSubSqr );
  }
   
void Algebra1::OpeningBrackets()
  {
  Solve( new TSolvExpand );
  }

void Algebra1::SumSquare()
  {
  Solve( new TSolvSqrSubSum );
  }

void Algebra1::SumCubes()
  {
  Solve( new TSolvSumCub );
  }

void Algebra1::Trinomial()
  {
  Solve( new TTrinom );
  }

void Algebra1::SumCube()
  {
  Solve( new TSolvCubSubSum );
  }

void Algebra1::DifferenceCubes()
  {
  Solve( new TSolvSubCub );
  }

void Algebra1::Calculator()
  {
  Solve( new Alg1Calculator );
  }

Algebra2::Algebra2()
  {
  AddButton( ":/Resources/Calculator/btnAlg2Lg10.jpg", "MBitBtnLgCalcHint", "Calculate LG", &Algebra2::CalculateLg );
  AddButton( ":/Resources/Calculator/BtnLog1Eq.jpg", "MLog1EqAl2Hint", "Log Linear Eq", &Algebra2::LogEquations );
  AddButton( ":/Resources/Calculator/BtnSysInEq.jpg", "MSysInEqAl2Hint", "Sys In Eq", &Algebra2::Inequalities );
  AddButton( ":/Resources/Calculator/btnAlg2InEqXYGrph.jpg", "MBitBtnInEqXYGrphCalcHint", "Rat In Eq", &Algebra2::SystemInequalities );
  AddButton( ":/Resources/Calculator/BtnRatInEq.jpg", "MRatInEqAl2Hint", "Graphical Presentation", &Algebra2::GraphPresent );
  AddButton( ":/Resources/Calculator/BtnExpntEq.jpg", "MBitBtnExpntEqHint", "Exponential Equation", &Algebra2::ExpEquations );
  AddButton( ":/Resources/Calculator/SubstButton.jpg", "MSubstButton", "Method of substitution", &Algebra2::MethodSubstitution );
  AddButton( ":/Resources/Calculator/LinearSysButton.jpg", "MLinearSysButton", "System of linear equations", &Algebra2::LinearSystem );
  AddButton( ":/Resources/Calculator/IntroVarButton.jpg", "MIntroVarButton", "Introducing new variables", &Algebra2::IntroductingNewVars );
  }

void Algebra2::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Algebra2::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Algebra2::CalculateLg()
  {
  Solve( new TLg );
  }

void Algebra2::LogEquations()
  {
  Solve( new Log1Eq );
  }

void Algebra2::Inequalities()
  {
  Solve(new SysInEq );
  }

void Algebra2::SystemInequalities()
  {
  Solve( new SysInEqXY );
  }

void Algebra2::GraphPresent()
  {
  Solve( new RatInEq );
  }

void Algebra2::ExpEquations()
  {
  Solve( new ExpEq );
  }

void Algebra2::MethodSubstitution()
  {
  Solve( new MakeSubstitution );
  }

void Algebra2::LinearSystem()
  {
  Solve( new SolveLinear );
  }

void Algebra2::IntroductingNewVars()
  {
  Solve( new MakeExchange );
  }

Equations1::Equations1()
  {
  AddButton( ":/Resources/Calculator/DetLinEquBtnQu.jpg", "MDetLinEquQuHint", "Solving Linear Equation", &Equations1::LinearEq );
  AddButton( ":/Resources/Calculator/QuaEquBtnQu.jpg", "MQuaEquQuHint", "Roots of Square Equation", &Equations1::RootsQuadEq  );
  AddButton( ":/Resources/Calculator/DetQuaEquBtnQu.jpg", "MDetQuaEquQuHint", "Using Square Equation Formula", &Equations1::FormulaQuadEq );
  AddButton( ":/Resources/Calculator/DisQuaEquBtnQu.jpg", "MDisQuaEquQuHint", "Calculating Discriminant", &Equations1::CalcDiscrim );
  AddButton( ":/Resources/Calculator/DetVieEquBtnQu.jpg", "MDetVieEquQuHint", "Using VietteTheorem", &Equations1::VietteTheorem );
  AddButton( ":/Resources/Calculator/TrinomBtnQu.jpg", "MTrinomQuHint", "BiQuadratic Equation", &Equations1::BiquadraticEq );
  AddButton( ":/Resources/Calculator/btnAlgFrEquat.jpg", "MAlgFrEqHint", "Equations in Algebraic Fractions", &Equations1::AlgFracionsEq );
  AddButton( ":/Resources/Calculator/BtnSqLogEq.jpg", "MBitBtnSqLogEqHint", "Equations with Radicals", &Equations1::RadicalEq );
  AddButton( ":/Resources/Calculator/CalculateBtnQu.jpg", "MCalculateAlHint", "Calculator", &Equations1::CalcQuadEq );
  }

void Equations1::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Equations1::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Equations1::LinearEq()
  {
  Solve( new TSolvDetLinEqu );
  }

void Equations1::RootsQuadEq()
  {
  Solve( new TSolvQuaEqu );
  }

void Equations1::FormulaQuadEq()
  {
  Solve( new TSolvDetQuaEqu );
  }

void Equations1::CalcDiscrim()
  {
  Solve( new TSolvDisQuaEqu );
  }

void Equations1::VietteTheorem()
  {
  Solve( new TSolvDetVieEqu );
  }

void Equations1::BiquadraticEq()
  {
  Solve( new TSolvCalcDetBiQuEqu );
  }

void Equations1::AlgFracionsEq()
  {
  Solve( new TSolvFractRatEq );
  }

void Equations1::RadicalEq()
  {
  Solve( new TSolvCalcIrratEq );
  }

void Equations1::CalcQuadEq()
  {
  Solve( new TSolvCalcPolinomEqu );
  }

Equations2::Equations2()
  {
  AddButton( ":/Resources/Calculator/BitBtnSinCosEq.jpg", "MBitBtnSinEqHint", "Basic equation with Sine", &Equations2::BasicSinEq );
  AddButton( ":/Resources/Calculator/BitBtnTanCtnEq.jpg", "MBitBtnTanEqHint", "Basic equation with Tangent", &Equations2::BasicTanEq );
  AddButton( ":/Resources/Calculator/BitBtnSysSpecTrigoEq.jpg", "MBitBtnSysSpecTrigoEqHint", "Equation with Sin and Cos", &Equations2::SinCosEq );
  AddButton( ":/Resources/Calculator/BitBtnCosEq.jpg", "MBitBtnCosEqHint", "Basic equation with Cosine", &Equations2::BasiCosEq );
  AddButton( ":/Resources/Calculator/BitBtnCtnEq.jpg", "MBitBtnCtnEqHint", "Basic equation with Cotangent", &Equations2::BasicCotEq );
  AddButton( ":/Resources/Calculator/BitBtnSysTrigoBiQudrEq.jpg", "MBitBtnSysTrigoBiQudrEqHint", "Square Trigo-Equation", &Equations2::SquareTrigoEq );
  AddButton( ":/Resources/Calculator/BitBtnSysAlgFrEq.jpg", "MBitBtnBitBtnSysAlgFrEqHint", "Equations in Algebraic Fractions", &Equations2::AlgFracionsEq );
  AddButton( ":/Resources/Calculator/BitBtnHmgnTrigoEq.jpg", "MBitBtnHmgnTrigoEqHint", "Homogeneous equation", &Equations2::HomogenEq );
  AddButton( ":/Resources/Calculator/BitBtnSCalc.jpg", "MBitBtnSCalcHint", "Calculator", &Equations2::Calculator );
  }

void Equations2::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Equations2::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Equations2::BasicSinEq()
  {
  Solve( new TSolvCalcSimpleTrigoEq );
  }

void Equations2::BasicTanEq()
  {
  Solve( new TSolvCalcSimpleTrigoEq );
  }

void Equations2::SinCosEq()
  {
  Solve( new TSolvCalcSinCosEqu );
  }

void Equations2::BasiCosEq()
  {
  Solve( new TSolvCalcSimpleTrigoEq );
  }

void Equations2::BasicCotEq()
  {
  Solve( new TSolvCalcSimpleTrigoEq );
  }

void Equations2::SquareTrigoEq()
  {
  Solve( new TSolvCalcTrigoEqu );
  }

void Equations2::AlgFracionsEq()
  {
  Solve( new TSolvFractRatEq );
  }

void Equations2::HomogenEq()
  {
  Solve( new TSolvCalcHomogenTrigoEqu );
  }

void Equations2::Calculator()
  {
  Solve( new TSolvCalcEquation );
  }


SciCalc::SciCalc()
  {
  AddButton( ":/Resources/Calculator/btnSinCalc.jpg", "MBitBtnSinCalcHint", "Calculate Sine", &SciCalc::Sin );
  AddButton( ":/Resources/Calculator/btnCosCalc.jpg", "MBitBtnCosCalcHint", "Calculate Cosine", &SciCalc::Cos );
  AddButton( ":/Resources/Calculator/btnCalcTan.jpg", "MBitBtnTanCalcHint", "Calculate Tangent", &SciCalc::Tan );
  AddButton( ":/Resources/Calculator/btnLgCalc.jpg", "MBitBtnLgCalcHint", "Calculate Lg", &SciCalc::Lg10 );
  AddButton( ":/Resources/Calculator/btnLnCalc.jpg", "MBitBtnLnCalcHint", "Calculate Ln", &SciCalc::Ln );
  AddButton( ":/Resources/Calculator/btnDegRad.jpg", "MBitBtnDegRadCalcHint", "From Degrees to Radians", &SciCalc::ToRad );
  AddButton( ":/Resources/Calculator/btnRadDeg.jpg", "MBitBtnRadDegCalcHint", "From Radians to Degrees", &SciCalc::ToDeg );
  AddButton( ":/Resources/Calculator/btnPiGreek.jpg", "MBitBtnPiGreekCalcHint", "number Pi", &SciCalc::Pi );
  AddButton( ":/Resources/Calculator/BtnCalcCalc.jpg", "MBitBtnCalcCalcHint", "Calculator", &SciCalc::Calculator );
  }

void SciCalc::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( SciCalc::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void SciCalc::Sin()
  {
  Solve( new TSin );
  }

void SciCalc::Cos()
  {
  Solve( new TCos );
  }

void SciCalc::Tan()
  {
  Solve( new TTan );
  }

void SciCalc::Lg10()
  {
  Solve( new TLg );
  }

void SciCalc::Ln()
  {
  Solve( new TLn );
  }

void SciCalc::ToRad()
  {
  Solve( new TDegRad );
  }

void SciCalc::ToDeg()
  {
  Solve( new TRadDeg );
  }

void SciCalc::Pi()
  {
  U_A_T Uact;
  Uact = msPi;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
  }

void SciCalc::Calculator()
  {
  Solve( new TSciCalc );
  }

Calculus::Calculus()
  {
  AddButton( ":/Resources/Calculator/Deriv.jpg", "MdiffButton", "Derivative of function written with template", &Calculus::Derivative );
  AddButton( ":/Resources/Calculator/IndefInt.jpg", "MintegralButton", "Indefinite Integral", &Calculus::IndefIntegr );
  AddButton( ":/Resources/Calculator/Limit.jpg", "MlimitButton", "Limit", &Calculus::Limit );
  AddButton( ":/Resources/Calculator/DerivFun.jpg", "MoldDiffButton", "Derivative of function from variable x", &Calculus::DerivFunc );
  AddButton( ":/Resources/Calculator/DefInt.jpg", "MdefIntegralButton", "Definite Integral", &Calculus::DefIntegr );
  AddButton( ":/Resources/Calculator/EigenVals.jpg", "MComplxNmbr", "Eigen values of Matrix", &Calculus::EigenValues );
  AddButton( ":/Resources/Calculator/Determinant.jpg", "MBitBtnDtrmnntMHint", "Determinant of Matrix", &Calculus::Determinant );
  AddButton( ":/Resources/Calculator/Transpose.jpg", "MBitBtnTranspMHint", "Transposition of Matrix", &Calculus::Transposition );
  AddButton( ":/Resources/Calculator/MatrixInv.jpg", "ManalCalcButton", "Matrix Inversion", &Calculus::MatrixInv );
  }

void Calculus::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Calculus::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Calculus::Derivative()
  {
  Solve( new TDerivative );
  }

void Calculus::IndefIntegr()
  {
  Solve( new TIndefIntegr );
  }

void Calculus::Limit()
  {
  Solve( new TLimitCreator );
  }

void Calculus::DerivFunc()
  {
  Solve( new TMatrixOp );
  }

void Calculus::DefIntegr()
  {
  Solve( new TDefIntegrate );
  }

void Calculus::EigenValues()
  {
  Solve( new TEigen );
  }

void Calculus::Determinant()
  {
  Solve( new TDeterminant );
  }

void Calculus::Transposition()
  {
  Solve( new TTransposer );
  }

void Calculus::MatrixInv()
  {
  Solve( new TInverter );
  }

Trigo::Trigo()
  {
  AddButton( ":/Resources/Calculator/DoubleAngle.jpg", "MAngle2Hint", "Double angle formulas", &Trigo::Angle2 );
  AddButton( ":/Resources/Calculator/TripleAngle.jpg", "MAngle3Hint", "Triple angle formulas", &Trigo::Angle3 );
  AddButton( ":/Resources/Calculator/Tan2.jpg", "MTan2Hint", "Basic functions in terms of tangent of half angle", &Trigo::Tan2 );
  AddButton( ":/Resources/Calculator/Alpha2.jpg", "MAlpha2Hint", "Half angle formulas", &Trigo::Alpha2 );
  AddButton( ":/Resources/Calculator/Priv.jpg", "MPrivHint", "Reduction formulas", &Trigo::Priv );
  AddButton( ":/Resources/Calculator/TrigoOfSumma.jpg", "MTrigOfSummaHint", "Addition formulas", &Trigo::TrigOfSumma );
  AddButton( ":/Resources/Calculator/Summ_Mult.jpg", "MSumm_MultHint", "Sum to product formulas", &Trigo::Summ_Mult );
  AddButton( ":/Resources/Calculator/Trigo_Summ.jpg", "MTrigo_SummHint", "Product to sum formulas", &Trigo::Trigo_Summ );
  AddButton( ":/Resources/Calculator/CalcTrigo.jpg", "MCalcTrigoHint", "Trigo-calculator", &Trigo::CalcTrigo );
  }

void Trigo::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Trigo::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Trigo::Angle2()
  {
  Solve( new TAngle2 );
  }

void Trigo::Angle3()
  {
  Solve( new TAngle3 );
  }

void Trigo::Tan2()
  {
  Solve( new TTan2 );
  }

void Trigo::Alpha2()
  {
  Solve( new TAlpha2 );
  }

void Trigo::Priv()
  {
  Solve( new TPriv );
  }

void Trigo::TrigOfSumma()
  {
  Solve( new TTrigoOfSumma );
  }

void Trigo::Summ_Mult()
  {
  Solve( new TSumm_Mult );
  }

void Trigo::Trigo_Summ()
  {
  Solve( new TTrigo_Summ );
  }

void Trigo::CalcTrigo()
  {
  Solve( new TSciCalc );
  }

Advanced::Advanced()
  {
  AddButton( ":/Resources/Calculator/Permut.jpg", "MPerCount", "Permutation of n of n", &Advanced::Permut );
  AddButton( ":/Resources/Calculator/BinomCoeff.jpg", "MBinomCoef", "Binomial Coefficient", &Advanced::BinomCoef);
  AddButton( ":/Resources/Calculator/Accomod.jpg", "MPlcmCoef", "Permutation of k of n", &Advanced::Accomod );
  AddButton( ":/Resources/Calculator/Statistics.jpg", "MMStatist", "Statistics", &Advanced::Statistic );
  AddButton( ":/Resources/Calculator/Correl.jpg", "MCorreletion", "Correlation", &Advanced::Correl );
  AddButton( ":/Resources/Calculator/LineProg.jpg", "MLinProg", "Linear Programming", &Advanced::LineProgr );
  AddButton( ":/Resources/Calculator/AlgToTrigo.jpg", "MAlgToTrigo", "From Algebraic To Trigonometric Form", &Advanced::AlgToTrigon );
  AddButton( ":/Resources/Calculator/ComplexOp.jpg", "MCalcIm", "Operations between Complex numbers", &Advanced::ComplexOp );
  AddButton( ":/Resources/Calculator/AdvGrPrez.jpg", "MRatInEqAl2Hint", "Graphical Presentation", &Advanced::Present );
  }

void Advanced::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( Advanced::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void Advanced::Permut()
  {
  Solve( new Permutations );
  }

void Advanced::BinomCoef()
  {
  Solve( new BinomCoeff );
  }

void Advanced::Accomod()
  {
  Solve( new Accomodations );
  }

void Advanced::Statistic()
  {
  Solve( new Statistics );
  }

void Advanced::Correl()
  {
  Solve( new Correlation );
  }

void Advanced::LineProgr()
  {
  Solve( new SysInEqXY );
  }

void Advanced::AlgToTrigon()
  {
  Solve( new AlgToTrigo );
  }

void Advanced::ComplexOp()
  {
  Solve( new ComplexOper );
  }

void Advanced::Present()
  {
  Solve( new RatInEq );
  }

AlgFrct::AlgFrct()
  {
  AddButton( ":/Resources/Calculator/BitBtnAlgFrSum.jpg", "MAlgFrSumHint", "Detailed Sum", &AlgFrct::Sum );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrDif.jpg", "MAlgFrDiffHint", "Detailed Difference", &AlgFrct::Dif );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrCancel.jpg", "MAlgFrCancelHint", "Cancelling Fraction", &AlgFrct::Cancel );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrMltpl.jpg", "MAlgFrMltplHint", "Detailed Multiplication", &AlgFrct::Mltpl );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrDiv.jpg", "MAlgFrDivHint", "Detailed Division", &AlgFrct::Div );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrCmnDenom.jpg", "MAlgFrCmnDenomHint", "Common Denominator", &AlgFrct::CmnDenom );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrPolynom.jpg", "MAlgFrEqPolynomHint", "Factoring Polynomial", &AlgFrct::Polynom );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrDivPolynoms.jpg", "MAlgFrDivPolynomsHint", "Dividing Polynomial by Polynomial", &AlgFrct::DivPolynoms );
  AddButton( ":/Resources/Calculator/BitBtnAlgFrCalculator.jpg", "MAlgFrCalculatorHint", "Calculator", &AlgFrct::Calculator );
  }

void AlgFrct::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint, void ( AlgFrct::*Slot )( ) )
  {
  CalcButton *pButton = CalcWidget::AddButton( Icon, Hint, DefaultHint );
  QObject::connect( pButton, &CalcButton::clicked, this, Slot );
  }

void AlgFrct::Mltpl()
  {

  }

void AlgFrct::Div()
  {

  }

void AlgFrct::Calculator()
  {

  }

void AlgFrct::DivPolynoms()
  {

  }

void AlgFrct::Cancel()
  {

  }

void AlgFrct::CmnDenom()
  {

  }

void AlgFrct::Sum()
  {
  Solve( new AlgFrctSum );
  }

void AlgFrct::Dif()
  {

  }

void AlgFrct::Polynom()
  {

  }


CalcButton::CalcButton( const QByteArray& Hint, const QByteArray& DefaultHint, const QString& Icon, bool bToTitle ) : m_Hint( Hint ), m_DefaultHint( DefaultHint )
  {
  LangSwitch();
  if( Icon.isEmpty() ) return;
  QPixmap Pixmap( Icon );
  QPixmap NewPix;
  if( bToTitle )
    NewPix = Pixmap.scaledToHeight( sm_ButtonHeight - 4 );
  else
    NewPix = Pixmap.scaledToWidth(sm_ButtonHeight * 1.6 );
  setIcon( NewPix );
  setIconSize( NewPix.size() );
  setFlat( bToTitle );
  }

void CalcButton::LangSwitch()
  {
  setToolTip( X_Str( m_Hint, m_DefaultHint ) );
  }

CalcWidget::CalcWidget() : m_pButtons(new QGridLayout )
  {
  m_pButtons->setMargin( 0 );
  setStyleSheet( "QPushButton { margin:0 }" );
  setLayout( m_pButtons );
  }

CalcButton *CalcWidget::AddButton( const QString& Icon, const QByteArray& Hint, const QByteArray& DefaultHint )
  {
  CalcButton *pButton = new CalcButton( Hint, DefaultHint, Icon);
  int Count = m_pButtons->count();
  int Row = Count / 3;
  int Col = Count % 3;
  m_pButtons->addWidget( pButton, Row, Col );
  return pButton;
  }

void CalcWidget::LangSwitch()
  {
  for( int Item = 0; Item < m_pButtons->count(); Item++ )
    dynamic_cast< CalcButton* >( m_pButtons->itemAt( Item )->widget() )->LangSwitch();
  }

void CalcWidget::Solve( Solver *pSolver )
  {
  auto Final = [&] ()
    {
    delete pSolver;
    Panel::sm_pEditor->setFocus();
    };
  WinTesting::sm_pBottomWindow->RestoreTrigonomSystem();
  s_NoRootReduce = false;
  s_IntegralCount = 0;
  QByteArray Formula( Panel::sm_pEditor->Write() );
  if( Formula.isEmpty() ) return Final();
  bool TestMode = Solver::sm_TestMode && sm_Result.device()->isOpen();
  if( TestMode && pSolver->Code() == ESolvTestMode )
    {
    TestMode = false;
    QMessageBox::information( nullptr, "No Test Mode", "For this operation Test Mode not exists" );
    }
  if( TestMode )
    {
    sm_Result << pSolver->Name() << " " << Formula << " ";
    if( sm_Test.device()->isOpen() )
      {
      sm_Test << pSolver->Code() << " " << Formula << "\r\n";
      sm_Test.flush();
      }
    }
  try
    {
    ExpStore::sm_pExpStore->Init_var();
    Solver::m_OldExpr.Clear();
    TSolutionChain::sm_SolutionChain.Clear();
    TSolutionChain::sm_SolutionChain.m_Accumulate = true;
    s_LastError.clear();
    pSolver->SetExpression( Formula );
    }
  catch( ErrParser& Err )
    {
    MathExpr Result = pSolver->Result();
    MathExpr StepsResult = TSolutionChain::sm_SolutionChain.GetChain();
    if( !StepsResult.IsEmpty() )
      WinTesting::sm_pOutWindow->AddExp( StepsResult );
    if( Result.IsEmpty() )
      WinTesting::sm_pOutWindow->AddExp( Panel::sm_pEditor->m_pInEdit->SWrite() );
    else
      WinTesting::sm_pOutWindow->AddExp( Result );
    WinTesting::sm_pOutWindow->AddComm( Err.WMessage() );
    if( TestMode )
      {
      sm_Result << Err.Message() << "\r\n";
      sm_Result.flush();
      }
    return Final();
    }
  MathExpr Result = pSolver->Result();
  QString Comment = TSolutionChain::sm_SolutionChain.GetLastComment();
//  if( Result.IsEmpty() || !s_LastError.isEmpty())
  if( Result.IsEmpty() )
    {
    WinTesting::sm_pOutWindow->AddComm( s_LastError );
    return Final();
    }
  bool V;
  if( Result.Boolean_( V ) )
    {
    MathExpr Result = TSolutionChain::sm_SolutionChain.GetChain();
    if( Result.IsEmpty() )
      WinTesting::sm_pOutWindow->AddComm( X_Str( "MNotSuitableExpr", "Not suitable expression!" ) );
    else
      {
      Lexp L(Result);
      for( PExMemb pMemb = L.First(); !pMemb.isNull(); pMemb = pMemb->m_pNext )
        {
        MathExpr Memb = pMemb->m_Memb;
        if(IsType(TStr, Memb))
          {
          QString S = QString::fromUtf8(Memb.WriteE());
          S = S.mid(1, S.count() - 2);
          WinTesting::sm_pOutWindow->AddComm(S);
          }
        else
          WinTesting::sm_pOutWindow->AddExp( Memb );
        }
      }
    return Final();
    }
  QString Comm;
  WinTesting::sm_pOutWindow->AddExp( Result );
  if( pSolver->Success() )
    Comm = s_XPStatus.GetCurrentMessage();
  else
    Comm = s_LastError;
  WinTesting::sm_pOutWindow->AddComm( Comm );
  if( TestMode )
    {
    sm_Result << pSolver->Result().WriteE() << " " << Comm << " " << s_XPStatus.GetCurrentMessage() << "\r\n";
    sm_Result.flush();
    }
  return Final();
  }

void CalcWidget::AutoTest()
  {
  if( Solver::sm_TestMode && sm_Result.device()->isOpen() )
    {
    if( QMessageBox::question( nullptr, "Enables Test Mode", "Test Mode was enabled, disable?" ) != QMessageBox::Yes ) return;
    CloseFiles();
    }
  QString TestFile = QFileDialog::getOpenFileName( nullptr, "Select File for Tests", "", "Test file (*.tst)" );
  if( TestFile.isEmpty() ) return;
  QString ResultFile = QFileDialog::getSaveFileName( nullptr, "Select File for Test Report", "", "Test report file (*.trp)" );
  if( ResultFile.isEmpty() ) return;
  QFile *pFile = dynamic_cast< QFile* >( sm_Result.device() );
  pFile->setFileName( ResultFile );
  pFile->open( QIODevice::WriteOnly );
  pFile = dynamic_cast< QFile* >( sm_Test.device() );
  pFile->setFileName( TestFile );
  pFile->open( QIODevice::ReadOnly );
  Solver::sm_TestMode = true;
  while( !sm_Test.atEnd() )
    {
    QStringList Line( sm_Test.readLine().split( " " ) );
    TSolvExpand *pS = new TSolvExpand;
    QByteArray Formula( Line[1].toLocal8Bit() );
    sm_Result << pS->Name() << " " << Formula << " ";
    try
      {
      ExpStore::sm_pExpStore->Init_var();
      pS->SetExpression( Formula );
      }
    catch( ErrParser& Err )
      {
      sm_Result << Err.Message() << "\r\n";
      sm_Result.flush();
      delete pS;
      continue;
      }
    QString Comm;
    if( pS->Success() )
      Comm = s_XPStatus.GetCurrentMessage();
    else
      Comm = s_LastError;
    sm_Result << pS->Result().WriteE() << " " << Comm << " " << s_XPStatus.GetCurrentMessage() << "\r\n";
    sm_Result.flush();
    delete pS;
    }
  CloseFiles();
  Solver::sm_TestMode = false;
  }

void CalcWidget::ResetTestMode()
  {
  if( Solver::sm_TestMode = !Solver::sm_TestMode )
    {
    QString ResultFile = QFileDialog::getSaveFileName( nullptr, "Select File for Test Report", "", "Test report file (*.trp)" );
    if( ResultFile.isEmpty() ) return;
    QFile *pFile = dynamic_cast< QFile* >( sm_Result.device() );
    pFile->setFileName( ResultFile );
    pFile->open( QIODevice::WriteOnly );
    QString TestFile = QFileDialog::getSaveFileName( nullptr, "Select File for Tests", "", "Test file (*.tst)" );
    if( TestFile.isEmpty() ) return;
    pFile = dynamic_cast< QFile* >( sm_Test.device() );
    pFile->setFileName( TestFile );
    pFile->open( QIODevice::WriteOnly );
    return;
    }
  CloseFiles();
  QMessageBox::information( nullptr, "Cancel Test Mode", "Test Mode was cancelled" );
  }

void CalcWidget::CloseFiles()
  {
  QFile *pFile = dynamic_cast< QFile* >( sm_Result.device() );
  if( pFile->isOpen() ) pFile->close();
  pFile = dynamic_cast< QFile* >( sm_Test.device() );
  if( pFile->isOpen() ) pFile->close();
  }

QPushButton* CalcWidget::GetButton( int row, int col )
  {
  return dynamic_cast< QPushButton* >( m_pButtons->itemAtPosition( row, col )->widget() );
  }

void ListCalculators::AddItem( const QByteArray& Name, const QByteArray& DefaultName )
  {
  QListWidgetItem *pItem = new QListWidgetItem( DefaultName );
  QVariantList Parms;
  Parms.append( Name );
  Parms.append( DefaultName );
  pItem->setData( Qt::UserRole, Parms );
  addItem( pItem );
  }

void ListCalculators::LangSwitch()
  {
  for( int Row = 0; Row < count(); Row++ )
    {
    QListWidgetItem *pItem = item( Row );
    QVariantList Parms = pItem->data( Qt::UserRole ).toList();
    pItem->setText( X_Str( Parms[0].toByteArray(), Parms[1].toByteArray() ) );
    }
  }

