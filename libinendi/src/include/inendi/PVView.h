/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVVIEW_H
#define INENDI_PVVIEW_H

#include <QList>
#include <QStringList>
#include <QString>
#include <QVector>
#include <QMutex>

#include <pvkernel/core/general.h>

#include <pvkernel/core/PVHSVColor.h>
#include <pvkernel/core/PVArgument.h>
#include <pvkernel/core/PVSerializeArchive.h>
#include <pvkernel/core/PVSerializeArchiveOptions_types.h>
#include <pvkernel/core/PVDataTreeObject.h>
#include <pvkernel/rush/PVExtractor.h>

#include <inendi/PVLinesProperties.h>
#include <inendi/PVMapped.h>
#include <inendi/PVPlotted.h>
#include <inendi/PVRoot.h>
#include <inendi/PVSource.h>
#include <inendi/PVEventline.h>
#include <inendi/PVLayerStack.h>
#include <inendi/PVIndexArray.h>
#include <inendi/PVSquareArea.h>
#include <inendi/PVStateMachine.h>
#include <inendi/PVSortingFunc.h>
#include <inendi/PVDefaultSortingFunc.h>
#include <inendi/PVPlotted.h>

#include <inendi/PVView_types.h>

namespace Inendi {

/**
 * \class PVView
 */
typedef typename PVCore::PVDataTreeObject<PVPlotted, PVCore::PVDataTreeNoChildren<PVView> > data_tree_view_t;
class PVView: public data_tree_view_t
{
	friend class PVCore::PVSerializeObject;
	friend class PVRoot;
	friend class PVScene;
	friend class PVSource;
	friend class PVCore::PVDataTreeAutoShared<PVView>;
public:
	typedef QHash<QString,PVCore::PVArgumentList> map_filter_arguments;
	typedef int32_t id_t;
	typedef PVAxesCombination::axes_comb_id_t axes_comb_id_t;

public:
	PVView();

public:
	~PVView();

protected:
	PVView(const PVView& org);

	// For PVSource
	void add_column(PVAxis const& axis);
	inline void set_view_id(id_t id) { _view_id = id; }

public:

	/* Variables */
	QString    name;

	/*! \brief PVView's specific axes combination
	 *  It is originaly copied from the parent's PVSource, and then become specific
	 *  to that view.
	 */
	PVAxesCombination axes_combination;

	PVCore::PVHSVColor default_zombie_line_properties;
	PVSelection floating_selection;
	PVLayer pre_filter_layer;
	PVLayer post_filter_layer;
	PVLayer layer_stack_output_layer;
	PVLayer output_layer;
	PVRow row_count;
	PVLayerStack layer_stack;
	PVSelection nu_selection;
	PVSelection real_output_selection;
	PVEventline eventline;
	PVSquareArea square_area;
	PVStateMachine *state_machine;
	PVSelection volatile_selection;
	int last_extractor_batch_size;

	inline PVSelection& get_floating_selection() { return floating_selection; }
	inline PVSelection& get_volatile_selection() { return volatile_selection; }

    // Proxy functions for PVHive
	void remove_column(PVCol index) { axes_combination.remove_axis(index); }
	bool move_axis_to_new_position(PVCol index_source, PVCol index_dest) { return axes_combination.move_axis_to_new_position(index_source, index_dest); }
	void axis_append(const PVAxis &axis) { axes_combination.axis_append(axis); }

	//void init_from_plotted(PVPlotted* parent, bool keep_layers);
	void set_fake_axes_comb(PVCol const ncols);

	virtual QString get_serialize_description() const { return "View: " + get_name(); }

	/* Functions */
	PVCol get_axes_count() const;

	template <class T>
	QList<PVCol> get_original_axes_index_with_tag(T const& tag) const
	{
		return axes_combination.get_original_axes_index_with_tag<T>(tag);
	}

	/**
	 * Gets the QStringList of all Axes names according to the current PVAxesCombination
	 *
	 * @return The list of all names of all current axes
	 *
	 */
	QStringList get_axes_names_list() const;
	QStringList get_zones_names_list() const;
	inline QStringList get_original_axes_names_list() const { return get_axes_combination().get_original_axes_names_list(); }
	
	/**
	 * Gets the name of the chosen axis according to the actual PVAxesCombination
	 *
	 * @param index The index of the axis (starts at 0)
	 *
	 * @return The name of that axis
	 *
	 */
	const QString& get_axis_name(PVCol index) const;
	QString get_axis_type(PVCol index) const;
	PVAxis const& get_axis(PVCol const comb_index) const;
	PVAxis const& get_axis_by_id(axes_comb_id_t const axes_comb_id) const;
	bool is_last_axis(axes_comb_id_t const axes_comb_id) const { return get_axes_combination().is_last_axis(axes_comb_id); }
	bool is_last_axis(PVCol const axis_comb) const { return axis_comb == get_column_count()-1; }

