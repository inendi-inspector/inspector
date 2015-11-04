/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_PVUNICODESTRING16_H
#define PVCORE_PVUNICODESTRING16_H

#include <pvbase/export.h>
#include <pvkernel/core/PVPythonClassDecl.h>

namespace PVCore {

class PVBufferSlice;
class PVUnicodeString16;
class PVUnicodeString16HashNoCase;

}

#include <pvkernel/core/general.h>
#include <pvkernel/core/PVBufferSlice.h>

namespace PVCore {
/*! \brief Defines a read-only UTF16 string object.
 *  Design for performance, and does not use any explicit or implicit sharing as opposed to
 *  QString objects.
 *
 *  It is forced to be aligned on 4-byte (instead of 8-byte in 64 bits) for memory consumption issues.
 *  TODO: check the impact on performances !
 *
 *  These objects are constructed from PVBufferSlice.
 */
#pragma pack(push)
#pragma pack(4)
class PVUnicodeString16
{
public:
	typedef uint16_t utf_char;
public:
	// Inline constructors
	PVUnicodeString16(PVBufferSlice const& buf)
	{
		set_from_slice(buf);
	}

	PVUnicodeString16()
	{
		_buf = NULL;
		_len = 0;
	}

	/*
	PVUnicodeString16(PVUnicodeString16 const& other)
	{
		_buf = other._buf;
		_len = other._len;
	}*/

	PVUnicodeString16(const utf_char* buf, size_t len):
		_buf(buf),
		_len(len)
	{
		assert(buf);
	}
public:
	// == Conversions ==
	double to_double(bool& ok) const;
	float to_float(bool& ok) const;

	template <typename T>
	T to_integer() const
	{
		// TODO: implement this !
		T ret = 0;
		return ret;
	}

	// == Comparaisons ==
	// By default, memory-based comparaison are made
	bool operator==(const PVUnicodeString16& o) const;
	bool operator!=(const PVUnicodeString16& o) const;
	bool operator<(const PVUnicodeString16& o) const;
	int compare(const PVUnicodeString16& o) const;
	int compare(const char* str) const;
	int compareNoCase(const PVUnicodeString16& o) const;

	// == Data access ==
	inline const utf_char* buffer() const { return _buf; }
	inline uint32_t size() const { return _len; };
	inline uint32_t len() const { return _len; };
	inline QString get_qstr() const
	{
		/*
		if (_qstr.isNull()) {
			_qstr.setRawData((QChar*) _buf, _len);
		}
		return _qstr;
		*/
		return QString::fromRawData((QChar*) _buf, _len);
	}
	inline QString get_qstr_copy() const
	{
		// This will make a deep copy!
		return QString((const QChar*) _buf, _len);
	}
	inline QString& get_qstr(QString& s) const
	{
		s.setRawData((QChar*) _buf, _len);
		return s;
	}

	// == Data set ==
	inline void set_from_slice(PVBufferSlice const& buf)
	{
		_buf = (utf_char*) buf.begin();
		_len = buf.size()/sizeof(utf_char);
	}
	/*
	PVUnicodeString16& operator=(const PVUnicodeString16& other) 
	{
		_buf = other._buf;
		_len = other._len;
		_qstr.clear();
		return *this;
	}
	*/

private:
	const utf_char* _buf;
	uint32_t _len;
	//mutable QString _qstr;

	PYTHON_EXPOSE()
};
#pragma pack(pop)

class PVUnicodeString16HashNoCase
{
public:
	PVUnicodeString16HashNoCase(PVUnicodeString16 const& str): _str(str) { }
public:
	inline PVUnicodeString16 const& str() const { return _str; }
	inline bool operator==(const PVUnicodeString16HashNoCase& o) const { return _str.compareNoCase(o.str()) == 0; }
private:
	PVUnicodeString16 const& _str;
};

}

#endif
