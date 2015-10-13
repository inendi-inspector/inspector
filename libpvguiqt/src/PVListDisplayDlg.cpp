/**
 * \file PVListDisplayDlg.cpp
 *
 * Copyright (C) Picviz Labs 2010-2012
 */

#include <pvkernel/core/PVProgressBox.h>
#include <pvkernel/rush/PVNraw.h>

#include <picviz/PVView.h>

#include <pvguiqt/PVListDisplayDlg.h>
#include <pvguiqt/PVStringSortProxyModel.h>
#include <pvguiqt/PVToolTipDelegate.h>

#include <QClipboard>
#include <QMessageBox>
#include <QMenu>

#include <pvkernel/core/PVLogger.h>
#include <pvkernel/core/PVAlgorithms.h>
#include <pvkernel/core/picviz_bench.h>

#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>

#define AUTOMATIC_SORT_MAX_NUMBER 32768

class PVVerticalHeaderItemModel : public QAbstractItemModel
{
public:
	PVVerticalHeaderItemModel(int row_count, QWidget* parent = nullptr) : QAbstractItemModel(parent), _row_count(row_count) {}

protected:
	int columnCount(const QModelIndex& /*index*/) const override
	{
		return 1;
	}

	int rowCount(const QModelIndex& /*index*/) const override
	{
		return _row_count;
	}

	QModelIndex index(int row, int column, const QModelIndex& /*parent*/) const override
	{
		return createIndex(row, column, nullptr);
	}

	QVariant data(const QModelIndex & /*index*/, int /*role*/ = Qt::DisplayRole) const override
	{
		return QVariant();
	}

	QModelIndex parent(const QModelIndex & /*index*/) const override
	{
		return QModelIndex();
	}

private:
	int _row_count;
};

PVGuiQt::PVListDisplayDlg::PVListDisplayDlg(QAbstractListModel* model, QWidget* parent):
	QDialog(parent)
{
	setupUi(this);
	_line_separator_button->setClearButtonShow(PVWidgets::QKeySequenceWidget::NoShow);
	_line_separator_button->setKeySequence(QKeySequence(Qt::Key_Return));
	_line_separator_button->setMaxNumKey(1);

	// Sort proxy model
	PVStringSortProxyModel* proxy_model = new PVStringSortProxyModel(_values_view);
	proxy_model->setSourceModel(model);

	// `_values_view' is a QTableView, because QListView suffers from the same
	// bug than QTableView used to suffer when a "large" (> 75000000) number of
	// items are present in the model. See
	// https://bugreports.qt-project.org/browse/QTBUG-18490 for more
	// informations.
	// The order of the calls here are important, especially the call to
	// setDefaultSectionSize that must be called *before* setModel, or it could
	// take a huge amount of time.

	_values_view->setModel(proxy_model);
	_values_view->setGridStyle(Qt::NoPen);
	_values_view->setContextMenuPolicy(Qt::ActionsContextMenu);
	_values_view->verticalHeader()->hide();
	_values_view->setItemDelegate(new PVToolTipDelegate(this));
	_values_view->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
	_values_view->horizontalHeader()->setStretchLastSection(true);
	QHeaderView* vertical_header = new QHeaderView(Qt::Vertical);
	vertical_header->setModel(new PVVerticalHeaderItemModel(_values_view->model()->rowCount(), vertical_header));
	_values_view->setVerticalHeader(vertical_header);
	_values_view->verticalHeader()->setDefaultSectionSize(_values_view->verticalHeader()->minimumSectionSize());
	_values_view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	_values_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_values_view->setSelectionBehavior(QAbstractItemView::SelectRows);

	_copy_values_act = new QAction(tr("Copy values"), this);

	_ctxt_menu = new QMenu(this);
	_ctxt_menu->addAction(_copy_values_act);

	_nb_values_edit->setText(QString().setNum(model->rowCount()));

	set_description(QString());

	connect(_values_view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(show_ctxt_menu(const QPoint&)));
	_values_view->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(_btn_copy_clipboard, SIGNAL(clicked()), this, SLOT(copy_all_to_clipboard()));
	connect(_btn_copy_file, SIGNAL(clicked()), this, SLOT(copy_to_file()));
	connect(_btn_append_file, SIGNAL(clicked()), this, SLOT(append_to_file()));
	connect(_btn_sort, SIGNAL(clicked()), this, SLOT(sort()));

	// Bind the click on header to sort the clicked column
	connect(_values_view->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(section_clicked(int)));
	_values_view->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	if (model->rowCount() < AUTOMATIC_SORT_MAX_NUMBER) {
		sort();
	}
}

void PVGuiQt::PVListDisplayDlg::show_ctxt_menu(const QPoint& /*pos*/)
{
	QModelIndex index = _values_view->currentIndex();

	if (index.column() != 0) {
		// context menu only for the "value" column
		return;
	}

	// Show the menu at the given pos
	QAction* act_sel = _ctxt_menu->exec(QCursor::pos());

	process_context_menu(act_sel);
}

bool PVGuiQt::PVListDisplayDlg::process_context_menu(QAction* act)
{
	if (act) {
		if (act == _copy_values_act) {
			copy_selected_to_clipboard();
			return true;
		}
	}
	return false;
}

void PVGuiQt::PVListDisplayDlg::copy_all_to_clipboard()
{
	ask_for_copying_count();

	QString content;
	export_values(model()->rowCount(), [&](int i) -> QModelIndex {
		return proxy_model()->index(i, 0, QModelIndex());
	}, content);

	QApplication::clipboard()->setText(content);
}

