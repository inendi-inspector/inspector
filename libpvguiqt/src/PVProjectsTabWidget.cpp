/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvguiqt/PVProjectsTabWidget.h>
#include <pvguiqt/PVStartScreenWidget.h>

#include <pvhive/PVHive.h>
#include <pvhive/PVCallHelper.h>

#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QAction>

const QString star = "*";

/******************************************************************************
 *
 * PVGuiQt::__impl::PVTabBar
 *
 *****************************************************************************/
void PVGuiQt::__impl::PVTabBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	int index = tabAt(event->pos());
	if (index >= PVProjectsTabWidget::FIRST_PROJECT_INDEX) {
		rename_tab(index);
	}
	QTabBar::mouseDoubleClickEvent(event);
}

void PVGuiQt::__impl::PVTabBar::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton) {
		int index = tabAt(event->pos());
		if (index >= PVProjectsTabWidget::FIRST_PROJECT_INDEX) {
			QMenu* menu = new QMenu(this);
			QAction* rename_action = menu->addAction("&Rename...");
			rename_action->setData(qVariantFromValue(index));
			connect(rename_action, SIGNAL(triggered(bool)), this, SLOT(rename_tab()));
			menu->popup(event->globalPos());
		}
	}
	QTabBar::mousePressEvent(event);
}

void PVGuiQt::__impl::PVTabBar::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_F2) {
		int index = currentIndex();
		if (index >= PVProjectsTabWidget::FIRST_PROJECT_INDEX) {
			rename_tab(index);
		}
	}
	QTabBar::keyPressEvent(event);
}

void PVGuiQt::__impl::PVTabBar::rename_tab()
{
	QAction* rename_action = (QAction*)sender();
	assert(rename_action);
	int index = rename_action->data().toInt();
	rename_tab(index);
}

void PVGuiQt::__impl::PVTabBar::rename_tab(int index)
{
	QString tab_name = tabText(index);
	bool add_star = false;
	if (tab_name.endsWith(star)) {
		add_star = true;
		tab_name = tab_name.left(tab_name.size() - 1);
	}
	QString name = QInputDialog::getText(this, "Rename data collection",
	                                     "New data collection name:", QLineEdit::Normal, tab_name);
	if (!name.isEmpty()) {
		setTabText(index, name + (add_star ? star : ""));
		Inendi::PVScene* scene = _root.current_scene();
		assert(scene);
		Inendi::PVScene_p scene_p = scene->shared_from_this();
		PVHive::call<FUNC(Inendi::PVScene::set_name)>(scene_p, name.toStdString());
	}
}

/******************************************************************************
 *
 * PVGuiQt::PVProjectsTabWidget
 *
 *****************************************************************************/
PVGuiQt::PVProjectsTabWidget::PVProjectsTabWidget(Inendi::PVRoot* root, QWidget* parent /*= 0*/)
    : QWidget(parent), _root(root)
{
	assert(root);
	setObjectName("PVProjectsTabWidget");

	QHBoxLayout* main_layout = new QHBoxLayout();

	_tab_widget = new __impl::PVTabWidget(*root);
	_tab_widget->setTabsClosable(true);

	_stacked_widget = new QStackedWidget();

	create_unclosable_tabs();

	_splitter = new __impl::PVSplitter(Qt::Horizontal);
	_splitter->setChildrenCollapsible(true);
	_splitter->addWidget(_tab_widget);
	_splitter->addWidget(_stacked_widget);
	_splitter->setStretchFactor(0, 0);
	_splitter->setStretchFactor(1, 1);
	int tab_width = _tab_widget->tabBar()->tabRect(0).width();
	((__impl::PVSplitterHandle*)_splitter->handle(1))->set_max_size(tab_width);
	QList<int> sizes;
	sizes << 1 << 2;
	_splitter->setSizes(sizes);

	main_layout->addWidget(_splitter);

	setLayout(main_layout);

	connect(_tab_widget, SIGNAL(currentChanged(int)), this, SLOT(current_tab_changed(int)));
	connect(_tab_widget->tabBar(), SIGNAL(tabCloseRequested(int)), this,
	        SLOT(tab_close_requested(int)));

	// Hive
	// Register for current scene changing
	Inendi::PVRoot_sp root_sp = root->shared_from_this();
	PVHive::PVObserverSignal<Inendi::PVRoot>* obs =
	    new PVHive::PVObserverSignal<Inendi::PVRoot>(this);
	obs->connect_refresh(this, SLOT(select_tab_from_current_scene()));
	PVHive::get().register_observer(
	    root_sp, [=](Inendi::PVRoot& root) { return root.get_current_scene_hive_property(); },
	    *obs);
}

