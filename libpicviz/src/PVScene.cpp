/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/hash_sharedptr.h>
#include <pvkernel/core/PVSerializeArchiveOptions.h>
#include <pvkernel/core/PVSerializeArchiveZip.h>
#include <picviz/PVRoot.h>
#include <picviz/PVScene.h>
#include <picviz/PVSource.h>
#include <picviz/PVView.h>

#include <QFileInfo>


#define ARCHIVE_SCENE_DESC (QObject::tr("Workspace"))
/******************************************************************************
 *
 * Picviz::PVScene::PVScene
 *
 *****************************************************************************/
Picviz::PVScene::PVScene(QString scene_path) :
	_last_active_src(nullptr),
	_path(scene_path)
{
	QFileInfo info(_path);
	_name = info.fileName();
}

/******************************************************************************
 *
 * Picviz::PVScene::~PVScene
 *
 *****************************************************************************/
Picviz::PVScene::~PVScene()
{
	remove_all_children();
	PVLOG_DEBUG("In PVScene destructor\n");
	PVRoot* root = get_parent<PVRoot>();
	if (root) {
		root->scene_being_deleted(this);
	}
}

Picviz::PVScene::list_sources_t Picviz::PVScene::get_sources(PVRush::PVInputType const& type) const
{
	children_t const& sources = get_children();
	list_sources_t ret;
	for (PVSource_p const& src: sources) {
		if (*src->get_input_type() == type) {
			ret.push_back(src.get());
		}
	}
	return ret;
}

Picviz::PVSource* Picviz::PVScene::current_source()
{
	PVSource* cur_src = get_parent<PVRoot>()->current_source();
	if (cur_src->get_parent<PVScene>() == this) {
		return cur_src;
	}
	return nullptr;
}

Picviz::PVSource const* Picviz::PVScene::current_source() const
{
	PVSource const* cur_src = get_parent<PVRoot>()->current_source();
	if (cur_src->get_parent<PVScene>() == this) {
		return cur_src;
	}
	return nullptr;
}

Picviz::PVView* Picviz::PVScene::current_view()
{
	PVView* cur_view = get_parent<PVRoot>()->current_view();
	if (cur_view->get_parent<PVScene>() == this) {
		return cur_view;
	}
	return nullptr;
}

Picviz::PVView const* Picviz::PVScene::current_view() const
{
	PVView const* cur_view = get_parent<PVRoot>()->current_view();
	if (cur_view->get_parent<PVScene>() == this) {
		return cur_view;
	}
	return nullptr;
}

PVRush::PVInputType::list_inputs_desc Picviz::PVScene::get_inputs_desc(PVRush::PVInputType const& type) const
{
	children_t const& sources = get_children();
	QSet<PVRush::PVInputDescription_p> ret_set;
	for (PVSource_p const& src: sources) {
		if (*src->get_input_type() == type) {
			ret_set.unite(src->get_inputs().toSet());
		}
	}
	return ret_set.toList();
}

/*Picviz::PVView::id_t Picviz::PVScene::get_new_view_id() const
{
	return get_children<PVView>().size();
}

void Picviz::PVScene::set_views_id()
{
	std::multimap<PVView::id_t, PVView*> map_views;
	for (auto view : get_children<PVView>()) {
		map_views.insert(std::make_pair(view->get_view_id(), view.get()));
	}
	PVView::id_t cur_id = 0;
	std::multimap<PVView::id_t, PVView*>::iterator it;
	for (it = map_views.begin(); it != map_views.end(); it++) {
		it->second->set_view_id(cur_id);
		cur_id++;
	}
}

QColor Picviz::PVScene::get_new_view_color() const
{
	return QColor(_view_colors[(get_new_view_id()-1) % (sizeof(_view_colors)/sizeof(QRgb))]);
}*/

void Picviz::PVScene::child_added(PVSource& /*src*/)
{
	// For information, from PVScene.h:
	// typedef std::map<PVRush::PVInputType, PVRush::PVInputType::list_inputs> hash_type_sources_t;
	// hash_type_sources_t _sources;
	
#if 0
	PVRush::PVInputType::list_inputs_desc& inputs(_sources[*(src.get_input_type())]);

	// Add sources' inputs to our `inputs' if they do not exist yet
	PVRush::PVInputType::list_inputs_desc src_inputs = src.get_inputs();
	PVRush::PVInputType::list_inputs_desc::const_iterator it;
	for (it = src_inputs.begin(); it != src_inputs.end(); it++) {
		if (!inputs.contains(*it)) {
			inputs.push_back(*it);
		}
	}
#endif

	get_parent<PVRoot>()->set_views_id();
}

void Picviz::PVScene::child_about_to_be_removed(PVSource&)
{
#if 0
	// Remove this source's inputs if they are no longer used by other sources
	PVRush::PVInputType::list_inputs>& type_srcs = _sources[*(src->get_input_type())];
	list_sources_t& list_srcs(type_srcs.first);

	list_sources_t::iterator it;
	for (it = list_srcs.begin(); it != list_srcs.end(); it++) {
		if (it->get() == src) {
			list_srcs.erase(it);
			set_views_id();
			return;
		}
	}
#endif
}

