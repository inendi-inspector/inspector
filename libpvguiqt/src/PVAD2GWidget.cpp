/**
 * \file PVAD2GWidget.cpp
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#include <QFrame>

#include <tulip/EdgeExtremityGlyphManager.h>
#include <tulip/Interactor.h>
#include <tulip/InteractorManager.h>
#include <tulip/TlpQtTools.h>
#include <tulip/TlpTools.h>

#include <pvguiqt/PVAD2GWidget.h>
#include <pvguiqt/PVAD2GInteractor.h>

#include <picviz/widgets/PVAD2GListEdgesWidget.h>

#include <picviz/PVSource.h>
#include <picviz/PVView.h>
#include <picviz/PVView_types.h>

#include <pvhive/PVHive.h>
#include <pvhive/PVCallHelper.h>

namespace PVGuiQt {

class AD2GNodeLinkDiagramComponent : public tlp::NodeLinkDiagramComponent
{
public:
	// Disable view standard context menu
	virtual void buildContextMenu(QObject*, QContextMenuEvent*, QMenu*){};
	virtual void computeContextMenuAction(QAction*){};
};

}

void PVGuiQt::__impl::PVTableWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		_dragStartPosition = event->pos();
	}

	QTableWidget::mousePressEvent(event);
}

void PVGuiQt::__impl::PVTableWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - _dragStartPosition).manhattanLength() < QApplication::startDragDistance())
		return;

	QTableWidgetItem* item = ((PVGuiQt::PVAD2GWidget*) parent())->get_table()->currentItem();

	if (item && (item->flags() & Qt::ItemIsEnabled)) {
		QDrag* drag = new QDrag(this);
		QMimeData* mimeData = new QMimeData;
		void* ptr = item->data(Qt::UserRole).value<void*>();

		QByteArray byte_array;
		byte_array.reserve(sizeof(void*));
		byte_array.append((const char*)&ptr, sizeof(void*));

		mimeData->setData("application/x-qabstractitemmodeldatalist", byte_array);

		drag->setMimeData(mimeData);

		drag->exec(Qt::CopyAction | Qt::MoveAction);
	}

}

bool PVGuiQt::__impl::FilterDropEvent::eventFilter(QObject* /*object*/, QEvent *event)
{
	if(event->type() == QEvent::DragEnter)
	{
		QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*>(event);
		dragEnterEvent->accept(); // dragEnterEvent->acceptProposedAction();
		return true;
	}
	else if (event->type() == QEvent::Drop) {

		QDropEvent* dropEvent = static_cast<QDropEvent*>(event);

		if (dropEvent->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
			dropEvent->acceptProposedAction();

			const QMimeData* mimeData = dropEvent->mimeData();
			QByteArray itemData = mimeData->data("application/x-qabstractitemmodeldatalist");

			if (itemData.size() < (int)sizeof(Picviz::PVView**)) {
				return false;
			}
			Picviz::PVView* view = *(reinterpret_cast<Picviz::PVView* const*>(itemData.constData()));

			_widget->add_view(dropEvent->pos(), view);

			return true;
		}
	}

	return false;
}

