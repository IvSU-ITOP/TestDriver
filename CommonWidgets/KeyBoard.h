#ifndef MATH_KEYBOARD
#define MATH_KEYBOARD
#include "KeyboardButton.h"
#include <QtMultimedia\qsound.h>
#include <QDockWidget>

class KeyBoard : public QWidget
  {
  QWidget         *m_activeWindow;
  QSound          m_Player;
public:
  KeyBoard( QWidget *parent );
  void soundClick( void ) { m_Player.play(); }
  };
#endif