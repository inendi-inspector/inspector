/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvbase/qhashes.h>
#include <pvkernel/core/inendi_bench.h>
#include <pvkernel/core/inendi_assert.h>
#include <pvkernel/core/PVDirectory.h>
#include <pvkernel/rush/PVNrawDiskBackend.h>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_reduce.h>

#include <cstdlib>

static const std::string INDEX_FILENAME = std::string("nraw.idx");
//const uint64_t PVRush::PVNrawDiskBackend::READ_BUFFER_SIZE = std::max<size_t>(PVCore::PVHardwareConcurrency::get_level_n_cache_size(1), 256*1024);
//const uint64_t PVRush::PVNrawDiskBackend::READ_BUFFER_SIZE = 4*1024;

// class tbb_chunks_t
PVRush::PVNrawDiskBackend::tbb_chunks_t::tbb_chunks_t(size_t n):
			_n(n)
{
	_chunks = PVCore::PVAlignedAllocator<chunk_t, 16>().allocate(n);
	_queue.set_capacity(n);
	for (size_t i = 0; i < n; i++) {
		_queue.try_push(&_chunks[i]);
	}
}

PVRush::PVNrawDiskBackend::tbb_chunks_t::~tbb_chunks_t()
{
	PVCore::PVAlignedAllocator<chunk_t, 16>().deallocate(_chunks, _n);
}


// class PVNrawDiskBackend
PVRush::PVNrawDiskBackend::PVNrawDiskBackend():
		_cache_pool(*this),
		_nrows(0),
		_serial_read_buffer(nullptr),
		_chunks(48)
{
}

PVRush::PVNrawDiskBackend::~PVNrawDiskBackend()
{
	if (_serial_read_buffer) {
		PVCore::PVAlignedAllocator<char, BUF_ALIGN>().deallocate(_serial_read_buffer, SERIAL_READ_BUFFER_SIZE);
		_serial_read_buffer = nullptr;
	}
	close_files();
}

void PVRush::PVNrawDiskBackend::close_files()
{
	for (uint64_t col_idx = 0 ; col_idx < get_number_cols(); col_idx++) {
		PVColumn& column = get_col(col_idx);

		if (column.write_file != 0) {
			// Close write file descriptor
			this->Close(column.write_file);
		}

		// Close read files descriptors
		for (file_t& read_file : column.read_file_tls) {
			this->Close(read_file);
		}
	}
}

void PVRush::PVNrawDiskBackend::init(const char* nraw_folder, const uint64_t num_cols, bool open_write_files)
{
	_nraw_folder = nraw_folder;
	_columns.reserve(num_cols);
	for (uint64_t col = 0 ; col < num_cols ; col++) {
		_columns.emplace_back();
		PVColumn& column = _columns.back();

		column.filename = std::move(get_disk_column_file(col));

		if (open_write_files) {
			// Open file
			if(!this->Open(column.filename, &column.write_file, _direct_mode, _direct_mode)) {
				PVLOG_DEBUG("PVNrawDiskBackend: Warning: file %s will not be opened in direct mode\n", column.filename.c_str());
				set_direct_mode(false);
				if(!this->Open(column.filename, &column.write_file, _direct_mode, _direct_mode)) {
					PVLOG_ERROR("PVNrawDiskBackend: Error opening file %s (%s)\n", column.filename.c_str(), strerror(errno));
					return;
				}
			}

			// Create buffer
			uint64_t buffer_size = _write_buffers_size_pattern[0];
			column.buffer_write = PVCore::PVAlignedAllocator<char, BUF_ALIGN>().allocate(buffer_size);
			memset(column.buffer_write, 0, buffer_size);
			column.buffer_write_ptr = column.buffer_write;
			column.buffer_write_end_ptr = column.buffer_write + buffer_size;
			column.field_length = 0; // Or any value grater than 0 to specify a fixed field length;
		}
	}
	const offset_fields_t null_offset_fields{};
	_indexes.resize(_next_indexes_nrows, num_cols, null_offset_fields);
	_next_indexes_nrows += _index_fields_size_pattern[++_fields_size_idx];

	_serial_read_buffer = PVCore::PVAlignedAllocator<char, BUF_ALIGN>().allocate(SERIAL_READ_BUFFER_SIZE);
}

void PVRush::PVNrawDiskBackend::clear_and_remove()
{
	clear();
	close_files();

#if 0 // RH: inhibited to allow nraw reuse on project load
	PVCore::PVDirectory::remove_rec(QString::fromLocal8Bit(_nraw_folder.c_str()));
#endif
}

