/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef _PICVIZ_LIMITS_H_
#define _PICVIZ_LIMITS_H_


#ifdef __cplusplus
 extern "C" {
#endif

/* FIXME: 65535 means unknown */

#define PICVIZ_MAXFLOAT 3.40282347e+38F
#define PICVIZ_MAXDOUBLE 1.79769e+308
#define PICVIZ_MINFLOAT -65535
#define PICVIZ_MINDOUBLE -65535

#define PICVIZ_IPV4_MAXVAL 4294967295UL /* -> 11111111 11111111 11111111 11111111 */
#define PICVIZ_TIME_24H_MAX 86399
#define PICVIZ_TIME_WEEK_MAX 604793 /* 'Saturday 23:59:59'. Sun = 0, Sat = 6. (6 * 86399 = 518394) */
#define PICVIZ_TIME_MONTH_MAX 2678369
#define PICVIZ_TIME_YEAR_MAX 29030064

#ifdef __cplusplus
 }
#endif

#endif /* _PICVIZ_AREA_H_ */
