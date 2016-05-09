/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <PVMainWindow.h>
#include <pvguiqt/PVInputTypeMenuEntries.h>

#include <QAction>
#include <QMenuBar>

/******************************************************************************
 *
 * PVInspector::PVMainWindow::create_actions
 *
 *****************************************************************************/
void PVInspector::PVMainWindow::create_actions()
{
	PVLOG_DEBUG("PVInspector::PVMainWindow::%s\n", __FUNCTION__);
	/************************
	 * For the "File" menu entry
	 ************************/

	// The solution actions
	solution_new_Action = new QAction(tr("&New investigation"), this);
	solution_load_Action = new QAction(tr("&Load an investigation..."), this);
	solution_save_Action = new QAction(tr("&Save investigation"), this);
	solution_saveas_Action = new QAction(tr("S&ave investigation as..."), this);

	// The project actions
	project_new_Action = new QAction(tr("&New data collection"), this);

	// The new_file Action
	new_file_Action = new QAction(tr("&New"), this);
	new_file_Action->setIcon(QIcon(":/document-new.png"));
	new_file_Action->setShortcut(QKeySequence::New);
	new_file_Action->setStatusTip(tr("Create a new file."));
	new_file_Action->setWhatsThis(tr("Use this to create a new file."));

	// Export our selection Action
	export_selection_Action = new QAction(tr("&Selection..."), this);
	export_selection_Action->setToolTip(tr("Export the current selection"));

#ifdef WITH_MINESET
	export_selection_to_mineset_Action = new QAction(tr("&Selection to Mineset..."), this);
	export_selection_to_mineset_Action->setToolTip(tr("Export the current selection to Mineset"));
#endif

	quit_Action = new QAction(tr("&Quit"), this);
	quit_Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

	/************************
	 * For the "Edit" menu entry
	 ************************/

	undo_Action = new QAction(tr("Undo"), this);
	undo_Action->setIcon(QIcon(":/edit-undo.png"));

	redo_Action = new QAction(tr("Redo"), this);
	redo_Action->setIcon(QIcon(":/edit-redo.png"));

	undo_history_Action = new QAction(tr("Undo history"), this);

	cut_Action = new QAction(tr("Cut"), this);
	cut_Action->setIcon(QIcon(":/edit-cut.png"));

	copy_Action = new QAction(tr("Copy"), this);
	copy_Action->setIcon(QIcon(":/edit-copy.png"));

	paste_Action = new QAction(tr("Paste"), this);
	paste_Action->setIcon(QIcon(":/edit-paste.png"));

	/************************
	 * For the "Selection" menu entry
	 ************************/
	selection_all_Action = new QAction(tr("Select &all events"), this);
	selection_all_Action->setShortcut(QKeySequence(Qt::Key_A));
	selection_none_Action = new QAction(tr("&Empty selection"), this);
	selection_inverse_Action = new QAction(tr("&Invert selection"), this);
	selection_inverse_Action->setShortcut(QKeySequence(Qt::Key_I));
	set_color_Action = new QAction(tr("Set color"), this);
	set_color_Action->setShortcut(QKeySequence(Qt::Key_C));
	selection_from_current_layer_Action = new QAction(tr("Set selection from current layer"), this);
	selection_from_layer_Action = new QAction(tr("Set selection from layer..."), this);

	// commit_selection_in_current_layer_Action = new QAction(tr("Keep &current
	// layer"), this);
	// commit_selection_in_current_layer_Action->setShortcut(QKeySequence(Qt::Key_K));
	commit_selection_to_new_layer_Action = new QAction(tr("Create new layer from selection"), this);
	commit_selection_to_new_layer_Action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_K));
	move_selection_to_new_layer_Action = new QAction(tr("Move selection to new layer"), this);
	move_selection_to_new_layer_Action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_M));

	/******************************
	 * For the "Filter" menu entry
	 ******************************/
	filter_reprocess_last_filter = new QAction(tr("Apply last filter..."), this);
	filter_reprocess_last_filter->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));

	/************************
	 * For the "Tools" menu entry
	 ************************/
	tools_new_format_Action = new QAction(tr("&New format..."), this);
	tools_cur_format_Action = new QAction(tr("&Edit current format..."), this);

	/************************
	 * For the "View" menu entry
	 ************************/
	view_new_scatter_Action = new QAction(tr("New scatter &view"), this);
	view_display_inv_elts_Action = new QAction(tr("&Display invalid events..."), this);

	/***************************
	 * For the "Axes" menu entry
	 ***************************/
	axes_editor_Action = new QAction(tr("Edit axes..."), this);
	axes_combination_editor_Action = new QAction(tr("Edit axes combination..."), this);
	axes_mode_Action = new QAction(tr("Enter axes mode"), this);
	axes_mode_Action->setShortcut(QKeySequence(Qt::Key_X));
	axes_display_edges_Action = new QAction(tr("Display edges"), this);
	axes_display_edges_Action->setShortcut(QKeySequence(Qt::Key_Y));
	axes_new_Action = new QAction(tr("Create new axis..."), this);

	/***************************
	 * For the "Layers" menu entries
	 ***************************/
	layer_reset_color_Action = new QAction(tr("Reset current layer's colors to white"), this);
	layer_export_Action = new QAction(tr("Export the current layer..."), this);
	layer_import_Action = new QAction(tr("Import a layer..."), this);
	layer_save_ls_Action = new QAction(tr("Save the layer stack..."), this);
	layer_load_ls_Action = new QAction(tr("Load a layer stack..."), this);
	layer_copy_ls_details_to_clipboard_Action =
	    new QAction(tr("Copy the layer stack's details to clipboard"), this);

	/***************************
	 * For the "Events" menu entry
	 ***************************/
	// events_display_unselected_GLview_Action = new QAction(tr("Toggle unselected
	// events"), this);
	// events_display_unselected_GLview_Action->setShortcut(QKeySequence(Qt::Key_U));
	events_display_unselected_listing_Action =
	    new QAction(tr("Toggle unselected events in listing"), this);
	events_display_unselected_listing_Action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_U));

	// events_display_zombies_GLview_Action = new QAction(tr("Toggle zombies
	// events"), this);
	// events_display_zombies_GLview_Action->setShortcut(QKeySequence(Qt::Key_Z));
	events_display_zombies_listing_Action =
	    new QAction(tr("Toggle zombies events in listing"), this);
	events_display_zombies_listing_Action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Z));

	events_display_unselected_zombies_parallelview_Action =
	    new QAction(tr("Toggle unselected and zombies events"), this);
	events_display_unselected_zombies_parallelview_Action->setShortcut(QKeySequence(Qt::Key_U));

	/**************************
	 * For the "Help" menu entry
	 **************************/
	about_Action = new QAction(tr("&About"), this);
	// whats_this_Action = new QAction(tr("&What's this?"), this);
}

