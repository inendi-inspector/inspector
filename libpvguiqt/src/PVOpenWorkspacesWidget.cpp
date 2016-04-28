/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvdisplays/PVDisplaysImpl.h>
#include <pvguiqt/PVWorkspace.h>
#include <pvguiqt/PVOpenWorkspacesWidget.h>
#include <pvguiqt/PVWorkspacesTabWidget.h>
#include <pvguiqt/PVRootTreeModel.h>
#include <pvguiqt/PVRootTreeView.h>

#include <inendi/PVRoot.h>
#include <inendi/PVView.h>

#include <inendi/PVMapped.h>
#include <inendi/PVPlotted.h>

#include <pvkernel/widgets/PVDataTreeMaskProxyModel.h>
#include <inendi/PVSelection.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QSplitter>

namespace PVGuiQt
{
namespace __impl
{

class RootTreeModelViewsSelectable : public PVGuiQt::PVRootTreeModel
{
  public:
	RootTreeModelViewsSelectable(PVCore::PVDataTreeObjectBase& root, QObject* parent = 0)
	    : PVGuiQt::PVRootTreeModel(root, parent)
	{
	}

  public:
	Qt::ItemFlags flags(const QModelIndex& index) const
	{
		PVCore::PVDataTreeObjectBase const* obj =
		    (PVCore::PVDataTreeObjectBase const*)index.internalPointer();
		Inendi::PVView const* view = dynamic_cast<Inendi::PVView const*>(obj);
		Qt::ItemFlags flags = PVGuiQt::PVRootTreeModel::flags(index);
		if (view) {
			flags |= Qt::ItemIsSelectable;
		} else {
			flags &= ~Qt::ItemIsSelectable;
		}
		return flags;
	}
};
}
}

PVGuiQt::PVOpenWorkspacesWidget::PVOpenWorkspacesWidget(Inendi::PVRoot* root, QWidget* parent)
    : QWidget(parent)
{
	typedef PVWidgets::PVDataTreeMaskProxyModel<Inendi::PVMapped> maping_mask_proxy_t;
	typedef PVWidgets::PVDataTreeMaskProxyModel<Inendi::PVPlotted> plotting_mask_proxy_t;

	// Layouts
	//
	QHBoxLayout* main_layout = new QHBoxLayout();

	// to make projects and workspaces tab aligned
	main_layout->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout* left_layout = new QVBoxLayout();

	// to make left widgets be aligned with the right widgets
	left_layout->setContentsMargins(0, 0, 0, 0);
	left_layout->setSpacing(0);

	// Widgets
	//
	QSplitter* main_splitter = new QSplitter(Qt::Horizontal, this);

	// Open workspaces
	_tab_widget = new PVOpenWorkspacesTabWidget(*root);

	// Data tree from PVRoot
	__impl::RootTreeModelViewsSelectable* tree_model =
	    new __impl::RootTreeModelViewsSelectable(*root);

	// the mask proxies
	maping_mask_proxy_t* mapping_mask_proxy = new maping_mask_proxy_t();
	mapping_mask_proxy->setSourceModel(tree_model);
	plotting_mask_proxy_t* plotting_mask_proxy = new plotting_mask_proxy_t();
	plotting_mask_proxy->setSourceModel(mapping_mask_proxy);

	_root_view = new PVRootTreeView(plotting_mask_proxy);

	_root_view->expandAll();
	_root_view->setContextMenuPolicy(Qt::NoContextMenu); // Disable data-tree creation context menu
	_root_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// View creation tab bar
	QToolBar* toolbar = new QToolBar();
	toolbar->setIconSize(QSize(24, 24));

	PVDisplays::get().visit_displays_by_if<PVDisplays::PVDisplayViewIf>(
	    [&](PVDisplays::PVDisplayViewIf& obj) {
		    if (!obj.match_flags(PVDisplays::PVDisplayIf::UniquePerParameters)) {
			    QToolButton* btn = new QToolButton(toolbar);

			    QAction* act = new QAction(btn);
			    act->setIcon(obj.toolbar_icon());
			    act->setToolTip(obj.tooltip_str());

			    QVariant var;
			    var.setValue<void*>(&obj);
			    act->setData(var);

			    connect(act, SIGNAL(triggered()), this, SLOT(create_views_widget()));

			    btn->setDefaultAction(act);
			    toolbar->addWidget(btn);
		    }
		},
	    PVDisplays::PVDisplayIf::ShowInToolbar);

	// Composition of everyone
	//

	left_layout->addWidget(toolbar);
	left_layout->addWidget(_root_view);
	QWidget* left_widget = new QWidget(this);
	left_widget->setAutoFillBackground(true);
	left_widget->setLayout(left_layout);

	main_splitter->addWidget(left_widget);
	main_splitter->addWidget(_tab_widget);

	// Workspaces tab widget isn't collapsible
	main_splitter->setCollapsible(1, false);
	QList<int> sizes;
	sizes << 1 << 1;
	main_splitter->setSizes(sizes);

	main_layout->addWidget(main_splitter);

	setLayout(main_layout);
}

void PVGuiQt::PVOpenWorkspacesWidget::create_views_widget()
{
	QAction* act = qobject_cast<QAction*>(sender());
	if (!act) {
		return;
	}

	PVOpenWorkspace* cur_workspace = _tab_widget->current_workspace_or_create();

	PVDisplays::PVDisplayViewIf& interface =
	    *(reinterpret_cast<PVDisplays::PVDisplayViewIf*>(act->data().value<void*>()));
	_root_view->visit_selected_objs_as<Inendi::PVView>([&](Inendi::PVView* view) {
		QAction* creation_act = PVDisplays::get().action_bound_to_params(interface, view);
		cur_workspace->create_view_widget(creation_act);
	});
}
