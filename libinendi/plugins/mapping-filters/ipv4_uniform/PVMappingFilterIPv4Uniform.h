/**
 * @file
 *
 * @copyright (C) Picviz Labs 2014-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVFILTER_PVMAPPINGFILTERIPV4UNIFORM_H
#define PVFILTER_PVMAPPINGFILTERIPV4UNIFORM_H

#include <inendi/PVMappingFilter.h>

#include <QString>
#include <QMetaType>

namespace Inendi {

class PVMappingFilterIPv4Uniform: public PVMappingFilter
{
public:

public:
	PVMappingFilterIPv4Uniform(PVCore::PVArgumentList const& args = PVMappingFilterIPv4Uniform::default_args());

public:
	void init() override;

	QString get_human_name() const override;
	PVCore::DecimalType get_decimal_type() const override;

	decimal_storage_type* operator()(PVCol const col, PVRush::PVNraw const& nraw) override;
	decimal_storage_type operator()(PVCore::PVField const& field) override;

	decimal_storage_type* finalize(PVCol const c,PVRush::PVNraw const& nraw) override;

protected:
	CLASS_FILTER(PVMappingFilterIPv4Uniform)
};

}

#endif // PVFILTER_PVMAPPINGFILTERIPV4UNIFORM_H