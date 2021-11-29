#include "SelectTask.h"
#include "WinTesting.h"
#include "../FormulaPainter/InEdit.h"
#include <QNetworkRequest>

QByteArray PasswordDialog::sm_RootUrl( "https://halomda.org/TestingDriverMy/" );
//QByteArray PasswordDialog::sm_RootUrl( "http://89.179.68.230/TestingDriverMy/" );

QByteArray Connector::Connect()
  {
  QEventLoop Loop;
  QUrl U( m_Url );
  QNetworkRequest R( U );
  R.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  QNetworkAccessManager *pAM = new QNetworkAccessManager(this);
  m_pReply = pAM->post( R, m_Command );
//  m_pReply = WinTesting::sm_NetworkManager.post( R, m_Command );
  connect( m_pReply, SIGNAL( readyRead() ), SLOT( ReadyRead() ) );
  connect( m_pReply, SIGNAL( finished() ), &Loop, SLOT( quit() ) );
  Loop.exec();
  disconnect( m_pReply, SIGNAL( finished() ), &Loop, SLOT( quit() ) );
  disconnect( m_pReply, SIGNAL( readyRead() ), this, SLOT( ReadyRead() ) );
  return m_Response;
  }

void  Connector::ReadyRead()
  {
  m_Response += m_pReply->readAll();
  }

