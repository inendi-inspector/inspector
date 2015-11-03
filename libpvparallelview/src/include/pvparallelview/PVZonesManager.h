/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVPARALLELVIEW_PVZONESMANAGER_H
#define PVPARALLELVIEW_PVZONESMANAGER_H

#include <QObject>

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVAlgorithms.h>

#include <picviz/PVPlotted.h>
#include <picviz/PVView_types.h>

#include <pvparallelview/PVZone.h>
#include <pvparallelview/PVZoneTree.h>
#include <pvparallelview/PVRenderingJob.h>

#include <boost/utility.hpp>

// Forward declarations
namespace Picviz {
class PVView;
class PVSelection;
}

namespace PVParallelView {

namespace __impl {
class ZoneCreation;
}

class PVZonesManager: public QObject, boost::noncopyable
{
	Q_OBJECT;

	friend class PVParallelView::__impl::ZoneCreation;

	typedef std::vector<PVZone> list_zones_t;
	typedef tbb::enumerable_thread_specific<PVZoneTree::ProcessData> process_ztree_tls_t;

public:
	PVZonesManager();

public:
	typedef Picviz::PVAxesCombination::axes_comb_id_t    axes_comb_id_t;
	typedef Picviz::PVAxesCombination::columns_indexes_t columns_indexes_t;

	void update_all();
	void reset_axes_comb();
	std::vector<PVZoneID> update_from_axes_comb(columns_indexes_t const& ac);
	std::vector<PVZoneID> update_from_axes_comb(Picviz::PVView const& view);
	void update_zone(PVZoneID zone);
	void reverse_zone(PVZoneID zone);
	void add_zone(PVZoneID zone);

	QSet<PVZoneID> list_cols_to_zones(QSet<PVCol> const& cols) const;

	void request_zoomed_zone(PVZoneID zone);

public:
	template <class Tree>
	inline Tree const& get_zone_tree(PVZoneID z) const
	{
		assert(z < get_number_of_managed_zones());
		return _zones[z].get_tree<Tree>();
	}

	template <class Tree>
	inline Tree& get_zone_tree(PVZoneID z)
	{
		assert(z < get_number_of_managed_zones());
		return _zones[z].get_tree<Tree>();
	}

	void filter_zone_by_sel(PVZoneID zone_id, const Picviz::PVSelection& sel);
	void filter_zone_by_sel_background(PVZoneID zone_id, const Picviz::PVSelection& sel);

public:
	void set_uint_plotted(Picviz::PVPlotted::uint_plotted_table_t const& plotted, PVRow nrows, PVCol ncols);
	void set_uint_plotted(Picviz::PVView const& view);
	void lazy_init_from_view(Picviz::PVView const& view);
	inline PVZoneID get_number_of_managed_zones() const { return _axes_comb.size()-1; }
	inline PVCol get_number_cols() const { return _ncols; }
	inline PVRow get_number_rows() const { return _nrows; }

	inline Picviz::PVPlotted::uint_plotted_table_t const& get_uint_plotted() const { assert(_uint_plotted); return *_uint_plotted; }

public:
	inline void get_zone_plotteds(PVZoneID const z, uint32_t const* *plotted_a, uint32_t const* *plotted_b) const
	{
		PVCol a, b;
		get_zone_cols(z, a, b);
		*plotted_a = Picviz::PVPlotted::get_plotted_col_addr(get_uint_plotted(), get_number_rows(), a);
		*plotted_b = Picviz::PVPlotted::get_plotted_col_addr(get_uint_plotted(), get_number_rows(), b);
	}

	inline void get_zone_cols(PVZoneID z, PVCol& a, PVCol& b) const
	{
		assert(z < get_number_of_managed_zones());
		a = _axes_comb[z].get_axis();
		b = _axes_comb[z+1].get_axis();
	}

signals:
	void filter_by_sel_finished(int zone_id, bool changed);

protected:
	Picviz::PVPlotted::uint_plotted_table_t const* _uint_plotted = NULL;
	PVRow _nrows = 0;
	PVCol _ncols = 0;
	columns_indexes_t _axes_comb;
	list_zones_t _zones;
	process_ztree_tls_t _tls_ztree;
	//list_zones_tree_t _quad_trees;
};



}

#endif
