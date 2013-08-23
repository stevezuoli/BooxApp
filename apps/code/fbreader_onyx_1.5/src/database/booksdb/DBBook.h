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


#ifndef __DBBOOK_H__
#define __DBBOOK_H__

#include <string>
#include <vector>
#include <shared_ptr.h>

#include "DBAuthor.h"
#include "DBTag.h"

class DBBook {

friend class BooksDB;
friend class BooksDBUtil;

friend class SaveTableBookRunnable;
friend class SaveAuthorsRunnable;
friend class SaveTagsRunnable;
friend class SaveSeriesRunnable;
friend class SaveRecentBooksRunnable;
friend class SaveBookStateRunnable;
friend class Migration_0_11_0_Runnable;

public:
	static fb::shared_ptr<DBBook> loadFromFile(const std::string &fileName);
	static fb::shared_ptr<DBBook> loadFromBookInfo(const std::string &fileName);

	//static void removeWhiteSpacesFromTag(std::string &tag);

protected:
	DBBook(const std::string filename);

public: // unmodifiable book methods
	const std::string &title() const;
	const std::string &fileName() const;
	const std::string &language() const;
	const std::string &encoding() const;
	const std::string &seriesName() const;
	int numberInSeries() const;

	const std::vector<fb::shared_ptr<DBTag> > &tags() const;
	const std::vector<fb::shared_ptr<DBAuthor> > &authors() const;

public: // modifiable book methods
	void setTitle(const std::string &title);
	void setLanguage(const std::string &language);
	void setEncoding(const std::string &encoding);
	void setSeriesName(const std::string &name);
	void setNumberInSeries(int num);

	std::vector<fb::shared_ptr<DBTag> > &tags();
	std::vector<fb::shared_ptr<DBAuthor> > &authors();

	void appendTitle(const std::string &str);
	void appendLanguage(const std::string &str);
	void appendEncoding(const std::string &str);
	void appendSeriesName(const std::string &str);

public:
	bool addTag(fb::shared_ptr<DBTag> tag);
	bool removeTag(fb::shared_ptr<DBTag> tag, bool includeSubTags);
	bool renameTag(fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags);
	bool cloneTag(fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags);
	void removeAllTags();

	std::string authorDisplayName() const;
	std::string authorSortKey() const;

	bool updateAuthor(fb::shared_ptr<DBAuthor> from, fb::shared_ptr<DBAuthor> to);

protected:
	unsigned bookId() const;
	void setBookId(unsigned bookId);

private:
	unsigned myBookId;

	const std::string myFileName;
	std::string myTitle;
	std::string myLanguage;
	std::string myEncoding;
	std::string mySeriesName;
	int myNumberInSeries;
	std::vector<fb::shared_ptr<DBTag> > myTags;
	std::vector<fb::shared_ptr<DBAuthor> > myAuthors;

private: // disable copying
	DBBook(const DBBook &);
	const DBBook &operator = (const DBBook &);
};

inline DBBook::DBBook(const std::string filename) : myBookId(0), myFileName(filename), myNumberInSeries(0) {}

inline const std::string &DBBook::title() const { return myTitle; }
inline const std::string &DBBook::fileName() const { return myFileName; }
inline const std::string &DBBook::language() const { return myLanguage; }
inline const std::string &DBBook::encoding() const { return myEncoding; }
inline const std::string &DBBook::seriesName() const { return mySeriesName; }
inline int DBBook::numberInSeries() const { return myNumberInSeries; }

inline const std::vector<fb::shared_ptr<DBTag> > &DBBook::tags() const { return myTags; }
inline const std::vector<fb::shared_ptr<DBAuthor> > &DBBook::authors() const { return myAuthors; }

inline void DBBook::setTitle(const std::string &title) { myTitle = title; }
inline void DBBook::setLanguage(const std::string &language) { myLanguage = language; }
inline void DBBook::setEncoding(const std::string &encoding) { myEncoding = encoding; }
inline void DBBook::setSeriesName(const std::string &name) { mySeriesName = name; }
inline void DBBook::setNumberInSeries(int num) { myNumberInSeries = num; }

inline void DBBook::appendTitle(const std::string &str) { myTitle += str; }
inline void DBBook::appendLanguage(const std::string &str) { myLanguage += str; }
inline void DBBook::appendEncoding(const std::string &str) { myEncoding += str; }
inline void DBBook::appendSeriesName(const std::string &str) { mySeriesName += str; }

inline void DBBook::removeAllTags() { myTags.clear(); }

inline std::vector<fb::shared_ptr<DBTag> > &DBBook::tags() { return myTags; }
inline std::vector<fb::shared_ptr<DBAuthor> > &DBBook::authors() { return myAuthors; }

inline unsigned DBBook::bookId() const { return myBookId; }
inline void DBBook::setBookId(unsigned bookId) { myBookId = bookId; }


#endif /* __DBVALUES_H__ */
