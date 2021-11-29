#include "TaskWork.h"
#include "../TaskFileManager/Globa.h"
#include "../FormulaPainter/Messtrs.h"
#include "../CommonWidgets/XPGedit.h"
#include "TaskTools.h"
#include "QuestWindow.h"
#include "WinTesting.h"
extern TXPTask s_Task;

HelpButton::HelpButton( HelpButtonsWindow *pParent, PStepMemb pStepMemb ) :
 QLabel( "", pParent ), m_pStepMemb( pStepMemb )
  {
  setLayoutDirection( Qt::LeftToRight );
  setStyleSheet( "QLabel {background:silver;font-size:14pt;border-width:5px;border-color:darkgray;border-style:solid;border-radius:10px}" );
  setAlignment( Qt::AlignHCenter );
  connect( this, SIGNAL( clicked() ), pParent->parent(), SLOT( ShowStep() ) );
  QString Text = FromQBA( pStepMemb->m_Name, pStepMemb->m_pMethodL->GetTask().GetLanguage() );
  if( pStepMemb->m_pMethodL->m_Already )
    Text = QChar( 0x2660 ) + Text.trimmed();
  setText( Text );
  adjustSize();
  if( s_TaskEditorOn )
    {
    QHBoxLayout *pLayout = new QHBoxLayout;
    m_pEdit = new BoundedButton( this, ":/Resources/page_edit.png", "Edit step" );
    connect( m_pEdit, SIGNAL( clicked() ), pParent, SLOT( StartEditStep() ) );
    pLayout->addWidget( m_pEdit );
    m_pDelete = new BoundedButton( this, ":/Resources/page_delete.png", "Delete step" );
    connect( m_pDelete, SIGNAL( clicked() ), pParent, SLOT( DeleteStep() ) );
    pLayout->addWidget( m_pDelete );
    m_pAddBefore = new BoundedButton( this, ":/Resources/bd_insrow.png", "Insert before" );
    connect( m_pAddBefore, SIGNAL( clicked() ), pParent, SLOT( AddBefore() ) );
    pLayout->addWidget( m_pAddBefore );
    pLayout->addWidget( this );
    pParent->m_pLayout->addLayout( pLayout );
    }
  else
    pParent->m_pLayout->addWidget( this );
  WinTesting::sm_TranslateObjects.AddObject( this );
  }

void HelpButton::mousePressEvent( QMouseEvent *e )
  {
  if( e->button() == Qt::LeftButton || !s_Task.sm_EditTask )
    {
    emit clicked();
    return;
    }
  QString NewText = QInputDialog::getText( nullptr, "Help Step Edition", "Type Step Name", QLineEdit::Normal, text() );
  if( NewText.isEmpty() || text() == NewText ) return;
  setText( NewText );
  m_pStepMemb->m_Name = EdStr::sm_pCodec->fromUnicode( NewText );
  WinTesting::SaveEnable( this );
  dynamic_cast< HelpButtonsWindow* >( parent() )->adjustSize();
  }

int HelpButton::GetPercent()
  {
  QString Text = text();
  int StartPercent = Text.lastIndexOf("%)");
  if( StartPercent == -1 ) return 0;
  int iStart = Text.lastIndexOf('(', StartPercent);
  if(iStart == -1) return 0;
  return Text.mid(iStart + 1, StartPercent - iStart - 1 ).toInt();
  }

HelpButtonsWindow::HelpButtonsWindow( HelpTaskWindow *pParent, PStepList pStepList ) : QGroupBox( pParent ), m_pLayout( new QVBoxLayout ),
m_pStepEdit( nullptr ), m_pTaskWindow( pParent )
  {
  setStyleSheet( "QPushButton {font-size:12pt;}" );
  QVector <HelpButton*> Buttons;
  int MaxWidth = 0;
  int SummPercent = 0;
  for( PStepMemb pSindex = pStepList->m_pFirst; !pSindex.isNull(); pSindex = pSindex->m_pNext )
    {
    if( pSindex->m_pMethodL->m_pFirst == nullptr )  continue;
    pSindex->m_pRichText = nullptr;
    pSindex->m_pPromptText = nullptr;
    HelpButton *pButton = new HelpButton( this, pSindex );
    SummPercent += pButton->GetPercent();
    Buttons.append(pButton);
    MaxWidth = max( MaxWidth, Buttons.constLast()->width() );
    }
  m_TrueSummPercent = SummPercent == 100;
  for( auto pButton = Buttons.begin(); pButton != Buttons.end(); pButton++ ) ( *pButton )->setFixedWidth( MaxWidth );
  if( s_TaskEditorOn )
    {
    QHBoxLayout *pLayout = new QHBoxLayout;
    QPushButton *pButton = new QPushButton( "Refresh" );
    connect( pButton, SIGNAL( clicked() ), pParent, SLOT( RefreshButtons() ) );
    pLayout->addWidget( pButton );
    pButton = new QPushButton( "AddStep" );
    connect( pButton, SIGNAL( clicked() ), SLOT( AddStep() ) );
    pLayout->addWidget( pButton );
    m_pLayout->addLayout( pLayout );
    }
  setLayout( m_pLayout );
  adjustSize();
  }

int HelpButtonsWindow::GetMinHeight()
  {
  int iBtnCount = m_pLayout->count();
  int iHeight = 60 * iBtnCount + m_pLayout->margin() * ( iBtnCount + 2 );
  return iHeight;
  }

void	HelpButtonsWindow::resizeEvent( QResizeEvent *pE )
  {
//  QScrollArea::resizeEvent( pE );
  /*
  int iBtnCount = m_pLayout->count();
  if( iBtnCount == 0 ) return;
  int iBthHeight = max( 30, min( 60, ( height() - m_pLayout->margin() * ( iBtnCount + 1 ) ) / iBtnCount) );
  for( int iButton = 0; iButton < iBtnCount; iButton++ )
    m_pLayout->itemAt( iButton )->widget()->setFixedHeight( iBthHeight );
    */
  }

void HelpButtonsWindow::StartEditStep()
  {
  if( m_pStepEdit != nullptr ) return;
  WinTesting::sm_pMainWindow->ShowExpression();
  BoundedButton *pButton = dynamic_cast< BoundedButton* >( sender() );
  m_pStepEdit = new StepEdit( pButton->m_pOwner );
  m_pStepEdit->show();
  }

void HelpButtonsWindow::StopEditStep()
  {
  m_pStepEdit->accept();
  delete m_pStepEdit;
  m_pStepEdit = nullptr;
  m_pTaskWindow->RefreshButtons();
  }

void HelpButtonsWindow::RestartEditStep()
  {
  HelpButton *pButton = m_pStepEdit->m_pButton;
  QRect OldGeometry = m_pStepEdit->geometry();
  m_pStepEdit->hide();
  delete m_pStepEdit;
  m_pStepEdit = new StepEdit( pButton );
  m_pStepEdit->setGeometry( OldGeometry );
  m_pStepEdit->show();
  }

void HelpButtonsWindow::AddBefore()
  {
  BoundedButton *pButton = dynamic_cast< BoundedButton* >( sender() );
  m_pTaskWindow->m_pStepList->AddNewStep( "New step", s_Task, pButton->m_pOwner->m_pStepMemb );
  m_pTaskWindow->RefreshButtons();
  WinTesting::SaveEnable( this );
  }

void HelpButtonsWindow::DeleteStep()
  {
  BoundedButton *pButton = dynamic_cast< BoundedButton* >( sender() );
  PStepMemb pStep = pButton->m_pOwner->m_pStepMemb;
  if( QMessageBox::question( nullptr, "Delete Step", "Do you want to delete step " + ToLang( pStep->m_Name ) + '?' ) != QMessageBox::Yes ) return;
  if( QMessageBox::question( nullptr, "Delete Step", "Are you sure?" ) != QMessageBox::Yes ) return;
  m_pTaskWindow->m_pStepList->DeleteStep( pStep );
  m_pTaskWindow->RefreshButtons();
  WinTesting::SaveEnable( this );
  }

