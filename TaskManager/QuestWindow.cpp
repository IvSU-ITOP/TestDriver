#include "QuestWindow.h"
#include <qlayout.h>
#include <qlabel.h>
#include "WinTesting.h"
#include "../Mathematics/Parser.h"
#include "../CommonWidgets/XPGedit.h"

bool QuestWindow::sm_ShowUnarm = true;
bool QuestWindow::sm_ShowRad = false;
bool QuestWindow::sm_ShowMinusByAdd = false;

QuestWindow::QuestWindow(QWidget *pParent) : QWidget(pParent), m_ContentsWidth(0)
  {
  setWindowFlags( Qt::FramelessWindowHint );
  WinTesting::sm_pQuestWindow = this;
  QVBoxLayout *pLayout = new QVBoxLayout;
  pLayout->addWidget( WinTesting::sm_pMenuBar );
  pLayout->addWidget( WinTesting::sm_pToolBar );
  pLayout->addWidget( m_pRTWindow = new RichTextWindow( this ) );
  setLayout( pLayout );
  pLayout->setMargin( 0 );
  }

void QuestWindow::SetContentsWidth()
  {
  if(m_ContentsWidth == 0) m_ContentsWidth = contentsRect().width();
  }

void QuestWindow::SetContent( PDescrList List ) 
  {
  bool ShowUnarm = BaseTask::sm_GlobalShowUnarMinus;
  bool ShowMinusByAdd = BaseTask::sm_GlobalShowMinusByAddition;
  bool ShowRad = BaseTask::sm_GlobalShowRad;
  BaseTask::sm_GlobalShowUnarMinus = sm_ShowUnarm;
  BaseTask::sm_GlobalShowMinusByAddition = sm_ShowMinusByAdd;
  BaseTask::sm_GlobalShowRad = sm_ShowRad;
  m_pRTWindow->SetFixedWidth( m_ContentsWidth - m_pRTWindow->verticalScrollBar()->width() );
  m_pRTWindow->SetContent( List );
  BaseTask::sm_GlobalShowUnarMinus = ShowUnarm;
  BaseTask::sm_GlobalShowMinusByAddition = ShowMinusByAdd;
  BaseTask::sm_GlobalShowRad = ShowRad;
  }

