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



#ifndef __DBRUNNABLES_H__
#define __DBRUNNABLES_H__

#include <deque>
#include <vector>

#include "../../fbreader/ReadingState.h"

#include "../sqldb/DBConnection.h"
#include "../sqldb/DBCommand.h"
#include "../sqldb/DBRunnable.h"

#include "DBBook.h"
#include "BooksDBQuery.h"


class FindFileIdRunnable;
class LoadAuthorsRunnable;
class LoadTagsRunnable;
class LoadSeriesRunnable;
class LoadFileEntriesRunnable;
class DeleteFileEntriesRunnable;

/*
 * Save Runnables
 */
 

class InitBooksDBRunnable : public DBRunnable {

public:
	InitBooksDBRunnable(DBConnection &connection);
	bool run();

private:
	DBConnection &myConnection;
};

class ClearBooksDBRunnable : public DBRunnable {

public:
	ClearBooksDBRunnable(DBConnection &connection);
	bool run();

private:
	DBConnection &myConnection;
};

class SaveTableBookRunnable : public DBRunnable {

public:
	SaveTableBookRunnable(DBConnection &connection);
	bool run();
	void setBook(fb::shared_ptr<DBBook> book);

private:
	bool addTableBook(const fb::shared_ptr<DBBook> book, int fileId);
	bool updateTableBook(const fb::shared_ptr<DBBook> book);

private:
	fb::shared_ptr<DBBook> myBook;

	fb::shared_ptr<DBCommand> myFindBookId;

	fb::shared_ptr<DBCommand> myAddBook;
	fb::shared_ptr<DBCommand> myUpdateBook;

	fb::shared_ptr<FindFileIdRunnable> myFindFileId;
};

class SaveAuthorsRunnable : public DBRunnable {

public:
	SaveAuthorsRunnable(DBConnection &connection);
	bool run();
	void setBook(fb::shared_ptr<DBBook> book);

private:
	fb::shared_ptr<DBBook> myBook;

	fb::shared_ptr<DBCommand> mySetBookAuthor;
	fb::shared_ptr<DBCommand> myTrimBookAuthors;

	fb::shared_ptr<DBCommand> myFindAuthorId;
	fb::shared_ptr<DBCommand> myAddAuthor;
};

class SaveTagsRunnable : public DBRunnable {

public:
	SaveTagsRunnable(DBConnection &connection);
	bool run();
	void setBook(fb::shared_ptr<DBBook> book);

private:
	int findTagId(DBTag &tag);

private:
	fb::shared_ptr<DBBook> myBook;

	fb::shared_ptr<DBCommand> myAddBookTag;
	fb::shared_ptr<DBCommand> myDeleteBookTag;

	fb::shared_ptr<DBCommand> myFindTagId;
	fb::shared_ptr<DBCommand> myAddTag;

	fb::shared_ptr<LoadTagsRunnable> myLoadTags;
};


class SaveSeriesRunnable : public DBRunnable {

public:
	SaveSeriesRunnable(DBConnection &connection);
	bool run();
	void setBook(fb::shared_ptr<DBBook> book);

private:
	fb::shared_ptr<DBBook> myBook;

	fb::shared_ptr<DBCommand> mySetBookSeries;
	fb::shared_ptr<DBCommand> myDeleteBookSeries;

	fb::shared_ptr<DBCommand> myFindSeriesId;
	fb::shared_ptr<DBCommand> myAddSeries;
};

class SaveBookRunnable : public DBRunnable {
public:
	SaveBookRunnable(SaveTableBookRunnable &saveTableBook, SaveAuthorsRunnable &saveAuthors, 
		SaveSeriesRunnable &saveSeries, SaveTagsRunnable &saveTags);

	bool run();
	void setBook(fb::shared_ptr<DBBook> book);

private:
	SaveTableBookRunnable &mySaveTableBook;
	SaveAuthorsRunnable &mySaveAuthors;
	SaveSeriesRunnable &mySaveSeries;
	SaveTagsRunnable &mySaveTags;
};

