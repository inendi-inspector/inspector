/**
 * @file
 *
 * @copyright (C) ESI Group INENDI April 2015-2015
 */
#include "common.h"

#include <inendi/PVLayerFilter.h>
#include <inendi/PVLayer.h>
#include <pvkernel/core/PVEnumType.h>
#include <pvkernel/core/PVOriginalAxisIndexType.h>
#include <pvkernel/core/PVPlainTextType.h>

#include <pvkernel/core/inendi_assert.h>

#ifdef INSPECTOR_BENCH
constexpr char FILENAME[] = TEST_FOLDER "/picviz/multiple_search_bench.csv";
constexpr size_t DUPL = 1000;
#else
constexpr char FILENAME[] = TEST_FOLDER "/picviz/multiple_search.csv";
constexpr size_t DUPL = 1;
#endif

constexpr char FORMAT[] = TEST_FOLDER "/picviz/multiple_search.csv.format";
constexpr PVCol COLUMN_INDEX = 1;
static constexpr size_t row_count = 5150 * DUPL;

using options_t = std::pair<std::array<uint8_t, 4>, std::string>;
using testcase_t = std::pair<options_t, size_t>;

void set_args(PVCore::PVArgumentList& args, const options_t& values)
{
	static constexpr const char* params[] = { "include", "case", "entire", "interpret" };

	for (size_t i = 0; i < values.first.size(); i++) {
		PVCore::PVEnumType e = args[params[i]].value<PVCore::PVEnumType>();
		e.set_sel(values.first[i]);
		args[params[i]].setValue(e);
	}

	args["exps"].setValue(PVCore::PVPlainTextType(values.second.c_str()));
}

void run_tests(
	Inendi::PVLayerFilter::p_type& plugin,
	PVCore::PVArgumentList &args,
	Inendi::PVLayer& in,
	Inendi::PVLayer& out
)
{
	std::vector<testcase_t> tests {
		{{{0, 1, 1, 0}, "Tue Jan 06 01:23:28 GMT 2004\n"
				        "Mon Dec 12 23:56:00 GMT 2005"}, 100}, // EXACT_MATCH
		{{{0, 0, 1, 0}, "MoN dEc 12 23:56:00 Gmt 2005"}, 50},  // EXACT_MATCH + CASE_INSENSITIVE
		{{{0, 1, 1, 1}, ".*01.*"}, 700},                       // EXACT_MATCH + REGULAR_EXPRESSION
		{{{0, 0, 1, 1}, ".*w\\D{2}.*"}, 700},                  // EXACT_MATCH + REGULAR_EXPRESSION + CASE_INSENSITIVE
		{{{0, 1, 0, 1}, "\\d{2}\\:\\d{2}\\:00"}, 200},         // REGULAR_EXPRESSION
		{{{0, 0, 0, 1}, "j\\D{2}"}, 950},                      // REGULAR_EXPRESSION + CASE_INSENSITIVE
		{{{1, 0, 0, 0}, "jan"}, 4700},                         // CASE_INSENSITIVE
		{{{0, 1, 0, 0}, "Oct\nDec"}, 750},                         // NONE
	};

	for (const testcase_t& test : tests) {
		set_args(args, test.first);
		plugin->set_args(args);
		plugin->operator()(in);

		pvcop::db::selection sel = out.get_selection();
		pvcop::db::selection pvsel(sel, 0, row_count);
		PV_VALID(pvcop::core::algo::bit_count(pvsel), test.second * DUPL);
	}
}

int main()
{
    // Init nraw
    pvtest::TestEnv env(FILENAME, FORMAT);
    Inendi::PVView* view = env.view;

    // Get plugin reference
    constexpr char plugin_name[] = "search-multiple";
    Inendi::PVLayerFilter::p_type filter_org = LIB_CLASS(Inendi::PVLayerFilter)::get().get_class_by_name(plugin_name);
    Inendi::PVLayerFilter::p_type fclone = filter_org->clone<Inendi::PVLayerFilter>();
    PVCore::PVArgumentList &args = view->get_last_args_filter(plugin_name);

    Inendi::PVLayer out("Out");
    out.reset_to_empty_and_default_color();
    Inendi::PVLayer& in = view->get_layer_stack_output_layer();
    args["axis"].setValue(PVCore::PVOriginalAxisIndexType(COLUMN_INDEX));

    fclone->set_view(view);
    fclone->set_output(&out);

    auto start = std::chrono::system_clock::now();
    run_tests(fclone, args, in, out);
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - start;
    std::cout << diff.count();
}