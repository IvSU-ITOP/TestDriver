#include "TaskTools.h"
#include "TaskWork.h"
#include "../Mathematics/Parser.h"
#include "WinTesting.h"
#include "../Mathematics/ExpStore.h"
#include "../Mathematics/Statistics.h"

TXPTask s_Task;
bool TXPTask::sm_EditTask = false;

TXPTask::TXPTask() : m_pHelpWindow( nullptr ), m_pHintWindow( nullptr ), m_pPromptWindow(nullptr)
  {}

void TXPTask::Clear()
  {
  BaseTask::Clear();
  HideHelps();
  }

void TXPTask::ResetParms()
  {
  QuestWindow::sm_ShowUnarm = BaseTask::sm_GlobalShowUnarMinus;
  QuestWindow::sm_ShowRad = BaseTask::sm_GlobalShowRad;
  QuestWindow::sm_ShowMinusByAdd = BaseTask::sm_GlobalShowMinusByAddition;
  XPGedit::sm_ShowMultSignInQWindow = BaseTask::sm_GlobalShowMultSign;
  WinTesting::sm_pMainWindow->SetBtnShowUnar();
  WinTesting::sm_pMainWindow->SetBtnShowUnarByAdd();
  WinTesting::sm_pMainWindow->SetBtnShowMultSign();
  }

void TXPTask::LoadTrackDependentFromFile()
  {
  if( Panel::sm_pGraphEdit != nullptr )
    Panel::sm_pGraphEdit->CancelComparison();
  BaseTask::LoadTrackDependentFromFile();
  }

void TXPTask::SetCurr( PStepMemb pNewCurr )
  {
  m_pCurr = pNewCurr;
  if( m_pCurr.isNull() ) return;
  if( m_pCurr->m_ShowUnarm != BaseTask::sm_GlobalShowUnarMinus )
    WinTesting::sm_pMainWindow->btnShowUnarClick();
  if (m_pCurr->m_ShowMinusByAdd != BaseTask::sm_GlobalShowMinusByAddition)
    WinTesting::sm_pMainWindow->btnShowUnarByAddClick();
  if( m_pCurr->m_ShowMSign != BaseTask::sm_GlobalShowMultSign )
    WinTesting::sm_pMainWindow->btnShowMultSignClick();
  BaseTask::sm_GlobalShowRad = m_pCurr->m_ShowRad;
  for( PDescrMemb pIndex = m_pCurr->m_pResultE->m_pFirst; !pIndex.isNull(); pIndex = pIndex->m_pNext )
    if( pIndex->m_Kind == tXDexpress )
      {
      if( pIndex->m_CompareType == AsConstraint )
        {
        Panel::sm_pGraphEdit->SetConstraints( CastPtr( TPolygon, Parser::StrToExpr( pIndex->m_Content ) )->Points() );
        m_WasConstraint = true;
        if( m_pCurr->m_pPrev.isNull() )
          {
          Panel::sm_pPanel->SetTab( Panel::tAlgebraTab );
          Panel::sm_pEditor->activateWindow();
          }
        else
          {
          if( Panel::sm_pPanel->GetCurrentTab() != Panel::tGraphEditTab )
            Panel::sm_pPanel->SetTab( Panel::tGraphEditTab );
          Panel::sm_pGraphEdit->activateWindow();
          }
        }
      else
        {
        if( m_WasConstraint ) Panel::sm_pGraphEdit->SetTraining( GraphEdit::NoTraining );
        m_WasConstraint = false;
        //        if( Panel::sm_pPanel->GetCurrentTab() != Panel::tAlgebraTab )
        //          Panel::sm_pPanel->SetTab( Panel::tAlgebraTab );
        Panel::sm_pEditor->activateWindow();
        if( !m_pCurr->m_pAnswerTemplate->m_pFirst.isNull() &&
          m_pCurr->m_pAnswerTemplate->m_pFirst->m_Kind == tXDexpress &&
          ( m_WorkMode == wrkLearn || m_WorkMode == wrkTrain ) )
          if( m_OutTemplate )
            OutTemplate( m_pCurr->m_pAnswerTemplate->m_pFirst->m_Content );
          else
            m_Template = m_pCurr->m_pAnswerTemplate->m_pFirst->m_Content;
        }
      return;
      }
  }