uint64_t PVRush::PVNrawDiskBackend::add(PVCol col_idx, const char* field, const uint64_t field_size)
{
	PVColumn& column = get_col(col_idx);
	const uint64_t written_field_size = field_size + column.end_char();
	const offset_fields_t null_offset_fields{};
	uint64_t field_part2_size = 0;
	char* field_part2 = nullptr;
	uint64_t write_size = 0;

	// Index field
	if (column.fields_ignored_size + written_field_size > READ_BUFFER_SIZE) {
		assert(column.buffer_write <= column.buffer_write_ptr);
		uint64_t field_offset_in_file = this->Tell(column.write_file) + ((uintptr_t)column.buffer_write_ptr - (uintptr_t)column.buffer_write) ;

		// Resize indexes matrix if needed
		if (column.fields_indexed == _indexes.get_nrows()) {
			_indexes.resize_nrows(_next_indexes_nrows, null_offset_fields);
			uint64_t index = std::min(++_fields_size_idx, _max_fields_size_idx);
			_next_indexes_nrows += _index_fields_size_pattern[index];
		}

		offset_fields_t offset_field;
		offset_field.offset = field_offset_in_file;
		offset_field.field = column.fields_nb;
		_indexes.set_value(column.fields_indexed, col_idx, offset_field);
		column.fields_ignored_size = 0;
		column.fields_indexed++;
	}
	column.fields_ignored_size += written_field_size;
	_indexes_nrows = std::max(column.fields_indexed, _indexes_nrows);
	column.fields_nb++;

	// Fill the buffer with complete field
	if (column.buffer_write_ptr + written_field_size <= column.buffer_write_end_ptr) {
		memcpy(column.buffer_write_ptr, field, field_size);
		column.buffer_write_ptr[field_size] = 0;
		column.buffer_write_ptr += written_field_size;
		assert(column.buffer_write_ptr <= column.buffer_write_end_ptr);
	}
	// Fill the buffer_write with splitted field
	else {
		assert((uintptr_t)column.buffer_write_end_ptr >= (uintptr_t)column.buffer_write_ptr);
		uint64_t field_part1_size = column.buffer_write_end_ptr - column.buffer_write_ptr;
		memcpy(column.buffer_write_ptr, field, field_part1_size);
		field_part2 = (char *)(field + field_part1_size);
		field_part2_size = written_field_size - field_part1_size;
		column.buffer_write_ptr += field_part1_size;
		assert(column.buffer_write_ptr == column.buffer_write_end_ptr);
	}

	// Write buffer_write to disk
	if (column.buffer_write_ptr == column.buffer_write_end_ptr) {
		write_size = this->Write(column.buffer_write, _write_buffers_size_pattern[column.buffers_write_size_idx], column.write_file);
		if(write_size <= 0) {
			PVLOG_ERROR("PVNrawDiskBackend: Error writing column %d to disk (%s)\n", col_idx, strerror(errno));
			return 0;
		}

		uint64_t buffer_max_size = _write_buffers_size_pattern[column.buffers_write_size_idx];

		// Reallocate a bigger buffer_write and copy the end of splitted field
		if (column.buffers_write_size_idx < _max_write_size_idx) {
			uint64_t new_buffer_max_size = _write_buffers_size_pattern[++column.buffers_write_size_idx];
			PVCore::PVAlignedAllocator<char, BUF_ALIGN>().deallocate(column.buffer_write, buffer_max_size);
			column.buffer_write = PVCore::PVAlignedAllocator<char, BUF_ALIGN>().allocate(new_buffer_max_size);
			column.buffer_write_ptr = column.buffer_write;
			column.buffer_write_end_ptr = column.buffer_write + new_buffer_max_size;
		}
		// Recycle previously allocated buffer_write
		else {
			column.buffer_write_ptr = column.buffer_write;
		}

		if (field_part2_size > 0) {
			memcpy(column.buffer_write, field_part2, field_part2_size-1); // '-1' because there is no trailing '\0' in field_part2
			column.buffer_write[field_part2_size-1] = 0;
			column.buffer_write_ptr = column.buffer_write + field_part2_size;
			assert(column.buffer_write_ptr <= column.buffer_write_end_ptr);
		}
	}
	_nrows = std::max(_nrows, column.fields_nb);

	return write_size;
}

