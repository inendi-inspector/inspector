/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include "PVFieldSplitterRegexpParamWidget.h"
#include "PVFieldSplitterRegexp.h"
#include <pvkernel/filter/PVFieldsFilter.h>
#include <pvkernel/filter/PVElementFilterByFields.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPushButton>

/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::PVFieldSplitterRegexpParamWidget
 *
 *****************************************************************************/
PVFilter::PVFieldSplitterRegexpParamWidget::PVFieldSplitterRegexpParamWidget()
    : PVFieldsSplitterParamWidget(PVFilter::PVFieldsSplitter_p(new PVFieldSplitterRegexp()))
{
	validator_textEdit = NULL;
	PVLOG_DEBUG("constructor PVFieldSplitterRegexpParamWidget\n");

	expressionChanged = false;
}

/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::initWidget
 *
 *****************************************************************************/
void PVFilter::PVFieldSplitterRegexpParamWidget::initWidget()
{
	PVCore::PVArgumentList l = get_filter()->get_args();
	expression_lineEdit = new QLineEdit(l["regexp"].toString());
	child_count_text = new QLabel("Expression validator");
	validator_textEdit = new QTextEdit(get_data().join("\n"));
	table_validator_TableWidget = new QTableWidget();
	btn_apply = new QPushButton(tr("Apply"));
	fullline_checkBox =
	    new QCheckBox(tr("Match the regular expression on the whole line\n(warning: disabling this "
	                     "can cause severe performance loss !)"));
	bool fullline = l["full-line"].toBool();
	fullline_checkBox->setChecked(fullline);
}

/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::get_action_menu
 *
 *****************************************************************************/
QAction* PVFilter::PVFieldSplitterRegexpParamWidget::get_action_menu(QWidget* parent)
{
	return new QAction(QString("add RegExp Splitter"), parent);
}

/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::get_param_widget
 *
 *****************************************************************************/
QWidget* PVFilter::PVFieldSplitterRegexpParamWidget::get_param_widget()
{
	PVLOG_DEBUG("get_param_widget PVFieldSplitterRegexpParamWidget\n");
	initWidget();

	param_widget = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(param_widget);
	param_widget->setLayout(layout);
	param_widget->setObjectName("splitter");

	// Expression
	layout->addWidget(new QLabel("Expression"));
	layout->addWidget(expression_lineEdit);

	// Full line match
	layout->addWidget(fullline_checkBox);

	// Child count
	layout->addWidget(child_count_text);

	// Validator zone
	layout->addWidget(validator_textEdit);
	layout->addWidget(table_validator_TableWidget);
	layout->addWidget(btn_apply);

	connect(expression_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotExpressionChanged()));
	connect(fullline_checkBox, SIGNAL(stateChanged(int)), this, SLOT(slotFullineChanged(int)));
	connect(validator_textEdit, SIGNAL(textChanged()), this, SLOT(slotUpdateTableValidator()));

	update_data_display();
	slotUpdateTableValidator();
	return param_widget;
}
/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::slotExpressionChanged
 *
 *****************************************************************************/
void PVFilter::PVFieldSplitterRegexpParamWidget::slotExpressionChanged()
{
	expressionChanged = true;
	// child count
	QRegExp reg = QRegExp(expression_lineEdit->text());
	set_child_count(reg.captureCount());

	PVCore::PVArgumentList l = get_filter()->get_args();
	l["regexp"] = PVCore::PVArgument(expression_lineEdit->text());

	try {
		get_filter()->set_args(l);
	} catch (PVFilter::PVFieldsFilterInvalidArguments const&) {
		// Don't throw an exception here because the user is currently
		// typing the regex and it can therefore be temporarily malformed
	}

	emit args_changed_Signal();
	emit nchilds_changed_Signal();

	slotUpdateTableValidator();
}

void PVFilter::PVFieldSplitterRegexpParamWidget::slotFullineChanged(int state)
{
	expressionChanged = true;
	PVCore::PVArgumentList l = get_filter()->get_args();
	l["full-line"] = PVCore::PVArgument(state == Qt::Checked);
	get_filter()->set_args(l);

	emit args_changed_Signal();
	slotUpdateTableValidator();
}

/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::slotUpdateTableValidator
 *
 *****************************************************************************/
void PVFilter::PVFieldSplitterRegexpParamWidget::slotUpdateTableValidator()
{
	PVLOG_DEBUG("slotUpdateTableValidator() : PVFieldSplitterRegexpParamWidget\n");
	PVCol nfields;
	{
		QRegExp reg(expression_lineEdit->text());
		nfields = reg.captureCount();
		// update the number of column
		table_validator_TableWidget->setColumnCount(nfields);
	}

	// feed each line with the matching in text zone.
	QStringList myText = validator_textEdit->toPlainText().split("\n");
	table_validator_TableWidget->setRowCount(myText.count());

	PVFilter::PVElementFilterByFields elt_f(_filter->f());

	// Text selections list
	QList<QTextEdit::ExtraSelection> rx_sels;
	QTextEdit::ExtraSelection txt_sel;
	txt_sel.format.setBackground(QBrush(QColor(Qt::gray).lighter(100)));
	txt_sel.cursor = validator_textEdit->textCursor();
	uint64_t line_index = 0;
	for (PVRow line = 0; line < (PVRow)myText.count(); line++) { // for each line...
		QString myLine = myText.at(line);
		const QChar* start = myLine.constData();
		PVCore::PVElement elt(NULL, (char*)start, (char*)(start + myLine.size()));
		elt.fields().push_back(PVCore::PVField(elt, elt.begin(), elt.end()));
		// Filter this element
		elt_f(elt);
		if (elt.valid()) {
			PVCore::list_fields& lf = elt.fields();
			PVCore::list_fields::iterator it;
			PVCol col = 0;
			for (it = lf.begin(); it != lf.end(); it++) {
				PVCore::PVField& out_f = *it;
				// Create a deep copy of that field
				QString deep_copy((const QChar*)out_f.begin(), out_f.size() / sizeof(QChar));

				// Compute indexes for text selection
				uintptr_t index_start =
				    ((uintptr_t)out_f.begin() - (uintptr_t)start) / sizeof(QChar);
				uintptr_t index_end = ((uintptr_t)out_f.end() - (uintptr_t)start) / sizeof(QChar);

				// Set the item in the "validation table"
				table_validator_TableWidget->setItem(line, col, new QTableWidgetItem(deep_copy));

				// Colorize the field in the original text
				txt_sel.cursor.setPosition(line_index + index_start);
				txt_sel.cursor.setPosition(line_index + index_end, QTextCursor::KeepAnchor);
				rx_sels << txt_sel;

				col++;
			}
		}
		line_index += myLine.size() + 1;
	}
	table_validator_TableWidget->setContentsMargins(3, 0, 3, 0);
	validator_textEdit->setExtraSelections(rx_sels);
}

/******************************************************************************
 *
 * PVFilter::PVFieldSplitterRegexpParamWidget::update_data_display
 *
 *****************************************************************************/
void PVFilter::PVFieldSplitterRegexpParamWidget::update_data_display()
{
	PVLOG_DEBUG("update_data_display() : PVFieldSplitterRegexpParamWidget\n");
	if (validator_textEdit) {
		validator_textEdit->setText(get_data().join("\n"));
	}
}
