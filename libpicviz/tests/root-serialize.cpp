/**
 * \file source-serialize.cpp
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#include <pvkernel/filter/PVPluginsLoad.h>
#include <pvkernel/rush/PVInputType.h>
#include <pvkernel/rush/PVPluginsLoad.h>
#include <pvkernel/rush/PVExtractor.h>
#include <pvkernel/rush/PVControllerJob.h>
#include <pvkernel/rush/PVFormat.h>
#include <pvkernel/rush/PVTests.h>
#include <pvkernel/rush/PVFileDescription.h>
#include <picviz/PVRoot.h>
#include <picviz/PVScene.h>
#include <picviz/PVSource.h>
#include <picviz/PVMapping.h>
#include <picviz/PVMapped.h>
#include <picviz/PVPlotted.h>
#include <picviz/PVView.h>
#include <cstdlib>
#include <iostream>
#include <QCoreApplication>
#include "test-env.h"

#include <tulip/Graph.h>

int main(int argc, char** argv)
{
	if (argc <= 2) {
		std::cerr << "Usage: " << argv[0] << " file format" << std::endl;
		return 1;
	}

	init_env();
	QCoreApplication app(argc, argv);
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

	// Create the PVSource object
	Picviz::PVRoot_p root;
	Picviz::PVScene_p scene(root, "scene0");
	Picviz::PVSource_p src(scene, PVRush::PVInputType::list_inputs() << file, sc_file, format);
	scene->add_source(src);
	PVRush::PVControllerJob_p job = src->extract();
	job->wait_end();
	src->create_default_view();
	src->create_default_view();

	Picviz::PVView& v0 = *src->get_children<Picviz::PVView>().at(0);
	Picviz::PVView& v1 = *src->get_children<Picviz::PVView>().at(1);

	Picviz::PVScene_p scene2(root, "scene1");
	Picviz::PVSource_p src2(scene2, PVRush::PVInputType::list_inputs() << file, sc_file, format);
	src2->create_default_view();
	src2->create_default_view();

	Picviz::PVView& v2 = *src2->get_children<Picviz::PVView>().at(0);
	Picviz::PVView& v3 = *src2->get_children<Picviz::PVView>().at(1);

	// Set correlations
	Picviz::PVAD2GView& corr0 = *root->add_correlation("corr0");
	corr0.add_view(&v0);
	corr0.add_view(&v1);

	Picviz::PVCombiningFunctionView_p cfv0(new Picviz::PVCombiningFunctionView());
	corr0.set_edge_f(&v0, &v1, cfv0);

	Picviz::PVAD2GView& corr1 = *root->add_correlation("corr1");
	corr1.add_view(&v2);
	corr1.add_view(&v3);

	Picviz::PVCombiningFunctionView_p cfv1(new Picviz::PVCombiningFunctionView());
	corr1.set_edge_f(&v2, &v3, cfv1);

	Picviz::PVAD2GView& corr2 = *root->add_correlation("corr2");
	corr2.add_view(&v0);
	corr2.add_view(&v2);

	Picviz::PVCombiningFunctionView_p cfv2(new Picviz::PVCombiningFunctionView());
	corr2.set_edge_f(&v0, &v2, cfv2);

	root->select_correlation(0);

	// Serialize the root object
	PVCore::PVSerializeArchive_p ar(new PVCore::PVSerializeArchive("/srv/tmp-picviz/test", PVCore::PVSerializeArchive::write, 1));
	ar->get_root()->object("root", *root);
	ar->finish();

	src.reset();
	scene.reset();
	v0.remove_from_tree();
	v1.remove_from_tree();
	root.reset(new Picviz::PVRoot());

	// Get it back !
	ar.reset(new PVCore::PVSerializeArchive("/srv/tmp-picviz/test", PVCore::PVSerializeArchive::read, 1));
	ar->get_root()->object("root", *root);
	ar->finish();

	root->dump();

#if 0
	Picviz::PVScene::list_sources_t srcs = scene->get_sources(*sc_file->supported_type_lib());
	if (srcs.size() != 1) {
		std::cerr << "No source was recreated !" << std::endl;
		return 1;
	}
	src = srcs.at(0)->shared_from_this();
	
	job = src->extract();
	job->wait_end();

	std::cerr << "--------" << std::endl << "New output: " << std::endl << "----------" << std::endl << std::endl;
	// Dump the NRAW
	src->get_rushnraw().dump_csv();
#endif

	return 0;
}