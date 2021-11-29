#include "Yclasses.h"
#include "InEdit.h"
#include "Messtrs.h"
#include <qpainter.h>
#include <qdebug.h>

void Region::FloodFill()
  {
  int xStart = m_Start.x();
  int y = m_Start.y();
  if( GetColor( xStart, y ) != m_OldColor ) return;
  int x0 = -1;
  int xMax = -1;
  int x0First = -1;
  int xMaxFirst = -1;
  for( ; y >= top() && GetColor( xStart, y ) == m_OldColor; y-- )
    {
    int x = xStart;
    for( ; x >= left() && GetColor( x, y ) == m_OldColor; x-- )
      SetColor( x, y );
    if( x < x0 )
      {
      int yl = y + 1;
      int xr = x + 1;
      for( xr; xr < x0 && GetColor( xr, yl ) != m_OldColor; xr++ );
      if( xr != x0 )
        {
//        qDebug() << "en1 " << xr << yl;
        m_pQueue->enqueue( Region( QPoint( xr, yl ), *this ) );
        }
      }
    else
      if( x > x0 && x0 != -1 )
        {
        int xr = x - 1;
        for( xr--; xr > x0 && GetColor( xr, y ) != m_OldColor; xr-- );
        if( xr != x0 )
          {
//          qDebug() << "en2 " << xr << y;
          m_pQueue->enqueue( Region( QPoint( xr, y ), *this ) );
          }
        }
    x0 = x;
    if( x0First == -1 ) x0First = x0;;
    for( x = xStart + 1; x <= right() && GetColor( x, y ) == m_OldColor; x++ )
      SetColor( x, y );
    if( x < xMax )
      {
      int xr = x + 1;
      for( xr; xr < xMax && GetColor( xr, y ) != m_OldColor; xr++ );
      if( xr != xMax )
        {
//        qDebug() << "en3 " << xr << y;
        m_pQueue->enqueue( Region( QPoint( xr, y ), *this ) );
        }
      }
    else
      if( x > xMax && xMax != -1 )
        {
        int yl = y + 1;
        int xr = x - 1;
        for( xr--; xr > xMax && GetColor( xr, yl ) != m_OldColor; xr-- );
        if( xr != xMax )
          {
//          qDebug() << "en4 " << xr << yl;
          m_pQueue->enqueue( Region( QPoint( xr, yl ), *this ) );
          }
        }
    xMax = x;
    if( xMaxFirst == -1 ) xMaxFirst  = xMax;
    }
  if( y > top() )
    for( int x = x0 + 1; x < xMax; x++ )
      if( GetColor( x, y ) == m_OldColor && GetColor( x - 1, y ) != m_OldColor )
        {
 //       qDebug() << "en5 " << x << y;
        m_pQueue->enqueue( Region( QPoint( x, y ), *this ) );
        }
  x0 = x0First;
  xMax = xMaxFirst;
  for( y = m_Start.y() + 1; y <= bottom() && GetColor( m_Start.x(), y ) == m_OldColor; y++ )
    {
    int x = xStart;
    for( ; x >= left() && GetColor( x, y ) == m_OldColor; x-- )
      SetColor( x, y );
    if( x < x0 )
      {
      int xr = x - 1;
      for( xr--; xr > x0 && GetColor( xr, y ) != m_OldColor; xr-- );
      if( xr != x0 )
        {
//        qDebug() << "en6 " << xr << y;
        m_pQueue->enqueue( Region( QPoint( xr, y ), *this ) );
        }
      }
    else
      if( x > x0 )
        {
        int yu = y - 1;
        int xr = x + 1;
        for( xr; xr < x0 && GetColor( xr, yu ) != m_OldColor; xr++ );
        if( xr != x0 )
          {
//          qDebug() << "en7 " << xr << yu;
          m_pQueue->enqueue( Region( QPoint( xr, yu ), *this ) );
          }
        }
    x0 = x;
    for( x = xStart + 1; x <= right() && GetColor( x, y ) == m_OldColor; x++ )
      SetColor( x, y );
    if( x < xMax )
      {
      int xr = x + 1;
      for( xr; xr < xMax && GetColor( xr, y ) != m_OldColor; xr++ );
      if( xr != xMax )
        {
//        qDebug() << "en8 " << xr << y;
        m_pQueue->enqueue( Region( QPoint( xr, y ), *this ) );
        }
      }
    else
      if( x > xMax )
        {
        int yl = y - 1;
        int xr = x - 1;
        for( xr--; xr > xMax && GetColor( xr, yl ) != m_OldColor; xr-- );
        if( xr != xMax )
          {
          qDebug() << "en9 " << xr << yl;
          m_pQueue->enqueue( Region( QPoint( xr, yl ), *this ) );
          }
        }
    xMax = x;
    }
  if( y < bottom() )
    for( int x = x0 + 1; x < xMax; x++ )
      if( GetColor( x, y ) == m_OldColor && GetColor( x - 1, y ) != m_OldColor )
        {
//        qDebug() << "en10 " << x << y;
        m_pQueue->enqueue( Region( QPoint( x, y ), *this ) );
        }
  }

