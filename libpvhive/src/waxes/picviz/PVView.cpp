#include <pvhive/PVHive.h>
#include <pvhive/waxes/picviz/PVView.h>

PVHIVE_CALL_OBJECT_BLOCK_BEGIN()

// Processing waxes
//

IMPL_WAX(Picviz::PVView::process_eventline, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_eventline)>(view, args);
	refresh_observers(&view->get_output_layer());
}

IMPL_WAX(Picviz::PVView::process_selection, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_eventline)>(view, args);
	refresh_observers(&view->get_pre_filter_layer());
}

IMPL_WAX(Picviz::PVView::process_layer_stack, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_layer_stack)>(view, args);
	refresh_observers(&view->get_layer_stack_output_layer());
}

IMPL_WAX(Picviz::PVView::process_filter, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_filter)>(view, args);
	refresh_observers(&view->get_post_filter_layer());
}

IMPL_WAX(Picviz::PVView::process_visibility, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_visibility)>(view, args);
	refresh_observers(&view->get_output_layer());
}

IMPL_WAX(Picviz::PVView::process_from_selection, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_from_selection)>(view, args);
	refresh_observers(&view->get_pre_filter_layer());
	refresh_observers(&view->get_post_filter_layer());
	refresh_observers(&view->get_output_layer());
}

IMPL_WAX(Picviz::PVView::process_from_layer_stack, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_from_layer_stack)>(view, args);
	refresh_observers(&view->get_layer_stack_output_layer());
	refresh_observers(&view->get_pre_filter_layer());
	refresh_observers(&view->get_post_filter_layer());
	refresh_observers(&view->get_output_layer());
}

IMPL_WAX(Picviz::PVView::process_from_filter, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_from_filter)>(view, args);
	refresh_observers(&view->get_post_filter_layer());
	refresh_observers(&view->get_output_layer());
}

IMPL_WAX(Picviz::PVView::process_from_eventline, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_from_eventline)>(view, args);
	refresh_observers(&view->get_output_layer());
}

IMPL_WAX(Picviz::PVView::process_real_output_selection, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::process_real_output_selection)>(view, args);
	refresh_observers(&view->get_real_output_selection());
}

// Layer stack waxes
//

IMPL_WAX(Picviz::PVView::add_new_layer, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::add_new_layer)>(view, args);
	refresh_observers(&view->get_layer_stack());
}

IMPL_WAX(Picviz::PVView::delete_layer_n, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::delete_layer_n)>(view, args);
	refresh_observers(&view->get_layer_stack());
}

IMPL_WAX(Picviz::PVView::delete_selected_layer, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::delete_selected_layer)>(view, args);
	refresh_observers(&view->get_layer_stack());
}

IMPL_WAX(Picviz::PVView::commit_to_new_layer, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::commit_to_new_layer)>(view, args);
	refresh_observers(&view->get_layer_stack());
}

IMPL_WAX(Picviz::PVView::reset_layers, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::reset_layers)>(view, args);
	refresh_observers(&view->get_layer_stack());
}

IMPL_WAX(Picviz::PVView::set_layer_stack_selected_layer_index, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::set_layer_stack_selected_layer_index)>(view, args);
	refresh_observers(&view->get_layer_stack());
}

IMPL_WAX(Picviz::PVView::set_layer_stack_layer_n_name, view, args)
{
	auto ret = call_object_default<Picviz::PVView, FUNC(Picviz::PVView::set_layer_stack_layer_n_name)>(view, args);
	refresh_observers(&view->get_layer_stack());
	return std::move(ret);
}

IMPL_WAX(Picviz::PVView::toggle_layer_stack_layer_n_locked_state, view, args)
{
	auto ret = call_object_default<Picviz::PVView, FUNC(Picviz::PVView::toggle_layer_stack_layer_n_locked_state)>(view, args);
	refresh_observers(&view->get_layer_stack());
	return std::move(ret);
}

IMPL_WAX(Picviz::PVView::toggle_layer_stack_layer_n_visible_state, view, args)
{
	auto ret = call_object_default<Picviz::PVView, FUNC(Picviz::PVView::toggle_layer_stack_layer_n_visible_state)>(view, args);
	refresh_observers(&view->get_layer_stack());
	return std::move(ret);
}

// Axes combination waxes
//

IMPL_WAX(Picviz::PVView::set_axes_combination_list_id, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::set_axes_combination_list_id)>(view, args);
	refresh_observers(&view->get_axes_combination().get_axes_index_list());
}

IMPL_WAX(Picviz::PVView::move_axis_to_new_position, view, args)
{
	auto ret = call_object_default<Picviz::PVView, FUNC(Picviz::PVView::move_axis_to_new_position)>(view, args);
	refresh_observers(&view->get_axes_combination().get_axes_index_list());
	return std::move(ret);
}

IMPL_WAX(Picviz::PVView::remove_column, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::remove_column)>(view, args);
	refresh_observers(&view->get_axes_combination().get_axes_index_list());
}

IMPL_WAX(Picviz::PVView::axis_append, view, args)
{
	call_object_default<Picviz::PVView, FUNC(Picviz::PVView::axis_append)>(view, args);
	refresh_observers(&view->get_axes_combination().get_axes_index_list());
}

PVHIVE_CALL_OBJECT_BLOCK_END()