#include "KeyboardButton.h"
#include "KeyBoard.h"
#include <QKeyEvent>
#include <QCoreApplication>

QObject* KeyboardButton::sm_pReceiver = NULL;
QSize KeyboardButton::sm_ImageDim = QSize(32, 32);
int KeyboardButton::sm_Width = 54;
int KeyboardButton::sm_Height = 40;
QSet< KeyboardButton* > KeyboardButton::sm_ABCButtons;
bool KeyboardButton::sm_CapsActive = false;
KeyBoard* KeyboardButton::sm_KeyBoard = NULL;
QString  KeyboardButton::sm_DefaultBackground( "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #8C8F8C, stop: 1 #000000);" );
QString  KeyboardButton::sm_ChangedBackground( "background:lightgray;color:darkred;" );
QString  KeyboardButton::sm_DefaulStyleButton( "color:white;border: 1px solid #000000;border-radius:6px;" );
#ifdef Q_OS_ANDROID
double KeyboardButton::sm_ButtonHeight = 15.0;
#else
double KeyboardButton::sm_ButtonHeight = 6.0;
#endif

extern double PixelHeight;

KeyboardButton::KeyboardButton( int Char, const char* Text, const char * Icon ) : QPushButton( QPixmap( Icon ), "" ), m_Key( Char )
  {
  double Scale = sm_ButtonHeight / ( PixelHeight * sm_Height );
  setIconSize( sm_ImageDim * Scale ); Initialise();
  }

void KeyboardButton::Initialise() 
  { 
  setFont( QFont( "Arial" ) );
  setStyleSheet( sm_DefaulStyleButton + sm_DefaultBackground );
  double Scale = sm_ButtonHeight / ( PixelHeight * sm_Height );
  setMinimumSize( sm_Width * Scale, sm_Height * Scale );
  setMaximumSize( sm_Width * Scale, sm_Height * Scale );
  if( m_Key >= 'a' && m_Key <= 'z' )
    sm_ABCButtons.insert( this );
  setVisible( true );
  }

void KeyboardButton::mousePressEvent( QMouseEvent * )
  {
  setStyleSheet( sm_DefaulStyleButton + sm_ChangedBackground );
  if( !m_Timer.isNull() && m_Timer.elapsed() < 200 ) return;
  m_Timer.start();
  if( m_Key == Qt::Key_CapsLock ) 
    {
    sm_CapsActive = !sm_CapsActive;
    int iShift = 'a' - 'A';
    if( sm_CapsActive )
      iShift = -iShift;
    for( QSet< KeyboardButton* >::iterator pKey = sm_ABCButtons.begin(); pKey != sm_ABCButtons.end(); pKey++ )
      ( *pKey )->setText(  QChar( ( *pKey )->text()[0].toLatin1() + iShift ) );
    sm_KeyBoard->repaint();
    return;
	  }

  QKeyEvent KeyEvent( QEvent::KeyPress, m_Key, sm_CapsActive ? Qt::ShiftModifier : Qt::NoModifier, text() );
  QCoreApplication::sendEvent( sm_pReceiver, &KeyEvent );
  repaint();
  }

void KeyboardButton::mouseReleaseEvent(QMouseEvent *)
  {
  setStyleSheet( sm_DefaulStyleButton + sm_DefaultBackground );
  sm_KeyBoard->soundClick();
  repaint();
  }
