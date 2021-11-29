#ifndef MATH_PANEL
#define MATH_PANEL

#include <QtWidgets>
#include "XPGedit.h"
#include "GraphEdit.h"

extern COMMONWIDGETS_EXPORT double PixelHeight;
extern COMMONWIDGETS_EXPORT QSize ScreenSize;

void TileXPressForms();

class Panel;

class PanelButton : public QPushButton
  {
  public:
    COMMONWIDGETS_EXPORT static double sm_ButtonHeight;
    COMMONWIDGETS_EXPORT PanelButton( const QString &Icon );
   };

class SymbolButton : public QPushButton
  {
  uchar m_Code;
  friend Panel;
  public:
    SymbolButton( uchar Code );
  };

class MainTab : public QWidget
  {  
  Q_OBJECT
    friend Panel;
  PanelButton *m_pF1;
  PanelButton *m_pRetry;
  PanelButton *m_pFraction;
  PanelButton *m_pSquareRoot;
  PanelButton *m_pPower;
  public:
    PanelButton *m_pNewData;
    PanelButton *m_pF2;
    PanelButton *m_pExit;
    MainTab( Panel *pPanel );
    COMMONWIDGETS_EXPORT static void ( *sm_Help )();
    COMMONWIDGETS_EXPORT static void( *sm_Hint )();
    COMMONWIDGETS_EXPORT static void( *sm_Enter )();
    COMMONWIDGETS_EXPORT static void( *sm_NewData )();
    void LangSwitch();
    public slots:
     void SqRootBtnClick();
     void FractionBtnClick();
		 void PlusBtnClick();
		 void MinusBtnClick();
		 void MultBtnClick();
		 void Mult2BtnClick();
		 void DiviBtnClick();
     void ExitBtnClick();
     void EnterBtnClick();
     void RetryBtnClick();
     void HelpBtnClick();
     void HintBtnClick();
     void NewDataBtnClick();
     void PowrBtnClick();
  };

class FunctionsTab : public QWidget
  {
  Q_OBJECT
  PanelButton *m_pExp;
  PanelButton *m_pLg;
  PanelButton *m_pLog;
  PanelButton *m_pSin;
  PanelButton *m_pTan;
  PanelButton *m_pLn;
  PanelButton *m_pF;
  PanelButton *m_pCos;
  PanelButton *m_pGist;
  PanelButton *m_pGr;
  PanelButton *m_pMeas;
  PanelButton *m_pVector;
  public:
    FunctionsTab(Panel *pPanel);
    void LangSwitch();
    public slots:
    void LgBtnClick();
    void LnBtnClick();
    void SinBtnClick();
    void CosBtnClick();
    void TanBtnClick();
    void ExpBtnClick();
    void FuncBtnClick();
    void LogBtnClick();
    void MeasBtnClick();
    void VectorBtnClick();
    void ChartBtnClick();
    void PlotBtnClick();
  };

class MathTab : public QWidget
  {
  Q_OBJECT
    PanelButton *m_pUnIntg;
  PanelButton *m_pLim;
  PanelButton *m_pFN;
  PanelButton *m_pAbs;
  PanelButton *m_pMatr;
  PanelButton *m_pLinePartDer;
  PanelButton *m_pDoubleIntegral;
  PanelButton *m_pCurveIntegral;
  PanelButton *m_pSum;
  PanelButton *m_pDIntegr;
  PanelButton *m_pMultipl;
  PanelButton *m_pDiffr;
  PanelButton *m_pDiffrN;
  PanelButton *m_pLineDer;
  PanelButton *m_pPartlyDer;
  PanelButton *m_pPartlyDerN;
  PanelButton *m_pSurfaceIntegral;
  PanelButton *m_pCounterIntegral;
  public:
    MathTab(Panel *pPanel);
    void LangSwitch();
    public slots:
      void AbsBtnClick(); 
      void CounterIntegralClick();
      void IntegrBtnClick();
      void MatrBtnClick(); 
      void LimitBtnClick();
      void SurfaceIntegralClick();
      void DerivBtnClick();
      void DerivBtnNClick();
      void PartlyDerClick();
      void PartlyDerNClick();
      void LinePartDerClick();
      void DoubleIntegralClick();
      void CurveIntegralClick();
      void LineDerClick();
      void DfIntegrBtnClick();
      void GSummBtnClick();
      void GMultBtnClick();
      void SubstBtnClick();
  };

class AlgebraTab : public QWidget
  {
  Q_OBJECT
  PanelButton *m_pFrac;
  PanelButton *m_pPover;
  PanelButton *m_pRoot;
  PanelButton *m_pSqrt;
  PanelButton *m_pMult2;
  PanelButton *m_pMult;
  PanelButton *m_pBinC;
  PanelButton *m_pABin;
  PanelButton *m_pSyst;
  PanelButton *m_pTable;
  PanelButton *m_pNewLine;
  PanelButton *m_pPC;
  PanelButton *m_pIm;
  PanelButton *m_pInd;
  PanelButton *m_pSub;
  PanelButton *m_pSup;
  public:
    AlgebraTab( Panel *pPanel );
    void LangSwitch();
    public slots:
    void BinomCoeffBtnClick();
    void ABinomCoeffBtnClick();
    void SystBtnClick();
    void TableBtnClick();
    void NewLineBtnClick();
    void PerCountBtnClick();
    void ImUnitBtnClick();
    void IndxBtnClick();
    void SubBtnClick();
    void SupBtnClick();
    void SqRootBtnClick();
    void FracBtnClick();
    void MultBtnClick();
    void Mult2BtnClick();
    void PowrBtnClick();
    void RootBtnClick();
  };

class SymbolsPanel : public QWidget
  {
  public:
    SymbolsPanel( const QString& Title, uchar Start, uchar End );
    SymbolsPanel( const QString& Title );
  };

class Panel : public QWidget
  {
  Q_OBJECT
    QTabWidget *m_pTabWidget;
  QLabel *m_pTitle;
  public:
    MainTab *m_pMainTab;
    enum Tabs { tMainTab, tFunctionsTab, tMathTab, tAlgebraTab, tGraphEditTab };
    COMMONWIDGETS_EXPORT static Panel* sm_pPanel;
    COMMONWIDGETS_EXPORT static bool sm_GeoActivate;
    COMMONWIDGETS_EXPORT static XPGedit *sm_pEditor;
    COMMONWIDGETS_EXPORT static GraphEdit *sm_pGraphEdit;
    COMMONWIDGETS_EXPORT static SymbolsPanel *sm_pGreek;
    COMMONWIDGETS_EXPORT static SymbolsPanel *sm_pBigGreek;
    COMMONWIDGETS_EXPORT static SymbolsPanel *sm_pMathSymbol;
    COMMONWIDGETS_EXPORT Panel( QWidget *parent = 0 );
    COMMONWIDGETS_EXPORT void LangSwitch();
    COMMONWIDGETS_EXPORT void HideSymbols();
    COMMONWIDGETS_EXPORT void setExam(bool);
    void SetTab( Tabs Tab ) { m_pTabWidget->setCurrentIndex( Tab ); }
    int GetCurrentTab() { return m_pTabWidget->currentIndex(); }
    COMMONWIDGETS_EXPORT void EmitExit();
  signals:
    COMMONWIDGETS_EXPORT void ExitBtn();
  public slots:
    void SymbolClicked();
  };

#endif
