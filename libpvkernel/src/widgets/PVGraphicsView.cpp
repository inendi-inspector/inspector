#include <pvkernel/widgets/PVGraphicsView.h>

#include <QGridLayout>
#include <QGraphicsScene>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <QEvent>
#include <QGraphicsSceneWheelEvent>
#include <QScrollBar64>

// to mimic QGraphicsView::::sizeHint() :-D
#include <QApplication>
#include <QDesktopWidget>

#include <iostream>

static inline qint64 sb_round(const qreal &d)
{
	if (d <= (qreal) INT64_MIN)
		return INT64_MIN;
	else if (d >= (qreal) INT64_MAX)
		return INT64_MAX;
	return (d > (qreal)0.0) ? floor(d + (qreal)0.5) : ceil(d - (qreal)0.5);
}

#define print_rect(R) __print_rect(#R, R)

template <typename R>
void __print_rect(const char *text, const R &r)
{
	std::cout << text << ": "
	          << r.x() << " " << r.y() << ", "
	          << r.width() << " " << r.height()
	          << std::endl;
}

#define print_point(P) __print_point(#P, P)

template <typename P>
void __print_point(const char *text, const P &p)
{
	std::cout << text << ": "
	          << p.x() << " " << p.y()
	          << std::endl;
}

#define print_transform(T) __print_transform(#T, T)

template <typename T>
void __print_transform(const char *text, const T &t)
{
	std::cout << text << ": " << std::endl
	          << t.m11() << " " << t.m21() << " " << t.m31() << std::endl
	          << t.m12() << " " << t.m22() << " " << t.m32() << std::endl
	          << t.m13() << " " << t.m23() << " " << t.m33() << std::endl;
}

namespace PVWidgets { namespace __impl {

class PVViewportEventFilter: public QObject
{
public:
	PVViewportEventFilter(PVWidgets::PVGraphicsView* view):
		_view(view)
	{ }

protected:
	bool eventFilter(QObject* obj, QEvent* event)
	{
		if (obj == _view->get_viewport() &&
		    event->type() == QEvent::Paint) {
			return _view->viewportPaintEvent(static_cast<QPaintEvent*>(event));
		}

		return QObject::eventFilter(obj, event);
	}

private:
	PVWidgets::PVGraphicsView* _view;
};

} }

/*****************************************************************************
 * PVWidgets::PVGraphicsView::PVGraphicsView
 *****************************************************************************/

