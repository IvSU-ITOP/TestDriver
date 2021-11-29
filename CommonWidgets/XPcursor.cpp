#include "XPcursor.h"

XPCursor::XPCursor( QWidget *pOwner ) : m_CursorOn(false), m_CursorBlOn(false), m_Changing(false), m_StepTime( cm_DefaultStepTime ), m_pOwner( pOwner ),
  m_ClipRect( QRect(0, 0, 32767, 32767) ), m_PaintMode( QPainter::CompositionMode_SourceOver ), m_CanPaint( false )
  {
  m_Pen.setWidth( cm_DefaultThickness );
  }

bool XPCursor::Paint()
  {
	if( !m_CanPaint || m_Pos.X < m_ClipRect.left() || m_Pos.X > m_ClipRect.right() ) return false;
  m_CanPaint = false;
	int cTop = m_Pos.Y;
	if( cTop < m_ClipRect.top() )
		cTop = m_ClipRect.top();
	int cBottom = m_Pos.Y + m_Size;
	if( cBottom > m_ClipRect.bottom() )
		cBottom = m_ClipRect.bottom();
	if( cTop > cBottom ) return false;
  m_Canvas.begin( m_pOwner );
  m_Canvas.setPen( m_Pen );
  m_Canvas.setCompositionMode( static_cast<QPainter::CompositionMode>( m_PaintMode ) );
  m_Canvas.drawLine( m_Pos.X, cTop, m_Pos.X, cBottom );
  m_Canvas.end();
  return true;
  }

bool XPCursor::Erase() 
  { 
  m_PaintMode = QPainter::RasterOp_SourceXorDestination;
  bool Result = Paint();
  m_PaintMode =  QPainter::CompositionMode_SourceOver;
  return Result; 
  }

void XPCursor::Show()
  {
  if( m_CursorOn ) return;
  m_CursorOn = true;
  m_CursorBlOn = false;
  m_NextBlink = QTime::currentTime().addMSecs(m_StepTime);
  start( cm_DefaultStepTime );
  }

void XPCursor::Hide()
  {
	if( !m_CursorOn ) return;
	m_CursorOn = false;
  stop();
  }

bool XPCursor::Blink()
  {
  if( !m_CursorOn || m_Changing || QTime::currentTime() < m_NextBlink ) return false;
  bool Changed;
	if( m_CursorBlOn ) 
		Changed = Erase();
	else
		Changed = Paint();
	m_CursorBlOn = m_CursorBlOn ^ Changed;
    m_NextBlink = QTime::currentTime().addMSecs(m_StepTime);
    return true;
  }

void XPCursor::SetVisible( bool Value )
  {
	if( Value == m_CursorOn ) return;
	if( Value )
		Show();
	else
		Hide();
  }

void XPCursor::Move(const  TPoint &NewPos, int NewSize )
  {
	if( NewPos.X == m_Pos.X && NewPos.Y == m_Pos.Y &&	NewSize == m_Size ) return;
	m_Changing = true;
	if( m_CursorBlOn ) Erase();
	m_Pos = NewPos;
	m_Size = NewSize;
	if( m_CursorOn )
		m_CursorBlOn = Paint();
	m_Changing = false;
  }

void XPCursor::SetThickness( int Value )
  {
	if( Value == m_Pen.width() ) return;
	m_Changing = true;
  if( m_CursorBlOn ) Erase();
  m_Pen.setWidth( Value );
	if( m_CursorBlOn ) m_CursorBlOn = Paint();
	m_Changing = false;
  }

void XPCursor::SetStepTime( int Value )
  {
	if( Value < cm_MinStepTime )	Value = cm_MinStepTime;
	if( Value > cm_MaxStepTime )	Value = cm_MaxStepTime;
	if( Value == m_StepTime ) return;
	m_StepTime = Value;
  setInterval( m_StepTime );
  }

void XPCursor::SetClipRect( QRect &Value )
  {
  if( m_ClipRect == Value ) return;
	if( Value.right() < Value.left() )	Value.setRight( Value.left() );
	if( Value.bottom() < Value.top() )	Value.setBottom( Value.top() );
	m_Changing = true;
	if( m_CursorBlOn ) Erase();
	m_ClipRect = Value;
	if( m_CursorBlOn ) m_CursorBlOn = Paint();
	m_Changing = false;
  }