void TXPTask::OutTemplate( const QByteArray& sTemp )
    {
    if( sTemp.isEmpty() || s_TaskEditorOn ) return;
    Panel::sm_pEditor->m_pInEdit->Clear();
    EdTable::sm_WasTemplate = true;
    Panel::sm_pEditor->UnReadExpr( Parser::StrToExpr( sTemp ) );
    EdTable::sm_WasTemplate = false;
    Panel::sm_pEditor->RefreshXPE();
    }

QByteArray TXPTask::ComparePreliminaryUserExpr( const MathExpr& exi, OutWindow* pOutWin, bool& LastStepSolved )
  {
  bool Nothing_coinside;
  PDescrMemb pIndex;
  MathExpr exc;

  auto LOOK_FALSE = [&] ()
    {
    if( !Nothing_coinside ) return;
    if( pIndex.isNull() )
      {
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted" ) );
      return;
      }
    if( pIndex->m_Kind != tXDexpress ) return;
    s_RestoreObjectPicture = false;
    exc = Parser::StrToExpr( pIndex->m_Content );
    s_RestoreObjectPicture = true;
    if( s_GlobalInvalid )
      {
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pIndex->m_Content );
      return;
      }
    if( m_ExactCompare && exi.Eq( exc ) || !m_ExactCompare && exi.Equal( exc ) )
      {
      Nothing_coinside = false;
      while( !pIndex->m_pNext.isNull() && pIndex->m_pNext->m_Kind == tXDnewline )
        pIndex = pIndex->m_pNext;
      }
    };

  Nothing_coinside = true;
  LastStepSolved = false;
  QByteArray Txt_compare;
  QByteArray CurrName;
  //  {Check current answer,  if(it coincides with ANY Result in ANY Step	}
  bool MainComparison = true;
  QByteArray Result;
  PStepMemb pSindex = m_pStepsL->m_pFirst;
  for( ; !pSindex.isNull(); pSindex = pSindex->m_pNext )
    for( pIndex = pSindex->m_pResultE->m_pFirst; !pIndex.isNull() && Nothing_coinside; pIndex = pIndex->m_pNext )
      {
      if( pIndex->m_Kind != tXDexpress || pIndex->m_CompareType == AsConstraint && pIndex->m_Already ) continue;
      s_RestoreObjectPicture = false;
      exc = Parser::StrToExpr( pIndex->m_Content );
      s_RestoreObjectPicture = true;
      if( s_GlobalInvalid )
        {
        s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pIndex->m_Content );
        return Result;
        }
      MathExpr exComp = exc;
      if( ( pIndex->m_Content.indexOf( "Table" ) > -1 || pIndex->m_Content.indexOf( "SolutionSystNonEq" ) > -1 ) && IsType( TLexp, exc ) )
        for( PExMemb pIndexL = CastPtr( TLexp, exc )->First(); !pIndexL.isNull(); pIndexL = pIndexL->m_pNext )
          if( pIndexL->m_Memb.WriteE() != "" && !( IsType( TTable, pIndexL->m_Memb ) ) )
            {
            exComp = pIndexL->m_Memb;
            break;
            }
      if(m_ExactCompare && exi.Eq( exComp ) || !m_ExactCompare && exi.Equal( exComp ) )
        {
        Nothing_coinside = false;
        Result = "OK";
        pIndex->m_Already = true;
        pSindex->m_pMethodL->m_Already = true;
        if( pSindex->m_pNext.isNull() && s_XPMonitorMode ) m_Exiting = true;
        CurrName = pSindex->m_Name;
        }
      MainComparison = false;
      }

  for( pSindex = m_pStepsL->m_pFirst; !pSindex->m_pNext.isNull(); pSindex = pSindex->m_pNext );
  LastStepSolved = pSindex->m_pMethodL->m_Already;
  //{Check,  if(ALL Steps are decided }
  bool All_the_task = true;
  m_Something_skipped = false;
  for( pSindex = m_pStepsL->m_pFirst; !pSindex.isNull() && !m_Something_skipped && !pSindex->m_Name.isEmpty(); pSindex = pSindex->m_pNext )
    if( !pSindex->m_pMethodL->m_Already )
      {
      if( All_the_task ) SetCurr( pSindex );
      All_the_task = false;
      }
    else
      if( !All_the_task )
        m_Something_skipped = true;

  if( All_the_task )
    {
    if( m_WasConstraint ) Panel::sm_pGraphEdit->SetConstraints( nullptr );
    m_WasConstraint = false;
    }

  for( pSindex = m_pStepsL->m_pFirst; !pSindex.isNull() && Nothing_coinside && m_WorkMode == wrkTrain; pSindex = pSindex->m_pNext )
    {
    //      (* LOOK_FALSE has to work only in TIRGUL mode.*)
    //      (* Swich it out in the CALCULATOR program     *)
    pIndex = pSindex->m_pF1->m_pFirst;
    LOOK_FALSE();
    pIndex = pSindex->m_pF2->m_pFirst;
    LOOK_FALSE();
    pIndex = pSindex->m_pF3->m_pFirst;
    LOOK_FALSE();
    }

  if( All_the_task && !m_pCurr.isNull() )
    {
    WinTesting::sm_pOutWindow->AddPrompt( m_pComment->m_pFirst );
    SetCurr( PStepMemb() );
    }
  if( Nothing_coinside ) Result = Txt_compare;
  return Result;
  }

