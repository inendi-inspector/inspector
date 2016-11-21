/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

/* Test case for ticket 1.
 * Here, we try to reproduce this bug by reading a lot of format consecutively, and checking if,
 * for the regexp, the correct one is choosen. This is due to th fact that thread-local objects are
 * used
 * the regexp object, so that it cnap ersist between different formats.
 */

#include <pvkernel/core/PVMeanValue.h>

#include <pvkernel/rush/PVFormat.h>
#include <pvkernel/rush/PVUnicodeSource.h>
#include <pvkernel/rush/PVInputFile.h>
#include <pvkernel/rush/PVSourceCreatorFactory.h>
#include <pvkernel/rush/PVPluginsLoad.h>
#include <pvkernel/rush/PVFileDescription.h>

#include <pvkernel/filter/PVPluginsLoad.h>
#include <pvkernel/core/PVArgument.h>

#include <QDir>
#include <QStringList>

#include <iostream>

#include "helpers.h"
#include "common.h"

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " test-files-directory" << std::endl;
		return 1;
	}

	// Initialisation
	pvtest::init_ctxt();

	// Format reading
	QDir dir_files(QString::fromLocal8Bit(argv[1]) + QLatin1String("/tickets/1/"));
	QStringList files = dir_files.entryList(QStringList() << QString("*.format"),
	                                        QDir::Files | QDir::Readable, QDir::Name);

	// Text file source creator
	PVRush::PVSourceCreator_p text_file_lib =
	    LIB_CLASS(PVRush::PVSourceCreator)::get().get_class_by_name("text_file");

	if (files.size() == 0) {
		std::cerr << "No files in tickets/1/. Test failed." << std::endl;
		return 1;
	}

	for (int i = 0; i < files.size(); i++) {
		QFileInfo fiformat(files[i]);
		QString file_load = dir_files.absoluteFilePath(fiformat.completeBaseName());

		PVLOG_INFO("Test file %s...\n", qPrintable(file_load));

		bool ok = false;
		// Check the result
		for (int j = 0; j < files.size(); j++) {
			QString const& formatpath = files[j];
			QString fpath = dir_files.absoluteFilePath(formatpath);
			PVRush::PVFormat format("format", fpath);

			PVRush::PVInputDescription_p file_arg(new PVRush::PVFileDescription(file_load));
			float sr = PVRush::PVSourceCreatorFactory::discover_input(
			    PVRush::pair_format_creator(format, text_file_lib), file_arg);
			if (sr > 0.8 && files[j] == files[i]) {
				ok = true;
			}
		}
		if (!ok) {
			std::cerr << "Format discovery failed !\n" << std::endl;
			return 1;
		}
	}

	return 0;
}
