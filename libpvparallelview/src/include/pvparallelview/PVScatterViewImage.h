/**
 * @file
 *
 * @copyright (C) Picviz Labs 2013-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef __PVSCATTERVIEWIMAGE_H__
#define __PVSCATTERVIEWIMAGE_H__

#include <cstddef>
#include <cstdint>

#include <boost/utility.hpp>

#include <QRect>
#include <QImage>

class QPainter;

namespace PVCore
{
class PVHSVColor;
}

namespace PVParallelView
{

class PVZoomedZoneTree;

class PVScatterViewImage : boost::noncopyable
{
  public:
	constexpr static uint32_t image_width = 2048;
	constexpr static uint32_t image_height = image_width;

  public:
	PVScatterViewImage();
	PVScatterViewImage(PVScatterViewImage&& o) { swap(o); }

	~PVScatterViewImage();

  public:
	void clear(const QRect& rect = QRect());

	void convert_image_from_hsv_to_rgb(QRect const& img_rect = QRect());

	PVCore::PVHSVColor* get_hsv_image() { return _hsv_image; }
	QImage& get_rgb_image() { return _rgb_image; };

	const PVCore::PVHSVColor* get_hsv_image() const { return _hsv_image; }
	const QImage& get_rgb_image() const { return _rgb_image; };

  public:
	PVScatterViewImage& operator=(PVScatterViewImage&& o)
	{
		swap(o);
		return *this;
	}

	void swap(PVScatterViewImage& o)
	{
		if (&o != this) {
			std::swap(_hsv_image, o._hsv_image);
			_rgb_image.swap(o._rgb_image);
		}
	}

	void copy(PVScatterViewImage const& o);
	PVScatterViewImage& operator=(PVScatterViewImage const& o)
	{
		copy(o);
		return *this;
	}

  private:
	PVCore::PVHSVColor* _hsv_image;
	QImage _rgb_image;
};
}

#endif // __PVSCATTERVIEWIMAGE_H__
