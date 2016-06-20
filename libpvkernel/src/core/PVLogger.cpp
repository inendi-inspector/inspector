/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <QDateTime>
#include <QString>

#include <pvkernel/core/PVLogger.h>

#include <iostream>

PVCore::PVLogger::PVLogger()
{
	QByteArray log_level;

	log_filename = qgetenv("INENDI_LOG_FILE");
	datetime_format = "yyyy-MM-dd hh:mm:ss.zzz";

	if (!log_filename.isEmpty()) {
		fp = fopen(log_filename.constData(), "a");
	}

	level = PVLOG_INFO;
	log_level = qgetenv("INENDI_DEBUG_LEVEL");
	if (!log_level.isEmpty()) {
		if (log_level == QString("FATAL")) {
			level = PVLOG_FATAL;
		}
		if (log_level == QString("ERROR")) {
			level = PVLOG_ERROR;
		}
		if (log_level == QString("WARN")) {
			level = PVLOG_WARN;
		}
		if (log_level == QString("INFO")) {
			level = PVLOG_INFO;
		}
		if (log_level == QString("DEBUG")) {
			level = PVLOG_DEBUG;
		}
		if (log_level == QString("HEAVYDEBUG")) {
			level = PVLOG_HEAVYDEBUG;
		}
	}
}

PVCore::PVLogger::~PVLogger()
{
	if (!log_filename.isEmpty()) {
		fclose(fp);
	}
}

QString PVCore::PVLogger::get_now_str()
{
	QDateTime now = QDateTime::currentDateTime();
	return now.toString(datetime_format);
}

void PVCore::PVLogger::heavydebug(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_HEAVYDEBUG)
		return;

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(get_now_str()) << " *** HEAVYDEBUG *** " << qPrintable(res);
	} else {
		fprintf(fp, "%s *** HEAVYDEBUG *** %s", qPrintable(get_now_str()), qPrintable(res));
	}

	va_end(ap);
}

void PVCore::PVLogger::debug(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_DEBUG)
		return;

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(get_now_str()) << " *** DEBUG *** " << qPrintable(res);
	} else {
		fprintf(fp, "%s *** DEBUG *** %s", qPrintable(get_now_str()), qPrintable(res));
	}

	va_end(ap);
}

void PVCore::PVLogger::info(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_INFO)
		return;

	mutex.lock();

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(get_now_str()) << " *** INFO *** " << qPrintable(res);
	} else {
		fprintf(fp, "%s *** INFO *** %s", qPrintable(get_now_str()), qPrintable(res));
	}

	va_end(ap);

	mutex.unlock();
}

void PVCore::PVLogger::warn(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_WARN)
		return;

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(get_now_str()) << " *** WARN *** " << qPrintable(res);
	} else {
		fprintf(fp, "%s *** WARN *** %s", qPrintable(get_now_str()), qPrintable(res));
	}

	va_end(ap);
}

void PVCore::PVLogger::error(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_ERROR)
		return;

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(get_now_str()) << " *** ERROR *** " << qPrintable(res);
	} else {
		fprintf(fp, "%s *** ERROR *** %s", qPrintable(get_now_str()), qPrintable(res));
	}

	va_end(ap);
}

void PVCore::PVLogger::fatal(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_FATAL)
		return;

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(get_now_str()) << " *** FATAL *** " << qPrintable(res);
	} else {
		fprintf(fp, "%s *** FATAL *** %s", qPrintable(get_now_str()), qPrintable(res));
	}

	va_end(ap);
}

void PVCore::PVLogger::plain(const char* format, ...)
{
	QString res;
	va_list ap;

	if (level < PVLOG_INFO)
		return;

	va_start(ap, format);
	res.vsprintf(format, ap);

	if (log_filename.isEmpty()) {
		std::cerr << qPrintable(res);
	} else {
		fprintf(fp, "%s", qPrintable(res));
	}

	va_end(ap);
}

PVCore::PVLogger* PVCore::PVLogger::getInstance()
{
	static PVCore::PVLogger instance;
	return &instance;
}
