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

#ifndef LOGVIEWERWIDGET_H
#define LOGVIEWERWIDGET_H

#include "logviewer_export.h"
#include <QWidget>
#include <QUrl>

class QAction;
class QListWidgetItem;
class QTableWidgetItem;

/**
 * Log viewer widget.
 * @author KDAB
 */
class LOGVIEWER_EXPORT LogViewerWidget : public QWidget
{
	Q_OBJECT
  public:
	/**
	 * Create a log viewer widget
	 * @param parent The parent of Widget
	 */
	explicit LogViewerWidget(QWidget* parent = 0);

	/**
	 * Destroys the widget
	 */
	~LogViewerWidget();

	/**
	 * Return a QAction for adding a new machine.
	 * You should add this action to a menu.
	 */
	QAction* addMachineAction() const;
	/**
	 * Return a QAction for removing a machine.
	 * You should add this action to a menu.
	 */
	QAction* removeMachineAction() const;

	/**
	 * Save settings, this method saves list of machines and list of files
	 * on each machine.
	 * You should call this when the application is about to quit.
	 */
	void saveSettings();
	/**
	 * Load settings, this method loads the list of machines and list of files
	 * on each machine.
	 * You should call this after creating the widget.
	 */
	void loadSettings();

	/**
	 * Encrypt Password, this method encrypts password.
	 */
	QString encryptPassword(const QString& password);
	/**
	 * Unencrypt Password, this method unencrypts password.
	 */
	QString unencryptPassword(const QString& password);

	/**
	 * Load ssh key file, this method returns ssh key file when we load it
	 */
	QString loadSshKeyFile(const QString& sshKeyFile);
	/**
	 * Save ssh key file, this method returns ssh key file when we save it
	 */
	QString saveSshKeyFile(const QString& sshKeyFile);

	/**
	 * Load certificate file, this method returns certificate file when we load it.
	 */
	QString loadCertificateFile(const QString& certificateFile);
	/**
	 * Save certificate file, this method returns certificate file when we save it.
	 */
	QString saveCertificateFile(const QString& certificateFile);

	/**
	 * Download the selected files and store their temporary location
	 * @params[out] dl_files A hash whose keys are the temporary files and values a displayable name
	 * of the original one
	 * @return true if the download has been successful
	 */
	bool downloadSelectedFiles(QHash<QString, QUrl>& dl_files);

  Q_SIGNALS:
	/**
	 * This signal is emitted once a file has been downloaded.
	 * @param machine the name of machine where original file is stored
	 * @param remoteFile the name of remote file
	 * @param localPath the local file name
	 */
	void newFile(const QString& machine, const QString& remoteFile, const QString& localFile);

	/**
	 * This signal is emitted when an error happened during download
	 * @param errorMsg string message
	 * @param curlErrorCode the error code returned by libcurl
	 */
	void downloadError(const QString& errorMsg, int curlErrorCode);

	//// Below are only implementation details, not part of the public API ////

  private Q_SLOTS:
	/**
	 * This slot is called when the user wants to add a new file
	 */
	void slotAddFile();
	/**
	 * This slot is called when the user wants to remove a file
	 */
	void slotRemoveFile();
	/**
	 * This slot is called when the user wants to change the settings for a file
	 */
	void slotConfigureFile();

	/**
	 * This slot is called when the user wants to declare a new machine
	 */
	void slotAddMachine();
	/**
	 * This slot is called when the user wants to remove an existing machine
	 */
	void slotRemoveMachine();

  private Q_SLOTS:
	void slotFillFilesList(QListWidgetItem*);
	void slotUpdateButtons();

  private:
	class LogViewerWidgetPrivate;
	LogViewerWidgetPrivate* d;
};

#endif /* LOGVIEWERWIDGET_H */
