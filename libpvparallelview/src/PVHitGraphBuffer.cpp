/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvparallelview/PVHitGraphBuffer.h>

#include <cassert>
#include <cstdlib>
#include <string.h>
#include <iostream>

//
// PVHitGraphBuffer
//

PVParallelView::PVHitGraphBuffer::PVHitGraphBuffer(uint32_t nbits, uint32_t nblocks)
    : _nbits(nbits), _nblocks(nblocks), _size_block(1 << nbits)
{
	posix_memalign((void**)&_buf, 16, size_bytes());
}

PVParallelView::PVHitGraphBuffer::~PVHitGraphBuffer()
{
	if (_buf) {
		free(_buf);
	}
}

void PVParallelView::PVHitGraphBuffer::set_zero()
{
	memset(_buf, 0, size_bytes());
}

void PVParallelView::PVHitGraphBuffer::shift_left(const uint32_t n)
{
	assert(n <= nblocks());
	const uint32_t nb_moved_blocks = nblocks() - n;

	memmove(buffer(), buffer_block(n), nb_moved_blocks * size_block() * sizeof(uint32_t));
	memset(buffer_block(nb_moved_blocks), 0, n * size_block() * sizeof(uint32_t));
}

void PVParallelView::PVHitGraphBuffer::shift_zoomed_left(const uint32_t n, const float alpha)
{
	assert(n <= nblocks());
	const uint32_t nb_moved_blocks = nblocks() - n;

	memmove(buffer(), zoomed_buffer_block(n, alpha),
	        nb_moved_blocks * size_zoomed_block(alpha) * sizeof(uint32_t));
	memset(zoomed_buffer_block(nb_moved_blocks, alpha), 0,
	       n * size_zoomed_block(alpha) * sizeof(uint32_t));
}

void PVParallelView::PVHitGraphBuffer::shift_right(const uint32_t n)
{
	assert(n <= nblocks());
	const uint32_t nb_moved_blocks = nblocks() - n;

	memmove(buffer_block(n), buffer(), nb_moved_blocks * size_block() * sizeof(uint32_t));
	memset(buffer(), 0, n * size_block() * sizeof(uint32_t));
}

void PVParallelView::PVHitGraphBuffer::shift_zoomed_right(const uint32_t n, const float alpha)
{
	assert(n <= nblocks());
	const uint32_t nb_moved_blocks = nblocks() - n;

	memmove(zoomed_buffer_block(n, alpha), buffer(),
	        nb_moved_blocks * size_zoomed_block(alpha) * sizeof(uint32_t));
	memset(buffer(), 0, n * size_zoomed_block(alpha) * sizeof(uint32_t));
}

bool PVParallelView::PVHitGraphBuffer::copy_from(PVHitGraphBuffer const& o)
{
	if (o.size_int() != size_int()) {
		return false;
	}

	memcpy(_buf, o._buf, size_bytes());

	return true;
}

uint32_t PVParallelView::PVHitGraphBuffer::get_max_count() const
{
	// GCC should vectorize this !
	const size_t nints = size_block() * nblocks();
	uint32_t ret = 0;
	for (size_t i = 0; i < nints; i++) {
		const uint32_t v = _buf[i];
		if (v > ret) {
			ret = v;
		}
	}
	return ret;
}

uint32_t PVParallelView::PVHitGraphBuffer::get_zoomed_max_count(const float alpha) const
{
	// GCC should vectorize this !
	const size_t nints = size_zoomed_block(alpha) * nblocks();
	uint32_t ret = 0;
	for (size_t i = 0; i < nints; i++) {
		const uint32_t v = _buf[i];
		if (v > ret) {
			ret = v;
		}
	}
	return ret;
}
