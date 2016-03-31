/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVMAPPING_H
#define INENDI_PVMAPPING_H

#include <QList>
#include <QLibrary>
#include <QVector>

#include <pvkernel/core/PVDataTreeObject.h>

#include <pvkernel/rush/PVFormat.h>
#include <pvkernel/rush/PVNraw.h>

#include <inendi/general.h>
#include <inendi/PVMappingProperties.h>
#include <inendi/PVPtrObjects.h>
#include <inendi/PVMappingFilter.h>
#include <inendi/PVMandatoryMappingFilter.h>

#include <memory>

namespace Inendi {

class PVMapped;

/**
 * \class PVMapping
 */
class PVMapping
{
	friend class PVMapped;
	friend class PVCore::PVSerializeObject;
public:
	typedef std::shared_ptr<PVMapping> p_type;

public:
	PVMapping(PVMapped* mapped);

protected:
	// For serialization
	PVMapping();
	void serialize(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t v);

	// For PVMapped
	void set_parent(PVSource* src);
	void set_uptodate_for_col(PVCol j);

	void add_column(PVMappingProperties const& props);

public:
	float get_position(int column, QString const& value);
	bool is_uptodate() const;

public:
	void set_mapped(PVMapped* mapped) { _mapped = mapped; }
	PVMapped* get_mapped() { return _mapped; }

	void set_source(PVSource* src);

	PVRush::PVFormat const& get_format() const;

public:
	// Column properties
	PVMappingFilter::p_type get_filter_for_col(PVCol col);
	QString const& get_type_for_col(PVCol col) const;
	QString const& get_mode_for_col(PVCol col) const;
	PVMappingProperties const& get_properties_for_col(PVCol col) const { assert(col < columns.size()); return columns.at(col); }
	PVMappingProperties& get_properties_for_col(PVCol col) { assert(col < columns.size()); return columns[col]; }
	bool is_col_uptodate(PVCol j) const;
	PVCol get_number_cols() const { return columns.size(); }
	PVCore::DecimalType get_decimal_type_of_col(PVCol const j) const;

	QString const& get_name() const { return _name; }
	void set_name(QString const& name) { _name = name; }

	void reset_from_format(PVRush::PVFormat const& format);
	void set_default_args(PVRush::PVFormat const& format);

public:
	// Mandatory parameters
	mandatory_param_map const& get_mandatory_params_for_col(PVCol col) const;
	mandatory_param_map& get_mandatory_params_for_col(PVCol col);

protected:
	QVector<mandatory_param_map> _mandatory_filters_values;
	QList<PVMappingProperties> columns;

	QString _name;
	PVMapped* _mapped;
};

typedef PVMapping::p_type PVMapping_p;

}

#endif	/* INENDI_PVMAPPING_H */
