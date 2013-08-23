
#include "handlers.h"
#include "DocBookReader.h"


static DocBookReader *s_reader;
void setBookReader(DocBookReader *reader)
{
    s_reader = reader;
}

DocBookReader *bookReader()
{
    return s_reader;
}

std::string indent;
wvWare::U8 MyInlineReplacementHandler::tab()
{
    std::cout << indent << "INLINE: tab" << std::endl;
    return InlineReplacementHandler::tab();
}

wvWare::U8 MyInlineReplacementHandler::hardLineBreak()
{
    std::cout << indent << "INLINE: hard line break" << std::endl;
    return InlineReplacementHandler::hardLineBreak();
}

wvWare::U8 MyInlineReplacementHandler::columnBreak()
{
    std::cout << indent << "INLINE: column break" << std::endl;
    return InlineReplacementHandler::columnBreak();
}

wvWare::U8 MyInlineReplacementHandler::nonBreakingHyphen()
{
    std::cout << indent << "INLINE: non breaking hyphen" << std::endl;
    return InlineReplacementHandler::nonBreakingHyphen();
}

wvWare::U8 MyInlineReplacementHandler::nonRequiredHyphen()
{
    std::cout << indent << "INLINE: non required hyphen" << std::endl;
    return InlineReplacementHandler::nonRequiredHyphen();
}

wvWare::U8 MyInlineReplacementHandler::nonBreakingSpace()
{
    std::cout << indent << "INLINE: non breaking space" << std::endl;
    return InlineReplacementHandler::nonBreakingSpace();
}


void MySubDocumentHandler::bodyStart()
{
    std::cout << indent << "SUBDOCUMENT: body start" << std::endl;
    indent.push_back( ' ' );
    SubDocumentHandler::bodyStart();
}

void MySubDocumentHandler::bodyEnd()
{
    std::cout << indent << "SUBDOCUMENT: body end" << std::endl;
    indent.erase( indent.size() - 1 );
    SubDocumentHandler::bodyEnd();
}

void MySubDocumentHandler::footnoteStart()
{
    std::cout << indent << "SUBDOCUMENT: footnote start" << std::endl;
    indent.push_back( ' ' );
    SubDocumentHandler::footnoteStart();
}

void MySubDocumentHandler::footnoteEnd()
{
    std::cout << indent << "SUBDOCUMENT: footnote end" << std::endl;
    indent.erase( indent.size() - 1 );
    SubDocumentHandler::footnoteEnd();
}

void MySubDocumentHandler::headersStart()
{
    std::cout << indent << "SUBDOCUMENT: headers start" << std::endl;
    indent.push_back( ' ' );
    SubDocumentHandler::headersStart();
}

void MySubDocumentHandler::headersEnd()
{
    std::cout << indent << "SUBDOCUMENT: headers end" << std::endl;
    indent.erase( indent.size() - 1 );
    SubDocumentHandler::headersEnd();
}

void MySubDocumentHandler::headerStart( wvWare::HeaderData::Type type )
{
    std::cout << indent << "SUBDOCUMENT: header start " << static_cast<int>( type ) << std::endl;
    indent.push_back( ' ' );
    SubDocumentHandler::headerStart( type );
}

void MySubDocumentHandler::headerEnd()
{
    std::cout << indent << "SUBDOCUMENT: header end" << std::endl;
    indent.erase( indent.size() - 1 );
    SubDocumentHandler::headerEnd();
}


void MyTableHandler::tableRowStart( wvWare::SharedPtr<const wvWare::Word97::TAP> tap )
{
    std::cout << indent << "TABLE: table row start" << std::endl;
    indent.push_back( ' ' );
    tap->dump();
    TableHandler::tableRowStart( tap );
}

void MyTableHandler::tableRowEnd()
{
    std::cout << indent << "TABLE: table row end" << std::endl;
    indent.erase( indent.size() - 1 );
    TableHandler::tableRowEnd();
}

void MyTableHandler::tableCellStart()
{
    std::cout << indent << "TABLE: table cell start" << std::endl;
    indent.push_back( ' ' );
    TableHandler::tableCellStart();
}

void MyTableHandler::tableCellEnd()
{
    std::cout << indent << "TABLE: table cell end" << std::endl;
    indent.erase( indent.size() - 1 );
    TableHandler::tableCellEnd();
}


void MyPictureHandler::bitmapData( wvWare::OLEImageReader& /*reader*/, wvWare::SharedPtr<const wvWare::Word97::PICF> /*picf*/ )
{
    // ###### TODO
    std::cout << indent << "PICTURE: bitmapData" << std::endl;
}

void MyPictureHandler::wmfData( wvWare::OLEImageReader& /*reader*/, wvWare::SharedPtr<const wvWare::Word97::PICF> /*picf*/ )
{
    // ###### TODO
    std::cout << indent << "PICTURE: wmfData" << std::endl;
}

void MyPictureHandler::externalImage( const wvWare::UString& name, wvWare::SharedPtr<const wvWare::Word97::PICF> /*picf*/ )
{
    // ###### TODO
    std::cout << indent << "PICTURE: externalImage: " << name.ascii() << std::endl;
}


void MyTextHandler::sectionStart( wvWare::SharedPtr<const wvWare::Word97::SEP> sep )
{
    std::cout << indent << "TEXT: section start" << std::endl;
    indent.push_back( ' ' );
    sep->dump();
    TextHandler::sectionStart( sep );
}

