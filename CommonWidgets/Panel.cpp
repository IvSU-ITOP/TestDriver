#include "Panel.h"
#include "../FormulaPainter/InEdit.h"

extern double PixelHeight;
#ifdef Q_OS_ANDROID
double PanelButton::sm_ButtonHeight = 25.0;
#else
double PanelButton::sm_ButtonHeight = 10.0;
#endif

XPGedit* Panel::sm_pEditor = nullptr;
Panel* Panel::sm_pPanel = nullptr;
GraphEdit* Panel::sm_pGraphEdit = nullptr;
SymbolsPanel* Panel::sm_pGreek;
SymbolsPanel* Panel::sm_pBigGreek;
SymbolsPanel* Panel::sm_pMathSymbol;

bool Panel::sm_GeoActivate = false;
void (*MainTab::sm_Help)() = nullptr;
void( *MainTab::sm_Hint )() = nullptr;
void( *MainTab::sm_Enter )( ) = nullptr;
void( *MainTab::sm_NewData )( ) = nullptr;

void TileXPressForms() {}

Panel::Panel( QWidget *parent ) : QWidget( parent ), m_pTitle( new QLabel ), m_pMainTab( new MainTab( this ) )
  {
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed);
  QLabel *pLogo = new QLabel;
  pLogo->setPixmap( QPixmap( ":/Resources/logo-halomda.png" ) );
  pLogo->setAlignment( Qt::AlignCenter );
  QVBoxLayout *pLayout = new QVBoxLayout;
  pLayout->addWidget( pLogo );
  m_pTitle->setAlignment( Qt::AlignCenter );
  m_pTitle->setText( X_Str( "XPControlPanelCaption", "Editor Keys" ) );
  m_pTitle->setStyleSheet( "QLabel {font-size:16px}" );
  pLayout->addWidget( m_pTitle );
  m_pTabWidget = new QTabWidget(this);
  m_pTabWidget->setTabPosition( QTabWidget::West );
  m_pTabWidget->setTabShape( QTabWidget::Triangular );
  m_pTabWidget->setUsesScrollButtons( false );
  m_pTabWidget->addTab( m_pMainTab, tr( "Main" ) );
  ScreenSize = QApplication::primaryScreen()->availableSize();
  PanelButton::sm_ButtonHeight = Round( 36 * ScreenSize.height() / 1080.0 );
//  setStyleSheet( "QPushButton { width:" + QString::number( Round( 72 * ScreenSize.width() / 1920.0 ) ) + "px;height:" +
//    QString::number( PanelButton::sm_ButtonHeight ) + "px;margin:0px;border-radius:10px;border-style:solid;border-width:2px; \
//   border-color:#ff93a6;background-color:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 pink,stop:1 black) ;min-width:80px;} \
//   QPushButton:pressed {background-color:qradialgradient(cx:0.0,cy:0.0,radius:1,fx:0.5,fy:0.5,stop:0 white,stop:0.2 rgba(255,30,30,144), \
//   stop:0.4 black);}" );
  PixelHeight = 25.4 / QApplication::primaryScreen()->logicalDotsPerInchX();
  m_pTabWidget->addTab( new FunctionsTab( this ), tr( "Functions" ) );
  m_pTabWidget->addTab(new MathTab(this), tr("Math"));
  m_pTabWidget->addTab(new AlgebraTab(this), tr("Algebra"));
  pLayout->addWidget( m_pTabWidget );
  setLayout( pLayout );
  sm_pPanel = this;
  sm_pGreek = new SymbolsPanel( X_Str( "TabSheetSmallGreekCaption", "Small greek" ), msAlpha, msOmega );
  sm_pBigGreek = new SymbolsPanel( X_Str( "TabSheetBigGreekCaption", "Big greek"), msBigAlpha, msBigOmega );
  sm_pMathSymbol = new SymbolsPanel( X_Str( "TabSheetMathematicCaption", "Mathematical") );
  }

void Panel::EmitExit()
  {
  emit ExitBtn();
  }

void Panel::LangSwitch()
  {
  dynamic_cast< MainTab* >( m_pTabWidget->widget( tMainTab ) )->LangSwitch();
  dynamic_cast< FunctionsTab* >( m_pTabWidget->widget( tFunctionsTab ) )->LangSwitch();
  dynamic_cast< MathTab* >( m_pTabWidget->widget( tMathTab ) )->LangSwitch();
  dynamic_cast< AlgebraTab* >( m_pTabWidget->widget( tAlgebraTab ) )->LangSwitch();
  m_pTabWidget->setTabText( tMainTab, X_Str( "TabSheetFracCaption", "Main" ) );
  m_pTabWidget->setTabText( tMathTab, X_Str( "TabSheetAnalysCaption", "Math" ) );
  m_pTabWidget->setTabText( tFunctionsTab, X_Str( "TabSheetGraphCaption", "Algebra II" ) );
  m_pTabWidget->setTabText( tAlgebraTab, X_Str( "TabSheetAlgCaption", "Algebra I" ) );
  sm_pGreek->setWindowTitle( X_Str( "TabSheetSmallGreekCaption", "Small greek" ) );
  sm_pBigGreek->setWindowTitle( X_Str( "TabSheetBigGreekCaption", "Big greek" ) );
  sm_pMathSymbol->setWindowTitle( X_Str( "TabSheetMathematicCaption", "Mathematical" ) );
  m_pTitle->setText( X_Str( "XPControlPanelCaption", "Editor Keys" ) );
  }