PVGuiQt::PVAD2GWidget::PVAD2GWidget(Picviz::PVAD2GView_p ad2g, Picviz::PVRoot& root, QWidget* parent):
	QWidget(parent),
	_ad2g(ad2g),
	_graph(_ad2g->get_graph()),
	_root(root)
{
	// Widgets
	_nodeLinkView = new AD2GNodeLinkDiagramComponent();
	QWidget* nodeWidget = _nodeLinkView->construct(this);
	_table = new __impl::PVTableWidget(this);
	_list_edges_widget = new PVWidgets::PVAD2GListEdgesWidget(*_ad2g);

	// Layout
	QHBoxLayout* graph_views_layout = new QHBoxLayout();
	QHBoxLayout* node_widget_layout = new QHBoxLayout();
	node_widget_layout->addWidget(nodeWidget);
	QFrame* graph_frame = new QFrame();
	graph_frame->setFrameStyle(QFrame::Panel);
	graph_frame->setLayout(node_widget_layout);
	graph_views_layout->addWidget(graph_frame);
	graph_views_layout->addWidget(_table);
	graph_views_layout->setStretchFactor(graph_frame, 3);
	graph_views_layout->setStretchFactor(_table, 1);
	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->addLayout(graph_views_layout);
	main_layout->addWidget(_list_edges_widget);
	setLayout(main_layout);

	_ad2g_interactor = new AD2GInteractor(this, _nodeLinkView->getGlMainWidget());
	_nodeLinkView->setActiveInteractor(_ad2g_interactor);

	_nodeLinkView->hideOverview(true);

	tlp::DataSet dataSet;
	dataSet.set<bool>("arrow", true);
	dataSet.set<bool>("nodeLabel", true);
	dataSet.set<bool>("edgeLabel", false);

	_nodeLinkView->init();
	nodeWidget->setAcceptDrops(true);
	nodeWidget->installEventFilter(new __impl::FilterDropEvent(this));

	init_table();
	update_list_views();
	update_list_edges();

	if (_graph) {
		openGraphOnGlMainWidget(_graph, &dataSet, _nodeLinkView->getGlMainWidget());

		// Set up graph rendering properties
		tlp::GlGraphRenderingParameters params = _nodeLinkView->getGlMainWidget()->getScene()->getGlGraphComposite()->getRenderingParameters();
		params.setEdgeColorInterpolate(false);
		params.setViewArrow(true);
		params.setAntialiasing(true);
		params.setViewNodeLabel(true);
		_nodeLinkView->getGlMainWidget()->getScene()->getGlGraphComposite()->setRenderingParameters(params);

		tlp::ColorProperty* color_property = _graph->getLocalProperty<tlp::ColorProperty>("viewColor");
		color_property->setAllEdgeValue(tlp::Color(142, 142, 142));
		_graph->getProperty<tlp::IntegerProperty>("viewFontSize")->setAllNodeValue(12);
		_graph->getLocalProperty<tlp::ColorProperty>("viewLabelColor")->setAllNodeValue(tlp::Color(255, 255, 255));
		//_graph->getProperty<tlp::IntegerProperty>("viewShape")->setAllEdgeValue(4); // Bezier curve
		_graph->getProperty<tlp::IntegerProperty>("viewSrcAnchorShape")->setAllEdgeValue(-1);
		_graph->getProperty<tlp::IntegerProperty>("viewTgtAnchorShape")->setAllEdgeValue(50); // 28 for the Christmas Tree ! ;-)
		_graph->getProperty<tlp::SizeProperty>("viewTgtAnchorSize")->setAllEdgeValue(tlp::Size(0.5, 0.5, 0.5));

		initObservers();
	}
}


PVGuiQt::PVAD2GWidget::~PVAD2GWidget()
{
	clearObservers();
	delete _nodeLinkView;
}

void PVGuiQt::PVAD2GWidget::add_view_Slot(QObject* mouse_event)
{
	QMouseEvent* event = (QMouseEvent*) mouse_event;
	auto view_p = get_root().get_children<Picviz::PVView>()[_table->currentRow()];
	add_view(event->pos(), view_p.get());
}


tlp::node PVGuiQt::PVAD2GWidget::add_view(QPoint pos, Picviz::PVView* view)
{
	tlp::Observable::holdObservers();

	// Add view to graph
	Picviz::PVAD2GView_p ad2g_view_p = _ad2g->shared_from_this();
	tlp::node newNode = PVHive::call<FUNC(Picviz::PVAD2GView::add_view)>(ad2g_view_p, view);


	// Compute view position
	tlp::Graph* graph = _nodeLinkView->getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getGraph();
	tlp::Coord point((double) _nodeLinkView->getGlMainWidget()->width() - (double) pos.x(),(double) pos.y(), 0);
	point = _nodeLinkView->getGlMainWidget()->getScene()->getCamera().screenTo3DWorld(point);
	tlp::Coord cameraDirection = _nodeLinkView->getGlMainWidget()->getScene()->getCamera().getEyes() - _nodeLinkView->getGlMainWidget()->getScene()->getCamera().getCenter();
	if(cameraDirection[0]==0 && cameraDirection[1]==0)
		point[2]=0;
	tlp::LayoutProperty* mLayout = graph->getProperty<tlp::LayoutProperty>(_nodeLinkView->getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getElementLayoutPropName());
	mLayout->setNodeValue(newNode, point);

	// Disable the view from the list of views
	_table->setCurrentCell(-1, -1);
	set_enabled_view_item_in_table(view, false);

	// Add node text
	tlp::StringProperty* label = graph->getProperty<tlp::StringProperty>("viewLabel");
	label->setNodeValue(newNode, qPrintable(QString::number(view->get_display_view_id())));

	// Add node color
	QColor c = view->get_color();
	tlp::ColorProperty* color_property = _graph->getLocalProperty<tlp::ColorProperty>("viewColor");
	color_property->setNodeValue(newNode, tlp::Color(c.red(), c.green(), c.blue()));

	// Set view id property
	graph->getProperty<tlp::IntegerProperty>("view_id")->setNodeValue(newNode, view->get_display_view_id());

	tlp::Observable::unholdObservers();

	return newNode;
}

