/**
 * \file pvdecoder.h
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#ifndef PVCORE_DECODE_H
#define PVCORE_DECODE_H

#include <QLibrary>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QVector>

#include <pvkernel/core/general.h>
#include <pvkernel/rush/PVFormat.h>

#ifndef pvrush_decoder_run_string 
    #define pvrush_decoder_run_string "pvrush_decoder_run"
#endif

typedef int (*pvrush_decoder_run_function)(PVRush::PVFormat *format, QVector<QStringList> *normalized_list, QHash<QString, QString> decoderopt);

namespace PVRush {

	class LibKernelDecl PVDecoderFunction {
	public:
		QLibrary *lib;	/* We need this to destroy it later */
		pvrush_decoder_run_function function;
	};

	class LibKernelDecl PVDecoder {
  	private:
		PVDecoderFunction function;
		PVRush::PVFormat *format;
		QString name;
		QString type;
  	public:
  		PVDecoder(PVRush::PVFormat *decoder_format, QString decoder_type, QString decoder_name);

		QStringList decode(QString str);
  	};


	class LibKernelDecl PVDecoderFactory {
	public:
  		PVDecoderFactory();
  		~PVDecoderFactory();

		int plugins_register_all();
		int plugins_register_one(QString filename);

		QHash<QString, PVRush::PVDecoder> functions;
  	};

#ifndef pvrush_decoder_run_string 
    #define pvrush_decoder_run_string "pvrush_decoder_run"
#endif

typedef int (*pvrush_decoder_run_function)(PVRush::PVFormat *format, QVector<QStringList> *normalized_list, QHash<QString, QString> decoderopt);

	class LibKernelDecl DecodeFunctions {
	public:
		QLibrary *lib;	/* We need this to destroy it later */
		pvrush_decoder_run_function decode_function;
	};

	class LibKernelDecl Decode {
		public:
			Decode();
			~Decode();

			int plugins_register_all();
			int plugins_register_one(QString filename);
			int decode(PVRush::PVFormat *format, QString decoder, QVector<QStringList> *normalized);

			QHash<QString, PVRush::DecodeFunctions> functions;
	};

	LibKernelDecl QStringList decoders_get_plugins_dirs();
};


#endif	/* PVCORE_DECODE_H */
