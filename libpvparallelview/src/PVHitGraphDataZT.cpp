#include <pvkernel/core/PVHardwareConcurrency.h>
#include <picviz/PVSelection.h>
#include <pvparallelview/PVBCode.h>
#include <pvparallelview/PVBCode.h>
#include <pvparallelview/PVHitGraphDataZT.h>
#include <pvparallelview/PVZoneTree.h>

#include <numa.h>
#include <omp.h>

static void hist_from_zt_zoom0(PVParallelView::PVZoneTree const& zt, uint32_t* buf_red)
{
	// Special case for level zoom 0 where there is only one block and only the number of
	// elements of each branches is needed (no need to go through each branches)
	
	PVParallelView::PVBCode bcode;
	for (uint32_t br = 0; br < (1UL<<PARALLELVIEW_ZT_BBITS); br++) {
		bcode.s.r = br;
		uint32_t sum = 0;
		for (uint32_t bl = 0; bl < (1UL<<PARALLELVIEW_ZT_BBITS); bl++) {
			bcode.s.l = bl;
			sum += zt.get_branch_count(bcode.int_v);
		}
		buf_red[br] = sum;
	}
}

static void hist_from_zt(PVParallelView::PVZoneTree const& zt, uint32_t const* plotted, uint32_t const y_min, uint32_t const zoom, int const nblocks, uint32_t* buf_red)
{
	assert(zoom >=1 && zoom <= 10);
	const int idx_shift = (32 - NBITS_INDEX) - zoom;
	const uint32_t idx_mask = (1 << NBITS_INDEX) - 1;
	const uint32_t mask_branch = ((1<<zoom) - 1) << (32 - zoom);

	const uint32_t branch_start = y_min & mask_branch;

	PVParallelView::PVBCode b_start;
	b_start.s.r = branch_start;
	b_start.s.l = 0;

	PVParallelView::PVBCode b_end;
	b_end.s.r = branch_start | (~mask_branch);
	b_end.s.l = (1<<NBITS_INDEX)-1;

	// The advantage of doing this on the right axis is that our
	// indexes are sequentials !
	for (uint32_t b = b_start.int_v; b < b_end.int_v; b++) {
		PVParallelView::PVZoneTree::PVBranch const& branch = zt.get_branch(b);
		PVRow const* rows = branch.p;
		for (size_t i = 0; i < branch.count; i++) {
			const PVRow r = rows[i];
			const uint32_t y_plotted = plotted[r];

			const uint32_t idx = (y_plotted >> idx_shift) & idx_mask;
			buf_red[idx]++;
		}
	}
}

static void hist_from_zt_one_branch(PVParallelView::PVZoneTree const& zt, uint32_t const* plotted, uint32_t const y_min, uint32_t const zoom, int const nblocks, uint32_t* buf_red)
{
	assert(zoom > PARALLELVIEW_ZT_BBITS);
}

//
// Public interfaces
//

void PVParallelView::PVHitGraphDataZT::process_all(PVZoneTree const& zt, uint32_t const* col_plotted, PVRow const nrows, uint32_t const y_min, int const zoom, int const block_start, int const nblocks)
{
	int nblocks_ = std::min(nblocks, PVHitGraphCommon::NBLOCKS - block_start);
	if (nblocks_ <= 0) {
		return;
	}

	if (zoom == 0) {
		assert(nblocks_ == 1);
		hist_from_zt_zoom0(zt, buffer_all().buffer_block(block_start));
		return;
	}

	if (zoom >=1 && zoom <= PARALLELVIEW_ZT_BBITS) {
		hist_from_zt(zt, col_plotted, y_min, zoom, nblocks, buffer_all().buffer_block(block_start));
		return;
	}

	hist_from_zt_one_branch(zt, col_plotted, y_min, zoom, nblocks, buffer_all().buffer_block(block_start));
}

void PVParallelView::PVHitGraphDataZT::process_sel(PVZoneTree const&, uint32_t const* col_plotted, PVRow const nrows, uint32_t const y_min, int const zoom, int const block_start, int const nblocks, Picviz::PVSelection const& sel)
{
}