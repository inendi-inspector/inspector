/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef BCODE_TYPES_H
#define BCODE_TYPES_H

#include <cstdint>

namespace PVParallelView
{

#pragma pack(push)
#pragma pack(4)
struct PVBCode {
	union {
		uint32_t int_v;
		struct {
			uint32_t l : 10;
			uint32_t r : 10;
			uint32_t __free : 12;
		} s;
	};
	void
	to_pts(uint16_t w, uint16_t h, uint16_t& lx, uint16_t& ly, uint16_t& rx, uint16_t& ry) const;
	void to_pts_new(
	    uint16_t w, uint16_t h, uint16_t& lx, uint16_t& ly, uint16_t& rx, uint16_t& ry) const;
};

typedef PVBCode DECLARE_ALIGN(16) * PVBCode_ap;
#pragma pack(pop)
}

#ifdef NBDEUG
#define assert_bcode_valid(b)
#else
#define assert_bcode_valid(b)                                                                      \
	do {                                                                                           \
		assert((b).s.__free == 0);                                                                 \
	} while (0);
#endif

#endif