PVWidgets::PVGraphicsView::PVGraphicsView(QWidget *parent)
	: QWidget(parent), _viewport(nullptr), _scene(nullptr)
{
	init();
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::PVGraphicsView
 *****************************************************************************/

PVWidgets::PVGraphicsView::PVGraphicsView(QGraphicsScene *scene, QWidget *parent)
	: QWidget(parent), _viewport(nullptr), _scene(nullptr)
{
	init();
	set_scene(scene);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_viewport
 *****************************************************************************/

QWidget *PVWidgets::PVGraphicsView::get_viewport() const
{
	return _viewport;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_scene
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_scene(QGraphicsScene *scene)
{
	if(_scene && hasFocus()) {
            _scene->clearFocus();
	}

	_scene = scene;

	if (_scene == nullptr) {
		return;
	}

	recompute_viewport();

	if (hasFocus()) {
		_scene->setFocus();
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::map_to_scene
 *****************************************************************************/

QPointF PVWidgets::PVGraphicsView::map_to_scene(const QPointF &p) const
{
	QPointF np = p + get_scroll();
	return _inv_transform.map(np) + _scene_offset;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::map_to_scene
 *****************************************************************************/

QRectF PVWidgets::PVGraphicsView::map_to_scene(const QRectF &r) const
{
	QRectF tr = r.translated(get_scroll());
	return _inv_transform.mapRect(tr).translated(_scene_offset);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::map_from_scene
 *****************************************************************************/

QPointF PVWidgets::PVGraphicsView::map_from_scene(const QPointF &p) const
{
	QPointF np = _transform.map(p - _scene_offset);
	np -= get_scroll();
	return np;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::map_from_scene
 *****************************************************************************/

QRectF PVWidgets::PVGraphicsView::map_from_scene(const QRectF &r) const
{
	QRectF nr = _transform.mapRect(r.translated(-_scene_offset));
	nr.translate(-get_scroll());
	return nr;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_scene_rect
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_scene_rect(const QRectF &r)
{
	_scene_rect = r;
	recompute_viewport();
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_scene_rect
 *****************************************************************************/

QRectF PVWidgets::PVGraphicsView::get_scene_rect() const
{
	if (_scene_rect.isNull() && (_scene != nullptr)) {
		return _scene->sceneRect();
	} else {
		return _scene_rect;
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_transform
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_transform(const QTransform &t, bool combine)
{
	if (combine) {
		_transform = t * _transform;
	} else {
		_transform = t;
	}

	_inv_transform = _transform.inverted();

	recompute_viewport();

	/* if _transformation_anchor is equal to AnchorUnderMouse while the
	 * mouse is not on the view, there is an translation effect due to the
	 * use of QCursor::pos().
	 * So, when the mouse pointer is outside of the view, AnchorUnderMouse
	 * *must not* be used.
	 */
	if (underMouse()) {
		center_view(_transformation_anchor);
	} else {
		center_view(AnchorViewCenter);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::fit_in_view
 *****************************************************************************/

void PVWidgets::PVGraphicsView::fit_in_view(Qt::AspectRatioMode mode)
{
	if (_scene == nullptr) {
		return;
	}

	set_view(get_scene_rect(), mode);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::center_on
 *****************************************************************************/
void PVWidgets::PVGraphicsView::center_on(const QPointF &pos)
{
	QPointF new_pos = _transform.map(pos - _scene_offset);
	QRectF view_rect = _viewport->rect();
	new_pos -= QPointF(0.5 * view_rect.width(), 0.5 *view_rect.height());

	_hbar->setValue(new_pos.x());
	_vbar->setValue(new_pos.y());
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_horizontal_scrollbar_policy
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_horizontal_scrollbar_policy(Qt::ScrollBarPolicy policy)
{
	_hbar_policy = policy;
	recompute_viewport();
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_horizontal_scrollbar_policy
 *****************************************************************************/

Qt::ScrollBarPolicy PVWidgets::PVGraphicsView::get_horizontal_scrollbar_policy() const
{
	return _hbar_policy;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_vertical_scrollbar_policy
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_vertical_scrollbar_policy(Qt::ScrollBarPolicy policy)
{
	_vbar_policy = policy;
	recompute_viewport();
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_vertical_scrollbar_policy
 *****************************************************************************/

Qt::ScrollBarPolicy PVWidgets::PVGraphicsView::get_vertical_scrollbar_policy() const
{
	return _vbar_policy;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_resize_anchor
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_resize_anchor(const PVWidgets::PVGraphicsView::ViewportAnchor anchor)
{
	switch(anchor) {
	case NoAnchor:
	case AnchorViewCenter:
		_resize_anchor = anchor;
		break;
	case AnchorUnderMouse:
		qWarning("anchor AnchorUnderMouse is not supported for resize event");
		break;
	}

	if (anchor == AnchorUnderMouse) {
		_viewport->setMouseTracking(true);
		setMouseTracking(true);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_resize_anchor
 *****************************************************************************/

PVWidgets::PVGraphicsView::ViewportAnchor PVWidgets::PVGraphicsView::get_resize_anchor() const
{
	return _resize_anchor;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_transformation_anchor
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_transformation_anchor(const PVWidgets::PVGraphicsView::ViewportAnchor anchor)
{
	_transformation_anchor = anchor;

	if (anchor == AnchorUnderMouse) {
		_viewport->setMouseTracking(true);
		setMouseTracking(true);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_transformation_anchor
 *****************************************************************************/

PVWidgets::PVGraphicsView::ViewportAnchor PVWidgets::PVGraphicsView::get_transformation_anchor() const
{
	return _transformation_anchor;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_scene_margins
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_scene_margins(const int left,
                                                  const int right,
                                                  const int top,
                                                  const int bottom)
{
	if ((_scene_margin_left != left) || (_scene_margin_right = right) || (_scene_margin_top = top) || (_scene_margin_bottom = bottom)) {
		_scene_margin_left = left;
		_scene_margin_right = right;
		_scene_margin_top = top;
		_scene_margin_bottom = bottom;
		recompute_viewport();
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_alignment
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_alignment(const Qt::Alignment align)
{
	if (_alignment != align) {
		_alignment = align;
		recompute_viewport();
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::viewportPaintEvent
 *****************************************************************************/

bool PVWidgets::PVGraphicsView::viewportPaintEvent(QPaintEvent *event)
{
	if(_scene == nullptr) {
		return false;
	}

	QRectF viewport_rect = event->rect().intersected(_viewport->rect());

	QRect real_viewport_rect = QRect(_scene_margin_left,
	                                 _scene_margin_top,
	                                 get_real_viewport_width(),
	                                 get_real_viewport_height());
	QRectF real_viewport_area = map_to_scene(event->rect().intersected(real_viewport_rect));

	QPainter painter(get_viewport());

	drawBackground(&painter, viewport_rect);
	_scene->render(&painter, real_viewport_rect, real_viewport_area, Qt::IgnoreAspectRatio);
	drawForeground(&painter, viewport_rect);

	return true;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::resizeEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::resizeEvent(QResizeEvent *event)
{
	recompute_viewport();
	center_view(_resize_anchor);

	event->setAccepted(true);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::contextMenuEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
	_mouse_pressed_screen_coord = event->globalPos();
	_mouse_pressed_view_coord = event->pos();
	_mouse_pressed_scene_coord = map_to_scene(_mouse_pressed_view_coord);

	_last_mouse_move_screen_coord = _mouse_pressed_screen_coord;
	_last_mouse_move_scene_coord = _mouse_pressed_scene_coord;

	QGraphicsSceneContextMenuEvent scene_event(QEvent::GraphicsSceneContextMenu);

	scene_event.setScenePos(_mouse_pressed_scene_coord);
	scene_event.setScreenPos(_mouse_pressed_screen_coord);

	scene_event.setModifiers(event->modifiers());
	scene_event.setReason((QGraphicsSceneContextMenuEvent::Reason)(event->reason()));
	scene_event.setWidget(_viewport);
	scene_event.setAccepted(false);

	QApplication::sendEvent(_scene, &scene_event);

	event->setAccepted(scene_event.isAccepted());
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::event
 *****************************************************************************/

bool PVWidgets::PVGraphicsView::event(QEvent *event)
{
	bool ret = QWidget::event(event);

	if (ret) {
		switch(event->type()) {
		case QEvent::Paint:
		case QEvent::UpdateRequest:
			break;
		default:
			update();
			break;
		}
	}

	return ret;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::focusInEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::focusInEvent(QFocusEvent *event)
{
	QWidget::focusInEvent(event);
	if (_scene) {
		QApplication::sendEvent(_scene, event);
	}
	if (!event->isAccepted()) {
		QWidget::focusInEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::focusOutEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::focusOutEvent(QFocusEvent *event)
{
	QWidget::focusOutEvent(event);
	if (_scene) {
		QApplication::sendEvent(_scene, event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::keyPressEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::keyPressEvent(QKeyEvent *event)
{
	if (_scene) {
		QApplication::sendEvent(_scene, event);
	}
	if (!event->isAccepted()) {
		QWidget::keyPressEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::keyReleaseEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
	if (_scene) {
		QApplication::sendEvent(_scene, event);
	}
	if (!event->isAccepted()) {
		QWidget::keyReleaseEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::mouseDoubleClickEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
	_mouse_pressed_button = event->button();

	_mouse_pressed_screen_coord = event->globalPos();
	_mouse_pressed_view_coord = event->pos();
	_mouse_pressed_scene_coord = map_to_scene(_mouse_pressed_view_coord);

	_last_mouse_move_screen_coord = _mouse_pressed_screen_coord;
	_last_mouse_move_scene_coord = _mouse_pressed_scene_coord;

	QGraphicsSceneMouseEvent scene_event(QEvent::GraphicsSceneMouseDoubleClick);

	scene_event.setButtonDownScenePos(_mouse_pressed_button, _mouse_pressed_scene_coord);
	scene_event.setButtonDownScreenPos(_mouse_pressed_button, _mouse_pressed_screen_coord);

	scene_event.setScenePos(_mouse_pressed_scene_coord);
	scene_event.setScreenPos(_mouse_pressed_screen_coord);
	scene_event.setLastScenePos(_last_mouse_move_scene_coord);
	scene_event.setLastScreenPos(_last_mouse_move_screen_coord);

	scene_event.setButtons(event->buttons());
	scene_event.setButton(event->button());
	scene_event.setModifiers(event->modifiers());
	scene_event.setWidget(_viewport);
	scene_event.setAccepted(false);

	QApplication::sendEvent(_scene, &scene_event);

	if (scene_event.isAccepted()) {
		event->setAccepted(true);
	} else {
		QWidget::mouseDoubleClickEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::mouseMoveEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
	QGraphicsSceneMouseEvent scene_event(QEvent::GraphicsSceneMouseMove);

	scene_event.setButtonDownScenePos(_mouse_pressed_button, _mouse_pressed_scene_coord);
	scene_event.setButtonDownScreenPos(_mouse_pressed_button, _mouse_pressed_screen_coord);
	scene_event.setScenePos(map_to_scene(event->pos()));
	scene_event.setScreenPos(event->globalPos());
	scene_event.setLastScenePos(_last_mouse_move_scene_coord);
	scene_event.setLastScreenPos(_last_mouse_move_screen_coord);

	_last_mouse_move_scene_coord = scene_event.scenePos();
	_last_mouse_move_screen_coord = scene_event.screenPos();

	scene_event.setButtons(event->buttons());
	scene_event.setButton(event->button());
	scene_event.setModifiers(event->modifiers());
	scene_event.setWidget(_viewport);
	scene_event.setAccepted(false);

	QApplication::sendEvent(_scene, &scene_event);

	if (scene_event.isAccepted()) {
		event->setAccepted(true);
	} else {
		QWidget::mouseMoveEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::mousePressEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::mousePressEvent(QMouseEvent *event)
{
	_mouse_pressed_button = event->button();

	_mouse_pressed_screen_coord = event->globalPos();
	_mouse_pressed_view_coord = event->pos();
	_mouse_pressed_scene_coord = map_to_scene(_mouse_pressed_view_coord);

	_last_mouse_move_screen_coord = _mouse_pressed_screen_coord;
	_last_mouse_move_scene_coord = _mouse_pressed_scene_coord;

	QGraphicsSceneMouseEvent scene_event(QEvent::GraphicsSceneMousePress);

	scene_event.setButtonDownScenePos(_mouse_pressed_button, _mouse_pressed_scene_coord);
	scene_event.setButtonDownScreenPos(_mouse_pressed_button, _mouse_pressed_screen_coord);

	scene_event.setScenePos(_mouse_pressed_scene_coord);
	scene_event.setScreenPos(_mouse_pressed_screen_coord);
	scene_event.setLastScenePos(_last_mouse_move_scene_coord);
	scene_event.setLastScreenPos(_last_mouse_move_screen_coord);

	scene_event.setButtons(event->buttons());
	scene_event.setButton(event->button());
	scene_event.setModifiers(event->modifiers());
	scene_event.setWidget(_viewport);
	scene_event.setAccepted(false);

	QApplication::sendEvent(_scene, &scene_event);

	if (scene_event.isAccepted()) {
		event->setAccepted(true);
	} else {
		QWidget::mousePressEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::mouseReleaseEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
	QGraphicsSceneMouseEvent scene_event(QEvent::GraphicsSceneMouseRelease);

	scene_event.setWidget(_viewport);
	scene_event.setButtonDownScenePos(_mouse_pressed_button, _mouse_pressed_scene_coord);
	scene_event.setButtonDownScreenPos(_mouse_pressed_button, _mouse_pressed_screen_coord);

	scene_event.setScenePos(map_to_scene(event->pos()));
	scene_event.setScreenPos(event->globalPos());
	scene_event.setLastScenePos(_last_mouse_move_scene_coord);
	scene_event.setLastScreenPos(_last_mouse_move_screen_coord);
	scene_event.setButtons(event->buttons());
	scene_event.setButton(event->button());
	scene_event.setModifiers(event->modifiers());

	scene_event.setAccepted(false);

	QApplication::sendEvent(_scene, &scene_event);

	if (scene_event.isAccepted()) {
		event->setAccepted(true);
	} else {
		QWidget::mouseReleaseEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::wheelEvent
 *****************************************************************************/

void PVWidgets::PVGraphicsView::wheelEvent(QWheelEvent *event)
{
	QGraphicsSceneWheelEvent scene_event(QEvent::GraphicsSceneWheel);

	scene_event.setWidget(_viewport);
	scene_event.setScenePos(map_to_scene(event->pos()));
	scene_event.setScreenPos(event->globalPos());
	scene_event.setButtons(event->buttons());
	scene_event.setModifiers(event->modifiers());
	scene_event.setDelta(event->delta());
	scene_event.setOrientation(event->orientation());
	scene_event.setAccepted(false);

	QApplication::sendEvent(_scene, &scene_event);

	if (scene_event.isAccepted()) {
		event->setAccepted(true);
	} else {
		QWidget::wheelEvent(event);
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::drawBackground
 *****************************************************************************/

void PVWidgets::PVGraphicsView::drawBackground(QPainter *, const QRectF &)
{}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::drawForeground
 *****************************************************************************/

void PVWidgets::PVGraphicsView::drawForeground(QPainter *, const QRectF &)
{}


/*****************************************************************************
 * PVWidgets::PVGraphicsView::sizehint
 *****************************************************************************/

QSize PVWidgets::PVGraphicsView::sizeHint() const
{
	if (_scene) {
		QSizeF s = _transform.mapRect(get_scene_rect()).size();
		return s.boundedTo((3 * QApplication::desktop()->size()) / 4).toSize();
	}
	return QSize(256, 192);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::set_view
 *****************************************************************************/

void PVWidgets::PVGraphicsView::set_view(const QRectF &area, Qt::AspectRatioMode mode)
{
	/* FIXME: not stable when scrollbar are effectively hidden or shown
	 * by ::recompute_viewport because _viewport's size has changed.
	 */
	QTransform transfo;
	qreal viewport_width = get_real_viewport_width();
	qreal viewport_height = get_real_viewport_height();

	qreal x_scale = area.width() / viewport_width;
	qreal y_scale = area.height() / viewport_height;
	qreal tx = area.x();
	qreal ty = area.y();

	switch(mode) {
	case Qt::IgnoreAspectRatio:
		break;
	case Qt::KeepAspectRatio:
		if (x_scale < y_scale) {
			tx -= 0.5 * (viewport_height * y_scale - area.width());
			x_scale = y_scale;
		} else {
			ty -= 0.5 * (viewport_height * x_scale - area.height());
			y_scale = x_scale;
		}
		break;
	case Qt::KeepAspectRatioByExpanding:
		if (x_scale < y_scale) {
			ty -= 0.5 * (viewport_height * x_scale - area.height());
			y_scale = x_scale;
		} else {
			tx -= 0.5 * (viewport_height * y_scale - area.width());
			x_scale = y_scale;
		}
		break;
	}

	_scene_offset = QPointF(tx, ty);
	transfo.scale(1. / x_scale, 1. / y_scale);

	set_transform(transfo);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::init
 *****************************************************************************/

void PVWidgets::PVGraphicsView::init()
{
	_hbar_policy = Qt::ScrollBarAsNeeded;
	_vbar_policy = Qt::ScrollBarAsNeeded;
	_resize_anchor = NoAnchor;
	_transformation_anchor = AnchorViewCenter;

	_transform.reset();
	_inv_transform.reset();

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	_scene_margin_left = 0;
	_scene_margin_right = 0;
	_scene_margin_top = 0;
	_scene_margin_bottom = 0;

	_alignment = Qt::AlignHCenter | Qt::AlignVCenter;

	_layout = new QGridLayout(this);
	_layout->setSpacing(0);
	_layout->setContentsMargins(0, 0, 0, 0);

	setLayout(_layout);

	_viewport_event_filter = new __impl::PVViewportEventFilter(this);

	set_viewport(new QWidget());

	_hbar = new QScrollBar64(Qt::Horizontal);
	_vbar = new QScrollBar64(Qt::Vertical);

	connect(_hbar, SIGNAL(valueChanged(qint64)),
	        _viewport, SLOT(update()));
	connect(_vbar, SIGNAL(valueChanged(qint64)),
	        _viewport, SLOT(update()));

	_layout->addWidget(_hbar, 1, 0);
	_layout->addWidget(_vbar, 0, 1);
}

void PVWidgets::PVGraphicsView::set_viewport(QWidget* w)
{
	if (_viewport) {
		_layout->removeWidget(_viewport);
		get_viewport()->deleteLater();
	}

	_viewport = w;
	// needed to make sure the viewport can received key events
	_viewport->setFocusPolicy(Qt::StrongFocus);
	_viewport->setFocusProxy(this);
	_viewport->installEventFilter(_viewport_event_filter);
	_layout->addWidget(_viewport, 0, 0);
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::recompute_viewport
 *****************************************************************************/

void PVWidgets::PVGraphicsView::recompute_viewport()
{
	qint64 view_width = get_real_viewport_width();
	qint64 view_height = get_real_viewport_height();

	QRectF scene_rect = _transform.mapRect(get_scene_rect().translated(-_scene_offset));

	if (_hbar_policy == Qt::ScrollBarAlwaysOff) {
		_hbar->setRange(0, 0);
		_hbar->setVisible(false);
		_screen_offset_x = compute_screen_offset_x(view_width, scene_rect);
	} else {
		qint64 scene_left = sb_round(scene_rect.left());
		qint64 scene_right = sb_round(scene_rect.right() - view_width);

		if (scene_left >= scene_right) {
			_hbar->setRange(0, 0);
			_hbar->setVisible(_hbar_policy == Qt::ScrollBarAlwaysOn);
			_screen_offset_x = compute_screen_offset_x(view_width, scene_rect);
		} else {
			_hbar->setRange(scene_left, scene_right);
			_hbar->setPageStep(view_width);
			_hbar->setSingleStep(view_width / 20);
			_hbar->setVisible(true);
			_screen_offset_x = _scene_margin_left;
		}
	}

	if (_vbar_policy == Qt::ScrollBarAlwaysOff) {
		_vbar->setRange(0, 0);
		_vbar->setVisible(false);
		_screen_offset_y = compute_screen_offset_y(view_height, scene_rect);
	} else {
		qint64 scene_top = sb_round(scene_rect.top());
		qint64 scene_bottom = sb_round(scene_rect.bottom() - view_height);

		if (scene_top >= scene_bottom) {
			_vbar->setRange(0, 0);
			_vbar->setVisible(_vbar_policy == Qt::ScrollBarAlwaysOn);
			_screen_offset_y = compute_screen_offset_y(view_height, scene_rect);
		} else {
			_vbar->setRange(scene_top, scene_bottom);
			_vbar->setPageStep(view_height);
			_vbar->setSingleStep(view_height / 20);
			_vbar->setVisible(true);
			_screen_offset_y = _scene_margin_top;
		}
	}
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::recompute_screen_offset_x
 *****************************************************************************/

qreal PVWidgets::PVGraphicsView::compute_screen_offset_x(const qint64 view_width,
                                                         const QRectF scene_rect) const
{
	qreal ret = _scene_margin_left;

	switch(_alignment & Qt::AlignHorizontal_Mask) {
	case Qt::AlignLeft:
		break;
	case Qt::AlignRight:
		ret += view_width - (scene_rect.left() + scene_rect.width());
		break;
	case Qt::AlignHCenter:
		ret += 0.5 * (view_width - (scene_rect.left() + scene_rect.right()));
		break;
	}
	return ret;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::recompute_screen_offset_y
 *****************************************************************************/

qreal PVWidgets::PVGraphicsView::compute_screen_offset_y(const qint64 view_height,
                                                         const QRectF scene_rect) const
{
	qreal ret = _scene_margin_top;

	switch(_alignment & Qt::AlignVertical_Mask) {
	case Qt::AlignTop:
		break;
	case Qt::AlignBottom:
		ret += view_height - (scene_rect.top() + scene_rect.height());
		break;
	case Qt::AlignVCenter:
		ret += 0.5 * (view_height - (scene_rect.top() + scene_rect.bottom()));
		break;
	}
	return ret;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::center_view
 *****************************************************************************/

void PVWidgets::PVGraphicsView::center_view(ViewportAnchor anchor)
{
	if (anchor == AnchorViewCenter) {
		QPointF p = map_to_scene(_viewport->rect().center());
		center_on(p);
	} else if (anchor == AnchorUnderMouse) {
		QPointF delta = map_to_scene(_viewport->rect().center());
		delta -= map_to_scene(_viewport->mapFromGlobal(QCursor::pos())
		                      - QPoint(_scene_margin_left, _scene_margin_top));
		center_on(_last_mouse_move_scene_coord + delta);
	} // else if (anchor == NoAnchor) do nothing
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_scroll_x
 *****************************************************************************/

const qreal PVWidgets::PVGraphicsView::get_scroll_x() const
{
	return _hbar->value() - _screen_offset_x;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_scroll_y
 *****************************************************************************/

const qreal PVWidgets::PVGraphicsView::get_scroll_y() const
{
	return _vbar->value() - _screen_offset_y;
}

/*****************************************************************************
 * PVWidgets::PVGraphicsView::get_scroll
 *****************************************************************************/

const QPointF PVWidgets::PVGraphicsView::get_scroll() const
{
	return QPointF(get_scroll_x(), get_scroll_y());
}