void PVRush::PVNrawDiskBackend::flush()
{
	// direct mode prevents to write a buffer of an arbitrary size
	set_direct_mode(false);

	for (uint64_t col_idx = 0 ; col_idx < get_number_cols(); col_idx++) {
		PVColumn& column = get_col(col_idx);
		uint64_t partial_buffer_size = column.buffer_write_ptr - column.buffer_write;
		if (partial_buffer_size > 0) {
			if(!this->Write(column.buffer_write, partial_buffer_size, column.write_file)) {
				PVLOG_ERROR("PVNrawDiskBackend: Error writing column %d to disk (%s)\n", col_idx, strerror(errno));
				this->Close(column.write_file);
				return;
			}
		}
		PVCore::PVAlignedAllocator<char, BUF_ALIGN>().deallocate(column.buffer_write, _write_buffers_size_pattern[column.buffers_write_size_idx]);
		column.buffer_write = nullptr;
		column.buffer_write_ptr = nullptr;
		column.buffer_write_end_ptr = nullptr;
		//column.reset();
		this->Close(column.write_file);
	}

	store_index_to_disk();
}

uint64_t PVRush::PVNrawDiskBackend::search_in_column(uint64_t col_idx, std::string const& field)
{
	PVColumn& column = get_col(col_idx);
	file_t& read_file = column.read_file_tls.local();
	if (unlikely(!read_file)) {
		this->Open(column.filename, &read_file, _direct_mode);
	}

	uint64_t chunk_size = _write_buffers_size_pattern[_max_write_size_idx];
	uint64_t total_read_size = 0;
	uint64_t nb_occur = 0;
	uint64_t read_size = 0;

	char* const buffer = PVCore::PVAlignedAllocator<char, BUF_ALIGN>().allocate(chunk_size);

	char* buffer_ptr = buffer + BUF_ALIGN;
	bool last_chunk = false;
	do
	{
		read_size = this->Read(read_file, buffer+BUF_ALIGN, chunk_size-BUF_ALIGN);
		last_chunk = read_size < (chunk_size-BUF_ALIGN);

		// TODO: clean this mess
		while (true) {
			char* endl = nullptr;
			if (last_chunk) {
				endl = (char*) memchr(buffer_ptr, '\0', chunk_size);
				if (endl == nullptr || buffer_ptr >= buffer+BUF_ALIGN+read_size) {
					buffer_ptr = buffer;
					break;
				}
			}
			else {
				endl = (char*) memchr(buffer_ptr, '\0', chunk_size);
				if (endl == nullptr) {
					uint64_t partial_line_length = buffer+chunk_size-buffer_ptr;
					char* dst = buffer+BUF_ALIGN-partial_line_length;
					memcpy(dst, buffer_ptr, partial_line_length);
					buffer_ptr = dst;
					break;
				}
			}

			int64_t line_length = (&endl[0] - &buffer_ptr[0]);
			nb_occur += (memcmp(buffer_ptr, field.c_str(), field.length()) == 0);
			buffer_ptr += (line_length+1);
		}
		total_read_size += read_size;

	} while(!last_chunk);

	PVCore::PVAlignedAllocator<char, BUF_ALIGN>().deallocate(buffer, chunk_size);

	return nb_occur;
}

void PVRush::PVNrawDiskBackend::set_direct_mode(bool direct)
{
	if (_direct_mode == direct) {
		return;
	}

	for (size_t col = 0 ; col < get_number_cols() ; col++) {
		PVColumn& column = get_col(col);
		if (column.write_file != 0) {
			uint64_t pos = this->Tell(column.write_file);
			this->Close(column.write_file);
			this->Open(column.filename, &column.write_file, direct);
			this->Seek(column.write_file, pos);
		}
	}
	_direct_mode = direct;
}

void PVRush::PVNrawDiskBackend::store_index_to_disk()
{
	char* data = (char*) _indexes.get_data();
	file_t file;
	this->Open(get_disk_index_file(), &file, false);
	this->Write(&_nrows, sizeof(size_t), file);
	uint64_t size = get_number_cols() * _indexes_nrows * sizeof(offset_fields_t);

	if (size > 0) {
		int64_t write_size = this->Write(data, size, file);
		if(write_size <= 0) {
			PVLOG_ERROR("PVNrawDiskBackend: Error writing index to disk [size=%d] (%s)\n", size, strerror(errno));
			return;
		}
	}
	this->Close(file);
}

