/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_PVAXISINDEXCHECKBOXEDITOR_H
#define PVCORE_PVAXISINDEXBHECKBOXEDITOR_H

#include <QComboBox>
#include <QCheckBox>
#include <QString>
#include <QWidget>

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVAxisIndexCheckBoxType.h>

#include <picviz/PVView.h>

namespace PVWidgets {

class PVAxisIndexCheckBoxEditor : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(PVCore::PVAxisIndexCheckBoxType _axis_index READ get_axis_index WRITE set_axis_index USER true)

private:
	QComboBox *combobox;
	QCheckBox *checkbox;
	bool _checked;
	int _current_index;

public:
	PVAxisIndexCheckBoxEditor(Picviz::PVView const& view, QWidget *parent = 0);
	virtual ~PVAxisIndexCheckBoxEditor();

	PVCore::PVAxisIndexCheckBoxType get_axis_index() const;
	void set_axis_index(PVCore::PVAxisIndexCheckBoxType axis_index);

protected:
	Picviz::PVView const& _view;
};

}

#endif // PVCORE_PVAXISINDEXCHECKBOXEDITOR_H
