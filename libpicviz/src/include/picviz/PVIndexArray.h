/**
 * \file PVIndexArray.h
 *
 * Copyright (C) Picviz Labs 2009-2012
 */

#ifndef PICVIZ_PVINDEXARRAY_H
#define PICVIZ_PVINDEXARRAY_H

#include <QtCore>

#include <pvkernel/core/general.h>

#include <picviz/PVSelection.h>
#include <assert.h>

#define PICVIZ_INDEX_ARRAY_MAX_SIZE PICVIZ_LINES_MAX

namespace Picviz {

/**
 * \class PVIndexArray
 */
class PVIndexArray {
private:
	int array [PICVIZ_INDEX_ARRAY_MAX_SIZE];
	int index_count;
	int row_count;
	

public:
	/**
	 * Constructor
	 */
	PVIndexArray();

	/**
	 * Constructor
	 */
	PVIndexArray(PVRow initial_row_count);

	int get_index_count() const {return index_count;}
	PVRow get_row_count() const {return row_count;}
	int at(int index) const { assert(index < PICVIZ_INDEX_ARRAY_MAX_SIZE); return array[index]; }

	void set_from_selection(const PVSelection & selection_);
	void set_row_count(int row_count_) { row_count = row_count_; }
};
}

#endif /* PICVIZ_PVINDEXARRAY_H_ */





// These are still used

// int picviz_index_array_get_index_count(picviz_index_array_t *ia);
//
// void picviz_index_array_set_from_selection(picviz_index_array_t *ia, picviz_selection_t *selection);
// 
