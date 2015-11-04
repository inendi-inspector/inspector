/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/rush/PVChunkAlign.h>
#include <pvkernel/core/PVChunk.h>
#include <pvkernel/core/PVField.h>

bool PVRush::PVChunkAlign::operator()(PVCore::PVChunk &cur_chunk, PVCore::PVChunk& /*next_chunk*/)
{
	// By default, we only have one big element
	PVCore::PVElement* elt = cur_chunk.add_element(cur_chunk.begin(), cur_chunk.end());
	// That element can grow 'till the size of the chunk
	elt->set_physical_end(cur_chunk.physical_end());
	// And so can its (for now) unique field
	//elt->fields().front().set_physical_end(cur_chunk.physical_end());

	return true;
}

