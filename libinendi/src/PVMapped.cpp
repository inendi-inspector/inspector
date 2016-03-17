/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <QList>
#include <QStringList>
#include <QString>

#include <pvkernel/rush/PVFormat.h>

#include <inendi/PVMandatoryMappingFilter.h>
#include <inendi/PVMapping.h>
#include <inendi/PVMapped.h>
#include <inendi/PVPlotted.h>
#include <inendi/PVSelection.h>
#include <inendi/PVSource.h>
#include <inendi/PVView.h>

#include <inendi/PVRoot.h>

#include <boost/thread.hpp>

#include <iostream>

#include <unordered_set>
#include <float.h>
#include <omp.h>

#include <tbb/parallel_for.h>

#define DEFAULT_MAPPING_NROWS (16*1024*1024)

/******************************************************************************
 *
 * Inendi::PVMapped::PVMapped
 *
 *****************************************************************************/
Inendi::PVMapped::PVMapped()
{
}

/******************************************************************************
 *
 * Inendi::PVMapped::~PVMapped
 *
 *****************************************************************************/
Inendi::PVMapped::~PVMapped()
{
	remove_all_children();
	PVLOG_DEBUG("In PVMapped destructor\n");
}

void Inendi::PVMapped::set_parent_from_ptr(PVSource* source)
{
	data_tree_mapped_t::set_parent_from_ptr(source);
	_mapping = PVMapping_p(new PVMapping(this));
}

void Inendi::PVMapped::allocate_table(PVRow const nrows, PVCol const ncols)
{
	_trans_table.resize(ncols);
	reallocate_table(nrows);
}

void Inendi::PVMapped::reallocate_table(PVRow const nrows)
{
	for (mapped_row_t& mrow: _trans_table) {
		mrow.resize(nrows);
	}
}

void Inendi::PVMapped::init_process_from_rush_pipeline()
{
	PVCol const ncols = _mapping->get_number_cols();
	// Let's prepare the mappedtable for 10 million rows, and grow every 10 millions elements.
	// No matter if we go too far, this is just virtual memory, and isn't physically allocated
	// if nothing is written in it !
	allocate_table(DEFAULT_MAPPING_NROWS, ncols);

	// Create our own plugins from the library
	_mapping_filters_rush.resize(ncols);
	for (PVCol j = 0; j < ncols; j++) {
		PVMappingFilter::p_type mf = _mapping->get_filter_for_col(j);
		if (mf) {
			_mapping_filters_rush[j] = mf->clone<PVMappingFilter>();
			_mapping_filters_rush[j]->init();
		}
	}

	// Clear "group values" hash (just in case..)
	_grp_values_rush.clear();
}

