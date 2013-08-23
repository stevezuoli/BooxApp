/*
 * Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
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

#ifndef __BOOKMODEL_H__
#define __BOOKMODEL_H__

#include <map>
#include <string>

#include <ZLTextModel.h>
#include <ZLTextParagraph.h>
#include <ZLUserData.h>
#include "../database/booksdb/DBBook.h"

class ZLImage;

class ContentsModel : public ZLTextTreeModel {

public:
	void setReference(const ZLTextTreeParagraph *paragraph, int reference);
	int reference(const ZLTextTreeParagraph *paragraph) const;

private:
	std::map<const ZLTextTreeParagraph*,int> myReferenceByParagraph;
};

class BookModel : public ZLUserDataHolder {

public:
	struct Label {
		Label(fb::shared_ptr<ZLTextModel> model, int paragraphNumber) : Model(model), ParagraphNumber(paragraphNumber) {}

		const fb::shared_ptr<ZLTextModel> Model;
		const int ParagraphNumber;
	};

public:
	BookModel(const fb::shared_ptr<DBBook> book);
	~BookModel();

	const std::string &fileName() const;

	fb::shared_ptr<ZLTextModel> bookTextModel() const;
	fb::shared_ptr<ZLTextModel> contentsModel() const;

	const ZLImageMap &imageMap() const;
	Label label(const std::string &id) const;

	const fb::shared_ptr<DBBook> book() const;

private:
	const fb::shared_ptr<DBBook> myBook;
	fb::shared_ptr<ZLTextModel> myBookTextModel;
	fb::shared_ptr<ZLTextModel> myContentsModel;
	ZLImageMap myImages;
	std::map<std::string,fb::shared_ptr<ZLTextModel> > myFootnotes;
	std::map<std::string,Label> myInternalHyperlinks;

friend class BookReader;
};

inline fb::shared_ptr<ZLTextModel> BookModel::bookTextModel() const { return myBookTextModel; }
inline fb::shared_ptr<ZLTextModel> BookModel::contentsModel() const { return myContentsModel; }
inline const ZLImageMap &BookModel::imageMap() const { return myImages; }
inline const fb::shared_ptr<DBBook> BookModel::book() const { return myBook; }
inline const std::string &BookModel::fileName() const { return myBook->fileName(); }

#endif /* __BOOKMODEL_H__ */