/******************************************************************************
 *
 * PVInspector::PVMainWindow::create_menus
 *
 *****************************************************************************/
void PVInspector::PVMainWindow::create_menus()
{
	PVLOG_DEBUG("PVInspector::PVMainWindow::%s\n", __FUNCTION__);

	menubar = menuBar();

	file_Menu = menubar->addMenu(tr("&File"));
	QMenu* solution_Menu = new QMenu(tr("&Investigation"));
	solution_Menu->addAction(solution_new_Action);
	solution_Menu->addAction(solution_load_Action);
	solution_Menu->addAction(solution_save_Action);
	solution_Menu->addAction(solution_saveas_Action);

	QMenu* project_Menu = new QMenu(tr("&Data collection"));
	project_Menu->addAction(project_new_Action);

	file_Menu->addMenu(solution_Menu);
	file_Menu->addSeparator();
	file_Menu->addMenu(project_Menu);
	file_Menu->addSeparator();
	file_Menu->addSeparator();
	file_Menu->addSeparator();
	QMenu* import_Menu = new QMenu(tr("I&mport"));
	create_actions_import_types(import_Menu);
	file_Menu->addMenu(import_Menu);
	QMenu* export_Menu = new QMenu(tr("E&xport"));
	export_Menu->addAction(export_selection_Action);
#ifdef WITH_MINESET
	export_Menu->addAction(export_selection_to_mineset_Action);
#endif
	file_Menu->addMenu(export_Menu);
	file_Menu->addSeparator();
	file_Menu->addAction(quit_Action);

	// edit_Menu = menubar->addMenu(tr("&Edit"));
	// edit_Menu->addAction(undo_Action);
	// edit_Menu->addAction(redo_Action);
	// edit_Menu->addAction(undo_history_Action);
	// edit_Menu->addSeparator();
	// edit_Menu->addAction(cut_Action);
	// edit_Menu->addAction(copy_Action);
	// edit_Menu->addAction(paste_Action);
	// edit_Menu->addSeparator();

	selection_Menu = menubar->addMenu(tr("&Selection"));
	selection_Menu->addAction(selection_all_Action);
	selection_Menu->addAction(selection_none_Action);
	selection_Menu->addAction(selection_inverse_Action);
	selection_Menu->addSeparator();
	selection_Menu->addAction(selection_from_current_layer_Action);
	selection_Menu->addAction(selection_from_layer_Action);
	selection_Menu->addSeparator();
	selection_Menu->addAction(set_color_Action);
	selection_Menu->addSeparator();
	// selection_Menu->addAction(commit_selection_in_current_layer_Action);
	selection_Menu->addAction(commit_selection_to_new_layer_Action);
	selection_Menu->addAction(move_selection_to_new_layer_Action);
	selection_Menu->addSeparator();

	filter_Menu = menubar->addMenu(tr("Fil&ters"));
	filter_Menu->addAction(filter_reprocess_last_filter);
	filter_Menu->addSeparator();
	create_filters_menu_and_actions();

	source_Menu = menubar->addMenu(tr("&Source"));
	source_Menu->addAction(view_display_inv_elts_Action);

	view_Menu = menubar->addMenu(tr("&View"));
	view_Menu->addAction(axes_combination_editor_Action);
	// view_Menu->addAction(view_new_parallel_Action);
	// view_Menu->addAction(view_new_zoomed_parallel_Action);
	// view_Menu->addAction(view_new_scatter_Action);
	// view_Menu->addSeparator();

	/*axes_Menu = menubar->addMenu(tr("&Axes"));
	axes_Menu->addAction(axes_editor_Action);
	axes_Menu->addAction(axes_combination_editor_Action);
	axes_Menu->addSeparator();
	axes_Menu->addAction(axes_mode_Action);
	axes_Menu->addAction(axes_display_edges_Action);
	axes_Menu->addAction(axes_new_Action);
	axes_Menu->addSeparator();*/

	layer_Menu = menubar->addMenu(tr("&Layers"));
	layer_Menu->addAction(layer_export_Action);
	layer_Menu->addAction(layer_import_Action);
	layer_Menu->addSeparator();

	layer_Menu->addAction(layer_reset_color_Action);
	layer_Menu->addSeparator();
	layer_Menu->addAction(layer_save_ls_Action);
	layer_Menu->addAction(layer_load_ls_Action);
	layer_Menu->addSeparator();
	layer_Menu->addAction(layer_copy_ls_details_to_clipboard_Action);

	events_Menu = menubar->addMenu(tr("&Events"));
	events_Menu->addAction(events_display_unselected_listing_Action);
	// events_Menu->addAction(events_display_unselected_GLview_Action);
	// events_Menu->addSeparator();
	events_Menu->addAction(events_display_zombies_listing_Action);
	// events_Menu->addAction(events_display_zombies_GLview_Action);
	events_Menu->addSeparator();
	events_Menu->addAction(events_display_unselected_zombies_parallelview_Action);

	tools_Menu = menubar->addMenu(tr("F&ormat"));
	tools_Menu->addAction(tools_new_format_Action);
	tools_Menu->addAction(tools_cur_format_Action);

	// windows_Menu = menubar->addMenu(tr("&Windows"));

	help_Menu = menubar->addMenu(tr("&Help"));
	// help_Menu->addAction(whats_this_Action);
	help_Menu->addAction(about_Action);
}

