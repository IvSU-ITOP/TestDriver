#ifndef COMMONWIDGETS_GLOBAL_H
#define COMMONWIDGETS_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef COMMONWIDGETS_LIB
# define COMMONWIDGETS_EXPORT Q_DECL_EXPORT
#else
# define COMMONWIDGETS_EXPORT Q_DECL_IMPORT
#endif

#endif // COMMONWIDGETS_GLOBAL_H
