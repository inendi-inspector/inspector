/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include "helpers.h"
#include <iostream>

using PVCore::PVChunk;
using PVCore::PVElement;
using PVCore::PVField;
using PVCore::list_elts;
using PVCore::list_fields;
using std::cout;
using std::cerr;
using std::endl;

void dump_chunk(PVChunk const& c)
{
	cerr << "Chunk:  index:" << c.index() << " logical size/avail size: " << c.size() << "/" << c.avail() << endl;
	cerr << "Content: ";
	dump_buffer(c.begin(), c.end());
	cerr << endl << endl;

	list_elts const& l = c.c_elements();
	list_elts::const_iterator it,ite;
	ite = l.end();
	for (it = l.begin(); it != ite; it++) {
		dump_elt(*(*it));
		cerr << endl;
	}
}

void dump_elt(PVElement const& elt)
{
	cerr << " - Element: valid: " << elt.valid() << " content: ";
	dump_buffer(elt.begin(), elt.end());
	cerr << endl;
	list_fields const& l = elt.c_fields();
	list_fields::const_iterator it,ite;
	ite = l.end();
	for (it = l.begin(); it != ite; it++)
		dump_field(*it);
}

void dump_field(PVField const& f)
{
	cerr << "  -- Field: valid: " << f.valid() << " content: ";
	dump_buffer(f.begin(), f.end());
	cerr << endl;
}

void dump_buffer(char* start, char* end)
{
	QString qtmp = QString::fromRawData((QChar*)start,((uintptr_t)end-(uintptr_t)start)/sizeof(QChar));
//	while (start < end) {
//		cerr.width(2);
//		cerr.fill('0');
//		cerr << std::hex << (unsigned int) *start << " ";
//		start++;
//	}

	// Print UTF-16 converted version
	cerr << " (" << qPrintable(qtmp) << ")";
	
//	while (start < end) {
//		cerr << *start;
//		start++;
//	}
}

void dump_chunk_csv(PVChunk& c)
{
	// Assume locale is UTF8 !
	list_elts& l = c.elements();
	list_elts::iterator it,ite;
	ite = l.end();
	QString str_tmp;
	for (it = l.begin(); it != ite; it++) {
		PVElement& elt = *(*it);
		if (!elt.valid()) {
			continue;
		}
		list_fields& l = elt.fields();
		if (l.size() == 1) {
			cout << l.begin()->get_qstr(str_tmp).toUtf8().constData();
		}
		else {
			list_fields::iterator itf,itfe;
			itfe = l.end();
			itfe--;
			for (itf = l.begin(); itf != itfe; itf++) {
				PVField& f = *itf;
				cout << "'" << f.get_qstr(str_tmp).toUtf8().constData() << "',";
			}
			PVField& f = *itf;
			cout << "'" << f.get_qstr(str_tmp).toUtf8().constData() << "'";
		}
		if (!elt.valid()) {
			cout << " (invalid)";
		}
		cout << endl;
	}
}

void dump_chunk_size_elts(PVChunk& c)
{
	// Assume locale is UTF8 !
	list_elts& l = c.elements();
	list_elts::iterator it,ite;
	ite = l.end();
	for (it = l.begin(); it != ite; it++) {
		PVElement& elt = *(*it);
		if (!elt.valid()) {
			continue;
		}
		list_fields& l = elt.fields();
		std::cout << "Number of elements: " << l.size();
		if (!elt.valid()) {
			std::cout << " (invalid)";
		}
		std::cout << endl;
	}
}

void dump_chunk_raw(PVChunk const& c)
{
	list_elts const& l = c.c_elements();
	list_elts::const_iterator it,ite;
	ite = l.end();
	for (it = l.begin(); it != ite; it++) {
		PVElement const& elt = *(*it);
		list_fields const& l = elt.c_fields();
		list_fields::const_iterator itf;
		for (itf = l.begin(); itf != l.end(); itf++) {
			PVField const& f = *itf;
			const char* start = f.begin();
			const char* end = f.end();
			write(1, start, ((ptrdiff_t)end-(ptrdiff_t)start));
		}
	}
}

void dump_chunk_newline(PVChunk const& c)
{
	list_elts const& l = c.c_elements();
	list_elts::const_iterator it,ite;
	ite = l.end();
	const uint16_t newline = 0x0A;
	for (it = l.begin(); it != ite; it++) {
		PVElement const& elt = *(*it);
		list_fields const& l = elt.c_fields();
		list_fields::const_iterator itf;
		for (itf = l.begin(); itf != l.end(); itf++) {
			PVField const& f = *itf;
			const char* start = f.begin();
			const char* end = f.end();
			write(1, start, ((ptrdiff_t)end-(ptrdiff_t)start));
		}
		write(1, &newline, 2);
	}
}

bool process_filter(PVRush::PVRawSourceBase& source, PVFilter::PVChunkFilter_f flt_f)
{
	PVChunk* pc = source();
	if (!pc) {
		std::cerr << "Error: unable to read source file" << std::endl;
		return false;
	}

	size_t nelts_org,nelts_valid;
	nelts_org = nelts_valid = 0;
	while (pc) {
		flt_f(pc);
		size_t no,nv;
		no = nv = 0;
		pc->get_elts_stat(no, nv);
		nelts_org += no;
		nelts_valid += nv;
		dump_chunk_csv(*pc);
		//dump_chunk_size_elts(*pc);
		pc->free();
		pc = source();
	}

	std::cout << nelts_valid << "/" << nelts_org << " elements are valid." << std::endl;
	return true;
}

void dump_nraw_csv(PVRush::PVNraw& nraw_)
{
	nraw_.dump_csv();
}

void dump_nraw_csv(PVRush::PVNraw& nraw_, const QString& csv_path)
{
	nraw_.dump_csv(csv_path);
}
