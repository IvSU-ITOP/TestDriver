#include "XPGedit.h"
#include "../FormulaPainter/InEdit.h"
#include "../TaskFileManager/taskfilemanager.h";
#include "../Mathematics/Parser.h"
#include "../Mathematics/Algebra.h"
#include "Panel.h"
#include <qdebug.h>

#ifdef Q_OS_ANDROID
#include "KeyBoard.h"
  int iFontSize = 64;
  int iPowDecrease = 24;
  int iPenWidth = 2;
#else
  int iFontSize = 16;
  int iPowDecrease = 6;
  int iPenWidth = 1;
#endif

ViewSettings XPGedit::sm_ViewSettings;
bool XPGedit::sm_ShowMultSignInQWindow = false;
bool XPGedit::sm_SelectPictures = false;
void( *XPGedit::sm_ChangeState )( bool ) = nullptr;
void( *XPGedit::sm_ResetTestMode )( ) = nullptr;
void( *XPGedit::sm_AutoTest )( ) = nullptr;

XPGedit::XPGedit( QWidget *parent, EditSets *pEditSets ) : QWidget( parent ),
  m_pCursor( new XPCursor( this ) ), m_VShift( 0 ), m_pScrollArea( new QScrollArea ), m_KeyPressed(false),
  m_HShift( 0 ), m_Refreshing( false ), m_IsCopy( false ), m_pImage( NULL ), m_pInEdit( NULL )
  {
  QPalette P;
  setAcceptDrops( true );
  P.setColor( backgroundRole(), QColor("white") );
  setPalette(P);
  setAutoFillBackground(true);
  if( pEditSets == nullptr )
    {
    QString FontMath( "Cambria Math" );
    QFont MainFont( FontMath, iFontSize, QFont::Normal );
    QFont PowerFont( MainFont );
    PowerFont.setPointSize( MainFont.pointSize() - iPowDecrease );
    m_EditSets = EditSets( MainFont, PowerFont, PowerFont, QString( "white" ), QString( "black" ), QString( "red" ) );
    }
  else
    m_EditSets = *pEditSets;
#ifdef Q_OS_ANDROID
  grabKeyboard();
  KeyboardButton::sm_pReceiver = this;
#endif
  }

void	XPGedit::resizeEvent( QResizeEvent * pEvent )
  {
  QWidget::resizeEvent( pEvent );
  int W = width();
  if( m_pImage != nullptr || width() <= 0 || height() <= 0 ) return;
  m_pImage = new QImage( width() * 2, height() * 4, QImage::Format_ARGB32 );
  QPainter *pPainter = new QPainter( m_pImage );
  QPen Pen = pPainter->pen();
  Pen.setWidth( iPenWidth );
  pPainter->setPen( Pen );
  delete m_pInEdit;
  m_pInEdit = new XPInEdit( TPoint( 0, 0 ), pPainter, m_EditSets, 
    ViewSettings( QFont( "Arial", 16 ), QFont( "Arial", 16 ), QFont( "Arial", 16 ), "white") );
  TMult::sm_ShowMultSign = true;
  XPInEdit::sm_pEditor = m_pInEdit;
  QRect R( cm_LeftWGEdMargin, cm_TopWGEdMargin, 32767, 32767 );
  m_pCursor->SetClipRect( R );
  MoveCursor();
  RefreshXPE();
  setAttribute( Qt::WA_AcceptDrops, false );
  setAttribute( Qt::WA_AcceptDrops, true );
  }

void XPGedit::paintEvent(QPaintEvent*)
  {
  m_pCursor->m_CanPaint = true;
  if( !m_pCursor->m_CursorOn )
    DisplayCursor();
  else
    if( m_pCursor->Blink() ) return;
  HideCursor();
  m_Painter.begin(this);
  m_Painter.drawImage( QPoint(cm_LeftWGEdMargin, cm_TopWGEdMargin), *m_pImage, m_pInEdit->GetRect() ); 
  m_Painter.end();
  DisplayCursor();
  }

