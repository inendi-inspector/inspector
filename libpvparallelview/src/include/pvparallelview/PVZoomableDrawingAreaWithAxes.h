
#ifndef PVPARALLELVIEW_PVZOOMABLEDRAWINGAREAWITHAXES_H
#define PVPARALLELVIEW_PVZOOMABLEDRAWINGAREAWITHAXES_H

#include <pvparallelview/PVZoomableDrawingArea.h>

#include <QString>

class QWidget;
class QResizeEvent;

namespace PVParallelView
{

/**
 * @class PVZoomableDrawingAreaWithAxes
 *
 * A canvas to draw charts.
 *
 * @note As this widget use a QScrollArea and as all widgets use left-hand
 * coordinates system, this widget must follow this constraint. However, all
 * the internal computations use right-hand coordinates system to render
 * correctly the decoration and to interact with it. So that, your subclass
 * must make do:
 * - the scene must be defined as QRect(0, -N, M, N);
 * - your calculation must consider that ordinates are negative instead of
 *   being positive (doing a -v is generally enough to do the things right);
 * - the zoom's range and initial value must be properly set to have the
 *   expected viewport (for example, for [0,2^32]^2, the zoom range will be
 *   [-22, 0] for the default zoom behaviour);
 * - strictly positive value for the zoom range upper bound will permit to
 *   zoom further than theorical value precision.
 *
 * @note the scales are continuous, not discrete.
 *
 * @todo make sure that scene resizing properly works, for a chart like the
 * "hitgraph" view, changing ordinates range has not been tested.
 *
 * @todo axes actually hide "0" values. They also must be moved (one pixel to
 * the bottom for the abscissae and one pixel to the left for ordinates").
 */

class PVZoomableDrawingAreaWithAxes : public PVZoomableDrawingArea
{
public:
	/**
	 * CTOR
	 */
	PVZoomableDrawingAreaWithAxes(QWidget *parent = nullptr);

	/**
	 * DTOR
	 */
	~PVZoomableDrawingAreaWithAxes();

	/**
	 * Set the color to use when drawing scales & legends
	 *
	 * @param color the color to use
	 */
	void set_decoration_color(const QColor &color);

	/**
	 * Return the color used when drawing scales & legends
	 *
	 */
	QColor get_decoration_color() const
	{
		return _decoration_color;
	}

	/**
	 * Set the legend for the horizontal axis
	 *
	 * @param legend the text to use
	 */
	void set_x_legend(const QString &legend);

	/**
	 * Return the legend of the horizontal axis
	 */
	const QString &get_x_legend() const
	{
		return _x_legend;
	}

	/**
	 * Set the legend for the vertical axis
	 *
	 * @param legend the text to use
	 */
	void set_y_legend(const QString &legend);

	/**
	 * Return the legend of the vertical axis
	 */
	const QString &get_y_legend() const
	{
		return _y_legend;
	}

	/**
	 * Set the ticks count per level
	 */
	void set_ticks_count(int count);

	/**
	 * Return the current ticks count
	 */
	int get_ticks_count() const
	{
		return _ticks_count;
	}

protected:
	/**
	 * Return the text to print in x scale for a given value
	 *
	 * This method has to be overloaded in a subclass to display
	 * information related to the view.
	 *
	 * @param value the value for which we want a text
	 *
	 * @return the label corresponding to value
	 */
	virtual QString get_x_value_at(const qint64 value) const;

	/**
	 * Return the text to print on the y scale for a given value
	 *
	 * This method has to be overloaded in a subclass to display
	 * information related to the view.
	 *
	 * @param value the value for which we want a text
	 *
	 * @return the label corresponding to value
	 */
	virtual QString get_y_value_at(const qint64 value) const;

protected:
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void drawForeground(QPainter *painter, const QRectF &rect);
	virtual void resizeEvent(QResizeEvent *event);

private:
	QColor  _decoration_color;
	QString _x_legend;
	QString _y_legend;

	int _ticks_count;

	bool _first_resize;
};

}

#endif // PVPARALLELVIEW_PVZOOMABLEDRAWINGAREAWITHAXES_H