/**
 * @file
 *
 *
 * @copyright (C) ESI Group INENDI 2015-2015
 */

#include <pvbase/general.h>

#include "PVInputTypeElasticsearch.h"

#include "PVElasticsearchParamsWidget.h"

#include "../../common/elasticsearch/PVElasticsearchInfos.h"

PVRush::PVInputTypeElasticsearch::PVInputTypeElasticsearch()
    : PVInputTypeDesc<PVElasticsearchQuery>(), _is_custom_format(false)
{
}

bool PVRush::PVInputTypeElasticsearch::createWidget(hash_formats const& formats,
                                                    hash_formats& /*new_formats*/,
                                                    list_inputs& inputs,
                                                    QString& format,
                                                    PVCore::PVArgumentList& /*args_ext*/,
                                                    QWidget* parent) const
{
	connect_parent(parent);
	PVElasticsearchParamsWidget* params = new PVElasticsearchParamsWidget(this, formats, parent);
	if (params->exec() == QDialog::Rejected) {
		return false;
	}

	PVElasticsearchQuery* query = new PVElasticsearchQuery(params->get_query());

	PVInputDescription_p ind(query);
	inputs.push_back(ind);

	format = INENDI_BROWSE_FORMAT_STR;

	return true;
}

QString PVRush::PVInputTypeElasticsearch::name() const
{
	return QString("elasticsearch");
}

QString PVRush::PVInputTypeElasticsearch::human_name() const
{
	return QString("Elasticsearch import plugin");
}

QString PVRush::PVInputTypeElasticsearch::human_name_serialize() const
{
	return QString("Elasticsearch");
}

QString PVRush::PVInputTypeElasticsearch::internal_name() const
{
	return QString("04-elasticsearch");
}

QString PVRush::PVInputTypeElasticsearch::menu_input_name() const
{
	return QString("Elasticsearch...");
}

QString PVRush::PVInputTypeElasticsearch::tab_name_of_inputs(list_inputs const& in) const
{
	PVInputDescription_p query = in[0];
	return query->human_name();
}

bool PVRush::PVInputTypeElasticsearch::get_custom_formats(PVInputDescription_p /*in*/,
                                                          hash_formats& /*formats*/) const
{
	return false;
}

QKeySequence PVRush::PVInputTypeElasticsearch::menu_shortcut() const
{
	return QKeySequence();
}