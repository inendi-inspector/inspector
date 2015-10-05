/**
 * \file PVMappingFilterStringLogAlpha.h
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#ifndef PVFILTER_PVMAPPINGFILTERSTRINGDEFAULT_H
#define PVFILTER_PVMAPPINGFILTERSTRINGDEFAULT_H

#include <pvkernel/core/general.h>
#include <picviz/PVMappingFilter.h>
#include <tbb/atomic.h>

namespace Picviz {

class PVMappingFilterStringDefault: public PVMappingFilter
{
public:
	float* operator()(PVRush::PVNraw::const_trans_nraw_table_line const& values);

	CLASS_FILTER(PVMappingFilterStringDefault)
};

}

#endif
