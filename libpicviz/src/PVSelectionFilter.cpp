/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <picviz/PVSelectionFilter.h>
#include <assert.h>



/******************************************************************************
 *
 * Picviz::PVSelectionFilter::PVSelectionFilter
 *
 *****************************************************************************/
Picviz::PVSelectionFilter::PVSelectionFilter(PVCore::PVArgumentList const& args)
{
	INIT_FILTER(Picviz::PVSelectionFilter, args);
	set_output(NULL);
	_view = NULL;
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::
 *
 *****************************************************************************/
DEFAULT_ARGS_FILTER(Picviz::PVSelectionFilter)
{
	return PVCore::PVArgumentList();
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::operator_differentout
 *
 *****************************************************************************/
Picviz::PVSelection& Picviz::PVSelectionFilter::operator_differentout(PVSelection &selection)
{
	operator()(selection, *_out_p);
	return *_out_p;
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::operator()
 *
 *****************************************************************************/
Picviz::PVSelection& Picviz::PVSelectionFilter::operator()(PVSelection& selection)
{
	assert(_view);
	if (_out_p)
		return operator_differentout(selection);
	return operator_sameout(selection);
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::operator()
 *
 *****************************************************************************/
void Picviz::PVSelectionFilter::operator()(PVSelection &in, PVSelection &out)
{
	// By default, if out != in, copy it
	if (&out != &in)
		out = in;
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::operator_sameout
 *
 *****************************************************************************/
Picviz::PVSelection& Picviz::PVSelectionFilter::operator_sameout(PVSelection &selection)
{
	operator()(selection, selection);
	return selection;
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::set_output
 *
 *****************************************************************************/
void Picviz::PVSelectionFilter::set_output(PVSelection* out)
{
	_out_p = out;

/*	AG: do not remember the clean way to do this... later
 	if (out == NULL)
		this->operator()(PVSelection&) = &(this->operator_sameout);
	else
		this->operator()(PVSelection&) = &(this->operator_differentout);*/
}

/******************************************************************************
 *
 * Picviz::PVSelectionFilter::set_view
 *
 *****************************************************************************/
void Picviz::PVSelectionFilter::set_view(PVView const& view)
{
	_view = &view;
}

IMPL_FILTER(Picviz::PVSelectionFilter)