bool PVRush::PVNrawDiskBackend::load_index_from_disk()
{
	const offset_fields_t null_offset_fields{};
	// TODO: wrong size computation!
	set_direct_mode(false);
	file_t file;
	this->Open(get_disk_index_file(), &file, false);
	size_t size = this->Size(file) - sizeof(_nrows);
	if (this->Read(file, (size_t*) &_nrows, sizeof(size_t)) != sizeof(size_t)) {
		PVLOG_ERROR("PVNrawDiskBackend: Error reading index from disk (%s)\n", strerror(errno));
		return false;
	}
	_indexes_nrows = size / get_number_cols() / sizeof(offset_fields_t);
	_indexes.resize(_indexes_nrows, get_number_cols(), null_offset_fields);
	char* data = (char*) _indexes.get_data();
	int64_t read_size = this->Read(file, data, size);

	if(read_size <= 0) {
		PVLOG_ERROR("PVNrawDiskBackend: Error reading index from disk (%s)\n", strerror(errno));
		return false;
	}
	this->Close(file);

	/**
	 * the number of effectivly used entries in _indexes has to be computed (required by
	 * PVCachePool::get_index)
	 */
#pragma omp parallel for
	for (uint64_t i = 0; i < get_number_cols(); ++i) {
		typename index_table_t::column index_col = _indexes.get_col(i);

		uint64_t p = 0;
		while((p != _indexes_nrows) && (index_col.at(p).field != 0)) {
			++p;
		}
		get_col(i).fields_indexed = p;
	}

	return true;
}

void PVRush::PVNrawDiskBackend::clear()
{
#if 0 // RH: inhibited to allow nraw reuse on project load
	unlink(get_disk_index_file().c_str());

	for (uint64_t c = 0 ; c < get_number_cols() ; c++) {
		PVColumn& nraw_c = _columns[c];
		this->Truncate(nraw_c.write_file, 0);

		nraw_c.buffer_write_ptr = nraw_c.buffer_write;
		nraw_c.field_length = 0; // Or any value grater than 0 to specify a fixed field length;
	}
#endif

	_fields_size_idx = 0;
	_next_indexes_nrows += _index_fields_size_pattern[++_fields_size_idx];
	_nrows = 0;
}

void PVRush::PVNrawDiskBackend::print_stats()
{
	PVLOG_INFO("resize: %d msec\n", this->_matrix_resize_interval.seconds()*1000);
}

void PVRush::PVNrawDiskBackend::print_indexes()
{
	for (uint64_t r=0; r < _indexes_nrows; r++) {
		for (uint64_t c=0; c < get_number_cols(); c++) {
			PVLOG_INFO("index[%llu][%llu] = %llu (offset), %llu (field)\n", r, c, _indexes.at(r, c).offset, _indexes.at(r, c).field);
		}
	}
}

char* PVRush::PVNrawDiskBackend::next(uint64_t col, uint64_t nb_fields, char* buffer, size_t& size_ret, bool* complete /*= nullptr*/)
{
	PVColumn& column = get_col(col);

	if (buffer == nullptr) return nullptr;

	// Extract proper field from buffer
	char* buffer_ptr = buffer;
	if (column.field_length > 0) {
		buffer_ptr += nb_fields * column.field_length;
		size_ret = column.field_length;
	}
	else {
		uint64_t size_to_read = READ_BUFFER_SIZE - (buffer - column.buffer_read_ptr);
		buffer_ptr = (char*) PVCore::PVByteVisitor::get_nth_slice((const uint8_t*) buffer, size_to_read, nb_fields, size_ret, complete);
	}

	column.buffer_read_ptr = buffer_ptr;

	return buffer_ptr;
}

