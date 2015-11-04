/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVAlgorithms.h>
#include <pvkernel/core/PVPlainTextType.h>
#include <pvkernel/core/PVOriginalAxisIndexType.h>
#include <pvkernel/core/PVEnumType.h>
#include <pvkernel/core/PVProgressBox.h>
#include <pvkernel/core/PVColor.h>

#include <pvkernel/rush/PVNraw.h>

#include <pvkernel/widgets/PVAbstractRangePicker.h>

#include <pvguiqt/PVAbstractListStatsDlg.h>
#include <pvguiqt/PVLayerFilterProcessWidget.h>

#include <pvkernel/core/PVLogger.h>
#include <pvkernel/core/picviz_bench.h>
#include <pvguiqt/PVStringSortProxyModel.h>

#include <pvkernel/widgets/PVLayerNamingPatternDialog.h>

#include <pvhive/PVCallHelper.h>

#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>

#include <QComboBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>

/******************************************************************************
 * PVGuiQt::__impl::PVAbstractListStatsRangePicker
 *****************************************************************************/

static inline size_t freq_to_count_min(double value, double count)
{
	// see PVGuiQt::PVAbstractListStatsDlg::select_refresh(bool) for the formula
	if (value == 0.) {
		return 0;
	}
	return ceil((((int)(value * 10.) * 0.001) - 0.0005) * count);
}

static inline size_t freq_to_count_max(double value, double count)
{
	// see PVGuiQt::PVAbstractListStatsDlg::select_refresh(bool) for the formula
	if (value == 0.) {
		return 0;
	}
	return floor((((int)(value * 10.) * 0.001) + 0.0005) * count);
}

static inline double count_to_freq_min(size_t value, double count)
{
	return trunc((((double)value / count) * 1000.) + 0.5) / 10.;
}

static inline double count_to_freq_max(size_t value, double count)
{
	return trunc((((double)value / count) * 1000.) + 0.5) / 10.;
}

namespace PVGuiQt
{

namespace __impl
{

class PVAbstractListStatsRangePicker : public PVWidgets::PVAbstractRangePicker
{
public:
	PVAbstractListStatsRangePicker(size_t relative_min_count, size_t relative_max_count, size_t absolute_max_count, QWidget *parent = nullptr) :
		PVWidgets::PVAbstractRangePicker(relative_min_count, relative_max_count, parent),
		_relative_min_count(relative_min_count),
		_relative_max_count(relative_max_count),
		_absolute_max_count(absolute_max_count)
	{
		update_gradient();
	}

	double convert_to(const double& value) const
	{
		if (_use_percent_mode) {
			return value / max_count() * 100;
		}
		else {
			return value;
		}
	}

	double convert_from(const double& value) const
	{
		if (_use_percent_mode) {
			return value * max_count() / 100;
		}
		else {
			return value;
		}
	}

	void set_mode_count()
	{
		_use_percent_mode = false;
		_min_spinbox->use_floating_point(false);
		_max_spinbox->use_floating_point(false);

		disconnect_spinboxes_from_ranges();

		get_min_spinbox()->setDecimals(0);
		get_max_spinbox()->setDecimals(0);

		get_min_spinbox()->setSingleStep(1);
		get_max_spinbox()->setSingleStep(1);

		get_min_spinbox()->setSuffix("");
		get_max_spinbox()->setSuffix("");

		set_limits(_relative_min_count, _relative_max_count);
		set_range_min(convert_from(get_range_min()));
		set_range_max(convert_from(get_range_max()));

		connect_spinboxes_to_ranges();
	}

	void set_mode_percent()
	{
		_use_percent_mode = true;
		_min_spinbox->use_floating_point(true);
		_max_spinbox->use_floating_point(true);

		disconnect_spinboxes_from_ranges();

		get_min_spinbox()->setDecimals(1);
		get_max_spinbox()->setDecimals(1);

		get_min_spinbox()->setSingleStep(0.1);
		get_max_spinbox()->setSingleStep(0.1);

		get_min_spinbox()->setSuffix(" %");
		get_max_spinbox()->setSuffix(" %");

		use_absolute_max_count(_use_absolute_max_count);

		connect_spinboxes_to_ranges();
	}

	/**
	 * toggle between linear and logarithmic scales
	 */
	void use_logarithmic_scale(bool use_log)
	{
		double rmin = convert_to(get_range_min());
		double rmax = convert_to(get_range_max());

		_use_logarithmic_scale = use_log;
		update_gradient();

		set_range_max(rmax, true);
		set_range_min(rmin, true);
		update();
	}

	/**
	 * toggle between relative and absolute max count
	 */
	void use_absolute_max_count(bool use_count)
	{
		_use_absolute_max_count = use_count;
		update_gradient();

		double rmin = get_range_min();
		double rmax = get_range_max();

		set_limits(convert_to(_relative_min_count), convert_to(_relative_max_count));

		set_range_max(convert_to(rmax), true);
		set_range_min(convert_to(rmin), true);
		update();
	}

