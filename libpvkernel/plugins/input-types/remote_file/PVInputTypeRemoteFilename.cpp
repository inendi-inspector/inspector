//
// MIT License
//
// © ESI Group, 2015
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "PVInputTypeRemoteFilename.h"
#include "include/PVLogViewerDialog.h"

#include <pvkernel/rush/PVFileDescription.h>

#include <pvbase/general.h>

#include <QMessageBox>
#include <QFileInfo>

#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>

PVRush::PVInputTypeRemoteFilename::PVInputTypeRemoteFilename() : PVInputTypeFilename() {}

bool PVRush::PVInputTypeRemoteFilename::createWidget(hash_formats& formats,
                                                     list_inputs& inputs,
                                                     QString& format,
                                                     PVCore::PVArgumentList& /*args_ext*/,
                                                     QWidget* parent) const
{
	QStringList formats_str = formats.keys();

	formats_str.prepend(INENDI_BROWSE_FORMAT_STR);
	PVLogViewerDialog* RemoteLogDialog = new PVLogViewerDialog(formats_str, parent);
	if (RemoteLogDialog->exec() == QDialog::Rejected) {
		RemoteLogDialog->deleteLater();
		return false;
	}
	// Force deletion so that settings are saved
	RemoteLogDialog->deleteLater();

	_hash_real_filenames = RemoteLogDialog->getDlFiles();
	QStringList const& files = _hash_real_filenames.keys();
	for (int i = 0; i < files.size(); i++) {
		PVLOG_INFO("%s\n", qPrintable(files[i]));
	}
	format = RemoteLogDialog->getSelFormat();
	return load_files(files, inputs, parent);
}

QString PVRush::PVInputTypeRemoteFilename::name() const
{
	return QString("remote_file");
}

QString PVRush::PVInputTypeRemoteFilename::human_name() const
{
	return QString("Remote file import plugin");
}

QString PVRush::PVInputTypeRemoteFilename::human_name_serialize() const
{
	return tr("Remote files");
}

QString PVRush::PVInputTypeRemoteFilename::internal_name() const
{
	return QString("01-remote_file");
}
QString PVRush::PVInputTypeRemoteFilename::human_name_of_input(PVInputDescription_p in) const
{
	PVFileDescription* f = dynamic_cast<PVFileDescription*>(in.get());
	assert(f);
	QString fn = f->path();
	if (_hash_real_filenames.contains(fn)) {
		return _hash_real_filenames.value(fn).toString();
	}
	return fn;
}

QString PVRush::PVInputTypeRemoteFilename::tab_name_of_inputs(list_inputs const& in) const
{
	if (in.size() == 1) {
		return human_name_of_input(in[0]);
	}

	bool found_url = false;
	QUrl url;
	for (int i = 0; i < in.size(); i++) {
		PVFileDescription* f = dynamic_cast<PVFileDescription*>(in[i].get());
		assert(f);
		QString tmp_name = f->path();
		if (_hash_real_filenames.contains(tmp_name)) {
			found_url = true;
			url = _hash_real_filenames[tmp_name];
			break;
		}
	}
	if (!found_url) {
		return PVInputTypeFilename::tab_name_of_inputs(in);
	}

	return url.toString(QUrl::RemovePassword | QUrl::RemovePath | QUrl::RemoveQuery |
	                    QUrl::StripTrailingSlash);
}

QString PVRush::PVInputTypeRemoteFilename::menu_input_name() const
{
	return QString("Remote files...");
}

bool PVRush::PVInputTypeRemoteFilename::get_custom_formats(PVInputDescription_p /*in*/,
                                                           hash_formats& /*formats*/) const
{
	return false;
}

QKeySequence PVRush::PVInputTypeRemoteFilename::menu_shortcut() const
{
	return QKeySequence();
}
