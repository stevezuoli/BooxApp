
#include "elisa_content_parser.h"


ElisaContentParser::ElisaContentParser()
{
}

ElisaContentParser::~ElisaContentParser()
{
}

bool ElisaContentParser::parseContents(content::Books & books,
                                const QByteArray & data)

{
    QDomDocument doc;
    doc.setContent(data);
    QDomElement root = doc.documentElement();
    QDomNode n = root.firstChild();

    content::Book book;
    while (!n.isNull())
    {
        if (parseContent(n, book))
        {
            books.push_back(book);
        }
        n = n.nextSibling();
    }
    return true;
}

bool ElisaContentParser::parseContent(QDomNode & node, content::Book  & content)
{
    QDomElement e = node.toElement();
    if (!e.isNull())
    {
        QString tag = e.tagName();
        if (tag == "content")
        {
            content.bookid = node.firstChildElement("bookid").toElement().text();
            content.url = node.firstChildElement("url").toElement().text();
            content.internalid = node.firstChildElement("internalid").toElement().text().toInt();
        }
        return true;
    }
    return false;
}

bool ElisaContentParser::parseConfirmNewContents(const QByteArray & data)
{
    QDomDocument doc;
    doc.setContent(data);
    QDomElement root = doc.documentElement();
    return parseConfirmNewContent(root);
}

bool ElisaContentParser::parseConfirmNewContent(QDomElement & root)
{
    QString string = root.firstChildElement("success").text();
    if (string.compare("OK", Qt::CaseInsensitive) == 0)
    {
        return true;
    }

    qDebug("error in parseConfirmNewContent.");
    string = root.firstChildElement("bookid").text();
    qDebug("bookid %s", qPrintable(string));

    string = root.firstChildElement("description").text();
    qDebug("desc %s", qPrintable(string));
    return false;
}
