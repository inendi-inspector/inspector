/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVLogger.h>
#include "PVPlottingFilterMinmax.h"
#include <omp.h>

template <class T>
static void compute_minmax_plotting(pvcop::db::array const& mapped,
                                    pvcop::db::array const& minmax,
                                    uint32_t* dest)
{
	auto& mm = minmax.to_core_array<T>();
	double ymin = mm[0];
	double ymax = mm[1];

	if (ymin == ymax) {
		std::fill_n(dest, mm.size(), 0x80000000);
		return;
	}
	assert(ymax > ymin);

	// Use double to compute value to avoid rounding issue.
	// eg: if we use only uint32_t, with ymax - ymin > uint32_max / 2, ratio will be 1 and
	// no scaling will be applied
	const double ratio = std::numeric_limits<uint32_t>::max() / (ymax - ymin);
	auto& values = mapped.to_core_array<T>();
#pragma omp parallel for
	for (size_t i = 0; i < values.size(); i++) {
		dest[i] = ~uint32_t(((double)values[i] - ymin) * ratio);
	}
}

void Inendi::PVPlottingFilterMinmax::
operator()(pvcop::db::array const& mapped, pvcop::db::array const& minmax, uint32_t* dest)
{
	assert(dest);

	if (mapped.type() == pvcop::db::type_uint32) {
		compute_minmax_plotting<uint32_t>(mapped, minmax, dest);
	} else if (mapped.type() == pvcop::db::type_int32) {
		compute_minmax_plotting<int32_t>(mapped, minmax, dest);
	} else {
		compute_minmax_plotting<float>(mapped, minmax, dest);
	}
}