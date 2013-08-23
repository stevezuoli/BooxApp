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
#include <set>

#include <ZLStringUtil.h>
#include <ZLFile.h>

#include "DBBook.h"

#include "../../formats/FormatPlugin.h"

#include "../../migration/BookInfo.h"

fb::shared_ptr<DBBook> DBBook::loadFromFile(const std::string &fileName) {

	ZLFile bookFile(fileName);
	FormatPlugin *plugin = PluginCollection::instance().plugin(bookFile, false);
	if (plugin == 0) {
		return 0;
	}

	fb::shared_ptr<DBBook> book = new DBBook(fileName);
	if (!plugin->readDescription(fileName, *book)) {
		return 0;	
	}

	if (book->title().empty()) {
		book->setTitle(ZLFile::fileNameToUtf8(bookFile.name(true)));
	}

	if (book->authors().empty()) {
		book->authors().push_back(new DBAuthor());
	}

	if (book->encoding().empty()) {
		book->setEncoding("auto");
	}

	if (book->language().empty()) {
		book->setLanguage( PluginCollection::instance().DefaultLanguageOption.value() );
	}

	return book;
}


/*void DBBook::removeWhiteSpacesFromTag(std::string &tag) {
	int index = tag.find('/');
	if (index == -1) {
		ZLStringUtil::stripWhiteSpaces(tag);
	} else {
		std::string result;
		int index0 = 0;
		while (true) {
			std::string subtag = tag.substr(index0, index - index0); 
			ZLStringUtil::stripWhiteSpaces(subtag);
			if (!subtag.empty()) {
				if (!result.empty()) {
					result += '/';
				}
				result += subtag;
			}
			if (index == -1) {
				break;
			}
			index0 = index + 1;
			index = tag.find('/', index0);
		}
		tag = result;
	}
}*/



bool DBBook::addTag(fb::shared_ptr<DBTag> tag) {
	std::vector<fb::shared_ptr<DBTag> >::const_iterator it = std::find(myTags.begin(), myTags.end(), tag);
	if (it == myTags.end()) {
		myTags.push_back(tag);
		return true;
	}
	return false;
}


bool DBBook::removeTag(fb::shared_ptr<DBTag> tag, bool includeSubTags) {
	if (includeSubTags) {
		bool changed = false;
		for (std::vector<fb::shared_ptr<DBTag> >::iterator it = myTags.begin(); it != myTags.end();) {
			if ((*it)->isChildFor(*tag)) {
				it = myTags.erase(it);
				changed = true;
			} else {
				++it;
			}
		}
		return changed;
	} else {
		std::vector<fb::shared_ptr<DBTag> >::iterator it = std::find(myTags.begin(), myTags.end(), tag);
		if (it != myTags.end()) {
			myTags.erase(it);
			return true;
		}
		return false;
	}
}

bool DBBook::renameTag(fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags) {
	if (includeSubTags) {
		std::set<fb::shared_ptr<DBTag> > tagSet;
		bool changed = false;
		for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = myTags.begin(); it != myTags.end(); ++it) {
			if (*it == from) {
				tagSet.insert(to);
				changed = true;
			} else {
				fb::shared_ptr<DBTag> newtag = DBTag::cloneSubTag(**it, *from, *to);
				if (newtag.isNull()) {
					tagSet.insert(*it);
				} else {
					tagSet.insert(newtag);
					changed = true;
				}
			}
		}
		if (changed) {
			myTags.clear();
			myTags.insert(myTags.end(), tagSet.begin(), tagSet.end());
			return true;
		}
	} else {
		std::vector<fb::shared_ptr<DBTag> >::iterator it = std::find(myTags.begin(), myTags.end(), from);
		if (it != myTags.end()) {
			std::vector<fb::shared_ptr<DBTag> >::const_iterator jt = std::find(myTags.begin(), myTags.end(), to);
			if (jt == myTags.end()) {
				*it = to;
			} else {
				myTags.erase(it);
			}
			return true;
		}
	}
	return false;
}

