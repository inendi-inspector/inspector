/**
 * @file
 *
 *
 * @copyright (C) ESI Group INENDI 2015-2015
 */

#include "PVOpcUaParamsWidget.h"
#include "PVInputTypeOpcUa.h"
#include "../../common/opcua/PVOpcUaTreeModel.h"
#include "../../common/opcua/PVOpcUaAPI.h"

#include <pvkernel/core/PVProgressBox.h>
#include <pvkernel/rush/PVUtils.h>
#include <pvkernel/widgets/PVFilterableComboBox.h>
#include <pvkernel/widgets/PVQueryBuilder.h>
#include <pvkernel/rush/PVXmlTreeNodeDom.h>
#include <pvkernel/rush/PVFormat.h>
#include <pvkernel/rush/PVFormat_types.h>

#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QOpcUaClient>
#include <QOpcUaProvider>
#include <QOpcUaPkiConfiguration>
#include <QOpcUaAuthenticationInformation>
#include <QOpcUaErrorState>

// #include <QSslSocket>
// #include <openssl/ssl.h>

PVRush::PVOpcUaParamsWidget::PVOpcUaParamsWidget(PVInputTypeOpcUa const* in_t,
                                                 PVRush::hash_formats const& formats,
                                                 QWidget* parent)
    : PVParamsWidget<PVInputTypeOpcUa, PVOpcUaPresets, PVOpcUaInfos, PVOpcUaQuery>(
          in_t, formats, parent)
{
	_txt_host->setText("opc.tcp://your-opcua-server.com:4840/");
	_txt_host->setToolTip("opc.tcp://your-opcua-server.com:4840/");
	label_3->setVisible(false);
	_port_sb->setVisible(false);
	tabWidget->removeTab(1);

	_columns_tree_widget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	_help_label->setText(R"html(
<html><head/><body>
	<p>Currently able to load integer nodes.</p>
	<p>Limitations of <a link="https://open62541.org">Open62541</a> implementation applies.</p>
</body></html>
		)html");
	
	tabWidget->setCurrentIndex(0);

	resize(QApplication::desktop()->availableGeometry().height() - 50, QApplication::desktop()->availableGeometry().width() / 2);
}

void PVRush::PVOpcUaParamsWidget::reset_columns_tree_widget()
{
	disconnect(_columns_tree_widget, &QTreeWidget::itemChanged, this,
	           &PVOpcUaParamsWidget::tree_item_changed);

	_columns_tree_widget->clear();
	_root_item = new QTreeWidgetItem(_columns_tree_widget);
	_root_item->setExpanded(true);
	_root_item->setText(0, "properties");
	_root_item->setCheckState(0, Qt::Unchecked);

	auto deserialized_query = get_serialize_query().split(QRegularExpression("\\;\\$\\;"));
	size_t nodes_count = deserialized_query.size() / 3;
	for (size_t i = 0; i < nodes_count; ++i) {
		// configure node per node
		auto column_node_id = deserialized_query[3 * i + 0];
		auto column_name = deserialized_query[3 * i + 1];
		auto column_type = deserialized_query[3 * i + 2];
		QTreeWidgetItem* tree_item = new QTreeWidgetItem(_root_item);
		tree_item->setText(0, "(" + column_node_id + ") " + column_name);
		tree_item->setData(0, Qt::UserRole, column_node_id);
		tree_item->setText(1, column_type);
		tree_item->setExpanded(true);
		tree_item->setCheckState(0, Qt::Checked);
	}

	_columns_tree_widget->resizeColumnToContents(0);

	connect(_columns_tree_widget, &QTreeWidget::itemChanged, this,
	        &PVOpcUaParamsWidget::tree_item_changed);
}

/*
 * Recursively visit all the columns of the columns tree widget
 *
 * @param tree_item root item to visit
 * @param f the function that is called against each QTreeWidgetItem
 */
static void visit_columns(QTreeWidgetItem* tree_item,
                          const std::function<void(QTreeWidgetItem*)>& f)
{
	f(tree_item);
	for (int i = 0; i < tree_item->childCount(); i++) {
		QTreeWidgetItem* tree_child = tree_item->child(i);
		visit_columns(tree_child, f);
	}
}

/*
 * Recursively visit the checked columns of the columns tree widget
 *
 * @param tree_item root item to visit
 * @param f the function that is called against each QTreeWidgetItem
 */
static void visit_selected_columns(QTreeWidgetItem* tree_item,
                                   const std::function<void(QTreeWidgetItem*)>& f)
{
	if (not tree_item)
		return;

	static auto is_checked = [](const QTreeWidgetItem* item) {
		return item->checkState(0) == Qt::Checked or item->checkState(0) == Qt::PartiallyChecked;
	};

	if (is_checked(tree_item) and tree_item->childCount() == 0) {
		f(tree_item);
	}

	for (int i = 0; i < tree_item->childCount(); i++) {
		QTreeWidgetItem* tree_child = tree_item->child(i);
		if (is_checked(tree_child)) {
			visit_selected_columns(tree_child, f);
		}
	}
}