QString TXPTask::CompareUserExpr( const MathExpr& exi, OutWindow* pOutWin )
  {
  bool Nothing_coinside;
  PDescrMemb pIndex;
  MathExpr exc;

  auto Compare = [&] (const MathExpr& exi, const MathExpr& exComp )
    {
    double Precision = TExpr::sm_Precision;
    TExpr::sm_Precision = TExpr::sm_Accuracy;
    bool Result;
    if(m_ExactCompare)
      Result = exi.Eq( exComp );
    else
      {
      bool OldNoReduceByCompare = MathExpr::sm_NoReduceByCompare;
      MathExpr::sm_NoReduceByCompare = false;
      Result = exi.Equal( exComp );
      MathExpr::sm_NoReduceByCompare = OldNoReduceByCompare;
      }
    TExpr::sm_Precision = Precision;
    return Result;
    };

  auto LOOK_FALSE = [&] ()
    {
    if( !Nothing_coinside ) return;
    if( pIndex.isNull() )
      {
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted" ) );
      return;
      }
    if( pIndex->m_Kind != tXDexpress ) return;

    exc = Parser::StrToExpr( pIndex->m_Content );
    s_RestoreObjectPicture = true;
    if( s_GlobalInvalid )
      {
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pIndex->m_Content );
      return;
      }
    if( Compare( exi, exc ) )
      {
      Nothing_coinside = false;
      while( !pIndex->m_pNext.isNull() && pIndex->m_pNext->m_Kind == tXDnewline )
        pIndex = pIndex->m_pNext;
      if( pIndex->m_pNext.isNull() )
        pOutWin->AddComm( X_Str( "MAnswMistake", "Wrong!" ) );
      else
        pOutWin->AddPrompt( pIndex->m_pNext );
      }
    };

  Nothing_coinside = true;
  bool Twice = false;
  QString Txt_compare;
  //   {Check current answer, || it coincides with ANY Result in ANY Step	}
  QByteArray CurrName;
  QByteArray TextFromGraph;
  bool MainComparison = true;
  bool NewTrueAnswerFound = false;
  PStepMemb pSindex = m_pStepsL->m_pFirst;
  for( ; !pSindex.isNull(); pSindex = pSindex->m_pNext )
    for( pIndex = pSindex->m_pResultE->m_pFirst; !pIndex.isNull() && !NewTrueAnswerFound; pIndex = pIndex->m_pNext )
      {
      if( pIndex->m_Kind != tXDexpress || pIndex->m_CompareType == AsConstraint && pIndex->m_Already ) continue;
      exc = Parser::StrToExpr( pIndex->m_Content );
      if( s_GlobalInvalid )
        {
        s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pIndex->m_Content );
        return "";
        }
      MathExpr exComp = exc;
      if( ( pIndex->m_Content.indexOf( "Table" ) > -1 || pIndex->m_Content.indexOf( "SolutionSystNonEq" ) > -1 ) && IsType( TLexp, exc ) )
        for( PExMemb pIndexL = CastPtr( TLexp, exc )->First(); !pIndexL.isNull(); pIndexL = pIndexL->m_pNext )
          if( pIndexL->m_Memb.WriteE() != "" && !( IsType( TTable, pIndexL->m_Memb ) ) )
            {
            exComp = pIndexL->m_Memb;
            break;
            }      
      if(Compare( exi, exComp ) )
        {
        Nothing_coinside = false;
        pIndex->m_Already = true;
        NewTrueAnswerFound = !pSindex->m_pMethodL->m_Already; //MGR 040222
        if( pSindex->m_pMethodL->m_Already ) Twice = true;
        pSindex->m_pMethodL->m_Already = true;
        if( pSindex->m_pNext.isNull() && s_XPMonitorMode
#ifdef PLUGIN 
          || ( bPluginSeriesMode && ( PluginExam.Mode in[pgmLearn, pgmTrain] ) )
#endif                
          )
          m_Exiting = true;
        if( !pIndex->m_pNext.isNull() && pIndex->m_pNext->m_Kind == tXDtext )
          pOutWin->AddComm( ToLang(pIndex->m_pNext->m_Content) );
        pOutWin->AddPrompt( pSindex->m_pComm->m_pFirst );
        CurrName = pSindex->m_Name;
        if( pSindex->m_pComm->m_pFirst.isNull() )
          pOutWin->AddComm( ToLang(CurrName ) );
        }
      else
        if( MainComparison )
          if( Panel::sm_pGraphEdit != nullptr )
            TextFromGraph = Panel::sm_pGraphEdit->ComparisonResult();
      MainComparison = false;
      }

  //    {Check,  if(ALL Steps are decided} {Always}
  bool All_the_task = true;
  m_Something_skipped = false;
  for( pSindex = m_pStepsL->m_pFirst; !pSindex.isNull() && !m_Something_skipped && !pSindex->m_Name.isEmpty(); pSindex = pSindex->m_pNext )
    if( !pSindex->m_pMethodL->m_Already )
      {
      if( All_the_task ) SetCurr( pSindex );
      All_the_task = false;
      }
    else
      if( !All_the_task )
        m_Something_skipped = true;

  if( All_the_task )
    {
    if( m_WasConstraint ) Panel::sm_pGraphEdit->SetConstraints( nullptr );
    m_WasConstraint = false;
    }

  for( pSindex = m_pStepsL->m_pFirst; !pSindex.isNull() && Nothing_coinside && m_WorkMode == wrkTrain; pSindex = pSindex->m_pNext )
    {
    //      (* LOOK_FALSE has to work only in TIRGUL mode.*)
    //      (* Swich it out in the CALCULATOR program     *)
    pIndex = pSindex->m_pF1->m_pFirst;
    LOOK_FALSE();
    pIndex = pSindex->m_pF2->m_pFirst;
    LOOK_FALSE();
    pIndex = pSindex->m_pF3->m_pFirst;
    LOOK_FALSE();
    }

  if( Twice && !NewTrueAnswerFound ) pOutWin->AddComm( X_Str( "MAnswRepeat", "Repeats existing step!" ) );
  if( All_the_task && !m_pCurr.isNull() )
    {
    WinTesting::sm_pOutWindow->AddPrompt( m_pComment->m_pFirst );
    if( XPInEdit::sm_Language == lngHebrew )
      Txt_compare = X_Str( "MAnswCorrect", "Correct!" ) + Txt_compare;
    TXPStatus::sm_ShowErrMessage = false;
    s_XPStatus.SetMessage( X_Str( "MStatSolved", "Task solved." ) );
    TXPStatus::sm_ShowErrMessage = true;
    SetCurr( PStepMemb() );
    }

  if( Nothing_coinside )
    if( IsConstType( TEqualBase, exi ) )
      pOutWin->AddComm( TextFromGraph );
    else
      pOutWin->AddComm( X_Str( "MWhatThis", "(?)" ));
  return Txt_compare;
  }