	void update_gradient()
	{
		QLinearGradient gradient;

		QLinearGradient linear_gradient;
		double ratio1, ratio2, ratio3;
		QColor color;

		if (_use_logarithmic_scale && _relative_min_count != _relative_max_count) { // If only one value, use linear scale to avoid divisions by 0.
			ratio1 = PVCore::log_scale((double) _relative_min_count, 0., max_count());
			ratio3 = PVCore::log_scale((double) _relative_max_count, 0., max_count());
			ratio2 = ratio1 + (ratio3 - ratio1) / 2;
		}
		else {
			ratio1 = (double) _relative_min_count / max_count();
			ratio2 = (_relative_min_count + (double) (_relative_max_count - _relative_min_count) / 2) / max_count();
			ratio3 = (double) _relative_max_count / max_count();
		}

		color = QColor::fromHsv((ratio1) * (0 - 120) + 120, 255, 255);
		gradient.setColorAt(0, color);

		color = QColor::fromHsv((ratio2) * (0 - 120) + 120, 255, 255);
		gradient.setColorAt(0.5, color);

		color = QColor::fromHsv((ratio3) * (0 - 120) + 120, 255, 255);
		gradient.setColorAt(1, color);

		_range_ramp->set_gradient(gradient);
	}

protected:
	double map_from_spinbox(const double& value) const override
	{
		if (_use_logarithmic_scale) {
			double v = convert_from(value);
			if (_use_percent_mode) {
				v = std::max(convert_from(value-convert_to(_relative_min_count)), 1.0);
			}
			return PVCore::log_scale(v, _relative_min_count, _relative_max_count);
		} else {
			return PVWidgets::PVAbstractRangePicker::map_from_spinbox(convert_from(value));
		}
	}

	double map_to_spinbox(const double& value) const override
	{

		if (_use_logarithmic_scale) {
			return convert_to(PVCore::inv_log_scale(value, _relative_min_count, _relative_max_count));
		} else {
			return convert_to(PVWidgets::PVAbstractRangePicker::map_to_spinbox(value));
		}
	}

private:
	inline size_t max_count() const { return _use_absolute_max_count ? _absolute_max_count : _relative_max_count; }

protected:
	size_t _relative_min_count;
	size_t _relative_max_count;
	size_t _absolute_max_count;
	bool   _use_logarithmic_scale = true;
	bool   _use_absolute_max_count = true;
	bool   _use_percent_mode = false;
};

}

}

/******************************************************************************
 *
 * PVGuiQt::PVAbstractListStatsDlg
 *
 *****************************************************************************/
