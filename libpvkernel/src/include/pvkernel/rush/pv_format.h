/**
 * \file pv_format.h
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#ifndef PVCORE_PVFORMAT_H
#define PVCORE_PVFORMAT_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QHash>
#include <QList>

#include <pvkernel/core/general.h>

/**
 * \class PVRush::Format
 * \defgroup Format Input Formating
 * \brief Formating a log file
 * @{
 *
 * A format is used to know how to split the input file or buffer in columns. It is based on a XML description
 * Then is then used by the normalization part.
 *
 */


class PVAxisFormat {
	private:
		QString color;
		QString mapping;
		QString name;
		QString plotting;
		QString title_color;
		QString type;

	public:
		PVAxisFormat();
		~PVAxisFormat();

		QString get_color()const{return color;}
		QString get_mapping()const{return mapping;}
		QString get_name()const{return name;}
		QString get_plotting()const{return plotting;}
		QString get_title_color()const{return title_color;}
		QString get_type()const{return type;}


		void set_color(QString str);
		void set_mapping(QString str);
		void set_name(QString str);
		void set_plotting(QString str);
		void set_title_color(QString str);
		void set_type(QString str);

};


class PVFormat {
	private:


	public:
		PVFormat();
		~PVFormat();


};
/*@}*/

#endif	/* PVCORE_PVFORMAT_H */