static void propagate_check_state(QTreeWidgetItem* leaf_item)
{
	QTreeWidgetItem* parent = leaf_item->parent();
	if (parent == nullptr) {
		return;
	}

	int child_count = parent->childCount();
	int checked_count = 0;
	for (int i = 0; i < child_count; i++) {
		checked_count += parent->child(i)->checkState(0);
	}
	Qt::CheckState parent_state =
	    (checked_count == child_count * 2
	         ? Qt::Checked
	         : (checked_count == 0 ? Qt::Unchecked : Qt::PartiallyChecked));

	parent->setCheckState(0, parent_state);
	propagate_check_state(parent);
}

void PVRush::PVOpcUaParamsWidget::tree_item_changed(QTreeWidgetItem* item, int column)
{
	_columns_tree_widget->blockSignals(true);

	if (column == 0) {
		if (item->childCount() > 0) { // node
			Qt::CheckState check_state = item->checkState(0);
			visit_columns(item,
			              [&](QTreeWidgetItem* child) { child->setCheckState(0, check_state); });
		}
		propagate_check_state(item);
	}

	_columns_tree_widget->blockSignals(false);
}

void PVRush::PVOpcUaParamsWidget::fetch_server_data_slot()
{
	fetch_server_data(get_infos());
}

bool PVRush::PVOpcUaParamsWidget::check_connection(std::string* error /*= nullptr*/)
{
	const PVOpcUaInfos& infos = get_infos();

	QOpcUaProvider provider;
	if (provider.availableBackends().isEmpty()) {
		qDebug() << "No OpcUa backend available!";
		return false;
	}
	QOpcUaClient* client = provider.createClient(provider.availableBackends()[0]);
	if (!client) {
		qDebug() << "OpcUa backend (" << provider.availableBackends()[0]
		         << ") could not be loaded and could not create client.";
		return false;
	}


	QOpcUaPkiConfiguration pkiConfig;

#if 0 // Not yet supported
	QString pkidir("/home/===/pkidir/");
	pkiConfig.setClientCertificateFile(pkidir + "/own/certs/certificate.der");
	pkiConfig.setPrivateKeyFile(pkidir + "/own/private/privatekey.pem");
	pkiConfig.setTrustListDirectory(pkidir + "/trusted/certs");
	pkiConfig.setRevocationListDirectory(pkidir + "/trusted/crl");
	pkiConfig.setIssuerListDirectory(pkidir + "/issuers/certs");
	pkiConfig.setIssuerRevocationListDirectory(pkidir + "/issuers/crl");
#endif

	QOpcUaAuthenticationInformation authInfo;
	if (infos.get_login().isEmpty()) {
		authInfo.setAnonymousAuthentication();
	} else {
		authInfo.setUsernameAuthentication(infos.get_login(), infos.get_password());
	}

	client->setAuthenticationInformation(authInfo);
#if 0 // Not yet supported
	client->setPkiConfiguration(pkiConfig);
#endif
	client->setApplicationIdentity(pkiConfig.applicationIdentity());

	QObject::connect(
	    client, &QOpcUaClient::stateChanged, [client, this](QOpcUaClient::ClientState state) {
		    qDebug() << "Client state changed:" << state;
		    if (state == QOpcUaClient::ClientState::Connected) {
			    QOpcUaNode* node = client->node("ns=0;i=84");
			    _opcua_treeview = new QTreeView(this);
			    _opcua_treeview->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
			    _opcua_treeview->setTextElideMode(Qt::ElideRight);
			    _opcua_treeview->setAlternatingRowColors(true);
			    _opcua_treeview->setSelectionBehavior(QAbstractItemView::SelectRows);
			    _opcua_treeview->setSelectionMode(QAbstractItemView::MultiSelection);
			    _opcua_treeview->setMinimumWidth(width());
			    _opcua_treeview->setMinimumHeight(height());
			    auto treemodel = new PVOpcUaTreeModel(this);
			    treemodel->setOpcUaClient(client);
			    _opcua_treeview->setModel(treemodel);
			    _opcua_treeview->header()->setSectionResizeMode(1 /* Value column*/,
			                                                    QHeaderView::Interactive);
			    auto scroll_area = new QScrollArea;
				scroll_area->setWidget(_opcua_treeview);
				auto tab_index = tabWidget->addTab(scroll_area, "Browse nodes");
			    connect(_opcua_treeview->selectionModel(), &QItemSelectionModel::selectionChanged,
			            [this](const QItemSelection& selected, const QItemSelection& deselected) {
				            reset_columns_tree_widget();
				            for (auto& selitem : selected) {
					            //
				            }
			            });
				tabWidget->setCurrentIndex(tab_index);
		    }
	    });

	connect(client, &QOpcUaClient::connectError, [](QOpcUaErrorState* errorState) {
		qDebug() << "Client Error State:" << QOpcUa::statusToString(errorState->errorCode());
		if (errorState->isClientSideError() &&
		    errorState->connectionStep() ==
		        QOpcUaErrorState::ConnectionStep::CertificateValidation) {
			errorState->setIgnoreError(true);
		}
	});

	QObject::connect(client, &QOpcUaClient::endpointsRequestFinished,
	                 [client](QVector<QOpcUaEndpointDescription> endpoints,
	                          QOpcUa::UaStatusCode statusCode, QUrl requestUrl) {
		                 qDebug() << "Endpoints returned:" << endpoints.count() << statusCode
		                          << requestUrl;
		                 if (endpoints.size()) {
			                 endpoints.first().setEndpointUrl(
			                     requestUrl.toString()); // Needed to workaround an unconfigured
			                                             // reverse DNS.
			                 client->connectToEndpoint(
			                     endpoints.first()); // Connect to the first endpoint in the list
		                 }
	                 });

	if (client->requestEndpoints(QUrl(infos.get_host()))) {
		qDebug() << "OpcUa client requesting endpoints...";
		return true;
	} else {
		qDebug() << "OpcUa client could not request endpoints.";
	}

	return false;
}

