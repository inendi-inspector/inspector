/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVPARALLELVIEW_PVBCIBACKENDIMAGE_H
#define PVPARALLELVIEW_PVBCIBACKENDIMAGE_H

#include <pvkernel/core/general.h>

#include <pvparallelview/common.h>
#include <pvparallelview/PVBCIBackendImage_types.h>

#include <boost/utility.hpp>

#include <QImage>

namespace PVParallelView {

class PVBCIBackendImage: boost::noncopyable
{
public:
	typedef std::shared_ptr<PVBCIBackendImage> p_type;

protected:
	PVBCIBackendImage(uint32_t width, uint8_t height_bits):
		_width(width),
		_height_bits(height_bits)
	{ }

public:
	virtual ~PVBCIBackendImage() { }

public:
	inline QImage qimage() const { return qimage(height()); }
	virtual QImage qimage(size_t height_crop) const = 0;
	virtual bool set_width(uint32_t width) { _width = width; return true; }

	inline uint32_t width() const { return _width; }
	inline uint32_t height() const { return 1U<<_height_bits; }
	inline size_t size_pixel() const { return _width*height(); }
	inline uint8_t height_bits() const { return _height_bits; }

public:
	virtual void resize_width(PVBCIBackendImage& dst, const uint32_t width) const = 0;

private:
	uint32_t _width;
	uint8_t _height_bits;
};


}

#endif
