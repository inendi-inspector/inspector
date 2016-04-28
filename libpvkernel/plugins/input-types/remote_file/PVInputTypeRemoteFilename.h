/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVINPUTTYPEREMOTEFILENAME_H
#define INENDI_PVINPUTTYPEREMOTEFILENAME_H

#include <pvkernel/core/general.h>
#include <pvkernel/rush/PVInputType.h>

#include "../file/PVInputTypeFilename.h"

#include <QString>
#include <QIcon>
#include <QCursor>

namespace PVRush
{

class PVInputTypeRemoteFilename : public PVInputTypeFilename
{
  public:
	PVInputTypeRemoteFilename();

  public:
	bool createWidget(hash_formats const& formats,
	                  hash_formats& new_formats,
	                  list_inputs& inputs,
	                  QString& format,
	                  PVCore::PVArgumentList& args_ext,
	                  QWidget* parent = NULL) const;
	QString name() const;
	QString human_name() const;
	QString human_name_serialize() const;
	QString internal_name() const override;
	QString human_name_of_input(PVInputDescription_p in) const;
	QString menu_input_name() const;
	QString tab_name_of_inputs(list_inputs const& in) const;
	QKeySequence menu_shortcut() const;
	bool get_custom_formats(PVInputDescription_p in, hash_formats& formats) const;

	QIcon icon() const { return QIcon(":/import-icon-white"); }
	QCursor cursor() const { return QCursor(Qt::PointingHandCursor); }

  protected:
	mutable QHash<QString, QUrl> _hash_real_filenames;

	CLASS_REGISTRABLE_NOCOPY(PVInputTypeRemoteFilename)
};
}

#endif
