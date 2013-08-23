/*
 * Copyright (C) 2009 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#ifndef __DBDATAREADER_H__
#define __DBDATAREADER_H__

#include "DBValues.h"

class DBDataReader {

public:
	DBDataReader();
	virtual ~DBDataReader();

public: // to implement:
	virtual bool next() = 0;
	virtual bool reset() = 0;
	virtual void close() = 0;

	virtual unsigned columnsNumber() const = 0;
	
	virtual DBValue::ValueType type(unsigned column) const = 0;
	virtual fb::shared_ptr<DBValue> value(unsigned column) const = 0;

	virtual int intValue(unsigned column) const = 0;
	virtual double realValue(unsigned column) const = 0;
	virtual std::string textValue(unsigned column) const = 0;

public:
	bool isDBNull(unsigned column) const;
	bool isInt(unsigned column) const;
	bool isReal(unsigned column) const;
	bool isText(unsigned column) const;
};


inline bool DBDataReader::isDBNull(unsigned column) const { return type(column) == DBValue::DBNULL; }
inline bool DBDataReader::isInt(unsigned column) const { return type(column) == DBValue::DBINT; }
inline bool DBDataReader::isReal(unsigned column) const { return type(column) == DBValue::DBREAL; }
inline bool DBDataReader::isText(unsigned column) const { return type(column) == DBValue::DBTEXT; }

#endif /* __DBDATAREADER_H__ */

