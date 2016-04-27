/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <PVCustomStyle.h>

PVInspector::PVCustomStyle::PVCustomStyle() : QProxyStyle(/*new QGtkStyle()*/)
{
}

void PVInspector::PVCustomStyle::drawComplexControl(ComplexControl control,
                                                    const QStyleOptionComplex* option,
                                                    QPainter* painter, const QWidget* widget) const
{
	switch (control) {
	// case QStyle::CC_TitleBar:
	//	return;
	default:
		QProxyStyle::drawComplexControl(control, option, painter, widget);
	};
}

void PVInspector::PVCustomStyle::drawControl(ControlElement element, const QStyleOption* option,
                                             QPainter* painter, const QWidget* widget) const
{
	switch (element) {
	// case QStyle::CE_PushButton:
	//	break;
	default:
		QProxyStyle::drawControl(element, option, painter, widget);
	};
}
