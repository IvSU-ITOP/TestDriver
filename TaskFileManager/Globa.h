#ifndef MATH_GLOBA
#define MATH_GLOBA
#include "../FormulaPainter/XPTypes.h"
#include "taskfilemanager_global.h"

TASKFILEMANAGER_EXPORT QByteArray FromQBB( const QByteArray& CS, TLanguages );
TASKFILEMANAGER_EXPORT QString FromQBA( const QByteArray&, TLanguages, bool AddChar = false );
TASKFILEMANAGER_EXPORT QString ToLanguage( const QByteArray&, TLanguages );

#endif
