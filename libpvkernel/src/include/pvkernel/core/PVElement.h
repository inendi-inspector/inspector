/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVELEMENT_FILE_H
#define PVELEMENT_FILE_H

#include <pvkernel/core/PVAllocators.h>
#include <pvkernel/core/PVBufferSlice.h>
#include <pvkernel/core/PVField.h>

#include <list>

#include <tbb/scalable_allocator.h>

#include <QList>

namespace PVRush
{
class PVNraw;
}

namespace PVCore
{

class PVChunk;

typedef std::list<PVField, PVPreAllocatedListAllocator<PVField, tbb::scalable_allocator<PVField>>>
    list_fields;
// typedef std::list<PVField, tbb::tbb_allocator<PVField> > list_fields;
// typedef std::list<PVField> list_fields;

class PVElement : public PVBufferSlice
{
	friend class PVField;
	friend class PVChunk;

  public:
	PVElement(PVChunk* parent, char* begin, char* end);
	PVElement(PVChunk* parent);

  public:
	PVElement(PVElement const& src);

  public:
	virtual ~PVElement();

  public:
	bool valid() const;
	void set_invalid();
	void set_parent(PVChunk* parent);
	void save_elt_buffer();
	char* get_saved_elt_buffer(size_t& n);
	bool restore_elt_with_saved_buffer();
	void clear_saved_buf();
	PVChunk* chunk_parent();
	chunk_index get_elt_agg_index();
	size_t get_chunk_index() const { return _chunk_index; }

	list_fields& fields();
	list_fields const& c_fields() const;

	buf_list_t& realloc_bufs();

  public:
	// Element allocation and deallocation
	static inline PVElement* construct(PVChunk* parent, char* begin, char* end)
	{
		PVElement* ret = _alloc.allocate(1);
		new (ret) PVElement(parent, begin, end);
		return ret;
	}

	static inline PVElement* construct(PVChunk* parent)
	{
		PVElement* ret = _alloc.allocate(1);
		new (ret) PVElement(parent);
		return ret;
	}

	static inline void free(PVElement* elt)
	{
		_alloc.destroy(elt);
		_alloc.deallocate(elt, 1);
	}

  protected:
	// Set by the parent PVChunk
	void set_chunk_index(size_t i) { _chunk_index = i; }
	void init_fields(void* fields_buf, size_t size_buf);

  private:
	void init(PVChunk* parent);

  protected:
	bool _valid;
	list_fields _fields;
	PVChunk* _parent;
	buf_list_t _reallocated_buffers; // buf_list_t defined in PVBufferSlice.h
	char* _org_buf;
	size_t _org_buf_size;
	size_t _chunk_index;

  private:
	static tbb::scalable_allocator<PVElement> _alloc;
	// static std::allocator<PVElement> _alloc;
};
}

#endif
