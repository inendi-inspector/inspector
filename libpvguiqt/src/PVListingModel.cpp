/**
 * \file PVListingModel.cpp
 *
 * Copyright (C) Picviz Labs 2009-2012
 */

#include <QtCore>
#include <QtGui>

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVColor.h>

#include <picviz/PVSource.h>
#include <picviz/PVView.h>

#include <pvhive/PVHive.h>
#include <pvhive/PVCallHelper.h>
#include <pvhive/waxes/waxes.h>

#include <pvguiqt/PVCustomQtRoles.h>
#include <pvguiqt/PVListingModel.h>

/******************************************************************************
 *
 * PVInspector::PVListingModel::PVListingModel
 *
 *****************************************************************************/
PVGuiQt::PVListingModel::PVListingModel(Picviz::PVView_sp& view, QObject* parent):
	QAbstractTableModel(parent),
	_obs(this),
	_view_valid(true)
{
	//test_fontdatabase.addApplicationFont(QString("/donnees/HORS_SVN/TESTS_PHIL/GOOGLE_WEBFONTS/Convergence/Convergence-Regular.ttf"));
	//test_fontdatabase.addApplicationFont(QString("/donnees/HORS_SVN/TESTS_PHIL/GOOGLE_WEBFONTS/Metrophobic/Metrophobic.ttf"));

	test_fontdatabase.addApplicationFont(QString(":/Convergence-Regular.ttf"));

	row_header_font = QFont("Convergence-Regular", 6);

	select_brush = QBrush(QColor(255, 240, 200));
	unselect_brush = QBrush(QColor(180, 180, 180));
	select_font.setBold(true);
	not_zombie_font_brush = QBrush(QColor(0, 0, 0));
	zombie_font_brush = QBrush(QColor(200, 200, 200));

	PVHive::get().register_actor(view, _actor);
	PVHive::get().register_observer(view, _obs);

	_obs.connect_about_to_be_deleted(this, SLOT(view_about_to_be_deleted(PVHive::PVObserverBase*)));
}



/******************************************************************************
 *
 * PVGuiQt::PVListingModel::columnCount
 *
 *****************************************************************************/
int PVGuiQt::PVListingModel::columnCount(const QModelIndex &) const 
{
	if (!_view_valid) {
		return 0;
	}

	return lib_view().get_parent<Picviz::PVSource>()->get_column_count();
}

/******************************************************************************
 *
 * PVGuiQt::PVListingModel::data
 *
 *****************************************************************************/
QVariant PVGuiQt::PVListingModel::data(const QModelIndex &index, int role) const
{
	assert(_view_valid);

	switch (role) {
		case (Qt::DisplayRole):
			return lib_view().get_parent<Picviz::PVSource>()->get_value(index.row(), index.column());

		case (Qt::TextAlignmentRole):
			return (Qt::AlignLeft + Qt::AlignVCenter);

		case (Qt::BackgroundRole):
		{
			/* We get the current selected axis index */
			PVCol c = lib_view().get_active_axis();

			if ((lib_view().get_state_machine().is_axes_mode()) && (c == index.column())) {
				/* We must provide an evidence of the active_axis ! */
				return QBrush(QColor(130, 100, 25));
			} else {
				const PVRow r = index.row();
				if (lib_view().get_line_state_in_output_layer(r)) {
					const PVCore::PVHSVColor color = lib_view().get_color_in_output_layer(r);
					return QBrush(color.toQColor());
				} else {
					return unselect_brush;
				}
			}
		}

		case (Qt::ForegroundRole):
			if (lib_view().layer_stack_output_layer.get_selection().get_line(index.row())) {
				/* The line is NOT a ZOMBIE */
				return not_zombie_font_brush;
			} else {
				/* The line is a ZOMBIE */
				return zombie_font_brush;
			}
		case (PVCustomQtRoles::Sort):
			{
				QVariant ret;
				ret.setValue<void*>((void*) &lib_view().get_parent<Picviz::PVSource>()->get_data_unistr_raw(index.row(), index.column()));
				return ret;
			}
	}
	return QVariant();
}


/******************************************************************************
 *
 * PVGuiQt::PVListingModel::flags
 *
 *****************************************************************************/
Qt::ItemFlags PVGuiQt::PVListingModel::flags(const QModelIndex &/*index*/) const
{
	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}


/******************************************************************************
 *
 * PVGuiQt::PVListingModel::headerData
 *
 *****************************************************************************/
QVariant PVGuiQt::PVListingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	assert(_view_valid);

	switch (role) {
		case (Qt::DisplayRole):
			if (orientation == Qt::Horizontal) {
				QString axis_name(lib_view().get_original_axis_name(section));
				return QVariant(axis_name);
			} else {
				if (section < 0) {
					// That should never happen !
					return QVariant();
				}
				return section;
			}
		break;
		case (Qt::FontRole):
			if ((orientation == Qt::Vertical) && (lib_view().real_output_selection.get_line(section))) {
				return row_header_font;
			} else {
				return unselect_font;
			}
			break;
		case (Qt::TextAlignmentRole):
			if (orientation == Qt::Horizontal) {
				return (Qt::AlignLeft + Qt::AlignVCenter);
			} else {
				return (Qt::AlignRight + Qt::AlignVCenter);
			}
			break;

		default:
			return QVariant();
	}

	return QVariant();
}


/******************************************************************************
 *
 * PVGuiQt::PVListingModel::rowCount
 *
 *****************************************************************************/
int PVGuiQt::PVListingModel::rowCount(const QModelIndex &index) const 
{
	if (index.isValid() || !_view_valid) {
		return 0;
	}

	return lib_view().get_parent<Picviz::PVSource>()->get_row_count();
}

void PVGuiQt::PVListingModel::view_about_to_be_deleted(PVHive::PVObserverBase* /*o*/)
{
	beginResetModel();
	_view_valid = false;
	endResetModel();
}