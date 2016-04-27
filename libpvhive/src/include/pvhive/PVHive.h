/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef LIBPVHIVE_PVHIVE_H
#define LIBPVHIVE_PVHIVE_H

#include <iostream>
#include <set>
#include <list>
#include <algorithm>
#include <tuple>
#include <iterator>
#include <exception>
#include <unordered_map>

#include <tbb/concurrent_hash_map.h>
#include <tbb/recursive_mutex.h>

#include <pvkernel/core/PVSharedPointer.h>
#include <pvkernel/core/PVFunctionTraits.h>

#include <pvhive/PVObserver.h>
#include <pvhive/PVFuncObserver.h>
#include <pvhive/PVActorBase.h>

namespace PVCore
{
class PVDataTreeObjectBase;
class PVDataTreeObjectWithParentBase;
}

namespace PVHive
{

/**
 * \addtogroup exceptions
 * @{
 * Thrown when trying to register an actor on an unregistered object.
 */
class no_object : public std::exception
{
  public:
	no_object(const char* text) : _text(text) {}

	virtual ~no_object() throw() {}

	virtual const char* what() const throw() { return _text; }

  private:
	const char* _text;
};

/// @}

template <class T> class PVActor;

namespace __impl
{

// declaration of hive_deleter
inline void hive_deleter(void* ptr);

// definition of PVCallReturn
template <typename T, typename F, F f,
          bool is_void =
              std::is_void<typename PVCore::PVTypeTraits::function_traits<F>::result_type>::value,
          bool is_ref = std::is_reference<
              typename PVCore::PVTypeTraits::function_traits<F>::result_type>::value>
class PVCallReturn;

// Specialization for functions returning non-void and non-reference
template <typename T, typename F, F f> class PVCallReturn<T, F, f, false, false>
{
  public:
	typedef typename PVCore::PVTypeTraits::function_traits<F>::result_type result_type;

  public:
	template <typename... P> inline void call(T* object, P&&... params)
	{
		_result = (object->*f)(std::forward<P>(params)...);
	}
	inline result_type result() const { return {std::move(_result)}; }
	inline result_type default_value() const { return {}; }

  private:
	result_type _result;
};

// Specialization for functions returning non-void and reference
template <typename T, typename F, F f> class PVCallReturn<T, F, f, false, true>
{
  public:
	typedef typename PVCore::PVTypeTraits::function_traits<F>::result_type result_type;

  public:
	template <typename... P> inline void call(T* object, P&&... params)
	{
		_result = &(object->*f)(std::forward<P>(params)...);
	}
	inline result_type result() const { return *_result; }
	inline result_type default_value() const { return *_result; }

  private:
	typename std::remove_reference<result_type>::type* _result;
};

// Specialization for functions returning void
template <typename T, typename F, F f> class PVCallReturn<T, F, f, true, false>
{
  public:
	template <typename... P> inline void call(T* object, P&&... params)
	{
		(object->*f)(std::forward<P>(params)...);
	}
	void result() const {}
	void default_value() const {}
};
}

/**
 * @class PVHive
 *
 * @brief The PVHive a system to ease update propagation in programs with
 * interdependant objects.
 *
 * When dealing with changing objects, keeping all dependants objects updated
 * can be complex and can need lots of lines of code to maintain the coherency;
 * as keeping a correct behaviour each time a new functionnality is added.
 *
 * The PVHive also implements a callback-like system to automatize this
 * action-reaction scheme. It is based on 3 concepts:
 * - the hive which is the keeper of list of callbacks for each observable
 *   objects;
 * - the actor which subscribe to notify updates when modifying a given object;
 * - the observer which subscribe to update notification for a given object.
 *
 * To ease objects deletion, each registered object is a shared pointer whose
 * deleter is set by PVHive to automatically unregistere it.
 *
 * There are 2 kinds of notifications:
 * - "refresh": when an object is modified;
 * - "about_to_be_deleted" when an object is unregistered (ie. it will be deleted)
 *   the object is still usable until this callback returns.
 *
 * Note that:
 * - an actor can act on an object which have no observer (as no opened view
 *   for a data set);
 * - an observer can be registered for an object which have no actor (as no
 *   opened property editor for a property).
 *
 * Experimental (and theorical) memory usage per element:
 * - hive    : 568 octets
 * - object  : ~180 octets (120 octets)
 * - property: ~230 octets (128 octets)
 * - observer: ~32 octets (8 octets)
 * - actor   : ~48 octets (8 octets)
 */
class PVHive
{
	struct tbb_hash_ptr
	{
		static inline size_t hash(const void* a) { return (size_t)a; }
		static inline bool equal(const void* a, const void* b) { return a == b; }
	};

