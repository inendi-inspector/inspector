/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVClassLibrary.h>
#include <pvkernel/filter/PVChunkFilter.h>
#include <pvkernel/filter/PVChunkFilterByElt.h>
#include <pvkernel/filter/PVElementFilterByFields.h>
#include <pvkernel/filter/PVPluginsLoad.h>
#include <pvkernel/rush/PVInputFile.h>
#include <pvkernel/rush/PVUnicodeSource.h>
#include <cstdlib>
#include <iostream>
#include "helpers.h"
#include "test-env.h"

using std::cout;
using std::cerr;
using std::endl;

using namespace PVRush;
using namespace PVCore;

int main(int argc, char** argv)
{
	if (argc < 4) {
		cerr << "Usage: " << argv[0] << " file chunk_size regexp reverse" << endl;
		return 1;
	}

	bool reverse = (argc < 5) ? false : (argv[4][0] == '1');

	init_env();

	PVFilter::PVPluginsLoad::load_all_plugins();
	PVFilter::PVFieldsFilter<PVFilter::one_to_one>::p_type sp_lib_p =
	    LIB_CLASS(PVFilter::PVFieldsFilter<PVFilter::one_to_one>)::get().get_class_by_name(
	        "regexp");

	PVCore::PVArgumentList args;
	args["regexp"] = PVCore::PVArgument(QString(argv[3]));
	args["reverse"] = reverse;
	sp_lib_p->set_args(args);

	PVFilter::PVElementFilterByFields* elt_f = new PVFilter::PVElementFilterByFields(sp_lib_p->f());
	PVFilter::PVChunkFilterByElt* chk_flt = new PVFilter::PVChunkFilterByElt(elt_f->f());

	PVInput_p ifile(new PVInputFile(argv[1]));
	PVFilter::PVChunkFilter null;
	PVUnicodeSource<> source(ifile, atoi(argv[2]), null);

	return !process_filter(source, chk_flt->f());
}
