#ifndef MATH_YCLASSES
#define MATH_YCLASSES

#include <qfont.h>
#include <QColor>
#include "XPTypes.h"
#include <qrect.h>
#include <qqueue.h>
#include <qimage.h>
#include <qdialog.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <QRadioButton>
#pragma once

enum TableGrid { TGRVisible, TGRUnvisible, TGRPartlyVisible, TGRNoFreeze };
enum ChartType { ThinCol, ThickCol };
void FloodFill( const QRect& Region, QImage *pImage, QPoint Start, QColor NewColor );

struct U_A_T
  {
  TEdAction act;
  union
    {
    char c;
    char Name[64];
    BasePoint coord;
    uchar nstep;
    } Parms;
  char c() { return Parms.c; }
  char *k() { return Parms.Name; }
  char *i() { return Parms.Name; }
  TPoint coord() { return Parms.coord; }
  uchar nstep() { return Parms.nstep; }
  U_A_T &operator = ( char c ) { Parms.c = c; act = actPrintable; return *this; }
  U_A_T &operator = ( const char *s ) { strcpy( Parms.Name, s ); act = actCtrlKey; return *this; }
  U_A_T &operator = ( TPoint &P ) { Parms.coord = P; act = actMouseButton; return *this; }
  U_A_T &operator = ( uchar c ) { Parms.nstep = c; act = actStep; return *this; }
  bool operator == ( const char *s ) { return strcmp( Parms.Name, s ) == 0; }
  };

struct EditSets
  {
	QFont m_MainFont;
	QFont m_PowrFont;
	QFont m_IndxFont;
  QColor m_BkgrColor;
  QColor m_SignColor;
  QColor m_CursColor;
  EditSets() {}
  EditSets( const QFont &MainFont, const QFont &PowrFont, const QFont &IndxFont,const  QString& BkgrColor, const QString& SignColor, const QString& CursColor ) :
    m_MainFont( MainFont), m_PowrFont( PowrFont ), m_IndxFont( IndxFont ), m_BkgrColor( BkgrColor ), 
    m_SignColor( SignColor ), m_CursColor( CursColor ) 
      {
      }
  };

struct ViewSettings
  {
  QFont m_TaskCmFont;
  QFont m_CalcCmFont;
  QFont m_SimpCmFont;
  QColor m_BkgrColor;
  ViewSettings() {}
  ViewSettings( const QFont &TaskCmFont, const QFont &CalcCmFont, const QFont &SimpCmFont, const  QString& BkgrColor ) :
    m_TaskCmFont( TaskCmFont ), m_CalcCmFont( CalcCmFont ), m_SimpCmFont( SimpCmFont ), m_BkgrColor( BkgrColor )
    {
    }
  };

typedef QQueue<class Region> RegionQueue;

class Region : public QRect
  {
  QImage *m_pImage;
  QPoint m_Start;
  QColor m_OldColor;
  QColor m_NewColor;
  RegionQueue *m_pQueue;
  QColor GetColor( int x, int y ) { return m_pImage->pixelColor(x, y); }
  void SetColor( int x, int y ) { m_pImage->setPixelColor( x, y, m_NewColor ); }
  public:
    Region() {}
    Region( const QRect& Rect, QPoint Start, QColor OldColor, QColor NewColor, QImage *pImage, RegionQueue *pQueue ) : QRect( Rect ),
      m_Start( Start ), m_OldColor( OldColor ), m_NewColor( NewColor ), m_pImage( pImage ), m_pQueue( pQueue ) {}
    Region( const QPoint& Start, const Region& Parent ) : QRect( Parent.topLeft(), Parent.size() ), m_Start( Start ), 
      m_OldColor( Parent.m_OldColor ), m_NewColor( Parent.m_NewColor ), m_pImage( Parent.m_pImage ), m_pQueue( Parent.m_pQueue ) {}
    void FloodFill();
  };

class EdChartEditor;
class CreateChartDialog : public QDialog
  {
  Q_OBJECT
    friend class EdChartType;
  QSpinBox *m_pColumnNumber;
  QDoubleSpinBox *m_pScale;
  QLabel *m_pType;
  ChartType m_Type;
  EdChartEditor *m_pEditor;
  bool m_WasChanged;
  public:
    CreateChartDialog( EdChartEditor *pEditor );
    public slots:
    void accept();
    void SetThin();
    void SetThick();
  };

class CreateTableDialog : public QDialog
  {
  QSpinBox *m_pColumnNumber;
  QSpinBox *m_pRowNumber;
  QRadioButton *m_pVisible;
  QRadioButton *m_pUnvisible;
  QRadioButton *m_pPartlyVisible;
  public:
    CreateTableDialog();
    QByteArray GetResult();
  };

class CreateMatrixDialog : public QDialog
  {
  Q_OBJECT
    QSpinBox *m_pColumnNumber;
  QSpinBox *m_pRowNumber;
  QRadioButton *m_pMatrix;
  QRadioButton *m_pDeterminant;
  public:
    CreateMatrixDialog();
    QByteArray GetResult();
    public slots:
    void ToggleDeterminant(bool);
    void ChangeRowColNumber(int);
  };

class EditString : public QDialog
  {
  QLineEdit *m_pEdit;
  QRadioButton *m_pString;
  QRadioButton *m_pExpression;
  public:
    EditString( const QString& Text, bool HebString = false );
    QString GetString();
    bool IsString() { return m_pString->isChecked(); }
    QByteArray GetFormula();
  };

#endif