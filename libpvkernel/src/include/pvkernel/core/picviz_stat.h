/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_PICVIZSTAT_H
#define PVCORE_PICVIZSTAT_H

#include <iostream>

/**
 * @file picviz_stat.h
 *
 * statistics framework.
 *
 * This module must be used through the following macros: #PV_STAT
 * and derivates.
 */

/**
 * @def PV_STAT(NAME, VALUE)
 *
 * Print a generic statistic line using the parameters.
 *
 * @param NAME the statistic's name
 * @param UNIT the statistic's unit
 * @param VALUE the statistic's value
 */
#define PV_STAT(NAME, UNIT, VALUE) std::cerr << "__pvstat__{{" << (NAME) << "," << (UNIT) << "}}:" << (VALUE) << std::endl

/**
 * @def PV_STAT_MEM_USE(NAME, VALUE)
 *
 * Print a statistic line about memory consumption in Mio.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_MEM_USE(NAME, VALUE) PV_STAT((NAME), "Mio", (VALUE))

/**
 * @def PV_STAT_MEM_USE_O(NAME, VALUE)
 *
 * Print a statistic line about memory consumption in octets.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_MEM_USE_O(NAME, VALUE) PV_STAT((NAME), "octets", (VALUE))

/**
 * @def PV_STAT_MEM_BW(NAME, VALUE)
 *
 * Print a statistic line about memory bandwidth.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_MEM_BW(NAME, VALUE) PV_STAT((NAME), "Mio/s", (VALUE))

/**
 * @def PV_STAT_PROCESS_BW(NAME, VALUE)
 *
 * Print a statistic line about data processing bandwidth. It is expressed in
 * elements per second.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_PROCESS_BW(NAME, VALUE) PV_STAT((NAME), "ele/s", (VALUE))

/**
 * @def PV_STAT_CALLS(NAME, VALUE)
 *
 * Print a statistic line about a number of calls per second.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_CALLS(NAME, VALUE) PV_STAT((NAME), "calls/s", (VALUE))

/**
 * @def PV_STAT_SPEEDUP(NAME, VALUE)
 *
 * Print a statistic line about a speedup.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_SPEEDUP(NAME, VALUE) PV_STAT((NAME), "speedup", (VALUE))

/**
 * @def PV_STAT_TIME_SEC(NAME, VALUE)
 *
 * Print a statistic line about time in seconds.
 *
 * @param NAME the statistic's name
 * @param VALUE the statistic's value
 */
#define PV_STAT_TIME_SEC(NAME, VALUE) PV_STAT((NAME), "s", (VALUE))

/**
 * @def PV_STAT_TIME_MSEC(NAME, VALUE)
 *
 * Print a statistic line about time in milliseconds.
 */
#define PV_STAT_TIME_MSEC(NAME, VALUE) PV_STAT((NAME), "ms", (VALUE))

/**
 * @def PV_STAT_TIME_MSEC(NAME, VALUE)
 *
 * Print a statistic line about time in milliseconds.
 */
#define PV_STAT_TIME_MICROSEC(NAME, VALUE) PV_STAT((NAME), "µs", (VALUE))

#endif // PVCORE_PICVIZSTAT_H