void PVGuiQt::PVAbstractListStatsDlg::init(Picviz::PVView_sp& view)
{
	PVHive::get().register_observer(view, _obs);
	PVHive::get().register_actor(view, _actor);
	_obs.connect_about_to_be_deleted(this, SLOT(deleteLater()));

	QString search_multiples = "search-multiple";
	Picviz::PVLayerFilter::p_type search_multiple = LIB_CLASS(Picviz::PVLayerFilter)::get().get_class_by_name(search_multiples);
	Picviz::PVLayerFilter::p_type fclone = search_multiple->clone<Picviz::PVLayerFilter>();
	Picviz::PVLayerFilter::hash_menu_function_t const& entries = fclone->get_menu_entries();
	Picviz::PVLayerFilter::hash_menu_function_t::const_iterator it_ent;
	for (it_ent = entries.begin(); it_ent != entries.end(); it_ent++) {
		QAction* act = new QAction(it_ent->key(), _values_view);
		act->setData(QVariant(search_multiples)); // Save the name of the layer filter associated to this action
		_ctxt_menu->addAction(act);

		// RH: a little hack. See comment of _msearch_action_for_layer_creation in .h
		if (act->text() == QString("Search for this value")) {
			_msearch_action_for_layer_creation = act;
		}
	}

	__impl::PVTableViewResizeEventFilter* table_view_resize_event_handler = new __impl::PVTableViewResizeEventFilter();

	Picviz::PVSortingFunc_p sf = view->get_sort_plugin_for_col(_col);
	PVStringSortProxyModel* proxy_model = static_cast<PVStringSortProxyModel*>(_values_view->model());
	proxy_model->set_qt_order_func(sf->qt_f_lesser());

	sort_by_column(0);

	_values_view->installEventFilter(table_view_resize_event_handler);
	connect(table_view_resize_event_handler, SIGNAL(resized()), this, SLOT(view_resized()));
	_values_view->horizontalHeader()->show();
	_values_view->verticalHeader()->show();
	_values_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	_values_view->setAlternatingRowColors (true);
	connect(_values_view->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(section_resized(int, int, int)));
	_values_view->setItemDelegateForColumn(1, new __impl::PVListStringsDelegate(this));

	// Add content for right click menu on vertical headers
	_hhead_ctxt_menu = new QMenu(this);
	connect(_values_view->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(show_hhead_ctxt_menu(const QPoint&)));

	QActionGroup* act_group_scale = new QActionGroup(this);
	act_group_scale->setExclusive(true);
	connect(act_group_scale, SIGNAL(triggered(QAction*)), this, SLOT(scale_changed(QAction*)));
	_act_toggle_linear = new QAction("Linear scale", act_group_scale);
	_act_toggle_linear->setCheckable(true);
	_act_toggle_log = new QAction("Logarithmic scale", act_group_scale);
	_act_toggle_log->setCheckable(true);
	_hhead_ctxt_menu->addAction(_act_toggle_linear);
	_hhead_ctxt_menu->addAction(_act_toggle_log);
	_hhead_ctxt_menu->addSeparator();

	QActionGroup* act_group_max = new QActionGroup(this);
	act_group_max->setExclusive(true);
	connect(act_group_max, SIGNAL(triggered(QAction*)), this, SLOT(max_changed(QAction*)));
	_act_toggle_absolute = new QAction("Absolute max", act_group_max);
	_act_toggle_absolute->setCheckable(true);
	_act_toggle_absolute->setChecked(true);
	_act_toggle_relative = new QAction("Relative max", act_group_max);
	_act_toggle_relative->setCheckable(true);
	_hhead_ctxt_menu->addAction(_act_toggle_absolute);
	_hhead_ctxt_menu->addAction(_act_toggle_relative);
	_hhead_ctxt_menu->addSeparator();

	_act_show_count = new QAction("Count", _hhead_ctxt_menu);
	_act_show_count->setCheckable(true);
	_act_show_count->setChecked(true);
	_act_show_scientific_notation = new QAction("Scientific notation", _hhead_ctxt_menu);
	_act_show_scientific_notation->setCheckable(true);
	_act_show_percentage = new QAction("Percentage", _hhead_ctxt_menu);
	_act_show_percentage->setCheckable(true);
	_act_show_percentage->setChecked(true);

	_hhead_ctxt_menu->addAction(_act_show_count);
	_hhead_ctxt_menu->addAction(_act_show_scientific_notation);
	_hhead_ctxt_menu->addAction(_act_show_percentage);

	//_values_view->setShowGrid(false);
	//_values_view->setStyleSheet("QTableView::item { border-left: 1px solid grey; }");

	QHBoxLayout *hbox = new QHBoxLayout();

	_select_groupbox->setLayout(hbox);

	QVBoxLayout* vl = new QVBoxLayout();

	hbox->addLayout(vl, 1);

	QRadioButton* r1 = new QRadioButton("by count");
	QRadioButton* r2 = new QRadioButton("by frequency");

	vl->addWidget(r1);
	vl->addWidget(r2);

	connect(r1, SIGNAL(toggled(bool)), this, SLOT(select_set_mode_count(bool)));
	connect(r2, SIGNAL(toggled(bool)), this, SLOT(select_set_mode_frequency(bool)));

	QPushButton *b = new QPushButton("Select");
	connect(b, SIGNAL(clicked(bool)), this, SLOT(select_refresh(bool)));

	vl->addWidget(b);

	_select_picker = new __impl::PVAbstractListStatsRangePicker(relative_min_count(), relative_max_count(), absolute_max_count());
	_select_picker->use_logarithmic_scale(_use_logarithmic_scale);
	hbox->addWidget(_select_picker, 2);

	// set default mode to "count"
	r1->click();

	// propagate the scale mode
	_act_toggle_log->setChecked(_use_logarithmic_scale);

	// Copy values menu
	_copy_values_menu = new QMenu();
	_copy_values_act->setMenu(_copy_values_menu);
	_copy_values_with_count_act = new QAction("with count", this);
	_copy_values_menu->addAction(_copy_values_with_count_act);
	_copy_values_without_count_act = new QAction("without count", this);
	_copy_values_menu->addAction(_copy_values_without_count_act);

	// layer creation actions
	_create_layer_with_values_act = new QAction("Create one layer with those values", _values_view);
	_ctxt_menu->addAction(_create_layer_with_values_act);
	_create_layers_for_values_act = new QAction("Create layers from those values", _values_view);
	_ctxt_menu->addAction(_create_layers_for_values_act);
}

PVGuiQt::PVAbstractListStatsDlg::~PVAbstractListStatsDlg()
{
	// Force deletion so that the internal std::vector is destroyed!
	model()->deleteLater();
}

/******************************************************************************
 * show_hhead_ctxt_menu
 *****************************************************************************/

void PVGuiQt::PVAbstractListStatsDlg::show_hhead_ctxt_menu(const QPoint& pos)
{
   // Show the menu if we click on the second column
   if (_values_view->horizontalHeader()->logicalIndexAt(pos) == 1) {
	   //  Menu appear next to the mouse
	_hhead_ctxt_menu->exec(QCursor::pos());
   }
}

