/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVPARALLELVIEW_PVHITGRAPHDATAINTERFACE_H
#define PVPARALLELVIEW_PVHITGRAPHDATAINTERFACE_H

#include <pvbase/types.h>

#include <pvparallelview/PVHitGraphCommon.h>
#include <pvparallelview/PVHitGraphBuffer.h>

#include <boost/noncopyable.hpp>

#include <cstddef>
#include <cstdint>

namespace Inendi
{
class PVSelection;
} // namespace Inendi

namespace PVParallelView
{

// Forward declarations
class PVZoneTree;

class PVHitGraphDataInterface : boost::noncopyable
{
  public:
	PVHitGraphDataInterface(uint32_t nbits, uint32_t nblocks);
	virtual ~PVHitGraphDataInterface();

  public:
	struct ProcessParams {
		ProcessParams(uint32_t const* col_plotted_,
		              PVRow const nrows_,
		              uint32_t const y_min_,
		              int const zoom_,
		              const int nbits_,
		              const double& alpha_,
		              int const block_start_,
		              int const nblocks_)
		    : col_plotted(col_plotted_)
		    , nrows(nrows_)
		    , y_min(y_min_)
		    , zoom(zoom_)
		    , nbits(nbits_)
		    , alpha(alpha_)
		    , block_start(block_start_)
		    , nblocks(nblocks_)
		{
		}

		uint32_t const* col_plotted;
		PVRow nrows;
		uint32_t y_min;
		int zoom;
		int nbits;
		double alpha;
		int block_start;
		int nblocks;
	};

  public:
	inline void process_buffer_all(ProcessParams const& params) { process_all(params, _buf_all); }
	inline void process_buffer_selected(ProcessParams const& params, Inendi::PVSelection const& sel)
	{
		process_sel(params, _buf_selected, sel);
	}
	inline void process_buffer_selectable(ProcessParams const& params,
	                                      Inendi::PVSelection const& sel)
	{
		process_sel(params, _buf_selectable, sel);
	}
	virtual void process_all_buffers(ProcessParams const& params,
	                                 Inendi::PVSelection const& layer_sel,
	                                 Inendi::PVSelection const& sel);

  public:
	void shift_left(const uint32_t nblocks, const double alpha);
	void shift_right(const uint32_t nblocks, const double alpha);

  public:
	PVHitGraphBuffer const& buffer_all() const { return _buf_all; }
	PVHitGraphBuffer const& buffer_selected() const { return _buf_selected; }
	PVHitGraphBuffer const& buffer_selectable() const { return _buf_selectable; }

	PVHitGraphBuffer& buffer_all() { return _buf_all; }
	PVHitGraphBuffer& buffer_selected() { return _buf_selected; }
	PVHitGraphBuffer& buffer_selectable() { return _buf_selectable; }

	void set_zero()
	{
		buffer_all().set_zero();
		buffer_selected().set_zero();
		buffer_selectable().set_zero();
	}

  public:
	inline uint32_t nbits() const { return buffer_all().nbits(); }
	inline uint32_t size_block() const { return buffer_all().size_block(); }
	inline uint32_t size_int() const { return buffer_all().size_int(); }
	inline uint32_t nblocks() const { return buffer_all().nblocks(); }

  protected:
	virtual void process_all(ProcessParams const& params, PVHitGraphBuffer& buf) const = 0;
	virtual void process_sel(ProcessParams const& params,
	                         PVHitGraphBuffer& buf,
	                         Inendi::PVSelection const& sel) const = 0;

  private:
	PVHitGraphBuffer _buf_all;
	PVHitGraphBuffer _buf_selected;
	PVHitGraphBuffer _buf_selectable;
};
} // namespace PVParallelView

#endif
