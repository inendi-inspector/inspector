/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

// Register the plugin in PVFilterLibrary
//

#include <pvkernel/core/PVClassLibrary.h>
#include "PVMappingFilterDateDay.h"

// This method will be called by libinendi
LibCPPExport void register_class()
{
	// Register under the name "type_format"
	REGISTER_CLASS("date_day", Inendi::PVMappingFilterDateDay);
}