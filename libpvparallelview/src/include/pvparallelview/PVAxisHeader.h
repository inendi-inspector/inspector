/**
 * \file PVAxisHeader.h
 *
 * Copyright (C) Picviz Labs 2013
 */
#ifndef __PVPARALLELVIEW_PVAXISHEADER_H__
#define __PVPARALLELVIEW_PVAXISHEADER_H__

#include <pvbase/types.h>

#include <QGraphicsRectItem>
#include <QEasingCurve>
class QPropertyAnimation;
class QPainter;
class QGraphicsSceneMouseEvent;

namespace Picviz
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

class PVSlidersGroup;

class PVAxisHeader : public QObject, public QGraphicsRectItem
{
	Q_OBJECT

public:
	PVAxisHeader(
		const Picviz::PVView &view,
		PVSlidersGroup* sg,
		PVAxisGraphicsItem* parent
	);

public:
	void set_width(int width);
	void start(bool start);

	PVAxisGraphicsItem* axis();
	PVAxisGraphicsItem const* axis() const;
	bool is_last_axis() const;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget * widget = nullptr);

signals:
	void mouse_hover_entered(PVCol col, bool entered);
	void mouse_clicked(PVCol col);
	void new_zoomed_parallel_view(int _axis_index);

private slots:
	void new_zoomed_parallel_view();
	void new_selection_sliders();

private:
	PVCol get_axis_index() const;

private:
	const Picviz::PVView& 				_view;
	PVSlidersGroup*		 				_sliders_group;
	int 								_width;

	__impl::PVAxisSelectedAnimation*	_axis_selected_animation;
	bool								_started = false;
	QPointF								_clicking_pos;
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

private: // properties
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
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

};

}


}

#endif // __PVPARALLELVIEW_PVAXISHEADER_H__