void PVGuiQt::PVListDisplayDlg::copy_selected_to_clipboard()
{
	QModelIndexList indexes = _values_view->selectionModel()->selectedIndexes();

	QString content;
	export_values(indexes.size(), [&indexes](int i) -> QModelIndex {
		return indexes.at(i);
	}, content);

	QApplication::clipboard()->setText(content);
}

void PVGuiQt::PVListDisplayDlg::export_to_file(QFile& file)
{
	QString path(file.fileName());
	QTextStream outstream(&file);

	QString content;
	bool success = export_values(model()->rowCount(), [&](int i) -> QModelIndex {
		return proxy_model()->index(i, 0, QModelIndex());
	}, content);

	outstream << content;

	if (!success) {
		if (QMessageBox::question(this, tr("File writing cancelled"), tr("Do you want to remove the file '%1'?").arg(path), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
			if (!file.remove()) {
				QMessageBox::warning(this, tr("File writing cancelled"), tr("Error while removing '%1': %2").arg(path).arg(file.errorString()));
			}
		}
		return;
	}
}

QString PVGuiQt::PVListDisplayDlg::export_line(
	PVGuiQt::PVStringSortProxyModel* model,
	std::function<QModelIndex(int)> f,
	int i
)
{
	QModelIndex idx = f(i); // using return instead of ref parameter fails
	if (likely(idx.isValid())) {
		return model->data(idx).toString();
	}

	return QString();
}

bool PVGuiQt::PVListDisplayDlg::export_values(int count, std::function<QModelIndex (int)> f, QString& content)
{
	QApplication::setOverrideCursor(Qt::BusyCursor);

	QString sep(QChar::fromLatin1(PVWidgets::QKeySequenceWidget::get_ascii_from_sequence(_line_separator_button->keySequence())));
	if (sep.isEmpty()) {
		sep = "\n";
	}

	PVCore::PVProgressBox* pbox = new PVCore::PVProgressBox(QObject::tr("Copying values..."), parentWidget());

	const size_t nthreads = PVCore::PVHardwareConcurrency::get_physical_core_number();
	tbb::task_scheduler_init init(nthreads);
	tbb::task_group_context ctxt;

	bool success = PVCore::PVProgressBox::progress([&,count]() {

		BENCH_START(export_values);
		content = tbb::parallel_reduce(
			tbb::blocked_range<int>(0, count, std::max(nthreads, count / nthreads)),
			content,
			[&](const tbb::blocked_range<int>& range, QString l) -> QString {
				PVGuiQt::PVStringSortProxyModel* m = proxy_model();
				for (int i = range.begin(); i < range.end(); i++) {
					if unlikely(ctxt.is_group_execution_cancelled()) {
						return QString();
					}
					QString s = export_line(m, f, i);
					if (!s.isNull()) {
						l.append(s.append(sep));
					}
				}
				return l;
			},
			[](const QString& left, const QString& right) -> QString {
				const_cast<QString&>(left).append(right); // const_cast needed to use optimized append method
				return left;
			}
		, tbb::simple_partitioner());
		BENCH_END(export_values, "export_values", 0, 0, 1, content.size());

		return !ctxt.is_group_execution_cancelled();
	}, ctxt, pbox);

	//content.truncate(content.size()-sep.size()); // Remove last carriage return

	QApplication::restoreOverrideCursor();

	return success;
}

void PVGuiQt::PVListDisplayDlg::export_to_file_ui(bool append)
{
	ask_for_copying_count();

	QFileDialog::Options options = 0;
	if (append) {
		options = QFileDialog::DontConfirmOverwrite;
	}
	QString path = _file_dlg.getSaveFileName(this, tr("Save to file..."), QString(), QString(), NULL, options);
	if (path.isEmpty()) {
		return;
	}

	QFile file(path);
	QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
	if (append) {
		mode |= QIODevice::Append;
	}
	else {
		mode |= QIODevice::Truncate;
	}
	if (!file.open(mode)) {
		QMessageBox::critical(this, tr("Copy to file..."), tr("Unable to open '%1' for writing: %2.").arg(path).arg(file.errorString()));
		return;
	}
	export_to_file(file);
}

void PVGuiQt::PVListDisplayDlg::set_description(QString const& desc)
{
	if (desc.isEmpty()) {
		_desc_label->setVisible(false);
	}
	else {
		_desc_label->setVisible(true);
		_desc_label->setText(desc);
	}
}

PVGuiQt::PVStringSortProxyModel* PVGuiQt::PVListDisplayDlg::proxy_model()
{
	return static_cast<PVStringSortProxyModel*>(_values_view->model());
}

QAbstractListModel* PVGuiQt::PVListDisplayDlg::model()
{
	return static_cast<QAbstractListModel*>(proxy_model()->sourceModel());
}

void PVGuiQt::PVListDisplayDlg::section_clicked(int col)
{
	// Save selection
	QItemSelection sel = _values_view->selectionModel()->selection();
	sel = proxy_model()->mapSelectionToSource(sel);

	// Sort
	sort_by_column(col);

	// Restore selection
	_values_view->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear); // Get rid of the annoying misplaced focus...
	_values_view->selectionModel()->clearSelection();
	sel = proxy_model()->mapSelectionFromSource(sel);
	_values_view->selectionModel()->select(sel, QItemSelectionModel::Select);
}

void PVGuiQt::PVListDisplayDlg::sort_by_column(int col)
{
	if (col == 0) {
		sort();
	}
}

void PVGuiQt::PVListDisplayDlg::sort()
{
	Qt::SortOrder order =  _values_view->horizontalHeader()->sortIndicatorOrder();
	proxy_model()->sort(0, order);
	_btn_sort->hide();
}
