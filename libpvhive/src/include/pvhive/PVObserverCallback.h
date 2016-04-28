/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef LIBPVHIVE_PVOBSERVERCALLBACK_H
#define LIBPVHIVE_PVOBSERVERCALLBACK_H

#include <pvhive/PVObserver.h>

namespace PVHive
{

/**
 * @class PVObserverCallback
 *
 * A template class to specify observers using functions instead
 * of methods.
 *
 * Used functions can be of any kind: lambda, static defined,
 * std::function, boost::function, etc.
 *
 * All subclasses must implements PVObserverBase::refresh() and
 * PVObserverBase::about_to_be_deleted().
 */
template <class T, class AboutToBeRefreshedF, class RefreshF, class DeleteF>
class PVObserverCallback : public PVObserver<T>
{
  public:
	PVObserverCallback() {}

	PVObserverCallback(AboutToBeRefreshedF const& atbr, RefreshF const& r, DeleteF const& d)
	    : _about_to_be_refreshed_cb(atbr), _refresh_cb(r), _delete_cb(d)
	{
	}

  protected:
	virtual void about_to_be_refreshed() { _about_to_be_refreshed_cb(PVObserver<T>::get_object()); }

	virtual void refresh() { _refresh_cb(PVObserver<T>::get_object()); }

	virtual void about_to_be_deleted() { _delete_cb(PVObserver<T>::get_object()); }

  private:
	AboutToBeRefreshedF _about_to_be_refreshed_cb;
	RefreshF _refresh_cb;
	DeleteF _delete_cb;
};

/**
 * Helper function a create a PVObserverCallback without dealing
 * with the PVObserverCallback class itself.
 */
template <class T, class AboutToBeRefreshedF, class RefreshF, class DeleteF>
PVObserverCallback<T, AboutToBeRefreshedF, RefreshF, DeleteF>
create_observer_callback(AboutToBeRefreshedF const& atbr, RefreshF const& r, DeleteF const& d)
{
	return PVObserverCallback<T, AboutToBeRefreshedF, RefreshF, DeleteF>(atbr, r, d);
}

template <class T, class AboutToBeRefreshedF, class RefreshF, class DeleteF>
PVObserver_p<T>
create_observer_callback_heap(AboutToBeRefreshedF const& atbr, RefreshF const& r, DeleteF const& d)
{
	return PVObserver_p<T>(
	    new PVObserverCallback<T, AboutToBeRefreshedF, RefreshF, DeleteF>(atbr, r, d));
}
}

#endif // LIBPVHIVE_PVOBSERVERCALLBACK_H
