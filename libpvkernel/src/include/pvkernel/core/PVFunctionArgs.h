/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCORE_PVFUNCTION_ARGS_H
#define PVCORE_PVFUNCTION_ARGS_H

#include <pvkernel/core/PVArgument.h>

#include <QString>

#include <exception>

namespace PVCore
{

/*! \brief Exception class if a filter argument is missing during PVFilterFunction::set_args()
 */
class PVFunctionArgumentMissing : public std::exception
{
  public:
	PVFunctionArgumentMissing(QString const& arg) throw() : std::exception()
	{
		_what = QString("Argument %1 missing").arg(arg);
	}
	~PVFunctionArgumentMissing() throw(){};
	virtual const char* what() const throw() { return qPrintable(_what); };

  protected:
	QString _what;
};

class PVFunctionArgsBase
{

  public:
	PVFunctionArgsBase(PVArgumentList const& args = PVArgumentList()) : _args(args), _def_args(args)
	{
	}

	virtual ~PVFunctionArgsBase() {}

  public:
	virtual const PVArgumentList& get_args() const { return _args; }
	virtual void set_args(PVArgumentList const& args)
	{
		PVArgumentList::const_iterator it, ite;
		it = _def_args.begin();
		ite = _def_args.end();
		for (; it != ite; it++) {
			// If that default argument is not present in the given list
			if (args.find(it->key()) == args.end()) {
				// An exception is thrown
				throw PVFunctionArgumentMissing(it->key());
			}
		}
		_args = args;
	}

	PVArgumentList const& get_default_args() const { return _def_args; }

	PVArgumentList get_args_for_preset() const
	{
		PVArgumentList args = get_args();
		PVCore::PVArgumentKeyList keys = get_args_keys_for_preset();

		// Get rid of unwanted args
		PVArgumentList filtered_args;
		foreach (PVCore::PVArgumentKey key, keys) {
			PVArgumentList::const_iterator it = args.find(key);
			if (it != args.end()) {
				filtered_args[it->key()] = it->value();
			}
		}

		return filtered_args;
	}
	virtual PVCore::PVArgumentKeyList get_args_keys_for_preset() const
	{
		return get_default_args().keys();
	}
	void set_args_from_preset(PVArgumentList const& args)
	{
		PVArgumentList preset_args = get_args_for_preset();
		PVArgumentList::const_iterator it;
		for (it = args.begin(); it != args.end(); it++) {
			if (preset_args.contains(it->key())) {
				_args[it->key()] = it->value();
			}
		}
	}

  protected:
	PVArgumentList _args;
	PVArgumentList _def_args;
};

// FIXME: is this really useful ?!
template <class F>
class PVFunctionArgs : public PVFunctionArgsBase
{
  public:
	typedef F func_type;

  public:
	PVFunctionArgs(PVArgumentList const& args = PVArgumentList()) : PVFunctionArgsBase(args) {}

  public:
	virtual func_type f() = 0;
};
}

#define DEFAULT_ARGS_FUNC(T) PVCore::PVArgumentList T::default_args()

#define CLASS_FUNC_ARGS_NOPARAM(T)                                                                 \
  public:                                                                                          \
	static PVCore::PVArgumentList default_args() { return PVCore::PVArgumentList(); }

#define CLASS_FUNC_ARGS_PARAM(T)                                                                   \
  public:                                                                                          \
	static PVCore::PVArgumentList default_args();

#endif
