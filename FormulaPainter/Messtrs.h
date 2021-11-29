#ifndef MATH_MESSTRS
#define MATH_MESSTRS
#include <QTextStream>
#include <qmap.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qwidget.h>
#include "formulapainter_global.h"

class Translator : public QMap < QByteArray, QByteArray >
  {
  public:
    Translator( const QString& File );
  };

FORMULAPAINTER_EXPORT QString X_Str( const QByteArray& Ident, const QByteArray& Default );
FORMULAPAINTER_EXPORT QByteArray X_BStr( const QByteArray& Ident, const QByteArray& Default );
FORMULAPAINTER_EXPORT void Error_m( const QString& );
void Error_m( const QByteArray& );
void Info_m( const QByteArray& S );
FORMULAPAINTER_EXPORT void Info_m( const QString& S );
FORMULAPAINTER_EXPORT QString ToLocal( const QByteArray& Text );
FORMULAPAINTER_EXPORT extern bool s_CalcOnly;
FORMULAPAINTER_EXPORT extern QString s_LastError;
FORMULAPAINTER_EXPORT extern void( *s_AfterMsg )( );

class TXPStatus
  {
  QString m_CurrentMessage;
  public:
    FORMULAPAINTER_EXPORT static bool sm_ShowErrMessage;
    FORMULAPAINTER_EXPORT void SetMessage( const QByteArray& AMessage );
    FORMULAPAINTER_EXPORT void SetMessage(const QString& AMessage);
    FORMULAPAINTER_EXPORT QString GetCurrentMessage();
  };

#endif
