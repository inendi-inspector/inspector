/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/general.h>
#include <pvkernel/widgets/editors/PVRegexpEditor.h>

/******************************************************************************
 *
 * PVCore::PVRegexpEditor::PVRegexpEditor
 *
 *****************************************************************************/
PVWidgets::PVRegexpEditor::PVRegexpEditor(QWidget *parent):
	QLineEdit(parent)
{
}

/******************************************************************************
 *
 * PVWidgets::PVRegexpEditor::~PVRegexpEditor
 *
 *****************************************************************************/
PVWidgets::PVRegexpEditor::~PVRegexpEditor()
{
}

void PVWidgets::PVRegexpEditor::set_rx(QRegExp rx)
{
	setText(rx.pattern());
}

QRegExp PVWidgets::PVRegexpEditor::get_rx() const
{
	return QRegExp(text());
}