void Inendi::PVMapped::compute()
{
	// Prepare mandatory mapping filters
	std::vector<PVMandatoryMappingFilter::p_type> mand_mapping_filters;
	LIB_CLASS(Inendi::PVMandatoryMappingFilter)::list_classes const& lfmf = LIB_CLASS(Inendi::PVMandatoryMappingFilter)::get().get_list();
	mand_mapping_filters.reserve(lfmf.size());
	for (auto it_lfmf = lfmf.begin(); it_lfmf != lfmf.end(); it_lfmf++) {
		PVMandatoryMappingFilter::p_type mf = it_lfmf->value()->clone<PVMandatoryMappingFilter>();
		mand_mapping_filters.push_back(mf);
	}

	const PVRow nrows = get_parent()->get_row_count();

	PVCol const ncols = _mapping->get_number_cols();

	if (nrows == 0) {
		// Nothing to map, early stop.
		return;
	}

	// Prepare the mapping table.
	allocate_table(nrows, ncols);

	// finalize import's mapping filters
	PVRush::PVNraw const& nraw = get_parent()->get_rushnraw();

	_mapping_filters_rush.resize(ncols);
	for (PVCol j = 0; j < ncols; j++) {
		// Create our own plugins from the library
		PVMappingFilter::p_type mf = _mapping->get_filter_for_col(j);
		_mapping_filters_rush[j] = mf->clone<PVMappingFilter>();
		_mapping_filters_rush[j]->init();

		// Compute mapping on this column
		PVMappingFilter::p_type& mapping_filter = _mapping_filters_rush[j];

		// Set MappingFilter array in filter to set it from filter.
		// FIXME : Ugly interface
		mapping_filter->set_dest_array(nrows, get_column_pointer(j));
		
		// Set mapping for the full column
		mapping_filter->operator()(j, nraw);

		tbb::tick_count start = tbb::tick_count::now();
		mapping_filter->finalize(j, nraw);
		tbb::tick_count end = tbb::tick_count::now();
		PVLOG_INFO("(PVMapped::finish_process_from_rush_pipeline) finalizing mapping for axis %d took %0.4f seconds.\n", j, (end-start).seconds());

		mandatory_param_map& params_map = _mapping->get_mandatory_params_for_col(j);
		tbb::tick_count tmap_start = tbb::tick_count::now();
		// Init the mandatory mapping
		for (auto it_pmf = mand_mapping_filters.begin(); it_pmf != mand_mapping_filters.end(); it_pmf++) {
			(*it_pmf)->set_dest_params(params_map);
			(*it_pmf)->set_decimal_type(_mapping_filters_rush[j]->get_decimal_type());
			(*it_pmf)->set_mapped(*this);
			(*it_pmf)->operator()(Inendi::mandatory_param_list_values(j, get_column_pointer(j)));
		}
		tbb::tick_count tmap_end = tbb::tick_count::now();

		PVLOG_INFO("(PVMapped::finish_process_from_rush_pipeline) mandatory mapping for axis %d took %0.4f seconds.\n", j, (tmap_end-tmap_start).seconds());
	}

	// Validate all mapping!
	validate_all();

	// Clear mapping filters
	_mapping_filters_rush.clear();

	// Clear "group values" hash
	_grp_values_rush.clear();

	// force plotteds updates (in case of .pvi load)
	for (auto plotted : get_children<PVPlotted>()) {
		plotted->finish_process_from_rush_pipeline();
	}
}

void Inendi::PVMapped::init_pure_mapping_functions(PVFilter::PVPureMappingProcessing::list_pure_mapping_t& funcs)
{
	const PVCol ncols = _mapping->get_number_cols();
	assert((PVCol) _mapping_filters_rush.size() == ncols);
	funcs.clear();
	funcs.resize(ncols);
	for (PVCol c = 0; c < ncols; c++) {
		if (is_mapping_pure(c)) {
			funcs[c] = _mapping_filters_rush[c]->f();
		}
	}
}

