/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <PVXmlParamTextEdit.h>
#include <PVXmlRegValidatorHighLight.h>
#include <PVXmlTimeValidatorHighLight.h>

#include <QSizePolicy>

/******************************************************************************
 *
 * PVInspector::PVXmlParamTextEdit::PVXmlParamTextEdit
 *
 *****************************************************************************/
PVInspector::PVXmlParamTextEdit::PVXmlParamTextEdit(QString pName, QVariant var) : QTextEdit()
{
	setObjectName(pName);
	variable = var.toString();
	setText(variable);

	typeOfTextEdit = text;
	editing = true;

	QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
	sp.setHeightForWidth(sizePolicy().hasHeightForWidth());
	setSizePolicy(sp);

	connect(this, SIGNAL(textChanged()), this, SLOT(slotHighLight()));
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamTextEdit::~PVXmlParamTextEdit
 *
 *****************************************************************************/
PVInspector::PVXmlParamTextEdit::~PVXmlParamTextEdit() = default;

/******************************************************************************
 *
 * PVInspector::PVXmlParamTextEdit::setRegEx
 *
 *****************************************************************************/
void PVInspector::PVXmlParamTextEdit::setRegEx()
{
	setRegEx(((PVXmlParamTextEdit*)sender())->toPlainText());
}

void PVInspector::PVXmlParamTextEdit::setRegEx(const QString& regExp)
{
	if (editing) {
		highlight = new PVXmlRegValidatorHighLight(
		    (PVXmlParamTextEdit*)this); // we are in regexp validator case
		editing = false;
	}
	typeOfTextEdit = regexpValid; // define type as a regexp validator
	highlight->setRegExp(regExp);
	highlight->rehighlight();
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamTextEdit::slotHighLight
 *
 *****************************************************************************/
void PVInspector::PVXmlParamTextEdit::slotHighLight()
{
	if (typeOfTextEdit == dateValid) {
		// qDebug()<<"PVXmlParamTextEdit::slotHighLight for axis";
		// timeValid->rehighlight();
	}
}

/******************************************************************************
 *
 * QVariant PVInspector::PVXmlParamTextEdit::getVal
 *
 *****************************************************************************/
QVariant PVInspector::PVXmlParamTextEdit::getVal() const
{
	// variable=QString(toPlainText());
	return QVariant(toPlainText());
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamTextEdit::setVal
 *
 *****************************************************************************/
void PVInspector::PVXmlParamTextEdit::setVal(const QString& val)
{
	variable = val;
	setText(variable);
}

/******************************************************************************
 *
 * PVInspector::PVXmlParamTextEdit::validDateFormat
 *
 *****************************************************************************/
void PVInspector::PVXmlParamTextEdit::validDateFormat(const QStringList& pFormat)
{
	if (editing) {
		timeValid = new PVXmlTimeValidatorHighLight(this, format); // we are in time validator case
		editing = false;
	}
	typeOfTextEdit = dateValid; // define type as a date validator.
	format = pFormat;
	timeValid->setDateFormat(format); // set the format to validate.
	timeValid->rehighlight();         // update color for validated line
}