void PVGuiQt::PVAD2GWidget::remove_view_Slot(int node)
{
	QMessageBox* box = new QMessageBox(QMessageBox::Question, tr("Confirm remove."), tr("Do you really want to remove this view?"), QMessageBox::Yes | QMessageBox::No, this);
	if (box->exec() == QMessageBox::Yes) {

		tlp::node n = (tlp::node) node;
		tlp::Observable::holdObservers();

		// Enable view in the list of views
		Picviz::PVView* view = _ad2g->get_view(n);
		set_enabled_view_item_in_table(view, true);

		_list_edges_widget->clear_current_edge();

		Picviz::PVAD2GView_p ad2g_view_p = _ad2g->shared_from_this();
		PVHive::call<FUNC(Picviz::PVAD2GView::del_view_by_node)>(ad2g_view_p, n);

		_list_edges_widget->update_list_edges();

		_nodeLinkView->getGlMainWidget()->redraw();
		tlp::Observable::unholdObservers();
	}
}

tlp::edge PVGuiQt::PVAD2GWidget::add_combining_function(const tlp::node source, const tlp::node target)
{
	Picviz::PVCombiningFunctionView_p cf_sp(new Picviz::PVCombiningFunctionView());

	//Picviz::PVAD2GView_p ad2g_view_p = _ad2g->shared_from_this();
	//tlp::edge newEdge = PVHive::call<FUNC_PROTOTYPE(void, Picviz::PVAD2GView, set_edge_f, tlp::node, tlp::node, Picviz::PVCombiningFunctionView_p)>(ad2g_view_p, source, target, cf_sp);

	tlp::edge newEdge = _ad2g->set_edge_f(source, target, cf_sp);

	_list_edges_widget->update_list_edges();

	_nodeLinkView->elementSelectedSlot(newEdge.id, false);

	return newEdge;
}

void PVGuiQt::PVAD2GWidget::remove_combining_function_Slot(int edge)
{
	QMessageBox* box = new QMessageBox(QMessageBox::Question, tr("Confirm remove."), tr("Do you really want to remove this combining function?"), QMessageBox::Yes | QMessageBox::No, this);
	if (box->exec() == QMessageBox::Yes) {
		tlp::edge e = (tlp::edge) edge;

		tlp::Observable::holdObservers();

		_list_edges_widget->clear_current_edge();

		_ad2g->del_edge(e);

		_list_edges_widget->update_list_edges();

		_nodeLinkView->getGlMainWidget()->redraw();
		Observable::unholdObservers();
	}
}

/*void PVGuiQt::PVAD2GWidget::select_edge(Picviz::PVView* view_src, Picviz::PVView* view_dst)
{
	tlp::node src = _ad2g->get_graph_node(view_src);
	tlp::node dst = _ad2g->get_graph_node(view_dst);
	tlp::edge edge = _graph->existEdge(src, dst);
	tlp::Graph* graph = _nodeLinkView->getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getGraph();
	graph->getProperty<tlp::ColorProperty>("viewColor")->setEdgeValue(edge, tlp::Color(255, 0, 0));
}*/

