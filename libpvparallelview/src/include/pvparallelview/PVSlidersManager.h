/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVPARALLELVIEW_PVSLIDERSMANAGER_H
#define PVPARALLELVIEW_PVSLIDERSMANAGER_H

#include <pvbase/types.h>

#include <picviz/PVAxesCombination.h>

#include <pvparallelview/common.h>
#include <pvparallelview/PVSlidersManager_types.h>

#include <map>

/* TODO: test if FuncObserver work with inline functions
 *
 * IDEA: allow to highlight a distant slider?
 *
 * IDEA: allow to hide/show a distant slider?
 */

namespace PVParallelView
{

class PVSlidersManager
{
public:
	typedef enum {
		ZoomSliderNone   = 0,
		ZoomSliderMin    = 1,
		ZoomSliderMax    = 2,
		ZoomSliderBoth   = 3 // 1 + 2
	} ZoomSliderChange;

	typedef Picviz::PVAxesCombination::axes_comb_id_t axis_id_t;
	typedef void*                                     id_t;

	struct range_geometry_t
	{
		int64_t y_min;
		int64_t y_max;
	};

	typedef std::function<void(const axis_id_t, const id_t,
	                           const range_geometry_t &)> range_functor_t;

public:
	PVSlidersManager();

public:
	~PVSlidersManager();

public:
	/**
	 * Function to observe (in PVHive way) to be notified when a new
	 * range sliders pair is added
	 *
	 * @param axis the axis the slider is associated with
	 * @param id the id the slider is associated with
	 * @param y_min the low position of the sliders
	 * @param y_max the high position of the sliders
	 */
	void new_selection_sliders(const axis_id_t &axis_id, const id_t id,
	                           const int64_t y_min, const int64_t y_max);
	void new_zoom_sliders(const axis_id_t &axis_id, const id_t id,
	                      const int64_t y_min, const int64_t y_max);
	void new_zoomed_selection_sliders(const axis_id_t &axis_id, const id_t id,
	                                  const int64_t y_min, const int64_t y_max);

	/**
	 * Function to observe (in PVHive way) to be notified when a new
	 * range sliders pair is deleted
	 *
	 * @param axis the axis the slider is associated with
	 * @param id the id the slider is associated with
	 */
	void del_selection_sliders(const axis_id_t &axis_id, const id_t id);
	void del_zoom_sliders(const axis_id_t &axis_id, const id_t id);
	void del_zoomed_selection_sliders(const axis_id_t &axis_id, const id_t id);

	/**
	 * Function to observe (in PVHive way) to be notified when a
	 * range sliders pair is changed
	 *
	 * @param axis the axis the slider is associated with
	 * @param id the id the slider is associated with
	 * @param y_min the low position of the sliders
	 * @param y_max the high position of the sliders
	 */
	void update_selection_sliders(const axis_id_t &axis_id, const id_t id,
	                              const int64_t y_min, const int64_t y_max);
	void update_zoom_sliders(const axis_id_t &axis_id, const id_t id,
	                         const int64_t y_min, const int64_t y_max,
	                         const ZoomSliderChange change);
	void update_zoomed_selection_sliders(const axis_id_t &axis_id, const id_t id,
	                                     const int64_t y_min, const int64_t y_max);

	/**
	 * Function to iterate on all range sliders
	 *
	 * @param functor the function called on each range sliders pair
	 *
	 * This method has to be used when creating a new graphical view
	 * to initialize it with each existing range sliders
	 */
	void iterate_selection_sliders(const range_functor_t &functor) const;
	void iterate_zoom_sliders(const range_functor_t &functor) const;
	void iterate_zoomed_selection_sliders(const range_functor_t &functor) const;

private:
	typedef std::map<id_t, range_geometry_t> range_geometry_list_t;
	typedef std::map<axis_id_t, range_geometry_list_t> range_geometry_set_t;

private:
	void new_range_sliders(range_geometry_set_t &range,
	                          const axis_id_t &axis_id, const id_t id,
	                          const int64_t y_min, const int64_t y_max);

	void del_range_sliders(range_geometry_set_t &range,
	                          const axis_id_t &axis_id, const id_t id);

	void update_range_sliders(range_geometry_set_t &range,
	                             const axis_id_t &axis_id, const id_t id,
	                             const int64_t y_min, const int64_t y_max);

	void iterate_range_sliders(const range_geometry_set_t &range,
	                              const range_functor_t &functor) const;

private:
	range_geometry_set_t _zoom_geometries;
	range_geometry_set_t _selection_geometries;
	range_geometry_set_t _zoomed_selection_geometries;
};

}

#endif // PVPARALLELVIEW_PVSLIDERSMANAGER_H
