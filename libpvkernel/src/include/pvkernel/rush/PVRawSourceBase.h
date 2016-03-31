/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVRAWSOURCEBASE_FILE_H
#define PVRAWSOURCEBASE_FILE_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVChunk.h>
#include <pvkernel/filter/PVChunkFilter.h>
#include <pvkernel/filter/PVFilterFunction.h>
#include <pvkernel/rush/PVInput_types.h>
#include <QString>

#include <pvkernel/rush/PVRawSourceBase_types.h>

namespace PVRush {

class PVRawSourceBase : public PVFilter::PVFilterFunctionBase<PVCore::PVChunk*,void> {
public:
	typedef PVRawSourceBase_p p_type;
public:
	PVRawSourceBase();
	virtual ~PVRawSourceBase() {};
	PVRawSourceBase(const PVRawSourceBase& src) = delete;

public:
	virtual void release_input() {}

public:
	chunk_index last_elt_index() { return _last_elt_index; }
	void set_number_cols_to_reserve(PVCol col)
	{
		if (col == 0) {
			col = 1;
		}
		_ncols_to_reserve = col;
	}
	PVCol get_number_cols_to_reserve() const { return _ncols_to_reserve; }
	virtual QString human_name() = 0;
	virtual void seek_begin() = 0;
	virtual bool seek(input_offset off) = 0;
	virtual void prepare_for_nelts(chunk_index nelts) = 0;
	virtual PVCore::PVChunk* operator()() = 0;
	virtual input_offset get_input_offset_from_index(chunk_index idx, chunk_index& known_idx) = 0;

protected:
	mutable chunk_index _last_elt_index; // Local file index of the last element of that source. Can correspond to a number of lines
	PVCol _ncols_to_reserve;
};

}

#endif
