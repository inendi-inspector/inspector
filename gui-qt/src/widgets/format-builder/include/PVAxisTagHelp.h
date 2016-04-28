/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVAXISTAGHELP_FILE_H
#define PVAXISTAGHELP_FILE_H

#include <pvkernel/core/general.h>
#include <ui_PVAxisTagHelp.h>
#include <QDialog>

namespace PVInspector
{

class PVAxisTagHelp : public QDialog, private Ui::PVAxisTagHelp
{
	Q_OBJECT
  public:
	PVAxisTagHelp(QStringList sel_tags = QStringList(), QWidget* parent = NULL);

  protected:
	template <class T>
	QString tag_to_classes_name(T const& tag)
	{
		QString used_by;
		typename T::list_classes const& filters = tag.associated_classes();
		typename T::list_classes::const_iterator it_c;
		for (it_c = filters.begin(); it_c != filters.end(); it_c++) {
			used_by += (*it_c)->registered_name() + "\n";
		}
		return used_by;
	}
};
}

#endif
