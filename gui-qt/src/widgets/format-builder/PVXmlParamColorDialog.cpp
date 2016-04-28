/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <PVXmlParamColorDialog.h>

/******************************************************************************
 *
 * PVInspector::PVXmlParamColorDialog::PVXmlParamColorDialog
 *
 *****************************************************************************/
PVInspector::PVXmlParamColorDialog::PVXmlParamColorDialog(QString name,
                                                          QString p_color,
                                                          QWidget* p_parent)
    : QPushButton(p_color, p_parent)
{
	setObjectName(name);
	setColor(p_color);
	setText(p_color);
	parent = p_parent;
	connect(this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamColorDialog::~PVXmlParamColorDialog
 *
 *****************************************************************************/
PVInspector::PVXmlParamColorDialog::~PVXmlParamColorDialog()
{
	disconnect(this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamColorDialog::chooseColor
 *
 *****************************************************************************/
void PVInspector::PVXmlParamColorDialog::chooseColor()
{
	// qDebug()<<"PVXmlParamColorDialog::chooseColor()";

	QColorDialog cd(parent);
	QColor initialColor(color);
	QColor colorChoosed = cd.getColor(initialColor, parent);
	if (colorChoosed.isValid()) {
		setColor(colorChoosed.name());
		emit changed();
	}
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamColorDialog::setColor
 *
 *****************************************************************************/
void PVInspector::PVXmlParamColorDialog::setColor(QString newColor)
{
	color = newColor;
	setText(newColor);
	QString css = QString("background-color:%1;").arg(color);
	setStyleSheet(css);
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamColorDialog::getColor
 *
 *****************************************************************************/
QString PVInspector::PVXmlParamColorDialog::getColor()
{
	return color;
}
