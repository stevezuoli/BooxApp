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

#include <ZLOptions.h>
#include <ZLDialogManager.h>
#include <ZLStringUtil.h>

#include <ZLTextModel.h>
#include <ZLTextParagraph.h>
#include <ZLTextParagraphCursor.h>
#include <ZLTextWord.h>

#include "BookTextView.h"
#include "FBReader.h"

#include "../bookmodel/FBTextKind.h"
#include "../bookmodel/BookModel.h"
//#include "../external/ProgramCollection.h"

#include "../database/booksdb/BooksDB.h"
static const std::string LAST_STATE_GROUP = "LastState";
static const std::string PARAGRAPH_OPTION_NAME = "Paragraph";
static const std::string WORD_OPTION_NAME = "Word";
static const std::string CHAR_OPTION_NAME = "Char";
static const std::string BUFFER_SIZE = "UndoBufferSize";
static const std::string POSITION_IN_BUFFER = "PositionInBuffer";
static const std::string STATE_VALID = "Valid";
static const char * const BUFFER_WORD_PREFIX = "Word_";

BookTextView::BookTextView(FBReader &reader, fb::shared_ptr<ZLPaintContext> context) :
	FBView(reader, context),
	ShowTOCMarksOption(ZLCategoryKey::LOOK_AND_FEEL, "Indicator", "ShowTOCMarks", false) {
	myCurrentPointInStack = 0;
	myMaxStackSize = 20;
	myLockUndoStackChanges = false;
	myStackChanged = false;
}

BookTextView::~BookTextView() {
	saveState();
}

void BookTextView::readBookState(const DBBook &book) {
	ReadingState state;
	if (ZLBooleanOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, STATE_VALID, false).value()) {
		state.Paragraph = ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, PARAGRAPH_OPTION_NAME, 0).value();
		state.Word      = ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, WORD_OPTION_NAME, 0).value();
		state.Character = ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, CHAR_OPTION_NAME, 0).value();
	} else {
		BooksDB::instance().loadBookState(book, state);
	}
	gotoPosition(state.Paragraph, state.Word, state.Character);
}

int BookTextView::readStackPos(const DBBook &book) {
	if (ZLBooleanOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, STATE_VALID, false).value()) {
		return ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, POSITION_IN_BUFFER, 0).value();
	} else {
		return BooksDB::instance().loadStackPos(book);
	}
}

void BookTextView::saveBookState(const DBBook &book) {
	const ReadingState state(
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, PARAGRAPH_OPTION_NAME, 0).value(), 
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, WORD_OPTION_NAME, 0).value(), 
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, CHAR_OPTION_NAME, 0).value()
	);
	const int stackPos = ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, POSITION_IN_BUFFER, 0).value();

	BooksDB::instance().setBookState(book, state);
	BooksDB::instance().setStackPos(book, stackPos);

	ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, PARAGRAPH_OPTION_NAME, 0).setValue(0);
	ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, WORD_OPTION_NAME, 0).setValue(0);
	ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, CHAR_OPTION_NAME, 0).setValue(0);
	ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, POSITION_IN_BUFFER, 0).setValue(0);
	ZLBooleanOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, STATE_VALID, false).setValue(false);
}

void BookTextView::setModel(fb::shared_ptr<ZLTextModel> model, const std::string &language, fb::shared_ptr<DBBook> book) {
	FBView::setModel(model, language);
	if (!myBook.isNull()) {
		saveBookState(*myBook);
	}
	myBook = book;
	if (book.isNull()) {
		return;
	}
	readBookState(*book);
	myPositionStack.clear();
	myCurrentPointInStack = 0;
	BooksDB::instance().loadBookStateStack(*book, myPositionStack);
	myStackChanged = false;
	if (myPositionStack.size() > 0) {
		int stackPos = readStackPos(*book);
		if ((stackPos < 0) || (stackPos > (int) myPositionStack.size())) {
			stackPos = myPositionStack.size();
		}
		myCurrentPointInStack = stackPos;
		while (myPositionStack.size() > myMaxStackSize) {
			myPositionStack.erase(myPositionStack.begin());
			if (myCurrentPointInStack > 0) {
				--myCurrentPointInStack;
			}
			myStackChanged = true;
		}
	}
}

void BookTextView::setContentsModel(fb::shared_ptr<ZLTextModel> contentsModel) {
	myContentsModel = contentsModel;
}

