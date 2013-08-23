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
#include <ZLEncodingConverter.h>
#include <ZLStringUtil.h>
#include <ZLLanguageUtil.h>
#include <iostream>

#include "PdbPlugin.h"
#include "PalmDocStream.h"
#include "MobipocketHtmlBookReader.h"

#include "../../database/booksdb/DBBook.h"

bool MobipocketPlugin::acceptsFile(const ZLFile &file) const {
	return PdbPlugin::fileType(file) == "BOOKMOBI";
}

void MobipocketPlugin::readDocumentInternal(const std::string &fileName, BookModel &model, const PlainTextFormat &format, const std::string &encoding, ZLInputStream &stream) const {
	std::cerr << "Read document internal specified by MobipocketPlugin...\n";
	MobipocketHtmlBookReader(fileName, model, format, encoding).readDocument(stream);
}

const std::string &MobipocketPlugin::iconName() const {
	static const std::string ICON_NAME = "mobipocket";
	return ICON_NAME;
}

bool MobipocketPlugin::readDescription(const std::string &path, DBBook &book) const {
	std::cerr << "Read description specified by MobipocketPlugin...\n";
	fb::shared_ptr<ZLInputStream> stream = ZLFile(path).inputStream();
	if (stream.isNull() || ! stream->open()) {
		return false;
	}
	PdbHeader header;
	if (!header.read(stream)) {
		return false;
	}
	stream->seek(header.Offsets[0] + 16, true);
	char test[5];
	test[4] = '\0';
	stream->read(test, 4);
	static const std::string MOBI = "MOBI";
	if (MOBI != test) {
		return PalmDocLikePlugin::readDescription(path, book);
	}

	unsigned long length;
	PdbUtil::readUnsignedLongBE(*stream, length);

	stream->seek(4, false);

	unsigned long encodingCode;
	PdbUtil::readUnsignedLongBE(*stream, encodingCode);
	if (book.encoding().empty()) {
		ZLEncodingConverterInfoPtr info = ZLEncodingCollection::instance().info(encodingCode);
		if (!info.isNull()) {
			book.setEncoding(info->name());
		}
	}

	stream->seek(52, false);

	unsigned long fullNameOffset;
	PdbUtil::readUnsignedLongBE(*stream, fullNameOffset);
	unsigned long fullNameLength;
	PdbUtil::readUnsignedLongBE(*stream, fullNameLength);

	unsigned long languageCode;
	PdbUtil::readUnsignedLongBE(*stream, languageCode);
	book.setLanguage( ZLLanguageUtil::languageByCode(languageCode & 0xFF, (languageCode >> 8) & 0xFF) );

	stream->seek(32, false);

	unsigned long exthFlags;
	PdbUtil::readUnsignedLongBE(*stream, exthFlags);
	if (exthFlags & 0x40) {
		stream->seek(header.Offsets[0] + 16 + length, true);

		stream->read(test, 4);
		static const std::string EXTH = "EXTH";
		if (EXTH == test) {
			stream->seek(4, false);
			unsigned long recordsNum;
			PdbUtil::readUnsignedLongBE(*stream, recordsNum);
			for (unsigned long i = 0; i < recordsNum; ++i) {
				unsigned long type;
				PdbUtil::readUnsignedLongBE(*stream, type);
				unsigned long size;
				PdbUtil::readUnsignedLongBE(*stream, size);
				if (size > 8) {
					std::string value(size - 8, '\0');
					stream->read((char*)value.data(), size - 8);
					switch (type) {
						case 100:
						{
							int index = value.find(',');
							if (index != -1) {
								std::string part0 = value.substr(0, index);
								std::string part1 = value.substr(index + 1);
								ZLStringUtil::stripWhiteSpaces(part0);
								ZLStringUtil::stripWhiteSpaces(part1);
								value = part1 + ' ' + part0;
							} else {
								ZLStringUtil::stripWhiteSpaces(value);
							}
							fb::shared_ptr<DBAuthor> author = DBAuthor::create(value);
							if (!author.isNull()) {
								book.authors().push_back( author );
							}
							break;
						}
						case 105:
							//book.addTag(value);
							book.addTag(  DBTag::getSubTag(value)  );
							break;
					}
				}
			}
		}
	}

	stream->seek(header.Offsets[0] + fullNameOffset, true);
	std::string title(fullNameLength, '\0');
	stream->read((char*)title.data(), fullNameLength);
	book.setTitle(title);

	stream->close();
	return PalmDocLikePlugin::readDescription(path, book);
}