void TXPTask::AssignVar( const MathExpr& exi, OutWindow* pOutWin, const QByteArray& event )
  {
  if( exi.IsEmpty() ) return;
  MathExpr op1, op2, op3;
  QByteArray Name;
  if( exi.Binar( '=', op1, op2 ) && op1.Variab( Name ) )
    {
    if( !ExpStore::sm_pExpStore->Known_var( Name ) )
      {
      if( event == "DEFINITION" )
        {
        pOutWin->AddComm( X_Str( "MVarDefined", "Variable defined" ) );
        ExpStore::sm_pExpStore->Store_var( Name, op2 );
        CalcRead();
        }
      }
    else
      {
      pOutWin->AddComm( X_Str( "MVarReDefine", "Variable already defined!" ) );
      ExpStore::sm_pExpStore->Known_var_expr( Name, op2 );
      pOutWin->AddExp( new TBinar( '=', new TVariable( false, Name ), op2 ) );
      }
    return;
    }
  pOutWin->AddComm( X_Str( "MWhatThis", "(?)" ) );
  }

void TXPTask::Exam( const MathExpr& exi, OutWindow* pOutWin )
  {
  Panel::sm_pEditor->m_pInEdit->Clear();
  Panel::sm_pEditor->RefreshXPE();
  QByteArray FirstCorrect;
  m_NeedPrompt = false;
  bool Ok = false;
  for( PDescrMemb pIndex = m_pCurr->m_pResultE->m_pFirst; !pIndex.isNull() && !Ok; pIndex = pIndex->m_pNext )
    {
    if( pIndex->m_Kind != tXDexpress ) continue;
    QByteArray Correct = pIndex->m_Content;
    if( FirstCorrect.isEmpty() )
      FirstCorrect = Correct;
    MathExpr exc = Parser::StrToExpr( pIndex->m_Content );
    if( s_GlobalInvalid || s_CheckError )
      {
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pIndex->m_Content );
      return;
      }
    Ok = m_ExactCompare && exi.Eq( exc ) || !m_ExactCompare && exi.Equal( exc );
    }
  if( Ok )
    {
    m_SummMarks += m_pCurr->m_Mark;
    if( m_pCurr == m_pStepsL->m_pLast && m_SummMarks == 0 ) m_SummMarks = 1;
    if( m_pCurr->m_pComm->m_pFirst.isNull() )
      pOutWin->AddComm( m_pCurr->m_Name );
    else
      pOutWin->AddPrompt( m_pCurr->m_pComm->m_pFirst );
    }
  else
    {
    pOutWin->AddComm( X_Str( "MAnswMistake2", "Sorry, wrong." ) );
    pOutWin->AddComm( X_Str( "MCrrctAnswIs", "Correct answer is: " ) );
    pOutWin->AddExp( Parser::StrToExpr( FirstCorrect ) );
    }
  m_Exiting = m_pCurr == m_pStepsL->m_pLast;
  m_NeedPrompt = !m_Exiting;
    if( m_Exiting ) return;
  PStepMemb Step = m_pCurr->m_pNext;
  for( ; Step != m_pStepsL->m_pLast && Step->m_Mark == 0; Step = Step->m_pNext );
  SetCurr( Step );
  }