void XPGedit::MoveCursor()
  {
  HideCursor();
  TPoint aCursPos = m_pInEdit->GetCursorPosition();
	int CursSize = m_pInEdit->GetCursorSize();
	TPoint CursPos = aCursPos;
	bool NeedShift = false;
//  if( TXPGrEl::sm_Language == lngHebrew ) 
//    CursPos.X += EEditWinWidth() - width();
	QRect ClipR( m_pCursor->m_ClipRect );
  CursPos.X += ClipR.left() - m_HShift;
	CursPos.Y += ClipR.top() - m_VShift;
	if( CursPos.X < ClipR.left() )
    {
		NeedShift = true;
		m_HShift = aCursPos.X;
    }
	if( CursPos.X > ClipR.right() )
    {
		NeedShift = true;
		m_HShift = aCursPos.X + ClipR.left() - ClipR.right();
    }
	if( CursPos.Y < ClipR.top() )
    {
		NeedShift = true;
		m_VShift = aCursPos.Y;
    }
	if( CursPos.Y + CursSize > ClipR.bottom() )
    {
		NeedShift = true;
		m_VShift = aCursPos.Y + CursSize + ClipR.top() - ClipR.bottom();
    }
	if( NeedShift )
    {
		CursPos = aCursPos;
		CursPos.X += ClipR.left() - m_HShift;
		CursPos.Y += ClipR.top()  - m_VShift;
    }
	m_pCursor->Move( CursPos, CursSize );
	repaint();
  }

int XPGedit::EEditWinHeight()
  {
	int Result = height() - 2 * cm_TopWGEdMargin;
  if( Result > 0 ) return Result;
	return 0;
  }

int XPGedit::EEditWinWidth()
  {
	int Result = width()  - 2 * cm_LeftWGEdMargin;
  if( Result > 0 ) return Result;
	return 0;
  }

void XPGedit::RefreshXPE()
  {
  TMult::sm_ShowMultSign = true;
  HideCursor();
  m_Refreshing = true;
  m_pImage->fill( m_EditSets.m_BkgrColor );
//  TMult::sm_ShowUnarMinus = true;
//  TSumm::sm_ShowMinusByAddition = false;
	m_pInEdit->EditDraw();
  int dh = m_pInEdit->m_Size.height() - height();
  bool bScroll = m_KeyPressed && dh > 0;
  m_KeyPressed = false;
//  SetSize(m_pInEdit->m_Size);
  if( bScroll )
    {
    QScrollBar *pScrollBar = m_pScrollArea->verticalScrollBar();
    pScrollBar->setValue(pScrollBar->value() + dh);
    }
	MoveCursor();
  m_Refreshing = false;
  repaint();
  setFocus();
  if( sm_ChangeState != nullptr ) sm_ChangeState( m_pInEdit->IsEmpty() );
  }

bool XPGedit::EdKeyPress( QKeyEvent *pMessage )
  {
  U_A_T Uact;
  char A;
  A = char( pMessage->key() );
  if ( !_printable( A ) ) exit(1);
  Uact.act = actPrintable;
  m_KeyPressed = true;
  Uact = A;
  Editor( Uact );
  return true;
  }

void XPGedit::mouseDoubleClickEvent( QMouseEvent *event )
  {
  qDebug() << "DoubleClick";
  m_pInEdit->ClearSelection();
  if( m_pInEdit->GetRect().contains( event->x(), event->y() ) )
    {
    qDebug() << "Select";
    if(sm_SelectPictures)
      m_pInEdit->SelectString(event->y());
    else
      m_pInEdit->Select();
    }
  m_pInEdit->sm_pEditor = nullptr;
  qDebug() << "EditDraw start";
  m_pInEdit->EditDraw();
  qDebug() << "EditDraw stop";
  m_pInEdit->sm_pEditor = m_pInEdit;
  repaint();
  qDebug() << "Was Repaint";
  }

void XPGedit::dragEnterEvent( QDragEnterEvent *event )
  {
  qDebug() << "Drag Start:";
  qDebug() << event;
  qDebug() << event->mimeData();
  qDebug() << event->mimeData()->hasHtml();
  qDebug() << event->mimeData()->html();
  if( event->mimeData()->hasHtml() ) event->acceptProposedAction();
  qDebug() << "Drag Enter Success";
  }

