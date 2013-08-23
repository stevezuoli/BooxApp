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


#include "../DBRunnables.h"
#include "../../sqldb/implsqlite/SQLiteFactory.h"


LoadTagsRunnable::LoadTagsRunnable(DBConnection &connection) {
	myLoadTags = SQLiteFactory::createCommand(BooksDBQuery::LOAD_TAGS, connection, "@book_id", DBValue::DBINT);
	myFindParentTag = SQLiteFactory::createCommand(BooksDBQuery::FIND_PARENT_TAG, connection, "@tag_id", DBValue::DBINT);
}

bool LoadTagsRunnable::run() {
	((DBIntValue &) *myLoadTags->parameter("@book_id").value()) = myBookId;
	fb::shared_ptr<DBDataReader> reader = myLoadTags->executeReader();
	if (reader.isNull()) {
		return false;
	}

	std::vector<fb::shared_ptr<DBTag> > dbTags;
	while (reader->next()) {
		if (reader->type(0) != DBValue::DBINT /* tag_id */
			|| reader->type(1) != DBValue::DBTEXT /* name */) {
			reader->close();
			return false;
		}
		const int tagId = reader->intValue(0);
		const std::string tag = reader->textValue(1);
		DBTag *ptr = new DBTag(tag);
		ptr->setTagId(tagId);
		dbTags.push_back( ptr );
	}
	reader->close();
	
	myTags.clear();

	DBIntValue &parentId = (DBIntValue &) *myFindParentTag->parameter("@tag_id").value();

	for (unsigned i = 0; i < dbTags.size(); ++i) {
		std::vector<fb::shared_ptr<DBTag> > subtags;
		subtags.push_back( dbTags[i] );
		while (true) {
			parentId.setValue( subtags.back()->tagId() );
			fb::shared_ptr<DBDataReader> reader = myFindParentTag->executeReader();
			if (reader.isNull()) {
				return false;
			}
			if (!reader->next()) {
				reader->close();
				break;
			}
			if (reader->type(0) != DBValue::DBINT /* tag_id */
				|| reader->type(1) != DBValue::DBTEXT /* name */) {
				reader->close();
				return false;
			}
			const int tagId = reader->intValue(0);
			const std::string tag = reader->textValue(1);
			reader->close();

			DBTag *ptr = new DBTag(tag);
			ptr->setTagId(tagId);
			subtags.push_back(ptr);
		}
		fb::shared_ptr<DBTag> res = DBTag::getTag(subtags.back()->name(), false);
		while (true) {
			if (res.isNull()) {
				return false;
			}
			res->setTagId( subtags.back()->tagId() );
			subtags.pop_back();
			if (subtags.empty()) {
				break;
			}
			fb::shared_ptr<DBTag> next = res->findChild(subtags.back()->name());
			if (next.isNull()) {
				next = res->addChild(subtags.back()->name(), false);
			}
			res = next;
		}
		myTags.push_back( res );
	}
	return true;
}

