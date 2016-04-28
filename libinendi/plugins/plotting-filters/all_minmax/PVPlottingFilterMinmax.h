/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVFILTER_PVPLOTTINGFILTERMINMAX_H
#define PVFILTER_PVPLOTTINGFILTERMINMAX_H

#include <pvkernel/core/general.h>
#include <inendi/PVPlottingFilter.h>

namespace Inendi
{

class PVPlottingFilterMinmax : public PVPlottingFilter
{
  public:
	uint32_t* operator()(mapped_decimal_storage_type const* value) override;
	void init_expand(uint32_t min, uint32_t max) override;
	uint32_t expand_plotted(uint32_t value) const override;
	QString get_human_name() const override { return QString("Min/max"); }
	bool can_expand() const override { return true; }

  private:
	uint32_t _expand_min;
	uint32_t _expand_max;
	uint32_t _expand_diff;

	CLASS_FILTER(PVPlottingFilterMinmax)
};
}

#endif