void PVGuiQt::PVProjectsTabWidget::create_unclosable_tabs()
{
	// Start screen widget
	_start_screen_widget = new PVGuiQt::PVStartScreenWidget();
	_tab_widget->addTab(new QWidget(), "");
	_tab_widget->setTabPosition(QTabWidget::West);
	_tab_widget->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0, 0);

	QPixmap pm(":/inendi");
	QTransform trans;
	_tab_widget->setTabIcon(0, pm.transformed(trans.rotate(90)));

	_tab_widget->setTabToolTip(0, "Start screen");
	_stacked_widget->addWidget(_start_screen_widget);
	connect(_start_screen_widget, SIGNAL(load_source_from_description(PVRush::PVSourceDescription)),
	        this, SIGNAL(load_source_from_description(PVRush::PVSourceDescription)));
	connect(_start_screen_widget, SIGNAL(new_project()), this, SIGNAL(new_project()));
	connect(_start_screen_widget, SIGNAL(load_project()), this, SIGNAL(load_project()));
	connect(_start_screen_widget, SIGNAL(load_project_from_path(const QString&)), this,
	        SIGNAL(load_project_from_path(const QString&)));
	connect(_start_screen_widget, SIGNAL(import_type(const QString&)), this,
	        SIGNAL(import_type(const QString&)));
	connect(_start_screen_widget, SIGNAL(new_format()), this, SIGNAL(new_format()));
	connect(_start_screen_widget, SIGNAL(load_format()), this, SIGNAL(load_format()));
	connect(_start_screen_widget, SIGNAL(edit_format(const QString&)), this,
	        SIGNAL(edit_format(const QString&)));
}

void PVGuiQt::PVProjectsTabWidget::collapse_tabs(bool collapse /* = true */)
{
	int max_size = ((__impl::PVSplitterHandle*)_splitter->handle(1))->get_max_size();
	QList<int> sizes;
	sizes << (collapse ? 0 : max_size) << 1;
	_splitter->setSizes(sizes);
}

PVGuiQt::PVSceneWorkspacesTabWidget*
PVGuiQt::PVProjectsTabWidget::add_project(Inendi::PVScene& scene_p)
{
	PVSceneWorkspacesTabWidget* workspace_tab_widget = new PVSceneWorkspacesTabWidget(scene_p);
	connect(workspace_tab_widget, SIGNAL(workspace_dragged_outside(QWidget*)), this,
	        SLOT(emit_workspace_dragged_outside(QWidget*)));
	connect(workspace_tab_widget, SIGNAL(is_empty()), this, SLOT(close_project()));
	connect(workspace_tab_widget, SIGNAL(project_modified(bool, QString)), this,
	        SLOT(project_modified(bool, QString)));

	int index = _tab_widget->count();
	_tab_widget->insertTab(index, new QWidget(), QString::fromStdString(scene_p.get_name()));
	_stacked_widget->insertWidget(index, workspace_tab_widget);
	_tab_widget->setTabToolTip(index, QString::fromStdString(scene_p.get_name()));
	_tab_widget->setCurrentIndex(index);

	return workspace_tab_widget;
}

void PVGuiQt::PVProjectsTabWidget::project_modified(bool modified, QString path /* = QString */)
{
	PVSceneWorkspacesTabWidget* workspace_tab_widget = (PVSceneWorkspacesTabWidget*)sender();
	assert(workspace_tab_widget);
	int index = _stacked_widget->indexOf(workspace_tab_widget);
	QString text = _tab_widget->tabText(index);
	if (modified && !text.endsWith(star)) {
		_tab_widget->setTabText(index, text + "*");
	} else if (!modified && text.endsWith(star)) {
		if (path.isEmpty()) {
			_tab_widget->setTabText(index, text.left(text.size()));
		} else {
			QFileInfo info(path);
			QString basename = info.fileName();
			_tab_widget->setTabToolTip(index, path);
			_tab_widget->setTabText(index, basename);
		}
	}
}

bool PVGuiQt::PVProjectsTabWidget::save_modified_projects()
{
	for (int i = FIRST_PROJECT_INDEX; i < _tab_widget->count(); i++) {
		PVSceneWorkspacesTabWidget* tab_widget =
		    (PVSceneWorkspacesTabWidget*)_stacked_widget->widget(i);
		if (tab_widget->is_project_modified()) {
			if (!tab_close_requested(i)) {
				return false;
			}
		}
	}

	return true;
}

void PVGuiQt::PVProjectsTabWidget::close_project()
{
	PVSceneWorkspacesTabWidget* workspace_tab_widget = (PVSceneWorkspacesTabWidget*)sender();
	assert(workspace_tab_widget);
	int index = _stacked_widget->indexOf(workspace_tab_widget);
	remove_project(index);
}

bool PVGuiQt::PVProjectsTabWidget::tab_close_requested(int index)
{
	QMessageBox ask(QMessageBox::Question, tr("Close data collection?"),
	                tr("Are you sure you want to close \"%1\"").arg(_tab_widget->tabText(index)),
	                QMessageBox::Yes | QMessageBox::No);
	if (ask.exec() == QMessageBox::Yes) {
		remove_project(index);
		return false;
	}
	return true;
}

