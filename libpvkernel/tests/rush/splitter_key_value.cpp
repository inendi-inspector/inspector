/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2016
 */

#include <chrono>

#include <pvkernel/core/PVClassLibrary.h>
#include <pvkernel/filter/PVChunkFilterByElt.h>
#include <pvkernel/filter/PVElementFilterByFields.h>
#include <pvkernel/rush/PVUtils.h>
#include <pvkernel/core/inendi_assert.h>

#include "helpers.h"
#include "common.h"

#ifdef INSPECTOR_BENCH
constexpr static size_t nb_dup = 1000;
#else
constexpr static size_t nb_dup = 1;
#endif

static constexpr const char* log_file =
    TEST_FOLDER "/pvkernel/rush/splitters/key_value/key_value.log";
static constexpr const char* ref_file =
    TEST_FOLDER "/pvkernel/rush/splitters/key_value/key_value.log.out";

int main()
{
	pvtest::TestSplitter ts(log_file, nb_dup);

	// Prepare splitter plugin
	PVFilter::PVFieldsSplitter::p_type sp_lib_p =
	    LIB_CLASS(PVFilter::PVFieldsSplitter)::get().get_class_by_name("key_value");

	PVCore::PVArgumentList args;
	args["sep"] = "  ";
	args["quote"] = '"';
	args["affectation"] = "=";
	args["keys"] = QStringList() << "time"
	                             << "fw"
	                             << "tz"
	                             << "startime"
	                             << "pri"
	                             << "confid"
	                             << "slotlevel"
	                             << "ruleid"
	                             << "srcif"
	                             << "srcifname"
	                             << "ipproto"
	                             << "dstif"
	                             << "dstifname"
	                             << "proto"
	                             << "src"
	                             << "srcport"
	                             << "dst"
	                             << "dstport"
	                             << "dstportname"
	                             << "dstname"
	                             << "modsrc"
	                             << "modsrcport"
	                             << "origdst"
	                             << "origdstport"
	                             << "sent"
	                             << "rcvd"
	                             << "duration"
	                             << "op"
	                             << "result"
	                             << "arg"
	                             << "logtype";

	sp_lib_p->set_args(args);

	PVFilter::PVElementFilterByFields* elt_f = new PVFilter::PVElementFilterByFields(sp_lib_p->f());
	PVFilter::PVChunkFilterByElt* chk_flt = new PVFilter::PVChunkFilterByElt(elt_f->f());
	PVFilter::PVChunkFilter_f flt_f = chk_flt->f();

	auto res = ts.run_normalization(flt_f);
	std::string output_file = std::get<2>(res);
	size_t nelts_org = std::get<0>(res);
	size_t nelts_valid = std::get<1>(res);

	PV_VALID(nelts_valid, nelts_org);

#ifndef INSPECTOR_BENCH
	// Check output is the same as the reference
	std::cout << std::endl << output_file << " - " << ref_file << std::endl;
	PV_ASSERT_VALID(PVRush::PVUtils::files_have_same_content(output_file, ref_file));
#endif
	std::remove(output_file.c_str());
	return 0;
}