/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/inendi_assert.h>
#include <pvkernel/core/inendi_intrin.h>
#include <pvkernel/filter/PVPluginsLoad.h>
#include <pvkernel/rush/PVInputType.h>
#include <pvkernel/rush/PVPluginsLoad.h>
#include <pvkernel/rush/PVExtractor.h>
#include <pvkernel/rush/PVControllerJob.h>
#include <pvkernel/rush/PVFormat.h>
#include <pvkernel/rush/PVTests.h>
#include <pvkernel/rush/PVFileDescription.h>
#include <inendi/PVRoot.h>
#include <inendi/PVScene.h>
#include <inendi/PVSource.h>
#include <inendi/PVMapping.h>
#include <inendi/PVMapped.h>
#include <cstdlib>
#include <iostream>
#include "test-env.h"

int main(int argc, char** argv)
{
	if (argc <= 2) {
		std::cerr << "Usage: " << argv[0] << " file format [raw_dump]" << std::endl;
		return 1;
	}

	init_env();
	PVCore::PVIntrinsics::init_cpuid();
	PVFilter::PVPluginsLoad::load_all_plugins();
	PVRush::PVPluginsLoad::load_all_plugins();

	// Input file
	QString path_file(argv[1]);
	PVRush::PVInputDescription_p file(new PVRush::PVFileDescription(path_file));

	// Load the given format file
	QString path_format(argv[2]);
	PVRush::PVFormat format("format", path_format);
	if (!format.populate()) {
		std::cerr << "Can't read format file " << qPrintable(path_format) << std::endl;
		return false;
	}

	// Get the source creator
	QString file_path(argv[1]);
	PVRush::PVSourceCreator_p sc_file;
	if (!PVRush::PVTests::get_file_sc(file, format, sc_file)) {
		return 1;
	}

	bool raw_dump = true;
	if (argc >= 4) {
		raw_dump = argv[3][0] == '1';
	}

	// Create the PVSource object
	Inendi::PVRoot_p root(new Inendi::PVRoot());
	Inendi::PVScene_p scene(new Inendi::PVScene("scene"));
	scene->set_parent(root);
	Inendi::PVSource_sp src(new Inendi::PVSource(PVRush::PVInputType::list_inputs() << file, sc_file, format));
	src->set_parent(scene);
	Inendi::PVMapped_p mapped(new Inendi::PVMapped());
	mapped->set_parent(src);
	PVRush::PVControllerJob_p job;

	if (raw_dump) {
		job = src->extract();
	} else {
		job = src->extract_from_agg_nlines(0, 200000000);
	}

	src->wait_extract_end(job);
	PVLOG_INFO("Extraction job bytes: %lu, time: %0.4fs, mean bw: %0.4f MB/s\n", job->total_bytes_processed(), job->duration().seconds(), job->mean_bw());


	if (raw_dump) {
		mapped->to_csv();
	} else {
		PVLOG_INFO("Extracted %u lines...\n", src->get_row_count());
	}

	//
	/*QStringList const& inv(job->get_invalid_evts());
	foreach (QString const& sinv, inv) {
		PVLOG_INFO("invalid: %s\n", qPrintable(sinv));
	}*/

	// Map the nraw
	//mapped->process_from_parent_source();
	// Dump the mapped table to stdout in a CSV format
	//mapped->to_csv();

	// Save current mapped table
#if 0
	Inendi::PVMapped_p mapped(src);
	Inendi::PVMapped::mapped_table_t save(mapped->get_table());

	mapped->invalidate_all();
	mapped->process_from_parent_source();
	//mapped->to_csv();
	
	// Compare table
	PV_ASSERT_VALID(save.size() == mapped->get_table().size());
	for (size_t i = 0; i < save.size(); i++) {
		Inendi::PVMapped::mapped_row_t const& rsave = save[i];
		Inendi::PVMapped::mapped_row_t const& rcmp  = mapped->get_table()[i];

		PV_ASSERT_VALID(rsave.size() == rcmp.size());
		//PV_ASSERT_VALID(memcmp(&rsave.at(0), &rcmp.at(0), rsave.size()*sizeof(Inendi::PVMapped::decimal_storage_type)) == 0);
		write(4, &rsave.at(0), rsave.size()*sizeof(Inendi::PVMapped::decimal_storage_type));
		write(5, &rcmp.at(0),  rcmp.size()*sizeof(Inendi::PVMapped::decimal_storage_type));
	}
#endif

	return 0;
}
