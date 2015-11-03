/**
 * @file
 *
 * @copyright (C) Picviz Labs 2014-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

// Register the plugin in PVFilterLibrary
//

#include <pvkernel/core/PVClassLibrary.h>
#include "PVFieldSplitterDnsFqdn.h"
#include "PVFieldSplitterDnsFqdnParamWidget.h"

#include <pvkernel/rush/PVAxisTagsDec.h>

// This method will be called by libpicviz
LibCPPExport void register_class()
{
	REGISTER_CLASS("dns_fqdn", PVFilter::PVFieldSplitterDnsFqdn);
	REGISTER_CLASS_AS("splitter_dns_fqdn", PVFilter::PVFieldSplitterDnsFqdn, PVFilter::PVFieldsFilterReg);

	REGISTER_CLASS("dns_fqdn", PVFilter::PVFieldSplitterDnsFqdnParamWidget);
}