	const PVCore::PVHSVColor get_color_in_output_layer(PVRow index) const;
	PVCol get_column_count() const;
	float get_column_count_as_float();
	int get_layer_index(int index);
	float get_layer_index_as_float(int index);
	PVLayerStack &get_layer_stack();
	inline PVLayerStack const& get_layer_stack() const { return layer_stack; };
	int get_layer_stack_layer_n_locked_state(int n) const;;
	QString get_layer_stack_layer_n_name(int n) const;
	int get_layer_stack_layer_n_visible_state(int n) const;
	PVLayer &get_layer_stack_output_layer();
	PVLayer const& get_layer_stack_output_layer() const { return layer_stack_output_layer; }
	void hide_layers() { layer_stack.hide_layers(); }

	PVCol get_active_axis() const { assert(_active_axis < get_column_count()); return _active_axis; }
	PVStateMachine& get_state_machine() { return *state_machine; }
	PVStateMachine const& get_state_machine() const { return *state_machine; }

	PVAxesCombination const& get_axes_combination() const { return axes_combination; }
	PVAxesCombination& get_axes_combination() { return axes_combination; }
	void set_axes_combination_list_id(PVAxesCombination::columns_indexes_t const& idxes, PVAxesCombination::list_axes_t const& axes);

	inline PVLayer const& get_current_layer() const { return layer_stack.get_selected_layer(); }
	inline PVLayer& get_current_layer() { return layer_stack.get_selected_layer(); }

	inline void move_selected_layer_up() { layer_stack.move_selected_layer_up(); }
	inline void move_selected_layer_down() { layer_stack.move_selected_layer_down(); }

	inline PVCore::PVHSVColor const* get_output_layer_color_buffer() const { return output_layer.get_lines_properties().get_buffer(); }
	
	bool get_line_state_in_layer_stack_output_layer(PVRow index);
	bool get_line_state_in_layer_stack_output_layer(PVRow index) const;
	bool get_line_state_in_output_layer(PVRow index);
	bool get_line_state_in_output_layer(PVRow index) const;
	bool get_line_state_in_pre_filter_layer(PVRow index);
	bool get_line_state_in_pre_filter_layer(PVRow index) const;
	bool is_line_visible_listing(PVRow index) const;
	bool is_real_output_selection_empty() const;
	PVSelection const* get_selection_visible_listing() const;

	PVSelection &get_nu_selection();
	inline PVSelection const& get_nu_selection() const { return nu_selection; };
	int get_number_of_selected_lines();

	inline id_t get_view_id() const { return _view_id; }
	inline id_t get_display_view_id() const { return _view_id+1; }


	PVCol get_original_axes_count() const;
	QString get_original_axis_name(PVCol axis_id) const;
	QString get_original_axis_type(PVCol axis_id) const;
	inline PVCol get_original_axis_index(PVCol view_idx) const { return axes_combination.get_axis_column_index(view_idx); }

	PVLayer& get_output_layer();
	PVLayer const& get_output_layer() const { return output_layer; }

	PVRush::PVExtractor& get_extractor();

	QString get_name() const;
	QString get_window_name() const;

	void set_color(QColor color) { _color = color; }
	QColor get_color() const { return _color; }

	PVLayer &get_post_filter_layer();
	PVLayer &get_pre_filter_layer();

	PVSelection &get_real_output_selection();
	PVSelection const& get_real_output_selection() const;
	int get_real_row_index(int index);
	PVRow get_row_count() const;

	void reset_layers();

	int move_active_axis_closest_to_position(float x);
	PVCol get_active_axis_closest_to_position(float x);

	void expand_selection_on_axis(PVCol axis_id, QString const& mode);

	void set_active_axis_closest_to_position(float x);
	void set_axis_name(PVCol index, const QString &name_);
	
	void set_color_on_active_layer(const PVCore::PVHSVColor c);
	void set_color_on_post_filter_layer(const PVCore::PVHSVColor c);

	int set_layer_stack_layer_n_name(int n, QString const& name);

	void set_layer_stack_selected_layer_index(int index);

	void set_floating_selection(PVSelection &selection);

	//void set_selection_with_square_area_selection(PVSelection &selection, float xmin, float ymin, float xmax, float ymax);
	void set_selection_with_final_selection(PVSelection &selection);
	void set_selection_from_layer(PVLayer const& layer);
	void set_selection_view(PVSelection const& sel);

	int toggle_layer_stack_layer_n_locked_state(int n);
	int toggle_layer_stack_layer_n_visible_state(int n);
	void move_selected_layer_to(int new_index);

	void select_all_nonzb_lines();
	void select_no_line();
	void select_inv_lines();

	void toggle_listing_unselected_visibility();
	void toggle_listing_zombie_visibility();
	void toggle_view_unselected_zombie_visibility();
	bool& are_view_unselected_zombie_visible();

