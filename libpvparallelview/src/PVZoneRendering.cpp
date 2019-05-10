/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvparallelview/PVZonesProcessor.h>
#include <pvparallelview/PVZoneRendering.h>
#include <QMetaObject>

#include <QThread>

/******************************************************************************
 *
 * PVParallelView::PVZoneRenderingBase::finished
 *
 *****************************************************************************/
void PVParallelView::PVZoneRendering::finished(p_type const& this_sp)
{
	// Having `this_sp' as parameter allows not to have an internal weak_ptr in
	// PVZoneRenderingBase
	assert(this_sp.get() == this);

	{
		boost::lock_guard<boost::mutex> lock(_wait_mut);
		_finished = true;

		// Cancellation state may have been changed in the middle, but the listeners
		// are aware of that!
		// We need to be coherent according to the state at the beggining of this
		// function.
		if (_qobject_finished_success != nullptr && !_should_cancel) {
			assert(QThread::currentThread() != _qobject_finished_success->thread());
			const PVZoneID zone_id = get_zone_id();
			QMetaObject::invokeMethod(
			    _qobject_finished_success, _qobject_slot, Qt::QueuedConnection,
			    Q_ARG(PVParallelView::PVZoneRendering_p, this_sp), Q_ARG(PVZoneID, zone_id));
		}
	}

	_wait_cond.notify_all();

	// Reread the cancellation state here, as it may have changed in the middle
	// but we still need to launch the job.
	// The "launch()" method will atomically check that the job is launched
	// only once.
	if (should_cancel()) {
		_job_after_canceled.launch();
	}
}

void PVParallelView::PVZoneRendering::cancel_and_add_job(PVZonesProcessor& zp, p_type const& zr)
{
	_job_after_canceled.zr = zr;
	_job_after_canceled.zp = &zp;
	if (cancel() || finished()) {
		// We are already canceled or finished, so just launch the job.
		_job_after_canceled.launch();
	}
}

void PVParallelView::PVZoneRendering::next_job::launch()
{
	PVZonesProcessor* const zp_ = zp.fetch_and_store(nullptr);
	if (zp_) {
		zp_->add_job(zr);
		zr.reset();
	}
}