class SaveFileEntriesRunnable : public DBRunnable {

public:
	SaveFileEntriesRunnable(DBConnection &connection);
	bool run();
	void setEntries(const std::string &fileName, const std::vector<std::string> &entries);

private:
	std::string myFileName;
	std::vector<std::string> myEntries;

	fb::shared_ptr<DBCommand> myAddFile;

	fb::shared_ptr<FindFileIdRunnable> myFindFileId;
	fb::shared_ptr<DeleteFileEntriesRunnable> myDeleteFileEntries;
};

class SaveRecentBooksRunnable : public DBRunnable {

public:
	SaveRecentBooksRunnable(DBConnection &connection);
	bool run();
	void setBooks(const std::vector<fb::shared_ptr<DBBook> > &books);

private:
	std::vector<fb::shared_ptr<DBBook> > myBooks;

	fb::shared_ptr<DBCommand> myClearRecentBooks;
	fb::shared_ptr<DBCommand> myInsertRecentBooks;
};

class SaveBookStateStackRunnable : public DBRunnable {

public:
	SaveBookStateStackRunnable(DBConnection &connection);
	bool run();
	void setState(int bookId, const std::deque<ReadingState > &stack);

private:
	int myBookId;
	std::deque<ReadingState > myStack;

	fb::shared_ptr<DBCommand> myTrimBookStateStack;
	fb::shared_ptr<DBCommand> mySetBookStateStack;
};

class DeleteFileEntriesRunnable : public DBRunnable {

public:
	DeleteFileEntriesRunnable(DBConnection &connection);
	bool run();
	void setFileId(int fileId);

private:
	bool doDelete(int fileId);

private:
	int myFileId;

	fb::shared_ptr<DBCommand> myDeleteFileEntries;
	fb::shared_ptr<DBCommand> myLoadFileEntryIds;
};

class DeleteBookRunnable : public DBRunnable {

public:
	DeleteBookRunnable(DBConnection &connection);
	bool run();
	void setFileName(const std::string &fileName);

private:
	std::string myFileName;

	fb::shared_ptr<FindFileIdRunnable> myFindFileId;
	fb::shared_ptr<DBCommand> myDeleteFile;
};


inline InitBooksDBRunnable::InitBooksDBRunnable(DBConnection &connection) : myConnection(connection) {}
inline ClearBooksDBRunnable::ClearBooksDBRunnable(DBConnection &connection) : myConnection(connection) {}

inline void SaveTableBookRunnable::setBook(fb::shared_ptr<DBBook> book) { myBook = book; }
inline void SaveAuthorsRunnable::setBook(fb::shared_ptr<DBBook> book) { myBook = book; }
inline void SaveTagsRunnable::setBook(fb::shared_ptr<DBBook> book) { myBook = book; }
inline void SaveSeriesRunnable::setBook(fb::shared_ptr<DBBook> book) { myBook = book; }

inline void SaveBookRunnable::setBook(fb::shared_ptr<DBBook> book) {
	mySaveTableBook.setBook(book);
	mySaveAuthors.setBook(book);
	mySaveTags.setBook(book);
	mySaveSeries.setBook(book);
}

inline void SaveFileEntriesRunnable::setEntries(const std::string &fileName, const std::vector<std::string> &entries) {
	myFileName = fileName;
	myEntries  = entries; // copy vector
}

inline void SaveRecentBooksRunnable::setBooks(const std::vector<fb::shared_ptr<DBBook> > &books) {
	myBooks = books; // copy vector
}

inline void SaveBookStateStackRunnable::setState(int bookId, const std::deque<ReadingState > &stack) {
	myBookId = bookId;
	myStack = stack; // copy deque
}

inline void DeleteFileEntriesRunnable::setFileId(int fileId) { myFileId = fileId; }

inline void DeleteBookRunnable::setFileName(const std::string &fileName) { myFileName = fileName; }

/*
 * Load & Modify Runnables
 */

class FindFileIdRunnable : public DBRunnable {

public:
	FindFileIdRunnable(DBConnection &connection);
	bool run();
	void setFileName(const std::string &fileName, bool add = false);

