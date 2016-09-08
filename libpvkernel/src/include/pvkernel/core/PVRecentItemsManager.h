/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVRECENTITEMSMANAGER_H_
#define PVRECENTITEMSMANAGER_H_

#include <pvkernel/rush/PVInputType.h>
#include <pvkernel/rush/PVSourceDescription.h>

#include <QSettings>
#include <QStringList>
#include <QList>
#include <QVariant>

#include <sigc++/sigc++.h>

namespace PVRush
{
class PVFormat;
}

Q_DECLARE_METATYPE(PVRush::PVSourceDescription)
Q_DECLARE_METATYPE(PVRush::PVFormat)

namespace PVCore
{

/**
 * \class PVRecentItemsManager
 *
 * \note This class is a singleton managing the data recently used by the application.
 */
class PVRecentItemsManager
{
  public:
	typedef QList<QVariant> variant_list_t;

  public:
	// List of all available categories of recent items
	enum Category {
		FIRST = 0,

		PROJECTS = FIRST,
		SOURCES,
		USED_FORMATS,
		EDITED_FORMATS,
		SUPPORTED_FORMATS,

		LAST
	};

	static PVRecentItemsManager& get()
	{
		static PVRecentItemsManager recent_items_manager;
		return recent_items_manager;
	}

	/*! \brief Return the serializable name of a given category.
	 */
	const QString get_key(Category category) { return _recents_items_keys[category]; }

	/*! \brief Add an item (path) for a given category.
	 */
	void add(const QString& item_path, Category category);

	/*! \brief Add a source item for a given category.
	 */
	void add_source(PVRush::PVSourceCreator_p source_creator_p,
	                const PVRush::PVInputType::list_inputs& inputs,
	                const PVRush::PVFormat& format);

	/*! \brief Return a source description from the settings current group.
	 */
	const variant_list_t get_list(Category category);

	void clear(Category category, QList<int> indexes = QList<int>());

  private:
	PVRush::PVSourceDescription deserialize_source_description();

	/*! \brief Get the best source timestamp to replace (oldest, matching the same source
	 * description or 0).
	 *  \return The timestamp to replace or 0 if the list count is less than the maximum allowed
	 * items.
	 */
	uint64_t get_source_timestamp_to_replace(const PVRush::PVSourceDescription& source_description);

	void clear_missing_files();

  private:
	/*! \brief Return a list of recent items of a given category as a list of QString QVariant.
	 */
	const variant_list_t items_list(Category category) const;

	/**
	 * Remove value in recent file when pointed file is missing.
	 */
	void remove_missing_files(Category category);

	/*! \brief Return the recent sources description as a list of QVariant.
	 */
	// FIXME : This function is not const as it required group to list sources and Qt doesn't
	// provide this interface
	const variant_list_t sources_description_list();
	void remove_invalid_source();

	/*! \brief Return the supported formats as a list of QVariant.
	 */
	const variant_list_t supported_format_list() const;

  private:
	PVRecentItemsManager();
	PVRecentItemsManager(const PVRecentItemsManager&);
	PVRecentItemsManager& operator=(const PVRecentItemsManager&);

  public:
	sigc::signal<void, Category> _add_item;

  private:
	QSettings _recents_settings;
	const int64_t _max_recent_items = 30;
	const QStringList _recents_items_keys = {"recent_projects", "recent_sources",
	                                         "recent_used_formats", "recent_edited_formats",
	                                         "supported_formats"};
};
}

#endif /* PVRECENTITEMSMANAGER_H_ */
