/**
 * \file PVRoot.cpp
 *
 * Copyright (C) Picviz Labs 2012
 */

#include <pvhive/PVHive.h>
#include <pvhive/waxes/picviz/PVRoot.h>

#include <picviz/PVRoot.h>
#include <picviz/PVView.h>

#include <iostream>

PVHIVE_CALL_OBJECT_BLOCK_BEGIN()

IMPL_WAX(Picviz::PVRoot::select_view, root, args)
{
	about_to_refresh_observers(root->get_current_view_hive_property());
	about_to_refresh_observers(root->get_current_source_hive_property());
	about_to_refresh_observers(root->get_current_scene_hive_property());
	call_object_default<Picviz::PVRoot, FUNC(Picviz::PVRoot::select_view)>(root, args);
	refresh_observers(root->get_current_view_hive_property());
	refresh_observers(root->get_current_source_hive_property());
	refresh_observers(root->get_current_scene_hive_property());
}

IMPL_WAX(Picviz::PVRoot::select_source, root, args)
{
	about_to_refresh_observers(root->get_current_view_hive_property());
	about_to_refresh_observers(root->get_current_source_hive_property());
	about_to_refresh_observers(root->get_current_scene_hive_property());
	call_object_default<Picviz::PVRoot, FUNC(Picviz::PVRoot::select_source)>(root, args);
	refresh_observers(root->get_current_view_hive_property());
	refresh_observers(root->get_current_source_hive_property());
	refresh_observers(root->get_current_scene_hive_property());
}

IMPL_WAX(Picviz::PVRoot::select_scene, root, args)
{
	about_to_refresh_observers(root->get_current_view_hive_property());
	about_to_refresh_observers(root->get_current_source_hive_property());
	about_to_refresh_observers(root->get_current_scene_hive_property());
	call_object_default<Picviz::PVRoot, FUNC(Picviz::PVRoot::select_scene)>(root, args);
	refresh_observers(root->get_current_view_hive_property());
	refresh_observers(root->get_current_source_hive_property());
	refresh_observers(root->get_current_scene_hive_property());
}

PVHIVE_CALL_OBJECT_BLOCK_END()