bool TXPTask::ProcessEnter( const QByteArray& InpStr, OutWindow* pOutWin )
  {
  MathExpr exi = Parser::StrToExpr( InpStr );
  if( s_GlobalInvalid || s_CheckError || exi.IsTemplate()) return false;
  if( exi.IsEmpty() )
    {
    pOutWin->AddComm( X_Str( "MEmptyInput", "Empty input!" ) );
    pOutWin->show();
    QApplication::beep();
    return false;
    }
  pOutWin->AddExp( exi );
  return ProcessEnter( exi, pOutWin );
  }

bool TXPTask::ProcessPreliminaryEnter( const QByteArray& InpStr, OutWindow* pOutWin, bool& LastStepSolved )
  {
  MathExpr exi = Parser::StrToExpr( InpStr );
  if( s_GlobalInvalid || s_CheckError ) return false;
  return ComparePreliminaryUserExpr( exi, pOutWin, LastStepSolved ) == "OK";
  }

bool TXPTask::ProcessEnter( const MathExpr& exi, OutWindow* pOutWin )
  {
  if( m_WorkMode == wrkExam  )
    Exam( exi, pOutWin );
  else
    CompareUserExpr( exi, pOutWin );
  pOutWin->show();
  bool Result = m_Exiting;
  if( m_pCurr.isNull() && ( m_WorkMode == wrkLearn || m_WorkMode == wrkTrain ) )
    {
//    if( Panel::sm_pPanel->GetCurrentTab() != Panel::tAlgebraTab )
//      Panel::sm_pPanel->SetTab( Panel::tAlgebraTab );
    if( !m_Exiting ) Panel::sm_pEditor->activateWindow();
    }
  return Result;
  }

