/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvparallelview/PVSlidersManager.h>

#include <iostream>

/*****************************************************************************
 * PVParallelView::PVSlidersManager::PVSlidersManager
 *****************************************************************************/

PVParallelView::PVSlidersManager::PVSlidersManager()
{
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::~PVSlidersManager
 *****************************************************************************/

PVParallelView::PVSlidersManager::~PVSlidersManager()
{
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::new_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::new_selection_sliders(const axis_id_t& axis_id,
                                                             const id_t id,
                                                             const int64_t y_min,
                                                             const int64_t y_max)
{
	new_range_sliders(_selection_geometries, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::new_zoom_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::new_zoom_sliders(const axis_id_t& axis_id,
                                                        const id_t id,
                                                        const int64_t y_min,
                                                        const int64_t y_max)
{
	new_range_sliders(_zoom_geometries, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::new_zoomedselection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::new_zoomed_selection_sliders(const axis_id_t& axis_id,
                                                                    const id_t id,
                                                                    const int64_t y_min,
                                                                    const int64_t y_max)
{
	new_range_sliders(_zoomed_selection_geometries, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::del_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::del_selection_sliders(const axis_id_t& axis_id,
                                                             const id_t id)
{
	del_range_sliders(_selection_geometries, axis_id, id);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::del_zoom_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::del_zoom_sliders(const axis_id_t& axis_id, const id_t id)
{
	del_range_sliders(_zoom_geometries, axis_id, id);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::del_zoomed_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::del_zoomed_selection_sliders(const axis_id_t& axis_id,
                                                                    const id_t id)
{
	del_range_sliders(_zoomed_selection_geometries, axis_id, id);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::update_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::update_selection_sliders(const axis_id_t& axis_id,
                                                                const id_t id,
                                                                const int64_t y_min,
                                                                const int64_t y_max)
{
	update_range_sliders(_selection_geometries, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::update_zoom_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::update_zoom_sliders(const axis_id_t& axis_id,
                                                           const id_t id,
                                                           const int64_t y_min,
                                                           const int64_t y_max,
                                                           const ZoomSliderChange)
{
	// the last parameter is useless for the manager
	update_range_sliders(_zoom_geometries, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::update_zoomed_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::update_zoomed_selection_sliders(const axis_id_t& axis_id,
                                                                       const id_t id,
                                                                       const int64_t y_min,
                                                                       const int64_t y_max)
{
	update_range_sliders(_zoomed_selection_geometries, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::iterate_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::iterate_selection_sliders(
    const range_functor_t& functor) const
{
	iterate_range_sliders(_selection_geometries, functor);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::iterate_zoom_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::iterate_zoom_sliders(const range_functor_t& functor) const
{
	iterate_range_sliders(_zoom_geometries, functor);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::iterate_zoomed_selection_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::iterate_zoomed_selection_sliders(
    const range_functor_t& functor) const
{
	iterate_range_sliders(_zoomed_selection_geometries, functor);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::new_range_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::new_range_sliders(range_geometry_set_t& range,
                                                         const axis_id_t& axis_id,
                                                         const id_t id,
                                                         const int64_t y_min,
                                                         const int64_t y_max)
{
	update_range_sliders(range, axis_id, id, y_min, y_max);
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::del_range_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::del_range_sliders(range_geometry_set_t& range,
                                                         const axis_id_t& axis_id,
                                                         const id_t id)
{
	range_geometry_set_t::iterator ai = range.find(axis_id);
	if (ai != range.end()) {
		range_geometry_list_t::const_iterator ii = ai->second.find(id);
		if (ii != ai->second.end()) {
			ai->second.erase(ii);
		}
		if (ai->second.empty()) {
			range.erase(ai);
		}
	}
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::update_range_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::update_range_sliders(range_geometry_set_t& range,
                                                            const axis_id_t& axis_id,
                                                            const id_t id,
                                                            const int64_t y_min,
                                                            const int64_t y_max)
{
	range_geometry_t& geom = range[axis_id][id];
	geom.y_min = y_min;
	geom.y_max = y_max;
}

/*****************************************************************************
 * PVParallelView::PVSlidersManager::iterate_range_sliders
 *****************************************************************************/

void PVParallelView::PVSlidersManager::iterate_range_sliders(const range_geometry_set_t& range,
                                                             const range_functor_t& functor) const
{
	for (range_geometry_set_t::const_iterator it = range.begin(); it != range.end(); ++it) {
		for (range_geometry_list_t::const_iterator ait = it->second.begin();
		     ait != it->second.end(); ++ait) {
			functor(it->first, ait->first, ait->second);
		}
	}
}