const char* PVRush::PVNrawDiskBackend::at_no_cache(PVRow field, PVCol col, size_t& size_ret, char* read_buffer, bool* complete /*= nullptr*/) const
{
	// Read buffer must have a length >= READ_BUFFER_SIZE+BUF_ALIGN
	// Fetch content from disk
	const PVColumn& column = get_col(col);
	file_t& read_file = column.read_file_tls.local();
	if (unlikely(!read_file)) {
		this->Open(column.filename, &read_file, _direct_mode);
	}

	char* buffer = read_buffer;
	//BENCH_START(getindex);
	int64_t field_index = _cache_pool.get_index(col, field);
	//BENCH_STOP(getindex);
	//_stats_getindex += (uint64_t) (BENCH_END_TIME(getindex)*1000000.0);
	uint64_t disk_offset = unlikely(field_index == -1) ? 0 : _indexes.at(field_index, col).offset;
	uint64_t aligned_disk_offset = disk_offset;
	if (unlikely(_direct_mode)) {
		aligned_disk_offset = (disk_offset / BUF_ALIGN) * BUF_ALIGN;
	}

	//BENCH_START(read);
	int64_t read_size = this->ReadAt(read_file, aligned_disk_offset, buffer, READ_BUFFER_SIZE);
	//BENCH_STOP(read);
	//_stats_read += (uint64_t) (BENCH_END_TIME(read)*1000000.0);
	if(unlikely(read_size <= 0)) {
		PVLOG_ERROR("PVNrawDiskBackend: Error reading column %d [offset=%d] from disk (%s)\n", col, aligned_disk_offset, strerror(errno));
		return 0;
	}
	uint64_t first_field = field_index == -1 ? 0 :_indexes.at(field_index, col).field;
	uint64_t nb_fields = field - first_field;

	// Search field in content
	char* buffer_ptr = buffer;
	if (column.field_length > 0) {
		buffer_ptr += nb_fields * column.field_length;
		size_ret = column.field_length;
	}
	else {
		//BENCH_START(search);
		buffer_ptr = (char*) PVCore::PVByteVisitor::get_nth_slice((const uint8_t*) buffer, read_size, nb_fields, size_ret, complete);
		buffer_ptr = buffer_ptr ? buffer_ptr : (char*) "";
		//BENCH_STOP(search);
		//_stats_search += (uint64_t) (BENCH_END_TIME(search)*1000000.0);
	}

	return buffer_ptr;
}

std::string PVRush::PVNrawDiskBackend::get_disk_index_file() const
{
	return  _nraw_folder + "/" + INDEX_FILENAME;
}

std::string PVRush::PVNrawDiskBackend::get_disk_column_file(uint64_t col) const
{
	assert(col < get_number_cols());
	std::stringstream filename;
	filename << _nraw_folder << "/column_" << col;
	return filename.str();
}

// class PVCachePool
PVRush::PVNrawDiskBackend::PVCachePool::PVCachePool(PVRush::PVNrawDiskBackend& parent) : _backend(parent)
{
	for (uint64_t cache_idx = 0; cache_idx < NB_CACHE_BUFFERS; cache_idx++) {
		_caches[cache_idx].buffer = PVCore::PVAlignedAllocator<char, BUF_ALIGN>().allocate(READ_BUFFER_SIZE);
	}
}

uint64_t PVRush::PVNrawDiskBackend::PVCachePool::get_cache(uint64_t field, uint64_t col)
{
	bool cache_miss = true;

	PVColumn& column = _backend.get_col(col);
	uint64_t cache_idx = column.cache_index;

	// Is there a cache for this column?

	// Yes
	if (cache_idx != INVALID) {
		PVReadCache& cache = _caches[cache_idx];
		cache_miss = (field < cache.first_field || field > cache.last_field);
	}
	// No: find LRU cache
	else {
		tbb::tick_count older_timestamp = tbb::tick_count::now();
		for (uint64_t i = 0; i < NB_CACHE_BUFFERS; i++) {
			PVReadCache& cache = _caches[i];
			tbb::tick_count::interval_t interval = cache.timestamp - older_timestamp;
			if (interval.seconds() < 0) {
				older_timestamp = cache.timestamp;
				cache_idx = i;
			}
		}
		if (_caches[cache_idx].column != INVALID) {
			_backend.get_col(_caches[cache_idx].column).cache_index = INVALID;  // Tell the column that we are stealing its cache...
		}

		column.cache_index = cache_idx;
		_caches[cache_idx].column = col;
	}

	PVReadCache& cache = _caches[cache_idx];

	// Fetch data from disk
	uint64_t nb_fields_left = 0;
	if (cache_miss) {
		file_t& read_file = column.read_file_tls.local();
		if (unlikely(!read_file)) {
			this->Open(column.filename, &read_file, _backend._direct_mode);
		}
		char* buffer_ptr = cache.buffer;
		int64_t field_index = get_index(col, field);
		uint64_t disk_offset = field_index == -1 ? 0 : _backend._indexes.at(field_index, col).offset;
		uint64_t aligned_disk_offset = disk_offset;
		if (_backend._direct_mode) {
			aligned_disk_offset = (disk_offset / BUF_ALIGN) * BUF_ALIGN;
			buffer_ptr += (disk_offset - aligned_disk_offset);
		}
		int64_t read_size = _backend.ReadAt(read_file, aligned_disk_offset, cache.buffer, READ_BUFFER_SIZE);
		if(read_size <= 0) {
			PVLOG_ERROR("PVNrawDiskBackend: Error reading column %d [offset=%d] from disk (%s)\n", col, aligned_disk_offset, strerror(errno));
			return 0;
		}
		cache.first_field = field_index == -1 ? 0 :_backend._indexes.at(field_index, col).field;
		PVRow index = std::min<PVRow>(_backend._indexes.get_nrows()-1, field_index+1);
		cache.last_field = _backend._indexes.at(index, col).field-1;
		nb_fields_left = field - cache.first_field;
		column.buffer_read = buffer_ptr;
		column.buffer_read_ptr = buffer_ptr;
	}
	else {
		if (field > column.last_read_field) {
			nb_fields_left = field - column.last_read_field;
		}
		else {
			nb_fields_left = field - cache.first_field;
			column.buffer_read_ptr = column.buffer_read;
		}
	}

	// Update cache timestamp
	cache.timestamp = tbb::tick_count::now();
	column.last_read_field = field;

	return nb_fields_left;
}