bool TXPTask::ProcessDefine( const QByteArray& InpStr, OutWindow* pOutWin )
  {
  MathExpr exi = Parser::StrToExpr( InpStr );
  if( s_GlobalInvalid || s_CheckError ) return false;
  if( exi.IsEmpty() )
    {
    QApplication::beep();
    return false;
    }
  pOutWin->AddExp( exi );
  AssignVar( exi, pOutWin, "DEFINITION" );
  pOutWin->show();
  return true;
  }

bool TXPTask::CheckAndShowExpr( const QByteArray& InpStr, OutWindow* pOutWin )
  {
  MathExpr exi = Parser::StrToExpr( InpStr );
  if( s_GlobalInvalid || s_CheckError ) return false;
  if( exi.IsEmpty() ) return false;
  pOutWin->AddExp( exi );
  pOutWin->show();
  return true;
  }

PPromptDescr TXPTask::StepPrompt()
  {
  if( m_WorkMode == wrkLearn ) return OneStepPrompt();
  return OptStepPrompt();
  }

PPromptDescr TXPTask::OneStepPrompt()
  {
  PPromptDescr Result;
  PStepMemb pViewStep;
  if( m_pHelpWindow == nullptr || m_pHelpWindow->m_pStepWindow == nullptr )
    pViewStep = m_pCurr;
  else
    pViewStep = m_pHelpWindow->m_pStepWindow->m_pStepMemb;

  if( pViewStep.isNull() || pViewStep->m_pMethodL->m_pFirst.isNull() )
    {
    Info_m( X_Str( "MTaskSolved", "Task solved." ) );
    return Result;
    }
  MathExpr exc;
  if( !pViewStep->m_pResultE->m_pFirst.isNull() )
    exc = Parser::StrToExpr( pViewStep->m_pResultE->m_pFirst->m_Content );
  else 
    {
    s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pViewStep->m_Name );
    return Result;
    }
  if( s_GlobalInvalid )
    {
    s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pViewStep->m_pResultE->m_pFirst->m_Content );
    return Result;
    }
  return PPromptDescr( new TXPStepPromptDescr( pViewStep->m_Name, exc ) );
  }

PPromptDescr TXPTask::OptStepPrompt()
  {
  PPromptDescr Result;
  if( m_Something_skipped )
    Info_m( X_Str( "MStepSkipped", "Step(s) is skipped!" ) );
  m_Something_skipped = false;
  QString TaskFileName = m_pFile->fileName();
  TaskFileName = TaskFileName.mid( TaskFileName.lastIndexOf( '/' ) ).toUpper();
  if( ( TaskFileName == "MAIN.TSK" || TaskFileName == "MAIN.HEB" ) && m_pHelpWindow != nullptr && m_pHelpWindow->m_pStepWindow != nullptr )
    SetCurr( m_pHelpWindow->m_pStepWindow->m_pStepMemb );

  if( m_pCurr.isNull() )
    {
    Info_m( X_Str( "MTaskSolved", "Task solved." ) );
    return Result;
    }
  byte rand[4];
  TOptExpr exc;
  qsrand( 1346 );
  rand[0] = Random( 3 );
  while( ( rand[1] = Random( 3 ) ) == rand[0] );
  for( rand[2] = Random( 3 ); rand[2] == rand[0] || rand[2] == rand[1]; rand[2] = Random( 3 ) );
  rand[3] = 6 - ( rand[0] + rand[1] + rand[2] );

  PDescrMemb pIndex;
  for( int i = 0; i < 4; i++ )
    {
    switch( rand[i] )
      {
      case 0:
        pIndex = m_pCurr->m_pResultE->m_pFirst;
        break;
      case 1:
        pIndex = m_pCurr->m_pF1->m_pFirst;
        break;
      case 2:
        pIndex = m_pCurr->m_pF2->m_pFirst;
        break;
      case 3:
        pIndex = m_pCurr->m_pF3->m_pFirst;
      }
    if( pIndex.isNull() ) return Result;
    s_iDogOption = !s_TaskEditorOn;
    exc.push_back( Parser::PureStrToExpr( pIndex->m_Content ) );
    s_iDogOption = 0;
    if( s_GlobalInvalid )
      {
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + pIndex->m_Content );
      return Result;
      }
    }
  QByteArray OptName;
  if( m_WorkMode != wrkExam ) OptName = m_pCurr->m_Name;
  return PPromptDescr( new TXPSOptPromptDescr( OptName, exc ) );
  }

