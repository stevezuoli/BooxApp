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



#ifndef __BOOKSDB_H__
#define __BOOKSDB_H__

#include <set>
#include <deque>

#include "../sqldb/implsqlite/SQLiteDataBase.h"
#include "DBBook.h"
#include "DBRunnables.h"

#include "../../fbreader/ReadingState.h"

class BooksDB : public SQLiteDataBase {

public:
	static const std::string DATABASE_NAME;
	static const std::string STATE_DATABASE_NAME;
	static const std::string NET_DATABASE_NAME;

	static BooksDB &instance();

private:
	static fb::shared_ptr<BooksDB> ourInstance;

	BooksDB(const std::string &path);

public:
	virtual ~BooksDB();

public:
	bool initDatabase();
	bool isInitialized() const;
	bool clearDatabase();

	fb::shared_ptr<DBBook> loadBook(const std::string &fileName);
	bool saveBook(const fb::shared_ptr<DBBook> book);
	bool saveAuthors(const fb::shared_ptr<DBBook> book);
	bool saveSeries(const fb::shared_ptr<DBBook> book);
	bool saveTags(const fb::shared_ptr<DBBook> book);

	int getFileSize(const std::string fileName);
	bool setFileSize(const std::string fileName, int size);

	bool setEncoding(const DBBook &book, const std::string &encoding);

	bool loadFileEntries(const std::string &fileName, std::vector<std::string> &entries);
	bool saveFileEntries(const std::string &fileName, const std::vector<std::string> &entries);

	bool loadRecentBooks(std::vector<std::string> &fileNames);
	bool saveRecentBooks(const std::vector<fb::shared_ptr<DBBook> > &books);

	bool collectSeriesNames(const DBAuthor &author, std::set<std::string> &list);
	bool collectAuthorNames(std::vector<fb::shared_ptr<DBAuthor> > &authors);

	bool loadBooks(std::vector<fb::shared_ptr<DBBook> > &books);

	bool loadBookStateStack(const DBBook &book, std::deque<ReadingState> &stack);
	bool saveBookStateStack(const DBBook &book, const std::deque<ReadingState> &stack);

	bool removeBook(const DBBook &book);

	std::string getPalmType(const std::string &fileName);
	bool setPalmType(const std::string &fileName, const std::string &type);

	std::string getNetFile(const std::string &url);
	bool setNetFile(const std::string &url, const std::string &fileName);

	bool loadBookState(const DBBook &book, ReadingState &state);
	bool setBookState(const DBBook &book, const ReadingState &state);

	int loadStackPos(const DBBook &book);
	bool setStackPos(const DBBook &book, int stackPos);
	
	bool insertIntoBookList(const DBBook &book);
	bool deleteFromBookList(const DBBook &book);
	bool checkBookList(const DBBook &book);

private:
	fb::shared_ptr<DBBook> loadTableBook(const std::string fileName);
	bool loadAuthors(int bookId, std::vector<fb::shared_ptr<DBAuthor> > &authors);
	bool loadSeries(int bookId, std::string &seriesName, int &numberInSeries);

	std::string getFileName(int fileId);

private:
	void initCommands();

private:
	bool myInitialized;

	fb::shared_ptr<SaveTableBookRunnable> mySaveTableBook;
	fb::shared_ptr<SaveAuthorsRunnable> mySaveAuthors;
	fb::shared_ptr<SaveSeriesRunnable> mySaveSeries;
	fb::shared_ptr<SaveTagsRunnable> mySaveTags;
	fb::shared_ptr<SaveBookRunnable> mySaveBook;
	fb::shared_ptr<SaveFileEntriesRunnable> mySaveFileEntries;

	fb::shared_ptr<FindFileIdRunnable> myFindFileId;

	fb::shared_ptr<LoadAuthorsRunnable> myLoadAuthors;
	fb::shared_ptr<LoadTagsRunnable> myLoadTags;
	fb::shared_ptr<LoadSeriesRunnable> myLoadSeries;
	fb::shared_ptr<LoadFileEntriesRunnable> myLoadFileEntries;

	fb::shared_ptr<LoadRecentBooksRunnable> myLoadRecentBooks;
	fb::shared_ptr<SaveRecentBooksRunnable> mySaveRecentBooks;

	fb::shared_ptr<SaveBookStateStackRunnable> mySaveBookStateStack;
	
	fb::shared_ptr<DeleteBookRunnable> myDeleteBook;

	fb::shared_ptr<DBCommand> myLoadBook;

	fb::shared_ptr<DBCommand> myGetFileSize;
	fb::shared_ptr<DBCommand> mySetFileSize;

	fb::shared_ptr<DBCommand> myFindFileName;

	fb::shared_ptr<DBCommand> myFindAuthorId;
	fb::shared_ptr<DBCommand> myLoadSeriesNames;

	fb::shared_ptr<DBCommand> myLoadAuthorNames;

	fb::shared_ptr<DBCommand> myLoadBooks;

	fb::shared_ptr<DBCommand> myLoadBookStateStack;

	fb::shared_ptr<DBCommand> myGetPalmType;
	fb::shared_ptr<DBCommand> mySetPalmType;

	fb::shared_ptr<DBCommand> myGetNetFile;
	fb::shared_ptr<DBCommand> mySetNetFile;

	fb::shared_ptr<DBCommand> myLoadStackPos;
	fb::shared_ptr<DBCommand> mySetStackPos;
	fb::shared_ptr<DBCommand> myLoadBookState;
	fb::shared_ptr<DBCommand> mySetBookState;

	fb::shared_ptr<DBCommand> myInsertBookList;
	fb::shared_ptr<DBCommand> myDeleteBookList;
	fb::shared_ptr<DBCommand> myCheckBookList;

private: // disable copying
	BooksDB(const BooksDB &);
	const BooksDB &operator = (const BooksDB &);
};


inline bool BooksDB::isInitialized() const { return myInitialized; }

#endif /* __BOOKSDB_H__ */
