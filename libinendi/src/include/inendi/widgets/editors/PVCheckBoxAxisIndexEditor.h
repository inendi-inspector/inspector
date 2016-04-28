/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCHECKBOXAXISINDEXEDITOR_H
#define PVCHECKBOXAXISINDEXEDITOR_H

#include <QWidget>
#include <inendi/PVView.h>
#include <pvkernel/core/general.h>
#include <pvkernel/core/PVCheckBoxAxisIndexType.h>

namespace PVWidgets
{
class PVMainWindow;

/**
 * \class PVCheckBoxAxisIndexEditor
 */
class PVCheckBoxAxisIndexEditor : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(
	    PVCore::PVCheckBoxAxisIndexType _color_slider READ get_values WRITE set_values USER true)

	float kx;
	float ky;
	QPointF last_mouse_press_position;
	float left_margin;
	QPolygonF left_slider_polygon;
	QPainter* painter;
	int SELECTED_SLIDER;
	float right_margin;
	QPolygonF right_slider_polygon;
	float SLIDER_WIDTH;
	float sliders_positions[2];
	QPolygonF sliders_relative_extents;
	float XMAX;
	float YMAX;

  public:
	/**
	 * Constructor
	 */
	PVCheckBoxAxisIndexEditor(Inendi::PVView const& view, QWidget* parent = 0);

	/**
	 * Gets the index of the closest sliders from position x
	 *
	 * @param x The absciss position (float) of the mouse on the slider
	 *
	 * @return The index (int) of the selected slider
	 */
	int get_selected_slider_index(float x);

	/**
	 * Get the actual value of the slider
	 *
	 * @param index
	 *
	 * @return The value of the given slider
	 */
	float get_slider_value(int index);
	QSize sizeHint() const;

  public:
	PVCore::PVCheckBoxAxisIndexType get_values() const;
	void set_values(PVCore::PVCheckBoxAxisIndexType v);

  public slots:
	void toggle_visibility_Slot();

  signals:
	void value_changed_Signal();

  protected:
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);

  protected:
	Inendi::PVView const& _view;
};
}

#endif // PVCHECKBOXAXISINDEXEDITOR_H
