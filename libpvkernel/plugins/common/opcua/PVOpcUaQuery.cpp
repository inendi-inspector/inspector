/**
 * @file
 *
 * @copyright (C) ESI Group INENDI 2019
 */

#include "PVOpcUaQuery.h"

PVRush::PVOpcUaQuery::PVOpcUaQuery(PVOpcUaInfos const& infos,
                                   QString const& query,
                                   QString const& query_type)
    : _infos(infos), _query(query), _query_type(query_type)
{
}

bool PVRush::PVOpcUaQuery::operator==(const PVInputDescription& other) const
{
	PVOpcUaQuery const* other_query = dynamic_cast<PVOpcUaQuery const*>(&other);
	if (!other_query) {
		return false;
	}
	return _infos == other_query->_infos && _query == other_query->_query &&
	       _query_type == other_query->_query_type;
}

QString PVRush::PVOpcUaQuery::human_name() const
{
	return QString("opcua");
}

void PVRush::PVOpcUaQuery::serialize_write(PVCore::PVSerializeObject& so) const
{
	so.set_current_status("Saving OpcUa information...");
	so.attribute_write("query", _query);
	so.attribute_write("query_type", _query_type);
	_infos.serialize_write(*so.create_object("server"));
}

std::unique_ptr<PVRush::PVInputDescription>
PVRush::PVOpcUaQuery::serialize_read(PVCore::PVSerializeObject& so)
{
	so.set_current_status("Loading OpcUa information...");
	QString query = so.attribute_read<QString>("query");
	QString query_type = so.attribute_read<QString>("query_type");
	PVOpcUaInfos infos = PVOpcUaInfos::serialize_read(*so.create_object("server"));
	return std::unique_ptr<PVOpcUaQuery>(new PVOpcUaQuery(infos, query, query_type));
}

void PVRush::PVOpcUaQuery::save_to_qsettings(QSettings& settings) const
{
	settings.setValue("host", _infos.get_host());
	settings.setValue("port", _infos.get_port());
	settings.setValue("index", _infos.get_index());
	settings.setValue("importer", _infos.get_importer());
	settings.setValue("format", _infos.get_format());
	settings.setValue("is_format_custom", _infos.is_format_custom() ? "true" : "false");
	settings.setValue("filter_path", _infos.get_filter_path());

	settings.setValue("query", _query);
	settings.setValue("query_type", _query_type);
}

std::unique_ptr<PVRush::PVInputDescription>
PVRush::PVOpcUaQuery::load_from_string(std::vector<std::string> const& vl, bool /*multi_inputs*/)
{
	QString query = QString::fromStdString(vl[0]);
	QString query_type = QString::fromStdString(vl[1]);

	PVOpcUaInfos infos;
	infos.set_host(QString::fromStdString(vl[2]));
	infos.set_port(std::stoi(vl[3]));
	infos.set_index(QString::fromStdString(vl[4]));
	infos.set_importer(QString::fromStdString(vl[5]));
	infos.set_format(QString::fromStdString(vl[6]));
	infos.set_custom_format(QString::fromStdString(vl[7]) == "true");
	infos.set_filter_path(QString::fromStdString(vl[8]));

	if (vl.size() == 11) {
		infos.set_login(QString::fromStdString(vl[9]));
		infos.set_password(QString::fromStdString(vl[10]));
	}

	return std::unique_ptr<PVOpcUaQuery>(new PVOpcUaQuery(infos, query, query_type));
}

std::vector<std::string> PVRush::PVOpcUaQuery::desc_from_qsetting(QSettings const& s)
{
	std::vector<std::string> res = {s.value("query").toString().toStdString(),
	                                s.value("query_type").toString().toStdString(),
	                                s.value("host").toString().toStdString(),
	                                s.value("port").toString().toStdString(),
	                                s.value("index").toString().toStdString(),
	                                s.value("importer").toString().toStdString(),
	                                s.value("format").toString().toStdString(),
	                                s.value("is_format_custom").toString().toStdString(),
	                                s.value("filter_path").toString().toStdString()};
	return res;
}