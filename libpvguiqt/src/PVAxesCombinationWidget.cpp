/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <inendi/PVAxesCombination.h>
#include <inendi/PVView.h>
#include <inendi/PVPlotted.h>
#include <pvguiqt/PVAxesCombinationWidget.h>

#include <QDialogButtonBox>

PVGuiQt::PVAxesCombinationWidget::PVAxesCombinationWidget(
    Inendi::PVAxesCombination& axes_combination, Inendi::PVView* view, QWidget* parent)
    : QWidget(parent), _axes_combination(axes_combination), _view(view)
{
	setupUi(this);

	update_all();

	_move_dlg = new PVMoveToDlg(this);

	connect(_btn_axis_add, SIGNAL(clicked()), this, SLOT(axis_add_Slot()));
	connect(_btn_axis_up, SIGNAL(clicked()), this, SLOT(axis_up_Slot()));
	connect(_btn_axis_down, SIGNAL(clicked()), this, SLOT(axis_down_Slot()));
	connect(_btn_axis_move, SIGNAL(clicked()), this, SLOT(axis_move_Slot()));
	connect(_btn_axis_remove, SIGNAL(clicked()), this, SLOT(axis_remove_Slot()));
	connect(_btn_sort, SIGNAL(clicked()), this, SLOT(sort_Slot()));
	connect(_btn_reset, SIGNAL(clicked()), this, SLOT(reset_comb_Slot()));

	_btn_sel_singleton->setEnabled(view != NULL);
	if (view != NULL) {
		connect(_btn_sel_singleton, SIGNAL(clicked()), this, SLOT(sel_singleton_Slot()));
	}
}

void PVGuiQt::PVAxesCombinationWidget::axis_add_Slot()
{
	if (!is_original_axis_selected()) {
		return;
	}

	PVCol axis_id = get_original_axis_selected();
	QString axis_name = get_original_axis_selected_name();

	_list_used->addItem(axis_name);
	_axes_combination.axis_append(axis_id);

	_list_used->setCurrentRow(_list_used->count() - 1);

	emit axes_count_changed();
	emit axes_combination_changed();
}

void PVGuiQt::PVAxesCombinationWidget::axis_up_Slot()
{
	if (!is_used_axis_selected()) {
		return;
	}

	QVector<PVCol> axes_id(get_used_axes_selected());
	foreach (PVCol c, axes_id) {
		if (c == 0) {
			return;
		}
	}

	_axes_combination.move_axes_left_one_position(axes_id.begin(), axes_id.end());
	update_used_axes();
	QItemSelection new_sel;
	foreach (PVCol c, axes_id) {
		QModelIndex midx = _list_used->model()->index(c - 1, 0);
		new_sel.select(midx, midx);
	}
	_list_used->selectionModel()->select(new_sel, QItemSelectionModel::ClearAndSelect);

	emit axes_combination_changed();
}

void PVGuiQt::PVAxesCombinationWidget::axis_down_Slot()
{
	if (!is_used_axis_selected()) {
		return;
	}

	QVector<PVCol> axes_id(get_used_axes_selected());
	foreach (PVCol c, axes_id) {
		if (c == _list_used->count() - 1) {
			return;
		}
	}

	_axes_combination.move_axes_right_one_position(axes_id.begin(), axes_id.end());
	update_used_axes();
	QItemSelection new_sel;
	foreach (PVCol c, axes_id) {
		QModelIndex midx = _list_used->model()->index(c + 1, 0);
		new_sel.select(midx, midx);
	}
	_list_used->selectionModel()->select(new_sel, QItemSelectionModel::ClearAndSelect);

	emit axes_combination_changed();
}

void PVGuiQt::PVAxesCombinationWidget::axis_move_Slot()
{
	if (!is_used_axis_selected()) {
		return;
	}

	_move_dlg->update_axes();
	if (_move_dlg->exec() != QDialog::Accepted) {
		return;
	}

	QVector<PVCol> org(get_used_axes_selected());
	PVCol dest = _move_dlg->get_dest_col(org.at(0));
	if (!_axes_combination.move_axes_to_new_position(org.begin(), org.end(), dest)) {
		return;
	}

	update_used_axes();
	_list_used->setCurrentRow(dest);
	emit axes_combination_changed();
}

void PVGuiQt::PVAxesCombinationWidget::axis_remove_Slot()
{
	if (!is_used_axis_selected()) {
		return;
	}

	// We need a minimum of 2 axes !
	if (_list_used->count() <= 2) {
		return;
	}

	QVector<PVCol> axes_id = get_used_axes_selected();
	_axes_combination.remove_axes(axes_id);
	update_used_axes();
	_list_used->setCurrentRow(std::min(axes_id.at(0), _list_used->count() - 1));

	emit axes_count_changed();
	emit axes_combination_changed();
}

void PVGuiQt::PVAxesCombinationWidget::reset_comb_Slot()
{
	PVCol nold_axes = _axes_combination.get_axes_count();
	_axes_combination.reset_to_default();

	update_used_axes();

	if (nold_axes != _axes_combination.get_axes_count()) {
		emit axes_count_changed();
	}
	emit axes_combination_changed();
}

PVCol PVGuiQt::PVAxesCombinationWidget::get_original_axis_selected()
{
	return _list_org->currentRow();
}

QString PVGuiQt::PVAxesCombinationWidget::get_original_axis_selected_name()
{
	return _list_org->currentItem()->text();
}

