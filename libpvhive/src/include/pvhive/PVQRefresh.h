/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef LIBPVHIVE_PVQREFRESH_H
#define LIBPVHIVE_PVQREFRESH_H

#include <pvhive/PVRefreshSignal.h>

namespace PVHive
{

namespace __impl
{

/**
 * @class PVQRefresh
 *
 * Base (and non template) class to manage events using Qt's signal/slot
 * mechanism.
 *
 * This class has to be subclassed to be used.
 *
 * the slots to implement are do_about_to_be_refreshed(PVHive::PVObserverBase *),
 * do_refresh(PVHive::PVObserverBase *), and do_about_to_be_deleted(PVHive::PVObserverBase *).
 */
class PVQRefresh : public PVRefreshSignal
{
	Q_OBJECT

  public:
	PVQRefresh(QObject* parent = nullptr) : PVRefreshSignal(parent)
	{
		connect_about_to_be_refreshed(this,
		                              SLOT(do_about_to_be_refreshed(PVHive::PVObserverBase*)));
		connect_refresh(this, SLOT(do_refresh(PVHive::PVObserverBase*)));
		connect_about_to_be_deleted(this, SLOT(do_about_to_be_deleted(PVHive::PVObserverBase*)));
	}

  protected slots:
/* Qt's signals/slots mechanism can not work properly with namespaces; leading
 * to run-time errors of type "Incompatible sender/receiver arguments" or
 * "No such signal": the signals use implicit namespaces prefix (otherwise it
 * does not compile) and the slots use explicit namespaces prefix. So that the
 * MOC's internal strcmp fails when comparing signals/slots signatures.
 *
 * To get round, the symbol Q_MOC_RUN has to be used to test if moc is running
 * or not. See http://qt-project.org/doc/qt-4.8/moc.html
 */
#ifdef Q_MOC_RUN
	virtual void do_about_to_be_refreshed(PVHive::PVObserverBase* o){};
	virtual void do_refresh(PVHive::PVObserverBase* o){};
	virtual void do_about_to_be_deleted(PVHive::PVObserverBase* o){};
#else
	virtual void do_about_to_be_refreshed(PVObserverBase* /*o*/){};
	virtual void do_refresh(PVObserverBase* /*o*/){};
	virtual void do_about_to_be_deleted(PVObserverBase* /*o*/){};
#endif
};
}
}

#endif // LIBPVHIVE_PVQREFRESH_H
