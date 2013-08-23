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


#ifndef __DBAUTHOR_H__
#define __DBAUTHOR_H__

#include <string>
#include <vector>
#include <shared_ptr.h>

class DBAuthor {

public:
	static const std::string UNKNOWN_AUTHOR_NAME;
	static const std::string UNKNOWN_AUTHOR_SORTKEY;

public:
	DBAuthor();
	static fb::shared_ptr<DBAuthor> create(const std::string &name, const std::string &sortKey = "");

private:
	DBAuthor(const std::string &name, const std::string &sortkey);

public:
	const std::string &name() const;
	const std::string &sortKey() const;

	void setName(const std::string &name);
	void setSortKey(const std::string &sortkey);

	bool isDefault() const;

public:
	bool operator == (const DBAuthor &a) const;
	bool operator != (const DBAuthor &a) const;

private:
	std::string myName;
	std::string mySortKey;

private: // disable copying:
	DBAuthor(const DBAuthor &);
	const DBAuthor &operator = (const DBAuthor &);
};


class DBAuthorComparator {
public:
	bool operator() (const fb::shared_ptr<DBAuthor> a1, const fb::shared_ptr<DBAuthor> a2);
};

class DBAuthorPredicate {
public:
	DBAuthorPredicate(const fb::shared_ptr<DBAuthor> author);
	bool operator() (const fb::shared_ptr<DBAuthor> author);
private:
	const fb::shared_ptr<DBAuthor> myAuthor;
};


inline DBAuthor::DBAuthor() : /*myAuthorId(0),*/ myName(UNKNOWN_AUTHOR_NAME), mySortKey(UNKNOWN_AUTHOR_SORTKEY) {}
inline DBAuthor::DBAuthor(const std::string &name, const std::string &sortkey) : /*myAuthorId(0),*/ myName(name), mySortKey(sortkey) {}

inline const std::string &DBAuthor::name() const { return myName; }
inline const std::string &DBAuthor::sortKey() const { return mySortKey; }
inline void DBAuthor::setName(const std::string &name) { myName = name; }
inline void DBAuthor::setSortKey(const std::string &sortkey) { mySortKey = sortkey; }
inline bool DBAuthor::isDefault() const { return myName == UNKNOWN_AUTHOR_NAME && mySortKey == UNKNOWN_AUTHOR_SORTKEY; }
inline bool DBAuthor::operator == (const DBAuthor &a) const { return mySortKey == a.sortKey() && myName == a.name(); }
inline bool DBAuthor::operator != (const DBAuthor &a) const { return !operator == (a); }


inline bool DBAuthorComparator::operator() (const fb::shared_ptr<DBAuthor> a1, const fb::shared_ptr<DBAuthor> a2) {
	return
		(a1->sortKey() < a2->sortKey()) ||
		((a1->sortKey() == a2->sortKey()) && (a1->name() < a2->name()));
}


inline DBAuthorPredicate::DBAuthorPredicate(const fb::shared_ptr<DBAuthor> author) : myAuthor(author) {}

inline bool DBAuthorPredicate::operator() (const fb::shared_ptr<DBAuthor> author) {
	return *myAuthor == *author;
}


#endif /* __DBAUTHOR_H__ */