void XPGedit::dropEvent( QDropEvent *event )
  {
  qDebug() << "XPGedit dropEvent: ";
  if( event->source() == this )
    {
    m_pInEdit->ClearSelection();
    RefreshXPE();
    return;
    }
  event->acceptProposedAction();
  QString Text = event->mimeData()->html();
  int Img = Text.indexOf( "<img" );
  if( Img == -1 ) return;
  Img = Text.indexOf( "src", Img );
  Img = Text.indexOf( '"', Img ) + 1;
  QString Path = Text.mid( Img, Text.indexOf( '"', Img ) - Img );
  qDebug() << "Path: " + Path;
  QImage Image( Path );
  if( Image.isNull() ) return;  
  QByteArray Formula = Parser::UnpackUnAscii(Image.text( "F1" ).toLatin1());
  Formula = Formula.replace("@Table(", "Table(" );
  Formula = Formula.replace("@UnvisibleTable(", "UnvisibleTable(" );
  Formula = Formula.replace("@PartlyVisibleTable(", "PartlyVisibleTable(" );
  if( Formula.isEmpty() ) return;
  qDebug() << "Formula: " + Formula;
  m_pInEdit->SetMathFont();
  s_iDogOption = 1;
  MathExpr Expr = MathExpr( Parser::StrToExpr( Formula ) );
  s_iDogOption = 0;
  if( s_GlobalInvalid || Expr.IsEmpty() ) return;
  Formula = Expr.SWrite();
  qDebug() << "Formula for Paint: " + Formula;
  m_pInEdit->Clear();
  RestoreFormula( ShrinkFuncName(Formula) );
  }

void XPGedit::mousePressEvent( QMouseEvent* pEvent )
  {
  setFocus();
  if (pEvent->button() == Qt::RightButton && s_TaskEditorOn )
    {
    QMenu Menu;
/*
    const QMimeData * pReceived = QApplication::clipboard()->mimeData();
    QString Pict;
    if( pReceived  != nullptr ) Pict = pReceived->html();
    if(!Pict.isEmpty())
      {
      QAction *pPaste = Menu.addAction("Paste");
      Menu.addAction("Cancel");
      if (Menu.exec(QCursor::pos()) != pPaste) return;
      Pict = Pict.left(Pict.lastIndexOf('/'));
      Pict = Pict.mid(Pict.lastIndexOf('/') + 1);
      Pict = Pict.left(Pict.indexOf('"'));
      TExprPict *pPict = new TExprPict(Pict.toLocal8Bit());
      m_pInEdit->RestoreFormula(pPict->SWrite());
      delete pPict;
      RefreshXPE();
      return;
      }
*/
    QAction *pCopyFormula = Menu.addAction("Copy Expression");
    pCopyFormula->setEnabled(CanCopy());
    QAction *pExpression = Menu.addAction("Copy Formula");
    pExpression->setEnabled(pCopyFormula->isEnabled());
    QAction *pToWord = Menu.addAction("Copy to Word");
    pToWord->setEnabled(pCopyFormula->isEnabled());
    QAction *pPaste = Menu.addAction("Paste");
    pPaste->setEnabled(CanPaste());
    QAction *pCancel = Menu.addAction("Cancel");
    QAction *pResult = Menu.exec(QCursor::pos());
    if(pResult == pCancel) return;
    if(pResult == pCopyFormula)
      {
      QApplication::clipboard()->setText( Parser::PackUnAscii( m_FormulaForCopy ) );
      return;
      }
    if(pResult == pExpression)
      {
      Copy();
      return;
      }
    if(pResult == pToWord)
      {
      Copy(true);
      return;
      }
    QString SF = QApplication::clipboard()->text();
    if( SF.length() < 2)
      {
      Paste();
      return;
      }
    QByteArray Formula( SF.toLocal8Bit() );
    s_iDogOption = 1;
    MathExpr Expr = MathExpr( Parser::StrToExpr( ExpandFuncName( Formula ) ) );
    s_iDogOption = 0;
    if( s_GlobalInvalid || Expr.IsEmpty() )
      {
      QMessageBox::warning(nullptr, "Bad expression", Formula + " is bad formula!");
      return;
      }
    m_pInEdit->Clear();
    RestoreFormula( Expr.SWrite() );
    return;
    }
  if(m_pInEdit->Selected())
    {
    QByteArray Formula;
    Formula = m_pInEdit->Write();
    if( Formula.isEmpty() ) return;
    if( sm_SelectPictures )
      {
      if(Formula.back() == ';') Formula.chop(1);
      Formula = "Picture(\"" + Formula + "\")";
      }
    if( s_TaskEditorOn )
      {
      m_pInEdit->SetMathFont();
      MathExpr Expr = MathExpr( Parser::StrToExpr( ExpandFuncName(Formula) ) );
      if( s_GlobalInvalid || Expr.IsEmpty() )
        {
        m_pInEdit->ClearSelection();
        repaint();
        return;
        }
       QString Path( RichTextDocument::GetTempPath());
       bool bText = XPInEdit::sm_TextFont;
       if( bText ) m_pInEdit->SetMathFont();
       BaseTask::sm_pEditSets->m_BkgrColor = sm_ViewSettings.m_BkgrColor;
       bool ShowMultSignOld = TMult::sm_ShowMultSign;
       TMult::sm_ShowMultSign = sm_ShowMultSignInQWindow;
       try {
      XPInEdit InEd( Expr.SWrite(), *BaseTask::sm_pEditSets, sm_ViewSettings );
      if( bText ) m_pInEdit->SetTextFont();
      QImage *pImage = InEd.GetImage();
      TMult::sm_ShowMultSign = ShowMultSignOld;
      pImage->setText( "F1", Parser::PackUnAscii( Formula ) );
      pImage->save( Path );
       }
//      pImage->save("C:/ProgramData/Halomda/Formula.jpg");
       catch( ErrParser& ErrMsg )
         {
         QMessageBox::critical( nullptr, "Error", X_Str( ErrMsg.Name(), ErrMsg.Message() ) );
         return;
         }
      QString Html = "<img src=\"" + Path + "\" style=\"vertical-align:middle;\" />";
      QDrag *pDrag = new QDrag( this );
      pDrag->setPixmap( QPixmap( ":/Resources/Drag.png" ) );
      QMimeData *pMimeData = new QMimeData;
      pMimeData->setHtml( Html );
      pDrag->setMimeData( pMimeData );
      Qt::DropAction Action = pDrag->exec();
      qDebug() << "Drop executed with result " << Action;
      m_pInEdit->ClearSelection();
      repaint();
      qDebug() << "Success repaint afer drop" << Action;
      return;
      }
   }
  QPoint Pos = pEvent->pos();
  U_A_T Uact;
  QSize Size;
  int Axis;
  Uact.act = actMouseButton;
  Uact.Parms.coord.X = Pos.x() - cm_LeftWGEdMargin;
  Uact.Parms.coord.Y = Pos.y() - cm_TopWGEdMargin;
  m_pInEdit->PreCalc( TPoint( 0, 0 ), Size, Axis );
  if( pEvent->button() != Qt::LeftButton ) XPInEdit::sm_EditString = true;
  if( m_pInEdit->GlobalSetCurrent( Uact.coord().X, Uact.coord().Y ) )
    Editor( Uact );
  XPInEdit::sm_EditString = false;
  }