bool PVGuiQt::PVAbstractListStatsDlg::process_context_menu(QAction* act)
{
	if (PVListDisplayDlg::process_context_menu(act)) {
		return true;
	}

	if (act == _copy_values_with_count_act) {
		_copy_count = true;
		copy_selected_to_clipboard();
		return true;
	}

	if (act == _copy_values_without_count_act) {
		_copy_count = false;
		copy_selected_to_clipboard();
		return true;
	}

	if (act == _create_layer_with_values_act) {
		create_layer_with_selected_values();
		return true;
	}

	if (act == _create_layers_for_values_act) {
		create_layers_for_selected_values();
		return true;
	}

	if (act) {
		QStringList values;
		for (const auto& index : _values_view->selectionModel()->selection().indexes()) {
			if (index.column() != 0) {
				continue;
			}
			values.append(index.data().toString());
		}

		multiple_search(act, values);
		return true;
	}

	return false;
}

void PVGuiQt::PVAbstractListStatsDlg::scale_changed(QAction* act)
{
	if (act) {
		_use_logarithmic_scale = (act == _act_toggle_log);
		_select_picker->use_logarithmic_scale(_use_logarithmic_scale);
		((__impl::PVAbstractListStatsModel*) model())->use_logarithmic_scale(_use_logarithmic_scale);
		_values_view->update();
		_values_view->horizontalHeader()->viewport()->update();
	}
}

void PVGuiQt::PVAbstractListStatsDlg::max_changed(QAction* act)
{
	if (act) {
		_use_absolute_max_count = (act == _act_toggle_absolute);
		_use_logarithmic_scale = (act == _act_toggle_absolute);
		_act_toggle_linear->setChecked(act == _act_toggle_relative);
		_act_toggle_log->setChecked(act == _act_toggle_absolute);
		_select_picker->use_logarithmic_scale(act == _act_toggle_absolute);
		((__impl::PVAbstractListStatsModel*) model())->use_logarithmic_scale(act == _act_toggle_absolute);
		_select_picker->use_absolute_max_count(act == _act_toggle_absolute);
		((__impl::PVAbstractListStatsModel*) model())->use_absolute_max_count(act == _act_toggle_absolute);
		_values_view->update();
		_values_view->horizontalHeader()->viewport()->update();
	}
}

void PVGuiQt::PVAbstractListStatsDlg::select_set_mode_count(bool checked)
{
	if (checked) {
		_select_picker->set_mode_count();
		_select_is_count = true;
	}
}

void PVGuiQt::PVAbstractListStatsDlg::select_set_mode_frequency(bool checked)
{
	if (checked) {
		_select_picker->set_mode_percent();
		_select_is_count = false;
	}
}

void PVGuiQt::PVAbstractListStatsDlg::select_refresh(bool)
{
	/**
	 * As percentage are rounded to be displayed using "%.1f", the entries
	 * can also not be selected using their exact values but using their
	 * rounded ones.
	 *
	 * So that, the nice formula to get the count values corresponding to
	 * the displayed percentage are (in LaTeX):
	 * - v_{min} = \lceil N × ( \frac{ \lfloor 10 × p_{min} \rfloor}{1000} - \frac{5}{10000} ) \rceil
	 * - v_{max} = \lfloor N × ( \frac{ \lfloor 10 × p_{max} \rfloor}{1000} + \frac{5}{10000} ) \rfloor
	 * where:
	 * - p_{min} is the lower bound percentage
	 * - p_{max} is the upper bound percentage
	 * - N is the events count
	 */
	uint64_t vmin;
	uint64_t vmax;
	if (_select_is_count) {
		vmin = _select_picker->get_range_min();
		vmax = _select_picker->get_range_max();
	}
	else {
		vmin = freq_to_count_min(count_to_freq_min(_select_picker->get_range_min(), max_count()), max_count());
		vmax = freq_to_count_max(count_to_freq_max(_select_picker->get_range_max(), max_count()), max_count());
	}

	QAbstractItemModel* data_model = _values_view->model();
	QItemSelectionModel* sel_model = _values_view->selectionModel();

	int row_count = data_model->rowCount();

	sel_model->clear();

	PVCore::PVProgressBox* pbox = new PVCore::PVProgressBox(QObject::tr("Computing selection..."), this);
	pbox->set_enable_cancel(true);
	tbb::task_group_context ctxt(tbb::task_group_context::isolated);

	// Use our own selection because QItemSelection::select(...) crashes due to the persistent indexes' update stored in the shared model
	typedef std::vector<std::pair<QModelIndex, QModelIndex>> selection_t;

	selection_t sel;

	BENCH_START(select_values);

	// See https://bugreports.qt-project.org/browse/QTBUG-25904

	bool res = PVCore::PVProgressBox::progress([&sel, &data_model, vmin, vmax, row_count, &ctxt]
	{
		const size_t nthreads = PVCore::PVHardwareConcurrency::get_physical_core_number();

		sel = tbb::parallel_reduce(
			tbb::blocked_range<int>(0, row_count, std::max(nthreads, row_count / nthreads)),
			selection_t(),
			[&](const tbb::blocked_range<int>& range, selection_t sel) -> selection_t const {

				sel.reserve(sel.size() + (range.end() - range.begin()));

				// Compute our own ranges to minimise the QItemSelectionRange to be created

				int first_row = -1;
				int last_row = -1;

				for (int row = range.begin(); row < range.end(); row++) {

					const uint64_t v = data_model->index(row, 1).data(Qt::UserRole).toULongLong();

					if ((v >= vmin) && (v <= vmax)) {
						if (first_row == -1) {
							first_row = row;
						}
						last_row = row;
					}
					else {
						if (first_row != -1) {
							sel.emplace_back(data_model->index(first_row, 0), data_model->index(last_row, 0));
						}
						first_row = -1;
						last_row = -1;
					}
				}

				// last range
				if (first_row != -1) {
					sel.emplace_back(data_model->index(first_row, 0), data_model->index(last_row, 0));
				}

				return sel;
			},
			[](selection_t sel1, selection_t sel2) -> selection_t {
				sel1.insert(sel1.end(), sel2.begin(), sel2.end());
				return sel1;
			});

		return true;
	}, ctxt, pbox);

	BENCH_END(select_values, "select_values", 0, 0, 1, row_count);


	BENCH_START(compute_item_selection);

	if (res) {
		QItemSelection item_selection;

		for (auto& range : sel) {
			item_selection.select(std::move(range.first), std::move(range.second));
		}

		sel_model->select(item_selection, QItemSelectionModel::Select);
	}

	BENCH_END(compute_item_selection, "compute_item_selection", 0, 0, 1, row_count);
}

