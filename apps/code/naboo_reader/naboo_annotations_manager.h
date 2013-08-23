#ifndef NABOO_ANNOTATIONS_MANAGER_H_
#define NABOO_ANNOTATIONS_MANAGER_H_

#include "naboo_utils.h"
#include "onyx/data/annotation_agent.h"

using namespace ui;
using namespace vbf;
using namespace anno;
using namespace adobe_view;

namespace naboo_reader
{

class AnnotationManager
{
public:
    AnnotationManager();
    ~AnnotationManager();

    inline void attachDocument(AdobeDocumentClient *document);
    void close();
    void save();

    bool addAnnotation( const Annotation & anno );
    bool removeAnnotation( AdobeLocationPtr position,
                           Range & result_range );
    bool updateAnnotation( const Annotation & anno );
    bool getAnnotationFromRange( Range range,
                                 const QString & title,
                                 Annotation & result_anno );
    Range getRangeFromAnnotation( const Annotation & anno );
    bool isValidRange( const Range & range );

    QList<Annotation> getAnnotationListByRange( const Range & range );
    bool getAnnotationsModel( QStandardItemModel & model );
    AdobeLocationPtr getPositionByAnnoIndex( const QModelIndex & index );

private:
    bool isAnnotationValid( const Annotations & annotations, const Range & range );

private:
    AdobeDocumentClient     *document_;
    AnnotationAgent         agent_;
    AnnotationDocumentPtr   annotation_doc_;
    bool                    loaded_;
};

void AnnotationManager::attachDocument(AdobeDocumentClient *document)
{
    document_ = document;
    annotation_doc_ = agent_.getDocument(document_->path());
}

};

#endif