void Inendi::PVMapped::finish_process_from_rush_pipeline()
{
	const PVRow nrows = get_parent()->get_row_count();

	if (nrows == 0) {
		_trans_table.clear();
		return;
	}

	// Give back unused memory (over-allocated)
	reallocate_table(nrows);

	// finalize import's mapping filters
	PVRush::PVNraw const& nraw = get_parent()->get_rushnraw();

	for (PVCol col = 0; col < get_column_count(); ++col) {
		PVMappingFilter::p_type mapping_filter = _mapping_filters_rush[col];
		mapping_filter->set_dest_array(nrows, get_column_pointer(col));

		tbb::tick_count start = tbb::tick_count::now();
		mapping_filter->finalize(col, nraw);
		tbb::tick_count end = tbb::tick_count::now();
		PVLOG_INFO("(PVMapped::finish_process_from_rush_pipeline) finalizing mapping for axis %d took %0.4f seconds.\n", col, (end-start).seconds());
	}




	// Process mandatory mapping filters
	std::vector<PVMandatoryMappingFilter::p_type> mand_mapping_filters;
	LIB_CLASS(Inendi::PVMandatoryMappingFilter)::list_classes const& lfmf = LIB_CLASS(Inendi::PVMandatoryMappingFilter)::get().get_list();
	LIB_CLASS(Inendi::PVMandatoryMappingFilter)::list_classes::const_iterator it_lfmf;
	mand_mapping_filters.reserve(lfmf.size());
	for (it_lfmf = lfmf.begin(); it_lfmf != lfmf.end(); it_lfmf++) {
		PVMandatoryMappingFilter::p_type mf = it_lfmf->value()->clone<PVMandatoryMappingFilter>();
		mand_mapping_filters.push_back(mf);
	}
	std::vector<PVMandatoryMappingFilter::p_type>::const_iterator it_pmf;

	for (PVCol j = 0; j < get_column_count(); j++) {
		mandatory_param_map& params_map = _mapping->get_mandatory_params_for_col(j);
		tbb::tick_count tmap_start = tbb::tick_count::now();
		// Init the mandatory mapping
		for (it_pmf = mand_mapping_filters.begin(); it_pmf != mand_mapping_filters.end(); it_pmf++) {
			(*it_pmf)->set_dest_params(params_map);
			(*it_pmf)->set_decimal_type(_mapping_filters_rush[j]->get_decimal_type());
			(*it_pmf)->set_mapped(*this);
			(*it_pmf)->operator()(Inendi::mandatory_param_list_values(j, get_column_pointer(j)));
		}
		tbb::tick_count tmap_end = tbb::tick_count::now();

		PVLOG_INFO("(PVMapped::finish_process_from_rush_pipeline) mandatory mapping for axis %d took %0.4f seconds.\n", j, (tmap_end-tmap_start).seconds());
	}

	// Validate all mapping!
	validate_all();

	// Clear mapping filters
	_mapping_filters_rush.clear();

	// Clear "group values" hash
	_grp_values_rush.clear();

	// force plotteds updates (in case of .pvi load)
	for (auto plotted : get_children<PVPlotted>()) {
		plotted->finish_process_from_rush_pipeline();
	}

	//compute_unique_values();
}

void Inendi::PVMapped::compute_unique_values()
{
	tbb::tick_count t_start = tbb::tick_count::now();

	const PVRow nrows = get_parent()->get_row_count();
	const PVCol ncols = _mapping->get_number_cols();
	_unique_values_count.resize(ncols);

	std::vector<std::unordered_set<uint32_t>> unique_values;
	unique_values.resize(ncols);

	tbb::parallel_for(tbb::blocked_range<size_t>(0, ncols, 1),
		[&](tbb::blocked_range<size_t> const& range) {
			PVCol col = range.begin();
			auto& column_unique_values = unique_values[col];
			decimal_storage_type* buffer = get_column_pointer(col);
			for (size_t row = 0; row < nrows; row++) {
				column_unique_values.emplace(buffer[row].storage_as_uint());
			}
		}, tbb::simple_partitioner());

	for (PVCol col = 0; col < ncols; col++) {
		_unique_values_count[col] = unique_values[col].size();
		PVLOG_DEBUG("Column %d unique values: %u\n", col,  _unique_values_count[col]);
	}

	tbb::tick_count t_end = tbb::tick_count::now();

	PVLOG_INFO("(PVMapped::compute_unique_values) Computing mapped columns entropy took %0.4f seconds.\n", (t_end-t_start).seconds());
}

void Inendi::PVMapped::process_rush_pipeline_chunk(PVCore::PVChunk const* chunk, PVRow const cur_r)
{
	PVCore::list_elts const& elts = chunk->c_elements();
	const PVRow new_size = cur_r + elts.size();
	if (new_size > _trans_table[0].size()) {
		// Reallocate everyone for 10 more millions
		for (mapped_row_t& mrow: _trans_table) {
			mrow.resize(mrow.size() + DEFAULT_MAPPING_NROWS);
		}
	}

	chunk->visit_by_column([&](PVRow const r, PVCol const c, PVCore::PVField const& field)
		{
			assert(c < (PVCol) _mapping_filters_rush.size());
			if (is_mapping_pure(c)) {
				// AG: HACK: if we are not the first mapping, that's a failure..
				this->_trans_table[c].at(r+cur_r) = field.mapped_value();
			}
			else {
				PVMappingFilter::p_type& mapping_filter = _mapping_filters_rush[c];
				this->_trans_table[c].at(r+cur_r) = mapping_filter->operator()(field);
			}
		});
}

