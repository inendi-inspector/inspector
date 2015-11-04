/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVFILTER_PVFIELDSPLITTERIRONPORTMAIL_H
#define PVFILTER_PVFIELDSPLITTERIRONPORTMAIL_H

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVField.h>
#include <pvkernel/filter/PVFieldsFilter.h>

#include <tbb/combinable.h>

#include <QRegExp>

namespace PVFilter {

class PVFieldSplitterIronPortMail : public PVFieldsFilter<one_to_many>
{
public:
	enum col_type {
		COL_LOG_TYPE = 0,
		COL_LOG_TEXT_SHORT,

		COL_MSGID,

		COL_MSG_FROM,
		COL_MSG_TO,
		COL_MSG_SUBJECT,
		COL_MSG_SIZE,
		COL_MSG_DOMAIN_KEYS,
		COL_MSG_DKIM,

		COL_MID,
		COL_ICID,
		COL_RID,
		COL_DCID,

		COL_FIELDS_NUMBER
} ;

public:
	PVFieldSplitterIronPortMail();

protected:
	PVCore::list_fields::size_type one_to_many(PVCore::list_fields &l, PVCore::list_fields::iterator it_ins, PVCore::PVField &field);
protected:
	void set_children_axes_tag(filter_child_axes_tag_t const& axes);

private:
	typedef struct regexps_t {
		regexps_t();

		QRegExp text_short;

		QRegExp com_start;
		QRegExp com_finish;
		QRegExp com_msgid;

		QRegExp msg_from;
		QRegExp msg_to;
		QRegExp msg_subject;
		QRegExp msg_size;
		QRegExp msg_domain_keys;
		QRegExp msg_dkim;

		QRegExp dlvy_queued;
		QRegExp dlvy_start;
		QRegExp dlvy_done;
		QRegExp dlvy_reply;
	} regexps_t;

private:
	int _col_log_type;
	int _col_log_text_short;

	int _col_msgid;

	int _col_msg_from;
	int _col_msg_to;
	int _col_msg_subject;
	int _col_msg_size;
	int _col_msg_domain_keys;
	int _col_msg_dkim;

	int _col_mid;
	int _col_icid;
	int _col_rid;
	int _col_dcid;

	PVCol _ncols;

	tbb::combinable<regexps_t> _regexps;

	CLASS_FILTER(PVFilter::PVFieldSplitterIronPortMail)
};

}

#endif // PVFILTER_PVFIELDSPLITTERIRONPORTMAIL_H
