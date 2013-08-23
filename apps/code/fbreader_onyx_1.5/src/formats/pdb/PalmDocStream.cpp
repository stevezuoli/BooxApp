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
#include <ZLResource.h>
#include <ZLZDecompressor.h>
#include <iostream>

#include "PalmDocStream.h"
#include "DocDecompressor.h"
#include "HuffDecompressor.h"


PalmDocStream::PalmDocStream(ZLFile &file) : PalmDocLikeStream(file) {
}

PalmDocStream::~PalmDocStream() {
	close();
}

bool PalmDocStream::processRecord() {
	size_t currentOffset = myHeader.Offsets[myRecordIndex];
	if (currentOffset < myBase->offset()) {
	    return false;
	}
	myBase->seek(currentOffset, true);
	size_t nextOffset =
		(myRecordIndex + 1 < myHeader.Offsets.size()) ? 
			myHeader.Offsets[myRecordIndex + 1] : myBase->sizeOfOpened();
	if (nextOffset < currentOffset) {
	    return false;
	}
	unsigned short recordSize = nextOffset - currentOffset;
	switch(myCompressionVersion) {
		case 17480://'DH'	// HuffCDic compression
			myBufferLength = myHuffDecompressorPtr->decompress(*myBase, myBuffer, recordSize, myMaxRecordSize);
			//if (myHuffDecompressorPtr->error()) {
			//	myErrorCode = ERROR_UNKNOWN;
			//} 
			break;
		case 2:				// PalmDoc compression
			myBufferLength = DocDecompressor().decompress(*myBase, myBuffer, recordSize, myMaxRecordSize);
			break;
		case 1:				// No compression
			myBase->read(myBuffer, recordSize);
			myBufferLength = recordSize;
			break;
	}
	myBufferOffset = 0;
	return true;
}

bool PalmDocStream::processZeroRecord() {
	// Uses with offset presetting to zero record offset value
	PdbUtil::readUnsignedShort(*myBase, myCompressionVersion); // myBase offset: ^ + 2
	switch (myCompressionVersion) {
		case 1:
		case 2:
		case 17480:
			break;
		default:
			myErrorCode = ERROR_COMPRESSION;
			return false;
	}	
	myBase->seek(2, false);									// myBase offset: ^ + 4
	PdbUtil::readUnsignedLongBE(*myBase, myTextLength); 	// myBase offset: ^ + 8	
	PdbUtil::readUnsignedShort(*myBase, myTextRecords); 	// myBase offset: ^ + 10

	unsigned short endSectionIndex = myHeader.Offsets.size();
	myMaxRecordIndex = std::min(myTextRecords, (unsigned short)(endSectionIndex - 1));
	//TODO Insert in this point error message about uncompatible records and numRecords from Header
	
	PdbUtil::readUnsignedShort(*myBase, myMaxRecordSize); 	// myBase offset: ^ + 12
	if (myMaxRecordSize == 0) {
		myErrorCode = ERROR_UNKNOWN;
		return false;
	}

	/*
	std::cerr << "PalmDocStream::processRecord0():\n";
	std::cerr << "PDB header indentificator            : " << myHeader.Id << "\n";
	std::cerr << "PDB file system: sizeof opened       : " << myBaseSize << "\n";
	std::cerr << "PDB header/record[0] max index       : " << myMaxRecordIndex << "\n";
	std::cerr << "PDB record[0][0..2] compression      : " << myCompressionVersion << "\n";
	std::cerr << "PDB record[0][2..4] spare            : " << mySpare << "\n";
	std::cerr << "PDB record[0][4..8] text length      : " << myTextLength << "\n";
	std::cerr << "PDB record[0][8..10] text records    : " << myTextRecords << "\n";
	std::cerr << "PDB record[0][10..12] max record size: " << myMaxRecordSize << "\n";
	*/

	if (myHeader.Id == "BOOKMOBI") {
		unsigned short encrypted = 0;
		PdbUtil::readUnsignedShort(*myBase, encrypted); 		// myBase offset: ^ + 14
		if (encrypted) { 										//Always = 2, if encrypted 
			myErrorCode = ERROR_ENCRYPTION;
			return false;
		}
	} else {
		myBase->seek(2, false);
	}


	if (myCompressionVersion == 17480) {
		unsigned long mobiHeaderLength;
		unsigned long huffSectionIndex;
		unsigned long huffSectionNumber;
		unsigned short extraFlags;
		unsigned long initialOffset = myHeader.Offsets[0];				// myBase offset: ^ 
		
		myBase->seek(6, false); 										// myBase offset: ^ + 20
		PdbUtil::readUnsignedLongBE(*myBase, mobiHeaderLength); 		// myBase offset: ^ + 24

		myBase->seek(0x70 - 24, false); 								// myBase offset: ^ + 102 (0x70)
		PdbUtil::readUnsignedLongBE(*myBase, huffSectionIndex); 		// myBase offset: ^ + 106 (0x74)
		PdbUtil::readUnsignedLongBE(*myBase, huffSectionNumber);		// myBase offset: ^ + 110 (0x78)

		if (mobiHeaderLength >= 244) {
			myBase->seek(0xF2 - 0x78, false); 							// myBase offset: ^ + 242 (0xF2) 
			PdbUtil::readUnsignedShort(*myBase, extraFlags);			// myBase offset: ^ + 244 (0xF4)
		} else {
			extraFlags = 0;
		}
		/*
		std::cerr << "mobi header length: " <<  mobiHeaderLength << "\n";
		std::cerr << "Huff's start record  : " << huffSectionIndex << " from " << endSectionIndex - 1 << "\n";
		std::cerr << "Huff's records number: " << huffSectionNumber << "\n";
		std::cerr << "Huff's extraFlags    : " << extraFlags << "\n";
		*/
		const unsigned long endHuffSectionIndex = huffSectionIndex + huffSectionNumber; 
		if ( endHuffSectionIndex > endSectionIndex || huffSectionNumber <= 1) {
			myErrorCode = ERROR_COMPRESSION;
			return false;
		}
		const unsigned long endHuffDataOffset = (endHuffSectionIndex == endSectionIndex) ? (unsigned long)myBase->sizeOfOpened() : myHeader.Offsets[endHuffSectionIndex]; 
		std::vector<unsigned long>::const_iterator beginHuffSectionOffsetIt = myHeader.Offsets.begin() + huffSectionIndex;
		// point to first Huff section
		std::vector<unsigned long>::const_iterator endHuffSectionOffsetIt =	myHeader.Offsets.begin() + endHuffSectionIndex;
		// point behind last Huff section 

		
		myHuffDecompressorPtr = new HuffDecompressor(*myBase, beginHuffSectionOffsetIt, endHuffSectionOffsetIt, endHuffDataOffset, extraFlags);
		myBase->seek(initialOffset, true);									// myBase offset: ^ + 14
	}
	return true;
}

bool PalmDocStream::hasExtraSections() const {
	return myMaxRecordIndex < myHeader.Offsets.size() - 1;
}

std::pair<int,int> PalmDocStream::imageLocation(int index) {
	index += myMaxRecordIndex + 1;
	int recordNumber = myHeader.Offsets.size();
	if (index > recordNumber - 1) {
		return std::pair<int,int>(-1, -1);
	} else {
		int start = myHeader.Offsets[index];
		int end = (index < recordNumber - 1) ?
			myHeader.Offsets[index + 1] : myBaseSize;
		return std::pair<int,int>(start, end - start);
	}
}