PVRush::PVNrawDiskBackend::PVCachePool::~PVCachePool()
{
	for (uint64_t cache_idx = 0; cache_idx < NB_CACHE_BUFFERS; cache_idx++) {
		PVCore::PVAlignedAllocator<char, BUF_ALIGN>().deallocate(_caches[cache_idx].buffer, READ_BUFFER_SIZE);
	}
}

int64_t PVRush::PVNrawDiskBackend::PVCachePool::get_index(uint64_t col, uint64_t field) const
{
	index_table_t& indexes = _backend._indexes;
	const size_t findexed = _backend.get_col(col).fields_indexed;
	if (findexed == 0) {
		return -1;
	}

    int64_t first = 0;
	int64_t last = findexed-1;

	while (first <= (last-8)) {
		const int64_t index = (first + last) / 2;

		if (field >= indexes.at(index, col).field) {
			if (field < indexes.at(index+1, col).field) {
				return index;
			}
		    first = index + 1;
		}
		else {
			last = index - 1;
			if (last < 0) {
				return -1;
			}
		}
	}

	for (int64_t index = last; index >= first; index--) {
		if (indexes.at(index, col).field <= field) {
			return index;
		}
	}

#if 0
	for (int64_t index = findexed-1; index >= 0; index--) {
		if (indexes.at(index, col).field <= field) {
			return index;
		}
	}
#endif
	return -1;
}

bool PVRush::PVNrawDiskBackend::get_unique_values(PVCol const c, unique_values_t& ret, uint64_t& min, uint64_t& max, PVCore::PVSelBitField const& sel, tbb::task_group_context* ctxt)
{
	BENCH_START(get_unique_values);

	return operation_on_distinct_values<unique_values_unordered_map_t>(
		c,
		ret,
		min,
		max,
		[&](unique_values_unordered_map_t& values, size_t row, const char* buf, size_t n)
		{
			distinct_values_insertion(values, row, buf, n);
		},
		[&](unique_values_unordered_map_t& values, const std::string_tbb& v1, const uint64_t& v2)
		{
			values[v1] += v2;
		},
		[&](uint64_t& min, uint64_t& max, uint64_t& val, const unique_values_value_t& value)
		{
			val = value.second;
			min = std::min(min, (uint64_t) val);
			max = std::max(max, (uint64_t) val);
		},
		sel,
		ctxt
	);

	BENCH_END(get_unique_values, "get_unique_values", 1, 1, 1, 1);

	return true;
}

bool PVRush::PVNrawDiskBackend::sum_by(PVCol const col1, PVCol const col2, sum_by_t& ret, uint64_t& min, uint64_t& max, PVCore::PVSelBitField const& sel, uint64_t& sum, tbb::task_group_context* ctxt /* = nullptr */)
{
	BENCH_START(sum_by);

	max = 0;

	bool res = false;

	res = operation_on_distinct_values<unique_values_unordered_map_t>(
		col1,
		ret,
		min,
		max,
		[&, col2](unique_values_unordered_map_t& values, size_t row, const char* buf, size_t n)
		{
			op_by_insertion(
				values,
				row,
				col2,
				buf,
				n,
				[&](unique_values_unordered_map_t& values, const std::string_tbb& str, uint64_t value)
				{
					values[str] += value;
				}
			);
		},
		[&](unique_values_unordered_map_t& values, const std::string_tbb& v1, const uint64_t& v2)
		{
			auto& m = values[v1];
			m += v2;
		},
		[&](uint64_t& min, uint64_t& max, uint64_t& val, const unique_values_value_t& value)
		{
			val = value.second;
			min = std::min(min, (uint64_t) val);
			max = std::max(max, (uint64_t) val);
		},
		sel,
		ctxt
	);

	if (!res) {
		return false;
	}

	res = get_sum(col2, sum, sel, ctxt);

	BENCH_END(sum_by, "sum_by", 1, 1, 1, 1);

	return res;
}

