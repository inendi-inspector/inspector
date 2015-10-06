/**
 * \file plugins.h
 *
 * Copyright (C) Picviz Labs 2009-2012
 */

#ifndef _PICVIZ_PLUGINS_H_
#define _PICVIZ_PLUGINS_H_

#include <picviz/general.h>
#include <QString>

#define PICVIZ_PLUGINSLIST_MAXSIZE 32768

const char *picviz_plugins_get_functions_dir(void);
const char *picviz_plugins_get_filters_dir(void);
QString picviz_plugins_get_layer_filters_dir(void);
const char *picviz_plugins_get_layer_filters_config_dir(void);
QString picviz_plugins_get_mapping_filters_dir(void);
QString picviz_plugins_get_plotting_filters_dir(void);
QString picviz_plugins_get_axis_computation_dir(void);
QString picviz_plugins_get_sorting_functions_dir(void);

#endif /* _PICVIZ_PLUGINS_H_ */
