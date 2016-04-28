/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef BCICUDA_H
#define BCICUDA_H

// for GLuint
#include <GL/gl.h>

#include <pvparallelview/PVBCICode.h>

void show_codes_cuda(PVParallelView::PVBCICode<NBITS_INDEX>* codes,
                     uint32_t n,
                     uint32_t width,
                     uint32_t* img_dst);
void show_codes_cuda(PVParallelView::PVBCICode<NBITS_INDEX>* codes,
                     uint32_t n,
                     uint32_t width,
                     GLuint buffer_id);

#endif
