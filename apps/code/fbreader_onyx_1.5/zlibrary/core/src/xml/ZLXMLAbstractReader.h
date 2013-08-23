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

#ifndef __ZLXMLABSTRACTREADER_H__
#define __ZLXMLABSTRACTREADER_H__

#include <string>

class ZLXMLAbstractReader {

public:
	ZLXMLAbstractReader();
	virtual ~ZLXMLAbstractReader();

	virtual void initialize(const char *encoding = 0) = 0;
	virtual void shutdown() = 0;
	virtual bool readFromBuffer(const char *data, size_t len) = 0;

};

#endif /* __ZLXMLABSTRACTREADER_H__ */