void Panel::HideSymbols()
  {
  sm_pGreek->hide();
  sm_pBigGreek->hide();
  sm_pMathSymbol->hide();
  }

void Panel::SymbolClicked()
  {
  uchar Code = dynamic_cast< SymbolButton* >( sender() )->m_Code;
  if( Code == msMean )
    Panel::sm_pEditor->m_pInEdit->m_IndReg = irShape;
  else
    {
    U_A_T Uact;
    Uact = Code;
    Uact.act = actPrintable;
    Panel::sm_pEditor->Editor( Uact );
    }
  Panel::sm_pEditor->setFocus();
  Panel::sm_pEditor->activateWindow();
  }

void Panel::setExam(bool Exam)
  {
  MainTab *pT = dynamic_cast< MainTab* >( m_pTabWidget->widget( tMainTab ) );
  pT->m_pF1->setEnabled( !Exam );
  pT->m_pF2->setEnabled( !Exam );
  pT->m_pRetry->setEnabled( !Exam );
  pT->m_pNewData->setEnabled( !Exam );
  }

PanelButton::PanelButton( const QString &Icon ) 
  {
  setStyleSheet( "QPushButton {margin-top:0px;margin-bottom:0px;}" );
  QPixmap Pixmap( Icon );
//  int Scale = sm_ButtonHeight / PixelHeight;
  QPixmap NewPix = Pixmap.scaledToHeight( sm_ButtonHeight - 6 );
  setIcon(NewPix);
  setIconSize(NewPix.size());
  setFlat(true);
  }

SymbolButton::SymbolButton(uchar Code) : m_Code(Code)
  {
  setStyleSheet( "QPushButton {font-size:25px}" );
  switch( Code )
    {
    case msCConjugation:
      setText( 'z' + QString( ToUnicode( msMean ) ) );
      break;
    case msMinute:
    case msDegree:
      setText( "()" + QString( ToUnicode( Code ) ) );
      break;
    default:
      setText( ToUnicode( Code ) );
    }
  connect( this, SIGNAL( clicked() ), Panel::sm_pPanel, SLOT( SymbolClicked() ) );
  }

SymbolsPanel::SymbolsPanel( const QString& Title, uchar Start, uchar End ) : QWidget( nullptr, Qt::WindowStaysOnTopHint )
  {
  setWindowTitle( Title );
  setWindowIcon( QIcon( ":/Resources/xpress99.ico" ) );
  QGridLayout *pLayout = new QGridLayout;
  int iRow = 0, iCol = 0;
  for( uchar C = Start; C <= End; C++, iCol++ )
    {
    if( iCol == 5 )
      {
      iCol = 0;
      iRow++;
      }
    pLayout->addWidget( new SymbolButton( C ), iRow, iCol );
    }
  setLayout( pLayout );
  hide();
  }

SymbolsPanel::SymbolsPanel( const QString& Title ) : QWidget( nullptr, Qt::WindowStaysOnTopHint )
  {
  setWindowTitle( Title );
  setWindowIcon( QIcon( ":/Resources/xpress99.ico" ) );
  uchar Symbols[] = { 
    msBird, msGenQuantifier, msExQuantifier, msUnion, msIntersection, msStrictInclusion, 
    msUnstrictInclusion, msAddition, msNotBelong, msMinequal, msMaxequal, msNotequal, 
    msApprox, msIdentity, msRound, msAround, msCConjugation, msPlusMinus, 
    msParallel, msCross, msTriangle, msArrowsUp, msArrowsUpDown, msArrowToRight,
    msDoubleArrow, msInfinity, ms3Points, msMean, msMinute, msDegree, 0
    };
  QGridLayout *pLayout = new QGridLayout; 
  for( int iRow = 0, iCol = 0, iC = 0; Symbols[iC] != 0; iC++, iCol++ )
    {
    if( iCol == 6 )
      {
      iCol = 0;
      iRow++;
      }
    pLayout->addWidget( new SymbolButton( Symbols[iC] ), iRow, iCol );
    }
  setLayout( pLayout );
  hide();
  }

