/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVMAINWINDOW_H
#define PVMAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSet>
#include <QStackedWidget>

#include <pvkernel/core/PVArgument.h>
#include <pvkernel/core/PVMeanValue.h>

#include <pvkernel/rush/PVInput.h>
#include <pvkernel/rush/PVSourceCreator.h>
#include <pvkernel/rush/PVSourceCreatorFactory.h>

#include <inendi/PVRoot_types.h>
#include <inendi/PVScene_types.h>
#include <inendi/PVSource_types.h>
#include <inendi/PVView_types.h>
#include <inendi/PVLayerFilter.h>
#include <inendi/PVSelection.h>


#include <pvhive/PVObserverSignal.h>

#include <pvguiqt/PVProjectsTabWidget.h>

#include <PVFilesTypesSelWidget.h>

//#include <>

/* #include <logviewer/logviewerwidget.h> */

#include <tbb/task_scheduler_init.h>


QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace PVCore {
class PVSerializeArchive;
}

namespace PVGuiQt
{
class PVSourceWorkspace;
class PVAboutBoxDialog;
class PVExportSelectionDlg;
}

namespace PVInspector {

class PVMainWindow;
class PVStartScreenWidget;

/**
 * \class PVMainWindow
 */
class PVMainWindow : public QMainWindow
{
	Q_OBJECT

	friend class PVStartScreenWidget;

private:
	struct PVFormatDetectCtxt
	{
		PVFormatDetectCtxt(PVRush::PVInputType::list_inputs const& inputs_, QHash<QString,PVRush::PVInputDescription_p>& hash_input_name_, PVRush::hash_formats& formats_, PVRush::hash_format_creator& format_creator_, map_files_types& files_multi_formats_, QHash< QString,PVRush::PVInputType::list_inputs >& discovered_, QHash<QString, std::pair<QString,QString> >& formats_error_, PVRush::list_creators& lcr_, PVRush::PVInputType_p in_t_, QHash<QString,PVCore::PVMeanValue<float> >& discovered_types_):
			inputs(inputs_),
			hash_input_name(hash_input_name_),
			formats(formats_),
			format_creator(format_creator_),
			files_multi_formats(files_multi_formats_),
			discovered(discovered_),
			formats_error(formats_error_),
			lcr(lcr_),
			in_t(in_t_),
			discovered_types(discovered_types_)
		{ }

		PVRush::PVInputType::list_inputs const& inputs;
		QHash<QString,PVRush::PVInputDescription_p>& hash_input_name;
		PVRush::hash_formats& formats;
		PVRush::hash_format_creator& format_creator;
		map_files_types& files_multi_formats;
		QHash< QString,PVRush::PVInputType::list_inputs >& discovered;
		QHash<QString,std::pair<QString,QString> >& formats_error;
		PVRush::list_creators& lcr;
		PVRush::PVInputType_p in_t;
		QHash<QString,PVCore::PVMeanValue<float> >& discovered_types;
	};

public:
	PVMainWindow(QWidget *parent = 0);

	PVGuiQt::PVProjectsTabWidget* _projects_tab_widget;

	QMenuBar *menubar;
	QMenu *filter_Menu;
	QLabel *statemachine_label;

	char *last_sendername;
	bool report_started;
	int report_image_index;
	QString *report_filename;

	Inendi::PVView* current_view() { return get_root().current_view(); }
	Inendi::PVView const* current_view() const { return get_root().current_view(); }

	Inendi::PVScene* current_scene() { return get_root().current_scene(); }
	Inendi::PVScene const* current_scene() const { return get_root().current_scene(); }

	void commit_selection_in_current_layer(Inendi::PVView* view);
	void move_selection_to_new_layer(Inendi::PVView* view);
	void commit_selection_to_new_layer(Inendi::PVView* view);
	void set_color(Inendi::PVView* view);

	void import_type(PVRush::PVInputType_p in_t);
	void import_type(PVRush::PVInputType_p in_t, PVRush::PVInputType::list_inputs const& inputs, PVRush::hash_formats& formats, PVRush::hash_format_creator& format_creator, QString const& choosenFormat);
	void load_files(std::vector<QString> const& files, QString format);
	/* void import_type(); */
	void update_statemachine_label(Inendi::PVView_sp view);

	QString get_solution_path() const { return get_root().get_path(); }

	void set_window_title_with_filename();

