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

#ifndef __BOOKTEXTVIEW_H__
#define __BOOKTEXTVIEW_H__

#include <deque>

#include "ReadingState.h"
#include <ZLOptions.h>

#include "FBView.h"
#include "../database/booksdb/DBBook.h"

class BookTextView : public FBView {

public:
	ZLBooleanOption ShowTOCMarksOption;

public:
	BookTextView(FBReader &reader, fb::shared_ptr<ZLPaintContext> context);
	~BookTextView();

	void setModel(fb::shared_ptr<ZLTextModel> model, const std::string &language, fb::shared_ptr<DBBook> book);
	void setContentsModel(fb::shared_ptr<ZLTextModel> contentsModel);
	void saveState();

	void gotoParagraph(int num, bool end = false);
	bool canUndoPageMove();
	void undoPageMove();
	bool canRedoPageMove();
	void redoPageMove();

	void scrollToHome();

	bool _onStylusPress(int x, int y);
	bool _onStylusMove(int x, int y);
	bool _onStylusRelease(int x, int y);

    virtual bool openInternalLink(int x, int y);

private:
	typedef ReadingState Position;
	Position cursorPosition(const ZLTextWordCursor &cursor) const;
	bool pushCurrentPositionIntoStack(bool doPushSamePosition = true);
	void replaceCurrentPositionInStack();

	void preparePaintInfo();

	bool getHyperlinkInfo(const ZLTextElementArea &area, std::string &id, std::string &type) const;

	fb::shared_ptr<PositionIndicator> createPositionIndicator(const ZLTextPositionIndicatorInfo &info);

	void paint();

private:
	class PositionIndicatorWithLabels : public PositionIndicator {

	public:
		PositionIndicatorWithLabels(BookTextView &bookTextView, const ZLTextPositionIndicatorInfo &info);

	private:
		void draw();
	};

	friend class BookTextView::PositionIndicatorWithLabels;

private:
	void readBookState(const DBBook &book);
	int readStackPos(const DBBook &book);
	void saveBookState(const DBBook &book);
private:
	fb::shared_ptr<ZLTextModel> myContentsModel;

	fb::shared_ptr<DBBook> myBook;

	typedef std::deque<Position> PositionStack;
	PositionStack myPositionStack;
	unsigned int myCurrentPointInStack;
	unsigned int myMaxStackSize;

	bool myLockUndoStackChanges;
	bool myStackChanged;
	int myPressedX;
	int myPressedY;
};

inline void BookTextView::preparePaintInfo() {
	ZLTextView::preparePaintInfo();
	saveState();
}

#endif /* __BOOKTEXTVIEW_H__ */