/******************************************************************************
 *
 * Inendi::PVMapped::create_table
 *
 *****************************************************************************/
void Inendi::PVMapped::create_table()
{
	PVRush::PVNraw const& nraw = get_parent()->get_rushnraw();
	const PVRow nrows = nraw.get_row_count();
	const PVCol ncols = nraw.get_number_cols();

	tbb::tick_count tstart = tbb::tick_count::now();
	allocate_table(nrows, ncols);

	PVLOG_INFO("(pvmapped::create_table) begin parallel mapping\n");

	// Create our own plugins from the library
	std::vector<PVMappingFilter::p_type> mapping_filters;
	mapping_filters.resize(ncols);
	for (PVCol j = 0; j < ncols; j++) {
		PVMappingFilter::p_type mf = _mapping->get_filter_for_col(j);
		if (mf) {
			mapping_filters[j] = mf->clone<PVMappingFilter>();
		}
	}

	// Do the same for the mandatory mappings
	std::vector<PVMandatoryMappingFilter::p_type> mand_mapping_filters;
	LIB_CLASS(Inendi::PVMandatoryMappingFilter)::list_classes const& lfmf = LIB_CLASS(Inendi::PVMandatoryMappingFilter)::get().get_list();
	LIB_CLASS(Inendi::PVMandatoryMappingFilter)::list_classes::const_iterator it_lfmf;
	mand_mapping_filters.reserve(lfmf.size());
	for (it_lfmf = lfmf.begin(); it_lfmf != lfmf.end(); it_lfmf++) {
		PVMandatoryMappingFilter::p_type mf = it_lfmf->value()->clone<PVMandatoryMappingFilter>();
		mand_mapping_filters.push_back(mf);
	}
	std::vector<PVMandatoryMappingFilter::p_type>::const_iterator it_pmf;

	try {
		// This is a hash whose key is "group_type", that contains the PVArgument
		// passed through all mapping filters that have the same group and type
		QHash<QString, PVCore::PVArgument> grp_values;
		for (PVCol j = 0; j < ncols; j++) {
			// Check that an update is required
			if (_mapping->get_properties_for_col(j).is_uptodate()) {
				continue;
			}

			// Get the corresponding object
			PVMappingFilter::p_type mapping_filter = mapping_filters[j];
			mandatory_param_map& params_map = _mapping->get_mandatory_params_for_col(j);
			params_map.clear();

			if (!mapping_filter) {
				PVLOG_ERROR("An invalid mapping type and/or mode is set for axis %d !\n", j);
				continue;
			}

			// Let's make our mapping
			mapping_filter->set_dest_array(nrows, get_column_pointer(j));
			//mapping_filter->set_axis(j, *get_format());
			// Get the group specific value if relevant
			QString group_key = _mapping->get_group_key_for_col(j);
			if (!group_key.isEmpty()) {
				PVCore::PVArgument& group_v = grp_values[group_key];
				mapping_filter->set_group_value(group_v);
			}
			boost::this_thread::interruption_point();
			tbb::tick_count tmap_start = tbb::tick_count::now();
			mapping_filter->init();
			mapping_filter->operator()(j, nraw);
			tbb::tick_count tmap_end = tbb::tick_count::now();
			PVLOG_INFO("(PVMapped::create_table) parallel mapping for axis %d took %0.4f seconds.\n", j, (tmap_end-tmap_start).seconds());

			tmap_start = tbb::tick_count::now();
			// Init the mandatory mapping
			boost::this_thread::interruption_point();
			for (it_pmf = mand_mapping_filters.begin(); it_pmf != mand_mapping_filters.end(); it_pmf++) {
				(*it_pmf)->set_dest_params(params_map);
				(*it_pmf)->set_decimal_type(mapping_filter->get_decimal_type());
				(*it_pmf)->set_mapped(*this);
				(*it_pmf)->operator()(Inendi::mandatory_param_list_values(j, get_column_pointer(j)));
			}
			tmap_end = tbb::tick_count::now();

			PVLOG_INFO("(PVMapped::create_table) mandatory mapping for axis %d took %0.4f seconds.\n", j, (tmap_end-tmap_start).seconds());

			_mapping->set_uptodate_for_col(j);
			invalidate_plotted_children_column(j);
		}
		PVLOG_INFO("(pvmapped::create_table) end parallel mapping\n");
		tbb::tick_count tend = tbb::tick_count::now();
		PVLOG_INFO("(PVPlotted::create_table) mapping took %0.4f seconds.\n", (tend-tstart).seconds());
	}
	catch (boost::thread_interrupted const& e)
	{
		PVLOG_INFO("(PVPlotted::create_table) mapping canceled.\n");
		throw e;
	}
}