QVector<PVCol> PVGuiQt::PVAxesCombinationWidget::get_list_selection(QListWidget* widget)
{
	QVector<PVCol> ret;
	QModelIndexList list = widget->selectionModel()->selectedIndexes();
	ret.reserve(list.size());
	foreach (const QModelIndex& idx, list) {
		ret.push_back(idx.row());
	}
	return ret;
}

QVector<PVCol> PVGuiQt::PVAxesCombinationWidget::get_used_axes_selected()
{
	return get_list_selection(_list_used);
}

void PVGuiQt::PVAxesCombinationWidget::update_orig_axes()
{
	_list_org->clear();
	_list_org->addItems(_axes_combination.get_original_axes_names_list());
}

void PVGuiQt::PVAxesCombinationWidget::update_used_axes()
{
	_list_used->clear();
	_list_used->addItems(_axes_combination.get_axes_names_list());
}

void PVGuiQt::PVAxesCombinationWidget::update_all()
{
	update_orig_axes();
	update_used_axes();
}

void PVGuiQt::PVAxesCombinationWidget::reset_used_axes()
{
	_axes_combination = _view->get_axes_combination();
	update_used_axes();
}

void PVGuiQt::PVAxesCombinationWidget::sort_Slot()
{
	_axes_combination.sort_by_name(true);
	update_used_axes();
	emit axes_combination_changed();
}

bool PVGuiQt::PVAxesCombinationWidget::is_used_axis_selected()
{
	return _list_used->selectedItems().size() > 0;
}

bool PVGuiQt::PVAxesCombinationWidget::is_original_axis_selected()
{
	return _list_org->selectedItems().size() > 0;
}

// PVMoveToDlg implementation
PVGuiQt::PVAxesCombinationWidget::PVMoveToDlg::PVMoveToDlg(PVAxesCombinationWidget* parent)
    : QDialog(parent), _parent(parent)
{
	setWindowTitle(tr("Move to..."));

	QVBoxLayout* main_layout = new QVBoxLayout();
	QHBoxLayout* combo_layout = new QHBoxLayout();

	_after_combo = new QComboBox();
	_after_combo->addItem(tr("Before"));
	_after_combo->addItem(tr("After"));

	_axes_combo = new QComboBox();
	_axes_combo->addItems(parent->_axes_combination.get_axes_names_list());

	combo_layout->addWidget(_after_combo);
	combo_layout->addWidget(_axes_combo);
	main_layout->addLayout(combo_layout);

	QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btns, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btns, SIGNAL(rejected()), this, SLOT(reject()));
	main_layout->addWidget(btns);

	setLayout(main_layout);
}

PVCol PVGuiQt::PVAxesCombinationWidget::PVMoveToDlg::get_dest_col(PVCol org)
{
	bool after = _after_combo->currentIndex() == 1;
	PVCol pos = _axes_combo->currentIndex();
	if (after) {
		pos++;
	}

	if (pos > org) {
		pos--;
	}

	return pos;
}

void PVGuiQt::PVAxesCombinationWidget::PVMoveToDlg::update_axes()
{
	QString _old_sel = _axes_combo->currentText();
	_axes_combo->clear();
	_axes_combo->addItems(_parent->_axes_combination.get_axes_names_list());
	int old_sel_idx = _axes_combo->findText(_old_sel);
	if (old_sel_idx >= 0) {
		_axes_combo->setCurrentIndex(old_sel_idx);
	}
}

void PVGuiQt::PVAxesCombinationWidget::set_selection_from_cols(QList<PVCol> const& cols)
{
	QItemSelection new_sel;
	foreach (PVCol c, cols) {
		QList<PVCol> comb_cols = _axes_combination.get_combined_axes_columns_indexes(c);
		foreach (PVCol comb_c, comb_cols) {
			QModelIndex midx = _list_used->model()->index(comb_c, 0);
			new_sel.select(midx, midx);
		}
	}
	_list_used->selectionModel()->select(new_sel, QItemSelectionModel::ClearAndSelect);
}

void PVGuiQt::PVAxesCombinationWidget::sel_singleton_Slot()
{
	assert(_view);
	QList<PVCol> cols_rem = _view->get_parent<Inendi::PVPlotted>().get_singleton_columns_indexes();
	set_selection_from_cols(cols_rem);
}

// PVAxesCombinationWidgetSelRange implementation
PVGuiQt::PVAxesCombinationWidgetSelRange::PVAxesCombinationWidgetSelRange(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);
}

bool PVGuiQt::PVAxesCombinationWidgetSelRange::get_range(float& min, float& max)
{
	bool ret = true;
	min = _edit_min->text().toFloat(&ret);
	if (!ret) {
		return false;
	}
	max = _edit_max->text().toFloat(&ret);
	return ret;
}

bool PVGuiQt::PVAxesCombinationWidgetSelRange::reversed()
{
	return _combo_reverse->currentIndex() == 1;
}

double PVGuiQt::PVAxesCombinationWidgetSelRange::rate()
{
	double rate = _edit_rate->text().toDouble() / 100.0;
	if (rate == 0.0) {
		rate = 1.0;
	}
	return rate;
}

PVGuiQt::PVAxesCombinationWidgetSelRange::values_source_t
PVGuiQt::PVAxesCombinationWidgetSelRange::get_source()
{
	return (_combo_values_src->currentIndex() == 0) ? plotted : mapped;
}
