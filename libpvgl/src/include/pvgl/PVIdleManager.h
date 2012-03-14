//! \file PVIdleManager.h
//! $Id: PVIdleManager.h 2975 2011-05-26 03:54:42Z aguinet $
//! Copyright (C) Sébastien Tricaud 2009-2011
//! Copyright (C) Philippe Saade 2009-2011
//! Copyright (C) Picviz Labs 2011

#ifndef LIBPVGL_PVIDLE_MANAGER_H
#define LIBPVGL_PVIDLE_MANAGER_H

#include <pvkernel/core/general.h>
#include <tbb/tick_count.h>

namespace PVGL {
class PVDrawable;

/**
* \enum PVIdleTaskKinds
*/
enum PVIdleTaskKinds {
	IDLE_REDRAW_LINES,
	IDLE_REDRAW_ZOMBIE_LINES,
	IDLE_REDRAW_MAP_LINES,
	IDLE_REDRAW_ZOMBIE_MAP_LINES,
};

/**
 *
 */
class LibGLDecl PVIdleManager {
	public:
	class IdleTask {
		PVGL::PVDrawable      *drawable; //!<
		PVGL::PVIdleTaskKinds  kind;     //!<
		int                    first_axis;
		int                    last_axis;
		public:
		/**
		 * Default constructor.
		 */
		IdleTask():drawable(0),kind(IDLE_REDRAW_LINES),first_axis(-1),last_axis(-1) {}
		/**
		*  Constructor.
		*/
		IdleTask(PVGL::PVDrawable *drawable_, PVGL::PVIdleTaskKinds kind_, int first_axis_, int last_axis_):drawable(drawable_),kind(kind_),first_axis(first_axis_),last_axis(last_axis_)
		{
		}

		/**
		 *
		 */
		PVGL::PVDrawable *get_drawable()const{return drawable;}
		/**
		 *
		 */
		PVGL::PVIdleTaskKinds get_kind()const{return kind;}
		/**
		*
		*/
		bool operator==(const IdleTask&t)const
			{
				return drawable == t.drawable && kind == t.kind && first_axis == t.first_axis && last_axis == t.last_axis;
			}

		/**
		*
		*/
		bool operator<(const IdleTask&t)const
			{
				if (drawable > t.drawable ||
					   	(drawable == t.drawable && kind > t.kind) ||
						(drawable == t.drawable && kind == t.kind && first_axis > t.first_axis) ||
						(drawable == t.drawable && kind == t.kind && first_axis == t.first_axis && last_axis > t.last_axis)) {
					return true;
				}
				return false;
			}
	};
	private:
	struct IdleValue {
		int  nb_lines;  //!<
		int  drawn_lines;
		bool removed;   //!<
		tbb::tick_count time_start;
		IdleValue(int nb_lines_ = 25000, bool removed_ = false):nb_lines(nb_lines_),removed(removed_){ time_start = tbb::tick_count::now(); drawn_lines = 0; }
	};

	std::map<IdleTask, IdleValue> tasks; //!<
public:
	/**
	 * Get the first task of a given kind for the given drawable.
	 *
	 * @param drawable The drawable
	 * @param kind     The kind...
	 * @param task     A place-holder for the task.
	 *
	 * @return         true if such a task has been found, false otherwise.
	 */
	bool get_first_task(PVGL::PVDrawable *drawable, PVGL::PVIdleTaskKinds kind, PVGL::PVIdleManager::IdleTask &task) const;
	/**
	*
	* @param drawable
	* @param kind
	*
	* @return
	*/
	bool task_exists(PVGL::PVDrawable *drawable, PVGL::PVIdleTaskKinds kind) const;

	/**
	*
	*/
	void callback();

	/**
	* @param drawable
	* @param kind
	*/
	void new_task(PVGL::PVDrawable *drawable, PVGL::PVIdleTaskKinds kind, int first, int last);

	/**
	*
	* @param drawable
	* @param kind
	*
	* @return
	*/
	int get_number_of_lines(PVGL::PVDrawable *drawable, PVGL::PVIdleTaskKinds kind);

	/**
	* Remove a task.
	*
	* @param task   The task to remove (to mark as removed, really).
	*/
	void remove_task(const PVGL::PVIdleManager::IdleTask &task);

	/**
	*
	* @param drawable
	*/
	void remove_tasks_for_drawable(PVGL::PVDrawable *drawable);
};
}
#endif
