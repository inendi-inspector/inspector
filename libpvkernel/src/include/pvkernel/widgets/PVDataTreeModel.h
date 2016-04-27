/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVVIEWSMODEL_H
#define PVVIEWSMODEL_H

#include <pvkernel/core/general.h>
#include <QAbstractItemModel>

// Forward declaration
namespace PVCore
{
class PVDataTreeObjectBase;
class PVDataTreeObjectWithChildrenBase;
}

namespace PVWidgets
{

class PVDataTreeModel : public QAbstractItemModel
{
  public:
	PVDataTreeModel(PVCore::PVDataTreeObjectBase& root, QObject* parent = 0);

  public:
	QVariant data(const QModelIndex& index, int role) const;
	int rowCount(const QModelIndex& index) const;
	int columnCount(const QModelIndex& index) const;
	QModelIndex parent(const QModelIndex& index) const;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;

  public:
	PVCore::PVDataTreeObjectBase* get_object(QModelIndex const& index) const;

  protected:
	QModelIndex index_from_obj(PVCore::PVDataTreeObjectBase const* obj) const;

  private:
	QModelIndex index_from_obj_rec(QModelIndex const& cur,
	                               PVCore::PVDataTreeObjectWithChildrenBase const* idx_obj,
	                               PVCore::PVDataTreeObjectBase const* obj_test) const;

  protected:
	PVCore::PVDataTreeObjectWithChildrenBase* _root;
	PVCore::PVDataTreeObjectBase* _root_base;
};
};

#endif
