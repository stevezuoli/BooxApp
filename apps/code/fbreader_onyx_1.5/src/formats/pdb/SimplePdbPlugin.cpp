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

#include <ZLFile.h>
#include <ZLInputStream.h>
#include <iostream>

#include "PdbPlugin.h"
#include "../txt/TxtBookReader.h"
#include "../html/HtmlBookReader.h"
#include "HtmlMetainfoReader.h"
#include "../util/TextFormatDetector.h"

#include "../../database/booksdb/DBBook.h"

bool SimplePdbPlugin::readDescription(const std::string &path, DBBook &book) const {
	ZLFile file(path);
	std::cerr << "Read description specified by SimplePdbPlugin...\n";
	fb::shared_ptr<ZLInputStream> stream = createStream(file);
	detectEncodingAndLanguage(book, *stream);
	if (book.encoding().empty()) {
		return false;
	}
	int readType = HtmlMetainfoReader::NONE;
	if (book.title().empty()) {
		readType |= HtmlMetainfoReader::TITLE;
	}
	if (book.authors().empty()) {
		readType |= HtmlMetainfoReader::AUTHOR;
	}
	if ((readType != HtmlMetainfoReader::NONE) && TextFormatDetector().isHtml(*stream)) {
		readType |= HtmlMetainfoReader::TAGS;
		HtmlMetainfoReader metainfoReader(book, (HtmlMetainfoReader::ReadType)readType);
		metainfoReader.readDocument(*stream);
	}

	return true;
}

bool SimplePdbPlugin::readModel(const DBBook &book, BookModel &model) const {
	std::cerr << "Read model specified by SimplePdbPlugin...\n";
	ZLFile file(book.fileName());
	fb::shared_ptr<ZLInputStream> stream = createStream(file);

	PlainTextFormat format(book.fileName());
	if (!format.initialized()) {
		PlainTextFormatDetector detector;
		detector.detect(*stream, format);
	}
	readDocumentInternal(book.fileName(), model, format, book.encoding(), *stream);
	return true;
}

void SimplePdbPlugin::readDocumentInternal(const std::string&, BookModel &model, const PlainTextFormat &format, const std::string &encoding, ZLInputStream &stream) const {
	std::cerr << "Read document internal specified by SimplePdbPlugin...\n";
	if (TextFormatDetector().isHtml(stream)) {
		HtmlBookReader("", model, format, encoding).readDocument(stream);
	} else {
		TxtBookReader(model, format, encoding).readDocument(stream);
	}
}
