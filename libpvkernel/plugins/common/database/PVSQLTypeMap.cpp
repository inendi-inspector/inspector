/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include "PVSQLTypeMap.h"
#include "mysql_types.h"

PVRush::PVSQLTypeMap_p PVRush::PVSQLTypeMap::get_map(QString const& driver)
{
	if (driver == "QMYSQL") {
		return p_type(new PVSQLTypeMapMysql());
	}
	if (driver == "QSQLITE") {
		return p_type(new PVSQLTypeMapSQLite());
	}
	if (driver == "QODBC") {
		return p_type(new PVSQLTypeMapODBC());
	}
	return p_type();
}

QString PVRush::PVSQLTypeMapMysql::map(int type) const
{
	switch (type) {
	case FIELD_TYPE_TINY:
		return "tiny";
	case FIELD_TYPE_SHORT:
		return "short";
	case FIELD_TYPE_LONG:
		return "long";
	case FIELD_TYPE_INT24:
		return "int24";
	case FIELD_TYPE_YEAR:
		return "year";
	case FIELD_TYPE_LONGLONG:
		return "longlong";
	case FIELD_TYPE_FLOAT:
		return "float";
	case FIELD_TYPE_DOUBLE:
		return "double";
	case FIELD_TYPE_DECIMAL:
		return "decimal";
#if defined(FIELD_TYPE_NEWDECIMAL)
	case FIELD_TYPE_NEWDECIMAL:
		return "hexadecimal";
#endif
	case FIELD_TYPE_DATE:
		return "date";
	case FIELD_TYPE_TIME:
		return "time";
	case FIELD_TYPE_DATETIME:
		return "datetime";
	case FIELD_TYPE_TIMESTAMP:
		return "timestamp";
	case FIELD_TYPE_STRING:
		return "string";
	case FIELD_TYPE_VAR_STRING:
		return "varstring";
	case FIELD_TYPE_BLOB:
		return "blob";
	case FIELD_TYPE_TINY_BLOB:
		return "tinyblob";
	case FIELD_TYPE_MEDIUM_BLOB:
		return "mediumblob";
	case FIELD_TYPE_LONG_BLOB:
		return "longblob";
	case FIELD_TYPE_ENUM:
		return "enum";
	case FIELD_TYPE_SET:
		return "set";
	};
	return "unknown";
}

QString PVRush::PVSQLTypeMapMysql::map_inendi(int type) const
{
	switch (type) {
	case FIELD_TYPE_TINY:
	case FIELD_TYPE_SHORT:
	case FIELD_TYPE_LONG:
	case FIELD_TYPE_INT24:
	case FIELD_TYPE_YEAR:
	case FIELD_TYPE_LONGLONG:
		return "number_int32";

	case FIELD_TYPE_FLOAT:
	case FIELD_TYPE_DECIMAL:
		return "number_float";

	case FIELD_TYPE_DOUBLE:
		return "number_double";

	// TODO : dates must be propely supported
	case FIELD_TYPE_DATE:
	case FIELD_TYPE_TIME:
	case FIELD_TYPE_DATETIME:
	case FIELD_TYPE_TIMESTAMP:
		return "string";

	case FIELD_TYPE_STRING:
	case FIELD_TYPE_VAR_STRING:
	case FIELD_TYPE_BLOB:
	case FIELD_TYPE_TINY_BLOB:
	case FIELD_TYPE_MEDIUM_BLOB:
	case FIELD_TYPE_LONG_BLOB:
	case FIELD_TYPE_ENUM:
	case FIELD_TYPE_SET:
		return "string";
	};

	return "string";
}