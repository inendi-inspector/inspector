/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2020
 */

#include <pvguiqt/PVPythonScriptWidget.h>

#include <pvkernel/widgets/PVFileDialog.h>
#include <pvguiqt/PVAboutBoxDialog.h>
#include <pvguiqt/PVDisplayViewPythonConsole.h>
#include <pvguiqt/PVPythonCodeEditor.h>

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>

PVGuiQt::PVPythonScriptWidget::PVPythonScriptWidget(QWidget* parent /*= nullptr*/)
    : QGroupBox(tr("Execute Python script after import"), parent)
{
    setCheckable(true);
    setChecked(false);

	QVBoxLayout* layout = new QVBoxLayout(this);

    // Script path
	QHBoxLayout* python_script_path_layout = new QHBoxLayout();
	QWidget* python_script_path_container_widget = new QWidget;
	python_script_path_container_widget->setLayout(python_script_path_layout);
	_python_script_path_radio = new QRadioButton();
	_python_script_path_radio->setAutoExclusive(true);
	QLabel* exec_python_file_label = new QLabel("Python file:");
	_exec_python_file_line_edit = new QLineEdit();
	_exec_python_file_line_edit->setReadOnly(true);
	QPushButton* exec_python_file_browse =  new QPushButton("&Browse...");
	QObject::connect(exec_python_file_browse, &QPushButton::clicked, [=,this]() {
		QString file_path = PVWidgets::PVFileDialog::getOpenFileName(
			this,
			"Browse your python file",
			"",
			QString("Python file (*.py)"));
		if (not file_path.isEmpty()) {
			_exec_python_file_line_edit->setText(file_path);
            _python_script_path_radio->setChecked(true);
            notify_python_script_updated();
		}
	});
	python_script_path_layout->addWidget(_exec_python_file_line_edit);
	python_script_path_layout->addWidget(exec_python_file_browse);
    QHBoxLayout* python_script_path_radio_layout = new QHBoxLayout();
    python_script_path_container_widget->setLayout(python_script_path_layout);
    python_script_path_radio_layout->addWidget(_python_script_path_radio);
    python_script_path_radio_layout->addWidget(exec_python_file_label);
    python_script_path_radio_layout->addWidget(python_script_path_container_widget);

    // Script content
	QHBoxLayout* python_script_content_layout = new QHBoxLayout();
	QWidget* python_script_content_container_widget = new QWidget;
	_python_script_content_radio = new QRadioButton();
	_python_script_content_radio->setAutoExclusive(true);
	QLabel* exec_python_content_label = new QLabel("Python script:");
	_python_script_content_text = new PVGuiQt::PVPythonCodeEditor(PVGuiQt::PVPythonCodeEditor::EThemeType::LIGHT, parent);;

    connect(_python_script_content_text, &QTextEdit::textChanged, this, [this](){
        notify_python_script_updated();
    });
	python_script_content_layout->addWidget(_python_script_content_text);
    QHBoxLayout* python_script_content_radio_layout = new QHBoxLayout();
    python_script_content_container_widget->setLayout(python_script_content_layout);
    python_script_content_radio_layout->addWidget(_python_script_content_radio);
    python_script_content_radio_layout->addWidget(exec_python_content_label);
    python_script_content_radio_layout->addWidget(python_script_content_container_widget);

	QButtonGroup* python_script_radio_group = new QButtonGroup(this);
	python_script_radio_group->addButton(_python_script_path_radio);
	python_script_radio_group->addButton(_python_script_content_radio);

	connect(_python_script_content_radio, &QRadioButton::toggled, this, [=,this]() {
		bool checked = _python_script_content_radio->isChecked();
		python_script_path_container_widget->setEnabled(not checked);
        if (isChecked()) {
		    python_script_content_container_widget->setEnabled(checked);
        }
        notify_python_script_updated();
	});
	connect(_python_script_path_radio, &QRadioButton::toggled, this, [=,this]() {
		bool checked = _python_script_path_radio->isChecked();
        if (isChecked()) {
            python_script_path_container_widget->setEnabled(checked);
        }
		python_script_content_container_widget->setEnabled(not checked);
        notify_python_script_updated();
	});

    connect(this, &QGroupBox::toggled, this, [this](){
        notify_python_script_updated();
    });

    // Help
    QPushButton* help_button = new QPushButton("&Help");
    connect(help_button, &QPushButton::clicked, [=,this]() {
        QVariant data_anchor("_python_scripting");
        PVGuiQt::PVAboutBoxDialog* about_dialog = new PVGuiQt::PVAboutBoxDialog(PVGuiQt::PVAboutBoxDialog::Tab::REFERENCE_MANUAL, this, data_anchor);
        about_dialog->exec();
        about_dialog->deleteLater();
    });

	layout->addLayout(python_script_path_radio_layout);
	layout->addLayout(python_script_content_radio_layout);
    layout->addWidget(help_button);
}


void PVGuiQt::PVPythonScriptWidget::set_python_script(const QString& python_script, bool is_path, bool disabled)
{
    _python_script_path_radio->setChecked(is_path); 
    _python_script_content_radio->setChecked(not is_path); 
    if (is_path) {
        _exec_python_file_line_edit->setText(python_script);
    }
    else {
        _python_script_content_text->setText(python_script);
    }
    setChecked(not disabled);
}

QString PVGuiQt::PVPythonScriptWidget::get_python_script(bool& is_path, bool& disabled) const
{
    is_path = _python_script_path_radio->isChecked();
    disabled = not isChecked();
    if (is_path) {
        return _exec_python_file_line_edit->text();
    }
    else {
        return _python_script_content_text->toPlainText();
    }
}

void PVGuiQt::PVPythonScriptWidget::notify_python_script_updated()
{
    bool is_path, disabled;
    const QString& python_script = get_python_script(is_path, disabled);

    Q_EMIT python_script_updated(python_script, is_path, disabled);
}