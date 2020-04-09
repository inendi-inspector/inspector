/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include "PVInputTypeDatabase.h"
#include "PVDatabaseParamsWidget.h"

#include "../../common/database/PVDBInfos.h"

PVRush::PVInputTypeDatabase::PVInputTypeDatabase() : PVInputTypeDesc<PVDBQuery>() {}

bool PVRush::PVInputTypeDatabase::createWidget(hash_formats& formats,
                                               list_inputs& inputs,
                                               QString& format,
                                               PVCore::PVArgumentList& /*args_ext*/,
                                               QWidget* parent) const
{
	connect_parent(parent);
	PVDatabaseParamsWidget* params = new PVDatabaseParamsWidget(this, formats, parent);
	if (params->exec() == QDialog::Rejected) {
		return false;
	}

	PVDBInfos infos = params->get_infos();
	PVDBServ_p serv(new PVDBServ(infos));
	PVInputDescription_p query(new PVDBQuery(serv, params->get_query()));

	inputs.push_back(query);

	if (params->is_format_custom()) {
		PVRush::PVFormat custom_format(params->get_custom_format().documentElement());
		formats["custom"] = custom_format;
		format = "custom";
	} else {
		format = params->get_existing_format();
	}

	return true;
}

PVRush::PVInputTypeDatabase::~PVInputTypeDatabase() {}

QString PVRush::PVInputTypeDatabase::name() const
{
	return QString("database");
}

QString PVRush::PVInputTypeDatabase::human_name() const
{
	return QString("Database import plugin");
}

QString PVRush::PVInputTypeDatabase::human_name_serialize() const
{
	return QString("Databases");
}

QString PVRush::PVInputTypeDatabase::internal_name() const
{
	return QString("02-database");
}

QString PVRush::PVInputTypeDatabase::menu_input_name() const
{
	return QString("Database...");
}

QString PVRush::PVInputTypeDatabase::tab_name_of_inputs(list_inputs const& in) const
{
	PVInputDescription_p query = in[0];
	return query->human_name();
}

bool PVRush::PVInputTypeDatabase::get_custom_formats(PVInputDescription_p /*in*/,
                                                     hash_formats& /*formats*/) const
{
	return false;
}

QKeySequence PVRush::PVInputTypeDatabase::menu_shortcut() const
{
	return QKeySequence();
}
