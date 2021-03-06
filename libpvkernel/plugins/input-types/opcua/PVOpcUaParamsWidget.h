/* * MIT License
 *
 * © ESI Group, 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __PVOPCUAPARAMSWIDGET_H__
#define __PVOPCUAPARAMSWIDGET_H__

#include "../common/PVParamsWidget.h"
#include "../../common/opcua/PVOpcUaQuery.h"
#include "PVOpcUaPresets.h"

namespace PVWidgets
{
class PVFilterableComboBox;
}

namespace PVRush
{

class PVInputTypeOpcUa;
class PVOpcUaQuery;
class PVOpcUaInfos;

class PVOpcUaParamsWidget
    : public PVParamsWidget<PVInputTypeOpcUa, PVOpcUaPresets, PVOpcUaInfos, PVOpcUaQuery>
{
	Q_OBJECT

  private:
	enum EQueryType {
		QUERY_BUILDER = 0,
		JSON,
		SQL,

		COUNT
	};

  public:
	PVOpcUaParamsWidget(PVInputTypeOpcUa const* in_t,
	                    PVRush::hash_formats const& formats,
	                    QWidget* parent);

  public:
	QString get_server_query(std::string* error = nullptr) const override;
	QString get_serialize_query() const override;

  protected:
	size_t query_result_count(std::string* error = nullptr) override;
	bool fetch_server_data(const PVOpcUaInfos& infos) override;
	void query_type_changed_slot() override;
	QString get_export_filters() override;
	void accept() override;
	PVOpcUaInfos get_infos() const override;
	bool set_infos(PVOpcUaInfos const& infos) override;
	void set_query(QString const& query) override;
	void check_connection_slot() override;
	bool check_connection(std::string* error = nullptr) override;
	void export_query_result(PVCore::PVStreamingCompressor& compressor,
	                         const std::string& sep,
	                         const std::string& quote,
	                         bool header,
	                         PVCore::PVProgressBox& pbox,
	                         std::string* error = nullptr) override;
	void edit_custom_format() override;

  private:
	void fetch_server_data_slot();
	void update_custom_format();
	void reset_columns_tree_widget();
	void set_columns_tree_widget_selection(const QString& filter_path);
	void tree_item_changed(QTreeWidgetItem* item, int column);
	size_t get_selected_columns_count() const;
	void export_node(QString node_id, QString node_name, bool source_timestamp = true);

  private:
	QTreeWidgetItem* _root_item = nullptr;
	QTreeView* _opcua_treeview = nullptr;
	QString _serialized_query;
};

} // namespace PVRush

#endif // __PVOPCUAPARAMSWIDGET_H__