bool TXPTask::HintShow()
  {
  if( !m_pFile->isOpen() )
    {
    Info_m( X_Str( "MUnknownTask", "The unknown task" ) );
    return false;
    }
  if( m_WorkMode == wrkExam )
    {
    Info_m( X_Str( "PluginTasksMess", "Prompt not available in exam mode!" ) );
    return false;
    }
  SelectTrack(true);
  ShowPrompt();
  return true;
  }

void TXPTask::StartExam()
  {
  SelectTrack();
  SetFirstStep();
  ShowPrompt();
  }

bool TXPTask::SelectTrack( bool bIsHelp )
  {
  if( !m_pTrack->m_MultyTrack || ( bIsHelp && ( m_pTrack->m_TrackSelected || s_TaskEditorOn ) ) ) return true;
  int OldSelected = m_pTrack->m_SelectedTrack;
  TrackSelectionWindow TSW( WinTesting::sm_pMainWindow, bIsHelp );
  if( TSW.exec() == QDialog::Rejected ) return false;
  if( m_pTrack->m_SelectedTrack != OldSelected )
    {
    ClearTrackDependent();
    CalcRead();
    LoadTrackDependentFromFile();
    }
  if( sm_LastCreatedTrack < TrackCount() ) sm_LastCreatedTrack++;
  return true;
  }

bool TXPTask::ShowPrompt()
  {
  TXPStatus::sm_ShowErrMessage = true;
  PPromptDescr DPrompt = StepPrompt();
  TXPStatus::sm_ShowErrMessage = false;
  if( DPrompt.isNull() )
    {
    QMessageBox::critical(nullptr, "Task Error", s_XPStatus.GetCurrentMessage());
    return false;
    }
  HideHelps();
  TXPStepPromptDescr *pStepDescr = dynamic_cast< TXPStepPromptDescr* >( DPrompt.data() );
  delete m_pHintWindow;
  delete m_pPromptWindow;
  if( pStepDescr != nullptr )
    {
    m_pHintWindow = new HintWindow( WinTesting::sm_pMainWindow, pStepDescr );
    m_pHintWindow->show();
    return true;
    }
  if( m_WorkMode == wrkExam && m_pCurr->m_ShowParms.m_NoHint ) 
    m_pHintWindow = new HintWindow(WinTesting::sm_pMainWindow, m_pCurr->m_pAnswerPrompt);
  else
    {
    TXPSOptPromptDescr *pOptDescr = dynamic_cast<TXPSOptPromptDescr*>(DPrompt.data());
    if (pOptDescr == nullptr) return false;
    m_pHintWindow = new HintWindow(WinTesting::sm_pMainWindow, pOptDescr);
    if (m_pCurr->m_pAnswerPrompt->GetText() != "")
      m_pPromptWindow = new HintWindow(WinTesting::sm_pMainWindow, m_pCurr->m_pAnswerPrompt);
    }
  QPoint P( WinTesting::sm_pMainWindow->sm_pOutWindow->pos() );
  P.setX( P.x() + 500 );
  if (m_pPromptWindow != nullptr)
    {
    m_pPromptWindow->move(P);
    m_pPromptWindow->show();
    P.setY(P.y() + 150);
    }
  m_pHintWindow->move(P);
  m_pHintWindow->show();
  return true;
  }

