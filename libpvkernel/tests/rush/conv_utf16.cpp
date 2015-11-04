/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#define SIMULATE_PIPELINE
#include <pvkernel/core/picviz_intrin.h>
#include <pvkernel/rush/PVInputFile.h>
#include <pvkernel/rush/PVChunkAlign.h>
#include <pvkernel/rush/PVChunkTransformUTF16.h>
#include <pvkernel/rush/PVRawSource.h>
#include <pvkernel/filter/PVChunkFilter.h>
#include <cstdlib>
#include <iostream>
#include "helpers.h"
#include <QCoreApplication>

using std::cout;
using std::cerr;
using std::endl;

using namespace PVRush;
using namespace PVCore;

int main(int argc, char** argv)
{
	if (argc <= 2) {
		cerr << "Usage: " << argv[0] << " file chunk_size" << endl;
		return 1;
	}

	PVCore::PVIntrinsics::init_cpuid();
	PVInput_p ifile(new PVInputFile(argv[1]));
	PVChunkAlign calign;
	PVChunkTransformUTF16 transform;
	PVFilter::PVChunkFilter null;
	PVRawSource<> source(ifile, calign, atoi(argv[2]), transform, null);

	PVChunk* pc = source();

	while (pc) {
		dump_chunk_raw(*pc);
		pc->free();
		pc = source();
	}

	return 0;
}