void XPGedit::Editor( U_A_T &Uact )
  {
  switch( m_pInEdit->EditAction( Uact ) )
    {
  case edCursor:
    MoveCursor();
    return;
  case edRefresh:
    m_IsCopy = false;
    m_pInEdit->ClearSelection();
    RefreshXPE();
    return;
  case edBeep:
    QMessageBox::warning(this, "Error", "Bad key was pressed" );
    return;
  case edInvalid:
    QMessageBox::warning(this, "Internal Error", "Impossible to start new line!" );
    }
  }

bool XPGedit::CanCopy()
  {
  m_FormulaForCopy = m_pInEdit->Write();
  if(m_FormulaForCopy.isEmpty()) return false;
  m_ExprForCopy = MathExpr( Parser::StrToExpr( ExpandFuncName(m_FormulaForCopy) ) );
  if( s_GlobalInvalid || m_ExprForCopy.IsEmpty() ) return false;
  return true;
  }

bool XPGedit::CanPaste()
  {
  QClipboard *pClipboard = QApplication::clipboard();
  const QMimeData *pMime = pClipboard->mimeData();
  QImage Image;
  if(pMime != nullptr)
    {
    QString Html = pMime->html();
    if(Html.isEmpty()) return false;
    int iImg = Html.indexOf("<img");
    if(iImg == -1) return false;
    int iPath = Html.indexOf('"', iImg) + 1;
    QString Path = Html.mid(iPath, Html.indexOf('"', iPath) - iPath );
    Image.load(Path);
    }
  else
    Image = pClipboard->image();
  if( Image.isNull() ) return false;
  m_FormulaForPaste = Image.text( "F1" ).toLatin1();
  return !m_FormulaForPaste.isEmpty();
  }

