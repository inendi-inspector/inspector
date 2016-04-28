/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvparallelview/PVLibView.h>
#include <pvparallelview/PVParallelView.h>
#include <pvparallelview/PVHitCountView.h>

#include <pvparallelview/PVDisplayViewHitCount.h>

/*****************************************************************************
 * PVDisplays::PVDisplayViewHitCount::PVDisplayViewHitCount
 *****************************************************************************/

PVDisplays::PVDisplayViewHitCount::PVDisplayViewHitCount()
    : PVDisplayViewAxisIf(PVDisplayIf::ShowInToolbar | PVDisplayIf::ShowInCtxtMenu,
                          "Hit count view")
{
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewHitCount::create_widget
 *****************************************************************************/

QWidget* PVDisplays::PVDisplayViewHitCount::create_widget(Inendi::PVView* view,
                                                          PVCol axis_comb,
                                                          QWidget* parent) const
{
	PVParallelView::PVLibView* lib_view = PVParallelView::common::get_lib_view(*view);
	QWidget* widget = lib_view->create_hit_count_view(axis_comb, parent);

	return widget;
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewHitCount::toolbar_icon
 *****************************************************************************/

QIcon PVDisplays::PVDisplayViewHitCount::toolbar_icon() const
{
	return QIcon(":/view-hit-count");
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewHitCount::widget_title
 *****************************************************************************/

QString PVDisplays::PVDisplayViewHitCount::widget_title(Inendi::PVView* view, PVCol axis_comb) const
{
	return QString("Hit count view [" + view->get_name() + " on axis " +
	               view->get_axis_name(axis_comb) + "]");
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewHitCount::axis_menu_name
 *****************************************************************************/

QString PVDisplays::PVDisplayViewHitCount::axis_menu_name(Inendi::PVView const* /*view*/,
                                                          PVCol /*axis_comb*/) const
{
	return QString("New hit count view");
}
