/**
 * \file environ.h
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#ifndef PVCORE_ENVIRON_H
#define PVCORE_ENVIRON_H

#include <pvkernel/core/general.h>

namespace PVCore {

	int mkenv(const char *name, const char *value);
	int rmenv(const char *name);

}

#endif	/* PVCORE_ENVIRON_H */
