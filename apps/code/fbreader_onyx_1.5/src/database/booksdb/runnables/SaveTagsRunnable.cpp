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

#include <algorithm>
#include <iostream>

#include "../DBRunnables.h"
#include "../../sqldb/implsqlite/SQLiteFactory.h"


SaveTagsRunnable::SaveTagsRunnable(DBConnection &connection) {
	myDeleteBookTag    = SQLiteFactory::createCommand(BooksDBQuery::DELETE_BOOKTAG, connection, "@book_id", DBValue::DBINT, "@tag_id", DBValue::DBINT);
	myFindTagId        = SQLiteFactory::createCommand(BooksDBQuery::FIND_TAG_ID, connection, "@name", DBValue::DBTEXT, "@parent_id", DBValue::DBINT);
	myAddTag           = SQLiteFactory::createCommand(BooksDBQuery::ADD_TAG, connection, "@name", DBValue::DBTEXT, "@parent_id", DBValue::DBINT);
	myAddBookTag       = SQLiteFactory::createCommand(BooksDBQuery::ADD_BOOKTAG, connection, "@book_id", DBValue::DBINT, "@tag_id", DBValue::DBINT);

	myLoadTags = new LoadTagsRunnable(connection);
}

bool SaveTagsRunnable::run() {
	if (myBook->bookId() == 0) {
		return false;
	}

	((DBIntValue &) *myDeleteBookTag->parameter("@book_id").value()) = myBook->bookId();
	DBIntValue &delTagId  = (DBIntValue &) *myDeleteBookTag->parameter("@tag_id").value();
	((DBIntValue &) *myAddBookTag->parameter("@book_id").value()) = myBook->bookId();
	DBIntValue &addTagId = (DBIntValue &) *myAddBookTag->parameter("@tag_id").value();

	std::vector<fb::shared_ptr<DBTag> > dbTags;
	myLoadTags->setBookId(myBook->bookId());
	if (!myLoadTags->run()) {
		return 0;
	}
	myLoadTags->collectTags(dbTags);

	std::vector<fb::shared_ptr<DBTag> > tags = myBook->tags(); // make copy of vector

	for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = dbTags.begin(); it != dbTags.end(); ++it) {
		fb::shared_ptr<DBTag> tag = (*it);
		std::vector<fb::shared_ptr<DBTag> >::iterator jt = std::find(tags.begin(), tags.end(), tag);
		if (jt == tags.end()) {
			// Tag `tag` must be removed from BookTag table.
			
for (unsigned i = 0; i < tags.size(); ++i) {
	if (tag->fullName() == tags[i]->fullName()) {
		std::cerr << "ERROR: TAG POINTERS ARE NOT EQUALS for \"" << tag->fullName() << "\" tag at" << __FILE__ << ":" << __LINE__ << std::endl;	
	}
}

// TODO: remove debug check
if (tag->tagId() == 0) {
	std::cerr << "ERROR: TAG_ID EQUALS ZERO!!! at" << __FILE__ << ":" << __LINE__ << std::endl;
	return false;
}

			delTagId.setValue( tag->tagId() );
			if (!myDeleteBookTag->execute()) {
				return false;
			}
		} else {
			// This tag is already in DataBase => need not to be inserted.
			tags.erase(jt);
		}
	}

	for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
		fb::shared_ptr<DBTag> tag = (*it);
		int tableTagId = findTagId( *tag );
		if (tableTagId == 0) {
			return false;
		}
		addTagId.setValue( tableTagId );
		if (!myAddBookTag->execute()) {
			return false;
		}
	}
	return true;
}


int SaveTagsRunnable::findTagId(DBTag &tag) {
	if (tag.tagId() != 0) {
		return tag.tagId();
	}
	int parentId = 0;
	if (tag.hasParent()) {
		parentId = findTagId( tag.parent() );
		if (parentId == 0) {
			return 0;
		}
	}

	DBIntValue &findParent = (DBIntValue &) *myFindTagId->parameter("@parent_id").value();
	DBTextValue &findName  = (DBTextValue &) *myFindTagId->parameter("@name").value();
	DBIntValue &addParent  = (DBIntValue &) *myAddTag->parameter("@parent_id").value();
	DBTextValue &addName   = (DBTextValue &) *myAddTag->parameter("@name").value();

	findParent.setValue( parentId );
	findName.setValue( tag.name() );
	fb::shared_ptr<DBValue> tableTagId = myFindTagId->executeScalar();
	if (tableTagId.isNull() || tableTagId->type() != DBValue::DBINT || ((DBIntValue &) *tableTagId).value() == 0) {
		addParent.setValue( parentId );
		addName.setValue( tag.name() );
		tableTagId = myAddTag->executeScalar();
		if (tableTagId.isNull() || tableTagId->type() != DBValue::DBINT || ((DBIntValue &) *tableTagId).value() == 0) {
			return 0;
		}
		//tag.setTagId( ((DBIntValue &) *tableTagId).value() ); // TODO: why do not set tagid???
	}
	return ((DBIntValue &) *tableTagId).value();
}

