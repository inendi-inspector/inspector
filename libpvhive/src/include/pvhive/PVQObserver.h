/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef LIBPVHIVE_PVQOBSERVER_H
#define LIBPVHIVE_PVQOBSERVER_H

#include <QObject>

#include <pvhive/PVObserver.h>
#include <pvhive/PVQRefresh.h>

namespace PVHive
{

/**
 * @class PVObserverSignal
 *
 * A template class to specify observers which use Qt's signal/slot
 * mechanism.
 *
 * the slots to implement are do_refresh(PVHive::PVObserverBase *), and
 * do_about_to_be_deleted(PVHive::PVObserverBase *).
 */
template <class T> class PVQObserver : public __impl::PVQRefresh, public PVObserver<T>
{
  public:
	PVQObserver(QObject* parent) : __impl::PVQRefresh(parent) {}

  protected:
	virtual void about_to_be_refreshed() { emit_about_to_be_refreshed_signal(this); }

	virtual void refresh() { emit_refresh_signal(this); }

	virtual void about_to_be_deleted() { emit_about_to_be_deleted_signal(this); }
};
}

#endif // LIBPVHIVE_PVQOBSERVER_H
