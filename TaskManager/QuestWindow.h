#ifndef QUEST_WINDOW
#define QUEST_WINDOW

#include <qtextedit.h>
#include <qlayout.h>
#include "TaskTools.h"
#include "../TaskFileManager/Globa.h"
#include <qdockwidget.h>

class QuestWindow : public QWidget
  {
  Q_OBJECT
  friend class TXRestrList;
  friend class WinTesting;
  RichTextWindow *m_pRTWindow;
  int m_ContentsWidth;
  void SetContentsWidth();
  public:
    static bool sm_ShowUnarm;
    static bool sm_ShowMinusByAdd;
    static bool sm_ShowRad;
    QuestWindow( QWidget *pParent );
    public slots:
      void SetMenuEnable( bool Enable ) {}
      void SetContent( PDescrList List );
      void Clear() { m_pRTWindow->clear(); }
  };

#endif