void HelpButtonsWindow::AddStep()
  {
  if(s_Task.IsMultitask())
    m_pTaskWindow->m_pStepList->AddNewStep( "New step (1%)", s_Task );
  else
    m_pTaskWindow->m_pStepList->AddNewStep( "New step", s_Task );
  m_pTaskWindow->RefreshButtons();
  WinTesting::SaveEnable( this );
  }

void HelpPanelWindow::Create( QWidget *pWindow, const QString& Title )
  {
  QVBoxLayout *pLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel(Title);
  pLabel->setStyleSheet("QLabel {font-size:16pt}");
  pLabel->setAlignment(Qt::AlignCenter);
  pLayout->addWidget(pLabel);
  if(pWindow != nullptr) pLayout->addWidget( pWindow );
  setLayout( pLayout );
  }

RichTextWindow::RichTextWindow(QWidget *pParent) : QTextEdit(pParent), m_ContentLoading(false),
  m_OldViewSettings( XPGedit::sm_ViewSettings )
  {
  setStyleSheet( "QWidget {background:#dceff5;font-size:16pt;font-family:arial}" );
  m_ViewSettings.m_BkgrColor = "#dceff5";
  m_ViewSettings.m_TaskCmFont.setFamily( "Arial" );
  m_ViewSettings.m_TaskCmFont.setPointSize( 16 );
  m_ViewSettings.m_SimpCmFont.setFamily( "Arial" );
  m_ViewSettings.m_SimpCmFont.setPointSize( 16 );
  XPGedit::sm_ViewSettings = m_ViewSettings;
  setReadOnly( true );
  setWordWrapMode( QTextOption::NoWrap );
  setDocument( new RichTextDocument() );
  ResetLanguage();
  WinTesting::sm_TranslateObjects.AddObject( this );
  setTextInteractionFlags( textInteractionFlags() & Qt::LinksAccessibleByMouse );
  }

RichTextWindow::~RichTextWindow()
  {
  XPGedit::sm_ViewSettings = m_OldViewSettings;
  }

void RichTextWindow::ResetLanguage()
  {
  dynamic_cast< RichTextDocument* >( document() )->ResetLanguage();
  }

void RichTextWindow::SetFixedWidth( int Width )
  { 
  RichTextDocument *pDocument = dynamic_cast< RichTextDocument* >( document() );
  QMargins M = contentsMargins();
  pDocument->SetFixedWidth( Width - 3 * ( M.left() + M.right() ) );
  }


void RichTextWindow::SetContent( PDescrList Content, bool SetDocument)
  {
  setReadOnly(!TXPTask::sm_EditTask);
  setWordWrapMode(QTextOption::NoWrap);
  setLineWrapMode(QTextEdit::NoWrap);
  setCurrentFont(m_ViewSettings.m_TaskCmFont);
  RichTextDocument *pDocument = Content->m_pDocument;
  if (SetDocument && pDocument != nullptr)
    setDocument(pDocument);
  else
    {
    pDocument = dynamic_cast<RichTextDocument*>(document());
    if (!pDocument->isEmpty())
      {
      disconnect(this, SIGNAL(textChanged()), this, SLOT(ContentChanged()));
      clear();
      }
    m_ContentLoading = true;
    pDocument->SetContent(Content);
    m_ContentLoading = false;
    }
  setMinimumHeight( pDocument->documentLayout()->documentSize().height() );
  if( s_Task.sm_EditTask )
    connect( this, SIGNAL( textChanged() ), SLOT( ContentChanged() ) );
  }

void RichTextWindow::closeEvent( QCloseEvent *event )
  {
  if( s_Task.sm_EditTask )
    disconnect( this, SIGNAL( textChanged() ), this, SLOT( ContentChanged() ) );
  }

void RichTextWindow::ContentChanged()
  {
  if( !m_ContentLoading) WinTesting::SaveEnable( this );
  }

QByteArray RichTextWindow::GetText()
  {
  QTextCursor MainCursor( textCursor() );
  MainCursor.movePosition( QTextCursor::Start );
  QTextTable *pTable = MainCursor.currentTable();
  for( ; pTable == nullptr; pTable = MainCursor.currentTable() )
    MainCursor.movePosition( QTextCursor::NextBlock );
  QByteArray Result;
  bool IsHebrew = s_Task.GetLanguage() == lngHebrew;
  const QTextTableCell &Cell = IsHebrew ? pTable->cellAt( 0, 1 ) : pTable->cellAt( 0, 0 );
  QTextBlock LastBlock;
  if( !IsHebrew ) LastBlock = pTable->cellAt( 0, 1 ).firstCursorPosition().block();
  for( QTextBlock Block = Cell.firstCursorPosition().block(); IsHebrew ? Block.isValid() : Block != LastBlock; Block = Block.next() )
    {
    bool bEndBlock = false;
    for( auto it = Block.begin(); !it.atEnd(); ++it )
      {
      bEndBlock = true;
      QTextFragment CurrentFragment = it.fragment();
      if( CurrentFragment.isValid() )
        {
        if( CurrentFragment.charFormat().isImageFormat() )
          {
          QImage Image( CurrentFragment.charFormat().toImageFormat().name() );
          Result += "EXPR(" + Image.text( "F1" ).toLatin1() + ')';
          }
        else
          Result += '\'' + EdStr::sm_pCodec->fromUnicode( CurrentFragment.text() ) + '\'';
        }
      }
    if( bEndBlock ) Result += "\r\n";
    }
  return Result;
  }

QSize RichTextWindow::GetMinSize()
  {
  document()->adjustSize();
  QSizeF SF = document()->size();
  QMargins M = contentsMargins();
  QSize S( Round( SF.width() ) + verticalScrollBar()->width() + M.left() + M.right(),
    Round( SF.height() ) + horizontalScrollBar()->height() + M.bottom() + M.top() );
  QSize WS = WinTesting::sm_pOutWindow->size();
  if( S.height() > WS.height() ) S.setHeight( WS.height() );
  if( S.width() > WS.width() ) S.setWidth( WS.width() );
  if( minimumHeight() > S.height() ) setMinimumHeight( S.height() );
  return S;
  }

void RichTextWindow::ResetSize()
  {
  setMinimumSize( GetMinSize() );
  }

void RichTextWindow::showEvent( QShowEvent * )
  {
  verticalScrollBar()->setValue( 0 );
  }

void RichTextWindow::contextMenuEvent( QContextMenuEvent *event )
  {
  if( s_TaskEditorOn )
    {
    QMenu *pMenu = createStandardContextMenu();
    pMenu->addAction( "Add Table Row", this, SLOT( AddRow() ) );
    pMenu->addAction( "Delete Table Row", this, SLOT( DeleteRow() ) );
    pMenu->exec( event->globalPos() );
    delete pMenu;
    return;
    }
  QTextEdit::contextMenuEvent( event );
  }

void RichTextWindow::AddRow()
  {
  dynamic_cast< RichTextDocument* >( document() )->AddRow();
  }

void RichTextWindow::DeleteRow()
  {
  dynamic_cast< RichTextDocument* >( document() )->DeleteRow();
  }

void RichTextWindow::dropEvent( QDropEvent *event )
  {
  qDebug() << "RichTextWindow drop Event Start";
  QString Html(event->mimeData()->html());
  if( event->source() == this ) return;
  QString Text = event->mimeData()->html();
  int Img = Text.indexOf( "<img" );
  if( Img == -1 ) return;
  Img = Text.indexOf( "src", Img );
  Img = Text.indexOf( '"', Img ) + 1;
  QString Path = Text.mid( Img, Text.indexOf( '"', Img ) - Img );
  QImage Image( Path );
  if( Image.isNull() ) return;
  QTextEdit::dropEvent(event);
  qDebug() << "RichTextWindow drop Event Success";
  }

