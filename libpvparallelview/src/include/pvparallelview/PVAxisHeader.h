/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef __PVPARALLELVIEW_PVAXISHEADER_H__
#define __PVPARALLELVIEW_PVAXISHEADER_H__

#include <inendi/PVAxesCombination.h>

#include <pvbase/types.h>

#include <QGraphicsRectItem>
#include <QEasingCurve>
#include <QGraphicsSceneMouseEvent>

class QPropertyAnimation;
class QPainter;
class QGraphicsSceneMouseEvent;

namespace Inendi
{
class PVView;
}

namespace PVParallelView
{

class PVAxisGraphicsItem;
namespace __impl
{
class PVAxisSelectedAnimation;
}

/**
 * Axis label highlight decoration
 *
 * @note as this class reimplements a single click using mouse press/release,
 * it can interfere with other graphics items. To avoid that problem, each
 * time the "click" mouse button is pressed, the current event is backed-up
 * and resend only when the next event can not lead to a "click" event: press-
 * release is a click but press-move-release is not.
 */

class PVAxisHeader : public QObject, public QGraphicsRectItem
{
	Q_OBJECT
  public:
	using axis_id_t = Inendi::PVAxesCombination::axes_comb_id_t;

  public:
	PVAxisHeader(const Inendi::PVView& view, axis_id_t const& axis_id, PVAxisGraphicsItem* parent);

  public:
	void set_width(int width);
	void start(bool start);

	PVAxisGraphicsItem* axis();
	PVAxisGraphicsItem const* axis() const;
	bool is_last_axis() const;

  protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

  Q_SIGNALS:
	void mouse_hover_entered(PVCol col, bool entered);
	void mouse_clicked(PVCol col);
	void new_zoomed_parallel_view(int _axis_index);
	void new_selection_slider();

  private Q_SLOTS:
	void new_zoomed_parallel_view();

  private:
	PVCol get_axis_index() const;

  private:
	const Inendi::PVView& _view;
	axis_id_t _axis_id;

	__impl::PVAxisSelectedAnimation* _axis_selected_animation;
	bool _started = false;
	bool _clicked;
	QGraphicsSceneMouseEvent _click_event;
};

namespace __impl
{

class PVGraphicsPolygonItem;

class PVAxisSelectedAnimation : QObject
{
	Q_OBJECT

	Q_PROPERTY(qreal opacity READ get_opacity WRITE set_opacity);

  private:
	static constexpr qreal opacity_animation_min_amount = 0.2;
	static constexpr qreal opacity_animation_max_amount = 1.0;
	static constexpr size_t opacity_animation_duration_ms = 200;
	static constexpr QEasingCurve::Type opacity_animation_easing = QEasingCurve::Linear;

  public:
	PVAxisSelectedAnimation(PVAxisHeader* parent);
	~PVAxisSelectedAnimation();

  public:
	void start(bool start);

  private:                                    // properties
	qreal get_opacity() const { return 0.0; } // avoid Qt warning
	void set_opacity(qreal opacity);

  private:
	inline PVAxisHeader* header() { return static_cast<PVAxisHeader*>(parent()); }

  private:
	QPropertyAnimation* _opacity_animation;
	QPropertyAnimation* _blur_animation;

	PVGraphicsPolygonItem* _title_highlight;
};

class PVGraphicsPolygonItem : public QGraphicsPolygonItem
{
	void paint(QPainter* painter,
	           const QStyleOptionGraphicsItem* option,
	           QWidget* widget = nullptr) override;
};
}
}

#endif // __PVPARALLELVIEW_PVAXISHEADER_H__
