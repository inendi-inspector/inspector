/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVELASTICSEARCHINFOS_H
#define PVELASTICSEARCHINFOS_H

#include <pvkernel/core/PVSerializeArchive.h>

#include <QString>

namespace PVRush
{

class PVElasticsearchInfos
{
	friend class PVCore::PVSerializeObject;

  public:
	PVElasticsearchInfos();

  public:
	void set_host(QString const& host) { _host = host; }
	void set_port(uint16_t port) { _port = port; }
	void set_index(QString const& index) { _index = index; }
	void set_importer(QString const& importer) { _importer = importer; }
	void set_login(QString const& login) { _login = login; }
	void set_password(QString const& password) { _password = password; }

	QString const& get_host() const { return _host; }
	uint16_t get_port() const { return _port; }
	QString const& get_index() const { return _index; }
	QString const& get_importer() const { return _importer; }
	QString const& get_login() const { return _login; }
	QString const& get_password() const { return _password; }

	inline bool operator==(PVElasticsearchInfos const& o) const
	{
		return _host == o._host && _port == o._port && _index == o._index && _login == o._login &&
		       _importer == o._importer && _password == o._password;
	}

  protected:
	void serialize(PVCore::PVSerializeObject& so, PVCore::PVSerializeArchive::version_t v);

  protected:
	QString _host;
	uint16_t _port;
	QString _index;
	QString _importer;
	QString _login;
	QString _password;
};
}

#endif
