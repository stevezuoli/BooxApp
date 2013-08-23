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


#ifndef __SQLITEDATABASE_H__
#define __SQLITEDATABASE_H__

#include "../DataBase.h"
#include "../DBCommand.h"

#include "SQLiteFactory.h"


class SQLiteDataBase : public DataBase {

public:
	SQLiteDataBase(const std::string &path);
	virtual ~SQLiteDataBase();
	
	bool open();
	void close();

public:
	bool executeAsTransaction(DBRunnable &runnable);

private:
	friend class Transaction;

	class Transaction {
	public:
		Transaction(SQLiteDataBase &db);
		~Transaction();
	public:
		bool start();
		void setSuccessful();
	private:
		void end(bool success);
		std::string name() const;
	private:
		SQLiteDataBase &myDataBase;
		bool mySuccess;
		bool myStarted;
		unsigned myDepth;
	};

private: // Transaction handling
	unsigned myTransactionDepth;
	fb::shared_ptr<DBCommand> myBeginTransaction;
	fb::shared_ptr<DBCommand> myCommitTransaction;
	fb::shared_ptr<DBCommand> myRollbackTransaction;
	fb::shared_ptr<DBCommand> myMakeSavePoint;
	fb::shared_ptr<DBCommand> myCommitSavePoint;
	fb::shared_ptr<DBCommand> myRollbackSavePoint;

private: // disable copying:
	SQLiteDataBase(const SQLiteDataBase &);
	const SQLiteDataBase &operator = (const SQLiteDataBase &);
};

inline bool SQLiteDataBase::open() { return connection().open(); }
inline void SQLiteDataBase::close() { connection().close(); }

inline void SQLiteDataBase::Transaction::setSuccessful() { mySuccess = true; }


#endif /* __SQLITEDATABASE_H__ */
