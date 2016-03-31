/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVOUTPUT_FILE_H
#define PVOUTPUT_FILE_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVChunk.h>
#include <pvkernel/filter/PVFilterFunction.h>

namespace PVRush {

class PVControllerJob;

class PVOutput : public PVFilter::PVFilterFunctionBase<void,PVCore::PVChunk*> {
	friend class PVControllerJob;
public:
	// This is the output of a TBB pipeline
	// It takes a PVCore::PVChunk* as a parameter, and do whatever he wants with it
	// It *must* call PVChunk->free() in the end !!
	virtual void operator()(PVCore::PVChunk* out) = 0;

public:
	virtual PVRow get_rows_count() = 0;

	/*! \brief Set a pointer to a stop condition.
	 *  \param[in] cond Pointer to a bool variable to represent the stop condition.
	 *
	 * operator() will set it to TRUE if anough events have been saved.
	 */
	void set_stop_condition(bool* cond) { _stop_cond = cond; }

protected:
	bool *_stop_cond;

protected:
	// This function is called by PVControllerJob
	// when its job has finished.
	virtual void job_has_finished() {}

	CLASS_FILTER_NONREG(PVOutput)
};

}

#endif