QList<PVRush::PVInputType_p> Picviz::PVScene::get_all_input_types() const
{
	QList<PVRush::PVInputType_p> ret;
	for (PVSource_p const& src: get_children()) {
		PVRush::PVInputType_p in_type = src->get_input_type();
		bool found = false;
		for (PVRush::PVInputType_p const& known_in_t: ret) {
			if (known_in_t->registered_id() == in_type->registered_id()) {
				found = true;
				break;
			}
		}
		if (!found) {
			ret.push_back(in_type);
		}
	}
	return ret;
}

void Picviz::PVScene::add_source(PVSource_p const& src)
{
	add_child(src);
}

Picviz::PVSource_p Picviz::PVScene::add_source_from_description(const PVRush::PVSourceDescription& descr)
{
	PVSource_p src_p(
		shared_from_this(),
		descr.get_inputs(),
		descr.get_source_creator(),
		descr.get_format()
	);

	return src_p;
}

void Picviz::PVScene::serialize_read(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t v)
{
	// Get the list of input types
	QStringList input_types;
	so.list_attributes("types", input_types);
	so.attribute("name", _name);

	if (input_types.size() == 0) {
		// No input types, thus no sources, thus nothing !
		return;
	}

	// Temporary list of input descriptions
	PVRush::PVInputType::list_inputs_desc tmp_inputs;
	for (int i = 0; i < input_types.size(); i++) {
		// Get the input type lib object
		QString const& type_name = input_types.at(i);
		PVRush::PVInputType_p int_lib = LIB_CLASS(PVRush::PVInputType)::get().get_class_by_name(type_name);

		// Get the inputs list object for that input type
		PVRush::PVInputType::list_inputs_desc inputs_for_type;
		
		// Get back the inputs
		PVCore::PVSerializeObject_p so_inputs = int_lib->serialize_inputs(so, type_name, inputs_for_type);

		// Save the serialize object, so that PVSource objects can refere to them for their inputs
		_so_inputs[*int_lib] = so_inputs;

		tmp_inputs.append(inputs_for_type);
	}

	data_tree_scene_t::serialize_read(so, v);
}

void Picviz::PVScene::serialize_write(PVCore::PVSerializeObject& so)
{
	// First serialize the input descriptions.
	_so_inputs.clear();
	QList<PVRush::PVInputType_p> in_types(get_all_input_types());
	QStringList in_types_str;
	for (PVRush::PVInputType_p const& in_t: in_types) {
		list_sources_t sources = get_sources(*in_t);

		// Safety check
		if (sources.size() == 0) {
			continue;
		}

		PVRush::PVInputType::list_inputs_desc inputs = get_inputs_desc(*in_t);
		PVCore::PVSerializeObject_p so_inputs = in_t->serialize_inputs(so, in_t->registered_name(), inputs);
		_so_inputs[*in_t] = so_inputs;
		in_types_str.push_back(in_t->registered_name());
	}

	so.list_attributes("types", in_types_str);
	so.attribute("name", _name);

	data_tree_scene_t::serialize_write(so);
}

PVCore::PVSerializeObject_p Picviz::PVScene::get_so_inputs(PVSource const& src)
{
	return _so_inputs[*(src.get_input_type())];
}

PVCore::PVSerializeArchiveOptions_p Picviz::PVScene::get_default_serialize_options()
{
	PVCore::PVSerializeArchiveOptions_p ar(new PVCore::PVSerializeArchiveOptions(PICVIZ_ARCHIVES_VERSION));
	ar->get_root()->object("scene", *this, ARCHIVE_SCENE_DESC);
	return ar;
}

void Picviz::PVScene::save_to_file(QString const& path, PVCore::PVSerializeArchiveOptions_p options, bool save_everything)
{
#ifdef CUSTOMER_CAPABILITY_SAVE
	set_path(path);
	PVCore::PVSerializeArchive_p ar(new PVCore::PVSerializeArchiveZip(path, PVCore::PVSerializeArchive::write, PICVIZ_ARCHIVES_VERSION));
	if (options) {
		ar->set_options(options);
	}
	ar->set_save_everything(save_everything);
	ar->get_root()->object("scene", *this, ARCHIVE_SCENE_DESC);
	ar->finish();
#endif
}

void Picviz::PVScene::load_from_file(QString const& path)
{
#ifdef CUSTOMER_CAPABILITY_SAVE
	PVCore::PVSerializeArchive_p ar(new PVCore::PVSerializeArchiveZip(path, PVCore::PVSerializeArchive::read, PICVIZ_ARCHIVES_VERSION));
	load_from_archive(ar);
#endif
}

void Picviz::PVScene::load_from_archive(PVCore::PVSerializeArchive_p ar)
{
#ifdef CUSTOMER_CAPABILITY_SAVE
	ar->get_root()->object("scene", *this, ARCHIVE_SCENE_DESC);
	_original_archive = ar;
#endif
}
