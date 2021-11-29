#ifndef MATH_OUTWIN
#define MATH_OUTWIN
#include "../Mathematics/ResultReceiver.h"
#include "TaskTools.h"
#include <qtextedit.h>
#include "../TaskFileManager/Globa.h"

class OutWindow : public QTextEdit, public ResultReceiver
  {
  Q_OBJECT
  QTextCursor m_Cursor;
  QTextCursor m_ExprCursor;
  QTextCharFormat m_TextFormat;
  ViewSettings m_ViewSettings;
  QTextTable *m_pTable;
  class CentralWindow *m_pParent;
  bool m_NewLineNeed;
  void NewRow();
  public:
    OutWindow( CentralWindow *pParent );
    virtual void Clear();
    virtual void AddComm( const QString& );
    virtual void AddExp( const MathExpr& );
    virtual void AddExp( const QByteArray& );
    void AddPrompt( PDescrMemb );
    void SetTitle( const QString& Title );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    public slots:
    void scrollToBottom();
  };

#endif
