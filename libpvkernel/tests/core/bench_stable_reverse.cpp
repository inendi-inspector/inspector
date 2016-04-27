/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVAlgorithms.h>
#include <iostream>
#include <utility>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <tbb/tick_count.h>

#include <pvkernel/core/inendi_stat.h>

typedef std::pair<int, int> pair_int_t;
typedef std::vector<pair_int_t> vec_t;

bool less_p(pair_int_t const& v1, pair_int_t const& v2)
{
	return v1.second < v2.second;
}

bool greater_p(pair_int_t const& v1, pair_int_t const& v2)
{
	return v1.second > v2.second;
}

bool comp(pair_int_t const& v1, pair_int_t const& v2)
{
	return v1.second == v2.second;
}

void dump(vec_t const& v)
{
	vec_t::const_iterator it;
	for (it = v.begin(); it != v.end(); it++) {
		std::cout << it->second << "\t";
	}
	std::cout << std::endl;
	for (it = v.begin(); it != v.end(); it++) {
		std::cout << it->first << "\t";
	}
	std::cout << std::endl;
}

int main(int argc, char** argv)
{
	if (argc <= 2) {
		std::cerr << "Usage: " << argv[0] << " size_block nblocks" << std::endl;
		return 1;
	}

	tbb::tick_count start, end;
	srand(time(NULL));
	size_t sb = atoll(argv[1]);
	int nblocks = atoi(argv[2]);

	vec_t v;
	v.resize(nblocks * sb);

	for (int b = 0; b < nblocks; b++) {
		int nb = rand();
		for (size_t i = 0; i < sb; i++) {
			v[b * sb + i] = pair_int_t(i, nb);
		}
	}
	vec_t v_org(v);

	start = tbb::tick_count::now();
	std::stable_sort(v.begin(), v.end(), less_p);
	end = tbb::tick_count::now();
	PV_STAT_TIME_SEC("stable_sort", (end - start).seconds());
	vec_t v_save = v;

	start = tbb::tick_count::now();
	PVCore::stable_sort_reverse(v.begin(), v.end(), comp);
	end = tbb::tick_count::now();
	PV_STAT_TIME_SEC("stable_reverse", (end - start).seconds());

	v = v_save;
	start = tbb::tick_count::now();
	std::stable_sort(v.begin(), v.end(), greater_p);
	end = tbb::tick_count::now();
	PV_STAT_TIME_SEC("reverse_sort_from_sorted", (end - start).seconds());

	v = v_org;
	start = tbb::tick_count::now();
	std::stable_sort(v.begin(), v.end(), greater_p);
	end = tbb::tick_count::now();
	PV_STAT_TIME_SEC("reverse_sort_from_original", (end - start).seconds());

	return 0;
}