	bool maybe_save_solution();

protected:
	void remove_source(Inendi::PVSource* src_p);

protected:
	bool event(QEvent* event) override;

public slots:
	void about_Slot();
	void axes_editor_Slot();
	void axes_mode_Slot();
	void axes_display_edges_Slot();
	void axes_new_Slot();
	void commit_selection_in_current_layer_Slot();
	void commit_selection_to_new_layer_Slot();
	void move_selection_to_new_layer_Slot();
	void selection_set_from_current_layer_Slot();
	void selection_set_from_layer_Slot();
	void expand_selection_on_axis_Slot();
	void export_file_Slot();
	void export_selection_Slot();

#ifdef WITH_MINESET
	/**
	 * Export selection and import it on mineset using there REST API.
	 */
	void export_selection_to_mineset_Slot();
#endif

	void extractor_file_Slot();
	void filter_select_all_Slot();
	void filter_Slot();
	void new_format_Slot();
	void cur_format_Slot();
	void edit_format_Slot(const QString& format);
	void open_format_Slot();
	void filter_reprocess_last_Slot();
	void import_type_default_Slot();
	void import_type_Slot();
	void import_type_Slot(const QString & itype);
	void events_display_unselected_Slot();
	void events_display_unselected_listing_Slot();
	void events_display_unselected_GLview_Slot();
	void events_display_zombies_Slot();
	void events_display_zombies_listing_Slot();
	void events_display_zombies_GLview_Slot();
	void events_display_unselected_zombies_parallelview_Slot();
	bool load_source_from_description_Slot(PVRush::PVSourceDescription);
	Inendi::PVScene_p project_new_Slot();
	bool project_save_Slot();
	bool project_saveas_Slot();
	void quit_Slot();
	void refresh_current_view_Slot();
	void select_scene_Slot();
	void selection_all_Slot();
	void selection_inverse_Slot();
	void selection_none_Slot();
	void enable_menu_filter_Slot(bool);
	void set_color_Slot();
	void textedit_text_changed_Slot();
	void view_new_scatter_Slot();
	void view_display_inv_elts_Slot();
	void get_screenshot_widget();
	void get_screenshot_window();
	void get_screenshot_desktop();
	void update_reply_finished_Slot(QNetworkReply *reply);
	void whats_this_Slot();
	// Called by input_type plugins to edit a format.
	// Not an elegant solution, must find better.
	void edit_format_Slot(QString const& path, QWidget* parent);
	void edit_format_Slot(QDomDocument& doc, QWidget* parent);
	void axes_combination_editor_Slot();

	void solution_new_Slot();
	void solution_load_Slot();
	void solution_save_Slot();
	void solution_saveas_Slot();

	void close_solution_Slot();

	void create_new_window_for_workspace(QWidget* workspace);

	void layer_export_Slot();
	void layer_import_Slot();
	void layer_save_ls_Slot();
	void layer_load_ls_Slot();
	void layer_copy_ls_details_to_clipboard_Slot();
	void layer_reset_color_Slot();

#ifdef WITH_MINESET
	/**
	 * Show error message for mineset export.
	 */
	void mineset_error_slot(QString error_msg);
#endif

protected:
	void closeEvent(QCloseEvent* event);

private:
	bool save_project(const QString &file, PVCore::PVSerializeArchiveOptions_p options);
	void set_selection_from_layer(Inendi::PVView_sp view, Inendi::PVLayer const& layer);
	void display_inv_elts();

	void save_screenshot(const QPixmap& pixmap,
	                     const QString& title,
	                     const QString& name);

private slots:
	void root_modified();
	bool load_solution(QString const& file);
	void load_solution_and_create_mw(QString const& file);
	void set_auto_detect_cancellation(bool cancel = true) { _auto_detect_cancellation = cancel; }
	void menu_activate_is_file_opened(bool cond);

private:
	void connect_actions();
	void connect_widgets();
	void create_actions();
	void create_menus();
	void create_filters_menu_and_actions();
	void create_actions_import_types(QMenu* menu);

	// AG: that needs to be redesigned. I outlined this code as an automatic outliner would do, so that
	// a progress box can cancel this process.
	void auto_detect_formats(PVFormatDetectCtxt ctxt);

private:
	bool is_project_untitled()
	{
		return _projects_tab_widget->is_current_project_untitled();
	}
	bool load_root();
	bool load_scene(Inendi::PVScene* scene);
	bool load_source(Inendi::PVSource* src);
	bool fix_project_errors(std::shared_ptr<PVCore::PVSerializeArchive> ar);
	void flag_investigation_as_cached(const QString& file);

private:

	QMenu *axes_Menu;
	QMenu *file_Menu;
	QMenu *edit_Menu;
	QMenu *layer_Menu;
	QMenu *events_Menu;
	QMenu *selection_Menu;
	QMenu* tools_Menu;
	QMenu *source_Menu;
	QMenu *view_Menu;
	QMenu *windows_Menu;
	QMenu *help_Menu;

