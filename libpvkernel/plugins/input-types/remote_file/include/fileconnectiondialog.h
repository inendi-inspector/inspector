/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef FILECONNECTIONDIALOG_H
#define FILECONNECTIONDIALOG_H

#include "logviewer_export.h"
#include "connectionsettings.h"

#include <QDialog>
#include <QFileDialog>

class QLineEdit;
class QPushButton;

class FileNameSelectorWidget : public QWidget
{
	Q_OBJECT
  public:
	explicit FileNameSelectorWidget(QWidget* parent = 0);
	~FileNameSelectorWidget();
	QString text() const;
	void setText(const QString&);

  private slots:
	void slotPathChanged();

  private:
	QLineEdit* m_path;
	QPushButton* m_selectPath;
	QFileDialog _file_dlg;
};

class LOGVIEWER_EXPORT FileConnectionDialog : public QDialog
{
	Q_OBJECT
  public:
	explicit FileConnectionDialog(QWidget* parent);
	~FileConnectionDialog();

	RegisteredFile registeredFileSettings() const;

	void initialize(const RegisteredFile& registered, const QString& hostname);
  private slots:
	void slotTextChanged(const QString& text);
	void slotProtocolChanged(int index);

  private:
	class FileConnectionDialogPrivate;
	FileConnectionDialogPrivate* d;
};

#endif /* FILECONNECTIONDIALOG_H */