void PVGuiQt::PVAD2GWidget::edit_combining_function(tlp::edge edge, tlp::node src, tlp::node dst)
{

	Picviz::PVView* view_src = _ad2g->get_view(src);
	Picviz::PVView* view_dst = _ad2g->get_view(dst);
	Picviz::PVCombiningFunctionView_p combining_function = _ad2g->get_edge_f(edge);

	tlp::Graph* graph = _nodeLinkView->getGlMainWidget()->getScene()->getGlGraphComposite()->getInputData()->getGraph();
	tlp::IntegerProperty* view_id_property = graph->getProperty<tlp::IntegerProperty>("view_id");
	int src_view_id = view_id_property->getNodeValue(src);
	int dst_view_id = view_id_property->getNodeValue(dst);

	_list_edges_widget->select_row(src_view_id, dst_view_id);

	_ad2g->set_selected_edge(view_src, view_dst);

	//_edge_editor->update(*view_src, *view_dst, *combining_function);
}

void PVGuiQt::PVAD2GWidget::initObservers()
{
	if (_graph) {
		_graph->addObserver(this);
		tlp::Iterator<tlp::PropertyInterface*> *it = _graph->getObjectProperties();

		while (it->hasNext()) {
			tlp::PropertyInterface* tmp = it->next();
			tmp->addObserver(this);
		}

		delete it;
	}
}

void PVGuiQt::PVAD2GWidget::highlightViewItem(tlp::node n)
{
	/*Picviz::PVView* view = _ad2g->get_view(n);
	for (int i = 0; i < _table->rowCount(); i++) {
		QTableWidgetItem* item = _table->item(i, 0);
		item->setSelected(item->data(Qt::UserRole).value<void*>() == (void*) view && n != tlp::node());
	}*/
}

void PVGuiQt::PVAD2GWidget::init_table()
{
	_table->setColumnCount(1);

	_table->horizontalHeader()->hide();
	_table->horizontalHeader()->setStretchLastSection(true);

	_table->setDragEnabled(true);
	_table->setDragDropMode(QAbstractItemView::DragOnly);
	_table->setSelectionMode(QAbstractItemView::SingleSelection);
}

void PVGuiQt::PVAD2GWidget::update_list_views()
{
	_table->clear();
	_table->setRowCount(0);

	tlp::StringProperty* label = _graph->getProperty<tlp::StringProperty>("viewLabel");
	tlp::IntegerProperty* view_id_prop = _graph->getProperty<tlp::IntegerProperty>("view_id");

	auto all_views = get_root().get_children<Picviz::PVView>();
	_table->setRowCount(all_views.size());

	int index = 0;
	for (auto view_p : all_views) {
		QString name = QString("%1 - %2").arg(view_p->get_parent<Picviz::PVSource>()->get_name()).arg(view_p->get_name());
		QTableWidgetItem* item = new QTableWidgetItem(name);
		item->setBackground(QBrush(view_p->get_color()));
		item->setToolTip(view_p->get_window_name());
		item->setData(Qt::UserRole, qVariantFromValue((void*) view_p.get()));
		_table->setItem(index++, 0, item);
	}

	// Disable all the view present in the graph from the list of views
	tlp::node node;
	forEach(node, _graph->getNodes()) {
		Picviz::PVView* view = _ad2g->get_view(node);
		label->setNodeValue(node, qPrintable(QString::number(view->get_display_view_id())));
		view_id_prop->setNodeValue(node, view->get_display_view_id());
		set_enabled_view_item_in_table(view, false);
	}

	_table->resizeRowsToContents();
}

void PVGuiQt::PVAD2GWidget::set_enabled_view_item_in_table(Picviz::PVView* view, bool enabled)
{
	for (int i = 0; i < _table->rowCount(); i++) {
		QTableWidgetItem* item = _table->item(i, 0);
		if (item && item->data(Qt::UserRole).value<void*>() == (void*) view) {
			if (enabled) {
				item->setFlags(item->flags() | Qt::ItemIsEnabled);
			}
			else {
				item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
			}
		}
	}
}

void PVGuiQt::PVAD2GWidget::clearObservers()
{
	if (_graph) {
		_graph->removeObserver(this);
		tlp::Iterator<tlp::PropertyInterface*> *it = _graph->getObjectProperties();

		while (it->hasNext()) {
			(it->next())->removeObserver(this);
		}

		delete it;
	}
}

void PVGuiQt::PVAD2GWidget::update(ObserverIterator /*begin*/, ObserverIterator /*end*/)
{
	_nodeLinkView->draw();
}

void PVGuiQt::PVAD2GWidget::update_list_edges()
{
	if (_list_edges_widget) {
		_list_edges_widget->update_list_edges();
	}
}