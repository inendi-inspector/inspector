/**
 * \file PVSerializeArchiveZip.h
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#ifndef PVCORE_PVSERIALIZEARCHIVEZIP_H
#define PVCORE_PVSERIALIZEARCHIVEZIP_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVSerializeArchive.h>

namespace PVCore {

class PVSerializeArchiveZip: public PVSerializeArchive
{
public:
	PVSerializeArchiveZip(version_t v);
	PVSerializeArchiveZip(QString const& zip_path, archive_mode mode, version_t v);
	~PVSerializeArchiveZip();

public:
	void open_zip(QString const& zip_path, archive_mode mode);

public:
	virtual void finish();

protected:
	QString _zip_path;
	QString _tmp_path;
};

}

#endif
