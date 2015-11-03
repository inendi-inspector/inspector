/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVFILTER_PVFIELDSPLITTERURL_H
#define PVFILTER_PVFIELDSPLITTERURL_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVField.h>
#include <pvkernel/filter/PVFieldsFilter.h>
#include <QChar>

#include <furl/furl.h>

#include <tbb/combinable.h>

namespace PVFilter {

class PVFieldSplitterURL : public PVFieldsFilter<one_to_many> {
public:
	PVFieldSplitterURL();
protected:
	PVCore::list_fields::size_type one_to_many(PVCore::list_fields &l, PVCore::list_fields::iterator it_ins, PVCore::PVField &field);
protected:
	void set_children_axes_tag(filter_child_axes_tag_t const& axes);

private:
	int _col_proto;
	int _col_subdomain;
	int _col_host;
	int _col_domain;
	int _col_tld;
	int _col_port;
	int _col_url;
	int _col_variable;
	int _col_fragment;
	int _col_credentials;
	PVCol _ncols;

	tbb::combinable<furl_handler_t> _furl_handler;

	CLASS_FILTER(PVFilter::PVFieldSplitterURL)
};

}

#endif
