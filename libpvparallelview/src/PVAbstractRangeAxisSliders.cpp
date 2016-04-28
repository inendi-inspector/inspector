/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvparallelview/PVAbstractRangeAxisSliders.h>
#include <pvparallelview/PVSlidersGroup.h>

#include <QPainter>
#include <QGraphicsScene>

/*****************************************************************************
 * PVParallelView::PVAbstractRangeAxisSliders::PVAbstractRangeAxisSliders
 *****************************************************************************/

PVParallelView::PVAbstractRangeAxisSliders::PVAbstractRangeAxisSliders(
    QGraphicsItem* parent, PVSlidersManager_p sm_p, PVParallelView::PVSlidersGroup* group,
    const char* text)
    : PVAbstractAxisSliders(parent, sm_p, group, text)
{
}

/*****************************************************************************
 * PVParallelView::PVAbstractRangeAxisSliders::~PVAbstractRangeAxisSliders
 *****************************************************************************/

PVParallelView::PVAbstractRangeAxisSliders::~PVAbstractRangeAxisSliders()
{
	if (_sl_min) {
		if (scene()) {
			scene()->removeItem(_sl_min);
		}
		removeFromGroup(_sl_min);
		delete _sl_min;
	}
	if (_sl_max) {
		if (scene()) {
			scene()->removeItem(_sl_max);
		}
		removeFromGroup(_sl_max);
		delete _sl_max;
	}
}

/*****************************************************************************
 * PVParallelView::PVAbstractRangeAxisSliders::paint
 *****************************************************************************/

void PVParallelView::PVAbstractRangeAxisSliders::paint(QPainter* painter,
                                                       const QStyleOptionGraphicsItem* option,
                                                       QWidget* widget)
{
	qreal vmin = _sl_min->pos().y();
	_text->setPos(0, vmin);

	if (is_moving()) {
		painter->save();

		painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);

		painter->setPen(QPen(Qt::white, 0));
		qreal vmax = _sl_max->pos().y();
		painter->drawLine(QPointF(0., vmin), QPointF(0., vmax));

		_text->show();

		painter->restore();
	} else {
		_text->hide();
	}

	PVAbstractAxisSliders::paint(painter, option, widget);
}