void XPGedit::Copy(bool ToWord)
  {
  QClipboard *pClipboard = QApplication::clipboard();
  bool bText = XPInEdit::sm_TextFont;
  if( bText ) m_pInEdit->SetMathFont();
  if(ToWord)
    BaseTask::sm_pEditSets->m_BkgrColor = Qt::white;
  else
    BaseTask::sm_pEditSets->m_BkgrColor = sm_ViewSettings.m_BkgrColor;
  bool ShowMultSignOld = TMult::sm_ShowMultSign;
  TMult::sm_ShowMultSign = sm_ShowMultSignInQWindow;
  XPInEdit InEd( m_ExprForCopy.SWrite(), *BaseTask::sm_pEditSets, sm_ViewSettings );
  if( bText ) m_pInEdit->SetTextFont();
  QImage *pImage = InEd.GetImage();
  pImage->setText( "F1", Parser::PackUnAscii( m_FormulaForCopy) );
  TMult::sm_ShowMultSign = ShowMultSignOld;
  QString Path( RichTextDocument::GetTempPath());
  pImage->save( Path );
  QString Html = "<img src=\"" + Path + "\" style=\"vertical-align:middle;\" />";
  QMimeData *pMimeData = new QMimeData;
  pMimeData->setHtml(Html);
  pClipboard->setMimeData(pMimeData);
  }

void XPGedit::Paste()
  {
  m_pInEdit->Clear();
  RestoreFormula( m_FormulaForPaste );
  }

void XPGedit::keyPressEvent( QKeyEvent *pE )
  {
  U_A_T Uact;
  
  auto Default = [&] ()
    {
//    char C = pE->text()[0].cell();
    char C = FromLang(pE->text())[0];
    if( C == 0 || !_printable( C ) ) return;
    Uact = C;
    m_pInEdit->sm_EditKeyPress = true;
    };

  switch( int Key = pE->key() )
    {
    case Qt::Key_Shift:
      return;
    case Qt::Key_Left:
      Uact = "vk_Left";
      break;
    case Qt::Key_Up:
      Uact = "vk_Up";
      break;
    case Qt::Key_Right:
      Uact = "vk_Right";
      break;
    case Qt::Key_Down:
      Uact = "vk_Down";
      break;
    case Qt::Key_Delete:
      if(sm_SelectPictures)
        {
        sm_SelectPictures = false;
        m_pInEdit->Clear();
        RefreshXPE();
        return;
        }
      Uact = "vk_Delete";      
      break;
    case Qt::Key_Backspace:
      if(sm_SelectPictures)
        {
        sm_SelectPictures = false;
        m_pInEdit->Clear();
        RefreshXPE();
        return;
        }
      Uact = "vk_Back";
      break;
    case Qt::Key_Return:
      m_pInEdit->SetRegister( irNormal );
      if( MainTab::sm_Enter != nullptr )
        {
        MainTab::sm_Enter();
        return;
        }
      Uact = static_cast< char >( msCharNewLine );
      break;
    case Qt::Key_T:
      if( pE->modifiers() & Qt::ControlModifier )
        {
        if( sm_ResetTestMode != nullptr ) sm_ResetTestMode();
        return;
        }
      Default();
      break;
    case Qt::Key_P:
      if( pE->modifiers() & Qt::ControlModifier )
        {
        if( sm_AutoTest != nullptr ) sm_AutoTest();
        return;
        }
      Default();
      break;
    case Qt::Key_C:
    case Qt::Key_V:
      if( pE->modifiers() & Qt::ControlModifier )
        {
        if( Key == Qt::Key_C )
          {
          if( CanCopy() ) Copy();
          return;
          }
        if(CanPaste()) Paste();
        }
      Default();
      break;
    case Qt::Key_F:
      if( pE->modifiers() & Qt::ControlModifier )
        {
        QByteArray Formula( m_pInEdit->Write() );
        if( Formula.isEmpty() ) return;
        s_iDogOption = 1;
        MathExpr Expr = MathExpr( Parser::StrToExpr( ExpandFuncName( Formula ) ) );
        s_iDogOption = 0;
        if( s_GlobalInvalid || Expr.IsEmpty() ) return;
        QApplication::clipboard()->setText( Parser::PackUnAscii( Formula ) );
        return;
        }
      if( pE->modifiers() & Qt::AltModifier )
        {
        QString SF = QApplication::clipboard()->text();
        if( SF.isEmpty() ) return;
        QByteArray Formula( SF.toLocal8Bit() );
        s_iDogOption = 1;
        MathExpr Expr = MathExpr( Parser::StrToExpr( ExpandFuncName( Formula ) ) );
        s_iDogOption = 0;
        if( s_GlobalInvalid || Expr.IsEmpty() ) return;
        m_pInEdit->Clear();
        RestoreFormula( Expr.SWrite() );
        return;
        }
    default:
      Default();
    }
  m_KeyPressed = true;
  Editor( Uact );
  m_pInEdit->sm_EditKeyPress = false;
  }