MainTab::MainTab( Panel *pPanel ) : QWidget(pPanel) 
  {
  ScreenSize = QApplication::primaryScreen()->availableSize();
  PanelButton::sm_ButtonHeight = Round( 50 * ScreenSize.height() / 1080.0 );
//  setStyleSheet( "QPushButton { height:" + QString::number( PanelButton::sm_ButtonHeight ) +
//   "px;margin:0px;border-radius:10px;border-style:solid;border-width:2px; \
//   border-color:#ff93a6;background-color:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 pink,stop:1 black) ;min-width:80px;} \
//   QPushButton:pressed {background-color:qradialgradient(cx:0.0,cy:0.0,radius:1,fx:0.5,fy:0.5,stop:0 white,stop:0.2 rgba(255,30,30,144), \
//   stop:0.4 black);}" );
  m_pExit = new PanelButton( ":/Resources/ExitBtn.png");
  m_pF1 = new PanelButton( ":/Resources/F1Btn.png");
  m_pF2 = new PanelButton( ":/Resources/F2Btn.png");
  m_pNewData = new PanelButton( ":/Resources/NewDataBtn.png");
  m_pRetry = new PanelButton( ":/Resources/RetryBtn.png");
  m_pFraction = new PanelButton( ":/Resources/FractionBtn.png" );
  m_pSquareRoot = new PanelButton( ":/Resources/SQRMainBtn.png" );
  m_pPower = new PanelButton( ":/Resources/PowerMainBtn.png" );
  QVBoxLayout *pLayout = new QVBoxLayout;
  pLayout->addWidget(m_pF1 );
  pLayout->addWidget(m_pF2 );
  pLayout->addWidget( m_pNewData );
  pLayout->addWidget( m_pRetry );
  pLayout->addWidget( m_pFraction );
  pLayout->addWidget( m_pSquareRoot );
  pLayout->addWidget( m_pPower );
  pLayout->addWidget( m_pExit );
  setLayout( pLayout );
  connect( m_pSquareRoot, SIGNAL( clicked() ), SLOT( SqRootBtnClick() ) );
  connect( m_pFraction, SIGNAL( clicked() ), SLOT( FractionBtnClick() ) );
  connect(m_pExit, SIGNAL( clicked() ), SLOT( ExitBtnClick() ) );
  connect(m_pRetry, SIGNAL( clicked() ), SLOT( RetryBtnClick() ) );
  connect(m_pF1, SIGNAL( clicked() ), SLOT( HelpBtnClick() ) );
  connect(m_pF2, SIGNAL( clicked() ), SLOT( HintBtnClick() ) );
  connect(m_pNewData, SIGNAL( clicked() ), SLOT( NewDataBtnClick() ) );
  connect( m_pPower, SIGNAL( clicked() ), SLOT( PowrBtnClick() ) );
  }

void MainTab::LangSwitch()
  {
  m_pSquareRoot->setToolTip( X_Str( "SqRootBtnHint", "Square Root" ) );
  m_pExit->setToolTip( X_Str( "ExitBtnHint", "Finish and Exit" ) );
  m_pF1->setToolTip( X_Str( "F1BtnHint", "General Help" ) );
  m_pF2->setToolTip( X_Str( "F2BtnHint", "Step Result" ) );
  m_pFraction->setToolTip( X_Str( "FracBtnHint", "Simple Fraction" ) );
  m_pNewData->setToolTip( X_Str( "NewDataBtnHint", " Reset Data" ) );
  m_pRetry->setToolTip( X_Str( "EditPicBtnHint", "Edit Formula Picture" ) );
  m_pPower->setToolTip( X_Str( "PowerBtnHint", " Power" ) );
  }

void MainTab::RetryBtnClick()
  {
  Panel::sm_pEditor->RestoreFromFile();
  }

void MainTab::ExitBtnClick()
  {
  Panel::sm_pPanel->EmitExit();
  }

void MainTab::PlusBtnClick()
	{
	U_A_T Uact;
  Uact = msPlus;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
	}

void MainTab::MinusBtnClick()
	{
	U_A_T Uact;
  Uact = msMinus;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
	}

void MainTab::MultBtnClick()
	{
	U_A_T Uact;
  Uact = msMultSign1;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
	}

void MainTab::Mult2BtnClick()
	{
	U_A_T Uact;
  Uact = msMultSign2;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
	}

void MainTab::DiviBtnClick()
	{
	U_A_T Uact;
  Uact = msDivi;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
	}

void MainTab::EnterBtnClick()
  {
  if( sm_Enter != nullptr )
    sm_Enter();
  else
    Panel::sm_pEditor->SaveFormula();
  }

void MainTab::HelpBtnClick()
  {
  if( sm_Help != nullptr ) sm_Help();
  }

void MainTab::NewDataBtnClick()
  {
  if( sm_NewData != nullptr ) sm_NewData();
  }