bool PVRush::PVNrawDiskBackend::max_by(PVCol const col1, PVCol const col2, sum_by_t& ret, uint64_t& min, uint64_t& max, PVCore::PVSelBitField const& sel, tbb::task_group_context* ctxt /* = nullptr */)
{
	BENCH_START(max_by);

	max = 0;

	bool res = false;
	res = operation_on_distinct_values<unique_values_unordered_map_t>(
		col1,
		ret,
		min,
		max,
		[&, col2](unique_values_unordered_map_t& values, size_t row, const char* buf, size_t n)
		{
			op_by_insertion(
				values,
				row,
				col2,
				buf,
				n,
				[&](unique_values_unordered_map_t& values, const std::string_tbb& str, uint64_t value)
				{
					auto& m = values[str];
					m = std::max(m, value);
				}
			);
		},
		[&](unique_values_unordered_map_t& values, const std::string_tbb& v1, const uint64_t& v2)
		{
			auto& m = values[v1];
			m = std::max((uint64_t&)m, v2);
		},
		[&](uint64_t& min, uint64_t& max, uint64_t& val, const unique_values_value_t& value)
		{
			val = value.second;
			min = std::min(min, (uint64_t) val);
			max = std::max(max, (uint64_t) val);
		},
		sel,
		ctxt
	);

	if (!res) {
		return false;
	}

	BENCH_END(max_by, "max_by", 1, 1, 1, 1);

	return res;
}

bool PVRush::PVNrawDiskBackend::min_by(PVCol const col1, PVCol const col2, sum_by_t& ret, uint64_t& min, uint64_t& max, PVCore::PVSelBitField const& sel, tbb::task_group_context* ctxt /* = nullptr */)
{

	BENCH_START(min_by);

	max = 0;

	bool res = false;

	res = operation_on_distinct_values<min_by_unordered_map_t>(
		col1,
		ret,
		min,
		max,
		[&, col2](min_by_unordered_map_t& values, size_t row, const char* buf, size_t n)
		{
			op_by_insertion(
				values,
				row,
				col2,
				buf,
				n,
				[&](unique_values_unordered_map_t& values, const std::string_tbb& str, uint64_t value)
				{
					auto& m = values[str];
					m = std::min(m, value);
				}
			);
		},
		[&](min_by_unordered_map_t& values, const std::string_tbb& v1, const uint64_t& v2)
		{
			auto& m = values[v1];
			m = std::min((uint64_t&)m, v2);
		},
		[&](uint64_t& min, uint64_t& max, uint64_t& val, const unique_values_value_t& value)
		{
			val = value.second;
			min = std::min(min, (uint64_t) val);
			max = std::max(max, (uint64_t) val);
		},
		sel,
		ctxt
	);

	if (!res) {
		return false;
	}

	BENCH_END(min_by, "min_by", 1, 1, 1, 1);


	return res;
}

bool PVRush::PVNrawDiskBackend::avg_by(PVCol const col1, PVCol const col2, sum_by_t& ret, uint64_t& min, uint64_t& max, PVCore::PVSelBitField const& sel, tbb::task_group_context* ctxt /* = nullptr */)
{
	BENCH_START(average_by);

	max = 0;

	bool res = false;

	res = operation_on_distinct_values<average_by_unordered_map_t>(
		col1,
		ret,
		min,
		max,
		[&, col2](average_by_unordered_map_t& values, size_t row, const char* buf, size_t n)
		{
			op_by_insertion(
				values,
				row,
				col2,
				buf,
				n,
				[&](average_by_unordered_map_t& values, const std::string_tbb& str, uint64_t value)
				{
					auto& m = values[str];
					m.first += value;
					m.second++;
				}
			);
		},
		[&](average_by_unordered_map_t& values, const std::string_tbb& v1, const average_by_value_t& v2)
		{
			auto& m = values[v1];
			m.first += v2.first;
			m.second += v2.second;
		},
		[&](uint64_t& min, uint64_t& max, uint64_t& val, average_by_value_type_t& value)
		{
			val = value.second.first / value.second.second;
			min = std::min(min, (uint64_t) val);
			max = std::max(max, (uint64_t) val);
		},
		sel,
		ctxt
	);

	if (!res) {
		return false;
	}

	BENCH_END(average_by, "average_by", 1, 1, 1, 1);

	return res;
}