/******************************************************************************
 *
 * PVInspector::PVMainWindow::create_actions_import_types
 *
 *****************************************************************************/
void PVInspector::PVMainWindow::create_actions_import_types(QMenu* menu)
{
	PVGuiQt::PVInputTypeMenuEntries::add_inputs_to_menu(menu, this, SLOT(import_type_Slot()));
}

/******************************************************************************
 *
 * PVInspector::PVMainWindow::menu_activate_is_file_opened
 *
 *****************************************************************************/
void PVInspector::PVMainWindow::menu_activate_is_file_opened(bool cond)
{
	export_selection_Action->setEnabled(cond);
#ifdef WITH_MINESET
	export_selection_to_mineset_Action->setEnabled(cond);
#endif

	// axes_Menu->setEnabled(cond);
	filter_Menu->setEnabled(cond);
	events_Menu->setEnabled(cond);
	selection_Menu->setEnabled(cond);
	tools_cur_format_Action->setEnabled(cond);
	source_Menu->setEnabled(cond);
	view_Menu->setEnabled(cond);
	layer_Menu->setEnabled(cond);
	// windows_Menu->setEnabled(cond);
	solution_save_Action->setEnabled(cond);
	solution_saveas_Action->setEnabled(cond);
}

/******************************************************************************
 *
 * PVInspector::PVMainWindow::connect_actions()
 *
 *****************************************************************************/