void MyTextHandler::sectionEnd()
{
    std::cout << indent << "TEXT: section end" << std::endl;
    indent.erase( indent.size() - 1 );
    TextHandler::sectionEnd();
}

void MyTextHandler::pageBreak()
{
    std::cout << indent << "TEXT: page break" << std::endl;
    TextHandler::pageBreak();
}

void MyTextHandler::headersFound( const wvWare::HeaderFunctor& parseHeaders )
{
    std::cout << indent << "TEXT: headers found" << std::endl;
    TextHandler::headersFound( parseHeaders );
}

void MyTextHandler::paragraphStart( wvWare::SharedPtr<const wvWare::ParagraphProperties> paragraphProperties )
{
    std::cout << indent << "TEXT: paragraph start" << std::endl;
    indent.push_back( ' ' );
    paragraphProperties->pap().dump();
    if ( paragraphProperties->listInfo() )
        paragraphProperties->listInfo()->dump();
    TextHandler::paragraphStart( paragraphProperties );

    bookReader()->baseBookReader().beginParagraph();
    //bookReader()->baseBookReader().addControl(STRONG, true);

    ZLTextStyleEntry entry;
    switch (paragraphProperties->pap().jc)
    {
    case 0:
        entry.setAlignmentType(ALIGN_LEFT);
        break;
    case 1:
        entry.setAlignmentType(ALIGN_CENTER);
        break;
    case 2:
        entry.setAlignmentType(ALIGN_RIGHT);
        break;
    case 3:
        entry.setAlignmentType(ALIGN_JUSTIFY);
        break;
    default:
        entry.setAlignmentType(ALIGN_JUSTIFY);
        break;
    }
    bookReader()->baseBookReader().addControl(entry);
}

void MyTextHandler::paragraphEnd()
{
    std::cout << indent << "TEXT: paragraph end" << std::endl;
    indent.erase( indent.size() - 1 );
    TextHandler::paragraphEnd();
    bookReader()->baseBookReader().endParagraph();
}

void MyTextHandler::runOfText( const wvWare::UString& text, wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    /*
    std::cout << indent << "TEXT: run of text" << std::endl;
    chp->dump();
    std::cout << "TEXT: [";
    for ( int i = 0; i < text.length(); ++i )
        std::cout << "<" << text[ i ].unicode() << "|" << text[ i ].low() << ">";
    std::cout << "]" << std::endl;
    */
    TextHandler::runOfText( text, chp );
    ZLTextStyleEntry entry;
    entry.setBold(chp->fBold);
    entry.setItalic(chp->fItalic);
    entry.setFontSizeMag(chp->hps);

    //bookReader()->baseBookReader().pushKind(EMPHASIS);
    bookReader()->baseBookReader().addControl(entry);
    std::string t(text.ascii(), text.length());

    if (t.size() > 0)
    {
        bookReader()->baseBookReader().addData(t);
        if (t.at(t.size() - 1) == '\r')
        {
            bookReader()->baseBookReader().endParagraph();
        }
    }

    //bookReader()->baseBookReader().popKind();
    //bookReader()->baseBookReader().endParagraph();

}

void MyTextHandler::specialCharacter( wvWare::TextHandler::SpecialCharacter character,
                                    wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    std::cout << indent << "TEXT: special character " << static_cast<int>( character ) << std::endl;
    chp->dump();
    TextHandler::specialCharacter( character, chp );
}

void MyTextHandler::footnoteFound( wvWare::FootnoteData::Type type, wvWare::UChar character,
                                 wvWare::SharedPtr<const wvWare::Word97::CHP> chp,
                                 const wvWare::FootnoteFunctor& parseFootnote )
{
    std::cout << indent << "TEXT: footnote found " << static_cast<int>( type )
              << " character " << character.unicode() << std::endl;
    chp->dump();
    TextHandler::footnoteFound( type, character, chp, parseFootnote );
}

void MyTextHandler::footnoteAutoNumber( wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    std::cout << indent << "TEXT: footnote auto-number" << std::endl;
    chp->dump();
    TextHandler::footnoteAutoNumber( chp );
}

void MyTextHandler::fieldStart( const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    std::cout << indent << "TEXT: field start " << static_cast<int>( fld->ch ) << std::endl;
    indent.push_back( ' ' );
    chp->dump();
    TextHandler::fieldStart( fld, chp );
}

void MyTextHandler::fieldSeparator( const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    std::cout << indent << "TEXT: field separator " << static_cast<int>( fld->ch ) << std::endl;
    chp->dump();
    TextHandler::fieldSeparator( fld, chp );
}

void MyTextHandler::fieldEnd( const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    std::cout << indent << "TEXT: field end " << static_cast<int>( fld->ch ) << std::endl;
    chp->dump();
    indent.erase( indent.size() - 1 );
    TextHandler::fieldEnd( fld, chp );
}

void MyTextHandler::tableRowFound( const wvWare::TableRowFunctor& tableRow, wvWare::SharedPtr<const wvWare::Word97::TAP> tap )
{
    std::cout << indent << "TEXT: table row found" << std::endl;
    tap->dump();
    TextHandler::tableRowFound( tableRow, tap );
}

void MyTextHandler::pictureFound( const wvWare::PictureFunctor& picture, wvWare::SharedPtr<const wvWare::Word97::PICF> picf,
                             wvWare::SharedPtr<const wvWare::Word97::CHP> chp )
{
    std::cout << indent << "TEXT: picture found" << std::endl;
    picf->dump();
    chp->dump();
    TextHandler::pictureFound( picture, picf, chp );
}