HelpTaskWindow::HelpTaskWindow( QWidget *pParent, const QString& Label, PStepList pStepList, PDescrList pDescrList ) :
  QDialog( pParent, Qt::WindowSystemMenuHint ), m_pRichWindow( new RichTextWindow( this ) ),
  m_pButtonsWindow( new HelpButtonsWindow( this, pStepList ) ), m_pStepWindow( nullptr ), m_pStepList( pStepList ),
  m_pBtnOK(new QPushButton( "Close" ) ), m_pInvalidSumm (new QLabel("Percentage sum is invalid! Continue editing") )
  {
  QString Title( X_Str( "LangSwitch_HelpTaskCaption", "General HELP and a List of Steps/Operations" ) );
  if( s_Task.m_pTrack->m_MultyTrack )
    Title += " (Track " + QString::number( s_Task.m_pTrack->m_SelectedTrack ) + ')';
  setWindowTitle( Title );
  setStyleSheet( "QWidget {background:#dceff5} QLineEdit {background:white}" );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel( Label );
  pLabel->setStyleSheet( "QLabel {font-size:14pt}" );
  m_pBtnOK->setStyleSheet("QPushButton {font-size:14pt}");
  pLabel->setAlignment( Qt::AlignCenter );
  pMainLayout->addWidget( pLabel );
  m_pPanelLayout = new QHBoxLayout;
  m_pPanelLayout->addWidget( new HelpPanelWindow( this, m_pRichWindow, X_Str( "CMethod", "General Description" ) ) );
  QScrollArea *pPanelArea = new QScrollArea;
  pPanelArea->setWidget( m_pButtonsWindow );
  pPanelArea->setAlignment( Qt::AlignCenter );
  m_pButtonPanel = new HelpPanelWindow( this, pPanelArea, X_Str( "CSteps", "Steps / Operations" ) );
  m_pPanelLayout->addWidget( m_pButtonPanel );
  pMainLayout->addLayout( m_pPanelLayout );
  if( s_TaskEditorOn )
    { 
    m_pBtnOK->setText("Press when Finish editing");
    pMainLayout->addWidget( m_pInvalidSumm );
    m_pInvalidSumm->setStyleSheet("QLabel {color:red;font-size:12pt}");
    m_pInvalidSumm->setVisible(false);
    QHBoxLayout *pLayout = new QHBoxLayout;
    pLabel = new QLabel( "Final comment" );
    pLabel->setStyleSheet( "QLabel {font-size:12pt}" );
    pLayout->addWidget( pLabel );
    pLayout->addWidget( new CommentEd( s_Task.GetCommentPtr() ) );
    pMainLayout->addLayout( pLayout );
    }
  connect( m_pBtnOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pMainLayout->addWidget( m_pBtnOK );
  setLayout( pMainLayout );
  int WinWidth = WinTesting::sm_pMainWindow->width();
  int WihHeight = WinTesting::sm_pMainWindow->height();
  pLabel->adjustSize();
  m_pRichWindow->SetContent( pDescrList );
  int ButtonHeight = m_pButtonsWindow->GetMinHeight();
  if( pStepList->m_pFirst.isNull() )
    {
    m_pButtonsWindow->AddStep();
    ButtonHeight *= 2;
    }
  int Height = min( ( int ) ( WihHeight * 0.8 ),
    max( m_pRichWindow->GetMinSize().height(), ButtonHeight ) + pLabel->height() * 2 + pMainLayout->margin() * 3 );
  if( ButtonHeight > Height ) m_pButtonsWindow->setFixedHeight( ButtonHeight );
  if( s_TaskEditorOn )
    {
    int iLeft = WinWidth - WinWidth * 0.75;
    QRect R( iLeft, 20, WinWidth * 0.75, Height );
    setGeometry( R );
    }
  else
    {
    int iLeft = WinTesting::sm_pPanel->width();
    QRect R( iLeft, ( WihHeight - Height ) / 2, ( WinWidth - iLeft ) * 0.8, Height );
    setGeometry( R );
    }
  WinTesting::sm_TranslateObjects.AddObject( this );
  }

void HelpTaskWindow::keyPressEvent( QKeyEvent *pE )
  {
  if( pE->key() == Qt::Key_Escape ) close();
  }

void HelpTaskWindow::closeEvent( QCloseEvent *event )
  {
  if( !WinTesting::Translation() && (!TXPTask::sm_EditTask || TXPTask::sm_NewHebFormat ) ) return;
  for( PStepMemb pMemb = m_pStepList->m_pFirst; !pMemb.isNull(); pMemb = pMemb->m_pNext )
    if( pMemb->m_pRichText == nullptr && pMemb->m_pPromptText == nullptr )
      return;
  if( WinTesting::Translation() )
    WinTesting::SaveEnable( this );
  else
    WinTesting::sm_pSaveTaskFile->setEnabled( true );
  }

HelpStepWindow::HelpStepWindow( QWidget *pParent, PStepMemb pStepMemb ) : HelpPanelWindow( pParent ), m_pStepMemb( pStepMemb )
  {
  setWindowFlags( Qt::Window );
  setWindowTitle( X_Str("LangSwitch_HelpStepCaption2", "Help to Operation" ) );
  RichTextWindow *pRW = dynamic_cast<RichTextWindow*>(m_pStepMemb->m_pRichText);
  if( pRW == nullptr )
    {
    m_pStepMemb->m_pRichText = pRW = new RichTextWindow( this );
    pRW->SetContent( pStepMemb->m_pMethodL );
    pRW->ResetSize();
    }
  Create( pRW, FromQBA( pStepMemb->m_Name, pStepMemb->m_pMethodL->GetTask().GetLanguage() ) );
  }

void HelpStepWindow::closeEvent( QCloseEvent *event )
  {
  dynamic_cast< HelpTaskWindow* >( parent() )->m_pStepWindow = nullptr;
  HelpPanelWindow::closeEvent( event );
  }

void HelpTaskWindow::ShowStep()
  {
  static QObject *pPrevSender = nullptr;
  if( pPrevSender != sender() || m_pStepWindow == nullptr )
    {
    delete m_pStepWindow;
    m_pStepWindow = new HelpStepWindow( this, dynamic_cast< HelpButton* >( sender() )->m_pStepMemb );
    pPrevSender = sender();
    }
  m_pStepWindow->show();
  }

void HelpTaskWindow::RefreshButtons()
  {
  m_pPanelLayout->removeWidget( m_pButtonPanel );
  delete m_pButtonPanel;
  QScrollArea *pPanelArea = new QScrollArea;
  m_pButtonsWindow = new HelpButtonsWindow( this, m_pStepList );
  if(s_Task.IsMultitask())
    {
    m_pBtnOK->setEnabled(m_pButtonsWindow->m_TrueSummPercent);
    m_pInvalidSumm->setVisible(!m_pButtonsWindow->m_TrueSummPercent);
    }
  pPanelArea->setWidget( m_pButtonsWindow );
  pPanelArea->setAlignment( Qt::AlignCenter );
  m_pButtonPanel = new HelpPanelWindow( this, pPanelArea, X_Str( "CSteps", "Steps / Operations" ) );
  m_pPanelLayout->addWidget( m_pButtonPanel );
  }

ExprPanel::ExprPanel(QWidget *pParent, MathExpr Exp, const QString& BackColor) : m_Expr(Exp), 
  m_BackColor(BackColor), m_pExpr(nullptr)
  {
  setStyleSheet( "QLabel {background:" + BackColor + '}' );
  CreateContent();
  }

ExprPanel::ExprPanel( QByteArray *pExp, const QString& BackColor ) : m_BackColor( BackColor ), m_pExpr( pExp )
  {
  setStyleSheet( "QLabel {background:" + BackColor + '}' );
  setAcceptDrops( true );
  CreateContent();
  }

void ExprPanel::mouseDoubleClickEvent( QMouseEvent *event )
  {
  if( m_Expr.IsEmpty() ) return;
  Panel::sm_pEditor->Clear(true);
  Panel::sm_pEditor->RestoreFormula( m_Expr.SWrite() );
//  if( s_Task.m_WorkMode == wrkExam || m_Expr.HasStr() )
  if( s_Task.m_WorkMode == wrkExam )
    s_Task.EntBtnClick();
  Panel::sm_pEditor->activateWindow();
  }

void ExprPanel::CreateContent()
  {
  MathExpr Exp = m_Expr;
  int AddHeight = 300;
  if (m_pExpr != nullptr && ( m_pExpr->isEmpty() || *(m_pExpr) == "\"\"" ) )  *m_pExpr = "Answer";
  if( Exp.IsEmpty())
    {
    AddHeight = 0;
    Exp = Parser::StrToExpr( *m_pExpr );
    if( Exp.IsEmpty() || s_GlobalInvalid )
      {
      TXPStatus::sm_ShowErrMessage = true;
      s_XPStatus.SetMessage( X_Str( "MTaskCorrupt", "Task corrupted " ) + *m_pExpr );
      TXPStatus::sm_ShowErrMessage = false;
      return;
      }
    }
  BaseTask::sm_pEditSets->m_BkgrColor = m_BackColor;
  TMult::sm_ShowUnarMinus = BaseTask::sm_GlobalShowUnarMinus;
  TMult::sm_ShowMultSign = BaseTask::sm_GlobalShowMultSign;
  TSumm::sm_ShowMinusByAddition = BaseTask::sm_GlobalShowMinusByAddition;
  int OldAddHeight = XPInEdit::sm_AddHeight;
  XPInEdit::sm_AddHeight = 0;
  if (s_TaskEditorOn && IsType(TExprPict, Exp))
    {
    QByteArray P(Exp.SWrite());
    int iBp = P.indexOf('{');
    Exp = P.mid(iBp + 1, P.length() - iBp - 2);
    }
  XPInEdit InEd( Exp.SWrite(), *BaseTask::sm_pEditSets,
    ViewSettings( QFont( "Arial", 16 ), QFont( "Arial", 16 ), QFont( "Arial", 16 ), "white" ) );
  QPixmap PM(InEd.GetPixmap());
  setPixmap( PM );
  XPInEdit::sm_AddHeight = OldAddHeight;
  setAlignment( Qt::AlignCenter );
  adjustSize();
  QSize S = PM.size();
  setFixedSize( QSize( max( S.width() + 10, 400 ), max( S.height(), AddHeight ) ) );
  }

void ExprPanel::dropEvent( QDropEvent *event )
  {
  qDebug() << "Drop EP Start:" << event->mimeData()->html();
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
  QByteArray Formula = Image.text( "F1" ).toLatin1();
  if( Formula.isEmpty() ) return;
  *m_pExpr = Formula;
  CreateContent();
  WinTesting::SaveEnable( this );
  qDebug() << "Drop EP End";
  }

void ExprPanel::dragEnterEvent( QDragEnterEvent *event )
  {
  qDebug() << "Drag EP Start:" << event->mimeData()->html();
  if( event->mimeData()->hasHtml() ) event->acceptProposedAction();
  qDebug() << "Drag EP End";
  }

bool ExprPanel::CanPaste()
  {
  return QApplication::clipboard()->mimeData() != nullptr || !QApplication::clipboard()->text().isEmpty();
  }

void ExprPanel::mousePressEvent( QMouseEvent* pE)
  {
  if(pE->button() == Qt::RightButton )
    {
    QMenu Menu;
    QAction *pCopyFormula = Menu.addAction("Copy Formula");
    pCopyFormula->setEnabled( m_pExpr != nullptr);
    QAction *pPaste = Menu.addAction("Paste");
    pPaste->setEnabled(CanPaste());
    QAction *pCancel = Menu.addAction("Cancel");
    QAction *pResult = Menu.exec(QCursor::pos());
    if(pResult == pCancel) return;
    if(pResult == pCopyFormula)
      {
      QApplication::clipboard()->setText( Parser::PackUnAscii( *m_pExpr ) );
      return;
      }
    QString SF = QApplication::clipboard()->text();
    if( SF.isEmpty() || SF == "\"\"")
      {
      QClipboard *pClipboard = QApplication::clipboard();
      const QMimeData *pMime = pClipboard->mimeData();
      QImage Image;
      if(pMime != nullptr)
        {
        QString Html = pMime->html();
        if(Html.isEmpty()) return;
        int iImg = Html.indexOf("<img");
        if(iImg == -1) return;
        int iPath = Html.indexOf('"', iImg) + 1;
        QString Path = Html.mid(iPath, Html.indexOf('"', iPath) - iPath );
        Image.load(Path);
        }
      else
        Image = pClipboard->image();
      if( Image.isNull() ) return;
      QByteArray Formula = Image.text( "F1" ).toLatin1();
      if( Formula.isEmpty() ) return;
      *m_pExpr = Formula;
      CreateContent();
      return;
      }
    *m_pExpr = SF.toLocal8Bit();
    CreateContent();
    return;
    }

  if( m_pExpr == nullptr )
    {
    QLabel::mousePressEvent( pE );
    return;
    }
  MathExpr Expr = MathExpr( Parser::StrToExpr( *m_pExpr ) );
  if( s_GlobalInvalid || Expr.IsEmpty() ) return;
  QString Path( RichTextDocument::GetTempPath() );
  QByteArray Formula(Expr.SWrite());
  if(Formula.contains("comment") && Formula[9] == (char) msBaseLang) return;
  XPInEdit InEd( Formula, *BaseTask::sm_pEditSets,
    ViewSettings( QFont( "Arial", 16 ), QFont( "Arial", 16 ), QFont( "Arial", 16 ), "white" ) );
  QImage *pImage = InEd.GetImage();
  pImage->setText( "F1", Parser::PackUnAscii( *m_pExpr ) );
  pImage->save( Path );
  QString Html = "<img src=\"" + Path + "\" style=\"vertical-align:middle;\" />";
  QDrag *pDrag = new QDrag( this );
  pDrag->setPixmap( QPixmap( ":/Resources/Drag.png" ) );
  QMimeData *pMimeData = new QMimeData;
  pMimeData->setHtml( Html );
  pDrag->setMimeData( pMimeData );
  pDrag->exec();
  }

HintWindow::HintWindow( QWidget *pParent, TXPStepPromptDescr *pStepPrompt ) : HelpPanelWindow( pParent )
  {
  setWindowFlags( Qt::Window );
  QString Title( X_Str( "LangSwitch_StepPromptCaption", "Result of Step/Operation" ) );
  if( s_Task.m_pTrack->m_MultyTrack )
    Title += " (Track " + QString::number( s_Task.m_pTrack->m_SelectedTrack ) + ')';
  setWindowTitle( Title + ": " + s_Task.GetCurrstepName());
  ExprPanel *pPanel = new ExprPanel( this, pStepPrompt->m_ResExpr, "#dceff5" );
  Create( pPanel, ToLang(pStepPrompt->Name()) );
  }

HintWindow::HintWindow( QWidget *pParent, TXPSOptPromptDescr *pOptPrompt ) : HelpPanelWindow( pParent )
  {
  setWindowFlags( s_Task.m_WorkMode == wrkExam ? Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint : Qt::Window );
  QString Title( X_Str( "StepPromptSelectResult", "Select Result of Step/Operation" ) );
  if( s_Task.m_pTrack->m_MultyTrack )
    Title += " (Track " + QString::number( s_Task.m_pTrack->m_SelectedTrack ) + ')';
  setWindowTitle( Title + ": " + s_Task.GetCurrstepName() );
  TOptExpr &VExpr = pOptPrompt->m_OptExpr;
  QGridLayout *pGrid = new QGridLayout;
  QVector<ExprPanel*> Panels;
  int MaxWidth = 0, MaxHeight = 0;
  for( int iRow = 0, iExpr = 0; iExpr < VExpr.count(); iRow++ )
    for( int iCol = 0; iCol < 2 && iExpr < VExpr.count(); iCol++ )
      {
      ExprPanel *pPanel = new ExprPanel( this, VExpr[iExpr++], "#dceff5" );
      MaxWidth = max( MaxWidth, pPanel->width() );
      MaxHeight = max( MaxHeight, pPanel->height() );
      Panels.push_back( pPanel );
      }
  QSize S( MaxWidth, MaxHeight );
  for( int iRow = 0, iExpr = 0; iExpr < VExpr.count(); iRow++ )
    for( int iCol = 0; iCol < 2 && iExpr < VExpr.count(); iCol++, iExpr++ )
      {
      Panels[iExpr]->setFixedSize( S );
      pGrid->addWidget( Panels[iExpr], iRow, iCol );
      }
  setLayout( pGrid );
  }

HintWindow::HintWindow(QWidget *pParent, PDescrList AnswerPrompt) : HelpPanelWindow(pParent)
  {
  setWindowFlags(Qt::Window);
  setWindowTitle(X_Str("PromptAnswer", "The Prompt for Answer") + ": " + s_Task.GetCurrstepName());
  RichTextWindow *pRTW = nullptr;
  if ( !AnswerPrompt->m_pFirst.isNull() )
    {
    pRTW = new RichTextWindow(this);
    pRTW->SetContent(AnswerPrompt);
    pRTW->ResetSize();
    }
//  Create(pRTW, X_Str("TypeAnswer", "Create Answer by Formula Editor"));
  }

SelectTrackButton::SelectTrackButton( QWidget *pParent, int TrackNumber, const QString& Text ) : QPushButton( Text, pParent ), m_TrackNumber( TrackNumber )
  {
  setStyleSheet( "QPushButton {background:silver;font-size:14pt;border-width:5px;border-color:darkgray;border-style:solid;border-radius:10px}" );
  connect( this, SIGNAL( clicked() ), pParent, SLOT( accept() ) );
  setFixedHeight( 40 );
  setEnabled( TrackNumber <= BaseTask::sm_LastCreatedTrack );
  }

TrackSelectionWindow::TrackSelectionWindow( QWidget *pParent, bool bIsHelp ) : QDialog( pParent ), m_IsHelp( bIsHelp )
  {
  if( bIsHelp )
    {
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );
    setWindowTitle( X_Str( "XPCtrlPanelCaptions", "General HELP" ) );
    }
  else
    {
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowCloseButtonHint );
    setWindowTitle( X_Str( "TrackSelection", "Track Selection" ) );
   }
  setStyleSheet( "QWidget {background:#dceff5}" );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  RichTextWindow *pRichWindow = new RichTextWindow( this );
  pRichWindow->SetContent( s_Task.m_pTrack->m_TracksDescription );
  pRichWindow->ResetSize();
  pMainLayout->addWidget( pRichWindow );
  for( int iButton = 0; iButton < s_Task.m_pTrack->m_NameOfTrack.count(); iButton++ )
    pMainLayout->addWidget( new SelectTrackButton( this, iButton + 1, ToLang( s_Task.m_pTrack->m_NameOfTrack[iButton] ) ) );
  setLayout( pMainLayout );
  }