/******************************************************************************
 *
 * Inendi::PVMapped::to_csv
 *
 *****************************************************************************/
namespace Inendi { namespace __impl {
struct to_csv_value_holder
{
	template <typename T>
	static void call(Inendi::PVMapped::mapped_table_t const& trans_table, PVRow const i, PVCol const j)
	{
		std::cout << trans_table[j][i].storage_cast<T>();
	}
};
} }

void Inendi::PVMapped::to_csv()
{
	// WARNING: this is all but efficient. Uses this for testing and
	// debugging purpose only !
	for (PVRow i = 0; i < get_row_count(); i++) {
		for (PVCol j = 0; j < get_column_count(); j++) {
			decimal_storage_type::call_from_type<__impl::to_csv_value_holder>(get_decimal_type_of_col(j), _trans_table, i, j);
			if (j != (get_column_count()-1)) {
				std::cout << ",";
			}
		}
		std::cout << "\n";
	}
}

/******************************************************************************
 *
 * Inendi::PVMapped::get_format
 *
 *****************************************************************************/
PVRush::PVFormat_p Inendi::PVMapped::get_format() const
{
	return get_parent()->get_rushnraw().get_format();
}

/******************************************************************************
 *
 * Inendi::PVMapped::get_sub_col_minmax
 *
 *****************************************************************************/
namespace Inendi { namespace __impl {
struct get_sub_col_minmax_holder
{
	template <typename T>
	static void call(Inendi::PVMapped::mapped_sub_col_t& ret, Inendi::PVMapped::decimal_storage_type& min, Inendi::PVMapped::decimal_storage_type& max, Inendi::PVSelection const& sel, PVCol const col, Inendi::PVMapped::mapped_table_t const& trans_table)
	{
		min.set_max<T>();
		max.set_min<T>();

		const Inendi::PVMapped::decimal_storage_type* mapped_values = &trans_table[col][0];
		T& max_cast = max.storage_cast<T>();
		T& min_cast = min.storage_cast<T>();
		sel.visit_selected_lines([&](PVRow const i){
			const Inendi::PVMapped::decimal_storage_type v = mapped_values[i];
			const T v_cast = v.storage_cast<T>();
			if (v_cast > max_cast) {
				max_cast = v_cast;
			}
			if (v_cast < min_cast) {
				min_cast = v_cast;
			}
			ret.push_back(Inendi::PVMapped::mapped_sub_col_t::value_type(i, v));
		},
		trans_table[0].size());
	}
};
} }

void Inendi::PVMapped::get_sub_col_minmax(mapped_sub_col_t& ret, decimal_storage_type& min, decimal_storage_type& max, PVSelection const& sel, PVCol const col) const
{
	PVCore::DecimalType const type_col = get_decimal_type_of_col(col);
	PVRow size = get_row_count();
	ret.reserve(sel.get_number_of_selected_lines_in_range(0, size-1));

	decimal_storage_type::call_from_type<__impl::get_sub_col_minmax_holder>(type_col, ret, min, max, sel, col, _trans_table);
}