void BookTextView::saveState() {
	const ZLTextWordCursor &cursor = startCursor();

	if (myBook.isNull()) {
		return;
	}

	if (!cursor.isNull()) {
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, PARAGRAPH_OPTION_NAME, 0).setValue(cursor.paragraphCursor().index());
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, WORD_OPTION_NAME, 0).setValue(cursor.elementIndex());
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, CHAR_OPTION_NAME, 0).setValue(cursor.charIndex());
		ZLIntegerOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, POSITION_IN_BUFFER, 0).setValue(myCurrentPointInStack);
		ZLBooleanOption(ZLCategoryKey::STATE, LAST_STATE_GROUP, STATE_VALID, false).setValue(true);

		if (myStackChanged) {
			BooksDB::instance().saveBookStateStack(*myBook, myPositionStack);
			myStackChanged = false;
		}
	}
}

BookTextView::Position BookTextView::cursorPosition(const ZLTextWordCursor &cursor) const {
	return Position(cursor.paragraphCursor().index(), cursor.elementIndex(), cursor.charIndex());
}

bool BookTextView::pushCurrentPositionIntoStack(bool doPushSamePosition) {
	const ZLTextWordCursor &cursor = startCursor();
	if (cursor.isNull()) {
		return false;
	}

	Position pos = cursorPosition(cursor);
	if (!doPushSamePosition && !myPositionStack.empty() && (myPositionStack.back() == pos)) {
		return false;
	}

	myPositionStack.push_back(pos);
	myStackChanged = true;
	while (myPositionStack.size() > myMaxStackSize) {
		myPositionStack.erase(myPositionStack.begin());
		if (myCurrentPointInStack > 0) {
			--myCurrentPointInStack;
		}
	}
	return true;
}

void BookTextView::replaceCurrentPositionInStack() {
	const ZLTextWordCursor &cursor = startCursor();
	if (!cursor.isNull()) {
		myPositionStack[myCurrentPointInStack] = cursorPosition(cursor);
		myStackChanged = true;
	}
}

void BookTextView::gotoParagraph(int num, bool end) {
	if (!empty()) {
		if (!myLockUndoStackChanges) {
			if (myPositionStack.size() > myCurrentPointInStack) {
				myPositionStack.erase(myPositionStack.begin() + myCurrentPointInStack, myPositionStack.end());
				myStackChanged = true;
			}
			pushCurrentPositionIntoStack(false);
			myCurrentPointInStack = myPositionStack.size();
		}

		FBView::gotoParagraph(num, end);
	}
}

bool BookTextView::canUndoPageMove() {
	if (empty()) {
		return false;
	}
	if (myCurrentPointInStack == 0) {
		return false;
	}
	if ((myCurrentPointInStack == 1) && (myPositionStack.size() == 1)) {
		const ZLTextWordCursor &cursor = startCursor();
		if (!cursor.isNull()) {
			return myPositionStack.back() != cursorPosition(cursor);
		}
	}
	return true;
}

void BookTextView::undoPageMove() {
	if (canUndoPageMove()) {
		if (myCurrentPointInStack == myPositionStack.size()) {
			if (!pushCurrentPositionIntoStack(false)) {
				-- myCurrentPointInStack;
			}
		} else {
			replaceCurrentPositionInStack();
		}

		--myCurrentPointInStack;
		Position &pos = myPositionStack[myCurrentPointInStack];
		myLockUndoStackChanges = true;
		gotoPosition(pos.Paragraph, pos.Word, pos.Character);
		myLockUndoStackChanges = false;

		application().refreshWindow();
	}
}

bool BookTextView::canRedoPageMove() {
	return !empty() && (myCurrentPointInStack + 1 < myPositionStack.size());
}

void BookTextView::redoPageMove() {
	if (canRedoPageMove()) {
		replaceCurrentPositionInStack();
		++myCurrentPointInStack;
		Position &pos = myPositionStack[myCurrentPointInStack];
		myLockUndoStackChanges = true;
		gotoPosition(pos.Paragraph, pos.Word, pos.Character);
		myLockUndoStackChanges = false;

		if (myCurrentPointInStack + 1 == myPositionStack.size()) {
			myPositionStack.pop_back();
			myStackChanged = true;
		}

		application().refreshWindow();
	}
}

