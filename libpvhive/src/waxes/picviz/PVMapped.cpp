#include <pvhive/PVHive.h>
#include <pvhive/waxes/picviz/PVMapped.h>
#include <pvhive/waxes/picviz/PVPlotted.h>

PVHIVE_CALL_OBJECT_BLOCK_BEGIN()

// Mapped updating waxes
//

IMPL_WAX(Picviz::PVMapped::process_from_parent_source, mapped, args)
{
	call_object_default<Picviz::PVMapped, FUNC(Picviz::PVMapped::process_from_parent_source)>(mapped, args);
	for (auto const& c: mapped->get_children<Picviz::PVPlotted>()) {
		refresh_observers(&c->get_plotting());
	}
}

PVHIVE_CALL_OBJECT_BLOCK_END()