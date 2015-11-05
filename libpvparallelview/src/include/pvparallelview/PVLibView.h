/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVPARALLELVIEW_PVLIBVIEW_H
#define PVPARALLELVIEW_PVLIBVIEW_H

#include <pvkernel/core/PVSharedPointer.h>

#include <inendi/PVAxesCombination.h>
#include <inendi/PVPlotting.h>
#include <inendi/PVView_types.h>

#include <pvhive/PVObserverSignal.h>
#include <pvhive/PVCallHelper.h>

#include <pvparallelview/PVZonesProcessor.h>
#include <pvparallelview/PVZonesManager.h>

#include <QObject>

#include <tbb/task.h>

namespace PVParallelView
{

class PVZonesManager;
class PVFuncObserverSignal;
class PVFullParallelScene;
class PVFullParallelView;
class PVZoomedParallelScene;
class PVZoomedParallelView;
class PVHitCountView;
class PVScatterView;
class PVSlidersManager;

class PVLibView
{
private:
	typedef std::list<PVFullParallelScene*> scene_list_t;
	typedef std::vector<PVZoomedParallelScene*> zoomed_scene_list_t;
	typedef std::vector<PVHitCountView*> hit_count_view_list_t;
	typedef std::vector<PVScatterView*> scatter_view_list_t;
	friend class process_selection_Observer;

public:
	PVLibView(Inendi::PVView_sp& view_sp);
	// For testing purposes
	PVLibView(Inendi::PVView_sp& view_sp, Inendi::PVPlotted::uint_plotted_table_t const& plotted, PVRow nrows, PVCol ncols);
	~PVLibView();

public:
	PVFullParallelView* create_view(QWidget* parent = NULL);
	PVZoomedParallelView* create_zoomed_view(PVCol const axis, QWidget* parent = NULL);
	PVHitCountView* create_hit_count_view(PVCol const axis, QWidget* parent = NULL);
	PVScatterView* create_scatter_view(PVCol const axis, QWidget* parent = NULL);

	void request_zoomed_zone_trees(const PVCol axis);
	PVZonesManager& get_zones_manager() { return _zones_manager; }
	Inendi::PVView* lib_view() { return _obs_view->get_object(); }

	void remove_view(PVFullParallelScene *scene);
	void remove_zoomed_view(PVZoomedParallelScene *scene);
	void remove_hit_count_view(PVHitCountView *view);
	void remove_scatter_view(PVScatterView *view);

protected:
	void selection_updated();
	void output_layer_updated();
	void layer_stack_output_layer_updated();
	void view_about_to_be_deleted();
	void axes_comb_about_to_be_updated();
	void axes_comb_updated();
	void plotting_updated();

protected:
	void common_init_view(Inendi::PVView_sp& view_sp);
	void common_init_zm();

private:
	PVZonesManager                            _zones_manager;
	PVCore::PVSharedPtr<PVSlidersManager>     _sliders_manager_p;
	PVHive::PVObserver_p<Inendi::PVLayer>     _obs_output_layer;
	PVHive::PVObserver_p<Inendi::PVLayer>     _obs_layer_stack_output_layer;
	PVHive::PVObserver_p<Inendi::PVSelection> _obs_sel;
	PVHive::PVObserver_p<Inendi::PVView>      _obs_view;
	PVHive::PVObserver_p<Inendi::PVAxesCombination::columns_indexes_t> _obs_axes_comb;
	PVHive::PVObserver_p<Inendi::PVPlotting>  _obs_plotting;
	scene_list_t                              _parallel_scenes;
	zoomed_scene_list_t                       _zoomed_parallel_scenes;
	hit_count_view_list_t                     _hit_count_views;
	scatter_view_list_t                       _scatter_views;
	PVCore::PVHSVColor                 const* _colors;

	PVZonesProcessor _processor_sel;
	PVZonesProcessor _processor_bg;
};

}

#endif /* PVPARALLELVIEW_PVLIBVIEW_H */