/******************************************************************************
 *
 * Inendi::PVMapped::get_row_count
 *
 *****************************************************************************/
PVRow Inendi::PVMapped::get_row_count() const
{
	return get_parent()->get_row_count();
}

/******************************************************************************
 *
 * Inendi::PVMapped::get_column_count
 *
 *****************************************************************************/
PVCol Inendi::PVMapped::get_column_count() const
{
	return _trans_table.size();
}

void Inendi::PVMapped::add_column(PVMappingProperties const& props)
{
	_mapping->add_column(props);
}

void Inendi::PVMapped::process_parent_source()
{
	create_table();
}

void Inendi::PVMapped::process_from_parent_source()
{
	process_parent_source();
	// Process plotting children
	for (auto plotted_p : get_children<PVPlotted>()) {
		plotted_p->process_from_parent_mapped();
	}
}

void Inendi::PVMapped::invalidate_plotted_children_column(PVCol j)
{
	for (auto plotted_p : get_children<PVPlotted>()) {
		plotted_p->invalidate_column(j);
	}
}

void Inendi::PVMapped::invalidate_all()
{
	_mapping->invalidate_all();
}

void Inendi::PVMapped::validate_all()
{
	_mapping->validate_all();
}

#if 0
QList<PVCol> Inendi::PVMapped::get_columns_indexes_values_within_range(decimal_storage_type const min, decimal_storage_type const max, double rate)
{
	QList<PVCol> cols_ret;
	const PVRow nrows = get_row_count();
	const PVCol ncols = get_column_count();

	if (min > max) {
		return cols_ret;
	}

	double nrows_d = (double) nrows;
	for (PVCol j = 0; j < ncols; j++) {
		PVRow nmatch = 0;
		const float* values = trans_table.getRowData(j);
		// TODO: optimise w/ SIMD if relevant
		for (PVRow i = 0; i < nrows; i++) {
			const float v = values[i];
			if (v >= min && v <= max) {
				nmatch++;
			}
		}
		if ((double)nmatch/nrows_d >= rate) {
			cols_ret << j;
		}
	}

	return cols_ret;
}

QList<PVCol> Inendi::PVMapped::get_columns_indexes_values_not_within_range(decimal_storage_type const min, decimal_storage_type const max, double rate)
{
	QList<PVCol> cols_ret;
	const PVRow nrows = get_row_count();
	const PVCol ncols = get_column_count();

	if (min > max) {
		return cols_ret;
	}

	double nrows_d = (double) nrows;
	for (PVCol j = 0; j < ncols; j++) {
		PVRow nmatch = 0;
		const float* values = trans_table.getRowData(j);
		// TODO: optimise w/ SIMD if relevant
		for (PVRow i = 0; i < nrows; i++) {
			const float v = values[i];
			if (v < min || v > max) {
				nmatch++;
			}
		}
		if ((double)nmatch/nrows_d >= rate) {
			cols_ret << j;
		}
	}

	return cols_ret;
}
#endif

bool Inendi::PVMapped::is_current_mapped() const
{
	Inendi::PVView const* cur_view = get_parent<PVSource>()->current_view();
	for (auto const& cv: get_children<Inendi::PVView>()) {
		if (cv.get() == cur_view) {
			return true;
		}
	}
	return false;
}

void Inendi::PVMapped::serialize_write(PVCore::PVSerializeObject& so)
{
	data_tree_mapped_t::serialize_write(so);

	so.object(QString("mapping"), *_mapping, QString(), false, (PVMapping*) NULL, false);
}

void Inendi::PVMapped::serialize_read(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t v)
{
	PVMapping* mapping = new PVMapping();
	so.object(QString("mapping"), *mapping, QString(), false, (PVMapping*) NULL, false);
	_mapping = PVMapping_p(mapping);
	_mapping->set_mapped(this);

	// It important to deserialize the children after the mapping otherwise PVPlottingProperties complains that there is no mapping!
	data_tree_mapped_t::serialize_read(so, v);
}