void TrackSelectionWindow::keyPressEvent( QKeyEvent *pE )
  {
  if( m_IsHelp && pE->key() == Qt::Key_Escape ) reject();
  }

void TrackSelectionWindow::accept()
  {
  s_Task.m_pTrack->m_TrackSelected = true;
  s_Task.m_pTrack->m_SelectedTrack = dynamic_cast< SelectTrackButton* >( sender() )->m_TrackNumber;
  QDialog::accept();
  }

CommentEd::CommentEd( QByteArray *pHead ) : QLineEdit( ToLanguage( *pHead, s_Task.GetLanguage() ) ),
  m_pHead( pHead )
  {
  setStyleSheet( "QLineEdit {font-size:12pt}" );
  connect( this, SIGNAL( textEdited( const QString& ) ), SLOT( Edit() ) );
  }

void CommentEd::Edit()
  {
  *m_pHead = EdStr::sm_pCodec->fromUnicode( text() );
  WinTesting::SaveEnable( this );
  }

MarkEd::MarkEd( int *pMark ) : QSpinBox(), m_pMark(pMark)
  {
  setMaximum(100);
  setMinimum(1);
  setValue(*pMark);
  connect( this, SIGNAL( valueChanged( int ) ), SLOT( Edit(int) ) );
  }

