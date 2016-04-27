/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVSOURCECREATOR_H
#define INENDI_PVSOURCECREATOR_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVRegistrableClass.h>
#include <pvkernel/core/PVClassLibrary.h>
#include <pvkernel/core/PVArgument.h>
#include <pvkernel/rush/PVRawSourceBase_types.h>
#include <pvkernel/rush/PVFormat.h>
#include <pvkernel/rush/PVInputDescription.h>
#include <pvkernel/rush/PVInputType.h>

#include <memory>
#include <list>

namespace PVRush
{

class PVSourceCreator : public PVCore::PVRegistrableClass<PVSourceCreator>
{
  public:
	typedef PVRush::PVRawSourceBase source_t;
	typedef std::shared_ptr<source_t> source_p;
	typedef std::shared_ptr<PVSourceCreator> p_type;

  public:
	virtual ~PVSourceCreator() {}

  public:
	virtual source_p create_source_from_input(PVInputDescription_p input,
	                                          const PVFormat& format) const = 0;
	virtual QString supported_type() const = 0;
	PVInputType_p supported_type_lib()
	{
		QString name = supported_type();
		PVInputType_p type_lib = LIB_CLASS(PVInputType)::get().get_class_by_name(name);
		return type_lib->clone<PVInputType>();
	}
	virtual QString name() const = 0;
	virtual hash_formats get_supported_formats() const = 0;

	// "pre-discovery" is called before processing the source into the TBB filters created
	// by its PVFormat objects. If this function returns false, this PVSourceCreator is
	// automatically
	// discared (for *all* its formats) for this input.
	virtual bool pre_discovery(PVInputDescription_p input) const = 0;
};

typedef PVSourceCreator::p_type PVSourceCreator_p;
}

#endif