	void compute_layer_min_max(Inendi::PVLayer& layer);
	void compute_selectable_count(Inendi::PVLayer& layer);

	void recompute_all_selectable_count();

	/**
	 * do any process after a mapped load
	 */
	void finish_process_from_rush_pipeline();

/******************************************************************************
******************************************************************************
*
* functions to manipulate the Layers involved in the View
*
******************************************************************************
*****************************************************************************/

	void add_new_layer(QString name = QString());
	void add_new_layer_from_file(const QString& path);
	void delete_layer_n(int idx);
	void delete_selected_layer();
	void duplicate_selected_layer(const QString &name);
	void load_from_file(const QString& file);
	void commit_to_new_layer();
	void commit_selection_to_layer(PVLayer& layer);

	void load_post_to_pre();

	void process_from_eventline();
	void process_from_filter();
	QList<Inendi::PVView*> process_from_layer_stack();
	QList<Inendi::PVView*> process_from_selection();
	QList<Inendi::PVView*> process_real_output_selection();

	void process_eventline();
	void process_filter();
	void process_layer_stack();
	void process_selection();
	void process_visibility();

	void process_parent_plotted();
	void reset_view();

/******************************************************************************
******************************************************************************
*
* SPECIFIC functions
*
******************************************************************************
*****************************************************************************/

	void apply_filter_named_select_all();

	/**
	 * Gets the data using #PVAxesCombination
	 *
	 * @param row The row number
	 * @param column The column number
	 *
	 * @return a string containing wanted data
	 *
	 */
	std::string get_data(PVRow row, PVCol column) const;

	/**
	 * Gets the data directly from nraw, without #PVAxesCombination
	 *
	 * @param row The row number
	 * @param column The column number
	 *
	 * @return a string containing wanted data
	 *
	 */
	std::string get_data_raw(PVRow row, PVCol column) const { return get_rushnraw_parent().at_string(row, column); }


	void commit_volatile_in_floating_selection();
	
	/***********
	 * FILTERS
	 ***********/
	inline QString const& get_last_used_filter() const { return _last_filter_name; }
	inline void set_last_used_filter(QString const& name) { _last_filter_name = name; }
	inline bool is_last_filter_used_valid() const { return !_last_filter_name.isEmpty(); }
	inline PVCore::PVArgumentList& get_last_args_filter(QString const& name) { return filters_args[name]; }


	// Sorting functions
	void sort_indexes(PVCol col, pvcop::db::indexes& idxes, tbb::task_group_context* ctxt = NULL) const;

	// Helper functions for sorting
	inline void sort_indexes_with_axes_combination(PVCol column, pvcop::db::indexes& idxes, tbb::task_group_context* ctxt = NULL) const
	{
		sort_indexes(axes_combination.get_axis_column_index(column), idxes, ctxt);
	}

	std::weak_ptr<PVCore::PVSerializeObject> get_last_so() const { return _last_so; }
	void set_last_so(PVCore::PVSerializeObject_p const& so) { _last_so = std::weak_ptr<PVCore::PVSerializeObject>(so); }


/******************************************************************************
******************************************************************************
*
* ANCESTORS
*
******************************************************************************
*****************************************************************************/
	
	PVRush::PVNraw& get_rushnraw_parent() { assert(_rushnraw_parent); return *_rushnraw_parent; };
	PVRush::PVNraw const& get_rushnraw_parent() const { assert(_rushnraw_parent); return *_rushnraw_parent; };

	void debug();

	bool is_consistent() const ;
	void set_consistent(bool c);

	void recreate_mapping_plotting();

	PVCol get_real_axis_index(PVCol col) const;

	PVRow get_plotted_col_min_row(PVCol const combined_col) const;
	PVRow get_plotted_col_max_row(PVCol const combined_col) const;

public:
	// State machine
	inline void set_square_area_mode(PVStateMachine::SquareAreaModes mode) { state_machine->set_square_area_mode(mode); }

protected:
	void set_parent_from_ptr(PVPlotted* plotted);
/******************************************************************************
******************************************************************************
*
* Serialization
*
******************************************************************************
*****************************************************************************/
	void serialize_read(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t v);
	void serialize_write(PVCore::PVSerializeObject& so);
	PVSERIALIZEOBJECT_SPLIT

/******************************************************************************
******************************************************************************
*
* Initialisation
*
******************************************************************************
*****************************************************************************/
	void init_defaults();


protected:
	bool _is_consistent;
	QString _last_filter_name;
	map_filter_arguments filters_args;
	PVRush::PVNraw* _rushnraw_parent = nullptr;
	std::weak_ptr<PVCore::PVSerializeObject> _last_so;
	id_t _view_id;
	PVCol _active_axis;
	QColor _color;

	pvcop::db::collection* _collection;
};

typedef PVView::p_type PVView_p;

}

#endif	/* INENDI_PVVIEW_H */