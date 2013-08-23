#ifndef ADOBE_DOCUMENT_H_
#define ADOBE_DOCUMENT_H_

#include "adobe_utils.h"
#include "adobe_location.h"
#include "adobe_search_conf.h"

namespace dpdoc
{
    class Document;
    class TOCItem;
};

using namespace vbf;
namespace adobe_view
{

class AdobeDocumentClientPrivate;
class AdobeDocumentClient : public QObject
{
    Q_OBJECT
public:
    AdobeDocumentClient();
    ~AdobeDocumentClient();
    inline cms::ContentManager & database() { return database_; }
    inline TasksHandler* tasksHandler() { return &tasks_handler_; }
    inline bool isReady() const { return is_ready_; }
    inline bool errorFound() const { return error_found_; }
    inline bool tryingPassword() const { return tried_existing_password_; }
    inline bool hasLicenseRequired() const { return license_required_; }
    inline const QString & path() const { return path_; }

    bool open(const QString & path);
    bool close();
    bool isTheDocument(const QString &path);
    QString metaData(const QString & name, QString & attrs);
    void setPassword(const QString & password);
    void setSearchBusy(bool busy);

    // Type
    int type();

    // Search
    AdobeSearchConf & searchConf() { return search_conf_; }
    AdobeLocationPtr searchString( const AdobeLocationPtr & start,
                                   const AdobeLocationPtr & end,
                                   const QString & pattern,
                                   const bool forward );
    bool findText( const Range & from,
                   unsigned int flags,
                   const QString & pattern,
                   Range & result );
    void handleSearchResult( bool found, const AdobeSearchConf * search_conf );
    void sendSearchRequest( AdobeSearchConf * search_conf );

    // TOC
    bool hasToc();
    QStandardItemModel* getTOCModel();
    AdobeLocationPtr getPositionByTOCIndex(const QModelIndex & index);

    // Location
    AdobeLocationPtr getBeginning();
    AdobeLocationPtr getEnd();
    AdobeLocationPtr getLocationFromBookmark(const QString & bookmark);
    AdobeLocationPtr getLocationFromPagePosition(double pos);
    double getPagePositionFromLocation(const AdobeLocationPtr & location);
    QString getText(const Range & range);
    AdobeContentIteratorPtr getContentIterator(int variety, const AdobeLocationPtr & location);

    // Configuration
    bool saveOptions(bool add_to_history = true);
    bool loadOptions();
    Configuration & getConf() { return conf_; }
    bool openCMSByPath(const QString & path);    // for metadata scanning
    bool loadFulfillmentItemFromDB();
    bool loadFulfillmentItemFromDocument();

    // Page Count
    double getPageCount();

Q_SIGNALS:
    /// Document loading process is ready signal
    void documentReadySignal();
    void documentCloseSignal();
    void documentErrorSignal(const QString & error);
    void documentRequestLicense(const QString & type,
                                const QString & resource_id,
                                const QByteArray & request_data);
    void searchSucceeded();
    void searchNoMoreResults();
    void requestPassword();

private:
    typedef QVector<dpdoc::TOCItem*> TOCItems;

private:
    bool openCMS();
    void loadTOCItem(QStandardItem * parent, dpdoc::TOCItem * cur_item);
    void notifyDocumentLoadError(const QString & error);
    void notifyRequestLicense(const QString & type,
                              const QString & resource_id,
                              const QByteArray & request_data);
    void notifyDocumentReady();
    void notifyRequestPassword();
    QString retrievePasswordFromDB();
    dpdoc::Document* document();
    bool saveMetadata();

private:
    scoped_ptr<AdobeDocumentClientPrivate> doc_private_;
    TasksHandler               tasks_handler_;

    Configuration              conf_;
    cms::ContentManager        database_;
    FulfillmentItemInfo        fulfillment_item_;
    QByteArray                 url_;
    QString                    path_; ///< path in utf8
    QString                    mime_type_;
    int                        type_;

    // TOC elements
    dpdoc::TOCItem             *toc_root_;
    scoped_ptr<QStandardItemModel> toc_model_;
    TOCItems                   toc_items_;

    // Search
    AdobeSearchConf            search_conf_;

    bool                       is_ready_;
    bool                       options_loaded_;
    bool                       error_found_;
    bool                       tried_existing_password_;
    bool                       license_required_;

    friend class AdobeDocumentClientPrivate;
    friend class AdobeRendererClient;
};

};

#endif
