#ifndef MATHEMATICS_GLOBAL_H
#define MATHEMATICS_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MATHEMATICS_LIB
# define MATHEMATICS_EXPORT Q_DECL_EXPORT
#else
# define MATHEMATICS_EXPORT Q_DECL_IMPORT
#endif

#endif // MATHEMATICS_GLOBAL_H