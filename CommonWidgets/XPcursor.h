#ifndef MATH_XPCURSOR
#define MATH_XPCURSOR
#include <QtWidgets>
#include "../FormulaPainter/YClasses.h"

class XPCursor : public QTimer
  {
  static uint const
    cm_DefaultThickness =	2,
    cm_DefaultStepTime	=	500,
    cm_MinStepTime	=	50,
    cm_MaxStepTime	=	2000,
    cm_DefaultSize = 16;

  friend class XPGedit;
  QWidget *m_pOwner;
  QPainter m_Canvas;
  QPen m_Pen;
  QRect m_ClipRect;
  TPoint m_Pos;
  QTime m_NextBlink;
  int m_Size;
  uint m_StepTime;
  bool m_CursorOn, m_CursorBlOn, m_Changing;
  bool m_CanPaint;
  int m_PaintMode;
  bool Paint();
  bool Erase();
  virtual void	timerEvent(QTimerEvent * e) 
    { 
    m_pOwner->repaint(m_Pos.X - 1, m_Pos.Y - 1, m_Pen.width(), m_Size + 2); 
    }

  public:
    XPCursor ( QWidget *pOwner );
    void Show();
    void Hide();
    bool Blink();
    void Move( const TPoint &NewPos, int NewSize );
    int	GetThickness() { return m_Canvas.pen().width(); }
    void SetThickness( int Value );
    void SetStepTime( int Value );
    void SetClipRect(  QRect &Value );
    void SetVisible( bool Value );
    int	GetPosX() { return m_Pos.X; }
    void SetPosX( int Value ) { Move( TPoint( Value, m_Pos.Y), m_Size ); }
    int	GetPosY() { return m_Pos.Y; }
    void SetPosY( int Value ) { Move( TPoint( m_Pos.X, Value ), m_Size ); }
    void SetSize( int Value ) { Move( m_Pos, Value ); }
  };

#endif
