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


#ifndef __BOOKSDBUTIL_H__
#define __BOOKSDBUTIL_H__

#include <map>

#include "BooksDB.h"
#include "DBBook.h"


class ZLFile;

class BooksDBUtil {

public:
	static fb::shared_ptr<DBBook> getBook(const std::string &fileName, bool checkFile = true);

	static bool getBooks(std::map<std::string, fb::shared_ptr<DBBook> > &booksmap, bool checkFile = true);

	static bool getRecentBooks(std::vector<fb::shared_ptr<DBBook> > &books);

public:
	static void fixFB2Encoding(const DBBook &book);

	static void addTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> tag);
	static void removeTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> tag, bool includeSubTags);
	static void renameTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags);
	static void cloneTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags);
	static void removeAllTags(fb::shared_ptr<DBBook> book);

	static bool checkInfo(const ZLFile &file);
	static void saveInfo(const ZLFile &file);

	static void listZipEntries(const ZLFile &zipFile, std::vector<std::string> &entries);
	static void resetZipInfo(const ZLFile &zipFile);

	static bool isBookFull(const DBBook &book);

	static void updateAuthor(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBAuthor> from, fb::shared_ptr<DBAuthor> to);

	static bool canRemoveFile(const std::string &fileName);

private:
	static fb::shared_ptr<DBBook> loadFromDB(const std::string &fileName);
};


inline void BooksDBUtil::addTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> tag) {
	if (book->addTag(tag)) {
		BooksDB::instance().saveTags(book);
	}
}

inline void BooksDBUtil::removeTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> tag, bool includeSubTags) {
	if (book->removeTag(tag, includeSubTags)) {
		BooksDB::instance().saveTags(book);
	}
}

inline void BooksDBUtil::renameTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags) {
	if (book->renameTag(from, to, includeSubTags)) {
		BooksDB::instance().saveTags(book);
	}
}

inline void BooksDBUtil::cloneTag(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags) {
	if (book->cloneTag(from, to, includeSubTags)) {
		BooksDB::instance().saveTags(book);
	}
}

inline void BooksDBUtil::removeAllTags(fb::shared_ptr<DBBook> book) {
	book->removeAllTags();
}

inline void BooksDBUtil::updateAuthor(fb::shared_ptr<DBBook> book, fb::shared_ptr<DBAuthor> from, fb::shared_ptr<DBAuthor> to) {
	if (book->updateAuthor(from, to)) {
		BooksDB::instance().saveAuthors(book);
	}
}


#endif /* __BOOKSDBUTIL_H__ */