	QAction *about_Action;
	QAction *axes_editor_Action;
	QAction *axes_combination_editor_Action;
	QAction *axes_mode_Action;
	QAction *axes_display_edges_Action;
	QAction *axes_new_Action;
	QAction *expand_selection_on_axis_Action;
	QAction *events_display_unselected_listing_Action;
	QAction *events_display_unselected_GLview_Action;
	QAction *events_display_zombies_listing_Action;
	QAction *events_display_zombies_GLview_Action;
	QAction* events_display_unselected_zombies_parallelview_Action;
	QAction *copy_Action;
	QAction *commit_selection_in_current_layer_Action;
	QAction *commit_selection_to_new_layer_Action;
	QAction *move_selection_to_new_layer_Action;
	QAction *cut_Action;
	QAction *filter_reprocess_last_filter; 
	QAction *project_new_Action;
	QAction *project_save_Action;
	QAction *project_saveas_Action;
	QAction *solution_new_Action;
	QAction *solution_load_Action;
	QAction *solution_save_Action;
	QAction *solution_saveas_Action;
	QAction *export_file_Action;
	QAction *export_selection_Action;
#ifdef WITH_MINESET
	QAction *export_selection_to_mineset_Action; //!< Menu to trigger mineset export
#endif
	QAction *extractor_file_Action;
	QAction *new_file_Action;
	QAction *new_scene_Action;
	QAction *paste_Action;
	QAction *quit_Action;
	QAction *redo_Action;
	QAction *select_scene_Action;
	QAction *selection_all_Action;
	QAction *selection_inverse_Action;
	QAction *selection_none_Action;
	QAction *selection_from_current_layer_Action;
	QAction *selection_from_layer_Action;
	QAction *set_color_Action;
	QAction* tools_new_format_Action;
	QAction* tools_cur_format_Action;
	QAction *undo_Action;
	QAction *undo_history_Action;
	QAction *view_Action;
	QAction *view_new_scatter_Action;
	QAction *view_display_inv_elts_Action;
	QAction *whats_this_Action;

	QAction *layer_export_Action;
	QAction *layer_import_Action;
	QAction *layer_save_ls_Action;
	QAction *layer_load_ls_Action;
	QAction *layer_copy_ls_details_to_clipboard_Action;
	QAction *layer_reset_color_Action;

	QSpacerItem* pv_mainSpacerTop;
	QSpacerItem* pv_mainSpacerBottom;
	QWidget *pv_centralMainWidget;
	QStackedWidget* pv_centralWidget;
	QVBoxLayout *pv_mainLayout;
	QVBoxLayout *pv_startLayout;
	QLabel* pv_lastCurVersion;
	QLabel* pv_lastMajVersion;
	QFileDialog _load_solution_dlg;

	QString _current_save_root_folder;

protected:
	void keyPressEvent(QKeyEvent *event);
	int update_check();
	void treat_invalid_formats(QHash<QString, std::pair<QString,QString> > const& errors);
	PVGuiQt::PVSourceWorkspace* get_tab_from_view(Inendi::PVView* inendi_view);
	PVGuiQt::PVSourceWorkspace* get_tab_from_view(Inendi::PVView const& inendi_view);
	void set_version_informations();

private:
	Inendi::PVRoot& get_root();
	Inendi::PVRoot const& get_root() const;
	Inendi::PVRoot_sp get_root_sp();

private:
	static PVMainWindow* find_main_window(const QString& path);
	bool is_solution_untitled() const { return get_solution_path().isEmpty(); }
	void save_solution(QString const& file, PVCore::PVSerializeArchiveOptions_p const& options);
	void reset_root();
	void close_solution();

signals:
	void change_of_current_view_Signal();
	void color_changed_Signal();
	void filter_applied_Signal();
	void selection_changed_Signal();
	void zombie_mode_changed_Signal();

#ifdef WITH_MINESET
	/**
	 * Signal to show a mineset error from a thread.
	 */
	void mineset_error(QString error_msg);
#endif

private:
	QString _cur_project_file;
	bool _cur_project_save_everything;
	static int sequence_n;
	Inendi::PVRoot_sp _root;
	PVHive::PVObserverSignal<Inendi::PVRoot> _obs_root;
	bool _auto_detect_cancellation;

private:
	version_t _last_known_cur_release;
	version_t _last_known_maj_release;
	QString   _screenshot_root_dir;
};

}

#endif // PVMAINWINDOW_H