  private:
	typedef std::set<PVActorBase*> actors_t;
	typedef std::list<PVObserverBase*> observers_t;
	typedef std::unordered_multimap<void*, PVFuncObserverBase*> func_observers_t;
	typedef std::set<void*> properties_t;

	struct observable_t
	{
		actors_t actors;
		observers_t observers;
		func_observers_t func_observers;
		properties_t properties;
	};

	typedef tbb::concurrent_hash_map<void*, observable_t, tbb_hash_ptr> observables_t;

  private:
	friend void __impl::hive_deleter(void* ptr);
	friend class PVActorBase;
	template <typename T> friend class PVActor;

  public:
	/**
	 * @return a reference on the global PVHive
	 */
	static PVHive& get()
	{
		if (_hive == nullptr) {
			_hive = new PVHive;
		}
		return *_hive;
	}

  public:
	/**
	 * Register an object
	 *
	 * @param object the new managed object
	 */
	template <class T> void register_object(PVCore::PVSharedPtr<T>& object)
	{
		observables_t::accessor acc;

		_observables.insert(acc, PVCore::PVTypeTraits::get_starting_address(object.get()));
		object.set_deleter(&__impl::hive_deleter);
	}

	/**
	 * Register a member variable of an object
	 *
	 * @param object the parent object
	 * @param prop_get: a function returning a reference on object's property
	 *
	 * @attention using a method as prop_get will not compile.
	 */
	template <class T, class F>
	void register_object(PVCore::PVSharedPtr<T>& object, F const& prop_get)
	{
		auto* property = prop_get(*object);

		observables_t::accessor acc;

		// adding property's entry
		_observables.insert(acc, PVCore::PVTypeTraits::get_starting_address(property));

		// create/get object's entry
		_observables.insert(acc, PVCore::PVTypeTraits::get_starting_address(object.get()));
		acc->second.properties.insert(property);
		object.set_deleter(&__impl::hive_deleter);
	}

	/**
	 * Register an actor for an ob,ect
	 *
	 * @param object the managed object
	 * @param actor the actor
	 */
	template <class T> void register_actor(PVCore::PVSharedPtr<T>& object, PVActorBase& actor)
	{
		// an actor must be set for only one object
		assert(actor.get_object() == nullptr);

		observables_t::accessor acc;

		// create/get object's entry
		void* registered_object = PVCore::PVTypeTraits::get_starting_address(object.get());
		_observables.insert(acc, registered_object);

		// create/get object's entry
		acc->second.actors.insert(&actor);
		actor.set_object((void*)object.get(), registered_object);
		object.set_deleter(&__impl::hive_deleter);
	}

	/**
	 * Helper method easily create and register an actor for an object
	 *
	 * @param object the observed object
	 * @return the actor
	 */
	template <class T> PVActor<T>* register_actor(PVCore::PVSharedPtr<T>& object)
	{
		PVActor<T>* actor = new PVActor<T>();
		register_actor(object, *actor);

		return actor;
	}

	/**
	 * Register an observer for an object
	 *
	 * @param object the observed object
	 * @param observer the observer
	 */
	template <class T>
	void register_observer(PVCore::PVSharedPtr<T>& object, PVObserverBase& observer)
	{
		// an observer must be set for only one object
		assert(observer.get_object() == nullptr);

		observables_t::accessor acc;

		// create/get object's entry
		void* registered_object = (void*)PVCore::PVTypeTraits::get_starting_address(object.get());
		_observables.insert(acc, registered_object);

		// observer must not be in _observables[&object].observers
		assert(already_registered(acc->second.observers, observer) == false);

		acc->second.observers.push_back(&observer);
		observer.set_object((void*)object.get(), registered_object);
		object.set_deleter(&__impl::hive_deleter);
	}

