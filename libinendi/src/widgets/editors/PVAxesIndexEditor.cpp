/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVAxesIndexType.h>


#include <inendi/PVView.h>

#include <inendi/widgets/editors/PVAxesIndexEditor.h>

#include <QList>
#include <QAbstractItemView>
#include <QSizePolicy>

/******************************************************************************
 *
 * PVCore::PVAxesIndexEditor::PVAxesIndexEditor
 *
 *****************************************************************************/
PVWidgets::PVAxesIndexEditor::PVAxesIndexEditor(Inendi::PVView const& view, QWidget *parent):
	PVWidgets::PVSizeHintListWidget<>(parent),
	_view(view)
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Minimum);
	//sp.setHeightForWidth(sizePolicy().hasHeightForWidth());
	setSizePolicy(sp);
//	setMinimumHeight(20);
//	setMaximumHeight(40);
}

/******************************************************************************
 *
 * PVWidgets::PVAxesIndexEditor::~PVAxesIndexEditor
 *
 *****************************************************************************/
PVWidgets::PVAxesIndexEditor::~PVAxesIndexEditor()
{
}

/******************************************************************************
 *
 * PVWidgets::PVAxesIndexEditor::set_axes_index
 *
 *****************************************************************************/
void PVWidgets::PVAxesIndexEditor::set_axes_index(PVCore::PVAxesIndexType axes_index)
{
	clear();
			
	QStringList const& axes = _view.get_axes_names_list();
	QListWidgetItem* item;

	for (int i = 0; i < axes.count(); i++) {
		item = new QListWidgetItem(axes[i]);
		addItem(item);
		if (std::find(axes_index.begin(), axes_index.end(), _view.axes_combination.get_axis_column_index(i)) != axes_index.end()) {
			item->setSelected(true);
		}
	}
}

PVCore::PVAxesIndexType PVWidgets::PVAxesIndexEditor::get_axes_index() const
{
	QModelIndexList selitems = selectionModel()->selectedIndexes();
	QModelIndexList::iterator it;
	PVCore::PVAxesIndexType ret;
	for (it = selitems.begin(); it != selitems.end(); it++) {
		ret.push_back(_view.axes_combination.get_axis_column_index((*it).row()));
	}

	return ret;
}