void MarkEd::Edit(int i)
  {
  *m_pMark = i;
  WinTesting::SaveEnable( this );
  }

TemplEdit::TemplEdit( PStepMemb pStep ) : m_pTemplate( pStep->m_pAnswerTemplate )
  {
  setStyleSheet( "QLineEdit {font-size:12pt}" );
  connect( this, SIGNAL( textEdited( const QString& ) ), SLOT( Edit() ) );
  if( m_pTemplate->m_pFirst == nullptr ) return;
  setText( ToLang( m_pTemplate->m_pFirst->m_Content ) );
  setAcceptDrops( true );
  }

void TemplEdit::mouseDoubleClickEvent( QMouseEvent *event )
  {
  if( text().isEmpty() ) return;
  Panel::sm_pEditor->Clear( true );
  WinTesting::sm_pMainWindow->ShowExpression();
  s_iDogOption = 1;
  QString Formula =  text().replace( "@Table(", "Table(" );
  Formula = Formula.replace("@UnvisibleTable(", "UnvisibleTable(" );
  Formula = Formula.replace("@PartlyVisibleTable(", "PartlyVisibleTable(" );
  MathExpr Expr = MathExpr( Parser::StrToExpr(EdStr::sm_pCodec->fromUnicode( Formula ) ) );
  s_iDogOption = 0;
  if( s_GlobalInvalid || Expr.IsEmpty() ) return;
  Panel::sm_pEditor->RestoreFormula( Expr.SWrite() );
  Panel::sm_pEditor->activateWindow();
  }

void TemplEdit::dropEvent( QDropEvent *event )
  {
  qDebug() << "Drop TE Start:" << event->mimeData()->html();
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
  QString Formula = ToLang( Parser::UnpackUnAscii( Image.text( "F1" ).toLatin1()) );
  if( Formula.isEmpty() ) return;
  setText( Formula );
  Edit();
  WinTesting::SaveEnable( this );
  qDebug() << "Drop TE End";
  }

void TemplEdit::dragEnterEvent( QDragEnterEvent *event )
  {
  qDebug() << "Drag TE Start:" << event->mimeData()->html();
  if( event->mimeData()->hasHtml() ) event->acceptProposedAction();
  qDebug() << "Drag TE End";
  }

void TemplEdit::Edit()
  {
  if( text().isEmpty() )
    {
    if( m_pTemplate->m_pFirst == nullptr ) return;
    m_pTemplate->Delete( m_pTemplate->m_pFirst );
    WinTesting::SaveEnable( this );
    return;
    }
  if( m_pTemplate->m_pFirst == nullptr )
    m_pTemplate->Add( tXDexpress, EdStr::sm_pCodec->fromUnicode( text() ) );
  else
    m_pTemplate->m_pFirst->m_Content = EdStr::sm_pCodec->fromUnicode( text() );
  WinTesting::SaveEnable( this );
  }

