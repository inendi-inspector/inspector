/**
 * \file PVLayerFilterAxisGradient.cpp
 *
 * Copyright (C) Picviz Labs 2009-2012
 */

#include "PVLayerFilterAxisGradient.h"
#include <pvkernel/core/PVColor.h>
#include <pvkernel/core/PVOriginalAxisIndexType.h>
#include <picviz/PVView.h>

#define ARG_NAME_AXIS "axis"
#define ARG_DESC_AXIS "Axis"

/******************************************************************************
 *
 * Picviz::PVLayerFilterAxisGradient::PVLayerFilterAxisGradient
 *
 *****************************************************************************/
Picviz::PVLayerFilterAxisGradient::PVLayerFilterAxisGradient(PVCore::PVArgumentList const& l)
	: PVLayerFilter(l)
{
	INIT_FILTER(PVLayerFilterAxisGradient, l);
	add_ctxt_menu_entry("Gradient on this axis", &PVLayerFilterAxisGradient::gradient_menu);
}

/******************************************************************************
 *
 * DEFAULT_ARGS_FILTER(Picviz::PVLayerFilterAxisGradient)
 *
 *****************************************************************************/
DEFAULT_ARGS_FILTER(Picviz::PVLayerFilterAxisGradient)
{
	PVCore::PVArgumentList args;
	args[PVCore::PVArgumentKey(ARG_NAME_AXIS, QObject::tr(ARG_DESC_AXIS))].setValue(PVCore::PVOriginalAxisIndexType(0));
	return args;
}

/******************************************************************************
 *
 * Picviz::PVLayerFilterAxisGradient::operator()
 *
 *****************************************************************************/
void Picviz::PVLayerFilterAxisGradient::operator()(PVLayer& in, PVLayer &out)
{	
	int axis_id;

	PVCore::PVHSVColor color;

	//const PVSource* source = _view.get_source_parent();
	const PVPlotted* plotted = _view->get_parent<PVPlotted>();
	axis_id = _args[ARG_NAME_AXIS].value<PVCore::PVOriginalAxisIndexType>().get_original_index();

	PVRow r_max,r_min;
	plotted->get_col_minmax(r_min, r_max, in.get_selection(), axis_id);
	const uint32_t min_plotted = ~(plotted->get_value(r_min, axis_id));
	const uint32_t max_plotted = ~(plotted->get_value(r_max, axis_id));
	PVLOG_INFO("PVLayerFilterAxisGradient: min/max = %u/%u\n", min_plotted, max_plotted);
	const double diff = max_plotted-min_plotted;
	in.get_selection().visit_selected_lines([&](PVRow const r)
		{
			const uint32_t plotted_value = ~(plotted->get_value(r, axis_id));

			PVCore::PVHSVColor color;
			// From green to red.. !
			color = ((uint8_t) (((double)(plotted_value-min_plotted)/diff)*(double)(HSV_COLOR_RED-HSV_COLOR_GREEN))) + HSV_COLOR_GREEN;
			out.get_lines_properties().line_set_color(r, color);
		},
		_view->get_row_count());
}

std::vector<PVCore::PVArgumentKey> Picviz::PVLayerFilterAxisGradient::get_args_keys_for_preset() const
{
	PVCore::PVArgumentKeyList keys = get_default_args().keys();
	keys.erase(std::find(keys.begin(), keys.end(), ARG_NAME_AXIS));
	return keys;
}

QString Picviz::PVLayerFilterAxisGradient::status_bar_description()
{
	return QString("Apply a gradient of color on a given axis.");
}

QString Picviz::PVLayerFilterAxisGradient::detailed_description()
{
	return QString("<b>Purpose</b><br/>This filter applies a color gradient on a wanted axis<hr><b>Behavior</b><br/>It will colorize with a gradient from green to red from the lowest axis value to the highest.");
}

PVCore::PVArgumentList Picviz::PVLayerFilterAxisGradient::gradient_menu(PVRow /*row*/, PVCol /*col*/, PVCol org_col, QString const& /*v*/)
{
	PVCore::PVArgumentList args;
	args[ARG_NAME_AXIS].setValue(PVCore::PVOriginalAxisIndexType(org_col));
	return args;

}

IMPL_FILTER(Picviz::PVLayerFilterAxisGradient)
