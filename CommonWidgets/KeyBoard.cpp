#include "KeyBoard.h"
#include <QLayout>
#include <QKeyEvent>
#include <QDir>

KeyBoard::KeyBoard( QWidget *parent ) : QWidget( parent ), m_Player( QDir::currentPath() + CLICK_SOUND )
  {        
  setWindowIcon(QPixmap(":/TastieraVirtuale/logo"));
  KeyboardButton::sm_KeyBoard = this;
  KeyboardButton	*pBtn;
  QVBoxLayout     *pVLayout = new QVBoxLayout;
  QHBoxLayout     *pHLayout = new QHBoxLayout;
  QSpacerItem *pSpacer = new QSpacerItem( KeyboardButton::sm_Width, KeyboardButton::sm_Height );

  pHLayout->addWidget( new KeyboardButton( '!' ) );
  for( char iDigit = '1'; iDigit <= '9'; pHLayout->addWidget( new KeyboardButton( iDigit++ ) ) );
  pHLayout->addWidget( new KeyboardButton( '0' ) );
  pHLayout->addWidget( new KeyboardButton( '(' ) );
  pHLayout->addWidget( new KeyboardButton( ')' ) );
  pHLayout->addWidget( new KeyboardButton( '=' ) );
  pHLayout->addWidget( new KeyboardButton( '|' ) );
  pBtn = new KeyboardButton( Qt::Key_Backspace, "Backspace" );
  pBtn->setMaximumWidth( pBtn->maximumWidth() * 2 );
  pBtn->setMinimumWidth( pBtn->minimumWidth() * 2 );
  pHLayout->addWidget( pBtn );
  pVLayout->addLayout( pHLayout );

  pHLayout = new QHBoxLayout;
  pHLayout->addWidget( new KeyboardButton( 'q' ) );
  pHLayout->addWidget( new KeyboardButton( 'w' ) );
  pHLayout->addWidget( new KeyboardButton( 'e' ) );
  pHLayout->addWidget( new KeyboardButton( 'r' ) );
  pHLayout->addWidget( new KeyboardButton( 't' ) );
  pHLayout->addWidget( new KeyboardButton( 'y' ) );
  pHLayout->addWidget( new KeyboardButton( 'u' ) );
  pHLayout->addWidget( new KeyboardButton( 'i' ) );
  pHLayout->addWidget( new KeyboardButton( 'o' ) );
  pHLayout->addWidget( new KeyboardButton( 'p' ) );
  pHLayout->addWidget( new KeyboardButton( '[' ) );
  pHLayout->addWidget( new KeyboardButton( ']' ) );
  pHLayout->addWidget( new KeyboardButton( '{' ) );
  pHLayout->addWidget( new KeyboardButton( '}' ) );
  pHLayout->addWidget( new KeyboardButton( '?' ) );

  QVBoxLayout *ReturnLayout = new QVBoxLayout;
  ReturnLayout->addLayout( pHLayout, 1 );

  pHLayout = new QHBoxLayout;
  pBtn = new KeyboardButton( Qt::Key_CapsLock, "Caps Lock" );
  pBtn->setMaximumWidth( pBtn->maximumWidth() * 2 + 5 );
  pBtn->setMinimumWidth( pBtn->minimumWidth() * 2 + 5 );
  pHLayout->addWidget( pBtn );
  pHLayout->addWidget( new KeyboardButton( 'a' ) );
  pHLayout->addWidget( new KeyboardButton( 's' ) );
  pHLayout->addWidget( new KeyboardButton( 'd' ) );
  pHLayout->addWidget( new KeyboardButton( 'f' ) );
  pHLayout->addWidget( new KeyboardButton( 'g' ) );
  pHLayout->addWidget( new KeyboardButton( 'h' ) );
  pHLayout->addWidget( new KeyboardButton( 'j' ) );
  pHLayout->addWidget( new KeyboardButton( 'k' ) );
  pHLayout->addWidget( new KeyboardButton( 'l' ) );
  pHLayout->addWidget( new KeyboardButton( ';' ) );
  pHLayout->addWidget( new KeyboardButton( ':' ) );
  pHLayout->addWidget( new KeyboardButton( '/' ) );
  pHLayout->addWidget( new KeyboardButton( '*' ) );

  ReturnLayout->addLayout( pHLayout, 1 );
  pHLayout = new QHBoxLayout;
  pHLayout->addLayout( ReturnLayout, 1 );
  pBtn = new KeyboardButton( Qt::Key_Enter, "Enter" );
  pBtn->setMaximumWidth( pBtn->maximumWidth() * 2 );
  pBtn->setMinimumWidth( pBtn->minimumWidth() * 2 );
  pBtn->setMinimumHeight( pBtn->minimumHeight() * 2 );
  pBtn->setMaximumHeight( pBtn->maximumHeight() * 2 );
  pHLayout->addWidget( pBtn );
  pVLayout->addLayout( pHLayout );
 
  pHLayout = new QHBoxLayout;
  pHLayout->addSpacerItem( pSpacer );
  pHLayout->addStretch( 1 );
  pHLayout->addWidget( new KeyboardButton( 'z' ) );
  pHLayout->addWidget( new KeyboardButton( 'x' ) );
  pHLayout->addWidget( new KeyboardButton( 'c' ) );
  pHLayout->addWidget( new KeyboardButton( 'v' ) );
  pHLayout->addWidget( new KeyboardButton( 'b' ) );
  pHLayout->addWidget( new KeyboardButton( 'n' ) );
  pHLayout->addWidget( new KeyboardButton( 'm' ) );
  pHLayout->addWidget( new KeyboardButton( ',' ) );
  pHLayout->addWidget( new KeyboardButton( '.' ) );
  pHLayout->addWidget( new KeyboardButton( '<' ) );
  pHLayout->addWidget( new KeyboardButton( '>' ) );
  pHLayout->addWidget( new KeyboardButton( '+' ) );
  pHLayout->addWidget( new KeyboardButton( '-' ) );
  pHLayout->addWidget( new KeyboardButton( Qt::Key_Up, "Up", ":/Resources/ArrowUp.png" ) );
  pBtn = new KeyboardButton( Qt::Key_Delete, "Delete" );
  pBtn->setMaximumWidth( pBtn->maximumWidth() * 2 );
  pBtn->setMinimumWidth( pBtn->minimumWidth() * 2 );
  pHLayout->addWidget( pBtn );
  pVLayout->addLayout( pHLayout );

  pHLayout = new QHBoxLayout;
  pHLayout->addSpacerItem( new QSpacerItem( *pSpacer ) );
  pHLayout->addSpacerItem( new QSpacerItem( *pSpacer ) );
  pHLayout->addSpacing( 17 );
  pBtn = new KeyboardButton( ' ' );
  pBtn->setMaximumWidth( pBtn->maximumWidth() * 10.3 );
  pBtn->setMinimumWidth( pBtn->minimumWidth() * 10.3 );
  pHLayout->addWidget( pBtn );
  QHBoxLayout *pKeyLauout = new QHBoxLayout;
  pKeyLauout->addWidget( new KeyboardButton( Qt::Key_Left, "Left", ":/Resources/ArrowLeft.png" ) );
  pKeyLauout->addWidget( new KeyboardButton( Qt::Key_Down, "Down", ":/Resources/ArrowDown.png" ) );
  pKeyLauout->addWidget( new KeyboardButton( Qt::Key_Right, "Right", ":/Resources/ArrRight.png" ) );
  pHLayout->addLayout( pKeyLauout );
  pVLayout->addLayout( pHLayout );
  setLayout( pVLayout );
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  }