void PVRush::PVOpcUaParamsWidget::query_type_changed_slot()
{
}

/*****************************************************************************
 * PVRush::PVOpcUaParamsWidget::get_export_filters
 *****************************************************************************/

QString PVRush::PVOpcUaParamsWidget::get_export_filters()
{
	return "CSV File (*.csv)";
}

void PVRush::PVOpcUaParamsWidget::export_query_result(PVCore::PVStreamingCompressor& compressor,
                                                      const std::string& sep,
                                                      const std::string& quote,
                                                      bool header,
                                                      PVCore::PVProgressBox& pbox,
                                                      std::string* error)
{
	size_t count = 0;
	bool query_end = false;

	PVRush::PVOpcUaAPI es(get_infos());
	const PVOpcUaQuery& query = get_query(error);

	using PVRush::PVUtils::safe_export;

	try {
		if (header) {
			compressor.write(safe_export("Source Timestamp", sep, quote) + sep +
			                 safe_export("Data", sep, quote) + "\n");
		}

		// pbox.set_maximum(es.count(query));

		for (auto& opcua_column : _opcua_treeview->selectionModel()->selectedRows()) {

			if (error && error->empty() == false) {
				return;
			}

			if (pbox.get_cancel_state() == PVCore::PVProgressBox::CancelState::CANCEL ||
			    pbox.get_cancel_state() == PVCore::PVProgressBox::CancelState::CANCEL2) {
				break;
			}

			auto column_node_id =
			    opcua_column.siblingAtColumn(0).data(Qt::UserRole).value<QString>();
			es.read_node_history(
			    column_node_id, UA_DateTime_fromUnixTime(0), UA_DateTime_now(), 10000,
			    [&compressor, &sep, &quote, &count](UA_HistoryData* data, bool) {
				    for (UA_UInt32 i = 0; i < data->dataValuesSize; ++i) {
					    UA_DataValue& value = data->dataValues[i];
					    std::string v;
					    v += safe_export(std::to_string(value.sourceTimestamp), sep, quote) + sep;
					    v +=
					        safe_export(PVOpcUaAPI::to_json_string(value.value), sep, quote) + "\n";
					    compressor.write(v);
				    }
				    count += data->dataValuesSize;
				    return true;
			    });

			pbox.set_value(count);
			pbox.set_extended_status(QString("%L1 rows exported so far").arg(count));
		}
	} catch (const PVCore::PVStreamingCompressorError& e) {
		*error = e.what();
	}
}

bool PVRush::PVOpcUaParamsWidget::set_infos(PVOpcUaInfos const& infos)
{
	[[maybe_unused]] bool res =
	    PVParamsWidget<PVInputTypeOpcUa, PVOpcUaPresets, PVOpcUaInfos, PVOpcUaQuery>::set_infos(
	        infos);

	return true;
}

PVRush::PVOpcUaInfos PVRush::PVOpcUaParamsWidget::get_infos() const
{
	PVRush::PVOpcUaInfos infos =
	    PVParamsWidget<PVInputTypeOpcUa, PVOpcUaPresets, PVOpcUaInfos, PVOpcUaQuery>::get_infos();

	return infos;
}

