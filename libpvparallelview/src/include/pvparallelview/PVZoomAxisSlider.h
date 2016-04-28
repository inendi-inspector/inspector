/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVPARALLELVIEW_PVZOOMAXISSLIDER_H
#define PVPARALLELVIEW_PVZOOMAXISSLIDER_H

#include <pvparallelview/PVAbstractAxisSlider.h>

namespace PVParallelView
{

class PVZoomAxisSlider : public PVAbstractAxisSlider
{
  public:
	PVZoomAxisSlider(int64_t omin, int64_t omax, int64_t o,
	                 PVAxisSliderOrientation orientation = Min)
	    : PVAbstractAxisSlider(omin, omax, o, orientation)
	{
	}

	virtual QRectF boundingRect() const;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
	                   QWidget* widget = 0);
};
}

#endif // PVPARALLELVIEW_PVZOOMAXISSLIDER_H
