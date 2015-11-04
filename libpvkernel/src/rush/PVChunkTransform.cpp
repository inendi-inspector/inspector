/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/rush/PVChunkTransform.h>


size_t PVRush::PVChunkTransform::next_read_size(size_t org_size) const
{
	return org_size;
}

size_t PVRush::PVChunkTransform::operator()(char* /*data*/, size_t len_read, size_t /*len_avail*/) const
{
	// By default, no filtering/conversion
	return len_read;
}
