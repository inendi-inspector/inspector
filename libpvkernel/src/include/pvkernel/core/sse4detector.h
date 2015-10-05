/**
 * \file sse4detector.h
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#ifndef PVCORE_SSE4DETECTOR_H
#define PVCORE_SSE4DETECTOR_H

struct CPUIDinfo
{ 
	unsigned int EAX,EBX,ECX,EDX; 
};

int isSSE41Supported(void);
int isSSE41andSSE42supported(void);
int isGenuineIntel(void);
int isCPUIDsupported(void);


#endif
