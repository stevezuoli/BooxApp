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

#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>

#include <ZLFile.h> 
#include <ZLInputStream.h> 

#include "ZLCharSequence.h"
#include "ZLStatistics.h"
#include "ZLStatisticsXMLReader.h"

const std::string ZLStatisticsXMLReader::ITEM_TAG = "item";
const std::string ZLStatisticsXMLReader::STATISTICS_TAG = "statistics";

// C99 only.
static unsigned long long my_atoll(const char *instr)
{
#ifndef _WINDOWS
    return atoll(instr);
#else
    unsigned long long retval;
    retval = 0;
    for (; *instr; instr++) {
        retval = 10*retval + (*instr - '0');
    }
    return retval;
#endif
}

void ZLStatisticsXMLReader::startElementHandler(const char *tag, const char **attributes) {
	if (STATISTICS_TAG == tag) {
		size_t volume = atoi(attributeValue(attributes, "volume"));
		unsigned long long squaresVolume = my_atoll(attributeValue(attributes, "squaresVolume"));
		//std::cerr << "XMLReader: frequencies sum & ^2: " << volume << ":" << squaresVolume << "\n";
		myStatisticsPtr = new ZLArrayBasedStatistics( atoi(attributeValue(attributes, "charSequenceSize")), atoi(attributeValue(attributes, "size")), volume, squaresVolume);
	} else if (ITEM_TAG == tag) {
		const char *sequence = attributeValue(attributes, "sequence");
		const char *frequency = attributeValue(attributes, "frequency");
		if ((sequence != 0) && (frequency != 0)) {
			std::string hexString(sequence);
			myStatisticsPtr->insert(ZLCharSequence(hexString), atoi(frequency));
		}
	}
}

static std::map<std::string, fb::shared_ptr<ZLArrayBasedStatistics> > statisticsMap;

fb::shared_ptr<ZLArrayBasedStatistics> ZLStatisticsXMLReader::readStatistics(const std::string &fileName) {
	std::map<std::string, fb::shared_ptr<ZLArrayBasedStatistics> >::iterator it = statisticsMap.find(fileName);
	if (it != statisticsMap.end()) {
		return it->second;
	}

	fb::shared_ptr<ZLInputStream> statisticsStream = ZLFile(fileName).inputStream();
    if (statisticsStream.isNull() || !statisticsStream->open()) {
       	//std::cerr << "null stream\n";
        return 0;
    }
   	readDocument(statisticsStream);
	statisticsStream->close();

	statisticsMap.insert(std::make_pair(fileName, myStatisticsPtr));

	{ // print map sizing:
		unsigned sz = statisticsMap.size();
		unsigned ptrsize = sizeof(fb::shared_ptr<ZLArrayBasedStatistics>);
		unsigned statsize = sizeof(ZLArrayBasedStatistics);
		std::cerr << "STATMAP: size = " << sz * (ptrsize + statsize) << " bytes\n";
	}

	return myStatisticsPtr; 
}
