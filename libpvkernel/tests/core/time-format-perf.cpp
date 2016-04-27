/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVDateTimeParser.h>

#include <tbb/tick_count.h>

#include <QCoreApplication>
#include <QString>

#include <iostream>

#include <pvkernel/core/inendi_stat.h>

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);

	PVCore::PVDateTimeParser parser(QStringList() << QString("dd/MMM/yyyy"));
	UErrorCode err = U_ZERO_ERROR;
	Calendar* cal = Calendar::createInstance(err);

	QString time("01/Jan/2000");
	if (!parser.mapping_time_to_cal(time, cal)) {
		std::cerr << "Go and fix your time string, it can't be parsed !" << std::endl;
		return 1;
	}

	tbb::tick_count start, end;
	start = tbb::tick_count::now();
	int i;
	for (i = 0; i < 1000000; i++) {
		parser.mapping_time_to_cal(time, cal);
	}
	end = tbb::tick_count::now();
	PV_STAT_PROCESS_BW("mapping_bw", i / (end - start).seconds());

	return 0;
}
