/**
 * @file
 *
 * @copyright (C) Picviz Labs 2009-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef INENDI_PVPLOTTINGRANGETYPE_H
#define INENDI_PVPLOTTINGRANGETYPE_H

#include <pvkernel/core/general.h>

#include <QMetaType>

namespace Inendi {

class PVPlottingRangeType
{
public:
	enum {
		RangeUserDefined = 0,
		RangeAxisMinMax,
		RangeGroupMinMax
	} RangeType;

public:
	PVPlottingRangeType():
		_type(RangeAxisMinMax),
		_user_min(0.0),
		_user_max(0.0)
	{ }

	PVPlottingRangeType(QString const& group_name):
		_type(RangeGroupMinMax),
		_group_name(group_name)
	{ }

	PVPlottingRangeType(double min, double max):
		_type(RangeUserDefined),
		_user_min(min),
		_user_max(max)
	{ }

public:
	inline RangeType type() const { return _type; }
	inline double user_min() const
	{
		assert(_type == RangeUserDefined);
		return _user_min;
	}

	inline double user_max() const
	{
		assert(_type == RangeUserDefined);
		return _user_min;
	}
	QString const& group_name() const
	{
		assert(_type == RangeGroupMinMax);
		return _group_name;
	}

private:
	RangeType _type;
	double _user_min;
	double _user_max;
	QString _group_name;
};

}

// WARNING : This declaration MUST BE outside namespace's scope
Q_DECLARE_METATYPE(Inendi::PVPlottingRangeType)


#endif // PVCORE_PVAXESINDEXTYPE_H
