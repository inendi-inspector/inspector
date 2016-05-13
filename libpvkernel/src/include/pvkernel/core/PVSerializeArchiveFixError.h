/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_PVSERIALIZEARCHIVEFIXERROR
#define PVCORE_PVSERIALIZEARCHIVEFIXERROR

#include <pvkernel/core/PVTypeTraits.h>

#include <memory>

#include <QVariant>

namespace PVCore
{

class PVSerializeObject;
class PVSerializeArchiveError;

class PVSerializeArchiveFixError
{
  public:
	PVSerializeArchiveFixError(PVSerializeObject& so,
	                           std::shared_ptr<PVSerializeArchiveError> const& ar_err)
	    : _so(so), _ar_err(ar_err)
	{
	}

  public:
	void error_fixed();
	PVSerializeArchiveError const& exception() const { return *_ar_err; }

	template <class T>
	bool exception_of_type()
	{
		return dynamic_cast<typename PVTypeTraits::pointer<T>::type>(_ar_err.get()) != NULL;
	}

	template <class T>
	T* exception_as()
	{
		return dynamic_cast<typename PVTypeTraits::pointer<T>::type>(_ar_err.get());
	}

  protected:
	PVSerializeObject& _so;
	std::shared_ptr<PVSerializeArchiveError> _ar_err;
};

class PVSerializeArchiveFixAttribute : public PVSerializeArchiveFixError
{
  public:
	PVSerializeArchiveFixAttribute(PVSerializeObject& so,
	                               std::shared_ptr<PVSerializeArchiveError> const& ar_err,
	                               QString const& attribute)
	    : PVSerializeArchiveFixError(so, ar_err), _attribute(attribute)
	{
	}

  public:
	void fix(QVariant const& v);

  private:
	QString _attribute;
};

typedef std::shared_ptr<PVSerializeArchiveFixError> PVSerializeArchiveFixError_p;
}

#endif
