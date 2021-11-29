#ifndef MATH_TASKWORK
#define MATH_TASKWORK

#include "OutWindow.h"
#include "QuestWindow.h"
#include "../CommonWidgets/Panel.h"
#include "../TaskFileManager/taskfilemanager.h"

void TaskHelpShow();
void TaskHintShow();
void AnsverEnter();
void NewData();

class TXPTask : public BaseTask
  {
  friend class WinTesting;
  HelpTaskWindow *m_pHelpWindow;
  HelpPanelWindow *m_pHintWindow;
  HelpPanelWindow *m_pPromptWindow;
  void HideHelps() 
    { 
    delete m_pHelpWindow;  m_pHelpWindow = nullptr; 
    delete m_pHintWindow; m_pHintWindow = nullptr; 
    delete m_pPromptWindow; m_pPromptWindow = nullptr;
    }
  public:
    static bool sm_EditTask;
    virtual void OutTemplate( const QByteArray& sTemp );
    TXPTask();
    virtual void Clear();
    void ResetParms();
    void AssignVar( const MathExpr& exi, OutWindow* pOutWin, const QByteArray& event );
    QString CompareUserExpr( const MathExpr& exi, OutWindow* pOutWin );
    QByteArray ComparePreliminaryUserExpr( const MathExpr& exi, OutWindow* pOutWin, bool& LastStepSolved );
    virtual void LoadTrackDependentFromFile();
    virtual void SetCurr( PStepMemb NewCurr );
    void Exam( const MathExpr& exi, OutWindow* pOutWin );
    bool ProcessEnter( const QByteArray& InpStr, OutWindow* pOutWin );
    bool ProcessPreliminaryEnter( const QByteArray& InpStr, OutWindow* pOutWin, bool& LastStepSolved );
    bool ProcessEnter( const MathExpr& exi, OutWindow* pOutWin );
    bool ProcessDefine( const QByteArray& InpStr, OutWindow* pOutWin );
    bool CheckAndShowExpr( const QByteArray& InpStr, OutWindow* pOutWin );
    PPromptDescr StepPrompt();
    virtual PPromptDescr OneStepPrompt();
    virtual PPromptDescr OptStepPrompt();
    bool SelectTrack( bool bIsHelp = false );
    void StartExam();
    bool HelpShow();
    bool HintShow();
    bool ShowPrompt();
    void EntBtnClick();
    void OutTaskPrompt( OutWindow* pOutWin );
  };

#endif