StepEdit::StepEdit( HelpButton *pOwnerButton ) : QDialog( ( QWidget* ) pOwnerButton->parent(), Qt::WindowSystemMenuHint ), 
  m_pButton( pOwnerButton ), m_pShowUnarMinus( new QCheckBox ), m_pHideUnarMinus( new QCheckBox ), m_pNoHint(new QCheckBox),
  m_pShowMultSign( new QCheckBox() ), m_pHideMultSign( new QCheckBox ), m_pShowRad( new QCheckBox ), m_pShowDeg( new QCheckBox ),
  m_pOnExactCompare(new QCheckBox()), m_pShowMinusByAddition(new QCheckBox), m_pHideMinusByAddition(new QCheckBox),
  m_pShowPromptEdit(new QPushButton( "Edit Exam Prompt") ), m_pPromptEditor(nullptr), m_pFalseCommentEditor(nullptr),
  m_pBtnEdCommentF1(new QPushButton("Edit comment on False answer 1")),
  m_pBtnEdCommentF2(new QPushButton("Edit comment on False answer 2")),
  m_pBtnEdCommentF3(new QPushButton("Edit comment on False answer 3")),
  m_pHeightEditorWindow(new QDoubleSpinBox )
  {
  QString Title( "Edit task step" );
  if( s_Task.m_pTrack->m_MultyTrack )
    Title += " (Track " + QString::number( s_Task.m_pTrack->m_SelectedTrack ) + ')';
  setWindowTitle( Title );
  setStyleSheet( "QWidget {font-size:12pt;background:#f5f5f5} QLineEdit {background:white} QCheckBox {background:white}" );
  PStepMemb pStep = pOwnerButton->GetStep();
  QVBoxLayout *pVBox = new  QVBoxLayout;
  QHBoxLayout *pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Step name" ) );
  m_pStepName = new CommentEd( &pStep->m_Name );
  pHBox->addWidget( m_pStepName );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Step comment" ) );
  pHBox->addWidget( new CommentEd( pStep->GetCommentPtr() ) );
  pVBox->addLayout( pHBox );
  QGridLayout *pGrid = new  QGridLayout;
  pGrid->addWidget( new QLabel( "Unar minus" ), 0, 0 );
  QLabel *pLabel = new QLabel( "show:" );
  pLabel->setAlignment( Qt::AlignRight );
  pGrid->addWidget( pLabel, 0, 1 );
  m_pShowUnarMinus->setChecked( pStep->m_ShowParms.m_ShowUnarMinus );
  pGrid->addWidget( m_pShowUnarMinus, 0, 2 );
  connect( m_pShowUnarMinus, SIGNAL( stateChanged( int ) ), SLOT( ChangeUnarm( int ) ) );
  pLabel = new QLabel( "hide:" );
  pLabel->setAlignment( Qt::AlignRight );
  pGrid->addWidget( pLabel, 0, 3 );
  m_pHideUnarMinus->setChecked( pStep->m_ShowParms.m_HideUnarMinus );
  pGrid->addWidget( m_pHideUnarMinus, 0, 4 );
  connect( m_pHideUnarMinus, SIGNAL( stateChanged( int ) ), SLOT( ChangeUnarm( int ) ) );
  pGrid->addWidget(new QLabel("Minus by addition"), 1, 0);
  pLabel = new QLabel("show:");
  pLabel->setAlignment(Qt::AlignRight);
  pGrid->addWidget(pLabel, 1, 1);
  m_pShowMinusByAddition->setChecked(pStep->m_ShowParms.m_ShowMinusByAddition);
  pGrid->addWidget(m_pShowMinusByAddition, 1, 2);
  connect(m_pShowMinusByAddition, SIGNAL(stateChanged(int)), SLOT(ChangeMinusByAdd(int)));
  pLabel = new QLabel("hide:");
  pLabel->setAlignment(Qt::AlignRight);
  pGrid->addWidget(pLabel, 1, 3);
  m_pHideMinusByAddition->setChecked(pStep->m_ShowParms.m_HideMinusByAddition);
  pGrid->addWidget(m_pHideMinusByAddition, 1, 4);
  connect(m_pHideMinusByAddition, SIGNAL(stateChanged(int)), SLOT(ChangeMinusByAdd(int)));
  pGrid->addWidget( new QLabel( "Sign of multiplication" ), 2, 0 );
  pLabel = new QLabel( "show:" );
  pLabel->setAlignment( Qt::AlignRight );
  pGrid->addWidget( pLabel, 2, 1 );
  m_pShowMultSign->setChecked( pStep->m_ShowParms.m_ShowMultSign );
  m_pShowMultSign->adjustSize();
  QSize RR( m_pShowMultSign->size() );
  m_pShowMultSign->setFixedSize( RR );
  pGrid->addWidget( m_pShowMultSign, 2, 2 );
  connect( m_pShowMultSign, SIGNAL( stateChanged( int ) ), SLOT( ChangeMult( int ) ) );
  pLabel = new QLabel( "hide:" );
  pLabel->setAlignment( Qt::AlignRight );
  pGrid->addWidget( pLabel, 2, 3 );
  m_pHideMultSign->setFixedSize( RR );
  m_pHideMultSign->setChecked( pStep->m_ShowParms.m_HideMultSign );
  pGrid->addWidget( m_pHideMultSign, 2, 4 );
  connect( m_pHideMultSign, SIGNAL( stateChanged( int ) ), SLOT( ChangeMult( int ) ) );
  pGrid->addWidget( new QLabel( "Angle" ), 3, 0 );
  pLabel = new QLabel( "in radians:" );
  pLabel->setAlignment( Qt::AlignRight );
  pGrid->addWidget( pLabel, 3, 1 );
  m_pShowRad->setChecked( pStep->m_ShowParms.m_ShowRad );
  pGrid->addWidget( m_pShowRad, 3, 2 );
  connect( m_pShowRad, SIGNAL( stateChanged( int ) ), SLOT( ChangeRad( int ) ) );
  pLabel = new QLabel( "in degrees:" );
  pLabel->setAlignment( Qt::AlignRight );
  pGrid->addWidget( pLabel, 3, 3 );
  m_pShowDeg->setChecked( pStep->m_ShowParms.m_ShowDeg );
  pGrid->addWidget( m_pShowDeg, 3, 4 );
  connect( m_pShowDeg, SIGNAL( stateChanged( int ) ), SLOT( ChangeRad( int ) ) );
  pGrid->addWidget(new QLabel("Exact Comparison"), 4, 0);
  pLabel = new QLabel("");
  pGrid->addWidget(pLabel, 4, 1);
  m_pOnExactCompare->setChecked(pStep->m_ShowParms.m_OnExactCompare);
  pGrid->addWidget(m_pOnExactCompare, 4, 2);
  pLabel = new QLabel("");
  pGrid->addWidget(pLabel, 4, 3);
  pLabel = new QLabel("");
  pGrid->addWidget(pLabel, 4, 4);
  pGrid->addWidget(new QLabel("No hints for test"), 5, 0);
  pLabel = new QLabel("");
  pGrid->addWidget(pLabel, 5, 1);
  m_pNoHint->setChecked(pStep->m_ShowParms.m_NoHint);
  pGrid->addWidget(m_pNoHint, 5, 2);
  connect(m_pNoHint, SIGNAL(stateChanged(int)), SLOT(ChangeHint(int)));
  pLabel = new QLabel("");
  pGrid->addWidget(pLabel, 5, 3);
  pLabel = new QLabel("");
  m_pShowPromptEdit->setEnabled(pStep->m_ShowParms.m_NoHint);
  connect(m_pShowPromptEdit, SIGNAL(clicked()), SLOT(PromptEdit()));
  pGrid->addWidget(m_pShowPromptEdit, 5, 4);
  pVBox->addLayout( pGrid );
  if(s_Task.IsMultitask())
    {
    pHBox = new  QHBoxLayout;
    pLabel = new QLabel("Mark of step (%)");
    pLabel->setEnabled( s_Task.IsMultitask() );
    pHBox->addWidget(pLabel);
    m_pMarkEdit = new MarkEd(&pStep->m_Mark);
    m_pMarkEdit->setEnabled( s_Task.IsMultitask() );
    pHBox->addWidget(m_pMarkEdit);
    pVBox->addLayout( pHBox );
    }
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Part of window height for editor" ) );
  m_pHeightEditorWindow->setRange( 0.3, 0.7 );
  m_pHeightEditorWindow->setSingleStep( 0.05 );
  m_pHeightEditorWindow->setDecimals( 2 );
  m_pHeightEditorWindow->setValue(pStep->m_ShowParms.m_HeightEditorWindow);
  pHBox->addWidget( m_pHeightEditorWindow );
  pVBox->addLayout( pHBox );
  QLabel *pListAnswers = new QLabel( "List of true answers" );
  pListAnswers->setAlignment( Qt::AlignCenter );
  pVBox->addWidget( pListAnswers );
  bool AddDelete = pStep->m_pResultE->GetExpressCount() > 1;
  for( PDescrMemb pIndex = pStep->m_pResultE->m_pFirst; !pIndex.isNull(); pIndex = pIndex->m_pNext )
    if( pIndex->m_Kind == tXDexpress )
      {
      pHBox = new  QHBoxLayout;
      if( AddDelete )
        {
        BoundedButton *pButton = new BoundedButton( pIndex, ":/Resources/page_delete.png", "Delete answer" );
        connect( pButton, SIGNAL( clicked() ), SLOT( RemoveAnswer() ) );
        pHBox->addWidget( pButton );
        ExprPanel *pPanel = new ExprPanel(&pIndex->m_Content, "#dceff5");
        pHBox->addWidget( pPanel );
        pHBox->addWidget(new NationalTextButton(pPanel));
        }
      else
        {
        ExprPanel *pPanel = new ExprPanel( &pIndex->m_Content, "#dceff5" );
        pHBox->addWidget( pPanel );
        pHBox->addWidget(new NationalTextButton(pPanel));
        pHBox->setAlignment( Qt::AlignRight );
        }
      pVBox->addLayout( pHBox );
      }
  pListAnswers = new QLabel( "List of false answers" );
  pListAnswers->setAlignment( Qt::AlignCenter );
  pVBox->addWidget( pListAnswers );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "False answer 1" ) );
  ExprPanel *pPanel = new ExprPanel(&pStep->m_pF1->m_pFirst->m_Content, "#dceff5");
  pHBox->addWidget(  pPanel );
  pHBox->addWidget(new NationalTextButton(pPanel));
  pVBox->addLayout( pHBox );
  connect(m_pBtnEdCommentF1, SIGNAL(clicked()), SLOT(FalseCommentEdit()));
  pVBox->addWidget(m_pBtnEdCommentF1);
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "False answer 2" ) );
  pPanel = new ExprPanel(&pStep->m_pF2->m_pFirst->m_Content, "#dceff5");
  pHBox->addWidget(pPanel);
  pHBox->addWidget(new NationalTextButton(pPanel));
  pVBox->addLayout( pHBox );
  connect(m_pBtnEdCommentF2, SIGNAL(clicked()), SLOT(FalseCommentEdit()));
  pVBox->addWidget(m_pBtnEdCommentF2);
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "False answer 3" ) );
  pPanel = new ExprPanel(&pStep->m_pF3->m_pFirst->m_Content, "#dceff5");
  pHBox->addWidget(pPanel);
  pHBox->addWidget(new NationalTextButton(pPanel));
  pVBox->addLayout( pHBox );
  connect(m_pBtnEdCommentF3, SIGNAL(clicked()), SLOT(FalseCommentEdit()));
  pVBox->addWidget(m_pBtnEdCommentF3);
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( "Template" ) );
  TemplEdit *pTemplEdit = new TemplEdit( pStep );
  pHBox->addWidget( pTemplEdit );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  QPushButton *pButton = new QPushButton( "OK");
  connect( pButton, SIGNAL( clicked() ), pOwnerButton->parent(), SLOT( StopEditStep() ) );
  pHBox->addWidget( pButton );
  pButton = new QPushButton( "Add answer" );
  connect( pButton, SIGNAL( clicked() ), SLOT( AddAnswer() ) );
  pHBox->addWidget( pButton );
  pButton = new QPushButton( "Recalc" );
  connect( pButton, SIGNAL( clicked() ), SLOT( Recalc() ) );
  pHBox->addWidget( pButton );
  pButton = new QPushButton( "Refresh" );
  connect( pButton, SIGNAL( clicked() ), pOwnerButton->parent(), SLOT( RestartEditStep() ) );
  pHBox->addWidget( pButton );
  pVBox->addLayout( pHBox );
  QWidget *pWidget = new QWidget( this );
  pWidget->setLayout( pVBox );
  pWidget->adjustSize();
  QSize S = pWidget->size();
  QScrollArea *pArea = new QScrollArea;
  pArea->setWidget( pWidget );
  pArea->setAlignment( Qt::AlignCenter );
  pVBox = new  QVBoxLayout;
  pVBox->addWidget( pArea );
  setLayout( pVBox );
  setFixedWidth( S.width() + 100 );
  setFixedHeight( min( S.height() + 100, WinTesting::sm_pMainWindow->height() ) );
  }

