/**
 * \file general.h
 *
 * Copyright (C) Picviz Labs 2009-2012
 */

/* #ifdef CUSTOMER_RELEASE */
//AUTO_CUSTOMER_RELEASE #include <picviz/api-obfuscate.h> 
/* #endif */

#include <pvbase/types.h>

#include <pvkernel/core/general.h>

/* #define PICVIZ_BEGIN_NAMESPACE namespace Picviz { */
/* #define PICVIZ_CLOSE_NAMESPACE } */

#ifndef VM_START
	#define VM_START
	#define VM_END
	#define CODEREPLACE_START
	#define CODEREPLACE_END
#endif

/* Global that holds the error message */
/* extern char *debug_msg;  */

//#ifdef __cplusplus
// extern "C" {
//#endif

#define PICVIZ_DEFAULT_IMAGE_HEIGHT 350
#define PICVIZ_DEFAULT_IMAGE_HEADER_HEIGHT 15
#define PICVIZ_DEFAULT_IMAGE_WIDTH 0

#define PICVIZ_DEFAULT_PENWIDTH "1.0"

#define PICVIZ_RING_STRUCTOF(s) _##s
/* #define PICVIZ_ARRAY_PUSH(array,type) (*((type *)apr_array_push (array))) */
#define PICVIZ_USERDATA(userdata,type) (*(type *)(userdata))

/* #define PICVIZ_ARRAY(t) apr_array_header_t * */

#ifdef _POSIX_C_SOURCE
/* /usr/include/python2.6/pyconfig-32.h:1019:1: warning: "_POSIX_C_SOURCE" redefined */
#undef _POSIX_C_SOURCE 
#endif
#ifdef _XOPEN_SOURCE
/* /usr/include/python2.6/pyconfig-32.h:1028:1: warning: "_XOPEN_SOURCE" redefined */
#undef _XOPEN_SOURCE
#endif


#define PICVIZ_TIME_MSEC    * 1
#define PICVIZ_TIME_SEC     * 1000 MSEC
#define PICVIZ_TIME_MIN     * 60 SEC
#define PICVIZ_TIME_HOUR    * 60 MIN
#define PICVIZ_TIME_DAY     * 24 HOUR

#define PICVIZ_OBJECT_DTRI(x) x
#define PICVIZ_OBJECT_SCENE(x) x
#define PICVIZ_OBJECT_SOURCE(x) x
#define PICVIZ_OBJECT_MAPPING(x) x
#define PICVIZ_OBJECT_MAPPED(x) x
#define PICVIZ_OBJECT_PLOTTING(x) x
#define PICVIZ_OBJECT_PLOTTED(x) x
#define PICVIZ_OBJECT_VIEW(x) x
#define PICVIZ_OBJECT_SELECTION(x) x
#define PICVIZ_OBJECT_NRAW(x) x

/* #undef _DEBUG */
/* #include <Python.h> */

/* #include <apr_general.h> */
/* #include <apr_tables.h> */
/* #include <apr_hash.h> */
/* #include <apr_ring.h> */

//#ifdef __cplusplus
// }
//#endif