void XPGedit::SaveFormula()
  {
  QString sOutFile, Dir;
#ifdef Q_OS_ANDROID
  Dir = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation );
  Dir = Dir.left( Dir.lastIndexOf( '/' ) );
#endif
  if( ( sOutFile = QFileDialog::getSaveFileName( NULL, "Select file for saving of formula", Dir,
    "Expression Files (*.xpr);;Latex Files(*.txpr);;Picture files(*.png *.jpg);;HTML files (*.htm *.html)" ) ).isEmpty() ) return;
  QFile Fout( sOutFile );
  Fout.open( QIODevice::WriteOnly );
  if( !Fout.isOpen() )
    {
    Info_m( QString( "Can't create file " ) + sOutFile );
    return;
    }

  QByteArray FileExt = sOutFile.right( sOutFile.length() - sOutFile.lastIndexOf( '.' ) - 1 ).toLatin1();

  if( FileExt.isEmpty() )
    {
    FileExt = "htm";
    sOutFile += ".htm";
    }

  if( FileExt == "xpr" )
    {
    QByteArray Formula( m_pInEdit->Write() );
    MathExpr Expr = MathExpr( Parser::StrToExpr( Formula ) );
    if( s_GlobalInvalid || Expr.IsEmpty() )
      Info_m( X_Str( "MsyntaxErr", "Syntax error! " ) + Formula );
    else
      Fout.write( Formula );
    return;
    }
  
  if( FileExt == "txpr" )
    {
    Fout.write( m_pInEdit->SWrite() );
    return;
    }

  QImage Formula( m_pInEdit->PWrite() );
  QString Text( Formula.text( "F1" ) );
  QByteArray sFormula;
  QBuffer Buf( &sFormula );
  Buf.open( QIODevice::WriteOnly );
  if( FileExt == "png" || FileExt == "jpg" )
    {
    Formula.save( &Buf, FileExt );
    Fout.write( sFormula );
    return;
    }
  Formula.save( &Buf, "PNG" );
  sFormula = sFormula.toBase64();
  QTextStream FS( &Fout );
  FS << "<html>\n<head>\n<title>Picture</title>\n</head>\n<body>\n" << Text << "<br>";
  FS << "<img src=\"data:image/png;base64," << sFormula << "\">\n</body>\n</html>";
  }

void XPGedit::RestoreFormula( const QByteArray& Formula )
  {
  TMult::sm_ShowMultSign = true;
  m_pInEdit->RestoreFormula( Formula, s_TaskEditorOn );
  RefreshXPE();
  }

void XPGedit::RestoreFromFile()
  {
  QString sInFile, Dir;
#ifdef Q_OS_ANDROID
  Dir = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation );
  Dir = Dir.left( Dir.lastIndexOf( '/' ) );
