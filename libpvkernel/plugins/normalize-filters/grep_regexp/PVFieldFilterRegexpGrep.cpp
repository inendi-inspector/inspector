/**
 * @file
 *
 * @copyright (C) Picviz Labs 2011-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include "PVFieldFilterRegexpGrep.h"


/******************************************************************************
 *
 * PVFilter::PVFieldFilterRegexpGrepg::PVFieldFilterGrep
 *
 *****************************************************************************/
PVFilter::PVFieldFilterRegexpGrep::PVFieldFilterRegexpGrep(PVCore::PVArgumentList const& args) :
	PVFilter::PVFieldsFilter<PVFilter::one_to_one>()
{
	INIT_FILTER(PVFilter::PVFieldFilterRegexpGrep, args);
}

/******************************************************************************
 *
 * DEFAULT_ARGS_FILTER(PVFilter::PVFieldFilterRegexpGrep)
 *
 *****************************************************************************/
DEFAULT_ARGS_FILTER(PVFilter::PVFieldFilterRegexpGrep)
{
	PVCore::PVArgumentList args;
	args["regexp"] = QString("");
	args["reverse"] = PVCore::PVArgument(false);
	return args;
}

/******************************************************************************
 *
 * PVFilter::PVFieldFilterRegexpGrep::set_args
 *
 *****************************************************************************/
void PVFilter::PVFieldFilterRegexpGrep::set_args(PVCore::PVArgumentList const& args)
{
	FilterT::set_args(args);
	_rx = QRegExp(args.at("regexp").toString());
	_inverse = args.at("reverse").toBool();
}

/******************************************************************************
 *
 * PVFilter::PVFieldFilterRegexpGrep::one_to_one
 *
 *****************************************************************************/
PVCore::PVField& PVFilter::PVFieldFilterRegexpGrep::one_to_one(PVCore::PVField& obj)
{
	QRegExp rx(_rx); // Local object (local to a thread !)
	QString str_tmp;
	bool found = (rx.indexIn(obj.get_qstr(str_tmp)) != -1);
	if (!(found ^ _inverse))
	{
		obj.set_invalid();
	}
	return obj;
}

IMPL_FILTER(PVFilter::PVFieldFilterRegexpGrep)