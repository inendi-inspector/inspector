/*   Copyright 2009 Intel Corporation
 * sse41andsse42detection.cpp
 *   This file uses code first published by Intel as part of the processor enumeration
 * article available on the internet at:
 * http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-            *
 *enumeration/
 *  Some of the original code from cpu_topo.c
 * has been removed, while other code has been added to illustrate the CPUID usage
 *   to determine if the processor supports the SSE 4.1 and SSE 4.2 instruction sets.
 *  The reference code provided in this file is for demonstration purpose only. It assumes
 *    the hardware topology configuration within a coherent domain does not change during
 *   the life of an OS session. If an OS support advanced features that can change
 *    hardware topology configurations, more sophisticated adaptation may be necessary
 *  to account for the hardware configuration change that might have added and reduced
 *   the number of logical processors being managed by the OS.
 *
 *   Users of this code should be aware that the provided code
 * relies on CPUID instruction providing raw data reflecting the native hardware
 *    configuration. When an application runs inside a virtual machine hosted by a
 * Virtual Machine Monitor (VMM), any CPUID instructions issued by an app (or a guest OS)
 *   are trapped by the VMM and it is the VMM's responsibility and decision to emulate
 *   CPUID return data to the virtual machines. When deploying topology enumeration code based
 *  on CPUID inside a VM environment, the user must consult with the VMM vendor on how an VMM
 * will emulate CPUID instruction relating to topology enumeration.
 *
 *    Original code written by Patrick Fay, Ronen Zohar and Shihjong Kuo .
 *  Modified by Garrett Drysdale for current application note.
 */

// AG: add a function for SSE4.1 detection only

#include <pvkernel/core/sse4detector.h>
#include <cpuid.h>
#include <cstdint>

#define SSE4_1_FLAG 0x080000
#define SSE4_2_FLAG 0x100000

static void get_cpuid_infos(CPUIDinfo* infos, int InfoType)
{
	uint32_t* eax = &infos->EAX;
	uint32_t* ebx = &infos->EBX;
	uint32_t* ecx = &infos->ECX;
	uint32_t* edx = &infos->EDX;
	__get_cpuid(InfoType, eax, ebx, ecx, edx);
}

static bool _isSSE4Supported(const unsigned int CHECKBITS)
{
	// returns true if is a Nehalem or later processor, false if prior to Nehalem

	CPUIDinfo Info{0, 0, 0, 0};
	// The code first determines if the processor is an Intel Processor.  If it is, then
	// feature flags bit 19 (SSE 4.1) and 20 (SSE 4.2) in ECX after CPUID call with EAX = 0x1
	// are checked.
	// If both bits are 1 (indicating both SSE 4.1 and SSE 4.2 exist) then
	// the function returns 1

	if (isGenuineIntel() >= 1) {
		// execute CPUID with eax (leaf) = 1 to get feature bits,
		// subleaf doesn't matter so set it to zero
		get_cpuid_infos(&Info, 0x1);
		if ((Info.ECX & CHECKBITS) == CHECKBITS) {
			return true;
		}
	}
	return false;
}

bool isSSE41Supported()
{
	const unsigned int CHECKBITS = SSE4_1_FLAG;
	return _isSSE4Supported(CHECKBITS);
}

bool isSSE41andSSE42supported()
{
	const unsigned int CHECKBITS = SSE4_1_FLAG | SSE4_2_FLAG;
	return _isSSE4Supported(CHECKBITS);
}

bool isGenuineIntel()
{
	// returns largest function # supported by CPUID if it is a Geniune Intel processor AND it
	// supports
	// the CPUID instruction, 0 if not
	CPUIDinfo Info;
	char procString[] = "GenuineIntel";
	unsigned int* psint = (unsigned int*)procString;
	get_cpuid_infos(&Info, 0x0);

	// execute CPUID with eax = 0, subleaf doesn't matter so set it to zero
	if ((Info.EBX == *psint) && (Info.EDX == *(psint + 1)) && (Info.ECX == *(psint + 2))) {
		return Info.EAX;
	}
	return false;
}
