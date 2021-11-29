#ifndef MATH_KEYBOARDBUTTON
#define MATH_KEYBOARDBUTTON
#include <QPushButton>
#include <QTime>

#define CLICK_SOUND                     "/sounds/click.wav" 

//  #define DEFAULT_STYLE_BUTTON            "color:white;border: 1px solid #000000;border-radius:6px;"
//  #define STYLE_FOCUS_TEXT                "border: 1px solid red"

class KeyBoard;

class KeyboardButton : public QPushButton 
  {
  friend KeyBoard;
  static QSize sm_ImageDim;
  static int sm_Width;
  static int sm_Height;
  static bool sm_CapsActive;
  static QSet< KeyboardButton* > sm_ABCButtons;
  static KeyBoard* sm_KeyBoard;
  static QString  sm_DefaultBackground;
  static QString  sm_ChangedBackground;
  static QString  sm_DefaulStyleButton;
  static double sm_ButtonHeight;

  int m_Key;
  QTime m_Timer;

  void Initialise();
protected:
  void 		mousePressEvent( QMouseEvent * );
  void 		mouseReleaseEvent( QMouseEvent * );
public:
  static QObject* sm_pReceiver;
  KeyboardButton( int Char ) : QPushButton( QString( Char ) ), m_Key( Char ) { Initialise(); }
  KeyboardButton( int Char, const char* Text ) : QPushButton( Text ), m_Key( Char ) { Initialise(); }
  KeyboardButton( int Char, const char* Text, const char * Icon );
};

#endif