void MainTab::HintBtnClick()
  {
  if( sm_Hint != nullptr ) sm_Hint();
  }

void MainTab::PowrBtnClick()
  {
  U_A_T Uact;
  Uact = "POWER";
  Panel::sm_pEditor->Editor( Uact );
  }

FunctionsTab::FunctionsTab( Panel *pPanel ) : QWidget( pPanel )
  {
  m_pExp = new PanelButton( ":/Resources/ExpBtnGr.png" );
  m_pLg = new PanelButton( ":/Resources/LgBtnGr.png" );
  m_pLog = new PanelButton( ":/Resources/AlgTab/BitBtnLog.png" );
  m_pSin = new PanelButton( ":/Resources/SinBtnGr.png" );
  m_pTan = new PanelButton( ":/Resources/TanBtnGr.png" );
  m_pLn = new PanelButton( ":/Resources/LnBtnGr.png" );
  m_pF = new PanelButton( ":/Resources/AlgTab/btnFunc.png" );
  m_pCos = new PanelButton( ":/Resources/CosBtnGr.png" );
  m_pGist = new PanelButton( ":/Resources/AlgTab/btnChart.png" );
  m_pGr = new PanelButton( ":/Resources/AlgTab/btnPlots.png" );
  m_pMeas = new PanelButton( ":/Resources/MeasBtn.png" );
  m_pVector = new PanelButton( ":/Resources/VectBtn.png" );
  QGridLayout *pLayout = new QGridLayout;
  pLayout->addWidget( m_pLog, 0, 0 );
  pLayout->addWidget( m_pLn, 1, 0 );
  pLayout->addWidget( m_pCos, 2, 0 );
  pLayout->addWidget( m_pExp, 3, 0 );
  pLayout->addWidget( m_pGist, 4, 0 );
  pLayout->addWidget( m_pMeas, 5, 0 );
  pLayout->addWidget( m_pLg, 0, 1 );
  pLayout->addWidget( m_pSin, 1, 1 );
  pLayout->addWidget( m_pTan, 2, 1 );
  pLayout->addWidget( m_pF, 3, 1 );
  pLayout->addWidget( m_pGr, 4, 1 );
  pLayout->addWidget( m_pVector, 5, 1 );
  setLayout( pLayout );
  connect(m_pLg, SIGNAL( clicked() ), SLOT( LgBtnClick() ) );
  connect(m_pLn, SIGNAL( clicked() ), SLOT( LnBtnClick() ) );
  connect(m_pExp, SIGNAL( clicked() ), SLOT( ExpBtnClick() ) );
  connect(m_pCos, SIGNAL( clicked() ), SLOT( CosBtnClick() ) );
  connect(m_pSin, SIGNAL( clicked() ), SLOT( SinBtnClick() ) );
  connect(m_pTan, SIGNAL( clicked() ), SLOT( TanBtnClick() ) );
  connect(m_pF, SIGNAL( clicked() ), SLOT( FuncBtnClick() ) );
  connect(m_pLog, SIGNAL( clicked() ), SLOT( LogBtnClick() ) );
  connect( m_pGist, SIGNAL( clicked() ), SLOT( ChartBtnClick() ) );
  connect( m_pGr, SIGNAL( clicked() ), SLOT( PlotBtnClick() ) );
  connect( m_pMeas, SIGNAL( clicked() ), SLOT( MeasBtnClick() ) );
  connect( m_pVector, SIGNAL( clicked() ), SLOT( VectorBtnClick() ) );
  }

void FunctionsTab::LangSwitch()
  {
  m_pExp->setToolTip( X_Str( "ExpBtnGrHint", " Exp(..)" ) );
  m_pLg->setToolTip( X_Str( "LgBtnGrHint", " Lg(..)" ) );
  m_pLog->setToolTip( X_Str( "BitBtnLogHint", " Logarithm" ) );
  m_pSin->setToolTip( X_Str( "SinBtnGrHint", " Sin(..)" ) );
  m_pTan->setToolTip( X_Str( "TanBtnGrHint", " Tan(..)" ) );
  m_pLn->setToolTip( X_Str( "LnBtnGrHint", " Ln(..)" ) );
  m_pF->setToolTip( X_Str( "BtnFuncHint", " Function" ) );
  m_pCos->setToolTip( X_Str( "CosBtnGrHint", " Cos(..)" ) );
  m_pGist->setToolTip( X_Str( "ChartBtnHint", " Chart" ) );
  m_pGr->setToolTip( X_Str( "MGraphPlot_Caption", "Plot Graph" ) );
  m_pMeas->setToolTip( X_Str( "MeasBtnHint", " Dimensions" ) );
  m_pVector->setToolTip( X_Str( "VectBtnHint", " Vector Notation" ) );
  }

