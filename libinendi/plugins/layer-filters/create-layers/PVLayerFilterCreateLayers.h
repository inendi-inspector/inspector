/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVLAYERFILTERCreateLayers_H
#define INENDI_PVLAYERFILTERCreateLayers_H

#include <QList>
#include <QString>
#include <QPair>

#include <inendi/PVLayer.h>
#include <inendi/PVLayerFilter.h>

namespace Inendi
{

/**
 * \class PVLayerFilterCreateLayers
 */
class PVLayerFilterCreateLayers : public PVLayerFilter
{
  public:
	PVLayerFilterCreateLayers(
	    QString section_name,
	    QMap<QString, QStringList> layers_regex,
	    PVCore::PVArgumentList const& l = PVLayerFilterCreateLayers::default_args());

  public:
	void operator()(PVLayer const& in, PVLayer& out) override;
	PVCore::PVArgumentList get_default_args_for_view(PVView const& view) override;
	QString menu_name() const override { return _menu_name; }

  private:
	QString _section_name;
	QString _menu_name;
	QMap<QString, QStringList> _layers_regex;

	CLASS_FILTER(Inendi::PVLayerFilterCreateLayers)
};
}

#endif