	/**
	 * Register an observer for an object
	 *
	 * @param object the observed object
	 * @param observer the observer
	 */
	template <class B, class T, class F, F f>
	void register_func_observer(PVCore::PVSharedPtr<T>& object,
	                            PVFuncObserverTemplatedBase<B, T, F, f>& observer)
	{
		// an observer must be set for only one object
		assert(observer.get_object() == nullptr);

		observables_t::accessor acc;

		// create/get object's entry
		void* registered_object = PVCore::PVTypeTraits::get_starting_address(object.get());
		_observables.insert(acc, registered_object);

#ifdef __GNUG__
// Disable warning for GCC for this line
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
		acc->second.func_observers.insert(std::make_pair((void*)f, &observer));
#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif
		observer.set_object((void*)object.get(), registered_object);
		object.set_deleter(&__impl::hive_deleter);
	}

	/**
	 * Register an observer for a member variable of an object
	 *
	 * @param object the parent object
	 * @param prop_get: a function returning a reference on object's property
	 * @param observer the observer
	 *
	 * @attention using a method as prop_get will not compile.
	 */
	template <class T, class F>
	void register_observer(PVCore::PVSharedPtr<T>& object, F const& prop_get,
	                       PVObserverBase& observer)
	{
		// an observer must be set for only one object
		assert(observer.get_object() == nullptr);

		auto* property = prop_get(*object);

		observables_t::accessor acc;

		// create/get property's entry
		void* registered_object = (void*)PVCore::PVTypeTraits::get_starting_address(property);
		_observables.insert(acc, registered_object);

		// observer must not be in _observables[&property].observers
		assert(already_registered(acc->second.observers, observer) == false);

		// adding observer
		acc->second.observers.push_back(&observer);
		observer.set_object((void*)property, registered_object);

		// adding property
		// create/get object's entry
		_observables.insert(acc, PVCore::PVTypeTraits::get_starting_address(object.get()));
		acc->second.properties.insert(registered_object);
		object.set_deleter(&__impl::hive_deleter);
	}

  public:
	/**
	 * Unregister an actor an notify all dependent observers
	 * that they must stop observing
	 *
	 * @param actor the actor
	 */
	bool unregister_actor(PVActorBase& actor);

	/**
	 * Unregister an observer
	 *
	 * @param observer the observer
	 */
	bool unregister_observer(PVObserverBase& observer);

	/**
	 * Unregister a function observer
	 *
	 * @param observer the function observer
	 */
	bool unregister_func_observer(PVFuncObserverBase& observer, void* f);

  public:
	/**
	 * Write on std::cout a structured view of PVHive content
	 */
	void print() const
	{
		std::cout << "PVHive - " << this << " - content:" << std::endl;
		for (auto it : _observables) {
			std::cout << "    " << it.first << std::endl;

			std::cout << "        actors:" << std::endl;
			for (auto it2 : it.second.actors) {
				std::cout << "            " << it2 << std::endl;
			}

			std::cout << "        observers:" << std::endl;
			for (auto it2 : it.second.observers) {
				std::cout << "            " << it2 << std::endl;
			}

			std::cout << "        properties:" << std::endl;
			for (auto it2 : it.second.properties) {
				std::cout << "            " << it2 << std::endl;
			}
		}
	}

	/**
	 * Returns memory usage
	 */
	size_t memory() const
	{
		size_t s = sizeof(observables_t);

		// memory used by observables_t's entries
		s += _observables.size() * sizeof(observables_t::value_type);

		// memory used by entries values
		for (observables_t::const_iterator it = _observables.begin(); it != _observables.end();
		     ++it) {
			s += it->second.actors.size() * sizeof(actors_t::value_type);
			s += it->second.observers.size() * sizeof(observers_t::value_type);
			s += it->second.properties.size() * sizeof(properties_t::value_type);
		}

		return s;
	}

  protected:
	template <typename T, typename F, F f, typename... P>
	inline typename PVCore::PVTypeTraits::function_traits<F>::result_type
	call_object_interface(T* object, P&&... params)
	{
		typedef PVCore::PVTypeTraits::function_traits<F> ftraits;
		// object must be a valid address
		assert(object != nullptr);
		typename ftraits::arguments_type args;
		args.set_args(std::forward<P>(params)...);

		// This method can be specialized easily for a given function !
		return call_object<F, f>(object, args);
	}

  private:
	/**
	 * Generic call to apply an action on a object
	 *
	 * @param object the managed object
	 * @param params the method parameters
	 */
	template <typename F, F f>
	typename PVCore::PVTypeTraits::function_traits<F>::result_type
	call_object(typename PVCore::PVTypeTraits::function_traits<F>::class_type* object,
	            typename PVCore::PVTypeTraits::function_traits<F>::arguments_type const& args)
	{
		typedef PVCore::PVTypeTraits::function_traits<F> ftraits;
		typedef typename ftraits::class_type class_type;
		// std::cout << "call_object: " << __PRETTY_FUNCTION__ << std::endl;

		// object must be a valid address
		assert(object != nullptr);
		return call_object_default<class_type, F, f>(object, args);
	}