void FloodFill( const QRect& Rect, QImage *pImage, QPoint Start, QColor NewColor )
  {
  if( Start.x() == -1 ) Start = Rect.topLeft();
  if( !Rect.contains( Start ) || pImage->pixelColor( Start ) == NewColor ) return;
  RegionQueue Queue;
  Queue.enqueue( Region( Rect, Start, pImage->pixelColor( Start ), NewColor, pImage, &Queue ) );
  while( !Queue.isEmpty() )
    Queue.dequeue().FloodFill();
  }

CreateChartDialog::CreateChartDialog( EdChartEditor *pEditor ) : QDialog( nullptr, Qt::WindowSystemMenuHint ),
m_pEditor( pEditor ), m_pColumnNumber( new QSpinBox ), m_WasChanged( false ), m_pScale( new QDoubleSpinBox ),
m_pType( new QLabel ), m_Type( pEditor->m_pTemplChart->m_Type )
  {
  setWindowTitle( X_Str( "SelfCreateCharCaption", "Creating of Chart" ) );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  QHBoxLayout *pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( X_Str( "Label2CreateCharCaption", "Number of column" ) ) );
  m_pColumnNumber->setMinimum( 2 );
  m_pColumnNumber->setValue( pEditor->m_pTemplChart->m_Labels.count() );
  pHBox->addWidget( m_pColumnNumber );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( X_Str( "ScaleCreateTabCaption", "Scale" ) ) );
  m_pScale->setRange( 0.1, 10.0 );
  m_pScale->setSingleStep( 0.1 );
  m_pScale->setValue( pEditor->m_pTemplChart->m_Scale );
  pHBox->addWidget( m_pScale );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( X_Str( "Label1CreateCharCaption", "Type of chart" ) ) );
  m_pType->setPixmap( QPixmap( m_Type == ThinCol ? ":/Resources/ThinChart.png" : ":/Resources/ThickChart.png" ) );
  pHBox->addWidget( m_pType );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  QPushButton *cmdThinCol = new QPushButton( QIcon( ":/Resources/ThinChart.png" ), "" );
  connect( cmdThinCol, SIGNAL( clicked() ), SLOT( SetThin() ) );
  pHBox->addWidget( cmdThinCol );
  QPushButton *cmdThickCol = new QPushButton( QIcon( ":/Resources/ThickChart.png" ), "" );
  connect( cmdThickCol, SIGNAL( clicked() ), SLOT( SetThick() ) );
  pHBox->addWidget( cmdThickCol );
  pVBox->addLayout( pHBox );
  pHBox = new  QHBoxLayout;
  QPushButton *cmdOK = new QPushButton( X_Str( "btnOkCaption", "OK" ) );
  connect( cmdOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pHBox->addWidget( cmdOK );
  QPushButton *cmdCancel = new QPushButton( X_Str( "btnCancelCaption", "Cancel" ) );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHBox->addWidget( cmdCancel );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  }

void CreateChartDialog::accept()
  {
  EdChart &Chart = *m_pEditor->m_pTemplChart;
  if( m_pColumnNumber->value() != Chart.m_Labels.count() )
    {
    m_pEditor->SetColumnNumber( m_pColumnNumber->value() );
    m_WasChanged = true;
    }
  if( m_pScale->value() != Chart.m_Scale )
    {
    Chart.m_Scale = m_pScale->value();
    m_WasChanged = true;
    }
  if( m_Type != Chart.m_Type )
    {
    Chart.m_Type = m_Type;
    m_WasChanged = true;
    }
  QDialog::accept();
  }
  