void PVGuiQt::PVAbstractListStatsDlg::showEvent(QShowEvent * event)
{
	PVListDisplayDlg::showEvent(event);
	resize_section();
}

void PVGuiQt::PVAbstractListStatsDlg::view_resized()
{
	// We don't want the resize of the view to change the stored last section width
	_store_last_section_width = false;
	resize_section();
}

void PVGuiQt::PVAbstractListStatsDlg::resize_section()
{
	_values_view->horizontalHeader()->resizeSection(0, _values_view->width() - _last_section_width);
}

void PVGuiQt::PVAbstractListStatsDlg::section_resized(int logicalIndex, int /*oldSize*/, int newSize)
{
	if (logicalIndex == 1) {
		if (_store_last_section_width) {
			_last_section_width = newSize;
		}
		_store_last_section_width = true;
	}
}

void PVGuiQt::PVAbstractListStatsDlg::sort_by_column(int col)
{
	PVListDisplayDlg::sort_by_column(col);

	if (col == 1) {
		Qt::SortOrder order =  (Qt::SortOrder)!((bool)_values_view->horizontalHeader()->sortIndicatorOrder());
		proxy_model()->sort(col, order);
	}
}

void PVGuiQt::PVAbstractListStatsDlg::multiple_search(QAction* act, const QStringList &sl,
                                                      bool hide_dialog)
{

	// Get the filter associated with that menu entry
	QString filter_name = act->data().toString();
	Picviz::PVLayerFilter_p lib_filter = LIB_CLASS(Picviz::PVLayerFilter)::get().get_class_by_name(filter_name);
	if (!lib_filter) {
		PVLOG_ERROR("(listing context-menu) filter '%s' does not exist !\n", qPrintable(filter_name));
		return;
	}

	Picviz::PVLayerFilter::hash_menu_function_t entries = lib_filter->get_menu_entries();
	QString act_name = act->text();
	if (entries.find(act_name) == entries.end()) {
		PVLOG_ERROR("(listing context-menu) unable to find action '%s' in filter '%s'.\n", qPrintable(act_name), qPrintable(filter_name));
		return;
	}
	Picviz::PVLayerFilter::ctxt_menu_f args_f = entries[act_name];

	// Set the arguments
	_ctxt_args = lib_view()->get_last_args_filter(filter_name);

	PVCore::PVArgumentList custom_args = args_f(0U, 0, _col, sl.join("\n"));
	PVCore::PVArgumentList_set_common_args_from(_ctxt_args, custom_args);

	// Show the layout filter widget
	Picviz::PVLayerFilter_p fclone = lib_filter->clone<Picviz::PVLayerFilter>();
	assert(fclone);
	if (_ctxt_process) {
		_ctxt_process->deleteLater();
	}

	// Creating the PVLayerFilterProcessWidget will save the current args for this filter.
	// Then we can change them !
	_ctxt_process = new PVGuiQt::PVLayerFilterProcessWidget(lib_view(), _ctxt_args, fclone, _values_view);

	if (hide_dialog) {
		connect(_ctxt_process, SIGNAL(accepted()), this, SLOT(close()));
	}

	if (custom_args.get_edition_flag()) {
		_ctxt_process->show();
	} else {
		_ctxt_process->save_Slot();
	}
}

