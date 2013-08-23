// -*- mode: c++; c-basic-offset: 4; -*-

#include <stdlib.h>
#include "onyx/ui/languages.h"
#include "onyx/base/base.h"
#include "adobe_drm_engine/adobe_drm_application.h"
#include "adobe_view_engine/adobe_document.h"
#include "naboo_application.h"

using namespace ui;
using namespace naboo_reader;
using namespace adobe_drm;

int startNabooService(int argc, char** argv)
{
    NabooApplication app(argc, argv);
    NabooApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(dictionary_images);
    Q_INIT_RESOURCE(tts_images);

    if (app.open(app.currentPath()))
    {
        return app.exec();
    }
    else if (!app.errorFound())
    {
        sys::SysStatus::instance().setSystemBusy( false );
        QString err(QCoreApplication::tr("Cannot open the document."));
        ErrorDialog dialog(err, 0);
        return dialog.exec();
    }
    return 0;
}

int startDRMService(int argc, char** argv)
{
    AdobeDRMApplication app(argc, argv);
    AdobeDRMApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(vbf_icons);
    Q_INIT_RESOURCE(onyx_ui_images);

    if (app.execute())
    {
        return app.exec();
    }
    return 0;
}

int startMetadataService(const QString & doc_path)
{
    AdobeDocumentClient document_client;
    if (document_client.openCMSByPath(doc_path))
    {
        if (document_client.loadFulfillmentItemFromDB())
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    if (document_client.open(doc_path) && document_client.isReady())
    {
        if (document_client.loadOptions())
        {
            document_client.saveOptions(false);
        }
        document_client.close();
    }
    return 0;
}

int main(int argc, char** argv)
{
    int index = 0;
    int count = 0;
    QStringList parameters;
    while(index < argc)
    {
        QString arg_name = argv[index++];
        if (arg_name.startsWith("-display", Qt::CaseInsensitive))
        {
            index++;
            continue;
        }
        parameters.append(arg_name);
        count++;
    }

    if (count > 1)
    {
        sys::SysStatus::addDRMEnvironment();
        if (!initializePlatform())
        {
            return 0;
        }

        if (count == 2)
        {
            startNabooService(argc, argv);
        }
        else if (parameters[2] == "metadata")
        {
            startMetadataService(QString::fromLocal8Bit(argv[1]));
        }
        else
        {
            startDRMService(argc, argv);
        }
    }
    return 0;
}