  protected:
	/**
	 * Tell all observers of an object that a change is about to occure
	 *
	 * @param object the observed object
	 */
	template <typename T> inline void about_to_refresh_observers(T const* object)
	{
		// object must be a valid address
		assert(object != nullptr);

		do_about_to_refresh_observers((void*)PVCore::PVTypeTraits::get_starting_address(object));
	}

	/**
	 * Tell all observers of an object that a change has occured
	 *
	 * @param object the observed object
	 */
	template <typename T> inline void refresh_observers(T const* object)
	{
		// object must be a valid address
		assert(object != nullptr);

		// Check if that object still exists
		{
			observables_t::const_accessor acc;
			if (!_observables.find(acc, (void*)object)) {
				return;
			}
		}
		PVCore::PVDataTreeObjectWithParentBase const* dt =
		    PVCore::PVTypeTraits::dynamic_cast_if_possible<
		        PVCore::PVDataTreeObjectWithParentBase const*>(object);
		if (dt) {
			refresh_observers(dt, (void*)PVCore::PVTypeTraits::get_starting_address(object));
		} else {
			do_refresh_observers((void*)PVCore::PVTypeTraits::get_starting_address(object));
		}
	}

	template <typename T> inline void refresh_observers_maybe_recursive(T const* object)
	{
		// object must be a valid address
		assert(object != nullptr);
		PVCore::PVDataTreeObjectWithParentBase const* dt =
		    PVCore::PVTypeTraits::dynamic_cast_if_possible<
		        PVCore::PVDataTreeObjectWithParentBase const*>(object);
		if (dt) {
			refresh_observers_maybe_recursive(
			    dt, const_cast<void*>(
			            PVCore::PVTypeTraits::dynamic_cast_if_possible<const void*>(object)));
		} else {
			do_refresh_observers_maybe_recursive(
			    (void*)PVCore::PVTypeTraits::get_starting_address(object));
		}
	}

	void refresh_observers(PVCore::PVDataTreeObjectWithParentBase const* object, void* obj_refresh);
	void refresh_observers_maybe_recursive(PVCore::PVDataTreeObjectWithParentBase const* object,
	                                       void* obj_refresh);

	/**
	 * Tell all observers of function that a change is about to occure
	 *
	 * @param object the observed object
	 * @param params the f_hiveunction parameters
	 */
	template <typename T, typename F, F f, typename... P>
	void about_to_refresh_func_observers(T const* object, P&&... params)
	{
		process_func_observers<true, T, F, f>(object, std::forward<P>(params)...);
	}

	/**
	 * Tell all observers of function that a change has occured
	 *
	 * @param object the observed object
	 * @param params the function parameters
	 */
	template <typename T, typename F, F f, typename... P>
	void refresh_func_observers(T const* object, P&&... params)
	{
		process_func_observers<false, T, F, f>(object, std::forward<P>(params)...);
	}

	/**
	 * Unregister an object
	 *
	 * @param object the object
	 */
	void unregister_object(void* object);

