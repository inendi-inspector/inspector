/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVSerializeArchiveZip.h>
#include <inendi/PVLayer.h>
#include <inendi/PVLayerStack.h>
#include <inendi/PVSelection.h>

#include <QApplication>
#include <QClipboard>

/******************************************************************************
 *
 * Inendi::PVLayerStack::PVLayerStack
 *
 *****************************************************************************/
Inendi::PVLayerStack::PVLayerStack(PVRow row_count) :
	lia(row_count)
{
	layer_count = 0;
	next_new_layer_counter = 0;
	selected_layer_index = -1;
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::get_new_layer_name
 *
 *****************************************************************************/
QString Inendi::PVLayerStack::get_new_layer_name() const
{
	return QString("New layer %1").arg(next_new_layer_counter+1);
}

/******************************************************************************
 *
 * PVGuiQt::PVLayerStackModel::hide_layers
 *
 *****************************************************************************/
void Inendi::PVLayerStack::hide_layers()
{
	for (int i = 0; i < layer_count; i++) {
		PVLayer &layer = get_layer_n(i);
		layer.set_visible(false);
	}
	update_layer_index_array_completely();
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::append_layer
 *
 *****************************************************************************/
Inendi::PVLayer* Inendi::PVLayerStack::append_new_layer(QString const& name)
{
	/* We prepare the string for the name of the new layer */
	QString layer_name;
	if (name.isEmpty()) {
		layer_name = get_new_layer_name();
	}
	else {
		layer_name = name;
	}
	++next_new_layer_counter;
	return append_layer(PVLayer(layer_name));
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::append_layer
 *
 *****************************************************************************/
Inendi::PVLayer* Inendi::PVLayerStack::append_layer(const PVLayer &layer)
{
	/* We test if we have not reached the maximal number of layers */
	if (layer_count < INENDI_LAYER_STACK_MAX_DEPTH-1) {
		table.append(layer);
		selected_layer_index = layer_count;
		layer_count += 1;

		/* FIXME! This is before we do something more clever... */
		update_layer_index_array_completely();

		return &table.last();
	}
	// FIXME: should have an exception here, that will be treated by the GUI !
	return NULL;
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::append_new_layer_from_selection_and_lines_properties
 *
 *****************************************************************************/
Inendi::PVLayer* Inendi::PVLayerStack::append_new_layer_from_selection_and_lines_properties(PVSelection const& selection, PVLinesProperties const& lines_properties)
{
	/* We prepare the string for the name of the new layer */
	QString new_layer_automatic_name = QString("New layer %1").arg(layer_count);
	return append_layer(PVLayer(new_layer_automatic_name, selection, lines_properties));
}


/******************************************************************************
 *
 * Inendi::PVLayerStack::delete_by_index
 *
 *****************************************************************************/
void Inendi::PVLayerStack::delete_by_index(int index)
{
	if (layer_count == 1) {
		// if there is only one layer, it must not be removable
		return;
	}

	if ((0 <= index) && (index < layer_count)) {
		table.removeAt(index);
		layer_count--;

		/* We set the selected layer according to the posibilities */
		if (layer_count == 0) {
			selected_layer_index = -1;
		} else {
			selected_layer_index = 0;
		}
		
		/* FIXME! This is before we do something more clever... */
		update_layer_index_array_completely();
	}
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::delete_selected_layer
 *
 *****************************************************************************/
void Inendi::PVLayerStack::delete_selected_layer()
{
	delete_by_index(selected_layer_index);
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::duplicate_selected_layer
 *
 *****************************************************************************/
Inendi::PVLayer* Inendi::PVLayerStack::duplicate_selected_layer(const QString &name)
{
	if ((selected_layer_index < 0) || (selected_layer_index >= layer_count)) {
		return nullptr;
	}

	const PVLayer &selected_layer = table.at(selected_layer_index);
	PVLayer *new_layer = append_new_layer(name);

	new_layer->get_selection() = selected_layer.get_selection();
	new_layer->get_lines_properties() = selected_layer.get_lines_properties();

	/* FIXME! This is before we do something more clever... */
	update_layer_index_array_completely();

	return new_layer;
}

/******************************************************************************
 *
 * Inendi::PVLayerStack::delete_all_layers
 *
 *****************************************************************************/
void Inendi::PVLayerStack::delete_all_layers()
{
	table.clear();
	layer_count = 0;
	selected_layer_index = -1;
	lia.initialize();
}

/**********************************************************************
*
* Inendi::PVLayerStack::move_layer_down
*
**********************************************************************/
void Inendi::PVLayerStack::move_layer_down(int index)
{
	if (( 0 < index ) && (index < layer_count)) {
		table.move(index, index - 1);

		/* FIXME! This is before we do something more clever... */
		update_layer_index_array_completely();
	}
}

/**********************************************************************
*
* Inendi::PVLayerStack::move_layer_up
*
**********************************************************************/
void Inendi::PVLayerStack::move_layer_up(int index)
{
	if ( index < (layer_count - 1)) {
		table.move(index, index + 1);

		/* FIXME! This is before we do something more clever... */
		update_layer_index_array_completely();
	}
}

/**********************************************************************
* Inendi::PVLayerStack::move_selected_layer_to
**********************************************************************/

void Inendi::PVLayerStack::move_selected_layer_to(int new_index)
{
	if ((new_index < 0) || (new_index >= layer_count)) {
		return;
	}

	table.move(selected_layer_index, new_index);
	selected_layer_index = new_index;
	update_layer_index_array_completely();
}

/**********************************************************************
*
* Inendi::PVLayerStack::move_selected_layer_down
*
**********************************************************************/
void Inendi::PVLayerStack::move_selected_layer_down()
{
	if (selected_layer_index > 0) {
		move_layer_down(selected_layer_index);
		selected_layer_index  -= 1;
	}
}

/**********************************************************************
 *
 * Inendi::PVLayerStack::move_selected_layer_up
 *
 **********************************************************************/
void Inendi::PVLayerStack::move_selected_layer_up()
{
	if (selected_layer_index < layer_count - 1) {
		move_layer_up(selected_layer_index);
		selected_layer_index  += 1;
	}
}

/**********************************************************************
 *
 * Inendi::PVLayerStack::process
 *
 **********************************************************************/
void Inendi::PVLayerStack::process(PVLayer &output_layer, PVRow row_count)
{
	/******************************
	* Preparation
	******************************/
	/* We prepare a pointer to the layer of the LS being processed */
	PVLayer *layer_being_processed;
	/* We prepare a temporary selection, needed in our computations */
	PVSelection temp_selection;
	/* We store locally the layer-stack->layer_count and prepare a counter */
	int i;

	/******************************
	* Initializations
	******************************/
	/* It's time to erase the output_layer ! */
	output_layer.reset_to_empty_and_default_color();

	/******************************
	* Main computation
	******************************/
	/* The layer-stack might be empty so we have to be carefull... */
	if ( layer_count > 0 ) {
		/* We process the layers from TOP to BOTTOM, that is, from
		*  the most visible to the less visible */
		for ( i=layer_count-1; i >= 0; i--) {
			/* We prepare a direct access to the layer we have to process */
			layer_being_processed = &(table[i]);
			/* We check if this layer is visible */
			if (layer_being_processed->get_visible()) {
				/* we compute the selection of lines present
				*  in the layer being processed but not yet
				*  in the output layer */

				// The line below is an optimized version of:
				//temp_selection = layer_being_processed->get_selection() - output_layer.get_selection();
				temp_selection.AB_sub(layer_being_processed->get_selection(), output_layer.get_selection());

				/* and we already update the selection in
				*  the output_layer */
				output_layer.get_selection().or_optimized(layer_being_processed->get_selection());
				//output_layer.get_selection() |= layer_being_processed->get_selection();

				/* We copy in the output_layer the only new
				*  lines properties */
				layer_being_processed->get_lines_properties().A2B_copy_restricted_by_selection_and_nelts(output_layer.get_lines_properties(), temp_selection, row_count);
			}
		}
	}
}

/******************************************************************************
 *
 * update_layer_index_array_completely
 *
 * WARNING!
 *
 * This function, under normal circumstances, should not be use !
 *   The layer_index_array should be maintained up to date, step by step, by
 *   "tacking into account" the changes done in the layer_stack.
 *
 * This function is a starter...
 *
 * HOW IT WORKS.
 *
 * the LIA is a reverse-access data structure used to find the first layer in
 * which an event appears.
 *
 * To construct it, the layer stack is processed from top to bottom:
 * for each layer, if a contained event has not already been referenced in
 * the LIA, it's value is set to the current layer's index.
 *
 * To speed-up the processing, a PVSelection is used to reference already
 * processed events.
 *
 *****************************************************************************/
/**********************************************************************
 *
 * Inendi::PVLayerStack::update_layer_index_array_completely
 *
 **********************************************************************/
void Inendi::PVLayerStack::update_layer_index_array_completely()
{
	/******************************
	* Preparation
	******************************/
	/* We prepare a reference to the layer of the LS being processed */
// 	PVLayer &layer_being_processed;
	/* We prepare the two selections needed in our algorithm */
	PVSelection done_selection;
	PVSelection temp_selection;
	/* We prepare a direct access to the number of layers, and 2 counters */
       	int i, k;

	/******************************
	* Main computation
	******************************/
	/* The layer-stack might be empty so we have to be carefull... */
	if ( layer_count > 0 ) {
		/* We process the layers from top to bottom */
		for ( i=layer_count-1; i >= 0; i--) {
			/* We prepare a direct access to the layer we have to process */
			PVLayer &layer_being_processed = table[i];
			/* We check if the layer is visible */
			if (layer_being_processed.get_visible()) {
				/* We remove from temp selection the lines already encountered */
				temp_selection = layer_being_processed.get_selection();
				temp_selection -= done_selection;
				/* We add to done_selection the lines that we are about to process */
				done_selection |= layer_being_processed.get_selection();
				for (k = 0; k < lia.get_row_count(); k++) {
					if (temp_selection.get_line(k)) {
						lia.set_value(k, i + 1);
					}
				}
			}
		}

		/* We set to 0 the lines that never appeared in any layer */
		for (k = 0; k < lia.get_row_count(); k++) {
			if ( ! done_selection.get_line(k) ) {
				lia.set_value(k, 0);
			}
		}
	} else {
		/* We zero every line since there are no layers */
		for (k = 0; k < lia.get_row_count(); k++) {
			lia.set_value(k, 0);
		}
	}

	/******************************
	* Garbage collection
	******************************/
}

void Inendi::PVLayerStack::compute_min_maxs(PVPlotted const& plotted)
{
	for (int i = 0; i < table.size(); i++) {
		table[i].compute_min_max(plotted);
	}
}

void Inendi::PVLayerStack::compute_selectable_count(PVRow row_count)
{
	for (int i = 0; i < table.size(); i++) {
		table[i].compute_selectable_count(row_count);
	}
}

bool Inendi::PVLayerStack::contains_layer(PVLayer* layer) const
{
	foreach(PVLayer const& l, table) {
		if (&l == layer) {
			return true;
		}
	}
	return false;
}

void Inendi::PVLayerStack::serialize(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t /*v*/)
{
	so.list("layers", table, QString(), (PVLayer*) NULL, QStringList(), false);
	so.attribute("selected_layer_index", selected_layer_index);
	so.object("lia", lia, QString(), false, (PVLayerIndexArray*) NULL, false);

	if (!so.is_writing()) {
		layer_count = table.size();
		if (layer_count == 0) {
			// Create default layer
			append_new_layer();
		}
	}
}

void Inendi::PVLayerStack::load_from_file(QString const& path)
{
	PVCore::PVSerializeArchive_p ar(new PVCore::PVSerializeArchiveZip(path, PVCore::PVSerializeArchive::read, INENDI_ARCHIVES_VERSION));
	ar->get_root()->object("layer-stack", *this);
	ar->finish();
}

void Inendi::PVLayerStack::save_to_file(QString const& path)
{
	PVCore::PVSerializeArchive_p ar(new PVCore::PVSerializeArchiveZip(path, PVCore::PVSerializeArchive::write, INENDI_ARCHIVES_VERSION));
	ar->get_root()->object("layer-stack", *this);
	ar->finish();
}

void Inendi::PVLayerStack::copy_details_to_clipboard()
{
	QString s;

	for(int i = table.size() - 1; i >= 0; --i) {
		const auto& l = get_layer_n(i);
		s += l.get_name();
		s += "\t";
		s += QString::number(l.get_selectable_count());
		s += "\n";

	}

	QApplication::clipboard()->setText(s);
}