void MainTab::FractionBtnClick()
  {
   U_A_T Uact;
  Uact = "FRACTION";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::LgBtnClick()
  {
  U_A_T Uact;
  Uact = "LG";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::LogBtnClick()
  {
  U_A_T Uact;
  Uact = "LOG";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::FuncBtnClick()
  {
  U_A_T Uact;
  Uact = "FUNC";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::LnBtnClick()
  {
  U_A_T Uact;
  Uact = "LN";
  Panel::sm_pEditor->Editor ( Uact );
  }

void FunctionsTab::ExpBtnClick()
  {
   U_A_T Uact;
  Uact = "EXP";
  Panel::sm_pEditor->Editor ( Uact );
  }

void FunctionsTab::SinBtnClick()
  {
   U_A_T Uact;
  Uact = "SIN";
  Panel::sm_pEditor->Editor ( Uact );
  }

void FunctionsTab::CosBtnClick()
  {
   U_A_T Uact;
  Uact = "COS";
  Panel::sm_pEditor->Editor ( Uact );
  }

void FunctionsTab::TanBtnClick()
  {
   U_A_T Uact;
  Uact = "TAN";
  Panel::sm_pEditor->Editor ( Uact );
  }

void MainTab::SqRootBtnClick()
  {
  U_A_T Uact;
  Uact = "SQROOT";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::ChartBtnClick()
  {
  U_A_T Uact;
  Uact = "CHARTEDITOR";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::PlotBtnClick()
  {
  /*
  U_A_T Uact;
  Uact = "TABLE";
  Panel::sm_pEditor->Editor( Uact );
  */
  }

void FunctionsTab::MeasBtnClick()
  {
  U_A_T Uact;
  Uact = "MEASURED";
  Panel::sm_pEditor->Editor( Uact );
  }

void FunctionsTab::VectorBtnClick()
  {
  U_A_T Uact;
  Uact = "VECTOR";
  Panel::sm_pEditor->Editor( Uact );
  }

MathTab::MathTab( Panel *pPanel ) : QWidget( pPanel )
  {
  m_pUnIntg = new PanelButton( ":/Resources/UnIntgrBtn.png" );
  m_pLim = new PanelButton( ":/Resources/LimBtn.png" );
  m_pFN = new PanelButton( ":/Resources/SubstBtn.png" );
  m_pAbs = new PanelButton( ":/Resources/AbsBtn.png" );
  m_pMatr = new PanelButton( ":/Resources/MatrBtn.png" );
  m_pLinePartDer = new PanelButton( ":/Resources/LinePartDer.png" );
  m_pDoubleIntegral = new PanelButton( ":/Resources/DoubleIntegral.png" );
  m_pCurveIntegral = new PanelButton( ":/Resources/CurveIntegral.png" );
  m_pSum = new PanelButton( ":/Resources/GSummBtn.png" );
  m_pDIntegr = new PanelButton( ":/Resources/DfIntgrBtn.png" );
  m_pMultipl = new PanelButton( ":/Resources/GMultBtn.png" );
  m_pDiffr = new PanelButton( ":/Resources/DerivBtn.png" );
  m_pDiffrN = new PanelButton(":/Resources/DerivBtnN.png");
  m_pLineDer = new PanelButton( ":/Resources/LineDer.png" );
  m_pPartlyDer = new PanelButton( ":/Resources/PartlyDer.png" );
  m_pPartlyDerN = new PanelButton(":/Resources/PartlyDerN.png");
  m_pSurfaceIntegral = new PanelButton( ":/Resources/SurfaceIntegral.png" );
  m_pCounterIntegral = new PanelButton( ":/Resources/CountreIntegral.png" );
  QGridLayout *pLayout = new QGridLayout;
  pLayout->addWidget(m_pUnIntg, 0, 0 );
  pLayout->addWidget(m_pLim, 1, 0 );
  pLayout->addWidget(m_pFN, 2, 0 );
  pLayout->addWidget(m_pAbs, 3, 0 );
  pLayout->addWidget(m_pMatr, 4, 0 );
  pLayout->addWidget(m_pLinePartDer, 5, 0 );
  pLayout->addWidget(m_pDiffrN, 6, 0);
  pLayout->addWidget(m_pDoubleIntegral, 7, 0 );
  pLayout->addWidget(m_pCurveIntegral, 8, 0 );
  pLayout->addWidget(m_pSum, 0, 1 );
  pLayout->addWidget(m_pDIntegr, 1, 1 );
  pLayout->addWidget(m_pMultipl, 2, 1 );
  pLayout->addWidget(m_pDiffr, 3, 1 );
  pLayout->addWidget(m_pLineDer, 4, 1 );
  pLayout->addWidget(m_pPartlyDer, 5, 1 );
  pLayout->addWidget(m_pPartlyDerN, 6, 1);
  pLayout->addWidget(m_pSurfaceIntegral, 7, 1 );
  pLayout->addWidget(m_pCounterIntegral, 8, 1 );
  setLayout( pLayout );
  connect(m_pAbs, SIGNAL( clicked() ), SLOT( AbsBtnClick() ) );
  connect(m_pCounterIntegral, SIGNAL( clicked() ), SLOT( CounterIntegralClick() ) );
  connect(m_pUnIntg, SIGNAL( clicked() ), SLOT( IntegrBtnClick() ) );
  connect(m_pMatr, SIGNAL( clicked() ), SLOT( MatrBtnClick() ) );
  connect(m_pLim, SIGNAL( clicked() ), SLOT( LimitBtnClick() ) );
  connect(m_pSurfaceIntegral, SIGNAL( clicked() ), SLOT( SurfaceIntegralClick() ) );
  connect(m_pDiffr, SIGNAL( clicked() ), SLOT( DerivBtnClick() ) );
  connect(m_pPartlyDer, SIGNAL( clicked() ), SLOT( PartlyDerClick() ) );
  connect(m_pLinePartDer, SIGNAL( clicked() ), SLOT( LinePartDerClick() ) );
  connect(m_pDoubleIntegral, SIGNAL( clicked() ), SLOT( DoubleIntegralClick() ) );
  connect(m_pCurveIntegral, SIGNAL( clicked() ), SLOT( CurveIntegralClick() ) );
  connect(m_pLineDer, SIGNAL( clicked() ), SLOT( LineDerClick() ) );
  connect(m_pDIntegr, SIGNAL( clicked() ), SLOT( DfIntegrBtnClick() ) );
  connect(m_pSum, SIGNAL( clicked() ), SLOT( GSummBtnClick() ) );
  connect(m_pMultipl, SIGNAL( clicked() ), SLOT( GMultBtnClick() ) );
  connect(m_pFN, SIGNAL( clicked() ), SLOT( SubstBtnClick() ) );
  connect(m_pDiffrN, SIGNAL(clicked()), SLOT(DerivBtnNClick()));
  connect(m_pPartlyDerN, SIGNAL(clicked()), SLOT(PartlyDerNClick()));
  }

void MathTab::LangSwitch()
  {
  m_pUnIntg->setToolTip( X_Str( "UnIntgrBtnHint", "Indefinite Integral" ) );
  m_pLim->setToolTip( X_Str( "LimBtnHint", " Limit" ) );
  m_pFN->setToolTip( X_Str( "SubstBtnHint", " Substitution" ) );
  m_pAbs->setToolTip( X_Str( "AbsBtnHint", " Absolute value" ) );
  m_pMatr->setToolTip( X_Str( "MatrBtnHint", " Matrix" ) );
  m_pDoubleIntegral->setToolTip( X_Str( "DoubleIntegralHit", "Double Integral" ) );
  m_pCurveIntegral->setToolTip( X_Str( "CurveIntegralHint", "Curve Integral" ) );
  m_pSum->setToolTip( X_Str( "GSummBtnHint", " Sum of series" ) );
  m_pDIntegr->setToolTip( X_Str( "DfIntgrBtnHint", " Definite integral" ) );
  m_pMultipl->setToolTip( X_Str( "GMultBtnHint", " Product" ) );
  m_pSurfaceIntegral->setToolTip( X_Str( "SurfaceIntegralBtnHint", " Surface Integral" ) );
  m_pCounterIntegral->setToolTip( X_Str( "CounterIntegralBtnHint", "Counter Integral" ) );
  m_pLineDer->setToolTip(X_Str("LineDerBtnHint", " Derivative by Line"));
  m_pLinePartDer->setToolTip(X_Str("LinePartDerHint", " Parial Derivative by Line"));
  m_pDiffrN->setToolTip(X_Str("DerivBtnHintH", " Derivative of high Order"));
  m_pDiffr->setToolTip(X_Str("DerivBtnHint", " Derivative"));
  m_pPartlyDerN->setToolTip(X_Str("PartlyDerBtnHintH", " Parial Derivative of high Order"));
  m_pPartlyDer->setToolTip(X_Str("PartlyDerBtnHint", " Parial Derivative"));
  }

void MathTab::SubstBtnClick()
  {
  U_A_T Uact;
  Uact = "SUBST";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::GMultBtnClick()
  {
   U_A_T Uact;
  Uact = "GMULT";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::GSummBtnClick()
  {
  U_A_T Uact;
  Uact = "GSUMMA";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::DfIntegrBtnClick()
  {
  U_A_T Uact;
  Uact = "DEFINTEGRAL";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::AbsBtnClick()
  {
  U_A_T Uact;
  Uact = "ABS";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::CounterIntegralClick()
  {
  XPInEdit::sm_Text = SimpleChar( msContourIntegral );
  U_A_T Uact;
  Uact = "CRVINTEGRAL";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::IntegrBtnClick()
  {
  U_A_T Uact;
  Uact = "INTEGRAL";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::MatrBtnClick()
  {
   U_A_T Uact;
   Uact = "CREATEMATRIX";
   Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::LimitBtnClick()
  {
   U_A_T Uact;
   Uact = "LIMIT";
   Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::SurfaceIntegralClick()
  {
  XPInEdit::sm_Text = SimpleChar( msDoubleIntegral );
  U_A_T Uact;
  Uact = "CRVINTEGRAL";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::DerivBtnClick()
  {
  U_A_T Uact;
  Uact = "DERIVATIVE";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::PartlyDerClick()
  {
  U_A_T Uact;
  Uact = "PARTDERIV";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::DerivBtnNClick()
  {
  Panel::sm_pEditor->RestoreFormula((QByteArray("\\der{}{\\power{}{}}")));
  }

void MathTab::PartlyDerNClick()
  {
  Panel::sm_pEditor->RestoreFormula((QByteArray("\\prtder{}{\\power{}{}}")));
  }

void MathTab::LinePartDerClick()
  {
  U_A_T Uact;
  Uact = "STROKEPARTDERIV";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::DoubleIntegralClick()
  {
   U_A_T Uact;
   Uact = "DBLINTEGRAL";
   Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::CurveIntegralClick()
  {
  XPInEdit::sm_Text = SimpleChar( msIntegral );
  U_A_T Uact;
  Uact = "CRVINTEGRAL";
  Panel::sm_pEditor->Editor( Uact );
  }

void MathTab::LineDerClick()
  {
  U_A_T Uact;
  Uact = "STROKEDERIV";
  Panel::sm_pEditor->Editor( Uact );
  }

AlgebraTab::AlgebraTab( Panel *pPanel ) : QWidget( pPanel )
  {
  m_pBinC = new PanelButton( ":/Resources/bbBinomCoef.png" );
  m_pABin = new PanelButton( ":/Resources/bbPlcmCoef.png" );
  m_pSyst = new PanelButton( ":/Resources/AlgTab/SystBtn.png" );
  m_pTable = new PanelButton( ":/Resources/AlgTab/Table.png" );
  m_pNewLine = new PanelButton( ":/Resources/AlgTab/NewLineBtnA.png" );
  m_pPC = new PanelButton( ":/Resources/bbPerCount.png" );
  m_pIm = new PanelButton( ":/Resources/bbImUnit.png" );
  m_pInd = new PanelButton( ":/Resources/AlgTab/IndexBtn.png" );
  m_pSub = new PanelButton( ":/Resources/AlgTab/LowRegBtn.png" );
  m_pSup = new PanelButton( ":/Resources/AlgTab/HiRegBtn.png" );
  m_pFrac = new PanelButton( ":/Resources/FracBtnGr.png" );
  m_pRoot = new PanelButton( ":/Resources/RootBtnGr.png" );
  m_pSqrt = new PanelButton(":/Resources/SqRootBtn.png");
  m_pMult2 = new PanelButton( ":/Resources/Mult2Btn.png");
  m_pMult = new PanelButton( ":/Resources/MultBtn.png");
  m_pPover = new PanelButton( ":/Resources/PowerBtnGr.png" );
  QGridLayout *pLayout = new QGridLayout;
  pLayout->addWidget( m_pFrac, 0, 0 );
  pLayout->addWidget( m_pSqrt, 1, 0 );
  pLayout->addWidget( m_pMult, 2, 0 );
  pLayout->addWidget( m_pSup, 3, 0 );
  pLayout->addWidget( m_pInd, 4, 0 );
  pLayout->addWidget( m_pBinC, 5, 0 );
  pLayout->addWidget(m_pABin, 6, 0 );
  pLayout->addWidget( m_pTable, 7, 0 );
  pLayout->addWidget( m_pPover, 0, 1 );
  pLayout->addWidget( m_pRoot, 1, 1 );
  pLayout->addWidget( m_pMult2, 2, 1 );
  pLayout->addWidget( m_pSub, 3, 1 );
  pLayout->addWidget( m_pSyst, 4, 1 );
  pLayout->addWidget(m_pPC, 5, 1 );
  pLayout->addWidget(m_pIm, 6, 1 );
  pLayout->addWidget( m_pNewLine, 7, 1 );
  setLayout( pLayout );
  connect(m_pBinC, SIGNAL( clicked() ), SLOT( BinomCoeffBtnClick() ) );
  connect(m_pABin, SIGNAL( clicked() ), SLOT( ABinomCoeffBtnClick() ) );
  connect(m_pSyst, SIGNAL( clicked() ), SLOT( SystBtnClick() ) );
  connect(m_pTable, SIGNAL( clicked() ), SLOT( TableBtnClick() ) );
  connect(m_pNewLine, SIGNAL( clicked() ), SLOT( NewLineBtnClick() ) );
  connect(m_pPC, SIGNAL( clicked() ), SLOT( PerCountBtnClick() ) );
  connect(m_pIm, SIGNAL( clicked() ), SLOT( ImUnitBtnClick() ) );
  connect(m_pInd, SIGNAL( clicked() ), SLOT( IndxBtnClick() ) );
  connect(m_pSub, SIGNAL( clicked() ), SLOT( SubBtnClick() ) );
  connect(m_pSup, SIGNAL( clicked() ), SLOT( SupBtnClick() ) );
  connect( m_pPover, SIGNAL( clicked() ), SLOT( PowrBtnClick() ) );
  connect( m_pFrac, SIGNAL( clicked() ), SLOT( FracBtnClick() ) );
  connect( m_pRoot, SIGNAL( clicked() ), SLOT( RootBtnClick() ) );
  connect( m_pSqrt, SIGNAL( clicked() ), SLOT( SqRootBtnClick() ) );
  connect (m_pMult, SIGNAL(clicked()), SLOT(MultBtnClick()));
  connect (m_pMult2, SIGNAL(clicked()), SLOT(Mult2BtnClick()));
  }

void AlgebraTab::LangSwitch()
  {
  m_pBinC->setToolTip( X_Str( "MBinomCoef", "Binomial Coefficients" ) );
  m_pABin->setToolTip( X_Str( "MPlcmCoef", "Permutations of k of n" ) );
  m_pSyst->setToolTip( X_Str( "SystBtnHint", " Left bracket" ) );
  m_pTable->setToolTip( X_Str( "FracBtnGrHint", " Table" ) );
  m_pNewLine->setToolTip( X_Str( "NewLn2BtnHint", "Start New Line" ) );
  m_pPC->setToolTip( X_Str( "MPerCount", "Permutations of n of n" ) );
  m_pIm->setToolTip( X_Str( "MImUnit", "Imaginary Unit" ) );
  m_pInd->setToolTip( X_Str( "IndexBtnHint", "Index" ) );
  m_pSub->setToolTip( X_Str( "LowRegBtnHint", "Subscript" ) );
  m_pSup->setToolTip( X_Str( "HiRegBtnHint", "Superscript" ) );
  m_pMult2->setToolTip( X_Str( "Mult2BtnHint", " a x b" ) );
  m_pMult->setToolTip( X_Str( "MultBtnHint", " a * b" ) );
  m_pFrac->setToolTip( X_Str( "FracBtnHint", "Simple Fraction" ) );
  m_pPover->setToolTip( X_Str( "PowerBtnHint", " Power" ) );
  m_pSqrt->setToolTip( X_Str( "SqRootBtnHint", "Square Root" ) );
  m_pRoot->setToolTip( X_Str( "RootBtnHint", " Root" ) );
  }

void AlgebraTab::BinomCoeffBtnClick()
  {
  U_A_T Uact;
  Uact = "BinomCoeff";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::ABinomCoeffBtnClick()
  {
  U_A_T Uact;
  Uact = "ABinomCoeff";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::SystBtnClick()
  {
  U_A_T Uact;
  Uact = "SYSTEM";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::TableBtnClick()
  {
  U_A_T Uact;
  Uact = "CREATETABLE";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::NewLineBtnClick()
  {
  U_A_T Uact;
  Uact = ';';
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::PerCountBtnClick()
  {
  U_A_T Uact;
  Uact = "PerCount";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::ImUnitBtnClick()
  {
  U_A_T Uact;
  Uact = "IMUNIT";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::IndxBtnClick()
  {
  U_A_T Uact;
  Uact = "INDEX";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::SupBtnClick()
  {
  Panel::sm_pEditor->m_pInEdit->SetRegister(irPower);
  Panel::sm_pEditor->MoveCursor();
  Panel::sm_pEditor->setFocus();
  }

void AlgebraTab::SubBtnClick()
  {
  Panel::sm_pEditor->m_pInEdit->SetRegister(irIndex);
  Panel::sm_pEditor->MoveCursor();
  Panel::sm_pEditor->setFocus();
  }

void AlgebraTab::FracBtnClick()
  {
  U_A_T Uact;
  Uact = "FRACTION";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::RootBtnClick()
  {
  U_A_T Uact;
  Uact = "ROOT";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::PowrBtnClick()
  {
  U_A_T Uact;
  Uact = "POWER";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::SqRootBtnClick()
  {
  U_A_T Uact;
  Uact = "SQROOT";
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::MultBtnClick()
  {
  U_A_T Uact;
  Uact = msMultSign1;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
  }

void AlgebraTab::Mult2BtnClick()
  {
  U_A_T Uact;
  Uact = msMultSign2;
  Uact.act = actPrintable;
  Panel::sm_pEditor->Editor( Uact );
  }

