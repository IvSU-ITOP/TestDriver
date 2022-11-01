#include "SelectTask.h"
#include "WinTesting.h"
#include "../FormulaPainter/InEdit.h"
#include <QNetworkRequest>
#include <QAxObject>

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

PasswordDialog::PasswordDialog() : m_pPassInput( new QLineEdit( this ) ), m_pRootUrl( nullptr ),
  m_pPassError(new QLabel("Invalid Password")), m_pPassAccept(new QPushButton("Accept")), m_LokalWork(false)
  {
  setWindowTitle( "Input of Password" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  m_pPassInput->setMaxLength( 11 );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel;
#if WorkServer == MyServer
  pLabel = new QLabel( "Root Url:" );
  pMainLayout->addWidget( pLabel );
  m_pRootUrl = new QLineEdit( sm_RootUrl, this );
  pMainLayout->addWidget( m_pRootUrl );
#endif
  pLabel = new QLabel( "Type Student password for testing or \nLecturer password for load students group:" );
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
#if WorkServer == MyServer
  sm_RootUrl = m_pRootUrl->text().toLocal8Bit();
  if( sm_RootUrl.right( 1 ) != "/" ) sm_RootUrl += '/';
#elif WorkServer == HalomdaCom
  sm_RootUrl = "https://halomda.com/TestingDriverMy/";
#else
  sm_RootUrl = "https://halomda.org/TestingDriverMy/";
#endif
  Connector C( sm_RootUrl + "TestStudentForCPP.php", "Password=" + ( m_pPassInput->text() ).toLocal8Bit() );
  m_UsrId = C.Connect();
  if( m_UsrId.isEmpty() ) throw QString( "Password testing error");
  if( m_UsrId == "Error" )
    {
    m_pPassError->show();
    return;
    }
  int LoginPos = m_UsrId.indexOf(';');
  if( LoginPos == -1 ) throw QString( "Password testing error:" + m_UsrId );
  m_UsrLogin = ToLang(m_UsrId.mid(LoginPos + 1));
  m_UsrId = m_UsrId.left(LoginPos);
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
  Connector C( PasswordDialog::sm_RootUrl + "SelectTest.php", "Usr_id=" + UserCode );
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

SelectGroup::SelectGroup( const QByteArray& UserCode, const QString& LectLogin ) :
  m_pListGroups( new QComboBox ), m_LectLogin(LectLogin)
  {
  setWindowTitle( "Selection of Group" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  Connector C( PasswordDialog::sm_RootUrl + "SelectGroup.php", "UsrId=" + UserCode );
  QByteArray Response = C.Connect();
  if( Response == "No Group" ) throw QString( "Sorry, there is no group available for you!" );
  QByteArrayList GroupList( Response.split( '#' ) );
  if( GroupList.isEmpty() ) throw QString( "Group List error" );
  QByteArray Group;
  foreach( Group, GroupList )
    {
    QByteArrayList GroupParms( Group.split( '&' ) );
    if( GroupParms.count() != 2 ) throw QString( "Group List error: " + Group );
    m_pListGroups->addItem( ToLang( GroupParms[0] ), GroupParms[1] );
    }
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel( "Select an Existing Group or Type New Group Name" );
  pMainLayout->addWidget( pLabel );
  QLabel *pGroupLabel = new QLabel( "Existing Groups:" );
  pMainLayout->addWidget( pGroupLabel );
  pMainLayout->addWidget( m_pListGroups );
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  QPushButton *pNewGroup = new QPushButton( "New Group" );
  connect( pNewGroup, SIGNAL( clicked() ), SLOT( NewGroup() ) );
  pButtonLayout->addWidget(pNewGroup);
  QPushButton *pSelect = new QPushButton( "OK" );
  pButtonLayout->addWidget( pSelect );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  pButtonLayout->addWidget( pCancel );
  pMainLayout->addLayout( pButtonLayout );
  setLayout( pMainLayout );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  connect( pSelect, SIGNAL( clicked() ), SLOT( accept() ) );
  }

void SelectGroup::NewGroup()
  {
  class NewGroup NG(&m_NewGroup, m_LectLogin);
  NG.exec();
  if(!m_NewGroup.isEmpty()) accept();
  }

NewGroup::NewGroup(QString* pGroupId, const QString& LectName) : m_pNewGroup(pGroupId), m_LectName(LectName)
  {
  setWindowTitle( "Group Id Edition" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel( "Lecturer: " + LectName );
  pMainLayout->addWidget( pLabel );

  QHBoxLayout *pGroupNameLayout = new QHBoxLayout;
  pGroupNameLayout->addWidget(new QLabel("Group:"));
  m_pGroupName = new QLineEdit();
  pGroupNameLayout->addWidget( m_pGroupName );
  connect( m_pGroupName, SIGNAL( textChanged(const QString &) ), SLOT( ChangeValue() ) );
  pMainLayout->addLayout(pGroupNameLayout);

  QHBoxLayout *pInstitutionLayout = new QHBoxLayout;
  pInstitutionLayout->addWidget(new QLabel("Institution:"));
  m_pInstitution = new QLineEdit();
  pInstitutionLayout->addWidget( m_pInstitution );
  connect( m_pInstitution, SIGNAL( textChanged(const QString &) ), SLOT( ChangeValue() ) );
  pMainLayout->addLayout(pInstitutionLayout);

  QHBoxLayout *pCityLayout = new QHBoxLayout;
  pCityLayout->addWidget(new QLabel("City:"));
  m_pCity = new QLineEdit();
  pCityLayout->addWidget( m_pCity );
  connect( m_pCity, SIGNAL( textChanged(const QString &) ), SLOT( ChangeValue() ) );
  pMainLayout->addLayout(pCityLayout);

  QHBoxLayout *pSubjectLayout = new QHBoxLayout;
  pSubjectLayout->addWidget(new QLabel("Subject:"));
  m_pSubject = new QLineEdit();
  pSubjectLayout->addWidget( m_pSubject );
  connect( m_pSubject, SIGNAL( textChanged(const QString &) ), SLOT( ChangeValue() ) );
  pMainLayout->addLayout(pSubjectLayout);

  QHBoxLayout *pYearTermLayout = new QHBoxLayout;
  pYearTermLayout->addWidget(new QLabel("Year/term:"));
  m_pYearTerm = new QLineEdit();
  pYearTermLayout->addWidget( m_pYearTerm );
  connect( m_pYearTerm, SIGNAL( textChanged(const QString &) ), SLOT( ChangeValue() ) );
  pMainLayout->addLayout(pYearTermLayout);

  QHBoxLayout *pButtonsLayout = new QHBoxLayout;
  m_pOK = new QPushButton("OK");
  m_pOK->setEnabled(false);
  pButtonsLayout->addWidget( m_pOK );
  connect( m_pOK, SIGNAL( clicked() ), SLOT( SetValue() ) );
  QPushButton *pCancel = new QPushButton("Cancel");
  pButtonsLayout->addWidget( pCancel );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pMainLayout->addLayout(pButtonsLayout);
  setLayout( pMainLayout );
  }

void NewGroup::ChangeValue()
  {
  m_pOK->setEnabled( !m_pGroupName->text().isEmpty() &&
    !m_pInstitution->text().isEmpty() && !m_pCity->text().isEmpty() &&
    !m_pSubject->text().isEmpty() && !m_pYearTerm->text().isEmpty() );
  }

void NewGroup::SetValue()
  {
  *m_pNewGroup = m_LectName + ";" + m_pGroupName->text() + ";" + m_pInstitution->text() + ";" +
    m_pCity->text() + ";" + m_pSubject->text() + ";" + m_pYearTerm->text();
  accept();
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

WaitExcel::WaitExcel() : QDialog( nullptr, Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint ),
  m_pLabel( new QLabel ), m_pProgressLabel( new QLabel( SetProgress( 0, 0 ) ) ), m_pProgressBar( new QProgressBar( this ) ), m_Count( 0 )
  {
  setWindowTitle( "Recording Results" );
  m_pProgressBar->setTextVisible( false );
  m_pProgressBar->hide();
  m_pLabel->hide();
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  pMainLayout->addWidget( m_pLabel );
  pMainLayout->addWidget( m_pProgressLabel );
  pMainLayout->addWidget( m_pProgressBar );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  connect( pCancel, SIGNAL( clicked() ), SLOT( Cancel() ) );
  pMainLayout->addWidget( pCancel );
  setLayout( pMainLayout );
  }

void WaitExcel::Cancel()
  {
  throw "Cancel wait!";
  }

void WaitExcel::StepProgress()
  {
  m_pProgressBar->setValue( m_Count );
  m_pProgressLabel->setText( SetProgress( m_Count++, m_pProgressBar->maximum() ) );
  }

void WaitExcel::SetMax( const QString &Label, int UserCount )
  {
  m_pLabel->setText( Label );
  m_pProgressBar->setRange( 0, UserCount );
  m_pProgressBar->show();
  m_pLabel->show();
  StepProgress();
  }

SelectWork::SelectWork(QString &GroupName)
  {
  setMinimumSize(500, 350);
  setWindowTitle( "Selection work type" );
  SetForegroundWindow( ( HWND ) winId() );
  setWindowFlags( Qt::WindowStaysOnTopHint | Qt::Drawer );
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  QLabel *pLabel = new QLabel(QString("For this group You can:\n\rAdd student\n\r") +
    "Or load students list\n\rOr get work results;\n\rBesides You can correct group name");
  pMainLayout->addWidget( pLabel );
  QLabel *pGroup = new QLabel(QString("Group:"));
  pMainLayout->addWidget( pGroup );
  m_pGroupName = new QLineEdit(GroupName);
  pMainLayout->addWidget( m_pGroupName );
  m_pLoadList = new QCheckBox("Load students list");
  pMainLayout->addWidget( m_pLoadList );
  connect( m_pLoadList, SIGNAL( stateChanged(int) ), SLOT( LoadListChanged(int) ) );
  m_pLoadResult = new QCheckBox("Load test results");
  pMainLayout->addWidget( m_pLoadResult );
  connect( m_pLoadResult, SIGNAL( stateChanged(int) ), SLOT( LoadResultChanged(int) ) );
  QLabel *pFirstName = new QLabel(QString("Student First Name:"));
  pMainLayout->addWidget( pFirstName );
  m_pFirstName = new QLineEdit();
  pMainLayout->addWidget( m_pFirstName );
  connect( m_pFirstName, SIGNAL( textChanged(const QString&) ), SLOT( FirstNameChanged(const QString&) ) );
  QLabel *pLastName = new QLabel(QString("Student Last Name:"));
  pMainLayout->addWidget( pLastName );
  m_pLastName = new QLineEdit();
  pMainLayout->addWidget( m_pLastName );
  connect( m_pLastName, SIGNAL( textChanged(const QString&) ), SLOT( LastNameChanged(const QString&) ) );
  QLabel *pPassword = new QLabel(QString("Student Password:"));
  pMainLayout->addWidget( pPassword );
  m_pPassword = new QLineEdit();
  pMainLayout->addWidget( m_pPassword );
  connect( m_pPassword, SIGNAL( textChanged(const QString&) ), SLOT( PassChanged(const QString&) ) );
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  m_pOK = new QPushButton( "OK" );
  pButtonLayout->addWidget( m_pOK );
  connect( m_pOK, SIGNAL( clicked() ), SLOT( accept() ) );
  QPushButton *pCancel = new QPushButton( "Cancel" );
  connect( pCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pButtonLayout->addWidget( pCancel );
  pMainLayout->addLayout( pButtonLayout );
  setLayout( pMainLayout );
  }

void SelectWork::SetNameEnabled(bool Enable)
  {
  m_pFirstName->setEnabled(Enable);
  m_pLastName->setEnabled(Enable);
  m_pPassword->setEnabled(Enable);
  }

void SelectWork::SetFlagsEnabled(bool Enable)
  {
  if( Enable )
    {
    if(m_pFirstName->text().isEmpty() && m_pLastName->text().isEmpty() && m_pPassword->text().isEmpty())
      {
      m_pOK->setEnabled(true);
      m_pLoadList->setEnabled(true);
      m_pLoadResult->setEnabled(true);
      return;
      }
    m_pOK->setEnabled(false);
    }
  m_pLoadList->setEnabled(false);
  m_pLoadResult->setEnabled(false);
  m_pOK->setEnabled(!m_pFirstName->text().isEmpty() && !m_pLastName->text().isEmpty() && !m_pPassword->text().isEmpty());
  }

void SelectWork::PassChanged( const QString& NewPass)
  {
  SetFlagsEnabled(NewPass.isEmpty());
  }

void SelectWork::FirstNameChanged( const QString& newFM)
  {
  SetFlagsEnabled(newFM.isEmpty());
  }

void SelectWork::LastNameChanged( const QString& newLM)
  {
  SetFlagsEnabled(newLM.isEmpty());
  }

void SelectWork::LoadListChanged( int V )
  {
  bool Enable = V == 0;
  SetNameEnabled(Enable);
  m_pLoadResult->setEnabled(Enable);
  }

void SelectWork::LoadResultChanged( int V )
  {
  bool Enable = V == 0;
  SetNameEnabled(Enable);
  m_pLoadList->setEnabled(Enable);
  }

int SelectWork::LoadResult(int iGroupCode)
  {
  QAxObject *pExcel = nullptr;
  QAxObject *pWorkbooks = nullptr;

  auto RetCode = [&] ( const QString& Msg, bool Error )
    {
    if( pWorkbooks != nullptr ) pWorkbooks->dynamicCall( "Close" );
    delete pExcel;
    if( Msg.isEmpty() ) return 1;
    if( Error )
      QMessageBox::critical( nullptr, "Error", Msg );
    else
      QMessageBox::information( nullptr, "OK", Msg );
    return -1;
    };

  pExcel = new  QAxObject( "Excel.Application" );
  if( pExcel == NULL ) return RetCode( "Excel was not installed", true );
  pExcel->setProperty( "Visible", true );

  auto GetValue = [] ( QByteArray &Result )
    {
    QByteArray sResult;
    int iPos = Result.indexOf( ';' );
    if( iPos == -1 )
      {
      sResult = Result;
      Result = "";
      return sResult;
      }
    sResult = Result.left( iPos );
    Result = Result.right( Result.length() - iPos - 1 );
    return sResult;
    };
  QString Commonresults = "פילוג התוצאות";
  QString GroupId = "ציון ממוצע";
  QString MeanResult = "מספר קבוצה";
  QString Profile = "תוצאות כלליות";
  QString IdentificationCard = "ניסיונות";
  QString Chapter = "שם המבחן";
  QString Result = "תוצאת המבחן";
  QString Totalattempts = "מספר ניסיונות";
  QString Resultstestof = "ציון";
  QString TaskName = "פרק";
  QString Attempts = "תעודת זהות";
/*
  QString Commonresults = "Commonresults";
  QString GroupId = "GroupId";
  QString MeanResult = "MeanResult";
  QString Profile = "Profile";
  QString IdentificationCard = "IdentificationCard";
  QString Chapter = "Chapter";
  QString Result = "Result";
  QString Totalattempts = "Totalattempts";
  QString Resultstestof = "Resultstestof";
  QString TaskName = "TaskName";
  QString Attempts = "Attempts";
*/
  pExcel->setProperty( "Visible", true );
  WaitExcel EWait;
  EWait.show();
  pWorkbooks = pExcel->querySubObject( "WorkBooks" );
  pWorkbooks->dynamicCall( "Add" );
  QAxObject *pWorkbook = pExcel->querySubObject( "ActiveWorkBook" );
  QAxObject *pSheets = pWorkbook->querySubObject( "WorkSheets" );
  QAxObject *pWorkSheet = pExcel->querySubObject( "ActiveSheet" );
  pWorkSheet->setProperty( "Name", ( Commonresults ) );
  QByteArray sGroupCode = QByteArray::number( iGroupCode );
  Connector C( PasswordDialog::sm_RootUrl + "GetExcelResults.php", "WorkCode=0 &GroupCode="+ sGroupCode );
  QByteArray sResult = C.Connect();
  if( sResult == "-100" || sResult.isEmpty() ) return RetCode( "Error by GroopCode sending, site error!", true );
  pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", 1, 1 )->setProperty( "Value", GroupId );
  int iRecordCount = 0;
  for( int iPos = 0; ( iPos = sResult.indexOf( ';', iPos ) ) != -1; iPos++, iRecordCount++ );
  if( iRecordCount == 0 ) return RetCode( "List of Students was empty", false );
  QString sGroupName = ToLang( GetValue( sResult ) );
  pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", 1, 2 )->setProperty( "Value", sGroupName );

  QAxObject *pWorkSheetFinal = pSheets->querySubObject( "Item(QVariant)", 2 );
  if( pWorkSheetFinal == nullptr )
    {
    pSheets->dynamicCall( "Add(QVariant)", pWorkSheet->asVariant() );
    pWorkSheetFinal = pExcel->querySubObject( "ActiveSheet" );
    pWorkSheet->dynamicCall( "Move(QVariant)", pWorkSheetFinal->asVariant() );
    pWorkSheetFinal->dynamicCall( "Activate" );
    }
  pWorkSheetFinal->setProperty( "Name", "Final Results" );
  int iRowFin = 1;

  int iStudentCount = iRecordCount / 3;
  EWait.SetMax( sGroupName, iStudentCount );
  int iRow = 3;
  int iStudentSheet = 2;
  QAxObject *pStudentSheet = pWorkSheet;
  QVector<int> vStudentCounts;
  for( int iInterval = 0; iInterval < 100; iInterval += 10 )
    vStudentCounts.push_back( 0 );
  do
    {
    EWait.StepProgress();
    QString sName = ToLang( GetValue( sResult ) );
    QString IdCard = ToLang( GetValue( sResult ) );
    if( sName.isEmpty() ) return RetCode( "Name of Student was empty", true );
    pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 1 )->setProperty( "Value", sName );
    pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 2 )->setProperty( "Value", "\'" + IdCard );
    pWorkSheetFinal->querySubObject( "Cells(QVariant,QVariant)", iRowFin, 1 )->setProperty( "Value", sName );
    pWorkSheetFinal->querySubObject( "Cells(QVariant,QVariant)", iRowFin, 2 )->setProperty( "Value", "\'" + IdCard );
    QByteArray sStudCode = GetValue( sResult );
    if( sStudCode.isEmpty() ) return RetCode( "Code of Student was empty", true);
    pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", ++iRow, 1 )->setProperty( "Value", Chapter );
    pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 2 )->setProperty( "Value", Result );
    pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 3 )->setProperty( "Value", Totalattempts );
    if( ++iStudentSheet <= pSheets->property( "Count" ).toInt() )
      pStudentSheet = pSheets->querySubObject( "Item(QVariant)", iStudentSheet );
    else
      {
      QAxObject *pOldSheet = pStudentSheet;
      pSheets->dynamicCall( "Add(QVariant)", pStudentSheet->asVariant() );
      pStudentSheet = pExcel->querySubObject( "ActiveSheet" );
      pOldSheet->dynamicCall( "Move(QVariant)", pStudentSheet->asVariant() );
      pWorkSheet->dynamicCall( "Activate" );
      }
    pStudentSheet->setProperty( "Name", sName );
    pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 1, 4 )->setProperty( "Value", Resultstestof );
    pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 1, 5 )->setProperty( "Value", sName );
    int iStudentCol = 1;
    Connector C( PasswordDialog::sm_RootUrl + "GetExcelResults.php", "WorkCode=1&GroupCode="+ sGroupCode + "&StudCode=" + sStudCode);
    QByteArray sRow = C.Connect();
    if( sRow == "-100" ) return RetCode( "Error of GetExcelResults.php", true );
    if( sRow == "" ) continue;
    int iSummResult = 0;
    int iTotalCount = 0;
    do
      {
      QByteArray sTotalId = GetValue( sRow );
      if( sTotalId.isEmpty() ) return RetCode( "sTotalId was empty", true );
      QString sTotalResult = GetValue( sRow );
      if( sTotalResult.isEmpty() ) return RetCode( "sTotalResult was empty", true );
      QString sTotalAttempts = GetValue( sRow );
      if( sTotalAttempts.isEmpty() ) return RetCode( "sTotalAttempts was empty", true );
      QString sChapter = ToLang( GetValue( sRow ) );
      if( sChapter.isEmpty() ) return RetCode( "sChapter was empty", true );
      iTotalCount++;
      int iTotalResult = sTotalResult.toInt();
      iSummResult += iTotalResult;
      pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", ++iRow, 1 )->setProperty( "Value", sChapter );
      pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 2 )->setProperty( "Value", iTotalResult );
      pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 3 )->setProperty( "Value", sTotalAttempts.toInt() );
      pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 3, iStudentCol )->setProperty( "Value", Chapter );
      pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 3, iStudentCol + 1 )->setProperty( "Value", sChapter );
      pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 4, iStudentCol )->setProperty( "Value", Result );
      pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 4, iStudentCol + 1 )->setProperty( "Value", iTotalResult );
      pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 5, iStudentCol )->setProperty( "Value", Totalattempts );
      pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 5, iStudentCol + 1 )->setProperty( "Value", sTotalAttempts.toInt() );
      Connector C( PasswordDialog::sm_RootUrl + "GetExcelResults.php", "WorkCode=2&TotalResult="+ sTotalId);
      QByteArray sResults = C.Connect();
      if( sResults == "-100" || sResults.isEmpty() ) return RetCode( "sResults was empty", true );
      if( sResults != "0" )
        {
        pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 7, iStudentCol )->setProperty( "Value", TaskName );
        pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 7, iStudentCol + 1 )->setProperty( "Value", Result );
        pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", 7, iStudentCol + 2 )->setProperty( "Value", Attempts );
        int iResultRow = 7;
        do
          {
          QString sTask = ToLang( GetValue( sResults ) );
          if( sTask.isEmpty() ) return RetCode( "sTask was empty", true );
          QString sTaskResult = GetValue( sResults );
          if( sTaskResult.isEmpty() ) return RetCode( "sTaskResult was empty", true );
          QString sTaskAttempts = GetValue( sResults );
          if( sTaskAttempts.isEmpty() ) return RetCode( "sTaskAttempts was empty", true );
          pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", ++iResultRow, iStudentCol )->setProperty( "Value", sTask );
          pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", iResultRow, iStudentCol + 1 )->setProperty( "Value", sTaskResult.toInt() );
          pStudentSheet->querySubObject( "Cells(QVariant,QVariant)", iResultRow, iStudentCol + 2 )->setProperty( "Value", sTaskAttempts.toInt() );
          } while( !sResults.isEmpty() );
        }
      iStudentCol += 4;
      } while( !sRow.isEmpty() );
      if( iTotalCount == 0 ) iTotalCount = 1;
      double dMeanResult = ( double ) iSummResult / iTotalCount;
      if( dMeanResult < 80.0 )
        if( dMeanResult > 75.0 )
          dMeanResult = 80.0;
        else
          dMeanResult += 5.0;
      double dResult;
      int iResult;
      for( iResult = 9, dResult = 90; iResult > 0 && dMeanResult < dResult; iResult--, dResult -= 10 );
      vStudentCounts[iResult]++;
      pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", ++iRow, 1 )->setProperty( "Value", MeanResult );
      QString sMeanResult = QString::number( dMeanResult, 'f', 2 );
      pWorkSheet->querySubObject( "Cells(QVariant,QVariant)", iRow, 2 )->setProperty( "Value", sMeanResult.toDouble() );
      pWorkSheetFinal->querySubObject( "Cells(QVariant,QVariant)", iRowFin++, 3 )->setProperty( "Value", sMeanResult.toDouble() );
      iRow += 2;
    } while( !sResult.isEmpty() );
    pSheets->dynamicCall( "Add(QVariant)", pWorkSheet->asVariant() );
    QAxObject *pDiagramSheet = pExcel->querySubObject( "ActiveSheet" );
    pWorkSheet->dynamicCall( "Move(QVariant)", pDiagramSheet->asVariant() );
    pDiagramSheet->setProperty( "Name", Profile );
    pDiagramSheet->querySubObject( "Cells(QVariant,QVariant)", 1, 1 )->setProperty( "Value", Profile );
    for( int iTotalRow = 0; iTotalRow < 10; iTotalRow++ )
      {
      pDiagramSheet->querySubObject( "Cells(QVariant,QVariant)", iTotalRow + 2, 1 )->setProperty( "Value",
        '\'' + QString::number( iTotalRow * 10 ) + '-' + QString::number( iTotalRow * 10 + 10 ) );
      pDiagramSheet->querySubObject( "Cells(QVariant,QVariant)", iTotalRow + 2, 2 )->setProperty( "Value", vStudentCounts[iTotalRow] );
      }
    QAxObject *pCharts = pWorkbook->querySubObject( "Charts" );
    pCharts->dynamicCall( "Add" );
    QAxObject *pChart = pCharts->querySubObject( "Item(QVariant&)", 1 );
    pChart->setProperty( "HasLegend", false );
    pChart->setProperty( "ChartType", xl3DColumnClustered );
    QAxObject *pCellsLT = pDiagramSheet->querySubObject( "Cells(QVariant,QVariant)", 2, 1 );
    QAxObject *pCellsRU = pDiagramSheet->querySubObject( "Cells(QVariant,QVariant)", 11, 2 );
    QAxObject *pRange = pDiagramSheet->querySubObject( "Range(QVariant,QVariant)", pCellsLT->asVariant(), pCellsRU->asVariant() );
    pChart->dynamicCall( "SetSourceData(IDispatch*, QVariant)", pRange->asVariant(), ( int ) xlColumns );
    pChart->dynamicCall( "Location( QVariant, QVariant )", ( int ) xlLocationAsObject, Profile );
    pChart = pWorkbook->querySubObject( "ActiveChart" );
    QAxObject *pAxis = pChart->querySubObject( "Axes(QVariant, QVariant)", ( int ) xlCategory, ( int ) xlPrimary );
    pAxis->setProperty( "TickLabelSpacing", 1 );
    pWorkSheet->dynamicCall( "Activate" );
    EWait.reject();
    return RetCode( "", false );
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
