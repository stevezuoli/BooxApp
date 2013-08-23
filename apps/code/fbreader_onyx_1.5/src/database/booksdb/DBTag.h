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


#ifndef __DBTAG_H__
#define __DBTAG_H__

#include <string>
#include <shared_ptr.h>


class DBTag {

friend class BooksDB;

friend class SaveTagsRunnable;
friend class LoadTagsRunnable;

private:
	static std::vector<fb::shared_ptr<DBTag> > ourTags;

public:
	static fb::shared_ptr<DBTag> getTag(const std::string &name, bool checkName = true);
	static fb::shared_ptr<DBTag> cloneSubTag(DBTag &tag, DBTag &oldparent, DBTag &newparent);
	static fb::shared_ptr<DBTag> checkTag(DBTag &tag);
	
	static fb::shared_ptr<DBTag> reference2SharedPtr(DBTag &tag);
	static void fillParents(DBTag &tag, std::vector<fb::shared_ptr<DBTag> > &parents);

	static fb::shared_ptr<DBTag> getSubTag(const std::string &fullName); // TODO: remove temporary method

	static void collectTagNames(std::vector<std::string> &tags);

public:
	static const std::string DELIMITER;

private:
	DBTag(const std::string &name, DBTag &parent);

public:
	DBTag(const std::string &name);

	const std::string &fullName() const;
	const std::string &name() const;

	const std::vector<fb::shared_ptr<DBTag> > &children() const;
	bool hasParent() const;
	DBTag &parent() const;

	fb::shared_ptr<DBTag> addChild(const std::string &name, bool checkName = true);
	fb::shared_ptr<DBTag> addChild(DBTag &tag);
	void removeChild(unsigned pos);

	unsigned findChildPos(const std::string &name) const;
	fb::shared_ptr<DBTag> findChild(const std::string &name) const;

	bool isChildFor(DBTag &tag) const;

private:
	int tagId() const;
	void setTagId(int tagId);
	
private:
	const std::string myName;
	mutable std::string myFullName;
	std::vector<fb::shared_ptr<DBTag> > myChildren;
	DBTag *const myParent;
	
	int myTagId;

private: // disable copying
	DBTag(const DBTag &);
	const DBTag &operator = (const DBTag &);
};


inline DBTag::DBTag(const std::string &name) : myName(name), myFullName(""), myParent(0), myTagId(0) {}
inline DBTag::DBTag(const std::string &name, DBTag &parent) : myName(name), myFullName(""), myParent(&parent), myTagId(0) {}

inline const std::string &DBTag::name() const { return myName; }
inline const std::vector<fb::shared_ptr<DBTag> > &DBTag::children() const { return myChildren; }

inline bool DBTag::hasParent() const { return myParent != 0; }
inline DBTag &DBTag::parent() const { return *myParent; }


inline int DBTag::tagId() const { return myTagId; }
inline void DBTag::setTagId(int tagId) { myTagId = tagId; }


#endif /* __DBTAG_H__ */