bool BookTextView::getHyperlinkInfo(const ZLTextElementArea &area, std::string &id, std::string &type) const {
	if ((area.Kind != ZLTextElement::WORD_ELEMENT) &&
			(area.Kind != ZLTextElement::IMAGE_ELEMENT)) {
		return false;
	}
	ZLTextWordCursor cursor = startCursor();
	cursor.moveToParagraph(area.ParagraphIndex);
	cursor.moveToParagraphStart();
	ZLTextKind hyperlinkKind = REGULAR;
	for (int i = 0; i < area.ElementIndex; ++i) {
		const ZLTextElement &element = cursor.element();
		if (element.kind() == ZLTextElement::CONTROL_ELEMENT) {
			const ZLTextControlEntry &control = ((const ZLTextControlElement&)element).entry();
			if (control.isHyperlink()) {
				hyperlinkKind = control.kind();
				id = ((const ZLTextHyperlinkControlEntry&)control).label();
				type = ((const ZLTextHyperlinkControlEntry&)control).hyperlinkType();
			} else if (!control.isStart() && (control.kind() == hyperlinkKind)) {
				hyperlinkKind = REGULAR;
			}
		}
		cursor.nextWord();
	}

	return hyperlinkKind != REGULAR;
}

bool BookTextView::_onStylusPress(int x, int y) {
	myPressedX = x;
	myPressedY = y;

	return false;
}

bool BookTextView::_onStylusRelease(int x, int y) {

    // john: Disable the other processing
	if (FBView::onStylusRelease(x, y)) {
		return true;
	}

	const int deltaX = x - myPressedX;
	if ((deltaX > 5) || (deltaX < -5)) {
		return false;
	}
	const int deltaY = y - myPressedY;
	if ((deltaY > 5) || (deltaY < -5)) {
		return false;
	}
    return false;
}

bool BookTextView::openInternalLink(int x, int y)
{
    const ZLTextElementArea *area = elementByCoordinates(x, y);
    if (area != 0) {
        std::string id;
        std::string type;
        if (getHyperlinkInfo(*area, id, type)) {
            fbreader().tryShowFootnoteView(id, type);
            return true;
        }

        if (fbreader().isDictionarySupported() &&
            fbreader().EnableSingleClickDictionaryOption.value()) {
                const std::string txt = word(*area);
                if (!txt.empty()) {
                    fbreader().openInDictionary(txt);
                    return true;
                }
        }
    }

    return false;
}

bool BookTextView::_onStylusMove(int x, int y) {
	const ZLTextElementArea *area = elementByCoordinates(x, y);
	std::string id;
	std::string type;
	fbreader().setHyperlinkCursor((area != 0) && getHyperlinkInfo(*area, id, type));
	return true;
}

fb::shared_ptr<ZLTextView::PositionIndicator> BookTextView::createPositionIndicator(const ZLTextPositionIndicatorInfo &info) {
	return new PositionIndicatorWithLabels(*this, info);
}

BookTextView::PositionIndicatorWithLabels::PositionIndicatorWithLabels(BookTextView &bookTextView, const ZLTextPositionIndicatorInfo &info) : PositionIndicator(bookTextView, info) {
}

void BookTextView::PositionIndicatorWithLabels::draw() {
	PositionIndicator::draw();

	const BookTextView& bookTextView = (const BookTextView&)textView();

	if (bookTextView.ShowTOCMarksOption.value()) {
		fb::shared_ptr<ZLTextModel> contentsModelPtr = bookTextView.myContentsModel;
		if (!contentsModelPtr.isNull()) {
			ContentsModel &contentsModel = (ContentsModel&)*contentsModelPtr;
			const int marksNumber = contentsModel.paragraphsNumber();
			const size_t startIndex = startTextIndex();
			const size_t endIndex = endTextIndex();
			const std::vector<size_t> &textSizeVector = textSize();
			const int fullWidth = right() - left() - 1;
			const size_t startPosition = textSizeVector[startIndex];
			const size_t fullTextSize = textSizeVector[endIndex] - startPosition;
			const int bottom = this->bottom();
			const int top = this->top();
			for (int i = 0; i < marksNumber; ++i) {
				size_t reference = contentsModel.reference((ZLTextTreeParagraph*)contentsModel[i]);
				if ((startIndex < reference) && (reference < endIndex)) {
					int position = left() + 2 + (int)
						(1.0 * fullWidth * (textSizeVector[reference] - startPosition) / fullTextSize);
					context().drawLine(position, bottom, position, top);
				}
			}
		}
	}
}

void BookTextView::scrollToHome() {
	if (!startCursor().isNull() &&
			startCursor().isStartOfParagraph() &&
			startCursor().paragraphCursor().index() == 0) {
		return;
	}

	gotoParagraph(0, false);
	fbreader().refreshWindow();
}

void BookTextView::paint() {
	FBView::paint();
	std::string pn;
	ZLStringUtil::appendNumber(pn, pageIndex());
	fbreader().setVisualParameter(FBReader::PageIndexParameter, pn);
}