void PVInspector::PVMainWindow::connect_actions()
{
	PVLOG_DEBUG("PVInspector::PVMainWindow::%s\n", __FUNCTION__);
	connect(solution_new_Action, SIGNAL(triggered()), this, SLOT(solution_new_Slot()));
	connect(solution_load_Action, SIGNAL(triggered()), this, SLOT(solution_load_Slot()));
	connect(solution_save_Action, SIGNAL(triggered()), this, SLOT(solution_save_Slot()));
	connect(solution_saveas_Action, SIGNAL(triggered()), this, SLOT(solution_saveas_Slot()));

	connect(project_new_Action, SIGNAL(triggered()), this, SLOT(project_new_Slot()));
	connect(export_selection_Action, SIGNAL(triggered()), this, SLOT(export_selection_Slot()));
#ifdef WITH_MINESET
	connect(export_selection_to_mineset_Action, SIGNAL(triggered()), this,
	        SLOT(export_selection_to_mineset_Slot()));
#endif
	connect(quit_Action, SIGNAL(triggered()), this, SLOT(quit_Slot()));

	connect(view_new_scatter_Action, SIGNAL(triggered()), this, SLOT(view_new_scatter_Slot()));
	connect(view_display_inv_elts_Action, SIGNAL(triggered()), this,
	        SLOT(view_display_inv_elts_Slot()));

	connect(selection_all_Action, SIGNAL(triggered()), this, SLOT(selection_all_Slot()));
	connect(selection_none_Action, SIGNAL(triggered()), this, SLOT(selection_none_Slot()));
	connect(selection_inverse_Action, SIGNAL(triggered()), this, SLOT(selection_inverse_Slot()));
	connect(selection_from_current_layer_Action, SIGNAL(triggered()), this,
	        SLOT(selection_set_from_current_layer_Slot()));
	connect(selection_from_layer_Action, SIGNAL(triggered()), this,
	        SLOT(selection_set_from_layer_Slot()));

	connect(set_color_Action, SIGNAL(triggered()), this, SLOT(set_color_Slot()));

	// connect(commit_selection_in_current_layer_Action, SIGNAL(triggered()),
	// this, SLOT(commit_selection_in_current_layer_Slot()));
	connect(commit_selection_to_new_layer_Action, SIGNAL(triggered()), this,
	        SLOT(commit_selection_to_new_layer_Slot()));
	connect(move_selection_to_new_layer_Action, SIGNAL(triggered()), this,
	        SLOT(move_selection_to_new_layer_Slot()));

	connect(axes_editor_Action, SIGNAL(triggered()), this, SLOT(axes_editor_Slot())); //
	connect(axes_combination_editor_Action, SIGNAL(triggered()), this,
	        SLOT(axes_combination_editor_Slot())); //
	connect(axes_mode_Action, SIGNAL(triggered()), this, SLOT(axes_mode_Slot()));
	connect(axes_display_edges_Action, SIGNAL(triggered()), this, SLOT(axes_display_edges_Slot()));
	connect(axes_new_Action, SIGNAL(triggered()), this, SLOT(axes_new_Slot()));

	connect(filter_reprocess_last_filter, SIGNAL(triggered()), this,
	        SLOT(filter_reprocess_last_Slot()));

	connect(events_display_unselected_listing_Action, SIGNAL(triggered()), this,
	        SLOT(events_display_unselected_listing_Slot()));
	// connect(events_display_unselected_GLview_Action, SIGNAL(triggered()), this,
	// SLOT(events_display_unselected_GLview_Slot()));
	connect(events_display_zombies_listing_Action, SIGNAL(triggered()), this,
	        SLOT(events_display_zombies_listing_Slot()));
	// connect(events_display_zombies_GLview_Action, SIGNAL(triggered()), this,
	// SLOT(events_display_zombies_GLview_Slot()));
	connect(events_display_unselected_zombies_parallelview_Action, SIGNAL(triggered()), this,
	        SLOT(events_display_unselected_zombies_parallelview_Slot()));

	connect(layer_export_Action, SIGNAL(triggered()), this, SLOT(layer_export_Slot()));
	connect(layer_import_Action, SIGNAL(triggered()), this, SLOT(layer_import_Slot()));

	connect(layer_save_ls_Action, SIGNAL(triggered()), this, SLOT(layer_save_ls_Slot()));
	connect(layer_load_ls_Action, SIGNAL(triggered()), this, SLOT(layer_load_ls_Slot()));
	connect(layer_copy_ls_details_to_clipboard_Action, SIGNAL(triggered()), this,
	        SLOT(layer_copy_ls_details_to_clipboard_Slot()));
	connect(layer_reset_color_Action, SIGNAL(triggered()), this, SLOT(layer_reset_color_Slot()));

	connect(tools_new_format_Action, SIGNAL(triggered()), this, SLOT(new_format_Slot()));
	connect(tools_cur_format_Action, SIGNAL(triggered()), this, SLOT(cur_format_Slot()));

	// connect(whats_this_Action, SIGNAL(triggered()), this,
	// SLOT(whats_this_Slot()));
	connect(about_Action, SIGNAL(triggered()), this, SLOT(about_Slot()));
}