bool DBBook::cloneTag(fb::shared_ptr<DBTag> from, fb::shared_ptr<DBTag> to, bool includeSubTags) {
	if (includeSubTags) {
		std::set<fb::shared_ptr<DBTag> > tagSet;
		for (std::vector<fb::shared_ptr<DBTag> >::const_iterator it = myTags.begin(); it != myTags.end(); ++it) {
			if (*it == from) {
				tagSet.insert(to);
			} else {
				fb::shared_ptr<DBTag> newtag = DBTag::cloneSubTag(**it, *from, *to);
				if (!newtag.isNull()) {
					tagSet.insert(newtag);
				}
			}
		}
		if (!tagSet.empty()) {
			tagSet.insert(myTags.begin(), myTags.end());
			myTags.clear();
			myTags.insert(myTags.end(), tagSet.begin(), tagSet.end());
			return true;
		}
	} else {
		std::vector<fb::shared_ptr<DBTag> >::const_iterator it = std::find(myTags.begin(), myTags.end(), from);
		if (it != myTags.end()) {
			std::vector<fb::shared_ptr<DBTag> >::const_iterator jt = std::find(myTags.begin(), myTags.end(), to);
			if (jt == myTags.end()) {
				myTags.push_back(to);
				return true;
			}
		}
	}
	return false;
}

fb::shared_ptr<DBBook> DBBook::loadFromBookInfo(const std::string &fileName) {
	BookInfo info(fileName);

	fb::shared_ptr<DBBook> book = new DBBook(fileName);

	book->setTitle(info.TitleOption.value());
	book->setLanguage(info.LanguageOption.value());
	book->setEncoding(info.EncodingOption.value());
	book->setSeriesName(info.SeriesNameOption.value());
	book->setNumberInSeries(info.NumberInSeriesOption.value());

	if (book->language().empty()) {
		book->setLanguage( PluginCollection::instance().DefaultLanguageOption.value() );
	}

	const std::string &tagList = info.TagsOption.value();
	if (!tagList.empty()) {
		int index = 0;
		do {
			int newIndex = tagList.find(',', index);
			book->addTag( DBTag::getSubTag( tagList.substr(index, newIndex - index) ) );
			index = newIndex + 1;
		} while (index != 0);
	}

	const std::string &authorList = info.AuthorDisplayNameOption.value();
	if (!authorList.empty()) {
		int index = 0;
		do {
			int newIndex = authorList.find(',', index);
			book->authors().push_back( DBAuthor::create( authorList.substr(index, newIndex - index) ) );
			index = newIndex + 1;
		} while (index != 0);
	}

	return book;
}

std::string DBBook::authorDisplayName() const {
	std::string displayName;
	if (!myAuthors.empty()) {
		displayName = myAuthors.front()->name();
		for (unsigned int i = 1; i < myAuthors.size(); ++i) {
			displayName += ", ";
			displayName += myAuthors[i]->name();
		}
	}
	return displayName;
}

std::string DBBook::authorSortKey() const {
	std::string sortKey;
	if (!myAuthors.empty()) {
		sortKey = myAuthors.front()->sortKey();
		for (unsigned int i = 1; i < myAuthors.size(); ++i) {
			const std::string &key = myAuthors[i]->sortKey();
			if (key < sortKey) {
				sortKey = key;
			}
		}
	}
	return sortKey;
}

bool DBBook::updateAuthor(fb::shared_ptr<DBAuthor> from, fb::shared_ptr<DBAuthor> to) {
	std::vector<fb::shared_ptr<DBAuthor> >::iterator it = std::find_if(myAuthors.begin(), myAuthors.end(), DBAuthorPredicate(from));
	if (it == myAuthors.end()) {
		return false;
	}
	if (to->isDefault()) {
		myAuthors.erase(it);
		if (myAuthors.empty()) {
			myAuthors.push_back(to);
		}
		return true;
	}
	*it = to;
	return true;
}


