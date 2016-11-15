/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvparallelview/PVLibView.h>
#include <pvparallelview/PVParallelView.h>
#include <pvparallelview/PVScatterView.h>

#include <pvparallelview/PVDisplayViewScatter.h>

/*****************************************************************************
 * PVDisplays::PVDisplayViewScatter::PVDisplayViewHitCount
 *****************************************************************************/

PVDisplays::PVDisplayViewScatter::PVDisplayViewScatter()
    : PVDisplayViewZoneIf(PVDisplayIf::ShowInToolbar | PVDisplayIf::ShowInCtxtMenu, "Scatter view")
{
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewScatter::create_widget
 *****************************************************************************/

QWidget* PVDisplays::PVDisplayViewScatter::create_widget(Inendi::PVView* view,
                                                         PVCombCol axis_comb,
                                                         QWidget* parent) const
{
	PVParallelView::PVLibView* lib_view = PVParallelView::common::get_lib_view(*view);
	QWidget* widget = lib_view->create_scatter_view(axis_comb, parent);

	return widget;
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewScatter::toolbar_icon
 *****************************************************************************/

QIcon PVDisplays::PVDisplayViewScatter::toolbar_icon() const
{
	return QIcon(":/view-scatter");
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewScatter::widget_title
 *****************************************************************************/

QString PVDisplays::PVDisplayViewScatter::widget_title(Inendi::PVView* view,
                                                       PVCombCol axis_comb) const
{
	return QString("Scatter view on axes '" + view->get_axis_name(axis_comb) + "' and '" +
	               view->get_axis_name(axis_comb + 1) + "'");
}

/*****************************************************************************
 * PVDisplays::PVDisplayViewScatter::axis_menu_name
 *****************************************************************************/

QString PVDisplays::PVDisplayViewScatter::axis_menu_name(Inendi::PVView const* /*view*/,
                                                         PVCombCol /*axis_comb*/
                                                         ) const
{
	return QString("New scatter view with axis...");
}
