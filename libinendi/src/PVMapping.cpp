/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/debug.h>

#include <pvkernel/rush/PVFormat.h>
#include <inendi/PVMapping.h>
#include <inendi/PVMapped.h>
#include <inendi/PVSource.h>

#include <iostream>



/******************************************************************************
 *
 * Inendi::PVMapping::PVMapping
 *
 *****************************************************************************/
Inendi::PVMapping::PVMapping(PVMapped* mapped):
	_name("default"),
	_mapped(mapped)
{
	PVSource* source = _mapped->get_parent();

	PVCol naxes = source->get_column_count();

	if (naxes == 0) {
		PVLOG_ERROR("In PVMapping constructor, no axis have been defined in the format !!!!\n");
		assert(false);
	}

	_mandatory_filters_values.resize(naxes);

	PVLOG_DEBUG("In PVMapping::PVMapping(), debug PVFormat\n");
	source->get_rushnraw().get_format()->debug();
	for (PVCol i = 0; i < naxes; i++) {
		PVMappingProperties mapping_axis(*source->get_rushnraw().get_format(), i);
		add_column(mapping_axis);
		PVLOG_HEAVYDEBUG("%s: Add a column\n", __FUNCTION__);
	}
}

/**************************************get_mapping****************************************
 *
 * Inendi::PVMapping::PVMapping
 *
 *****************************************************************************/
Inendi::PVMapping::PVMapping() {}

/******************************************************************************
 *
 * Inendi::PVMapping::~PVMapping
 *
 *****************************************************************************/
Inendi::PVMapping::~PVMapping()
{
}



/******************************************************************************
 *
 * Inendi::PVMapping::add_column
 *
 *****************************************************************************/
void Inendi::PVMapping::add_column(PVMappingProperties const& props)
{

	columns.push_back(props);
	_mandatory_filters_values.push_back(mandatory_param_map());
}


/******************************************************************************
 *
 * Inendi::PVMapping::get_filter_for_col
 *
 *****************************************************************************/
Inendi::PVMappingFilter::p_type Inendi::PVMapping::get_filter_for_col(PVCol col)
{
	return columns.at(col).get_mapping_filter();
}

PVCore::DecimalType Inendi::PVMapping::get_decimal_type_of_col(PVCol const j) const
{
	assert(j < columns.size());
	return columns[j].get_mapping_filter()->get_decimal_type();
}

/******************************************************************************
 *
 * Inendi::PVMapping::get_format
 *
 *****************************************************************************/
PVRush::PVFormat_p Inendi::PVMapping::get_format() const
{
	return _mapped->get_parent()->get_rushnraw().get_format();
}



/******************************************************************************
 *
 * Inendi::PVMapping::get_group_key_for_col
 *
 *****************************************************************************/
QString Inendi::PVMapping::get_group_key_for_col(PVCol col) const
{
	return columns[col].get_group_key();
}



/******************************************************************************
 *
 * Inendi::PVMapping::get_mandatory_params_for_col
 *
 *****************************************************************************/
Inendi::mandatory_param_map const& Inendi::PVMapping::get_mandatory_params_for_col(PVCol col) const
{
	assert(col < _mandatory_filters_values.size());
	return _mandatory_filters_values[col];
}



/******************************************************************************
 *
 * Inendi::PVMapping::get_mandatory_params_for_col
 *
 *****************************************************************************/
Inendi::mandatory_param_map& Inendi::PVMapping::get_mandatory_params_for_col(PVCol col)
{
	assert(col < _mandatory_filters_values.size());
	return _mandatory_filters_values[col];
}



/******************************************************************************
 *
 * Inendi::PVMapping::get_mode_for_col
 *
 *****************************************************************************/
QString const& Inendi::PVMapping::get_mode_for_col(PVCol col) const
{
	assert(col < columns.size());
	return get_properties_for_col(col).get_mode();
}

/******************************************************************************
 *
 * Inendi::PVMapping::get_type_for_col
 *
 *****************************************************************************/
QString const& Inendi::PVMapping::get_type_for_col(PVCol col) const
{
	assert(col < columns.size());
	return get_properties_for_col(col).get_type();
}



/******************************************************************************
 *
 * Inendi::PVMapping::invalidate_all
 *
 *****************************************************************************/
void Inendi::PVMapping::invalidate_all()
{
	QList<PVMappingProperties>::iterator it;
	for (it = columns.begin(); it != columns.end(); it++) {
		it->invalidate();
	}
}

/******************************************************************************
 *
 * Inendi::PVMapping::validate_all
 *
 *****************************************************************************/
void Inendi::PVMapping::validate_all()
{
	QList<PVMappingProperties>::iterator it;
	for (it = columns.begin(); it != columns.end(); it++) {
		it->set_uptodate();
	}
}



/******************************************************************************
 *
 * Inendi::PVMapping::is_col_uptodate
 *
 *****************************************************************************/
bool Inendi::PVMapping::is_col_uptodate(PVCol j) const
{
	assert(j < columns.size());
	return get_properties_for_col(j).is_uptodate();
}



/******************************************************************************
 *
 * Inendi::PVMapping::is_uptodate
 *
 *****************************************************************************/
bool Inendi::PVMapping::is_uptodate() const
{
	QList<PVMappingProperties>::const_iterator it;
	for (it = columns.begin(); it != columns.end(); it++) {
		if (!it->is_uptodate()) {
			return false;
		}
	}
	return true;
}



/******************************************************************************
 *
 * Inendi::PVMapping::reset_from_format
 *
 *****************************************************************************/
void Inendi::PVMapping::reset_from_format(PVRush::PVFormat const& format)
{
	PVCol naxes = format.get_axes().size();
	if (columns.size() < naxes) {
		return;
	}

	for (PVCol i = 0; i < naxes; i++) {
		columns[i].set_from_axis(format.get_axes().at(i));
	}
}



/******************************************************************************
 *
 * Inendi::PVMapping::serialize
 *
 *****************************************************************************/
void Inendi::PVMapping::serialize(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t /*v*/)
{
	so.list("properties", columns);
	so.attribute("name", _name);
	if (!so.is_writing()) {
		_mandatory_filters_values.clear();
		_mandatory_filters_values.resize(columns.size());
	}
}



/******************************************************************************
 *
 * Inendi::PVMapping::set_default_args
 *
 *****************************************************************************/
void Inendi::PVMapping::set_default_args(PVRush::PVFormat const& format)
{
	QList<PVMappingProperties>::iterator it;
	PVCol i = 0;
	PVCol ncols = format.get_axes().size();
	for (it = columns.begin(); it != columns.end(); it++) {
		it->set_default_args(format.get_axes().at(i));
		i++;
		if (i >= ncols) {
			break;
		}
	}
}


/******************************************************************************
 *
 * Inendi::PVMapping::set_source
 *
 *****************************************************************************/
void Inendi::PVMapping::set_source(PVSource* src)
{
	PVCol naxes = src->get_column_count();
	_mandatory_filters_values.resize(naxes);
}

/******************************************************************************
 *
 * Inendi::PVMapping::set_uptodate_for_col
 *
 *****************************************************************************/
void Inendi::PVMapping::set_uptodate_for_col(PVCol j)
{
	assert(j < columns.size());
	get_properties_for_col(j).set_uptodate();
}


