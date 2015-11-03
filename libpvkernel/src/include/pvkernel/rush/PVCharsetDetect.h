/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCHARSETDETECT_FILE_H
#define PVCHARSETDETECT_FILE_H

#include <pvkernel/core/general.h>
#include <pvkernel/rush/uchardetect/nscore.h>
#include <pvkernel/rush/uchardetect/nsUniversalDetector.h>

#include <string>

namespace PVRush {

class PVCharsetDetect : public nsUniversalDetector {
public:
	PVCharsetDetect();

protected:
	virtual void Report(const char* charset);
	virtual void Reset();

public:
	std::string const& GetCharset() const;
	bool found() const;

protected:
	std::string _charset;
	bool _found;
};

}

#endif
