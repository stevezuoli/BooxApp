#include "naboo_annotations_manager.h"

using namespace ui;
using namespace vbf;
using namespace anno;
using namespace adobe_view;

namespace naboo_reader
{

static const unsigned int BLOB_INIT_SIZE = 512;
static const QChar SEPERATOR(';');

AnnotationManager::AnnotationManager()
    : document_( 0 )
    , loaded_( false )
{
}

AnnotationManager::~AnnotationManager()
{
}

void AnnotationManager::close()
{
    annotation_doc_ = AnnotationDocumentPtr();
    agent_.close();
}

bool AnnotationManager::isValidRange( const Range & range )
{
    PagePosition start_page = static_cast<int>(range.start->getPagePosition());
    PagePosition end_page   = static_cast<int>(range.end->getPagePosition());

    for ( PagePosition i = start_page; i <= end_page; ++i )
    {
        AnnotationPagePtr page = annotation_doc_->getPage( i );
        if (page == 0)
        {
            return false;
        }

        Annotations & annotations = page->annotations();
        if ( !isAnnotationValid( annotations, range ) )
        {
            return false;
        }
    }
    return true;
}

bool AnnotationManager::addAnnotation( const Annotation & anno )
{
    if (annotation_doc_ == 0)
    {
        return false;
    }

    Range anno_range = getRangeFromAnnotation( anno );
    if (!anno_range.isValid())
    {
        return false;
    }

    PagePosition page_position = static_cast<PagePosition>(anno_range.start->getPagePosition());
    AnnotationPagePtr page = annotation_doc_->getPage( page_position );
    if (page == 0)
    {
        return false;
    }

    Annotations & annotations = page->annotations();
    annotations.push_back( anno );
    annotation_doc_->setPageDirty( page );
    return true;
}

bool AnnotationManager::isAnnotationValid( const Annotations & annotations, const Range & range )
{
    // get the locations of annotation
    Annotations::const_iterator iter = annotations.begin();
    for ( ; iter != annotations.end(); ++iter )
    {
        const Annotation & dst = *iter;
        Range dst_range = getRangeFromAnnotation(dst);
        if ( !dst_range.isValid() )
        {
            continue;
        }

        if ( ( range.start > dst_range.start && range.start < dst_range.end ) ||
             ( range.end > dst_range.start && range.end < dst_range.end ) ||
             ( range.start <= dst_range.start && range.end >= dst_range.end ) )
        {
            return false;
        }
    }

    return true;
}

bool AnnotationManager::removeAnnotation( AdobeLocationPtr position,
                                          Range & result_range )
{
    if (annotation_doc_ == 0)
    {
        return false;
    }

    // get the page position from the given position
    PagePosition page_position = static_cast<PagePosition>(position->getPagePosition());
    AnnotationPagePtr page = annotation_doc_->getPage( page_position, false );
    if ( page == 0 )
    {
        return false;
    }

    AnnotationIter iter = page->annotations().begin();
    while ( iter != page->annotations().end() )
    {
        Annotation & anno = *iter;
        Range anno_range = getRangeFromAnnotation( anno );

        if ( anno_range.isValid() &&
             position >= anno_range.start &&
             position <= anno_range.end )
        {
            result_range = anno_range;
            page->annotations().erase( iter );
            annotation_doc_->setPageDirty( page );
            return true;
        }
        iter++;
    }
    return false;
}

bool AnnotationManager::updateAnnotation( const Annotation & anno )
{
    if (annotation_doc_ == 0)
    {
        return false;
    }

    Range anno_range = getRangeFromAnnotation( anno );
    if (!anno_range.isValid())
    {
        return false;
    }

    PagePosition page_position = static_cast<PagePosition>(anno_range.start->getPagePosition());
    AnnotationPagePtr page = annotation_doc_->getPage( page_position );
    if ( page == 0 )
    {
        return false;
    }

    AnnotationIter iter = page->annotations().begin();
    while ( iter != page->annotations().end() )
    {
        Annotation & dst = *iter;
        Range dst_range = getRangeFromAnnotation( dst );
        if ( dst_range.isValid() &&
             dst_range.start == anno_range.start &&
             dst_range.end == anno_range.end )
        {
            dst.mutable_title() = anno.title();
            annotation_doc_->setPageDirty( page );
            return true;
        }
        iter++;
    }
    return false;
}

bool AnnotationManager::getAnnotationFromRange( Range range,
                                                const QString & title,
                                                Annotation & result_anno )
{
    result_anno.mutable_title() = title;
    QStringList locations;
    locations.push_back(range.start->getBookmark());
    locations.push_back(range.end->getBookmark());
    result_anno.mutable_data() = QVariant::fromValue(locations);
    return true;
}

Range AnnotationManager::getRangeFromAnnotation( const Annotation & anno )
{
    Range result;
    QStringList locations = anno.data().value<QStringList>();
    if (locations.size() >= 2)
    {
        QString start = locations.at(0);
        QString end   = locations.at(1);
        result.start  = document_->getLocationFromBookmark( start );
        result.end    = document_->getLocationFromBookmark( end );
    }
    return result;
}

QList<Annotation> AnnotationManager::getAnnotationListByRange( const Range & range )
{
    QList<Annotation> ret;
    if (annotation_doc_ == 0)
    {
        return ret;
    }

    PagePosition start_page = static_cast<PagePosition>(range.start->getPagePosition());
    PagePosition end_page = static_cast<PagePosition>(range.end->getPagePosition());
    for ( PagePosition i = start_page; i <= end_page; ++i )
    {
        // try to load the page if there is any records
        agent_.loadPage( annotation_doc_->path(), i );

        // get the annotations
        AnnotationPagePtr page = annotation_doc_->getPage( i, false );
        if ( page != 0 && !page->annotations().empty())
        {
            ret += page->annotations();
        }
    }
    return ret;
}

bool AnnotationManager::getAnnotationsModel( QStandardItemModel & model )
{
    if (annotation_doc_ == 0)
    {
        qWarning("Annotation Document is Empty");
        return false;
    }

    if ( !loaded_ )
    {
        if ( !agent_.loadAllPages( annotation_doc_->path() ) )
        {
            qWarning("Load all annotation pages failed, document path:%s",
                     annotation_doc_->path().toUtf8().constData());
        }
        loaded_ = true;
    }

    model.setColumnCount(2);

    // get the annotations page by page
    int page_count = static_cast<int>(document_->getPageCount());
    AdobeLocationPtr begin_location = document_->getBeginning();
    PagePosition begin = static_cast<PagePosition>(begin_location->getPagePosition());
    int row = 0;
    for ( PagePosition i = begin; i < page_count; i++ )
    {
        AnnotationPagePtr page = annotation_doc_->getPage( i, false );
        if ( page != 0 && !page->annotations().empty() )
        {
            AnnotationIter iter = page->annotations().begin();
            while ( iter != page->annotations().end() )
            {
                Annotation & anno = *iter;
                Range anno_range = getRangeFromAnnotation( anno );
                if (!anno_range.isValid())
                {
                    iter++;
                    continue;
                }

                QStandardItem *title = new QStandardItem( anno.title() );
                title->setData( anno.data() );
                title->setEditable( false );
                model.setItem( row, 0, title );

                AdobeLocationPtr pos = anno_range.start;
                double page_pos = pos->getPagePosition();
                QString str( QObject::tr("%1") );
                str = str.arg( page_pos + 1.0 );    // the displayed page number should be 1-based

                QStandardItem *page = new QStandardItem( str );
                page->setEditable( false );
                page->setTextAlignment( Qt::AlignCenter );
                model.setItem( row, 1, page );
                iter++; row++;
            }
        }
    }

    model.setHeaderData(0, Qt::Horizontal, QVariant::fromValue(QApplication::tr("Title")), Qt::DisplayRole);
    model.setHeaderData(1, Qt::Horizontal, QVariant::fromValue(QApplication::tr("Page")), Qt::DisplayRole);
    return true;
}

AdobeLocationPtr AnnotationManager::getPositionByAnnoIndex( const QModelIndex & index )
{
    QStringList locations = index.data(Qt::UserRole + 1).value<QStringList>();
    AdobeLocationPtr position;
    if (!locations.isEmpty())
    {
        position = document_->getLocationFromBookmark(locations.at(0));
    }
    return position;
}

void AnnotationManager::save()
{
    agent_.save();
}

}