PVGuiQt::PVSourceWorkspace* PVGuiQt::PVProjectsTabWidget::add_source(Inendi::PVSource* source)
{
	PVGuiQt::PVSourceWorkspace* workspace = new PVGuiQt::PVSourceWorkspace(source);

	add_workspace(workspace);

	return workspace;
}

void PVGuiQt::PVProjectsTabWidget::add_workspace(PVSourceWorkspace* workspace)
{
	Inendi::PVScene& scene = workspace->get_source()->get_parent<Inendi::PVScene>();
	PVSceneWorkspacesTabWidget* workspace_tab_widget = get_workspace_tab_widget_from_scene(&scene);

	if (!workspace_tab_widget) {
		workspace_tab_widget = add_project(scene);
	}

	const Inendi::PVSource* src = workspace->get_source();
	workspace_tab_widget->add_workspace(workspace, QString::fromStdString(src->get_name()));

	int index = workspace_tab_widget->indexOf(workspace);
	workspace_tab_widget->setTabToolTip(index, src->get_tooltip());
	workspace_tab_widget->setCurrentIndex(index);

	_tab_widget->setCurrentIndex(_stacked_widget->indexOf(workspace_tab_widget));
}

void PVGuiQt::PVProjectsTabWidget::remove_workspace(PVSourceWorkspace* workspace)
{
	Inendi::PVScene& scene = workspace->get_source()->get_parent<Inendi::PVScene>();
	PVSceneWorkspacesTabWidget* workspace_tab_widget = get_workspace_tab_widget_from_scene(&scene);
	workspace_tab_widget->remove_workspace(workspace_tab_widget->indexOf(workspace));
}

void PVGuiQt::PVProjectsTabWidget::remove_project(PVSceneWorkspacesTabWidget* workspace_tab_widget)
{
	remove_project(_stacked_widget->indexOf(workspace_tab_widget));
}

void PVGuiQt::PVProjectsTabWidget::remove_project(int index)
{
	if (index != -1) {
		PVSceneWorkspacesTabWidget* tab_widget =
		    (PVSceneWorkspacesTabWidget*)_stacked_widget->widget(index);
		tab_widget->get_scene().remove_from_tree();
		_tab_widget->removeTab(index);
		_stacked_widget->removeWidget(tab_widget);

		tab_widget->deleteLater();

		if (_tab_widget->count() == FIRST_PROJECT_INDEX) {
			_tab_widget->setCurrentIndex(0);
			Q_EMIT is_empty();
		}
	}
}

void PVGuiQt::PVProjectsTabWidget::current_tab_changed(int index)
{
	_stacked_widget->setCurrentIndex(
	    index); // Map QTabBar signal to QStackedWidget to keep the sync
	_current_workspace_tab_widget_index = index;

	// to report if the active tab is for a project or not (start page or empty
	// worspaces page)
	QWidget* active_widget = _stacked_widget->currentWidget();
	if ((active_widget == _start_screen_widget) ||
	    (_stacked_widget->count() <= FIRST_PROJECT_INDEX)) {
		Q_EMIT active_project(false);
	} else {
		Q_EMIT active_project(true);
	}

	if (index == 0) {
		return;
	}
}

PVGuiQt::PVSceneWorkspacesTabWidget*
PVGuiQt::PVProjectsTabWidget::current_workspace_tab_widget() const
{
	if (_current_workspace_tab_widget_index < 0) {
		return nullptr;
	}

	QWidget* w = _stacked_widget->widget(_current_workspace_tab_widget_index);

	return qobject_cast<PVSceneWorkspacesTabWidget*>(w);
}

PVGuiQt::PVSceneWorkspacesTabWidget*
PVGuiQt::PVProjectsTabWidget::get_workspace_tab_widget_from_scene(const Inendi::PVScene* scene)
{
	for (int i = FIRST_PROJECT_INDEX; i < _stacked_widget->count(); i++) {
		PVSceneWorkspacesTabWidget* workspace_tab_widget =
		    (PVSceneWorkspacesTabWidget*)_stacked_widget->widget(i);
		if (&workspace_tab_widget->get_scene() == scene) {
			return workspace_tab_widget;
		}
	}
	return nullptr;
}

void PVGuiQt::PVProjectsTabWidget::select_tab_from_scene(Inendi::PVScene* scene)
{
	_tab_widget->setCurrentIndex(_tab_widget->indexOf(get_workspace_tab_widget_from_scene(scene)));
}

void PVGuiQt::PVProjectsTabWidget::select_tab_from_current_scene()
{
	Inendi::PVScene* cur_scene = _root->current_scene();
	if (&cur_scene->get_parent<Inendi::PVRoot>() != _root) {
		return;
	}

	select_tab_from_scene(cur_scene);
}
