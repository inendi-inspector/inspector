/**
 * \file full_parallel_view_selections_sync.cpp
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#include <QtGui>
#include <QGLWidget>
#include <iostream>

#include <pvparallelview/common.h>
#include <pvkernel/core/picviz_bench.h>
#include <picviz/PVPlotted.h>
#include <picviz/PVSelection.h>
#include <pvparallelview/PVBCICode.h>
#include <pvparallelview/PVBCIBackendImage.h>
#include <pvparallelview/PVBCIDrawingBackendCUDA.h>
#include <pvparallelview/PVZonesDrawing.h>
#include <pvparallelview/PVZonesManager.h>
#include <pvparallelview/PVLinesView.h>
#include <pvparallelview/PVLibView.h>

#include <pvparallelview/PVFullParallelScene.h>
#include <pvparallelview/PVFullParallelView.h>

#include <pvkernel/core/PVSharedPointer.h>

#include <QApplication>

#include <picviz/FakePVView.h>

void usage(const char* path)
{
	std::cerr << "Usage: " << path << " [plotted_file] [nrows] [ncols]" << std::endl;
}

void init_rand_plotted(Picviz::PVPlotted::plotted_table_t& p, PVRow nrows, PVCol ncols)
{
	srand(time(NULL));
	p.clear();
	p.reserve(nrows*ncols);
	for (PVRow i = 0; i < nrows*ncols; i++) {
		p.push_back((float)((double)(rand())/(double)RAND_MAX));
	}
}

#define RENDERING_BITS 10

int main(int argc, char** argv)
{
	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	QApplication app(argc, argv);

	PVCol ncols, nrows;
	Picviz::PVPlotted::plotted_table_t plotted;
	QString fplotted(argv[1]);
	if (fplotted == "0") {
		if (argc < 4) {
			usage(argv[0]);
			return 1;
		}
		srand(time(NULL));
		nrows = atol(argv[2]);
		ncols = atol(argv[3]);

		init_rand_plotted(plotted, nrows, ncols);
	}
	else
	{
		if (!Picviz::PVPlotted::load_buffer_from_file(plotted, ncols, true, QString(argv[1]))) {
			std::cerr << "Unable to load plotted !" << std::endl;
			return 1;
		}
		nrows = plotted.size()/ncols;
	}

	Picviz::PVPlotted::uint_plotted_table_t norm_plotted;
	BENCH_START(norm);
	Picviz::PVPlotted::norm_int_plotted(plotted, norm_plotted, ncols);
	BENCH_END_TRANSFORM(norm, "integer normalisation", sizeof(float), nrows*ncols);

	PVParallelView::PVBCIDrawingBackendCUDA<NBITS_INDEX> backend_cuda;
	Picviz::FakePVView::shared_pointer fake_pvview_sp(new Picviz::FakePVView());

	PVParallelView::PVLibView lib_view(fake_pvview_sp);

	/// TODO: Find a better way to pass the plotted to the zones manager
	lib_view.get_zones_manager().set_uint_plotted(norm_plotted, nrows, ncols);
	lib_view.get_zones_manager().update_all();
	///

	lib_view.create_view(backend_cuda);
	lib_view.create_view(backend_cuda);

	app.exec();


	return 0;
}