void PVGuiQt::PVAbstractListStatsDlg::ask_for_copying_count()
{
	if (QMessageBox::question(this, tr("Copy count values"), tr("Do you want to copy count values as well?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
		_copy_count = true;
	}
	else {
		_copy_count = false;
	}
}

QString PVGuiQt::PVAbstractListStatsDlg::export_line(
	PVGuiQt::PVStringSortProxyModel* model,
	std::function<QModelIndex(int)> f,
	int i
)
{
	static QString sep(",");
	static QString escaped_quote("\"\"");
	static QString quote("\"");
	QString s;

	// Get the indice for the i-th elements
	// It could be the i-th element of the model or the i-th element of the
	// selection depending on the f function.
	QModelIndex idx1 = f(i);

	if(idx1.column() == 0) {

		if (likely(idx1.isValid())) {

			QString value = idx1.data(Qt::DisplayRole).toString();

			if (!_copy_count) {
				s.append(value);
			}
			else {
				// Escape quotes
				value.replace(quote, escaped_quote);
				s.append(quote + value + quote + sep);

				// Extract data from second column of i-th element
				QModelIndex idx2 = model->index(idx1.row(), 1, QModelIndex());
				uint64_t occurence_count = idx2.data(Qt::UserRole).toULongLong();

				double ratio = (double) occurence_count / max_count();

				if (_act_show_count->isChecked()) {
					s.append(quote + __impl::PVAbstractListStatsModel::format_occurence(occurence_count) + quote + sep);
				}
				if (_act_show_scientific_notation->isChecked()) {
					s.append(quote + __impl::PVAbstractListStatsModel::format_scientific_notation(ratio) + quote + sep);
				}
				if (_act_show_percentage->isChecked()) {
					s.append(quote + __impl::PVAbstractListStatsModel::format_percentage(ratio) + quote + sep);
				}

				if (s.endsWith(sep)) {
					return s.left(s.size()-1);
				}
			}
		}
	}

	return s;
}

/******************************************************************************
 * PVGuiQt::PVAbstractListStatsDlg::create_layer_with_selected_values
 *****************************************************************************/

void PVGuiQt::PVAbstractListStatsDlg::create_layer_with_selected_values()
{
	PVWidgets::PVLayerNamingPatternDialog dlg("create one new layer with all selected values",
	                                          "string format for new layer's name",
	                                          "%v", PVWidgets::PVLayerNamingPatternDialog::ON_TOP,
	                                          this);

	if (dlg.exec() == QDialog::Rejected) {
		return;
	}

	QString text = dlg.get_name_pattern();
	PVWidgets::PVLayerNamingPatternDialog::insert_mode mode = dlg.get_insertion_mode();

	Picviz::PVView_sp view_sp = lib_view()->shared_from_this();
	PVHive::PVActor<Picviz::PVView> actor;
	PVHive::get().register_actor(view_sp, actor);
	Picviz::PVLayerStack &ls = view_sp->get_layer_stack();

	text.replace("%l", ls.get_selected_layer().get_name());
	text.replace("%a", view_sp->get_axes_combination().get_axis(_col).get_name());

	QStringList sl;

	/* as a selection is never sorted, we have to do it ourselves to have
	 * the values in the same order than the view's one
	 */
	QModelIndexList indexes = _values_view->selectionModel()->selection().indexes();
	qSort(indexes.begin(), indexes.end(), [](const QModelIndex& a, const QModelIndex& b) -> bool { return a.row() < b.row(); });

	QStringList value_names;

	for(const auto& index : indexes) {
		if (index.column() != 0) {
			continue;
		}
		QString s = index.data().toString();
		sl += s;
		if (s.isEmpty()) {
			value_names += "(empty)";
		} else {
			value_names += s;
		}
	}
	text.replace("%v", value_names.join(","));

	/*
	 * The process is little bit heavy:
	 * - backup the current layer's index
	 * - backup the current selection (the pre_filter_layer's one...
	 *   not volatile/floating/whatever)
	 * - run the multiple-search
	 * - create a new layer (do not for forget to notify the layerstackview
	 *   through the hive)
	 * - need to hide the newly created layer (it's better:)
	 * - need to move the newly created layer to the right place
	 * - do a commit from the selection to the newly created layer (like a
	 *   Alt-k and through the hive)
	 * - restore the (old) current layer's index (through the hive)
	 * - set the volatile selection to the backed-up one and force
	 *   reprocessing...
	 */
	int old_selected_layer_index = ls.get_selected_layer_index();
	Picviz::PVSelection old_sel(view_sp->get_pre_filter_layer().get_selection());

	multiple_search(_msearch_action_for_layer_creation, sl, false);

	actor.call<FUNC(Picviz::PVView::add_new_layer)>(text);
	Picviz::PVLayer &layer = view_sp->get_layer_stack().get_selected_layer();
	int ls_index = view_sp->get_layer_stack().get_selected_layer_index();
	actor.call<FUNC(Picviz::PVView::toggle_layer_stack_layer_n_visible_state)>(ls_index);

	// We need to configure the layer
	view_sp->commit_selection_to_layer(layer);
	actor.call<FUNC(Picviz::PVView::compute_layer_min_max)>(layer);
	actor.call<FUNC(Picviz::PVView::compute_selectable_count)>(layer);
	// and to update the layer-stack
	actor.call<FUNC(Picviz::PVView::process_from_layer_stack)>();

	if (mode != PVWidgets::PVLayerNamingPatternDialog::ON_TOP) {
		int insert_pos;

		if( mode == PVWidgets::PVLayerNamingPatternDialog::ABOVE_CURRENT) {
			insert_pos = old_selected_layer_index + 1;
		} else {
			insert_pos = old_selected_layer_index;
			++old_selected_layer_index;
		}
		actor.call<FUNC(Picviz::PVView::move_selected_layer_to)>(insert_pos);
	}

	ls.set_selected_layer_index(old_selected_layer_index);

	view_sp->get_volatile_selection() = old_sel;
	actor.call<FUNC(Picviz::PVView::commit_volatile_in_floating_selection)>();
        actor.call<FUNC(Picviz::PVView::process_real_output_selection)>();
}

/******************************************************************************
 * PVGuiQt::PVAbstractListStatsDlg::create_layers_for_selected_values
 *****************************************************************************/

void PVGuiQt::PVAbstractListStatsDlg::create_layers_for_selected_values()
{
	/* first, we have to check if there is enough free space for new layers
	 */
	QModelIndexList indexes = _values_view->selectionModel()->selection().indexes();

	Picviz::PVView_sp view_sp = lib_view()->shared_from_this();
	Picviz::PVLayerStack& ls = view_sp->get_layer_stack();

	int layer_num = indexes.size();
	int layer_max = PICVIZ_LAYER_STACK_MAX_DEPTH - ls.get_layer_count();
	if (layer_num >= layer_max) {
		QMessageBox::critical(this, "multiple layer creation",
		                      QString("You try to create %1 layer(s) but no more than %2 layer(s) can be created").arg(layer_num).arg(layer_max));
		return;
	}

	/* now, we can ask for the layers' names format
	 */
	PVWidgets::PVLayerNamingPatternDialog dlg("create one new layer with all selected values",
	                                          "string format for new layer's name",
	                                          "%v", PVWidgets::PVLayerNamingPatternDialog::ON_TOP,
	                                          this);

	if (dlg.exec() == QDialog::Rejected) {
		return;
	}

	QString text = dlg.get_name_pattern();
	PVWidgets::PVLayerNamingPatternDialog::insert_mode  mode = dlg.get_insertion_mode();

	/* some "static" formatting
	 */
	text.replace("%l", ls.get_selected_layer().get_name());
	text.replace("%a", lib_view()->get_axes_combination().get_axis(_col).get_name());

	PVHive::PVActor<Picviz::PVView> actor;
	PVHive::get().register_actor(view_sp, actor);

	/*
	 * The process is little bit heavy:
	 * - backup the current layer's index
	 * - backup the current selection (the pre_filter_layer's one...
	 *   not volatile/floating/whatever)
	 * and for each layer to create:
	 * - run the multiple-search
	 * - create a new layer (do not for forget to notify the layerstackview
	 *   through the hive)
	 * - need to hide the newly created layer (it's better:)
	 * - need to move the newly created layer to the right place
	 * - do a commit from the selection to the newly created layer (like a
	 *   Alt-k and through the hive)
	 * - restore the (old) current layer's index (through the hive)
	 * - set the volatile selection to the backed-up one and force
	 *   reprocessing...
	 */

	Picviz::PVSelection old_sel(view_sp->get_pre_filter_layer().get_selection());
	int old_selected_layer_index = ls.get_selected_layer_index();

	/* layers creation
	 */

	/* as a selection is never sorted, we have to do it ourselves to have
	 * the values in the same order than the view's one
	 */
	qSort(indexes.begin(), indexes.end(), [](const QModelIndex& a, const QModelIndex& b) -> bool { return a.row() < b.row(); });

	int offset = 1;
	for(const auto& index : indexes) {
		if (index.column() != 0) {
			continue;
		}
		QString layer_name(text);
		QString s = index.data().toString();
		if (s.isEmpty()) {
			layer_name.replace("%v", "(empty)");
		} else {
			layer_name.replace("%v", s);
		}

		QStringList sl;
		sl.append(index.data().toString());
		multiple_search(_msearch_action_for_layer_creation, sl, false);

		actor.call<FUNC(Picviz::PVView::add_new_layer)>(layer_name);
		Picviz::PVLayer &layer = view_sp->get_layer_stack().get_selected_layer();
		int ls_index = view_sp->get_layer_stack().get_selected_layer_index();
		actor.call<FUNC(Picviz::PVView::toggle_layer_stack_layer_n_visible_state)>(ls_index);

		// We need to configure the layer
		view_sp->commit_selection_to_layer(layer);
		actor.call<FUNC(Picviz::PVView::compute_layer_min_max)>(layer);
		actor.call<FUNC(Picviz::PVView::compute_selectable_count)>(layer);

		if (mode != PVWidgets::PVLayerNamingPatternDialog::ON_TOP) {
			int insert_pos;

			if( mode == PVWidgets::PVLayerNamingPatternDialog::ABOVE_CURRENT) {
				insert_pos = old_selected_layer_index + offset;
			} else {
				insert_pos = old_selected_layer_index;
				++old_selected_layer_index;
			}
			actor.call<FUNC(Picviz::PVView::move_selected_layer_to)>(insert_pos);
		}

		ls.set_selected_layer_index(old_selected_layer_index);
		view_sp->get_volatile_selection() = old_sel;
		actor.call<FUNC(Picviz::PVView::commit_volatile_in_floating_selection)>();
		actor.call<FUNC(Picviz::PVView::process_real_output_selection)>();
		++offset;
	}

	// we can update the layer-stack once all layers have been created
	actor.call<FUNC(Picviz::PVView::process_from_layer_stack)>();
}

/******************************************************************************
 *
 * PVGuiQt::__impl::PVListUniqStringsDelegate
 *
 *****************************************************************************/

#define ALTERNATING_BG_COLOR 1

void PVGuiQt::__impl::PVListStringsDelegate::paint(
	QPainter* painter,
	const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	assert(index.isValid());

	QStyledItemDelegate::paint(painter, option, index);

	if (index.column() == 1) {
		uint64_t occurence_count = index.data(Qt::UserRole).toULongLong();

		double ratio = (double) occurence_count / d()->max_count();
		double log_ratio = PVCore::log_scale(occurence_count, 0., d()->max_count());
		bool log_scale = d()->use_logarithmic_scale();

		// Draw bounding rectangle
		QRect r(option.rect.x(), option.rect.y(), option.rect.width(), option.rect.height());
		QColor base_color = QPalette().color(QPalette::Base);
		QColor alt_color = QPalette().color(QPalette::AlternateBase);
		painter->fillRect(r, index.row() % 2 ? alt_color : base_color);

		// Fill rectangle with color
		painter->fillRect(
			option.rect.x(),
			option.rect.y(),
			option.rect.width()*(log_scale ? log_ratio : ratio),
			option.rect.height(),
			QColor::fromHsv((log_scale ? log_ratio : ratio) * (0 - 120) + 120, 255, 255)
		);
		painter->setPen(Qt::black);

		// Draw occurence count and/or scientific notation and/or percentage
		size_t occurence_max_width = 0;
		size_t scientific_notation_max_width = 0;
		size_t percentage_max_width = 0;

		size_t margin = option.rect.width();

		QString occurence;
		QString scientific_notation;
		QString percentage;

		size_t representation_count = 0;
		if (d()->_act_show_count->isChecked()) {
			occurence = PVAbstractListStatsModel::format_occurence(occurence_count);
			occurence_max_width = QFontMetrics(painter->font()).width(PVAbstractListStatsModel::format_occurence(d()->relative_max_count()));
			margin -= occurence_max_width;
			representation_count++;
		}
		if (d()->_act_show_scientific_notation->isChecked()) {
			scientific_notation = PVAbstractListStatsModel::format_scientific_notation(ratio);
			scientific_notation_max_width = QFontMetrics(painter->font()).width(PVAbstractListStatsModel::format_scientific_notation(1));
			margin -= scientific_notation_max_width;
			representation_count++;
		}
		if (d()->_act_show_percentage->isChecked()) {
			percentage = PVAbstractListStatsModel::format_percentage(ratio);
			percentage_max_width = QFontMetrics(painter->font()).width(PVAbstractListStatsModel::format_percentage((double)d()->relative_max_count() / d()->max_count()));
			margin -= percentage_max_width;
			representation_count++;
		}

		margin /= representation_count+1;

		int x =  option.rect.x();
		if (d()->_act_show_count->isChecked()) {
			x += margin;
			painter->drawText(
				x,
				option.rect.y(),
				occurence_max_width,
				option.rect.height(),
				Qt::AlignRight,
				occurence
			);
			x += occurence_max_width;
		}
		if (d()->_act_show_scientific_notation->isChecked()) {
			x += margin;
			painter->drawText(
				x,
				option.rect.y(),
				scientific_notation_max_width,
				option.rect.height(),
				Qt::AlignLeft,
				scientific_notation
			);
			x += scientific_notation_max_width;
		}
		if (d()->_act_show_percentage->isChecked()) {
			x += margin;
			painter->drawText(
				x,
				option.rect.y(),
				percentage_max_width,
				option.rect.height(),
				Qt::AlignRight,
				percentage
			);
		}
	}
}

PVGuiQt::PVAbstractListStatsDlg* PVGuiQt::__impl::PVListStringsDelegate::d() const
{
	 return static_cast<PVGuiQt::PVAbstractListStatsDlg*>(parent());
}
