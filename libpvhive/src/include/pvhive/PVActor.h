/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef LIBPVHIVE_PVACTOR_H
#define LIBPVHIVE_PVACTOR_H

#include <pvkernel/core/PVFunctionTraits.h>

#include <pvhive/PVHive.h>
#include <pvhive/PVActorBase.h>

namespace PVHive
{

/**
 * @class PVActor
 *
 * A template class to specify actor on a given type/class.
 */
template <class T> class PVActor : public PVActorBase
{
	friend class PVHive;

  public:
	typedef T type;

  public:
	PVActor() {}

	/**
	 * Invoke an actor's method on its object
	 *
	 * @param params a variadic for method parameters
	 *
	 * @throw no_object
	 */
	template <typename F, F f, typename... P>
	typename PVCore::PVTypeTraits::function_traits<F>::result_type call(P&&... params)
	{
		T* object = (T*)get_object();
		if (object != nullptr) {
			return PVHive::get().call_object_interface<T, F, f>(object, std::forward<P>(params)...);
		} else {
			throw no_object("using an actor on a nullptr object");
		}
		return typename PVCore::PVTypeTraits::function_traits<F>::result_type();
	}
};

// Helper macro to hide the C++ decltype verbosity
#define PVACTOR_CALL(Actor, Method, Param...) (Actor).call<decltype(Method), Method>(Param)
}

#include <pvhive/waxes/waxes.h>

#endif // LIBPVHIVE_PVACTOR_H
