/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVWIDGETS_PVCOLORDIALOG_H
#define PVWIDGETS_PVCOLORDIALOG_H

#include <pvkernel/core/PVHSVColor.h>
#include <pvkernel/ui_PVColorDialog.h>

namespace PVCore
{
class PVHSVColor;
}

namespace PVWidgets
{

namespace __impl
{
class PVLabelEventFilter;
}

class PVColorPicker;

class PVColorDialog : public QDialog, Ui::PVColorDialog
{
	Q_OBJECT

	friend class __impl::PVLabelEventFilter;

  public:
	PVColorDialog(QWidget* parent = nullptr);
	PVColorDialog(PVCore::PVHSVColor const& c, QWidget* parent = nullptr);

  public:
	void set_color(PVCore::PVHSVColor const& c);
	inline PVCore::PVHSVColor color() const { return picker()->color(); };

	inline void set_interval(uint8_t x0, uint8_t x1)
	{
		assert(x1 > x0);
		picker()->set_x0(x0);
		picker()->set_x1(x1);
	}

  Q_SIGNALS:
	void color_changed(int h);

  protected:
	void label_button_pressed(QLabel* label, QMouseEvent* event);
	void label_button_released(QLabel* label, QMouseEvent* event);

  private:
	void show_color(PVCore::PVHSVColor const& c);

	inline PVColorPicker* picker() { return _picker; }
	inline PVColorPicker const* picker() const { return _picker; }

	void set_predefined_color_from_label(QLabel* label);
	void unselect_all_preselected_colors();

  private Q_SLOTS:
	void picker_color_changed(int h);
	void set_predefined_color_from_action();
	void reset_predefined_color_from_action();

  private:
	__impl::PVLabelEventFilter* _label_event_filter;
};
}

#endif
