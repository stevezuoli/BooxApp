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

#include <ZLStringUtil.h>
#include <ZLFile.h>
#include <ZLInputStream.h>

#include "RtfPlugin.h"
#include "RtfDescriptionReader.h"
#include "RtfBookReader.h"
#include "RtfReaderStream.h"

#include "../../database/booksdb/DBBook.h"

bool RtfPlugin::providesMetaInfo() const {
	return false;
}

bool RtfPlugin::acceptsFile(const ZLFile &file) const {
	return file.extension() == "rtf";
}

bool RtfPlugin::readDescription(const std::string &path, DBBook &book) const {
	ZLFile file(path);
	//fb::shared_ptr<ZLInputStream> stream = file.inputStream();
	fb::shared_ptr<ZLInputStream> stream = new RtfReaderStream(path, 50000);

	if (stream.isNull()) {
		return false;
	}

	detectEncodingAndLanguage(book, *stream);

	if (!RtfDescriptionReader(book).readDocument(path)) {
		return false;
	}
	
	return true;
}

bool RtfPlugin::readModel(const DBBook &book, BookModel &model) const {
	return RtfBookReader(model, book.encoding()).readDocument(book.fileName());
}

const std::string &RtfPlugin::iconName() const {
	static const std::string ICON_NAME = "rtf";
	return ICON_NAME;
}

FormatInfoPage *RtfPlugin::createInfoPage(ZLOptionsDialog&, const std::string&) {
	return 0;
}