#endif
  sInFile = QFileDialog::getOpenFileName( NULL,
    "Select file with formula", Dir, "Expression Files (*.xpr);;Latex Files(*.txpr);;Picture files(*.png *.jpg);;HTML files (*.htm *.html)" );
  if( sInFile.isEmpty() ) return;
  QFile Fin( sInFile );
  Fin.open( QIODevice::ReadOnly );
  QByteArray StartLine;
  try
    {
    if( !Fin.isOpen() ) throw "Can't open file " + sInFile;
    QByteArray FileExt = sInFile.right( sInFile.length() - sInFile.lastIndexOf( '.' ) - 1 ).toLatin1();

    if( FileExt == "xpr" )
      {
      QByteArray Formula( Fin.readAll() );
      MathExpr Expr = MathExpr( Parser::StrToExpr( Formula ) );
      if( s_GlobalInvalid || Expr.IsEmpty() )
        Info_m( X_Str( "MsyntaxErr", "Syntax error! " ) + Formula );
      else
        RestoreFormula( Expr.SWrite() );
      return;
      }

    if( FileExt == "txpr" )
      {
      RestoreFormula( Fin.readAll() );
      return;
      }

    QImage Img;
    m_pInEdit->Clear();
    if( FileExt == "htm" || FileExt == "html" )
      {
      QTextStream FS( &Fin );
      QString Html = FS.readAll();
      int iStart = Html.indexOf( "base64" ) + 7;
      if( iStart == 6 ) throw "The file " + sInFile + " not contain picture";
      int iEnd = Html.indexOf( '"', iStart );
      Img.loadFromData( QByteArray::fromBase64( Html.mid( iStart, iEnd - iStart ).toLatin1() ) );
      }
    else
      Img.load( sInFile );
    QByteArray Formula = Img.text( "F1" ).toLatin1();
    if( Formula.isEmpty() ) throw "Picture in file " + sInFile + " not contain formula";
    RestoreFormula( Formula );
    }
  catch( QString Err )
    {
    QMessageBox::information( NULL, "Error", Err );
    }
  }

void XPGedit::UnReadExpr( const MathExpr& Exp ) 
  {
  if( Exp.IsEmpty() )  return;
  if( IsConstType( TMathGraph, Exp ) )
    {
    Panel::sm_GeoActivate = true;
    Panel::sm_pGraphEdit->RestoreObject( CastConst( TMathGraph, Exp )->Source() );
    return;
    };
  s_GlobalInvalid = false;
  QByteArray Formula = Exp.SWrite();
  if( s_GlobalInvalid ) return;
  RestoreFormula( Formula );
  }

QByteArray& XPGedit::ShrinkFuncName( QByteArray& FuncName )
  {
  return FuncName.replace( "@Simplify(", "@S(" ).replace( "@PolinomSmplf(", "@PS(" ).replace( "@SimplifyEquation(", "@SE(" );
  }

QByteArray& XPGedit::ExpandFuncName( QByteArray& FuncName )
  {
  return FuncName.replace( "@S(", "@Simplify(" ).replace( "@PS(", "@PolinomSmplf(" ).replace( "@SE(", "@SimplifyEquation(" );
  }

void XPGedit::Clear( bool SaveTemplate )
  {
  m_pInEdit->Clear( SaveTemplate );
  }

void XPGedit::ResetView( bool ShowFunctions )
  {
  if( ShowFunctions )
    m_pInEdit->SetTextFont();
  else
    m_pInEdit->SetMathFont();
  QByteArray Formula( m_pInEdit->Write() );
  if( Formula.isEmpty() ) return;
  s_iDogOption = 1;
  MathExpr Expr = MathExpr( Parser::StrToExpr( ExpandFuncName( Formula ) ) );
  s_iDogOption = 0;
  if( s_GlobalInvalid || Expr.IsEmpty() ) return;
  if( ShowFunctions )
    Formula = Parser::PackUnAscii(Expr.WriteE());
  else
    Formula = Expr.SWrite();
  m_pInEdit->Clear();
  RestoreFormula( ShrinkFuncName( Formula ) );
  }

QScrollArea* XPGedit::SetSize( QSize Size )
  {
  if(m_MinSize.isValid())
    {
    Size.setHeight(max(m_MinSize.height(), Size.height()));
    Size.setWidth(max(m_MinSize.width(), Size.width()));
    m_pScrollArea->setFixedSize(Size.width(), Size.height() + 5);
    }
  else
    {
    m_MinSize = Size;
    m_pScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_pScrollArea->setFixedSize(Size.width(), Size.height() + 5);
    m_pScrollArea->setWidget( this );
    m_pScrollArea->setAlignment( Qt::AlignCenter );
    }
  setFixedSize(Size);
  return m_pScrollArea;
  }

DockWithoutTitle::DockWithoutTitle( QWidget *pWidget )
  {
  QWidget *pTitleBarWidget = titleBarWidget();
  setTitleBarWidget( new QWidget );
  delete pTitleBarWidget;
  setWidget( pWidget );
  }

