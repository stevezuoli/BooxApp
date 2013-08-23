
#ifndef ELISA_CONTENT_PARSER_H_
#define ELISA_CONTENT_PARSER_H_

#include "onyx/data/content.h"
#include <QtCore/QtCore>
#include <QtXml/QtXml>

class ElisaContentParser
{
public:
    ElisaContentParser();
    ~ElisaContentParser();

public:
    static bool parseContents(content::Books & books, const QByteArray & data);
    static bool parseConfirmNewContents(const QByteArray & data);

private:
    static bool parseContent(QDomNode & node, content::Book & content);
    static bool parseConfirmNewContent(QDomElement & root);
};

#endif
