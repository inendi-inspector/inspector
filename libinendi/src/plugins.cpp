/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <cstdlib>

#include <inendi/plugins.h>

#include <QCoreApplication>
#include <QDir>


const char * inendi_plugins_get_functions_dir(void)
{
	const char *pluginsdir;

	// FIXME : This is a dead code
	pluginsdir = std::getenv("INENDI_FUNCTIONS_DIR");

	return pluginsdir;
}


const char *inendi_plugins_get_filters_dir(void)
{
	const char *pluginsdir;

	pluginsdir = std::getenv("INENDI_FILTERS_DIR");
	// FIXME : This is a dead code

	return pluginsdir;
}

QString inendi_plugins_get_layer_filters_dir(void)
{
	const char* pluginsdir = std::getenv("INENDI_LAYER_FILTERS_DIR");
	if (! pluginsdir) {
		return QCoreApplication::applicationDirPath() + QDir::separator() + INENDI_LAYER_FILTERS_DIR; /* Variable generated by CMAKE */
	}

	return pluginsdir;
}

const char *inendi_plugins_get_layer_filters_config_dir(void)
{
	const char *pluginsdir;

	// FIXME : This is a dead code
	pluginsdir = std::getenv("INENDI_LAYER_FILTERS_CONFIG_DIR");

	return pluginsdir;
}

QString inendi_plugins_get_mapping_filters_dir(void)
{

	const char * pluginsdir = std::getenv("INENDI_MAPPING_FILTERS_DIR");
	if (! pluginsdir) {
		return QCoreApplication::applicationDirPath() + QDir::separator() + INENDI_MAPPING_FILTERS_DIR; /* Variable generated by CMAKE */
	}

	return pluginsdir;
}

QString inendi_plugins_get_plotting_filters_dir(void)
{
	const char * pluginsdir = std::getenv("INENDI_PLOTTING_FILTERS_DIR");
	if (! pluginsdir) {
		return QCoreApplication::applicationDirPath() + QDir::separator() + INENDI_PLOTTING_FILTERS_DIR; /* Variable generated by CMAKE */
	}

	return pluginsdir;
}

QString inendi_plugins_get_axis_computation_dir(void)
{
	const char * pluginsdir = std::getenv("INENDI_AXIS_COMPUTATION_PLUGINS_DIR");
	if (! pluginsdir) {
		return QCoreApplication::applicationDirPath() + QDir::separator() + INENDI_AXIS_COMPUTATION_PLUGINS_DIR; /* Variable generated by CMAKE */
	}

	return pluginsdir;
}

QString inendi_plugins_get_sorting_functions_dir(void)
{
	const char * pluginsdir = std::getenv("INENDI_SORTING_FUNCTIONS_PLUGINS_DIR");
	if (! pluginsdir) {
		return QCoreApplication::applicationDirPath() + QDir::separator() + INENDI_SORTING_FUNCTIONS_PLUGINS_DIR; /* Variable generated by CMAKE */
	}

	return pluginsdir;
}