  private:
	template <bool about, typename T, typename F, F f, typename... P>
	void process_func_observers(T const* object, P&&... params)
	{
		// object must be a valid address
		assert(object != nullptr);

		observables_t::accessor acc;
		if (!_observables.find(acc, (void*)object)) {
			return;
		}

		// Get function observers
		func_observers_t const& fobs(acc->second.func_observers);
		func_observers_t::const_iterator it_fo, it_fo_e;
#ifdef __GNUG__
// Disable warning for GCC for this line
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif
		std::tie(it_fo, it_fo_e) = fobs.equal_range((void*)f);
#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif

		/* RH & AG: to prevent a deadlock when the observers call
		 * PVHive::register_xxxx, the list is duplicated to release
		 * the accessor before calling observers. But it is not safe!
		 *
		 * TODO: find a solution to remove the deadlock *and*
		 * garanty data consistency while iterating
		 */
		std::vector<func_observers_t::mapped_type> func_obs(std::distance(it_fo, it_fo_e));
		size_t len = 0;
		// std::copy does not work when containers iterators differ
		while (it_fo != it_fo_e) {
			func_obs[len] = it_fo->second;
			++it_fo;
			++len;
		}

		acc.release();

		for (size_t i = 0; i < len; ++i) {
			const PVFuncObserverBase* fo = static_cast<const PVFuncObserverBase*>(func_obs[i]);
			const PVFuncObserverSignal<T, F, f>* fo_signal =
			    dynamic_cast<const PVFuncObserverSignal<T, F, f>*>(fo);

			if (fo_signal) {
				typedef typename PVCore::PVTypeTraits::function_traits<F>::arguments_deep_copy_type
				    arguments_deep_copy_type;

				arguments_deep_copy_type* args = new arguments_deep_copy_type;
				args->set_args(params...);

				if (about) {
					fo->do_about_to_be_updated((const void*)args);
				} else {
					fo->do_update((const void*)args);
				}
			} else {
				typedef typename PVCore::PVTypeTraits::function_traits<F>::arguments_type
				    arguments_type;

				arguments_type args;
				args.set_args(params...);

				if (about) {
					fo->do_about_to_be_updated((const void*)&args);
				} else {
					fo->do_update((const void*)&args);
				}
			}
		}
	}

	/**
	 * Apply an action on a object and propagate the change event
	 *
	 * @param object the managed object
	 * @param params the method parameters
	 */
	template <typename T, typename F, F f>
	inline typename PVCore::PVTypeTraits::function_traits<F>::result_type call_object_default(
	    T* object, typename PVCore::PVTypeTraits::function_traits<F>::arguments_type const& args)
	{
		// Unpack arguments
		return do_call_object_default<T, F, f>(
		    object, args, typename PVCore::PVTypeTraits::gen_seq_n<
		                      (int)PVCore::PVTypeTraits::function_traits<F>::arity>::type());
	}

	template <typename T, typename F, F f, int... S>
	inline typename PVCore::PVTypeTraits::function_traits<F>::result_type do_call_object_default(
	    T* object, typename PVCore::PVTypeTraits::function_traits<F>::arguments_type const& args,
	    PVCore::PVTypeTraits::seq_n<S...>)
	{
		return call_object_default<T, F, f>(object, std::get<S>(args)...);
	}

	template <typename T, typename F, F f, typename... P>
	typename PVCore::PVTypeTraits::function_traits<F>::result_type
	call_object_default(T* object, P&&... params)
	{
		__impl::PVCallReturn<T, F, f> caller;

		if (object != nullptr) {

			observables_t::const_accessor acc;

			if (_observables.find(acc, (void*)object)) {
				/* TODO: be sure releasing acc is safe
				 * RH: I think we may have the same problem than in
				 * ::process_func_observers
				 */
				acc.release();

				// Pre-call events
				about_to_refresh_observers(object);
				about_to_refresh_func_observers<T, F, f>(object, std::forward<P>(params)...);

				// Object call
				caller.call(object, std::forward<P>(params)...);

				// Post-call events
				refresh_observers(object);
				refresh_func_observers<T, F, f>(object, std::forward<P>(params)...);

				return caller.result();
			}
		}

		return caller.default_value();
	}

	/**
	 * Propagate about_to_be_refreshed event to all observers
	 *
	 * @param object the object which has been modified
	 */
	void do_about_to_refresh_observers(void* object);

	/**
	 * Propagate refresh event to all observers
	 *
	 * @param object the object which has been modified
	 */
	void do_refresh_observers(void* object);
	void do_refresh_observers_maybe_recursive(void* object);

  private:
	// private to secure the singleton
	PVHive() {}
	~PVHive() {}
	PVHive(const PVHive&) {}
	PVHive& operator=(const PVHive&) { return *this; }

  private:
	/**
	 * Tests if observer is already registered
	 *
	 * @param observers the container
	 * @param observer the observer to find
	 */
	bool already_registered(const observers_t& observers, const PVObserverBase& observer) const
	{
		return (std::find(observers.begin(), observers.end(), &observer) != observers.end());
	}

  private:
	static PVHive* _hive;

	observables_t _observables;
};

// Helper function
inline PVHive& get()
{
	return PVHive::get();
}

namespace __impl
{

// definition of hive_deleter
inline void hive_deleter(void* ptr)
{
	PVHive::get().unregister_object(ptr);
}
}
}

#endif // LIBPVHIVE_PVHIVE_H
