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


#include <ZLStringUtil.h>
#include <ZLUnicodeUtil.h>

#include "DBAuthor.h"


const std::string DBAuthor::UNKNOWN_AUTHOR_NAME = "Unknown Author";
const std::string DBAuthor::UNKNOWN_AUTHOR_SORTKEY = "___";



fb::shared_ptr<DBAuthor> DBAuthor::create(const std::string &name, const std::string &sortKey) {
	std::string strippedName = name;
	ZLStringUtil::stripWhiteSpaces(strippedName);
	if (strippedName.empty()) {
		return 0;
	}
	if (strippedName == UNKNOWN_AUTHOR_NAME) {
		return new DBAuthor();
	}
	std::string strippedKey = sortKey;
	ZLStringUtil::stripWhiteSpaces(strippedKey);
	if (strippedKey.empty()) {
		int index = strippedName.rfind(' ');
		if (index == (int) std::string::npos) {
			strippedKey = strippedName;
		} else {
			strippedKey = strippedName.substr(index + 1);
			while ((index >= 0) && (strippedName[index] == ' ')) {
				--index;
			}
			strippedName = strippedName.substr(0, index + 1) + ' ' + strippedKey;
		}
	}
	return new DBAuthor(strippedName, ZLUnicodeUtil::toLower(strippedKey));
}


/*void DBBookAuthors::add(fb::shared_ptr<DBAuthor> author) { 
	if (author.isNull()) {
		return;
	}	
	if (author->isDefault() && !myAuthors.empty()) {
		return;
	}
	myAuthors.push_back(author);
	mySortKey = "";
	myDisplayName = "";
}

void DBBookAuthors::clear() { 
	myAuthors.clear();
	mySortKey = "";
	myDisplayName = "";
}*/

