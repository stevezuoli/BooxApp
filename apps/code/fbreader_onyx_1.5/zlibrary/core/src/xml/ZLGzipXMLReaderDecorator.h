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

#ifndef __ZLGZIPXMLREADERDECORATOR_H__
#define __ZLGZIPXMLREADERDECORATOR_H__

#include <zlib.h>

#include <shared_ptr.h>

#include "ZLXMLAbstractReader.h"


class ZLGzipXMLReaderDecorator : public ZLXMLAbstractReader {

public:
	ZLGzipXMLReaderDecorator(fb::shared_ptr<ZLXMLAbstractReader> decoratee);
	ZLGzipXMLReaderDecorator(fb::shared_ptr<ZLXMLAbstractReader> decoratee, size_t bufferSize);
	~ZLGzipXMLReaderDecorator();

	void initialize(const char *encoding = 0);
	void shutdown();
	bool readFromBuffer(const char *data, size_t len);

private:
	bool skipHeader(const char *data, size_t len);

private:
	fb::shared_ptr<ZLXMLAbstractReader> myDecoratee;

	z_stream *myZStream;
	char *myOutBuffer;
	const size_t myOutBufferSize;
	
	bool myEndOfStream;
	int myStreamState;
	size_t myOffset;
	unsigned char myFlag;
	unsigned char myByte;
};

#endif /* __ZLGZIPXMLREADERDECORATOR_H__ */
