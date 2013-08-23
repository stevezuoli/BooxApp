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


#include <set>

#include <ZLStringUtil.h>

#include "DBTag.h"



std::vector<fb::shared_ptr<DBTag> > DBTag::ourTags;

const std::string DBTag::DELIMITER = "/";


fb::shared_ptr<DBTag> DBTag::getTag(const std::string &name, bool checkName) {
	std::string tag = name;
	if (checkName) {
		ZLStringUtil::stripWhiteSpaces(tag);
	}
	if (tag.empty()) {
		return 0;
	}
	for (unsigned i = 0; i < ourTags.size(); ++i) {
		if (ourTags[i]->name() == tag) {
			return ourTags[i];
		}
	}
	ourTags.push_back(new DBTag(tag));
	return ourTags.back();
}

fb::shared_ptr<DBTag> DBTag::getSubTag(const std::string &fullName) {
	int index = fullName.find(DELIMITER);
	if (index == -1) {
		return getTag(fullName);
	} else {
		std::string tag = fullName.substr(0, index);
		fb::shared_ptr<DBTag> ptr = getTag(tag);
		while (true) {
			int index0 = index + 1;
			index = tag.find(DELIMITER, index0);
			tag = fullName.substr(index0, index - index0); 
			ZLStringUtil::stripWhiteSpaces(tag);
			if (tag.empty()) {
				return 0;
			}
			fb::shared_ptr<DBTag> next = ptr->findChild(tag);
			if (next.isNull()) {
				next = ptr->addChild(tag);
			}
			if (index == -1) {
				return next;
			}
			ptr = next;
		}
	}
}


fb::shared_ptr<DBTag> DBTag::cloneSubTag(DBTag &tag, DBTag &oldparent, DBTag &newparent) {
	std::vector<std::string> levels;
	DBTag *ptr = &tag;

	while (ptr != &oldparent) {
		levels.push_back(ptr->name());
		if (!ptr->hasParent()) {
			return 0;
		}
		ptr = &ptr->parent();
	}

	if (levels.empty()) {
		return 0;		
	}

	fb::shared_ptr<DBTag> res = newparent.findChild(levels.back());
	if (res.isNull()) {
		res = newparent.addChild(levels.back(), false);
	}
	levels.pop_back();

	while (!levels.empty()) {
		fb::shared_ptr<DBTag> next = res->findChild(levels.back());
		if (next.isNull()) {
			next = res->addChild(levels.back(), false);
		}
		res = next;
	}
	return res;
}


fb::shared_ptr<DBTag> DBTag::checkTag(DBTag &tag) {

	fb::shared_ptr<DBTag> checked;
	
	if (!tag.hasParent()) {
		checked = getTag(tag.name(), false);
	} else {
		fb::shared_ptr<DBTag> parent = checkTag( tag.parent() );

		checked = parent->findChild( tag.name() );

		if (checked.isNull()) {
			checked = parent->addChild( tag );
		}
	}

	if (checked->tagId() == 0 || checked->tagId() != tag.tagId()) {
		checked->setTagId( tag.tagId() );
	}

	return checked;
}


fb::shared_ptr<DBTag> DBTag::reference2SharedPtr(DBTag &tag) {
	std::vector<DBTag *> tags;
	tags.push_back(&tag);
	while (tags.back()->hasParent()) {
		tags.push_back( &tags.back()->parent() );
	}

	fb::shared_ptr<DBTag> res;
	std::string name = tags.back()->name();
	for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = ourTags.begin(); it != ourTags.end(); ++it) {
		if ((*it)->name() == name) {
			res = *it;
			break;
		}
	}
	tags.pop_back();

	while (tags.size() != 0) {
		const std::vector<fb::shared_ptr<DBTag> > children = res->children();
		std::string name = tags.back()->name();
		for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = children.begin(); it != children.end(); ++it) {
			if ((*it)->name() == name) {
				res = *it;
				break;
			}
		}
		tags.pop_back();
	}
	return res;
}


void DBTag::fillParents(DBTag &tag, std::vector<fb::shared_ptr<DBTag> > &parents) {
	std::vector<DBTag *> tags;
	tags.push_back(&tag);
	while (tags.back()->hasParent()) {
		tags.push_back( &tags.back()->parent() );
	}

	fb::shared_ptr<DBTag> res;
	std::string name = tags.back()->name();
	for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = ourTags.begin(); it != ourTags.end(); ++it) {
		if ((*it)->name() == name) {
			res = *it;
			parents.push_back(res);
			break;
		}
	}
	tags.pop_back();

	while (tags.size() != 0) {
		const std::vector<fb::shared_ptr<DBTag> > children = res->children();
		std::string name = tags.back()->name();
		for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = children.begin(); it != children.end(); ++it) {
			if ((*it)->name() == name) {
				res = *it;
				parents.push_back(res);
				break;
			}
		}
		tags.pop_back();
	}
}

void DBTag::collectTagNames(std::vector<std::string> &tags) {
	std::set<std::string> tagsSet;
	std::vector<fb::shared_ptr<DBTag> > stack(ourTags);
	while (!stack.empty()) {
		fb::shared_ptr<DBTag> tag = stack.back();
		stack.pop_back();
		tagsSet.insert( tag->fullName() );
		stack.insert(stack.end(), tag->myChildren.begin(), tag->myChildren.end());
	}
	tags.insert(tags.end(), tagsSet.begin(), tagsSet.end());
}


const std::string &DBTag::fullName() const {
	if (myParent == 0) {
		return myName;
	}
	if (myFullName.empty()) {
		myFullName = myParent->fullName() + DELIMITER + myName;
	}
	return myFullName;
}

fb::shared_ptr<DBTag> DBTag::addChild(const std::string &name, bool checkName) {
	std::string tag = name;
	if (checkName) {
		ZLStringUtil::stripWhiteSpaces(tag);
	}
	if (!tag.empty()) {
		if (findChildPos(tag) == (unsigned) -1) {
			myChildren.push_back( new DBTag(tag, *this) );
			return myChildren.back();
		}
	}
	return 0;
}

fb::shared_ptr<DBTag> DBTag::addChild(DBTag &tag) {
	const std::string &name = tag.name();
	if (findChildPos(name) == (unsigned) -1) {
		DBTag *ptr = new DBTag(name, *this);
		ptr->setTagId( tag.tagId() );
		myChildren.push_back( ptr );
		return myChildren.back();
	}
	return 0;
}


void DBTag::removeChild(unsigned pos) {
	if (pos < myChildren.size()) {
		myChildren.erase( myChildren.begin() + pos );
	}
}

fb::shared_ptr<DBTag> DBTag::findChild(const std::string &name) const {
	for (unsigned i = 0; i < myChildren.size(); ++i) {
		if (myChildren[i]->name() == name) {
			return myChildren[i];
		}
	}
	return 0;
}

unsigned DBTag::findChildPos(const std::string &name) const {
	for (unsigned i = 0; i < myChildren.size(); ++i) {
		if (myChildren[i]->name() == name) {
			return i;
		}
	}
	return -1;
}

bool DBTag::isChildFor(DBTag &tag) const {
	if (this == &tag) {
		return true;
	}
	if (myParent == 0) {
		return false;
	}
	return myParent->isChildFor(tag);
}


