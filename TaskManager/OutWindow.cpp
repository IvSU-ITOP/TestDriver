#include "OutWindow.h"
#include "Wintesting.h"
#include "../Mathematics/Parser.h"

OutWindow::OutWindow( CentralWindow *pParent ) : m_NewLineNeed( false ), m_pTable( nullptr ), m_pParent( pParent )
  { 
  setStyleSheet( "QWidget {background:#dceff5}" );
  m_ViewSettings.m_BkgrColor = "#dceff5";
  m_ViewSettings.m_TaskCmFont.setFamily( "Arial" );
  m_ViewSettings.m_TaskCmFont.setPointSize( 12 );
  m_ViewSettings.m_SimpCmFont.setFamily( "Arial" );
  m_ViewSettings.m_SimpCmFont.setPointSize( 12 );
  m_TextFormat.setFont( m_ViewSettings.m_TaskCmFont );
  m_TextFormat.setVerticalAlignment( QTextCharFormat::AlignMiddle );
  setReadOnly( true );
  Clear();
  connect(this, SIGNAL( textChanged() ), SLOT( scrollToBottom() ) );
  }

void OutWindow::SetTitle( const QString& Title )
  {
  m_pParent->SetTitle( Title );
  }

void OutWindow::Clear()
  {
  clear();
  QTextCursor MainCursor( textCursor() );
  MainCursor.movePosition( QTextCursor::Start );
  m_pTable = MainCursor.insertTable( 1, 2 );
  QVector<QTextLength> Constraints;
  Constraints.push_back( QTextLength( QTextLength::VariableLength, 50 ) );
  Constraints.push_back( QTextLength( QTextLength::VariableLength, 50 ) );
  QTextTableFormat TableFormat;
  TableFormat.setColumnWidthConstraints( Constraints );
  TableFormat.setBorder( 0 );
  TableFormat.setCellPadding( 5.0 );
  m_pTable->setFormat( TableFormat );
  m_Cursor = m_pTable->cellAt( 0, 1 ).firstCursorPosition();
  m_pTable->cellAt( 0, 1 ).setFormat( m_TextFormat );
  m_ExprCursor = m_pTable->cellAt( 0, 0 ).firstCursorPosition();
  m_NewLineNeed = false;
  }

void OutWindow::AddComm( const QString& Comm )
  {
  if( s_CalcOnly ) return;
  m_TextFormat.setForeground( QColor( Comm.indexOf( X_Str( "MWhatThis", "(?)" ) ) != -1 ? "red" : "blue" ) );
  m_Cursor.insertText( "    " + Comm, m_TextFormat );
  m_Cursor.insertBlock();
  m_NewLineNeed = true;
  }

void OutWindow::NewRow()
  {
  m_pTable->appendRows( 1 );
  m_Cursor = m_pTable->cellAt( m_pTable->rows() - 1, 1 ).firstCursorPosition();
  m_ExprCursor = m_pTable->cellAt( m_pTable->rows() - 1, 0 ).firstCursorPosition();
  m_pTable->cellAt( m_pTable->rows() - 1, 1 ).setFormat( m_TextFormat );
  }

void OutWindow::AddExp( const QByteArray& Formula )
  {
  if( m_NewLineNeed ) NewRow();
  m_NewLineNeed = true;
  BaseTask::sm_pEditSets->m_BkgrColor = m_ViewSettings.m_BkgrColor;
  XPInEdit InEd( Formula, *BaseTask::sm_pEditSets, m_ViewSettings );
  static int NumTmp = 0;
  QString Path( QDir::tempPath() + '/' + "OutTmp" + QString::number( NumTmp++ ) + ".jpg" );
  QImage *pImage = InEd.GetImage();
  if (Formula.indexOf("comment{") != -1)
    {
    int Open = Formula.indexOf('{');
    int Close = Formula.indexOf('}', Open);
    pImage->setText("F1", Formula.left(Open) + ToLang(Formula.mid(Open, Close - Open)) + Formula.mid(Close) );
    }
  else
    pImage->setText( "F1", Formula.data() );
  pImage->save( Path );
  QTextImageFormat ExFormat;
  ExFormat.setName( Path );
  ExFormat.setVerticalAlignment( QTextCharFormat::AlignMiddle );
  m_ExprCursor.insertImage( ExFormat );
  }

void OutWindow::AddExp( const MathExpr& Expr) 
  {
  TMult::sm_ShowMultSign = BaseTask::sm_GlobalShowMultSign;
  TMult::sm_ShowUnarMinus = BaseTask::sm_GlobalShowUnarMinus;
  TSumm::sm_ShowMinusByAddition = BaseTask::sm_GlobalShowMinusByAddition;
  TExpr::sm_TrigonomSystem = BaseTask::sm_GlobalShowRad ? TExpr::tsRad : TExpr::tsDeg;
  AddExp( Expr.SWrite() );
  }

void OutWindow::AddPrompt( PDescrMemb D ) 
  {
  while( !D.isNull() )
    {
    switch( D->m_Kind )
      {
      case tXDexpress:
        {
        MathExpr Expr( Parser::StrToExpr( D->m_Content ) );
        if( s_GlobalInvalid || Expr.IsEmpty() )
          Info_m( X_Str( "XPStatMess", "Task corrupted " ) + D->m_Content );
        else
          AddExp( Expr );
        }
        break;
      case tXDnewline:
//        m_Cursor.insertBlock();
        break;
      case tXDtext:
        AddComm( ToLang(D->m_Content) );
      }
    D = D->m_pNext;
    }
  }

void OutWindow::mouseDoubleClickEvent( QMouseEvent *e )
  {
  QTextEdit::mouseDoubleClickEvent( e );
  QTextCursor Cursor( textCursor() );
  if( !Cursor.atEnd() )
    {
    QTextFormat format = Cursor.charFormat();
    if( format.isImageFormat() )
      {
      QString Name = format.toImageFormat().name();
      QImage Image( Name );
      if( !Image.isNull() )
        {
        QByteArray Formula = FromLang(Image.text("F1"));
        if( !Formula.isEmpty() )
          {
          Panel::sm_pEditor->Clear();
          Panel::sm_pEditor->RestoreFormula( Formula );
          }
        }
      }
    }
  }

void OutWindow::scrollToBottom()
  {
  QScrollBar *pBar = verticalScrollBar();
  pBar->setValue( pBar->maximum() );
  }