PasswordDialog::PasswordDialog() : m_pPassInput( new QLineEdit( this ) ), m_pRootUrl( new QLineEdit( sm_RootUrl, this ) ),
  m_pPassError(new QLabel("Invalid Password")), m_pPassAccept(new QPushButton("Accept")), m_LokalWork(false)
  {
  setWindowTitle( "Input of Password" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  m_pPassInput->setMaxLength( 11 );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel( "Root Url:" );
  pMainLayout->addWidget( pLabel );
  pMainLayout->addWidget( m_pRootUrl );
  pLabel = new QLabel( "Type password:" );
  pMainLayout->addWidget( pLabel );
  pMainLayout->addWidget( m_pPassInput );
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  m_pPassAccept->setEnabled( false );
  pButtonLayout->addWidget( m_pPassAccept );
  QPushButton *pLokalWork = new QPushButton( "To Work locally" );
  pButtonLayout->addWidget( pLokalWork );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  pButtonLayout->addWidget( pCancel );
  pMainLayout->addLayout( pButtonLayout );
  m_pPassError->hide();
  pMainLayout->addWidget( m_pPassError );
  setLayout( pMainLayout );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  connect( m_pPassAccept, SIGNAL( clicked() ), SLOT( accept() ) );
  connect( pLokalWork, SIGNAL( clicked() ), SLOT( LokalWork() ) );
  connect( m_pPassInput, SIGNAL( textChanged( const QString& ) ), SLOT( PassChanged( const QString& ) ) );
  }

void PasswordDialog::PassChanged( const QString& Pass )
  {
  m_pPassAccept->setEnabled( !Pass.isEmpty() );
  }

void PasswordDialog::accept()
  {
  sm_RootUrl = m_pRootUrl->text().toLocal8Bit();
  if( sm_RootUrl.right( 1 ) != "/" ) sm_RootUrl += '/';
  Connector C( sm_RootUrl + "TestStudentForCPP.php", "Password=" + ( m_pPassInput->text() ).toLocal8Bit() );
  m_UsrId = C.Connect();
  if( m_UsrId.isEmpty() || m_UsrId.length() > 11 ) throw QString( "Password testing error:" + m_UsrId );
  if( m_UsrId == "Error" )
    m_pPassError->show();
  else
    QDialog::accept();
  }

void PasswordDialog::LokalWork()
  {
  m_LokalWork = true;
  QDialog::accept();
  }

SelectTest::SelectTest( const QByteArray& UserCode ) : m_pListTests( new QComboBox )
  {
  setWindowTitle( "Selection of Test" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  Connector C( PasswordDialog::sm_RootUrl + "SelectTest.php", "UsrId=" + UserCode );
  QByteArray Response = C.Connect();
  if( Response == "No Test" ) throw QString( "Sorry, there is no test available for you!" );
  QByteArrayList TestList( Response.split( '#' ) );
  if( TestList.isEmpty() ) throw QString( "Test List error" );
  QByteArray Test;
  foreach( Test, TestList )
    {
    QByteArrayList TestParms( Test.split( ';' ) );
    if( TestParms.count() != 2 ) throw QString( "Test List error: " + Test );
    m_pListTests->addItem( ToLang( TestParms[0] ), TestParms[1] );
    }
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel( "Select the Test:" );
  pMainLayout->addWidget( pLabel );
  pMainLayout->addWidget( m_pListTests );
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  QPushButton *pSelect = new QPushButton( "Select" );
  pButtonLayout->addWidget( pSelect );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  pButtonLayout->addWidget( pCancel );
  pMainLayout->addLayout( pButtonLayout );
  setLayout( pMainLayout );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  connect( pSelect, SIGNAL( clicked() ), SLOT( accept() ) );
  }

MainTestMenu::MainTestMenu( const QByteArray& TstId, const QByteArray& UsrId, MainTestDlg *pDlg )
  { 
  Connector C( PasswordDialog::sm_RootUrl + "SelectChapter.php", "UsrId=" + UsrId + "&TestId=" + TstId );
  QByteArray Response = C.Connect();
  if( Response.isEmpty() ) throw QString( "List Main Menu was empty" );
  QByteArrayList ChapterList( Response.split( '#' ) );
  setColumnCount( 6 );
  setRowCount( ChapterList.count() );
  verticalHeader()->hide();
  setHorizontalHeaderLabels( QStringList() << ToLang( "цйеп" ) << ToLang( "щамеъ" ) << ToLang( "обзп" )
    << ToLang( "ъшвем" ) << ToLang( "мйоег" ) << ToLang( "реща" ) );
  for( int iRow = 0; iRow < ChapterList.count(); iRow++ )
    {
    QByteArrayList MenuParms( ChapterList[iRow].split( ';' ) );
    if( MenuParms.count() != 4 ) throw QString( "Row Main Menu error: " + ChapterList[iRow] );
    for( int iCol = 0; iCol < 2; iCol++ )
      setItem( iRow, iCol, new QTableWidgetItem( MenuParms[iCol], 0 ) );
    QTableWidgetItem *pItem = new QTableWidgetItem( QIcon( ":/Resources/mivhan.png" ), "", 1000 );
    pItem->setData( Qt::UserRole, 'E' + MenuParms[2] );
    setItem( iRow, 2, pItem );
    pItem = new QTableWidgetItem( QIcon( ":/Resources/tirgul.png" ), "", 1000 );
    pItem->setData( Qt::UserRole, 'T' + MenuParms[2] );
    setItem( iRow, 3, pItem );
    pItem = new QTableWidgetItem( QIcon( ":/Resources/limud.png" ), "", 1000 );
    pItem->setData( Qt::UserRole, 'L' + MenuParms[2] );
    setItem( iRow, 4, pItem );
    setItem( iRow, 5, new QTableWidgetItem( ToLang(MenuParms[3]), 0 ) );
    }
  setIconSize( QSize( 100, 100 ) );
  resizeRowsToContents();
  resizeColumnsToContents();
  setMouseTracking( true );
  connect( this, SIGNAL( itemClicked( QTableWidgetItem * ) ), pDlg, SLOT( ClickItem( QTableWidgetItem * ) ) );
  int iColHeight = 0;
  for( int iRow = 0; iRow < rowCount(); iColHeight += rowHeight( iRow++ ) );
  setFixedHeight( min( iColHeight += 100, 600 ) ); 
  int iRowWidth = 0;
  for( int iCol = 0; iCol < columnCount(); iRowWidth += columnWidth( iCol++ ) );
  verticalScrollBar()->adjustSize();
  if( height() >= 600 ) iRowWidth += verticalScrollBar()->width();
  setFixedWidth( iRowWidth + 5 );
  }

void MainTestMenu::mouseMoveEvent( QMouseEvent *pEvent )
  {
  QTableWidgetItem *pItem = itemAt( pEvent->pos() );
  if( pItem == nullptr || pItem->type() != 1000 )
    setCursor( Qt::ArrowCursor );
  else
    setCursor( Qt::PointingHandCursor );
  }

MainTestDlg::MainTestDlg( const QByteArray& Parms, const QByteArray& UsrId, const QString& TestName ) :  m_BackToSelectTest( false )
  {
  setWindowTitle( TestName );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  QPushButton *pBack = new QPushButton( "Back to Test Selection" );
  pButtonLayout->addWidget( pBack );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  connect( pBack, SIGNAL( clicked() ), SLOT( BackToSelectTest() ) );
  pButtonLayout->addWidget( pCancel );
  pMainLayout->addLayout( pButtonLayout );
  MainTestMenu *pMenu = new MainTestMenu( Parms, UsrId, this );
  pMainLayout->addWidget( pMenu );
  setLayout( pMainLayout );
  }

void MainTestDlg::ClickItem( QTableWidgetItem *pItem )
  {
  if( pItem->type() == 0 ) return;
  m_SelectedChapter = pItem->data( Qt::UserRole ).toByteArray();
  m_ChapterName = pItem->text();
  accept();
  }

void MainTestDlg::BackToSelectTest()
  {
  m_BackToSelectTest = true;
  accept();
  }

SelectTopic::SelectTopic( const QByteArray& PrmId, const QByteArray& UsrId, const QByteArray& Chp_id, SelectTopicDlg *pDlg )
  {
  Connector C( PasswordDialog::sm_RootUrl + "SelectTopic.php", "UsrId=" + UsrId + "&PrmId=" + PrmId + "&Parms=" + Chp_id );
  QByteArray Response = C.Connect();
  if( Response.isEmpty() ) throw QString( "List Topic was empty" );
  QByteArrayList TopicList( Response.split( '#' ) );
  int TopicCount = TopicList.count();
  if( TopicCount == 1 )
    {
    pDlg->m_NoSelection = true;
    QByteArrayList Parms( TopicList[0].split( ';' ) );
    pDlg->m_SelectedTopic = Parms[1];
    return;
    } 
  if( Chp_id[0] == 'E' )
    {
    QByteArrayList Parms( TopicList[0].split( ';' ) );
    pDlg->m_ETime = Parms[0];
    pDlg->m_DateDiff = Parms[1];
    TopicList.removeFirst();
    TopicCount--;
    }
  setColumnCount( 1 );
  setRowCount( TopicCount );
  verticalHeader()->hide();
  horizontalHeader()->hide();
  for( int iRow = 0; iRow < TopicCount; iRow++ )
    {
    QByteArrayList MenuParms( TopicList[iRow].split( ';' ) );
    QTableWidgetItem *pItem;
    QString Label( ToLang(MenuParms[0]) );
    switch( MenuParms.count() )
      {
      case 2:
        if( MenuParms[1][0] == '(' )
          pItem = new QTableWidgetItem( MenuParms[1] + ' ' + Label, 0 );
        else
          {
          pItem = new QTableWidgetItem( Label, 1000 );
          pItem->setData( Qt::UserRole, MenuParms[1] );
          }
        break;
      case 4:
        Label = MenuParms[3] + ' ' + Label;
      case 3:
        {
        QPixmap Pic( 30, 30 );
        QPainter Painter( &Pic );
        QByteArray &C = MenuParms[1];
        QBrush Brush( QColor( C.left( 2 ).toInt( nullptr, 16 ), C.mid( 2, 2 ).toInt( nullptr, 16 ), C.mid( 4 ).toInt( nullptr, 16 ) ) );
        Painter.fillRect( Pic.rect(), Brush );
        char Sign = MenuParms[2][0];
        if( Sign == 'P' || Sign == 'N' )
          Painter.setFont( QFont( "Wingdings 2", 16 ) );
        else
          Painter.setFont( QFont( "Wingdings", 16 ) );
        if( Sign == 'P' || Sign == 'K' || Sign == 'J' )
          Painter.setPen( Qt::blue );
        else
          Painter.setPen( Qt::white );
        Painter.drawText( Pic.rect(), Qt::AlignCenter, MenuParms[2] );
        Painter.end();
        pItem = new QTableWidgetItem( QIcon( Pic ), Label, 0 );
        pItem->setBackground( Brush );
        }
        break;
      default:
        throw QString( "Row Topic List error: " + TopicList[iRow] );
      }
    pItem->setTextAlignment( Qt::AlignCenter );
    setItem( iRow, 0, pItem );
    }
  setIconSize( QSize( 30, 30 ) );
  resizeRowsToContents();
  int iRowWidth = 400;
  setColumnWidth( 0, iRowWidth );
  setMouseTracking( true );
  connect( this, SIGNAL( itemClicked( QTableWidgetItem * ) ), pDlg, SLOT( ClickItem( QTableWidgetItem * ) ) );
  int iColHeight = 0;
  for( int iRow = 0; iRow < rowCount(); iColHeight += rowHeight( iRow++ ) );
  setFixedHeight( min( iColHeight += 100, 600 ) );
  verticalScrollBar()->adjustSize();
  if( height() >= 600 ) iRowWidth += verticalScrollBar()->width();
  setFixedWidth( iRowWidth + 5 );
  }

void SelectTopic::mouseMoveEvent( QMouseEvent *pEvent )
  {
  QTableWidgetItem *pItem = itemAt( pEvent->pos() );
  if( pItem == nullptr || pItem->type() != 1000 )
    setCursor( Qt::ArrowCursor );
  else
    setCursor( Qt::PointingHandCursor );
  }

SelectTopicDlg::SelectTopicDlg( const QByteArray& PrmId, const QByteArray& UsrId, const QByteArray& Chp_id, const QString& ChapterName ) :
  m_BackToSelectChapter( false ), m_NoSelection( false )
  {
  SelectTopic *pTopicList = new SelectTopic( PrmId, UsrId, Chp_id, this );
  if( m_NoSelection )
    {
    delete pTopicList;
    return;
    }
  setWindowTitle( ChapterName );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  QPushButton *pBack = new QPushButton( "Back to Test Chapter Selection" );
  pButtonLayout->addWidget( pBack );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  connect( pBack, SIGNAL( clicked() ), SLOT( BackToSelectChapter() ) );
  pButtonLayout->addWidget( pCancel );
  pMainLayout->addLayout( pButtonLayout );
  pMainLayout->addWidget( pTopicList );
  setLayout( pMainLayout );
  }

void SelectTopicDlg::ClickItem( QTableWidgetItem *pItem )
  {
  if( pItem->type() == 0 ) return;
  m_SelectedTopic = pItem->data( Qt::UserRole ).toByteArray();
  accept();
  }

void SelectTopicDlg::BackToSelectChapter()
  {
  m_BackToSelectChapter = true;
  accept();
  }

void OldDriverStarter::Start( const QString& Parms )
  {
  HKEY hkPlugin;
  if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Halomda\\XPressPlugin", 0, KEY_READ, &hkPlugin ) != ERROR_SUCCESS )
    throw QString( "Register Key Error");
  unsigned long iLen = _MAX_PATH;
  unsigned long iType = REG_SZ;
  char cPath[_MAX_PATH];
  if( RegQueryValueExA( hkPlugin, "Path_OldTestDriver", 0, &iType, ( unsigned char* ) cPath, &iLen ) != ERROR_SUCCESS )
    throw QString( "Register Path Error" );
  RegCloseKey( hkPlugin );
  QString sPath( cPath );
  if( sPath.right( 1 ) != "\\" ) sPath += "\\";
  m_pProcess->setWorkingDirectory( sPath );
  QString sPathName = sPath + "TestDriver.exe";
  if( GetFileAttributes( ( LPCWSTR ) sPathName.utf16() ) == 0xFFFFFFFF )
    throw sPathName + " not exists";
  connect( m_pProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( FinishDriver( int, QProcess::ExitStatus ) ) );
  m_pProcess->start( '"' + sPathName + "\" " + Parms );
  if( !m_pProcess->waitForStarted() ) throw QString( "Error by start of TestingDriver" );
  QEventLoop Loop;
  connect( this, SIGNAL( PostFinish() ), &Loop, SLOT( quit() ) );
  Loop.exec();
  disconnect( this, SIGNAL( PostFinish() ), &Loop, SLOT( quit() ) );
  disconnect( m_pProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( FinishDriver( int, QProcess::ExitStatus ) ) );
  }

void OldDriverStarter::FinishDriver( int, QProcess::ExitStatus )
  {
  emit PostFinish();
  }

CreateMoodleBank::CreateMoodleBank() : m_pCreateBank( new QPushButton( "Create Quiz" ) ), 
m_pSelectedTests(new QStandardItemModel(0, 1)), m_pQuizName(new QLineEdit)
  {
  Connector C( PasswordDialog::sm_RootUrl + "SelectAllTests.php", "UsrId=18361" );
  QByteArrayList Response = C.Connect().split('#');
  m_pAvailableTests = new QStandardItemModel( Response.size(), 1 );
  for( int iRow = 0; iRow < m_pAvailableTests->rowCount(); iRow++ )
    {
    QModelIndex Index = m_pAvailableTests->index( iRow, 0 );
    QByteArrayList Row = Response[iRow].split( ';');
    QByteArray R = Row[1];
    QString AS = ToLang( Row[1] );
    m_pAvailableTests->setData( Index, ToLang( Row[1] ), Qt::DisplayRole );
    m_pAvailableTests->setData( Index, Row[0], Qt::UserRole );
    }
  QListView *pAvailable = new QListView;
  pAvailable->setViewMode( QListView::ListMode );
  connect( pAvailable, SIGNAL( clicked( const QModelIndex& ) ), SLOT( SelectTest( const QModelIndex& ) ) );
  pAvailable->setModel( m_pAvailableTests );
  setWindowTitle( "Create Questions Bank for Moodle" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QHBoxLayout *pListLayout = new QHBoxLayout;
  QVBoxLayout *pLeftLayout = new QVBoxLayout;
  pLeftLayout->addWidget( new QLabel( "Available Tests" ) );
  pLeftLayout->addWidget( pAvailable );
  pAvailable->setFixedHeight( 600 );
  pListLayout->addLayout( pLeftLayout );
  QVBoxLayout *pRightLayout = new QVBoxLayout;
  pRightLayout->addWidget( new QLabel( "Selected Tests" ) );
  QListView *pSelected = new QListView;
  pSelected->setViewMode( QListView::ListMode );
  pSelected->setModel( m_pSelectedTests );
  pRightLayout->addWidget( pSelected );
  connect( pSelected, SIGNAL( clicked( const QModelIndex& ) ), SLOT( UnselectTest( const QModelIndex& ) ) );
  pSelected->setFixedHeight( 600 );
  pListLayout->addLayout( pRightLayout );
  pMainLayout->addLayout( pListLayout );
  QHBoxLayout *pQuizNameLayout = new QHBoxLayout;
  pQuizNameLayout->addWidget( new QLabel( "Name of Quiz" ) );
  pQuizNameLayout->addWidget( m_pQuizName );
  connect( m_pQuizName, SIGNAL( textChanged( const QString & ) ), SLOT( TestQuizName( const QString & ) ) );
  pMainLayout->addLayout( pQuizNameLayout );
  QHBoxLayout *pButtonsLayout = new QHBoxLayout;
  m_pCreateBank->setEnabled( false );
  connect( m_pCreateBank, SIGNAL( clicked() ), SLOT( CreateBank() ) );
  pButtonsLayout->addWidget( m_pCreateBank );
  QPushButton *pCloseButton = new QPushButton( "Close" );
  connect( pCloseButton, SIGNAL( clicked() ), SLOT( accept() ) );
  pButtonsLayout->addWidget( pCloseButton );
  pMainLayout->addLayout( pButtonsLayout );
  setLayout( pMainLayout );
  }

void CreateMoodleBank::SelectTest( const QModelIndex& Index)
  {
  QStandardItem *pAvItem = m_pAvailableTests->itemFromIndex( Index );
  QStandardItem *pItem = new QStandardItem;
  pItem->setData( pAvItem->data( Qt::DisplayRole ), Qt::DisplayRole );
  pItem->setData( pAvItem->data( Qt::UserRole ), Qt::UserRole );
  m_pSelectedTests->appendRow( pItem );
  m_pAvailableTests->removeRow( Index.row() ); 
  m_pSelectedTests->sort( 0 );
  m_pCreateBank->setEnabled( !m_pQuizName->text().isEmpty() );
  }

void CreateMoodleBank::UnselectTest( const QModelIndex& Index )
  {
  QStandardItem *pSelItem = m_pSelectedTests->itemFromIndex( Index );
  QStandardItem *pItem = new QStandardItem;
  pItem->setData( pSelItem->data( Qt::DisplayRole ), Qt::DisplayRole );
  pItem->setData( pSelItem->data( Qt::UserRole ), Qt::UserRole );
  m_pAvailableTests->appendRow( pItem );
  m_pSelectedTests->removeRow( Index.row() );
  m_pAvailableTests->sort( 0 );
  m_pCreateBank->setEnabled( m_pSelectedTests->rowCount() > 0 && !m_pQuizName->text().isEmpty() );
  }

void CreateMoodleBank::CreateBank()
  {
  QString FileName = QFileDialog::getSaveFileName( nullptr, "Create Moodle bank file",
    m_pQuizName->text() + ".xml", "XML file (*.xml)" );
  if( FileName.isEmpty() ) return;
  QByteArray Selected;
  for( int i = 0; i < m_pSelectedTests->rowCount(); i++ )
    {
    QStandardItem *pSelItem = m_pSelectedTests->item( i, 0 );
    if( !Selected.isEmpty() ) Selected += ';';
    Selected += pSelItem->data( Qt::UserRole ).toByteArray();
    }
  Connector C( PasswordDialog::sm_RootUrl + "SelectChapters.php", "List=" + Selected );
  QByteArrayList Response = C.Connect().split( '#' );
  QFile file( FileName );
  if( !file.open( QIODevice::WriteOnly ) )
    {
    QMessageBox::critical( nullptr, "Error", "Can't create file " + FileName );
    return;
    }
  QTextStream Stream( &file );
  Stream.setCodec( "UTF-8" );
  Stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<quiz>\n  <question type=\"category\">\n    <category>\n";
  Stream << "        <text>$course$/" << m_pQuizName->text() << "</text>\n    </category>\n  </question>\n";
  for( int i = 0; i < Response.count(); i++ )
    {
    QByteArray R( Response[i] );
    int Pos = R.indexOf( ';' );
    Stream << "  <question type=\"halomdatesting\">\n    <name>\n      <text>" << ToLang( R.mid( Pos + 1 ) );
    Stream << "</text>\n    </name>\n    <questiontext format=\"html\">\n      <text><![CDATA[<p>";
    Stream << R.left( Pos ) << ";heb</p>]]></text>\n    </questiontext>\n    <generalfeedback format=\"html\">\n";
    Stream << "      <text></text>\n    </generalfeedback>\n    <defaultgrade>100.0000000</defaultgrade>\n";
    Stream << "    <penalty>0.0000000</penalty>\n    <hidden>0</hidden>\n  </question>\n";
    }
  Stream << "</quiz>\n";
  file.close();
  }

void CreateMoodleBank::TestQuizName( const QString &NewText )
  {
  m_pCreateBank->setEnabled( m_pSelectedTests->rowCount() > 0 && !NewText.isEmpty() );
  }
