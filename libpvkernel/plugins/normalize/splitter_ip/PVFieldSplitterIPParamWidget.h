/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVFIELDSPLITTERIPPARAMWIDGET_H
#define PVFIELDSPLITTERIPPARAMWIDGET_H

#include <pvkernel/core/general.h>
#include <pvkernel/filter/PVFieldsFilterParamWidget.h>

class QWidget;
class QAction;
class QCheckBox;
class QRadioButton;
class QComboBox;
class QLabel;
#include <QList>

#include <pvkernel/widgets/qkeysequencewidget.h>

namespace PVFilter {

class PVFieldSplitterIPParamWidget: public PVFieldsSplitterParamWidget
{
	Q_OBJECT

public:
	PVFieldSplitterIPParamWidget();

public:
	QAction* get_action_menu();
	QWidget* get_param_widget();

private slots:
	void set_ip_type(bool reset_groups_check_state = true);
	void update_child_count();

private:
	void set_groups_check_state(bool check_all = false);

private:
	QAction* _action_menu;

	QRadioButton* _ipv4;
	QRadioButton* _ipv6;

	QList<QCheckBox*> _cb_list;
	QList<QLabel*> _label_list;

	size_t _group_count;

private:
	CLASS_REGISTRABLE_NOCOPY(PVFieldSplitterIPParamWidget)
};

}

#endif // PVFIELDSPLITTERIPPARAMWIDGET_H
