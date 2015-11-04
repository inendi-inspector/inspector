/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/filter/PVChunkFilterByElt.h>
#include <pvkernel/core/PVChunk.h>


/******************************************************************************
 *
 * PVFilter::PVChunkFilterByElt::PVChunkFilterByElt
 *
 *****************************************************************************/
PVFilter::PVChunkFilterByElt::PVChunkFilterByElt(PVElementFilter_f elt_filter) :
	PVChunkFilter()
{
	_elt_filter = elt_filter;
	_n_elts_invalid = 0;
}

/******************************************************************************
 *
 * PVFilter::PVChunkFilterByElt::operator()
 *
 *****************************************************************************/
PVCore::PVChunk* PVFilter::PVChunkFilterByElt::operator()(PVCore::PVChunk* chunk)
{
	PVCore::list_elts& elts = chunk->elements();
	PVCore::list_elts::iterator it,ite;
	it = elts.begin();
	ite = elts.end();
	size_t nelts = elts.size();
	size_t nelts_valid = 0;
	while (it != ite)
	{
		PVCore::PVElement &elt = _elt_filter(*(*it));
		if (!elt.valid())
		{
			PVCore::PVElement::free(*it);
			PVCore::list_elts::iterator it_rem = it;
			it++;
			elts.erase(it_rem);
		}
		else {
			it++;
			nelts_valid++;
		}
	}
	chunk->set_elts_stat(nelts, nelts_valid);
	return chunk;
}
