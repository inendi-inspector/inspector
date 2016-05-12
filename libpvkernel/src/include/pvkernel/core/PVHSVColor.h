/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_HSVCOLOR_H
#define PVCORE_HSVCOLOR_H

#include <pvbase/types.h>
#include <stdint.h>
#include <pvkernel/cuda/constexpr.h>

#include <QColor>
#include <QImage>

//#define HSV_COLOR_NBITS_ZONE 6
//#define HSV_COLOR_MASK_ZONE 63

#define HSV_COLOR_NBITS_ZONE 5
#define HSV_COLOR_MASK_ZONE 31
#define HSV_COLOR_COUNT 192 // = (2**5)*6, without black & white

// Special colors
#define HSV_COLOR_WHITE 255
#define HSV_COLOR_BLACK 254
#define HSV_COLOR_TRANSPARENT 253

// Some colors that can be useful
#define HSV_COLOR_BLUE 10
#define HSV_COLOR_GREEN 59
#define HSV_COLOR_RED 126

namespace PVCore
{

class PVHSVColor
{
	typedef uint8_t T;

  public:
	typedef T h_type;
	static CUDA_CONSTEXPR uint8_t color_max = (1 << HSV_COLOR_NBITS_ZONE) * 6;

  public:
	// Unitialized, and this is wanted !
	PVHSVColor() {}
	PVHSVColor(T h_) { _h = h_; }

  public:
	inline T& h() { return _h; };
	inline T h() const { return _h; };
	static PVHSVColor* init_colors(PVRow nb_colors);
	static void
	to_rgba(const PVHSVColor* hsv_image, QImage& rbg_image, QRect const& img_rect = QRect());
	bool is_valid() const;

	bool operator==(PVHSVColor const& c) const { return c._h == _h; }
	bool operator!=(PVHSVColor const& c) const { return not(c == *this); }

  public:
	void to_rgb(T& r, T& g, T& b) const;
	void to_rgb(T* rgb) const;

	void to_rgba(T& r, T& g, T& b, T& a) const;
	void to_rgba(T* rgb) const;

	void toQColor(QColor& qc) const;
	QColor toQColor() const;

	void toQColorA(QColor& qc) const;
	QColor toQColorA() const;

  private:
	T _h;
};
}

#endif