void StepEdit::ChangeUnarm( int State )
  {
  if( State == 0 ) return;
  QObject *pSender = sender();
  if( pSender == m_pShowUnarMinus )
    m_pHideUnarMinus->setChecked( false );
  else
    m_pShowUnarMinus->setChecked( false );
  }

void StepEdit::ChangeMinusByAdd(int State)
  {
  if (State == 0) return;
  QObject *pSender = sender();
  if (pSender == m_pShowMinusByAddition)
    m_pHideMinusByAddition->setChecked(false);
  else
    m_pShowMinusByAddition->setChecked(false);
  }

void StepEdit::ChangeMult( int State )
  {
  if( State == 0 ) return;
  QObject *pSender = sender();
  if( pSender == m_pShowMultSign )
    m_pHideMultSign->setChecked( false );
  else
    m_pShowMultSign->setChecked( false );
  }

void StepEdit::ChangeRad( int State )
  {
  if( State == 0 ) return;
  QObject *pSender = sender();
  if( pSender == m_pShowDeg )
    m_pShowRad->setChecked( false );
  else
    m_pShowDeg->setChecked( false );
  }

void StepEdit::accept()
  {
  PStepMemb pStep = m_pButton->GetStep();
  pStep->m_ShowParms.m_ShowUnarMinus = m_pShowUnarMinus->isChecked();
  pStep->m_ShowParms.m_HideUnarMinus = m_pHideUnarMinus->isChecked();
  pStep->m_ShowParms.m_ShowMinusByAddition = m_pShowMinusByAddition->isChecked();
  pStep->m_ShowParms.m_HideMinusByAddition = m_pHideMinusByAddition->isChecked();
  pStep->m_ShowParms.m_ShowMultSign = m_pShowMultSign->isChecked();
  pStep->m_ShowParms.m_HideMultSign = m_pHideMultSign->isChecked();
  pStep->m_ShowParms.m_ShowRad = m_pShowRad->isChecked();
  pStep->m_ShowParms.m_ShowDeg = m_pShowDeg->isChecked();
  pStep->m_ShowParms.m_OnExactCompare = m_pOnExactCompare->isChecked();
  pStep->m_ShowParms.m_NoHint = m_pNoHint->isChecked();
  pStep->m_ShowParms.m_HeightEditorWindow = m_pHeightEditorWindow->value();
  if( s_Task.IsMultitask() ) pStep->ResetMark(m_pMarkEdit->value());
  WinTesting::SaveEnable( this );
  delete m_pPromptEditor;
  delete m_pFalseCommentEditor;
  QDialog::accept();
  }