bool TXPTask::HelpShow()
  {
  if( m_WorkMode != wrkLearn && m_WorkMode != wrkTrain )
    {
    Info_m( X_Str( "XPStatMess", "Disabled in Test mode!" ) );
    return false;
    }
  if( !s_TaskEditorOn && !m_pFile->isOpen() )
    {
    Info_m( X_Str( "MUnknownTask", "The unknown task" ) );
    return false;
    }
  if( m_pCurr.isNull() && !s_TaskEditorOn )
    {
    Info_m( X_Str( "XPStatMess", "Task solved." ) );
    return false;
    }
  if( !SelectTrack( true ) )return false;
  if( m_pHelpWindow == nullptr) 
    m_pHelpWindow = new HelpTaskWindow(WinTesting::sm_pMainWindow,  FromQBA( m_Name, GetLanguage() ), m_pStepsL, m_pMethodL );
  else
    if( m_pTrack->m_MultyTrack )
      {
      delete m_pHelpWindow;
      m_pHelpWindow = new HelpTaskWindow( WinTesting::sm_pMainWindow, FromQBA( m_Name, GetLanguage() ), m_pStepsL, m_pMethodL );
      }
  m_pHelpWindow->show();
  return true;
  }

void TXPTask::OutTaskPrompt( OutWindow* pOutWin )
  {
  if( m_pPrompt->m_pFirst.isNull() ) return;
  pOutWin->AddPrompt( m_pPrompt->m_pFirst );
  pOutWin->show();
  }

void TXPTask::EntBtnClick()
  {
  QByteArray s1, s2, strNTracks, strIndTrack, strNSteps;
  int NumOfAppropriateTracks;
  int iAppropriateTrack;
  bool bLastStepSolved;
  QByteArray s;
  if( m_WorkMode == wrkExam && QMessageBox::question( nullptr, X_Str("MYourAnswer", "Answer"), 
    X_Str("ConfirmAnswer","You acknowledge this answer?" ) ) != QMessageBox::Yes )  return;
  TXPStatus::sm_ShowErrMessage = true;
  QByteArray Formula( Panel::sm_pEditor->Write() );
  if( m_pTrack->m_MultyTrack && !m_pTrack->m_TrackSelected && m_WorkMode != wrkExam )
    {
    NumOfAppropriateTracks = 0;
    if( !Formula.isEmpty() )
      {
      iAppropriateTrack = -1;
      for( int iTryTrack = 1; iTryTrack <= m_pTrack->m_NameOfTrack.count(); iTryTrack++ )
        {
        if( m_pTrack->m_SelectedTrack != iTryTrack )
          {
          m_pTrack->m_SelectedTrack = iTryTrack;
          ClearTrackDependent();
          CalcRead();
          LoadTrackDependentFromFile();
          }
        if( ProcessPreliminaryEnter( Formula, WinTesting::sm_pOutWindow, bLastStepSolved ) )
          {
          iAppropriateTrack = iTryTrack;
          NumOfAppropriateTracks++;
          }
        }
      if( NumOfAppropriateTracks == 1 )
        {
        m_pTrack->m_TrackSelected = true;
        m_pTrack->m_SelectedTrack = iAppropriateTrack;
        ClearTrackDependent();
        CalcRead();
        LoadTrackDependentFromFile();
        }
      }
    if( bLastStepSolved )
      m_pTrack->m_TrackSelected = true;
    if( !m_pTrack->m_TrackSelected )
      if( NumOfAppropriateTracks > 1 )
        SelectTrack();
      else
        {
        m_pTrack->m_SelectedTrack = 1;
        ClearTrackDependent();
        CalcRead();
        LoadTrackDependentFromFile();
        }
    }
  HideHelps();
  if( ProcessEnter( Formula, WinTesting::sm_pOutWindow ) ) WinTesting::sm_pMainWindow->ReturnResult();
  if( !s_GlobalInvalid && !s_CheckError && ( m_pCurr == nullptr || m_pCurr->m_pAnswerTemplate->m_pFirst == nullptr ) )
    if( Panel::sm_pGraphEdit->IsMathGraph() )
      Panel::sm_pGraphEdit->Reset();
    else
      {
      Panel::sm_pEditor->Clear();
      Panel::sm_pEditor->RefreshXPE();
      }
//  if( s_WasDefinition )
//    WinTesting::sm_pQuestWindow->RefreshQuest();
//  s_WasDefinition = false;

  if( m_NeedPrompt )
    ShowPrompt();
  else
    if( !m_WasConstraint )
      {
      Panel::sm_pEditor->activateWindow();
      Panel::sm_pEditor->setFocus();
      }
  TXPStatus::sm_ShowErrMessage = false;
  }
