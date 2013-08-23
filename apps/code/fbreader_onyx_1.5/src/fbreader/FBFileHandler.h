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

#ifndef __FBFILEHANDLER_H__
#define __FBFILEHANDLER_H__

#include <ZLSelectionDialog.h>
#include <ZLDir.h>

#include "../database/booksdb/DBBook.h"

class FBFileHandler : public ZLTreeOpenHandler {

public:
	ZLStringOption DirectoryOption;

public:
	FBFileHandler();
	~FBFileHandler();

	fb::shared_ptr<DBBook> description() const;

private:
	bool accept(const ZLTreeNode &node);
	void changeFolder(const ZLTreeNode &node);

	const std::string stateDisplayName() const;
	const std::vector<ZLTreeNodePtr> &subnodes() const;
	int selectedIndex() const;
	//std::string relativeId(const ZLTreeNode &node) const;

private:
	fb::shared_ptr<ZLDir> myDir;
	mutable bool myIsUpToDate;
	mutable std::vector<ZLTreeNodePtr> mySubnodes;

	mutable fb::shared_ptr<DBBook> myDescription;

	int mySelectedIndex;
};

inline fb::shared_ptr<DBBook> FBFileHandler::description() const { return myDescription; }

#endif /* __FBFILEHANDLER_H__ */
