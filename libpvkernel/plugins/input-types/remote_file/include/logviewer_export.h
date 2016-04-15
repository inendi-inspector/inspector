/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef LOGVIEWERLIB_H
#define LOGVIEWERLIB_H

#include <qglobal.h>

#define LOGVIEWER_STATICLIB 1

# ifdef LOGVIEWER_STATICLIB
#  undef LOGVIEWER_SHAREDLIB
#  define LOGVIEWER_EXPORT
# else
#  ifdef LOGVIEWER_BUILD_LOGVIEWER_LIB
#   define LOGVIEWER_EXPORT Q_DECL_EXPORT
#  else
#   define LOGVIEWER_EXPORT Q_DECL_IMPORT
#  endif
# endif

#endif /* LOGVIEWERLIB_H */