void CreateChartDialog::SetThin()
  {
  m_pType->setPixmap( QPixmap( ":/Resources/ThinChart.png" ) );
  m_Type = ThinCol;
  }
  
void CreateChartDialog::SetThick()
  {
  m_pType->setPixmap( QPixmap( ":/Resources/ThickChart.png" ) );
  m_Type = ThickCol;
  }

CreateTableDialog::CreateTableDialog() : QDialog( nullptr, Qt::WindowSystemMenuHint ), m_pColumnNumber( new QSpinBox ),
m_pRowNumber( new QSpinBox ), 
m_pVisible( new QRadioButton( X_Str( "VisibleGrid", "Visible" ) ) ), 
m_pUnvisible( new QRadioButton( X_Str( "UnVisibleGrid", "UnVisible" ) ) ), 
m_pPartlyVisible( new QRadioButton( X_Str( "PartlyVisibleGrid", "Partly Visible" ) ) )
  {
  setWindowTitle( X_Str( "SelfCreateTabCaption", "Creating of Table" ) );
  QVBoxLayout *pVBox = new  QVBoxLayout;
  QHBoxLayout *pHBox = new  QHBoxLayout;
  pHBox->addWidget( new QLabel( X_Str( "Label2CreateTabCaption", "Count of colomns" ) ) );
  m_pColumnNumber->setMinimum( 2 );
  pHBox->addWidget( m_pColumnNumber );
  pHBox->addWidget( new QLabel( X_Str( "Label1CreateTabCaption", "Count of rows" ) ) );
  m_pRowNumber->setMinimum( 1 );
  pHBox->addWidget( m_pRowNumber );
  pVBox->addLayout( pHBox );
  m_pVisible->setChecked( true );
  QVBoxLayout *pGroupBoxLayout = new QVBoxLayout;
  pGroupBoxLayout->addWidget( m_pVisible );
  pGroupBoxLayout->addWidget( m_pUnvisible );
  pGroupBoxLayout->addWidget( m_pPartlyVisible );
  QGroupBox *pGroupBox = new QGroupBox( X_Str( "TypeOfGridCaption", "Type of grid" ) );
  pGroupBox->setLayout( pGroupBoxLayout );
  pVBox->addWidget( pGroupBox );
  pHBox = new  QHBoxLayout;
  QPushButton *cmdOK = new QPushButton( X_Str( "btnOkCaption", "OK" ) );
  connect( cmdOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pHBox->addWidget( cmdOK );
  QPushButton *cmdCancel = new QPushButton( X_Str( "btnCancelCaption", "Cancel" ) );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHBox->addWidget( cmdCancel );
  pVBox->addLayout( pHBox );
  setLayout( pVBox );
  }

QByteArray CreateTableDialog::GetResult()
  {
  int GridState = TGRVisible;
  if( m_pUnvisible->isChecked() )
    GridState = TGRUnvisible;
  else
    if( m_pPartlyVisible->isChecked() )
      GridState = TGRPartlyVisible;
  return QByteArray::number( m_pRowNumber->value() ) + ',' +
    QByteArray::number( m_pColumnNumber->value() ) + ',' + QByteArray::number( GridState ) + ",0,0";
  }

CreateMatrixDialog::CreateMatrixDialog() : QDialog(nullptr, Qt::WindowSystemMenuHint), m_pColumnNumber(new QSpinBox),
  m_pRowNumber(new QSpinBox), m_pMatrix(new QRadioButton(X_Str("MatrBtnHint", "Matrix"))),
  m_pDeterminant(new QRadioButton(X_Str("MDeterminantTask", "Determinant")))
  {
  setWindowTitle(X_Str("SelfCreateMatrixCaption", "Creating of Matrix or Determinant"));
  QVBoxLayout *pVBox = new  QVBoxLayout;
  QHBoxLayout *pGroupBoxLayout = new QHBoxLayout;
  m_pColumnNumber->setMinimum(1);
  m_pRowNumber->setMinimum(1);
  m_pColumnNumber->setValue(2);
  m_pRowNumber->setValue(2);
  pGroupBoxLayout->addWidget(m_pMatrix);
  m_pMatrix->setChecked(true);
  pGroupBoxLayout->addWidget(m_pDeterminant);
  pGroupBoxLayout->setAlignment(m_pDeterminant, Qt::AlignRight);
  connect(m_pDeterminant, SIGNAL(toggled(bool)), SLOT(ToggleDeterminant(bool)));
  QGroupBox *pGroupBox = new QGroupBox;
  pGroupBox->setLayout(pGroupBoxLayout);
  pVBox->addWidget(pGroupBox);
  QHBoxLayout *pHBox = new  QHBoxLayout;
  pHBox->addWidget(new QLabel(X_Str("Label2CreateTabCaption", "Count of colomns")));
  pHBox->addWidget(m_pColumnNumber);
  connect(m_pColumnNumber, SIGNAL(valueChanged(int)), SLOT(ChangeRowColNumber(int)));
  pHBox->addWidget(new QLabel(X_Str("Label1CreateTabCaption", "Count of rows")));
  pHBox->addWidget(m_pRowNumber);
  connect(m_pRowNumber, SIGNAL(valueChanged(int)), SLOT(ChangeRowColNumber(int)));
  QPushButton *cmdOK = new QPushButton(X_Str("btnOkCaption", "OK"));
  connect(cmdOK, SIGNAL(clicked()), SLOT(accept()));
  pHBox->addWidget(cmdOK);
  QPushButton *cmdCancel = new QPushButton(X_Str("btnCancelCaption", "Cancel"));
  connect(cmdCancel, SIGNAL(clicked()), SLOT(reject()));
  pHBox->addWidget(cmdCancel);
  pVBox->addLayout(pHBox);
  setLayout(pVBox);
  }

void CreateMatrixDialog::ToggleDeterminant(bool Checked)
  {
  if (!Checked)
    {
    m_pRowNumber->setMinimum(1);
    m_pColumnNumber->setMinimum(1);
    return;
    }
  m_pRowNumber->setMinimum(2);
  m_pColumnNumber->setMinimum(2);
  int RowCount = m_pRowNumber->value();
  if (m_pColumnNumber->value() != RowCount) m_pColumnNumber->setValue(RowCount);
  }

void CreateMatrixDialog::ChangeRowColNumber(int n)
  {
  QSpinBox *pFriend = m_pRowNumber;
  if (sender() == pFriend) pFriend = m_pColumnNumber;
  if (n == 1)
    pFriend->setMinimum(2);
  else
    if (pFriend->minimum() == 2) pFriend->setMinimum(1);
  if (m_pDeterminant->isChecked()) pFriend->setValue(n);
  }

QByteArray CreateMatrixDialog::GetResult()
  {
  return (m_pMatrix->isChecked() ? 'M' : 'D') + QByteArray::number(m_pRowNumber->value()) + ',' +
    QByteArray::number(m_pColumnNumber->value());
  }

EditString::EditString( const QString& Text, bool HebString ) : QDialog( nullptr, Qt::WindowSystemMenuHint ),
  m_pString(nullptr), m_pExpression( nullptr )
  {
  QVBoxLayout *pVBox = new  QVBoxLayout;
  m_pEdit = new QLineEdit( Text );
  pVBox->addWidget( m_pEdit );
  if (HebString)
    setWindowTitle(X_Str("L43", "Edit string"));
  else
    {
    m_pString = new QRadioButton("As String", this);
    setWindowTitle(X_Str("L38", "Edit substrings, contained in formula"));
    QHBoxLayout *pHBox = new  QHBoxLayout;
    pHBox->addWidget(m_pString);
    m_pString->setChecked(true);
    m_pExpression = new QRadioButton("As Variable", this);
    pHBox->addWidget(m_pExpression);
    pVBox->addLayout(pHBox);
    }
  QHBoxLayout *pHButtonBox = new  QHBoxLayout;
  QPushButton *cmdOK = new QPushButton( X_Str( "btnOkCaption", "OK" ) );
  connect( cmdOK, SIGNAL( clicked() ), SLOT( accept() ) );
  pHButtonBox->addWidget( cmdOK );
  QPushButton *cmdCancel = new QPushButton( X_Str( "btnCancelCaption", "Cancel" ) );
  connect( cmdCancel, SIGNAL( clicked() ), SLOT( reject() ) );
  pHButtonBox->addWidget( cmdCancel );
  pVBox->addLayout(pHButtonBox);
  setLayout( pVBox );
  }

QString EditString::GetString()
  {
  return m_pEdit->text();
  }

QByteArray EditString::GetFormula()
  {
  return m_pEdit->text().toLocal8Bit();
  }
