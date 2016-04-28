/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVINPUT_FILE_H
#define PVINPUT_FILE_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVChunk.h>
#include <pvkernel/filter/PVFilterFunction.h>

#include <QString>

#include <pvkernel/rush/PVInput_types.h>

namespace PVRush
{

class PVInput
{
  public:
	typedef PVInput_p p_type;

  public:
	PVInput();
	virtual ~PVInput();

  public:
	virtual void release(){};

  public:
	// This method must read at most n bytes and put the result in buffer and returns the number of
	// bytes actually read.
	// It returns 0 if no more data is available
	virtual size_t operator()(char* buffer, size_t n) = 0;
	// This method must return the current input offset of the object. For instance, for a file, it
	// would be
	// the current offset of the file opened.
	virtual input_offset current_input_offset() = 0;
	// Seek to the beggining of the input
	virtual void seek_begin() = 0;
	virtual QString human_name() = 0;
	virtual bool seek(input_offset off) = 0;
};

class PVInputException
{
  public:
	virtual std::string const& what() const = 0;
};
}

#define IMPL_INPUT(T)
#define CLASS_INPUT(T)

#endif