	int fileId() const;

private:
	std::string myFileName;
	bool myAdd;
	int myFileId;

	fb::shared_ptr<DBCommand> myFindFileId;
	fb::shared_ptr<DBCommand> myAddFile;
};


inline void FindFileIdRunnable::setFileName(const std::string &fileName, bool add) {
	myFileName = fileName;
	myAdd = add;
	myFileId = 0;
}

inline int FindFileIdRunnable::fileId() const { return myFileId; }


/*
 * Load Runnables
 */

class LoadAuthorsRunnable : public DBRunnable {

public:
	LoadAuthorsRunnable(DBConnection &connection);
	bool run();
	void setBookId(int bookId);
	void collectAuthors(std::vector<fb::shared_ptr<DBAuthor> > &authors);

private:
	int myBookId;
	std::vector<fb::shared_ptr<DBAuthor> > myAuthors;

	fb::shared_ptr<DBCommand> myLoadAuthors;
};

class LoadTagsRunnable : public DBRunnable {

public:
	LoadTagsRunnable(DBConnection &connection);
	bool run();
	void setBookId(int bookId);
	void collectTags(std::vector<fb::shared_ptr<DBTag> > &tags);

private:
	int myBookId;
	std::vector<fb::shared_ptr<DBTag> > myTags;

	fb::shared_ptr<DBCommand> myLoadTags;
	fb::shared_ptr<DBCommand> myFindParentTag;
};

class LoadSeriesRunnable : public DBRunnable {

public:
	LoadSeriesRunnable(DBConnection &connection);
	bool run();
	void setBookId(int bookId);
	const std::string &seriesName() const;
	int numberInSeries() const;

private:
	int myBookId;
	std::string mySeriesName;
	int myNumberInSeries;

	fb::shared_ptr<DBCommand> myLoadSeries;
};

class LoadFileEntriesRunnable : public DBRunnable {

public:
	LoadFileEntriesRunnable(DBConnection &connection);
	bool run();
	void setFileName(const std::string &fileName);
	void collectEntries(std::vector<std::string> &entries);

private:
	std::string myFileName;
	std::vector<std::string> myEntries;

	fb::shared_ptr<FindFileIdRunnable> myFindFileId;

	fb::shared_ptr<DBCommand> myLoadFileEntries;
};

class LoadRecentBooksRunnable : public DBRunnable {

public:
	LoadRecentBooksRunnable(DBConnection &connection);
	bool run();
	void collectFileIds(std::vector<int> &fileIds);

private:
	std::vector<int> myFileIds;

	fb::shared_ptr<DBCommand> myLoadRecentBooks;
};



inline void LoadAuthorsRunnable::setBookId(int bookId) { myBookId = bookId; }
inline void LoadAuthorsRunnable::collectAuthors(std::vector<fb::shared_ptr<DBAuthor> > &authors) {
	myAuthors.swap(authors);
	myAuthors.clear();
}


inline void LoadTagsRunnable::setBookId(int bookId) { myBookId = bookId; }
inline void LoadTagsRunnable::collectTags(std::vector<fb::shared_ptr<DBTag> > &tags) {
	myTags.swap(tags);
	myTags.clear();
}


inline void LoadSeriesRunnable::setBookId(int bookId) { myBookId = bookId; }
inline const std::string &LoadSeriesRunnable::seriesName() const { return mySeriesName; }
inline int LoadSeriesRunnable::numberInSeries() const { return myNumberInSeries; }


inline void LoadFileEntriesRunnable::setFileName(const std::string &fileName) { myFileName = fileName; }
inline void LoadFileEntriesRunnable::collectEntries(std::vector<std::string> &entries) {
	myEntries.swap(entries);
	myEntries.clear();
}

inline void LoadRecentBooksRunnable::collectFileIds(std::vector<int> &fileIds) {
	myFileIds.swap(fileIds);
	myFileIds.clear();
}


#endif /* __DBRUNNABLES_H__ */