bool PVRush::PVNrawDiskBackend::count_by(
	PVCol const col1,
	PVCol const col2,
	count_by_t& ret,
	uint64_t& min,
	uint64_t& max,
	PVCore::PVSelBitField const& sel,
	size_t& v2_unique_values_count,
	tbb::task_group_context* ctxt /* = nullptr */
)
{
	BENCH_START(count_by_with_sel);

	min = std::numeric_limits<size_t>::max();
	max = 0;

	const size_t nreserve = std::sqrt(_nrows);
	tbb::enumerable_thread_specific<count_by_unordered_map_t> count_by_tls([nreserve]{ count_by_unordered_map_t ret; ret.reserve(nreserve); return ret; });
	bool res = visit_column_tbb_sel(col1, [this, &count_by_tls, col2](size_t row, const char* buf1, size_t n1)
	{
		std::string_tbb col1_str(buf1, n1);
		size_t n2;
		const char* buf2 = at_no_cache(row, col2, n2);
		std::string_tbb col2_str(buf2, n2);
		count_by_tls.local()[col1_str][col2_str]++;
	}, sel, ctxt);

	if (!res) {
		return false;
	}

	tbb::enumerable_thread_specific<count_by_unordered_map_t>::iterator it_tls = count_by_tls.begin();
	if (it_tls != count_by_tls.end()) {
		count_by_unordered_map_t& final = *it_tls;
		it_tls++;
		for (; it_tls != count_by_tls.end(); it_tls++) {
			if (ctxt && ctxt->is_group_execution_cancelled()) {
				ret.clear();
				return false; 	// return false if it has been cancelled
			}
			for (auto& v1 : *it_tls) {
				count_by_unique_values_t& final_unique_values = final[v1.first];
				count_by_unique_values_t& tls_unique_values = (*it_tls)[v1.first];
				for (auto& v2 : tls_unique_values) {
					final_unique_values[v2.first] += v2.second;
				}
			}

		}

		size_t v1_count = final.size();
		count_by_t values;
		values.reserve(v1_count);
		for (auto& v1 : final) {
			auto& v2_values = v1.second;

			// v1 count
			count_by_string_count_t v1_count_pair(std::move(v1.first) , v2_values.size());

			size_t size =  v2_values.size();
			min = std::min(min, size);
			max = std::max(max, size);
			values.emplace_back(std::move(v1_count_pair), std::move(v2_values));
		}

		ret = std::move(values);
	}
	else {
		ret.clear();
		return false;
	}

	unique_values_t unique_values_col2;
	uint64_t mi;
	uint64_t ma;
	res = get_unique_values(col2, unique_values_col2, mi, ma, sel, ctxt);
	v2_unique_values_count = unique_values_col2.size();

	BENCH_END(count_by_with_sel, "count_by_with_sel", 1, 1, 1, 1);

	return res;
}

bool PVRush::PVNrawDiskBackend::get_sum(const PVCol col, uint64_t& sum, const PVCore::PVSelBitField& sel, tbb::task_group_context* ctxt)
{
	sum = 0;
	return column_reduction(col, sum, sel, [&](uint64_t& v1, const uint64_t& v2){ v1 += v2; }, ctxt);
}

bool PVRush::PVNrawDiskBackend::get_min(const PVCol col, uint64_t& min, const PVCore::PVSelBitField& sel, tbb::task_group_context* ctxt)
{
	min = std::numeric_limits<uint64_t>::max();
	return column_reduction(col, min, sel, [&](uint64_t& v1, const uint64_t& v2){ v1 = std::min(v1, v2); }, ctxt);
}

bool PVRush::PVNrawDiskBackend::get_max(const PVCol col, uint64_t& max, const PVCore::PVSelBitField& sel, tbb::task_group_context* ctxt)
{
	max = 0;
	return column_reduction(col, max, sel, [&](uint64_t& v1, const uint64_t& v2){ v1 = std::max(v1, v2); }, ctxt);
}

bool PVRush::PVNrawDiskBackend::get_avg(const PVCol col, uint64_t& avg, const PVCore::PVSelBitField& sel, tbb::task_group_context* ctxt)
{
	avg = 0;

	bool ret = get_sum(col, avg, sel, ctxt);
	size_t lines_count = sel.get_number_of_selected_lines_in_range(0, get_number_rows());
	if (lines_count > 0)
		avg /= lines_count;
	else
		ret = false;

	return ret;
}