void StepEdit::AddAnswer()
  {
  PDescrList AnwerList = m_pButton->GetStep()->m_pResultE;
  AnwerList->Add( tXDexpress, "\"New Answer\"" );
  dynamic_cast< HelpButtonsWindow* > ( m_pButton->parent() )->RestartEditStep();
  }

void StepEdit::RemoveAnswer()
  {
  BoundedButton *pBButton = dynamic_cast< BoundedButton* >( sender() );
  m_pButton->GetStep()->m_pResultE->Delete( pBButton->m_pData );
  dynamic_cast< HelpButtonsWindow* > ( m_pButton->parent() )->RestartEditStep();
  }

void StepEdit::Recalc()
  {
  s_Task.m_pCalc->Calculate();
  dynamic_cast< HelpButtonsWindow* > ( m_pButton->parent() )->RestartEditStep();
  }

void StepEdit::ChangeHint(int Checked)
  {
  m_pShowPromptEdit->setEnabled(Checked);
  }

void StepEdit::PromptEdit() 
  {
  if( m_pPromptEditor == nullptr) m_pPromptEditor = new PromptEditor(this, m_pButton->GetStep());
  m_pPromptEditor->show();
  }

void StepEdit::FalseCommentEdit()
  {
  delete m_pFalseCommentEditor;
  PStepMemb pStep = GetStep();
  QPushButton *pButton = dynamic_cast< QPushButton* >(sender());
  PDescrList pFalse = pStep->m_pFComm1;
  if(pButton == m_pBtnEdCommentF2) pFalse = pStep->m_pFComm2;
  if (pButton == m_pBtnEdCommentF3) pFalse = pStep->m_pFComm3;
  m_pFalseCommentEditor = new FalseCommentEditor (this, pFalse);
  m_pFalseCommentEditor->show();
  }

PromptEditor::PromptEditor(QWidget *pParent, PStepMemb pStep) : QDialog(pParent, Qt::WindowSystemMenuHint)
  {
  setWindowTitle("Edit Prompt for Answer");
  setStyleSheet("QWidget {font-size:12pt;background:#D3D3D3}");
  RichTextWindow *pRW = dynamic_cast<RichTextWindow*>(pStep->m_pPromptText);
  if (pRW == nullptr)
    {
    pStep->m_pPromptText = pRW = new RichTextWindow(this);
    pRW->SetContent(pStep->m_pAnswerPrompt);
    pRW->ResetSize();
    }
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget(pRW);
  QPushButton *pButton = new QPushButton("OK");
  pVBox->addWidget(pButton);
  connect(pButton, SIGNAL(clicked()), SLOT(accept()));
  setLayout(pVBox);
  }

FalseCommentEditor::FalseCommentEditor(QWidget *pParent, PDescrList pFalse) : QDialog(pParent, Qt::WindowSystemMenuHint)
  {
  RichTextWindow *pRTW = new RichTextWindow(this);
  pRTW->SetContent(pFalse, true);
  pRTW->ResetSize();
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget(pRTW);
  QPushButton *pButton = new QPushButton("OK");
  connect(pButton, SIGNAL(clicked()),SLOT(accept()));
  pVBox->addWidget(pButton);
  setLayout(pVBox);
  }

BoundedButton::BoundedButton( HelpButton *pOwner, const QByteArray& Icon, const QByteArray& ToolTip ) : m_pOwner( pOwner )
  {
  setIcon( QIcon( Icon ) );
  setToolTip( ToolTip );
  }

BoundedButton::BoundedButton( PDescrMemb pData, const QByteArray& Icon, const QByteArray& ToolTip ) :
  BoundedButton( nullptr, Icon, ToolTip )
  {
  m_pData = pData;
  }

void NationalTextButton::mouseReleaseEvent(QMouseEvent *e)
  {
  QByteArray Text(*m_pExprPanel->m_pExpr);
  if (Text.length() > 0 && Text[0] == '"')
    {
    Text = Text.mid(1, Text.length() - 2);
    if(!Text.isEmpty() && Text[0] == (char) msBaseLang)
      Text = Text.mid(1);
    }
  else
    Text.clear();
  NationalTextEditor TE(ToLang(Text.replace(msPrime, '"').replace(msDoublePrime, '{' ).replace(msTriplePrime,'}' ).replace(msCharNewLine, '\n')) );
  if (TE.exec() == QDialog::Rejected) return;
  *m_pExprPanel->m_pExpr = TE.GetText();
  m_pExprPanel->CreateContent();
  }

NationalTextEditor::NationalTextEditor(const QString& Text) : QDialog(nullptr, Qt::WindowSystemMenuHint), m_TextEditor(new QTextEdit)
  {
    setStyleSheet("QWidget {font-size:12pt;}");
  setWindowTitle("Edit Answer as Text");
  resize(500, 500);
  if( !Text.isEmpty() ) m_TextEditor->setLineWrapMode(QTextEdit::NoWrap);
  if (s_Task.GetLanguage() == lngHebrew)
    {
    QStringList SList = Text.split('\n');
    QString Result;
    int Rest = SList.count();
    for (auto pRow = SList.begin(); pRow != SList.end(); pRow++, Rest--)
      {
      QString Row(*pRow);
      if ( QString("!:;,?.").indexOf(Row[0]) != -1)
        Result += Row.mid(1) + Row[0];
      else
        Result += Row;
      if (Rest > 1) Result += '\n';
      }
    m_TextEditor->setText(Result);
    }
  else
    m_TextEditor->setText(Text);
  QVBoxLayout *pVBox = new  QVBoxLayout;
  pVBox->addWidget(m_TextEditor);
  QHBoxLayout *pHBox = new  QHBoxLayout;
  QPushButton *pSave = new QPushButton("Save");
  connect(pSave, SIGNAL(clicked()), SLOT(accept()));
  pHBox->addWidget(pSave);
  QPushButton *pCancel = new QPushButton("Cancel");
  connect(pCancel, SIGNAL(clicked()), SLOT(reject()));
  pHBox->addWidget(pCancel);
  pVBox->addLayout(pHBox);
  setLayout(pVBox);
  }

QByteArray NationalTextEditor::GetText() 
  { 
  QByteArray Result(1, '"');
  Result += msBaseLang;
  if (s_Task.GetLanguage() == lngHebrew)
    {
    QString H = m_TextEditor->toHtml();
    QStringList SList = m_TextEditor->toPlainText().split('\n');
    int Rest = SList.count();
    for (auto pRow = SList.begin(); pRow != SList.end(); pRow++, Rest--)
      {
      QByteArray Row(FromLang(*pRow).replace('"', msPrime).replace('{', msDoublePrime).replace('}', msTriplePrime));
      if (Row.isEmpty()) continue;
      char LastC(Row.at(Row.size() - 1));
      if (In(LastC, "!:;,?."))
        Result += LastC + Row.left(Row.size() - 1);
      else
        Result += Row;
      if (Rest > 1) Result += msCharNewLine;
      }
    }
  else
    Result = FromLang(m_TextEditor->toPlainText()).replace('"', msPrime).replace('{', msDoublePrime).replace('}', msTriplePrime).replace('\n', msCharNewLine);
  if (Result.endsWith(msCharNewLine)) Result.remove(Result.count() - 1, 1);
  return Result  + '"';
  }
