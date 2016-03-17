/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVMAPPINGPROPERTIES_H
#define INENDI_PVMAPPINGPROPERTIES_H

//#include <QList>

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVSerializeArchive.h>
#include <pvkernel/rush/PVFormat.h>

#include <inendi/PVAxis.h>
#include <inendi/PVRoot.h>
#include <inendi/PVMappingFilter.h>

namespace Inendi {

class PVMapping;

/**
* \class PVMappingProperties
*
* \brief Stored functions and variables that can to be modified by those functions
*/
class PVMappingProperties {
	friend class PVCore::PVSerializeObject;
	friend class PVMapping;
public:
	PVMappingProperties(PVRush::PVFormat const& fmt, PVCol idx);
	PVMappingProperties(PVRush::PVAxisFormat const& axis, PVCol idx);
protected:
	// For serialization
	PVMappingProperties() { _index = 0; }
public:
	void set_type(QString const& type, QString const& mode);
	void set_mode(QString const& mode);
	void set_args(PVCore::PVArgumentList const& args);
	PVCore::PVArgumentList const& get_args() const { return _args; }
	inline PVMappingFilter::p_type get_mapping_filter() const { assert(_mapping_filter); return _mapping_filter; }
	inline QString const& get_type() const { return _type; }
	inline QString const& get_mode() const { return _mode; }
	inline bool is_uptodate() const { return _is_uptodate; }

public:
	bool operator==(const PVMappingProperties& org);

protected:
	void serialize(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t v);
	void set_uptodate() { _is_uptodate = true; }
	inline void invalidate() { _is_uptodate = false; }
	void set_default_args(PVRush::PVAxisFormat const& axis);

private:
	void set_from_axis(PVRush::PVAxisFormat const& axis);
	void set_from_axis(Inendi::PVAxis const& axis);

private:
	PVCol _index;
	PVMappingFilter::p_type _mapping_filter;
	PVCore::PVArgumentList _args;
	QString _type;
	QString _mode;
	bool _is_uptodate;
};

}

#endif	/* INENDI_PVMAPPINGPROPERTIES_H */