QString PVRush::PVOpcUaParamsWidget::get_server_query(std::string* error /* = nullptr */) const
{
	QString q = get_serialize_query();
	return q;
}

QString PVRush::PVOpcUaParamsWidget::get_serialize_query() const
{
	if (_opcua_treeview) {
		QString serialized;
		for (auto& opcua_column : _opcua_treeview->selectionModel()->selectedRows()) {
			auto column_node_id = opcua_column.siblingAtColumn(0).data(Qt::UserRole).value<QString>();
			auto column_type = opcua_column.siblingAtColumn(3).data(Qt::UserRole).value<QString>();
			auto column_name = opcua_column.siblingAtColumn(5).data(Qt::UserRole).value<QString>();
			serialized += column_node_id + ";$;" + column_type + ";$;" + column_name + ";$;";
		}
		qDebug() << __func__ << serialized;
		return serialized;
	} else {
		qDebug() << __func__ << _serialized_query;
		return _serialized_query;
	}
}

void PVRush::PVOpcUaParamsWidget::set_query(QString const& query)
{
	_serialized_query = query;
	reset_columns_tree_widget();
	qDebug() << __func__ << query;
}

size_t PVRush::PVOpcUaParamsWidget::query_result_count(std::string* error /* = nullptr */)
{
	return 0;
}

bool PVRush::PVOpcUaParamsWidget::fetch_server_data(const PVOpcUaInfos& infos)
{
	return true;
}

size_t PVRush::PVOpcUaParamsWidget::get_selected_columns_count() const
{
	if (_opcua_treeview) {
		return _opcua_treeview->selectionModel()->selectedRows().size();
	} else {
		return _serialized_query.split(QRegularExpression("\\;\\$\\;")).size() / 3;
	}
}

static size_t get_first_level_fields_count(const QString& format_path)
{
	QDomDocument doc;

	QFile f(format_path);
	if (not f.open(QFile::ReadOnly | QFile::Text))
		return {};
	QTextStream in(&f);

	doc.setContent(in.readAll());

	const QDomElement& root = doc.documentElement();

	size_t fields_count = 0;
	for (QDomElement n = root.firstChildElement(); not n.isNull(); n = n.nextSiblingElement()) {
		fields_count += n.tagName() == PVFORMAT_XML_TAG_FIELD_STR;
	}

	return fields_count;
}

void PVRush::PVOpcUaParamsWidget::accept()
{
	size_t fields_count = get_selected_columns_count();
	if (1 + fields_count < 2) {
		QMessageBox::critical(this, "Invalid format error",
		                      "At least two columns must be selected to have a valid format.");
		return;
	}

	if (is_format_custom()) {
		update_custom_format();
	} else if (1 + fields_count != get_first_level_fields_count(_format_path->text())) {
		QMessageBox::critical(
		    this, "Invalid format error",
		    "Number of columns mismatchs between existing format and selected mapping.\n\n"
		    "You may want to edit and save your custom format.");
		return;
	}

	QDialog::accept();
}

void PVRush::PVOpcUaParamsWidget::update_custom_format()
{
	// _custom_format = PVOpcUaAPI(get_infos()).get_format_from_mapping();
	std::unique_ptr<PVXmlTreeNodeDom> format_root(
	    PVRush::PVXmlTreeNodeDom::new_format(_custom_format));

	PVRush::PVXmlTreeNodeDom* time_node =
	    format_root->addOneField(QString("SourceTime"), QString("time"));
	time_node->setAttribute(QString(PVFORMAT_AXIS_TYPE_FORMAT_STR), "yyyy-MM-d H:m:ss.S");

	set_query(get_serialize_query());

	auto deserialized_query = _serialized_query.split(QRegularExpression("\\;\\$\\;"));
	size_t nodes_count = deserialized_query.size() / 3;
	for (size_t i = 0; i < nodes_count; ++i) {
		// configure node per node
		auto column_name = deserialized_query[3 * i + 2];
		auto node_id_open62541 = PVOpcUaAPI::NodeId(deserialized_query[3 * i + 1]).open62541();
		if (auto* data_type = UA_findDataType(&node_id_open62541)) {
			PVRush::PVXmlTreeNodeDom* node =
				format_root->addOneField(column_name, QString(PVRush::PVOpcUaAPI::pvcop_type(data_type->typeIndex)));
		} else {
			PVRush::PVXmlTreeNodeDom* node = format_root->addOneField(column_name, QString("uint8"));
		}
	}
}

void PVRush::PVOpcUaParamsWidget::edit_custom_format()
{
	update_custom_format();
	PVParamsWidgetBase::edit_custom_format();
}